#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#ifdef HAVE_ENVIRON
# define _GNU_SOURCE 1
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#include <Elementary.h>

#ifdef HAVE_ENVIRON
extern char **environ;
#endif

static double restart_time = 0.0;

#define LENGTH_OF_SOCKADDR_UN(s) (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))

static struct sigaction old_sigint;
static struct sigaction old_sigterm;
static struct sigaction old_sigquit;
static struct sigaction old_sigalrm;
static struct sigaction old_sigusr1;
static struct sigaction old_sigusr2;
static struct sigaction old_sighup;
static struct sigaction old_sigchld;
static struct sigaction old_sigsegv;
static struct sigaction old_sigill;
static struct sigaction old_sigfpe;
static struct sigaction old_sigbus;
static struct sigaction old_sigabrt;
static int _log_dom = -1;

#define CRI(...) EINA_LOG_DOM_CRIT(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)

static void
post_fork(void *data EINA_UNUSED)
{
   sigaction(SIGINT, &old_sigint, NULL);
   sigaction(SIGTERM, &old_sigterm, NULL);
   sigaction(SIGQUIT, &old_sigquit, NULL);
   sigaction(SIGALRM, &old_sigalrm, NULL);
   sigaction(SIGUSR1, &old_sigusr1, NULL);
   sigaction(SIGUSR2, &old_sigusr2, NULL);
   sigaction(SIGHUP, &old_sighup, NULL);
   sigaction(SIGCHLD, &old_sigchld, NULL);
   sigaction(SIGSEGV, &old_sigsegv, NULL);
   sigaction(SIGILL, &old_sigill, NULL);
   sigaction(SIGFPE, &old_sigfpe, NULL);
   sigaction(SIGBUS, &old_sigbus, NULL);
   sigaction(SIGABRT, &old_sigabrt, NULL);
   if ((_log_dom > -1) && (_log_dom != EINA_LOG_DOMAIN_GLOBAL))
     {
        eina_log_domain_unregister(_log_dom);
        _log_dom = -1;
     }
}

static void
child_handler(int x EINA_UNUSED, siginfo_t *info EINA_UNUSED, void *data EINA_UNUSED)
{
   int status;
   while (waitpid(-1, &status, WNOHANG) > 0);
}

static void
crash_handler(int x EINA_UNUSED, siginfo_t *info EINA_UNUSED, void *data EINA_UNUSED)
{
   double t;

   ERR("crash detected. restarting.");
   t = ecore_time_get();
   if ((t - restart_time) <= 2.0)
     {
        CRI("crash too fast - less than 2 seconds. abort restart");
        exit(-1);
     }
   ecore_app_restart();
}

static void
handle_run(int fd, unsigned long bytes)
{
   unsigned char *buf = NULL;
   int i;
   char **argv = NULL;
   char **envir = NULL;
   char *cwd;
   int argc, envnum;
   unsigned long off;

   if (bytes < 1)
     {
        CRI("no bytes to quicklaunch");
        return;
     }
   DBG("Starting building up process.");
   _elm_startup_time = ecore_time_unix_get();

   buf = alloca(bytes);
   if (read(fd, buf, bytes) != (int)bytes)
     {
        CRI("cannot read %i bytes of args and environment data", (int)bytes);
        close(fd);
        return;
     }
   close(fd);
   buf[bytes - 1] = 0;

   argc = ((unsigned long *)(buf))[0];
   envnum = ((unsigned long *)(buf))[1];

   if (argc <= 0)
     {
        CRI("no executable specified");
        return;
     }

   argv = alloca(argc * sizeof(char *));
   if (envnum > 0) envir = alloca(envnum * sizeof(char *));
   off = ((unsigned long *)(buf))[2 + argc + envnum] - sizeof(unsigned long);
   cwd = (char *)(buf + off);

   for (i = 0; i < argc; i++)
     {
        off = ((unsigned long *)(buf))[2 + i] - sizeof(unsigned long);
        argv[i] = (char *)(buf + off);
     }

#ifdef HAVE_ENVIRON
   if (envir)
     {
#ifdef HAVE_CLEARENV
        clearenv();
#else
        environ = NULL;
#endif
        for (i = 0; i < envnum; i++)
          {
             off = ((unsigned long *)(buf))[2 + argc + i] - sizeof(unsigned long);
             envir[i] = (char *)(buf + off);
             putenv(envir[i]);
          }
     }
#endif

   INF("Requested to run '%s' with %i arguments and %i environment.",
           argv[0], argc - 1, envnum);
   // Try new form before trying old form
   if (!efl_quicklaunch_prepare(argc, argv, cwd))
     {
        WRN("Failed to prepare with new EFL_MAIN macro, switching to legacy.");
        elm_quicklaunch_prepare(argc, argv, cwd);
     }

   elm_quicklaunch_fork(argc, argv, cwd, post_fork, NULL);
   elm_quicklaunch_cleanup();
}

