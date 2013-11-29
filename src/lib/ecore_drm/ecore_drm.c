#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"

/* local variables */
static int _ecore_drm_init_count = 0;
static int _ecore_drm_socket_fd[2] = { -1, -1 };
union cmsg_data 
{
   unsigned char d[4];
   int fd;
};

/* external variables */
struct udev *udev;
int _ecore_drm_log_dom = -1;
#ifdef LOG_TO_FILE
FILE *lg;
#endif

static ssize_t 
_ecore_drm_socket_send(int opcode, int fd, void *data)
{
   int s = 0;
   ssize_t size;
   Ecore_Drm_Message *dmsg;
   struct msghdr msg;
   struct iovec iov;
   struct cmsghdr *cmsg;
   union cmsg_data *cdata;
   char ctrl[CMSG_SPACE(sizeof(cdata->fd))];

   s = sizeof(*dmsg) + 1;
   if (!(dmsg = malloc(s))) return -1;

   /* assemble message to send */
   dmsg->opcode = opcode;
   dmsg->data = data;
   dmsg->size = sizeof(data);

   iov.iov_base = dmsg;
   iov.iov_len = sizeof(dmsg);

   memset(&msg, 0, sizeof(msg));

   msg.msg_name = NULL;
   msg.msg_namelen = 0;
   msg.msg_iov = &iov;
   msg.msg_iovlen = 1;
   msg.msg_control = ctrl;
   msg.msg_controllen = sizeof(ctrl);

   cmsg = CMSG_FIRSTHDR(&msg);
   cmsg->cmsg_len = CMSG_LEN(sizeof(int));
   cmsg->cmsg_level = SOL_SOCKET;
   cmsg->cmsg_type = SCM_RIGHTS;

   *((int *)CMSG_DATA(cmsg)) = fd;

   do
     {
        size = sendmsg(_ecore_drm_socket_fd[1], &msg, 0);
     } while ((size < 0) && (errno == EINTR));

   free(dmsg);

   if (size < 0)
     ERR("Failed to Send Socket Message");

   return size;
}

static Eina_Bool 
_ecore_drm_socket_create(void)
{
   /* test if we have already opened the socketpair */
   if (_ecore_drm_socket_fd[0] > -1) return EINA_TRUE;

   /* setup socketpair */
   if (socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, _ecore_drm_socket_fd) < 0)
     {
        ERR("Socketpair Failed: %m");
        return EINA_FALSE;
     }

   DBG("Parent Socket: %d", _ecore_drm_socket_fd[0]);
   DBG("Child Socket: %d", _ecore_drm_socket_fd[1]);

   return EINA_TRUE;
}

static void 
_ecore_drm_launcher_spawn(void)
{
   pid_t pid;

   /* TODO: test if spartacus is still alive */

   /* fork */
   if ((pid = vfork()) < 0)
     CRIT("Failed to fork: %m");

   if (pid != 0)
     {
        DBG("Parent Sending Pass Fd Message");
        _ecore_drm_socket_send(ECORE_DRM_OP_WRITE_FD_SET, 
                               _ecore_drm_socket_fd[1], NULL);
     }
   else if (pid == 0) /* child process. exec spartacus */
     {
        char buff[PATH_MAX], env[32];
        char *args[1] = { NULL };
        sigset_t mask;

        /* if we are the child, start a rebellion */
        DBG("Start Spartacus");

        /* drop privileges */
        if (setuid(getuid()) < 0)
          WRN("Failed to drop privileges");

        /* set to read socket for launch process to read from */
        snprintf(env, sizeof(env), "%d", _ecore_drm_socket_fd[0]);
        setenv("ECORE_DRM_LAUNCHER_SOCKET", env, 1);

        sigemptyset(&mask);
        sigaddset(&mask, SIGTERM);
        sigaddset(&mask, SIGCHLD);
        sigaddset(&mask, SIGINT);
        sigprocmask(SIG_UNBLOCK, &mask, NULL);

        /* assemble exec path */
        snprintf(buff, sizeof(buff), 
                 "%s/ecore_drm/bin/%s/ecore_drm_launch", 
                 PACKAGE_LIB_DIR, MODULE_ARCH);

        /* try to start a rebellion */
        execv(buff, args);

        /* execv does not return unless there is a problem, spew the error */
        ERR("Spartacus failed to start a rebellion: %m");

        /* unsetenv("ECORE_DRM_LAUNCHER_SOCKET"); */
     }
}

