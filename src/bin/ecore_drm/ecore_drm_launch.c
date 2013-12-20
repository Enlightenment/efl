#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <Eina.h>
#include <Ecore_Drm.h>

static int _send_msg(int opcode, int fd, void *data, size_t bytes);

static struct cmsghdr *cmsgptr = NULL;
static int _read_fd = -1;
static int _write_fd = -1;

#if defined(SCM_CREDS) // Bsd
# define CRED_STRUCT cmsgcred
# define SCM_CREDTYPE SCM_CREDS
#elif defined(SCM_CREDENTIALS) // Linux (3.2.0 ?)
# define CRED_STRUCT ucred
# define SCM_CREDTYPE SCM_CREDENTIALS
# define CRED_OPT SO_PASSCRED
#endif

#define RIGHTS_LEN CMSG_LEN(sizeof(int))
#define CREDS_LEN CMSG_LEN(sizeof(struct CRED_STRUCT))
#define CONTROL_LEN (RIGHTS_LEN + CREDS_LEN)

#define IOVSET(_iov, _addr, _len) \
   (_iov)->iov_base = (void *)(_addr); \
   (_iov)->iov_len = (_len);

static int 
_open_device(const char *device)
{
   int fd = -1, ret = ECORE_DRM_OP_SUCCESS;

   if ((fd = open(device, O_RDWR)) < 0)
     {
        fprintf(stderr, "Failed to open device: %s: %m\n", device);
        ret = ECORE_DRM_OP_FAILURE;
     }

   return ret;
}

static int 
_read_fd_get(void)
{
   char *ev, *end;
   int fd = -1, flags = -1;

   if (!(ev = getenv("ECORE_DRM_LAUNCHER_SOCKET_READ")))
     return -1;

   fd = strtol(ev, &end, 0);
   if (*end != '\0') return -1;

   flags = fcntl(fd, F_GETFD);
   if (flags < 0) return -1;

   return fd;
}

static int 
_write_fd_get(void)
{
   char *ev, *end;
   int fd = -1, flags = -1;

   if (!(ev = getenv("ECORE_DRM_LAUNCHER_SOCKET_WRITE")))
     return -1;

   fd = strtol(ev, &end, 0);
   if (*end != '\0') return -1;

   flags = fcntl(fd, F_GETFD);
   if (flags < 0) return -1;

   return fd;
}

static int 
_send_msg(int opcode, int fd, void *data, size_t bytes)
{
   Ecore_Drm_Message dmsg;
   struct CRED_STRUCT *creds;
   struct cmsghdr *cmsg;
   struct iovec iov[2];
   struct msghdr msg;
   ssize_t size;

   /* send a message to the calling process */
   /* 'fd' is the fd to send */

   IOVSET(iov + 0, &dmsg, sizeof(dmsg));
   IOVSET(iov + 1, data, bytes);

   dmsg.opcode = opcode;
   dmsg.size = bytes;

   msg.msg_iov = iov;
   msg.msg_iovlen = 2;
   msg.msg_name = NULL;
   msg.msg_namelen = 0;
   msg.msg_flags = 0;

   if ((!cmsgptr) && (!(cmsgptr = malloc(CONTROL_LEN))))
     return -1;

   msg.msg_control = cmsgptr;
   msg.msg_controllen = CONTROL_LEN;

   cmsg = cmsgptr;

   cmsg->cmsg_level = SOL_SOCKET;
   cmsg->cmsg_type = SCM_RIGHTS;
   cmsg->cmsg_len = RIGHTS_LEN;
   *((int *)CMSG_DATA(cmsg)) = fd;

   cmsg = CMSG_NXTHDR(&msg, cmsg);
   cmsg->cmsg_level = SOL_SOCKET;
   cmsg->cmsg_type = SCM_CREDTYPE;
   cmsg->cmsg_len = CREDS_LEN;
   creds = (struct CRED_STRUCT *)CMSG_DATA(cmsg);

#if defined(SCM_CREDENTIALS)
   creds->uid = geteuid();
   creds->gid = getegid();
   creds->pid = getpid();
#endif

   errno = 0;
   size = sendmsg(_write_fd, &msg, MSG_EOR);
   if (errno != 0)
     {
        fprintf(stderr, "Failed to send message: %s", strerror(errno));
        return -1;
     }

   return size;
}

