#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#ifdef HAVE_IEEEFP_H
# include <ieeefp.h> /* for Solaris */
#endif

#ifdef HAVE_ISFINITE
# define ECORE_FINITE(t) isfinite(t)
#else
# define ECORE_FINITE(t) finite(t)
#endif

#define FIX_HZ 1

#ifdef FIX_HZ
# include <sys/param.h>
# ifndef HZ
#  define HZ 100
# endif
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

#ifdef HAVE_EXOTIC
# include <Exotic.h>
#endif

/*
 * On Windows, pipe() is implemented with sockets.
 * Contrary to Linux, Windows uses different functions
 * for sockets and fd's: write() is for fd's and send
 * is for sockets. So I need to put some win32 code
 * here. I can't think of a solution where the win32
 * code is in Evil and not here.
 */

#define PIPE_FD_INVALID -1

#ifdef _WIN32
# include <winsock2.h>
# define pipe_write(fd, buffer, size) send((fd), (char *)(buffer), size, 0)
# define pipe_read(fd, buffer, size)  recv((fd), (char *)(buffer), size, 0)
# define pipe_close(fd)               closesocket(fd)
# define PIPE_FD_ERROR   SOCKET_ERROR
#else
# ifdef HAVE_SYS_EPOLL_H
#  include <sys/epoll.h>
# endif /* HAVE_SYS_EPOLL_H */
# ifdef HAVE_SYS_TIMERFD_H
#  include <sys/timerfd.h>
# endif
# include <unistd.h>
# include <fcntl.h>
# define pipe_write(fd, buffer, size) write((fd), buffer, size)
# define pipe_read(fd, buffer, size)  read((fd), buffer, size)
# define pipe_close(fd)               close(fd)
# define PIPE_FD_ERROR   -1
#endif /* ! _WIN32 */

#include "Ecore.h"
#include "ecore_private.h"

// How of then we should retry to write to the pipe
#define ECORE_PIPE_WRITE_RETRY 6

struct _Ecore_Pipe
{
   ECORE_MAGIC;
   int               fd_read;
   int               fd_write;
   Ecore_Fd_Handler *fd_handler;
   const void       *data;
   Ecore_Pipe_Cb     handler;
   unsigned int      len;
   int               handling;
   unsigned int      already_read;
   void             *passed_data;
   int               message;
#ifndef _WIN32
   int               pollfd;
   int               timerfd;
#endif
   Eina_Bool         delete_me : 1;
};
GENERIC_ALLOC_SIZE_DECLARE(Ecore_Pipe);

static Eina_Bool _ecore_pipe_read(void             *data,
                                  Ecore_Fd_Handler *fd_handler);

EAPI Ecore_Pipe *
ecore_pipe_add(Ecore_Pipe_Cb handler,
               const void   *data)
{
   return _ecore_pipe_add(handler, data);
}

EAPI void *
ecore_pipe_del(Ecore_Pipe *p)
{
   if (!p) return NULL;
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   return _ecore_pipe_del(p);
}

EAPI void
ecore_pipe_read_close(Ecore_Pipe *p)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_read_close");
        return;
     }
   if (p->fd_handler)
     {
        _ecore_main_fd_handler_del(ML_OBJ, ML_DAT, p->fd_handler);
        p->fd_handler = NULL;
     }
   if (p->fd_read != PIPE_FD_INVALID)
     {
        pipe_close(p->fd_read);
        p->fd_read = PIPE_FD_INVALID;
     }
}

EAPI int
ecore_pipe_read_fd(Ecore_Pipe *p)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(PIPE_FD_INVALID);
   if (!p) return PIPE_FD_INVALID;
   return p->fd_read;
}

EAPI void
ecore_pipe_freeze(Ecore_Pipe *p)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_read_freeze");
        return;
     }
   if (p->fd_handler)
     {
        _ecore_main_fd_handler_del(ML_OBJ, ML_DAT, p->fd_handler);
        p->fd_handler = NULL;
     }
}

EAPI void
ecore_pipe_thaw(Ecore_Pipe *p)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_read_thaw");
        return;
     }
   if ((!p->fd_handler) && (p->fd_read != PIPE_FD_INVALID))
     p->fd_handler = ecore_main_fd_handler_add(p->fd_read, ECORE_FD_READ,
                                               _ecore_pipe_read, p,
                                               NULL, NULL);
}

EAPI int
ecore_pipe_wait(Ecore_Pipe *p,
                int         message_count,
                double      wait)
{
   return _ecore_pipe_wait(p, message_count, wait);
}

