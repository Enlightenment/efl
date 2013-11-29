#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signalfd.h>
#include <poll.h>
#include <Ecore_Drm.h>

static int _drm_read_fd = -1;
static int _drm_write_fd = -1;

static int 
_read_fd_get(void)
{
   char *ev, *end;
   int fd = -1, flags = -1;

   if (!(ev = getenv("ECORE_DRM_LAUNCHER_SOCKET")))
     return -1;

   fd = strtol(ev, &end, 0);
   if (*end != '\0') return -1;

   flags = fcntl(fd, F_GETFD);
   if (flags == -1) return -1;

   fcntl(fd, F_SETFD, flags | FD_CLOEXEC);

   return fd;
}

static int 
_read_msg(void)
{
   int ret = -1;
   char buff[BUFSIZ]; // defined in stdio.h
   char ctrl[CMSG_SPACE(sizeof(int))];
   struct msghdr msg;
   struct iovec iov;
   struct cmsghdr *cmsg;
   ssize_t size;
   Ecore_Drm_Message *dmsg;

   memset(&msg, 0, sizeof(msg));

   iov.iov_base = buff;
   iov.iov_len = sizeof(buff);

   msg.msg_name = NULL;
   msg.msg_namelen = 0;
   msg.msg_iov = &iov;
   msg.msg_iovlen = 1;
   msg.msg_control = ctrl;
   msg.msg_controllen = sizeof(ctrl);

   do
     {
        size = recvmsg(_drm_read_fd, &msg, 0);
     } while ((size < 0) && (errno == EINTR));

   if (size < 1) 
     {
        fprintf(stderr, "Received Message too small\n");
        return -1;
     }

   fprintf(stderr, "Spartacus Received Message\n");

   cmsg = CMSG_FIRSTHDR(&msg);
   if ((cmsg) && (cmsg->cmsg_len == CMSG_LEN(sizeof(int))))
     {
        if (cmsg->cmsg_level != SOL_SOCKET)
          {
             fprintf(stderr, "\tInvalid cmsg level\n");
             return -1;
          }
        if (cmsg->cmsg_type != SCM_RIGHTS)
          {
             fprintf(stderr, "\tInvalid cmsg type\n");
             return -1;
          }
     }

   dmsg = (void *)buff;
   switch (dmsg->opcode)
     {
      case ECORE_DRM_OP_READ_FD_SET:
        _drm_read_fd = *((int *)CMSG_DATA(cmsg));
        if (_drm_read_fd >= 0) ret = 1;
        fprintf(stderr, "\tEcore_Drm_Operation_Read_Fd: %d\n", _drm_read_fd);
        break;
      case ECORE_DRM_OP_WRITE_FD_SET:
        _drm_write_fd = *((int *)CMSG_DATA(cmsg));
        if (_drm_write_fd >= 0) ret = 1;
        fprintf(stderr, "\tEcore_Drm_Operation_Write_Fd: %d\n", _drm_write_fd);
        break;
      case ECORE_DRM_OP_OPEN_FD:
        fprintf(stderr, "\tEcore_Drm_Operation_Open FD\n");
        ret = 1; // FIXME
        break;
      case ECORE_DRM_OP_CLOSE_FD:
        fprintf(stderr, "\tEcore_Drm_Operation_Close FD\n");
        ret = 1;
        break;
      default:
        break;
     }

   return ret;
}

int 
main(int argc, char **argv)
{
   fprintf(stderr, "Ecore_Drm_Launch Started\n");

   if ((_drm_read_fd = _read_fd_get()) < 0) return EXIT_FAILURE;

   fprintf(stderr, "\tGot Fd: %d\n", _drm_read_fd);

   while (1)
     {
        struct pollfd pfd[1];
        int p = 0, n = 0;

        fprintf(stderr, "\tChecking for Message...\n");

        pfd[0].fd = _drm_read_fd;
        pfd[0].events = (POLLIN | POLLPRI | POLLNVAL);

        if ((n = poll(pfd, 1, -1)) < 0)
          {
             fprintf(stderr, "\tPoll Failed\n");
             break;
          }

        if (pfd[0].revents & POLLIN)
          {
             _read_msg();
          }
        else if (pfd[0].revents & POLLNVAL)
          {
             fprintf(stderr, "\tInvalid FD to Poll\n");
             break;
          }
     }

   fprintf(stderr, "Spartacus Is Dead\n");

   return EXIT_SUCCESS;
}
