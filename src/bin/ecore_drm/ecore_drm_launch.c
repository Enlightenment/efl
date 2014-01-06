#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
/* #include <syslog.h> */
/* #include <pwd.h> */

/* #include <linux/major.h> */
/* #include <linux/vt.h> */
/* #include <linux/kd.h> */

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#include <Eina.h>
#include <Ecore_Drm.h>

#define RIGHTS_LEN CMSG_LEN(sizeof(int))

#define IOVSET(_iov, _addr, _len) \
   (_iov)->iov_base = (void *)(_addr); \
   (_iov)->iov_len = (_len);

/* local prototypes */
static int _send_msg(int opcode, int fd, void *data, size_t bytes);

/* local variables */
static struct cmsghdr *cmsgptr = NULL;
static int _read_fd = -1;
static int _write_fd = -1;

static int 
_open_device(const char *device)
{
   int fd = -1, ret = ECORE_DRM_OP_SUCCESS;

   if (!device) 
     {
        ret = ECORE_DRM_OP_FAILURE;
        _send_msg(ECORE_DRM_OP_DEVICE_OPEN, fd, &ret, sizeof(int));
        return ret;
     }

   fprintf(stderr, "Launcher Trying to Open Device: %s\n", device);

   if ((fd = open(device, O_RDWR | O_NONBLOCK)) < 0)
     {
        fprintf(stderr, "Failed to Open Device: %s: %m\n", device);
        ret = ECORE_DRM_OP_FAILURE;
     }
   else
     fprintf(stderr, "Launcher Opened Device: %s %d\n", device, fd);

   _send_msg(ECORE_DRM_OP_DEVICE_OPEN, fd, &ret, sizeof(int));

   return ret;
}

static int 
_close_device(int fd)
{
   int ret = ECORE_DRM_OP_SUCCESS;

   if (!fd)
     {
        ret = ECORE_DRM_OP_FAILURE;
        _send_msg(ECORE_DRM_OP_DEVICE_CLOSE, fd, &ret, sizeof(int));
        return ret;
     }

   close(fd);

   _send_msg(ECORE_DRM_OP_DEVICE_CLOSE, fd, &ret, sizeof(int));

   return ret;
}

static int 
_open_tty(const char *name)
{
   int fd = -1, ret = ECORE_DRM_OP_SUCCESS;
   /* struct stat st; */

   if (!name) goto fail;

   fprintf(stderr, "Launcher Trying to Open Tty: %s\n", name);

   if ((fd = open(name, O_RDWR | O_NOCTTY)) < 0)
     {
        fprintf(stderr, "Failed to Open Tty: %s: %m\n", name);
        goto fail;
     }
   else
     fprintf(stderr, "Launcher Opened Tty: %s %d\n", name, fd);

   /* if ((fstat(fd, &st) == -1) ||  */
   /*     (major(st.st_rdev) != TTY_MAJOR) || (minor(st.st_rdev) == 0)) */
   /*   { */
   /*      fprintf(stderr, "%d is Not a Tty\n", fd); */
   /*      goto fail; */
   /*   } */

   _send_msg(ECORE_DRM_OP_TTY_OPEN, fd, &ret, sizeof(int));

   return ret;

fail:
   if (fd > -1) close(fd);
   fd = -1;
   ret = ECORE_DRM_OP_FAILURE;
   _send_msg(ECORE_DRM_OP_DEVICE_OPEN, fd, &ret, sizeof(int));
   return ret;
}

static int 
_drop_master(int fd)
{
   int ret = ECORE_DRM_OP_SUCCESS;

   fprintf(stderr, "Drop Master: %d\n", fd);

   if (drmDropMaster(fd) != 0) 
     {
        ret = ECORE_DRM_OP_FAILURE;
        fprintf(stderr, "\tFailed to drop master: %m\n");
     }

   _send_msg(ECORE_DRM_OP_DEVICE_MASTER_DROP, fd, &ret, sizeof(int));

   close(fd);

   return ret;
}

static int 
_set_master(int fd)
{
   int ret = ECORE_DRM_OP_SUCCESS;

   fprintf(stderr, "Set Master: %d\n", fd);

   if (drmSetMaster(fd) != 0) 
     {
        ret = ECORE_DRM_OP_FAILURE;
        fprintf(stderr, "\tFailed to set master: %m\n");
     }

   _send_msg(ECORE_DRM_OP_DEVICE_MASTER_SET, fd, &ret, sizeof(int));

   close(fd);

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

   fprintf(stderr, "Got Read FD: %d\n", fd);

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

   fprintf(stderr, "Got Write FD: %d\n", fd);

   return fd;
}

