#include <Elementary.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

static double restart_time = 0.0;

#define LENGTH_OF_SOCKADDR_UN(s) (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))

static void
handle_run(int fd, unsigned long bytes)
{
   unsigned char *buf = NULL;
   int i, num;
   char **argv = NULL;
   char *cwd;
   int argc;
   
   buf = alloca(bytes);
   if ((num = read(fd, buf, bytes)) < 0)
     {
        close(fd);
        return;
     }
   close(fd);
   argc = ((unsigned long *)(buf))[0];
   argv = (char **)(&(((unsigned long *)(buf))[1]));
   for (i = 0; i < argc; i++) argv[i] = buf + (unsigned long)argv[i];
   cwd = argv[argc - 1] + strlen(argv[argc - 1]) + 1;
   elm_quicklaunch_prepare(argc, argv);
   elm_quicklaunch_fork(argc, argv, cwd);
   elm_quicklaunch_cleanup();
}

static void
child_handler(int n)
{
   int status;
   wait(&status);
}

EAPI void
crash_handler(int x, siginfo_t *info, void *data)
{
   double t;
   
   EINA_ERROR_PERR("elementary_quicklaunch: crash detected. restarting.\n");
   t = ecore_time_get();
   if ((t - restart_time) <= 2.0)
     {
        EINA_ERROR_PERR("elementary_quicklaunch: crash too fast - less than 2 seconds. abort restart\n");
        exit(-1);
     }
   ecore_app_restart();
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
   
   if (!getenv("DISPLAY"))
     {
        EINA_ERROR_PERR("elementary_quicklaunch: DISPLAY env var not set\n");
        exit(-1);
     }
   snprintf(buf, sizeof(buf), "/tmp/elm-ql-%i", getuid());
   if (stat(buf, &st) < 0) mkdir(buf, S_IRUSR | S_IWUSR | S_IXUSR);
   snprintf(buf, sizeof(buf), "/tmp/elm-ql-%i/%s", getuid(), getenv("DISPLAY"));
   unlink(buf);
   sock = socket(AF_UNIX, SOCK_STREAM, 0);
   if (sock < 0)
     {
        perror("elementary_quicklaunch: socket(AF_UNIX, SOCK_STREAM, 0)");
        EINA_ERROR_PERR("elementary_quicklaunch: cannot create socket for socket for '%s'\n", buf);
        exit(-1);
     }
   if (fcntl(sock, F_SETFD, FD_CLOEXEC) < 0)
     {
        perror("elementary_quicklaunch: fcntl(sock, F_SETFD, FD_CLOEXEC)");
        EINA_ERROR_PERR("elementary_quicklaunch: cannot set close on exec socket for '%s'\n", buf);
        exit(-1);
     }
   lin.l_onoff = 1;
   lin.l_linger = 0;
   if (setsockopt(sock, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) < 0)
     {
        perror("elementary_quicklaunch: setsockopt(sock, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) ");
        EINA_ERROR_PERR("elementary_quicklaunch: cannot set linger for socket for '%s'\n", buf);
        exit(-1);
     }
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   if (bind(sock, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     {
        perror("elementary_quicklaunch: bind(sock, (struct sockaddr *)&socket_unix, socket_unix_len)");
        EINA_ERROR_PERR("elementary_quicklaunch: cannot bind socket for '%s'\n", buf);
        exit(-1);
     }
   if (listen(sock, 4096) < 0)
     {
        perror("elementary_quicklaunch: listen(sock, 4096)");
        exit(-1);
     }
   elm_quicklaunch_init(argc, argv);
   signal(SIGINT, SIG_DFL);
   signal(SIGTERM, SIG_DFL);
   signal(SIGQUIT, SIG_DFL);
   signal(SIGALRM, SIG_DFL);
   signal(SIGUSR1, SIG_DFL);
   signal(SIGUSR2, SIG_DFL);
   signal(SIGHUP, SIG_DFL);
   signal(SIGCHLD, child_handler);

   restart_time = ecore_time_get();
   
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGSEGV, &action, NULL);
   
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGILL, &action, NULL);
   
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGFPE, &action, NULL);
   
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGBUS, &action, NULL);
   
   action.sa_sigaction = crash_handler;
   action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGABRT, &action, NULL);
   
   for (;;)
     {
        int fd;
        struct sockaddr_un client;
        socklen_t len;

        elm_quicklaunch_sub_init(argc, argv);
        elm_quicklaunch_seed();
        len = sizeof(struct sockaddr_un);
        fd = accept(sock, (struct sockaddr *)&client, &len);
        if (fd >= 0)
          {
             int bytes;
             char line[4096];

             read(fd, &bytes, sizeof(unsigned long));
             handle_run(fd, bytes);
          }
        elm_quicklaunch_sub_shutdown();
     }
   elm_quicklaunch_shutdown();
   return 0;
}
