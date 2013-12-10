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
#include <signal.h>
#include <poll.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/major.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

/* #include <xf86drm.h> */
/* #include <xf86drmMode.h> */
/* #include <drm_fourcc.h> */

#include <Eina.h>
#include <Ecore_Drm.h>

#define IOVSET(_iov, _addr, _len) \
   (_iov)->iov_base = (void *)(_addr); \
   (_iov)->iov_len = (_len);

static int _drm_read_fd = -1;
static int _drm_write_fd = -1;

static int 
_send_msg(int opcode, int fd, void *data, size_t bytes)
{
   Ecore_Drm_Message dmsg;
   struct iovec iov[2];
   char ctrl[CMSG_SPACE(sizeof(int))];
   struct msghdr msg;
   struct cmsghdr *cmsg;
   ssize_t size;

   IOVSET(iov + 0, &dmsg, sizeof(dmsg));
   IOVSET(iov + 1, data, bytes);

   dmsg.opcode = opcode;
   dmsg.size = bytes;

   memset(&msg, 0, sizeof(struct msghdr));
   memset(ctrl, 0, CMSG_SPACE(sizeof(int)));

   msg.msg_name = NULL;
   msg.msg_namelen = 0;
   msg.msg_iov = iov;
   msg.msg_iovlen = 2;
   msg.msg_controllen = CMSG_SPACE(sizeof(int));
   msg.msg_control = ctrl;

   cmsg = CMSG_FIRSTHDR(&msg);
   cmsg->cmsg_level = SOL_SOCKET;
   cmsg->cmsg_type = SCM_RIGHTS;
   cmsg->cmsg_len = CMSG_LEN(sizeof(int));

   *((int *)CMSG_DATA(cmsg)) = fd;

   errno = 0;
   size = sendmsg(_drm_write_fd, &msg, 0);
   if (errno != 0)
     {
        fprintf(stderr, "Failed to send message: %d\n", errno);
        fprintf(stderr, "\t%s\n", strerror(errno));
     }

   return size;
}

static int 
_open_device(const char *file)
{
   int fd = -1;
   int ret = ECORE_DRM_OP_SUCCESS;

   fprintf(stderr, "Open Device: %s\n", file);

   if ((fd = open(file, O_RDWR)) < 0)
     {
        fprintf(stderr, "\tFailed to open: %m\n");
        ret = ECORE_DRM_OP_FAILURE;
     }

   _send_msg(ECORE_DRM_OP_DEVICE_OPEN, fd, &ret, sizeof(int));

   return ret;
}

static int 
_open_tty(const char *file)
{
   int fd = -1, mode = 0;
   int ret = ECORE_DRM_OP_SUCCESS;
   struct stat st;
   struct vt_mode vtmode;

   fprintf(stderr, "Open Tty: %s\n", file);

   if (!strncmp(file, "/dev/tty0", 9))
     fd = dup(0);
   else if ((fd = open(file, (O_RDWR | O_NOCTTY))) < 0)
     {
        fprintf(stderr, "\tFailed to open: %m\n");
        ret = ECORE_DRM_OP_FAILURE;
        goto mode_err;
     }

   if ((fstat(fd, &st) < 0) || 
       (major(st.st_rdev) != TTY_MAJOR) || (minor(st.st_rdev) == 0))
     {
        fprintf(stderr, "\t%s not a tty\n", file);
        goto mode_err;
     }

   /* try to get the current mode of this vt */
   if (ioctl(fd, KDGETMODE, &mode))
     {
        fprintf(stderr, "\tCould not get vt mode: %m\n");
        goto mode_err;
     }

   /* if we are in text mode, switch to graphics mode */
   if (mode == KD_TEXT)
     {
        /* try to set graphics mode */
        if (ioctl(fd, KDSETMODE, KD_GRAPHICS) < 0)
          {
             fprintf(stderr, "\tCould not set graphics mode: %m\n");
             goto mode_err;
          }
     }

   /* setup vt mode signals */
   vtmode.mode = VT_PROCESS;
   vtmode.waitv = 0;
   vtmode.relsig = SIGUSR1;
   vtmode.acqsig = SIGUSR2;

   /* try to set mode */
   if (ioctl(fd, VT_SETMODE, &vtmode) < 0)
     {
        fprintf(stderr, "\tCould not set mode signals: %m\n");
        /* reset to text mode */
        if (mode == KD_TEXT) ioctl(fd, KDSETMODE, KD_TEXT);
        goto mode_err;
     }

   fprintf(stderr, "\tOpened Tty: %d\n", fd);

   _send_msg(ECORE_DRM_OP_TTY_OPEN, fd, &ret, sizeof(int));

   return ret;

mode_err:
   if (fd > 0) close(fd);
   fd = -1;
   ret = ECORE_DRM_OP_FAILURE;
   _send_msg(ECORE_DRM_OP_TTY_OPEN, fd, &ret, sizeof(int));

   return ret;
}