static int 
_send_msg(int opcode, int fd, void *data, size_t bytes)
{
   Ecore_Drm_Message dmsg;
   struct iovec iov[2];
   struct msghdr msg;
   ssize_t size;

   /* send a message to the calling process */
   /* 'fd' is the fd to send */

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

   fprintf(stderr, "Launcher Sending FD: %d\n", fd);
   *((int *)CMSG_DATA(cmsgptr)) = fd;

   errno = 0;
   size = sendmsg(_write_fd, &msg, MSG_EOR);
   if (errno != 0)
     {
        fprintf(stderr, "Failed to send message: %s", strerror(errno));
        return -1;
     }

   fprintf(stderr, "Launcher Wrote %li to %d\n", size, _write_fd);

   return size;
}

static int 
_recv_msg(void)
{
   int fd = -1;
   Ecore_Drm_Message dmsg;
   struct iovec iov[2];
   struct msghdr msg;
   struct cmsghdr *cmsg = NULL;
   char data[BUFSIZ];
   ssize_t size;

   fprintf(stderr, "Received Message\n");

   memset(&dmsg, 0, sizeof(dmsg));
   memset(&data, 0, sizeof(data));

   IOVSET(iov + 0, &dmsg, sizeof(dmsg));
   IOVSET(iov + 1, &data, sizeof(data));

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
   size = recvmsg(_read_fd, &msg, 0);
   if (errno != 0)
     {
        fprintf(stderr, "Failed to receive message: %m\n");
        return -1;
     }

   fprintf(stderr, "\tReceived %li bytes from %d\n", size, _read_fd);

   for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; 
        cmsg = CMSG_NXTHDR(&msg, cmsg))
     {
        if (cmsg->cmsg_level != SOL_SOCKET)
          continue;

        switch (cmsg->cmsg_type)
          {
           case SCM_RIGHTS:
             fd = *((int *)CMSG_DATA(cmsg));
             switch (dmsg.opcode)
               {
                case ECORE_DRM_OP_DEVICE_OPEN:
                  fprintf(stderr, "Open Device: %s\n", (char *)data);
                  _open_device((char *)data);
                  break;
                case ECORE_DRM_OP_DEVICE_CLOSE:
                  fprintf(stderr, "Close Device: %d\n", fd);
                  _close_device(fd);
                case ECORE_DRM_OP_TTY_OPEN:
                  fprintf(stderr, "Open Tty: %s\n", (char *)data);
                  _open_tty((char *)data);
                  break;
                case ECORE_DRM_OP_DEVICE_MASTER_DROP:
                  fprintf(stderr, "Drop Master: %d\n", fd);
                  _drop_master(fd);
                  break;
                case ECORE_DRM_OP_DEVICE_MASTER_SET:
                  fprintf(stderr, "Set Master\n");
                  _set_master(fd);
                  break;
                default:
                  fprintf(stderr, "Unhandled Opcode: %d\n", dmsg.opcode);
                  break;
               }
             break;
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

   setvbuf(stdout, NULL, _IONBF, 0);
   setvbuf(stderr, NULL, _IONBF, 0);

   fprintf(stderr, "Spartacus Is Alive\n");

   _read_fd = _read_fd_get();
   if (_read_fd < 0) return EXIT_FAILURE;

   _write_fd = _write_fd_get();
   if (_write_fd < 0) return EXIT_FAILURE;

   fprintf(stderr, "Creating Epoll\n");
   _epoll_fd = epoll_create(1);

   memset(&ev, 0, sizeof(ev));
   ev.events = EPOLLIN;
   ev.data.fd = _read_fd;

   if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _read_fd, &ev) < 0)
     {
        return EXIT_FAILURE;
     }

   memset(&events, 0, sizeof(events));

   while (1)
     {
        ret = epoll_wait(_epoll_fd, events, sizeof(events) / sizeof(struct epoll_event), -1);
        if (ret < 0)
          {
             fprintf(stderr, "Epoll Failed: %m\n");
             return EXIT_FAILURE;
          }

        for (i = 0; i < ret; i++)
          {
             fprintf(stderr, "Epoll Event on: %d\n", events[i].data.fd);
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