EAPI void
ecore_pipe_write_close(Ecore_Pipe *p)
{
   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_write_close");
        return;
     }
   if (p->fd_write != PIPE_FD_INVALID)
     {
        pipe_close(p->fd_write);
        p->fd_write = PIPE_FD_INVALID;
     }
}

EAPI int
ecore_pipe_write_fd(Ecore_Pipe *p)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(PIPE_FD_INVALID);
   if (!p) return PIPE_FD_INVALID;
   return p->fd_write;
}

EAPI Eina_Bool
ecore_pipe_write(Ecore_Pipe  *p,
                 const void  *buffer,
                 unsigned int nbytes)
{
   ssize_t ret;
   size_t already_written = 0;
   int retry = ECORE_PIPE_WRITE_RETRY;
   Eina_Bool ok = EINA_FALSE;

   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_write");
        goto out;
     }

   if (p->delete_me) goto out;

   if (p->fd_write == PIPE_FD_INVALID) goto out;

   do // First write the len into the pipe
     {
        ret = pipe_write(p->fd_write, &nbytes, sizeof(nbytes));
        if (ret == sizeof(nbytes))
          {
             retry = ECORE_PIPE_WRITE_RETRY;
             break;
          }
        else if (ret > 0)
          {
             // XXX What should we do here?
             ERR("The length of the data was not written complete"
                 " to the pipe");
             goto out;
          }
        else if ((ret == PIPE_FD_ERROR) && (errno == EPIPE))
          {
             pipe_close(p->fd_write);
             p->fd_write = PIPE_FD_INVALID;
             goto out;
          }
        else if ((ret == PIPE_FD_ERROR) && (errno == EINTR))
          // try it again
          ;
        else
          {
             ERR("An unhandled error (ret: %zd errno: %d)"
                 "occurred while writing to the pipe the length",
                 ret, errno);
          }
     }
   while (retry--);

   if (retry != ECORE_PIPE_WRITE_RETRY) goto out;

   do // and now pass the data to the pipe
     {
        ret = pipe_write(p->fd_write,
                         ((unsigned char *)buffer) + already_written,
                         nbytes - already_written);

        if (ret == (ssize_t)(nbytes - already_written))
          {
             ok = EINA_TRUE;
             goto out;
          }
        else if (ret >= 0)
          {
             already_written -= ret;
             continue;
          }
        else if ((ret == PIPE_FD_ERROR) && (errno == EPIPE))
          {
             pipe_close(p->fd_write);
             p->fd_write = PIPE_FD_INVALID;
             goto out;
          }
        else if ((ret == PIPE_FD_ERROR) && (errno == EINTR))
          // try it again
          ;
        else
          {
             ERR("An unhandled error (ret: %zd errno: %d)"
                 "occurred while writing to the pipe the length",
                 ret, errno);
          }
     }
   while (retry--);

out:
   return ok;
}

EAPI Ecore_Pipe *
ecore_pipe_full_add(Ecore_Pipe_Cb handler,
                    const void   *data,
                    int fd_read,
                    int fd_write,
                    Eina_Bool read_survive_fork,
                    Eina_Bool write_survive_fork)
{
   Ecore_Pipe *p = NULL;
   int fds[2];

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);
   if (!handler) return NULL;

   p = ecore_pipe_calloc(1);
   if (!p) return NULL;

   if ((fd_read == -1) && (fd_write == -1))
     {
        if (pipe(fds))
          {
             ecore_pipe_mp_free(p);
             return NULL;
          }
        fd_read = fds[0];
        fd_write = fds[1];
     }
   else
     {
        fd_read = (fd_read == -1) ? PIPE_FD_INVALID : fd_read;
        fd_write = (fd_write == -1) ? PIPE_FD_INVALID : fd_write;
     }

   ECORE_MAGIC_SET(p, ECORE_MAGIC_PIPE);
   p->fd_read = fd_read;
   p->fd_write = fd_write;
   p->handler = handler;
   p->data = data;

   if (!read_survive_fork) eina_file_close_on_exec(fd_read, EINA_TRUE);
   if (!write_survive_fork) eina_file_close_on_exec(fd_write, EINA_TRUE);

#if defined(HAVE_SYS_EPOLL_H) && defined(HAVE_SYS_TIMERFD_H)
   struct epoll_event pollev = { 0 };
   p->pollfd = epoll_create(1);
   p->timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
   eina_file_close_on_exec(p->pollfd, EINA_TRUE);

   pollev.data.ptr = &(p->fd_read);
   pollev.events = EPOLLIN;
   epoll_ctl(p->pollfd, EPOLL_CTL_ADD, p->fd_read, &pollev);

   pollev.data.ptr = &(p->timerfd);
   pollev.events = EPOLLIN;
   epoll_ctl(p->pollfd, EPOLL_CTL_ADD, p->timerfd, &pollev);
