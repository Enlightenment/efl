#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>

#define RIGHTS_LEN CMSG_LEN(sizeof(int))

#define IOVSET(_iov, _addr, _len) \
   (_iov)->iov_base = (void *)(_addr); \
   (_iov)->iov_len = (_len);

/* local variables */
static int _ecore_drm_init_count = 0;
static int _ecore_drm_sockets[2] = { -1, -1 };
static struct cmsghdr *cmsgptr = NULL;

/* external variables */
struct udev *udev;
int _ecore_drm_log_dom = -1;
#ifdef LOG_TO_FILE
FILE *lg;
#endif

static Eina_Bool 
_ecore_drm_sockets_create(void)
{
   if (_ecore_drm_sockets[0] > -1) return EINA_TRUE;

   /* create a pair of sequenced sockets (fixed-length)
    * NB: when reading from one of these, it is required that we read 
    * an entire packet with each read() call */
   if (socketpair(AF_LOCAL, SOCK_SEQPACKET | SOCK_NONBLOCK, 
                  0, _ecore_drm_sockets) < 0)
     {
        ERR("Socketpair Failed: %m");
        return EINA_FALSE;
     }

   /* NB: We don't want cloexec for the sockets. That would cause them to be 
    * closed when we exec the child process but we need them open so that 
    * we can pass messages */
   /* if (fcntl(_ecore_drm_sockets[0], F_SETFD, FD_CLOEXEC) < 0) */
   /*   { */
   /*      ERR("Failed to set CLOEXEC: %m"); */
   /*      return EINA_FALSE; */
   /*   } */

   /* DBG("Parent Socket: %d", _ecore_drm_sockets[0]); */
   /* DBG("Child Socket: %d", _ecore_drm_sockets[1]); */

   return EINA_TRUE;
}