static int 
_close_fd(int opcode, int fd)
{
   int ret = ECORE_DRM_OP_SUCCESS;

   fprintf(stderr, "Close FD: %d\n", fd);

   if (fd >= 0) close(fd);
   fd = -1;

   _send_msg(opcode, fd, &ret, sizeof(int));

   return ret;
}

static int 
_read_fd_get(void)
{
   char *ev, *end;
   int fd = -1, flags = -1;

   if (!(ev = getenv("ECORE_DRM_LAUNCHER_READ_SOCKET")))
     return -1;

   fd = strtol(ev, &end, 0);
   if (*end != '\0') return -1;

   flags = fcntl(fd, F_GETFD);
   if (flags == -1) return -1;

   /* fcntl(fd, F_SETFD, flags | FD_CLOEXEC); */

   return fd;
}

static int 
_write_fd_get(void)
{
   char *ev, *end;
   int fd = -1, flags = -1;

   if (!(ev = getenv("ECORE_DRM_LAUNCHER_WRITE_SOCKET")))
     return -1;

   fd = strtol(ev, &end, 0);
   if (*end != '\0') return -1;

   flags = fcntl(fd, F_GETFD);
   if (flags == -1) return -1;

   /* fcntl(fd, F_SETFD, flags | FD_CLOEXEC); */

   return fd;
}

static int 
_read_msg(void)
{
   int ret = -1;
   Ecore_Drm_Message dmsg;
   char data[BUFSIZ];
   struct iovec iov[2];
   char ctrl[CMSG_SPACE(sizeof(int))];
   struct msghdr msg;
   struct cmsghdr *cmsg;
   ssize_t size;

   IOVSET(iov + 0, &dmsg, sizeof(dmsg));
   IOVSET(iov + 1, &data, sizeof(data));

   memset(&msg, 0, sizeof(msg));

   msg.msg_name = NULL;
   msg.msg_namelen = 0;
   msg.msg_iov = iov;
   msg.msg_iovlen = 2;
   msg.msg_controllen = CMSG_SPACE(sizeof(int));
   msg.msg_control = ctrl;

   errno = 0;
   size = recvmsg(_drm_read_fd, &msg, 0);//MSG_CMSG_CLOEXEC);

   if (errno != 0)
     {
        fprintf(stderr, "Recvd %li\n", size);
        fprintf(stderr, "Recv Err: %d\n", errno);
        fprintf(stderr, "Recv Err: %s\n", strerror(errno));
     }

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

   switch (dmsg.opcode)
     {
      case ECORE_DRM_OP_READ_FD_SET:
        fprintf(stderr, "Ecore_Drm_Operation_Read_FD_Set\n");
        _drm_read_fd = *((int *)CMSG_DATA(cmsg));
        if (_drm_read_fd >= 0) ret = 1;
        break;
      case ECORE_DRM_OP_WRITE_FD_SET:
        fprintf(stderr, "Ecore_Drm_Operation_Write_FD_Set\n");
        _drm_write_fd = *((int *)CMSG_DATA(cmsg));
        if (_drm_write_fd >= 0) ret = 1;
        break;
      case ECORE_DRM_OP_DEVICE_OPEN:
        fprintf(stderr, "Ecore_Drm_Operation_Device_Open: %s\n", (char *)data);
        ret = _open_device((char *)data);
        break;
      case ECORE_DRM_OP_DEVICE_CLOSE:
        fprintf(stderr, "Ecore_Drm_Operation_Device_Close\n");
        ret = _close_fd(ECORE_DRM_OP_DEVICE_CLOSE, *((int *)data));
        break;
      case ECORE_DRM_OP_TTY_OPEN:
        fprintf(stderr, "Ecore_Drm_Operation_Tty_Open: %s\n", (char *)data);
        ret = _open_tty((char *)data);
        break;
      case ECORE_DRM_OP_TTY_CLOSE:
        fprintf(stderr, "Ecore_Drm_Operation_Tty_Close\n");
        ret = _close_fd(ECORE_DRM_OP_TTY_CLOSE, *((int *)data));
        break;
      default:
        fprintf(stderr, "Uknown operation: %d\n", dmsg.opcode);
        break;
     }

   return ret;
}

int 
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   fprintf(stderr, "Ecore_Drm_Launch Started\n");

   if ((_drm_read_fd = _read_fd_get()) < 0)
     {
        fprintf(stderr, "\tCould not Get FD\n");
        return EXIT_FAILURE;
     }

   fprintf(stderr, "\tGot Fd: %d\n", _drm_read_fd);

   /* if ((_drm_write_fd = _write_fd_get()) < 0) */
   /*   { */
   /*      fprintf(stderr, "\tCould not Get FD\n"); */
   /*      return EXIT_FAILURE; */
   /*   } */

   /* fprintf(stderr, "\tGot Fd: %d\n", _drm_write_fd); */

   while (1)
     {
        struct pollfd pfd[1];
        int n = 0;

        fprintf(stderr, "Checking for Messages on FD %d...\n", _drm_read_fd);

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