#endif

   if (fcntl(p->fd_read, F_SETFL, O_NONBLOCK) < 0)
     ERR("can't set pipe to NONBLOCK");
   p->fd_handler = ecore_main_fd_handler_add(p->fd_read, ECORE_FD_READ,
                                             _ecore_pipe_read, p, NULL, NULL);
   return p;
}

// Private functions
Ecore_Pipe *
_ecore_pipe_add(Ecore_Pipe_Cb handler,
                const void   *data)
{
   return ecore_pipe_full_add(handler, data, -1, -1, EINA_FALSE, EINA_FALSE);
}

void *
_ecore_pipe_del(Ecore_Pipe *p)
{
   void *data = NULL;

   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_del");
        return NULL;
     }
#if defined(HAVE_SYS_EPOLL_H) && defined(HAVE_SYS_TIMERFD_H)
   epoll_ctl(p->pollfd, EPOLL_CTL_DEL, p->fd_read, NULL);
   epoll_ctl(p->pollfd, EPOLL_CTL_DEL, p->timerfd, NULL);
   if (p->timerfd >= 0) close(p->timerfd);
   if (p->pollfd >= 0) close(p->pollfd);
   p->timerfd = PIPE_FD_INVALID;
   p->pollfd = PIPE_FD_INVALID;
#endif
   p->delete_me = EINA_TRUE;
   if (p->handling > 0) return (void *)p->data;
   if (p->fd_handler) _ecore_main_fd_handler_del(ML_OBJ, ML_DAT,
                                                 p->fd_handler);
   if (p->fd_read != PIPE_FD_INVALID) pipe_close(p->fd_read);
   if (p->fd_write != PIPE_FD_INVALID) pipe_close(p->fd_write);
   p->fd_handler = NULL;
   p->fd_read = PIPE_FD_INVALID;
   p->fd_write = PIPE_FD_INVALID;
   data = (void *)p->data;
   ecore_pipe_mp_free(p);
   return data;
}

static void
_ecore_pipe_unhandle(Ecore_Pipe *p)
{
   p->handling--;
   if (p->delete_me) _ecore_pipe_del(p);
}

#if ! defined(HAVE_SYS_EPOLL_H) || ! defined(HAVE_SYS_TIMERFD_H)
int
_ecore_pipe_wait(Ecore_Pipe *p,
                 int         message_count,
                 double      wait)
{
   struct timeval tv, *t;
   fd_set rset, wset, exset;
   double end = 0.0;
   double timeout;
   int ret;
   int total = 0;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(-1);
   if (p->fd_read == PIPE_FD_INVALID) return -1;

   FD_ZERO(&rset);
   FD_ZERO(&wset);
   FD_ZERO(&exset);
   FD_SET(p->fd_read, &rset);

   if (wait >= 0.0) end = ecore_time_get() + wait;
   timeout = wait;

   while ((message_count > 0) && ((timeout > 0.0) || (wait <= 0.0)))
     {
        if (wait >= 0.0)
          {
             // finite() tests for NaN, too big, too small, and infinity.
             if ((!ECORE_FINITE(timeout)) || (EINA_DBL_EQ(timeout, 0.0)))
               {
                  tv.tv_sec = 0;
                  tv.tv_usec = 0;
               }
             else if (timeout > 0.0)
               {
                  int sec, usec;
#ifdef FIX_HZ
                  timeout += (0.5 / HZ);
#endif
                  sec = (int)timeout;
                  usec = (int)((timeout - (double)sec) * 1000000);
                  tv.tv_sec = sec;
                  tv.tv_usec = usec;
               }
             t = &tv;
          }
        else t = NULL;

        ret = main_loop_select(p->fd_read + 1, &rset, &wset, &exset, t);

        if (ret > 0)
          {
             p->handling++;
             _ecore_pipe_read(p, NULL);
             message_count -= p->message;
             total += p->message;
             p->message = 0;
             _ecore_pipe_unhandle(p);
          }
        else if (ret == 0) break;
        else if (errno != EINTR)
          {
             if (p->fd_read != PIPE_FD_INVALID)
               {
                  close(p->fd_read);
                  p->fd_read = PIPE_FD_INVALID;
               }
             break;
          }

        if (wait >= 0.0) timeout = end - ecore_time_get();
     }

   return total;
}