/**
 * @defgroup Ecore_Drm_Init_Group Drm Library Init and Shutdown Functions
 * 
 * Functions that start and shutdown the Ecore_Drm Library.
 */

/**
 * Initialize the Ecore_Drm library
 * 
 * @return  The number of times the library has been initialized without
 *          being shut down. 0 is returned if an error occurs.
 * 
 * @ingroup Ecore_Drm_Init_Group
 */
EAPI int 
ecore_drm_init(void)
{
   /* if we have already initialized, return the count */
   if (++_ecore_drm_init_count != 1) return _ecore_drm_init_count;

   /* try to init eina */
   if (!eina_init()) return --_ecore_drm_init_count;

   /* set logging level */
   eina_log_level_set(EINA_LOG_LEVEL_DBG);

   setvbuf(stdout, NULL, _IONBF, 0);

   /* optionally log output to a file */
#ifdef LOG_TO_FILE
   int log_fd;
   char log_path[PATH_MAX];
   mode_t um;

   /* assemble logging file path */
   strcpy(log_path, "/tmp/ecore_drm_XXXXXX");

   /* create temporary logging file */
   um = umask(S_IRWXG | S_IRWXO);
   log_fd = mkstemp(log_path);
   umask(um);

   /* try to open logging file */
   if (!(lg = fdopen(log_fd, "w")))
     goto log_err;

   eina_log_print_cb_set(eina_log_print_cb_file, lg);
#endif

   /* try to create logging domain */
   _ecore_drm_log_dom = 
     eina_log_domain_register("ecore_drm", ECORE_DRM_DEFAULT_LOG_COLOR);
   if (!_ecore_drm_log_dom)
     {
        EINA_LOG_ERR("Could not create log domain for Ecore_Drm");
        goto log_err;
     }

   /* set default logging level for this domain */
   if (!eina_log_domain_level_check(_ecore_drm_log_dom, EINA_LOG_LEVEL_DBG))
     eina_log_domain_level_set("ecore_drm", EINA_LOG_LEVEL_DBG);

   /* try to create socketpair */
   if (!_ecore_drm_socket_create())
     {
        ERR("Could not create socketpair");
        goto sock_err;
     }

   /* create udev connection for later device handling */
   if (!(udev = udev_new()))
     {
        ERR("Could not create udev handle: %m");
        goto udev_err;
     }

   /* try to run SPARTACUS !! */
   _ecore_drm_launcher_spawn();

   /* return init count */
   return _ecore_drm_init_count;

udev_err:
   close(_ecore_drm_socket_fd[0]);
   close(_ecore_drm_socket_fd[1]);
sock_err:
   eina_log_domain_unregister(_ecore_drm_log_dom);
   _ecore_drm_log_dom = -1;
log_err:
#ifdef LOG_TO_FILE
   if (lg) fclose(lg);
#endif

   /* shutdown eina */
   eina_shutdown();

   return --_ecore_drm_init_count;
}

/**
 * Shutdown the Ecore_Drm library.
 * 
 * @return  The number of times the library has been initialized without
 *          being shutdown. 0 is returned if an error occurs.
 * 
 * @ingroup Ecore_Drm_Init_Group
 */
EAPI int 
ecore_drm_shutdown(void)
{
   /* if we are still in use, decrement init count and get out */
   if (--_ecore_drm_init_count != 0) return _ecore_drm_init_count;

   /* close udev handle */
   if (udev) udev_unref(udev);

   /* close sockets */
   close(_ecore_drm_socket_fd[0]);
   close(_ecore_drm_socket_fd[1]);

   /* unregsiter log domain */
   eina_log_domain_unregister(_ecore_drm_log_dom);
   _ecore_drm_log_dom = -1;

#ifdef LOG_TO_FILE
   if (lg) fclose(lg);
#endif

   /* shutdown eina */
   eina_shutdown();

   /* return init count */
   return _ecore_drm_init_count;
}