int
main(int argc, char **argv)
{
   int sock, socket_unix_len;
   struct stat st;
   struct sockaddr_un socket_unix;
   struct linger lin;
   char buf[PATH_MAX];
   struct sigaction action;
   const char *domain;
   int ret = 0;
   size_t len;

   if (!eina_init())
     {
        fprintf(stderr, "ERROR: failed to init eina.");
        exit(-1);
     }
   _log_dom = eina_log_domain_register
      ("elementary_quicklaunch", EINA_COLOR_CYAN);
   if (_log_dom < 0)
     {
        EINA_LOG_ERR("could not register elementary_quicklaunch log domain.");
        _log_dom = EINA_LOG_DOMAIN_GLOBAL;
     }

   if (!(domain = getenv("ELM_QUICKLAUNCH_DOMAIN")))
     {
        domain = getenv("WAYLAND_DISPLAY");
        if (!domain) domain = getenv("DISPLAY");
        if (!domain) domain = "unknown";
     }
   eina_vpath_resolve_snprintf(buf, sizeof(buf), "(:usr.run:)/elm-ql-%i", getuid());
   if (stat(buf, &st) < 0)
     {
        ret = mkdir(buf, S_IRUSR | S_IWUSR | S_IXUSR);
        if (ret < 0)
          {
             CRI("cannot create directory '%s'", buf);
             exit(-1);
          }
     }
   eina_vpath_resolve_snprintf(buf, sizeof(buf), "(:usr.run:)/elm-ql-%i/%s", getuid(), domain);
   unlink(buf);
   sock = socket(AF_UNIX, SOCK_STREAM, 0);
   if (sock < 0)
     {
        CRI("cannot create socket for socket for '%s': %s",
                 buf, strerror(errno));
        exit(-1);
     }
   if (!eina_file_close_on_exec(sock, EINA_TRUE))
     {
        CRI("cannot set close on exec socket for '%s' (fd=%d): %s",
                 buf, sock, strerror(errno));
        exit(-1);
     }
   lin.l_onoff = 1;
   lin.l_linger = 0;
   if (setsockopt(sock, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) < 0)
     {
        CRI("cannot set linger for socket for '%s' (fd=%d): %s",
                 buf, sock, strerror(errno));
        exit(-1);
     }
   socket_unix.sun_family = AF_UNIX;
   len = strlen(buf);
   if (len > sizeof(socket_unix.sun_path))
     {
        CRI("socket path '%s' is too long for buffer", buf);
        exit(-1);
     }
   memcpy(socket_unix.sun_path, buf, len);
   if (len < sizeof(socket_unix.sun_path)) socket_unix.sun_path[len] = 0;
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   if (bind(sock, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     {
        CRI("cannot bind socket for '%s' (fd=%d): %s",
                 buf, sock, strerror(errno));
        exit(-1);
     }
   if (listen(sock, 4096) < 0)
     {
        CRI("listen(sock=%d, 4096): %s", sock, strerror(errno));
        exit(-1);
     }
   elm_quicklaunch_mode_set(EINA_TRUE);
   elm_quicklaunch_init(argc, argv);
   restart_time = ecore_time_get();

   memset(&action, 0, sizeof(struct sigaction));
   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGINT, &action, &old_sigint);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGTERM, &action, &old_sigterm);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGQUIT, &action, &old_sigquit);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGALRM, &action, &old_sigalrm);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGUSR1, &action, &old_sigusr1);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGUSR2, &action, &old_sigusr2);

   action.sa_handler = SIG_DFL;
   action.sa_sigaction = NULL;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGHUP, &action, &old_sighup);

   action.sa_handler = NULL;
   action.sa_sigaction = child_handler;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGCHLD, &action, &old_sigchld);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGSEGV, &action, &old_sigsegv);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGILL, &action, &old_sigill);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGFPE, &action, &old_sigfpe);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGBUS, &action, &old_sigbus);

   action.sa_handler = NULL;
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGABRT, &action, &old_sigabrt);

   for (;;)
     {
        int fd;
        struct sockaddr_un client;
        socklen_t len;

        len = sizeof(struct sockaddr_un);
        fd = accept(sock, (struct sockaddr *)&client, &len);

        DBG("Accepting connection.");
        elm_quicklaunch_sub_init(argc, argv);
        // don't seed since we are doing this AFTER launch request
        // elm_quicklaunch_seed();
        if (fd >= 0)
          {
             unsigned long bytes;
             int num;

             num = read(fd, &bytes, sizeof(unsigned long));
             if (num == sizeof(unsigned long))
               handle_run(fd, bytes);
          }
        while (elm_quicklaunch_sub_shutdown() > 0);
     }
   elm_quicklaunch_shutdown();

   if ((_log_dom > -1) && (_log_dom != EINA_LOG_DOMAIN_GLOBAL))
     {
        eina_log_domain_unregister(_log_dom);
        _log_dom = -1;
     }
   eina_shutdown();

   return 0;
}