#else
int
_ecore_pipe_wait(Ecore_Pipe *p,
                 int         message_count,
                 double      wait)
{
   int64_t timerfdbuf;
   struct epoll_event pollincoming[2];
   double timeout;
   int ret = 0;
   int total = 0;
   int time_exit = -1;
   Eina_Bool fd_read_found;
   Eina_Bool fd_timer_found;
   struct itimerspec tspec_new;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(-1);
   if (p->fd_read == PIPE_FD_INVALID) return -1;

   timeout = wait;
   int sec, usec;
   if (wait >= 0.0)
     {
        if ((!ECORE_FINITE(timeout)) || (EINA_DBL_EQ(timeout, 0.0)))
          {
             tspec_new.it_value.tv_sec = 0;
             tspec_new.it_value.tv_nsec = 0;
             tspec_new.it_interval.tv_sec = 0;
             tspec_new.it_interval.tv_nsec = 0;
             time_exit = 0;
          }
        else
          {
#ifdef FIX_HZ
             timeout += (0.5 / HZ);
#endif
             sec = (int)timeout;
             usec = (int)((timeout - (double)sec) * 1000000000);
             tspec_new.it_value.tv_sec = sec;
             tspec_new.it_value.tv_nsec = (int)(usec) % 1000000000;
             tspec_new.it_interval.tv_sec = 0;
             tspec_new.it_interval.tv_nsec = 0;
             timerfd_settime(p->timerfd, 0, &tspec_new, NULL);
          }
     }

   while ((p->pollfd != PIPE_FD_INVALID) && (ret = epoll_wait(p->pollfd, pollincoming, 2, time_exit)) > 0)
     {
        fd_read_found  = EINA_FALSE;
        fd_timer_found = EINA_FALSE;

        for (int i = 0; i < ret;i++)
          {
             if ((&p->fd_read == pollincoming[i].data.ptr))
               fd_read_found  = EINA_TRUE;
             if ((&p->timerfd == pollincoming[i].data.ptr))
               fd_timer_found = EINA_TRUE;
          }

        p->handling++;
        if (fd_read_found)
          {
             _ecore_pipe_read(p, NULL);
             message_count -= p->message;
             total += p->message;
             p->message = 0;
             if (message_count <= 0)
               {
                  _ecore_pipe_unhandle(p);
                  break;
               }
          }

        if ((fd_timer_found) && (p->timerfd != PIPE_FD_INVALID))
          {
             if (pipe_read(p->timerfd, &timerfdbuf, sizeof(timerfdbuf)) <
                 (int)sizeof(int64_t))
               WRN("Could not read timerfd data");
             _ecore_pipe_unhandle(p);
             break;
          }
        _ecore_pipe_unhandle(p);
     }
   if (ret < 0)
     {
        if (errno != EBADF)
          WRN("epoll file descriptor is not a valid");
        else if (errno != EINVAL)
          WRN("epoll file descriptor is not an epoll file descriptor, or maxevents is less than or equal to zero.");
        else if (errno != EFAULT)
          WRN("The memory area pointed to by epoll_event is not accessible with write permissions.");
        else if (errno != EINTR)
          WRN("The call was interrupted by a signal handler before any of the requested epoll_event "
              "occurred or the timeout expired; see signal(7).");
     }
   return total;
}

#endif
static void
_ecore_pipe_handler_call(Ecore_Pipe *p,
                         unsigned char *buf,
                         size_t len)
{
   // on windows we seem to get a pipe wake with no data. don't pass on
   // zero data as there is nothing useful to do with it... and it causes
   // segfaults
   if ((!buf) || (!len)) return;

   void *data = (void*) p->data;

   // clear all values of pipe first.
   p->passed_data = NULL;
   p->already_read = 0;
   p->len = 0;
   p->message++;

   if (!p->delete_me) p->handler(data, buf, len);

   // free p->passed_data
   free(buf);
}