static int 
_recv_msg(void)
{
   int ret = -1;
   Ecore_Drm_Message dmsg;
   struct iovec iov[2];
   struct msghdr msg;
   struct cmsghdr *cmsg;
   char data[BUFSIZ];
   ssize_t size;

/* #if defined(CRED_OPT) */
/*    struct CRED_STRUCT *creds; */
/*    if (getsockopt(_read_fd, SOL_SOCKET, CRED_OPT,  */
/*                   &creds, sizeof(struct CRED_STRUCT)) < 0) */
/*      return -1; */
/* #endif */

   fprintf(stderr, "Received Message\n");

   IOVSET(iov + 0, &dmsg, sizeof(dmsg));
   IOVSET(iov + 1, &data, sizeof(data));

   msg.msg_iov = iov;
   msg.msg_iovlen = 2;
   msg.msg_name = NULL;
   msg.msg_namelen = 0;

   if ((!cmsgptr) && (!(cmsgptr = malloc(CONTROL_LEN))))
     return -1;

   msg.msg_control = cmsgptr;
   msg.msg_controllen = CONTROL_LEN;

   errno = 0;
   size = recvmsg(_read_fd, &msg, MSG_ERRQUEUE);
   if (errno != 0)
     {
        fprintf(stderr, "Failed to receive message: %m\n");
        return -1;
     }

   fprintf(stderr, "\tReceived %d bytes\n", (int)size);

   for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; 
        cmsg = CMSG_NXTHDR(&msg, cmsg))
     {
        if (cmsg->cmsg_level != SOL_SOCKET)
          continue;

        switch (cmsg->cmsg_type)
          {
           case SCM_RIGHTS:
             switch (dmsg.opcode)
               {
                case ECORE_DRM_OP_DEVICE_OPEN:
                  fprintf(stderr, "Open Device: %s\n", (char *)data);
                  ret = _open_device((char *)data);
                  break;
                default:
                  fprintf(stderr, "Unhandled Opcode: %d\n", dmsg.opcode);
                  break;
               }
             break;
           /* case SCM_CREDTYPE: */
           /*   creds = (struct CRED_STRUCT *)CMSG_DATA(cmsg); */
           /*   break; */
           default:
             fprintf(stderr, "Unhandled message type: %d\n", cmsg->cmsg_type);
             return -1;
             break;
          }
     }

   return size;
}

int 
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   struct epoll_event ev, events[1];
   int ret, i, _epoll_fd = -1;

   fprintf(stderr, "Spartacus Is Alive\n");

   _read_fd = _read_fd_get();
   if (_read_fd < 0) return EXIT_FAILURE;

   _write_fd = _write_fd_get();
   if (_write_fd < 0) return EXIT_FAILURE;

   _epoll_fd = epoll_create(1);

   memset(&ev, 0, sizeof(ev));
   ev.events = EPOLLIN;
   ev.data.fd = _read_fd;

   if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _read_fd, &ev) < 0)
     {
        return EXIT_FAILURE;
     }

   memset(&events, 0, sizeof(events));

   for (;;)
     {
        ret = epoll_wait(_epoll_fd, events, sizeof(events) / sizeof(struct epoll_event), 0);
        if (ret < 0)
          {
             fprintf(stderr, "Epoll Failed: %m\n");
             return EXIT_FAILURE;
          }

        for (i = 0; i < ret; i++)
          {
             if (events[i].data.fd != _read_fd) continue;

             if (events[i].events & EPOLLIN)
               {
                  fprintf(stderr, "Epoll Data In\n");
                  _recv_msg();
               }
             else if (events[i].events & EPOLLERR)
               {
                  fprintf(stderr, "Epoll Data Error\n");
               }
          }
     }

   fprintf(stderr, "Spartacus Is Dead\n");

   return EXIT_SUCCESS;
}