static Eina_Bool 
_ecore_drm_launcher_spawn(void)
{
   pid_t pid;

   if ((pid = fork()) < 0) return EINA_FALSE;

   if (pid == 0)
     {
        char renv[64], wenv[64], buff[PATH_MAX];
        char *args[1] = { NULL };
        sigset_t mask;

        /* read socket for slave is 1 */
        snprintf(renv, sizeof(renv), "ECORE_DRM_LAUNCHER_SOCKET_READ=%d", 
                 _ecore_drm_sockets[1]);

        /* write socket for slave is 0 */
        snprintf(wenv, sizeof(wenv), "ECORE_DRM_LAUNCHER_SOCKET_WRITE=%d", 
                 _ecore_drm_sockets[0]);

        /* assemble exec path */
        snprintf(buff, sizeof(buff), 
                 "%s/ecore_drm/bin/%s/ecore_drm_launch", 
                 PACKAGE_LIB_DIR, MODULE_ARCH);

        /* don't give our signal mask to the child */
        sigemptyset(&mask);
        sigaddset(&mask, SIGTERM);
        sigaddset(&mask, SIGCHLD);
        sigaddset(&mask, SIGINT);
        sigaddset(&mask, SIGTTIN);
        sigaddset(&mask, SIGTTOU);
        sigprocmask(SIG_UNBLOCK, &mask, NULL);

        /* NB: We need to use execve here so that capabilities are inherited.
         * Also, this should set Our (ecore_drm) effective uid to be the 
         * owner of the launched process (setuid in this case) */
        char *ev[3] = { strdup(renv), strdup(wenv), NULL };
        execve(buff, args, ev);
     }
   else
     {
        int status;

        while (waitpid(pid, &status, WNOHANG) < 0)
          if (errno != EINTR) break;

        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static ssize_t 
_ecore_drm_socket_send(int opcode, int fd, void *data, size_t bytes)
{
   Ecore_Drm_Message dmsg;
   struct iovec iov[2];
   struct msghdr msg;
   ssize_t size;

   /* Simplified version of sending messages. We don't need to send any 
    * 'credentials' with this as it is just basically an IPC to send over 
    * our request to the slave process */

   /* NB: Hmm, don't think we need to set any socket options here */

   memset(&dmsg, 0, sizeof(dmsg));

   IOVSET(iov + 0, &dmsg, sizeof(dmsg));
   IOVSET(iov + 1, &data, bytes);

   dmsg.opcode = opcode;
   dmsg.size = bytes;

   msg.msg_name = NULL;
   msg.msg_namelen = 0;
   msg.msg_iov = iov;
   msg.msg_iovlen = 2;
   msg.msg_flags = 0;

   if ((!cmsgptr) && (!(cmsgptr = malloc(RIGHTS_LEN))))
     return -1;

   cmsgptr->cmsg_level = SOL_SOCKET;
   cmsgptr->cmsg_type = SCM_RIGHTS;
   cmsgptr->cmsg_len = RIGHTS_LEN;

   msg.msg_control = cmsgptr;
   msg.msg_controllen = RIGHTS_LEN;

   if (fd > -1)
     *((int *)CMSG_DATA(cmsgptr)) = fd;
   else
     *((int *)CMSG_DATA(cmsgptr)) = _ecore_drm_sockets[1];

   errno = 0;
   size = sendmsg(_ecore_drm_sockets[1], &msg, MSG_EOR);
   if (errno != 0)
     {
        DBG("Error Sending Message: %m");
     }

   /* DBG("Sent %li bytes to Socket %d", size, _ecore_drm_sockets[1]); */

   return size;
}

static int 
_ecore_drm_socket_receive(int opcode EINA_UNUSED, int *fd, void **data, size_t bytes)
{
   int ret = ECORE_DRM_OP_FAILURE;
   Ecore_Drm_Message dmsg;
   struct cmsghdr *cmsg;
   struct iovec iov[2];
   struct msghdr msg;
   char buff[CMSG_SPACE(sizeof(fd))];
   /* ssize_t size; */

   memset(&dmsg, 0, sizeof(dmsg));
   memset(&buff, 0, sizeof(buff));

   IOVSET(iov + 0, &dmsg, sizeof(dmsg));
   IOVSET(iov + 1, &buff, sizeof(buff));

   msg.msg_name = NULL;
   msg.msg_namelen = 0;
   msg.msg_iov = iov;
   msg.msg_iovlen = 2;
   msg.msg_flags = 0;

   if ((!cmsgptr) && (!(cmsgptr = malloc(RIGHTS_LEN))))
     return -1;

   msg.msg_control = cmsgptr;
   msg.msg_controllen = RIGHTS_LEN;

   errno = 0;
   recvmsg(_ecore_drm_sockets[0], &msg, 0);
   if (errno != 0)
     {
        ERR("Failed to receive message: %m");
        return -1;
     }

   /* DBG("Received %li bytes from %d", size, _ecore_drm_sockets[0]); */

   for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; 
        cmsg = CMSG_NXTHDR(&msg, cmsg))
     {
        if (cmsg->cmsg_level != SOL_SOCKET)
          continue;

        switch (cmsg->cmsg_type)
          {
           case SCM_RIGHTS:
             if (fd) *fd = *((int *)CMSG_DATA(cmsg));
             switch (dmsg.opcode)
               {
                case ECORE_DRM_OP_DEVICE_OPEN:
                case ECORE_DRM_OP_DEVICE_CLOSE:
                case ECORE_DRM_OP_TTY_OPEN:
                case ECORE_DRM_OP_DEVICE_MASTER_DROP:
                case ECORE_DRM_OP_DEVICE_MASTER_SET:
                  if ((fd) && (*fd >= 0)) ret = ECORE_DRM_OP_SUCCESS;
                  if (data) memcpy(*data, buff, bytes);
                  break;
                default:
                  break;
               }
             break;
           default:
             break;
          }
     }

   return ret;
}

void 
_ecore_drm_message_send(int opcode, int fd, void *data, size_t bytes)
{
   _ecore_drm_socket_send(opcode, fd, data, bytes);
}

Eina_Bool 
_ecore_drm_message_receive(int opcode, int *fd, void **data, size_t bytes)
{
   int ret = ECORE_DRM_OP_FAILURE;

   ret = _ecore_drm_socket_receive(opcode, fd, data, bytes);
   if (ret != ECORE_DRM_OP_SUCCESS) return EINA_FALSE;

   return EINA_TRUE;
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

   /* try to init ecore */
   if (!ecore_init()) 
     {
        eina_shutdown();
        return --_ecore_drm_init_count;
     }

   /* try to init ecore_event */
   if (!ecore_event_init())
     {
        ecore_shutdown();
        eina_shutdown();
        return --_ecore_drm_init_count;
     }

   /* set logging level */
   eina_log_level_set(EINA_LOG_LEVEL_DBG);

   setvbuf(stdout, NULL, _IONBF, 0);
   setvbuf(stderr, NULL, _IONBF, 0);

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

   /* try to init udev */
   if (!(udev = udev_new()))
     goto udev_err;

   /* try to create the socketpair */
   if (!_ecore_drm_sockets_create())
     goto sock_err;

   /* try to run Spartacus */
   if (!_ecore_drm_launcher_spawn())
     goto spawn_err;

   /* return init count */
   return _ecore_drm_init_count;

spawn_err:
   close(_ecore_drm_sockets[0]);
   close(_ecore_drm_sockets[1]);
sock_err:
   if (udev) udev_unref(udev);
udev_err:
   ecore_shutdown();
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
   close(_ecore_drm_sockets[0]);
   close(_ecore_drm_sockets[1]);

   /* shutdown ecore_event */
   ecore_event_shutdown();

   /* shutdown ecore */
   ecore_shutdown();

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

EAPI void *
ecore_drm_gbm_get(Ecore_Drm_Device *dev)
{
   if (!dev) return NULL;

#ifdef HAVE_GBM
   return dev->gbm;
#endif

   return NULL;
}

EAPI unsigned int 
ecore_drm_gbm_format_get(Ecore_Drm_Device *dev)
{
   if (!dev) return 0;
   return dev->format;
}