static Eina_Bool
_ecore_pipe_read(void             *data,
                 Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Ecore_Pipe *p = (Ecore_Pipe *)data;
   int i;

   p->handling++;
   for (i = 0; i < 16; i++)
     {
        ssize_t ret;

        // if we already have read some data we don't need to read the len
        // but to finish the already started job
        if (p->len == 0)
          {
             // read the len of the passed data
             ret = pipe_read(p->fd_read, &p->len, sizeof(p->len));

             // catch the non error case first
             // read amount ok - nothing more to do
             if (ret == sizeof(p->len))
               ;
             else if (ret > 0)
               {
                  // we got more data than we asked for - definite error
                  ERR("Only read %i bytes from the pipe, although"
                      " we need to read %i bytes.",
                      (int)ret, (int)sizeof(p->len));
                  _ecore_pipe_unhandle(p);
                  return ECORE_CALLBACK_CANCEL;
               }
             else if (ret == 0)
               {
                  // we got no data
                  if (i == 0)
                    {
                       // no data on first try through means an error
                       _ecore_pipe_handler_call(p, NULL, 0);
                       pipe_close(p->fd_read);
                       p->fd_read = PIPE_FD_INVALID;
                       p->fd_handler = NULL;
                       _ecore_pipe_unhandle(p);
                       return ECORE_CALLBACK_CANCEL;
                    }
                  else
                    {
                       // no data after first loop try is ok
                       _ecore_pipe_unhandle(p);
                       return ECORE_CALLBACK_RENEW;
                    }
               }
#ifndef _WIN32
             else if ((ret == PIPE_FD_ERROR) &&
                      ((errno == EINTR) || (errno == EAGAIN)))
                {
                   _ecore_pipe_unhandle(p);
                   return ECORE_CALLBACK_RENEW;
                }
             else
               {
                  ERR("An unhandled error (ret: %i errno: %i [%s])"
                      "occurred while reading from the pipe the length",
                      (int)ret, errno, strerror(errno));
                  _ecore_pipe_unhandle(p);
                  return ECORE_CALLBACK_RENEW;
               }
#else
             else // ret == PIPE_FD_ERROR is the only other case on Windows
                {
                   if (WSAGetLastError() != WSAEWOULDBLOCK)
                     {
                        _ecore_pipe_handler_call(p, NULL, 0);
                        pipe_close(p->fd_read);
                        p->fd_read = PIPE_FD_INVALID;
                        p->fd_handler = NULL;
                        _ecore_pipe_unhandle(p);
                        return ECORE_CALLBACK_CANCEL;
                     }
                }
#endif
          }
        // if somehow we got less than or equal to 0 we got an errnoneous
        // messages so call callback with null and len we got. this case should
        // never happen
        if (p->len == 0)
          {
             _ecore_pipe_handler_call(p, NULL, 0);
             _ecore_pipe_unhandle(p);
             return ECORE_CALLBACK_RENEW;
          }

        // we dont have a buffer to hold the data, so alloc it
        if (!p->passed_data)
          {
             p->passed_data = malloc(p->len);
             // alloc failed - error case
             if (!p->passed_data)
               {
                  _ecore_pipe_handler_call(p, NULL, 0);
                  // close the pipe
                  pipe_close(p->fd_read);
                  p->fd_read = PIPE_FD_INVALID;
                  p->fd_handler = NULL;
                  _ecore_pipe_unhandle(p);
                  return ECORE_CALLBACK_CANCEL;
               }
          }

        // and read the passed data
        ret = pipe_read(p->fd_read,
                        ((unsigned char *)p->passed_data) + p->already_read,
                        p->len - p->already_read);

        // catch the non error case first
        // if we read enough data to finish the message/buffer
        if (ret == (ssize_t)(p->len - p->already_read))
          _ecore_pipe_handler_call(p, p->passed_data, p->len);
        else if (ret > 0)
          {
             // more data left to read
             p->already_read += (unsigned int)ret;
             _ecore_pipe_unhandle(p);
             return ECORE_CALLBACK_RENEW;
          }
        else if (ret == 0)
          {
             // 0 bytes to read - could be more to read next select wake up
             _ecore_pipe_unhandle(p);
             return ECORE_CALLBACK_RENEW;
          }
#ifndef _WIN32
        else if ((ret == PIPE_FD_ERROR) &&
                 ((errno == EINTR) || (errno == EAGAIN)))
          {
             _ecore_pipe_unhandle(p);
             return ECORE_CALLBACK_RENEW;
          }
        else
          {
             ERR("An unhandled error (ret: %zd errno: %d)"
                 "occurred while reading from the pipe the data",
                 ret, errno);
             _ecore_pipe_unhandle(p);
             return ECORE_CALLBACK_RENEW;
          }
#else
        else // ret == PIPE_FD_ERROR is the only other case on Windows
          {
             if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
                  _ecore_pipe_handler_call(p, NULL, 0);
                  pipe_close(p->fd_read);
                  p->fd_read = PIPE_FD_INVALID;
                  p->fd_handler = NULL;
                  _ecore_pipe_unhandle(p);
                  return ECORE_CALLBACK_CANCEL;
               }
             else break;
          }
#endif
     }

   _ecore_pipe_unhandle(p);
   return ECORE_CALLBACK_RENEW;
}

