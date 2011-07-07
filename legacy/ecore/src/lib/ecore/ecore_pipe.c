#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#ifdef HAVE_ISFINITE
# define ECORE_FINITE(t) isfinite(t)
#else
# ifdef _MSC_VER
#  define ECORE_FINITE(t) _finite(t)
# else
#  define ECORE_FINITE(t) finite(t)
# endif
#endif

#define FIX_HZ 1

#ifdef FIX_HZ
# ifndef _MSC_VER
#  include <sys/param.h>
# endif
# ifndef HZ
#  define HZ 100
# endif
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

/* How of then we should retry to write to the pipe */
#define ECORE_PIPE_WRITE_RETRY 6

/*
 * On Windows, pipe() is implemented with sockets.
 * Contrary to Linux, Windows uses different functions
 * for sockets and fd's: write() is for fd's and send
 * is for sockets. So I need to put some win32 code
 * here. I can't think of a solution where the win32
 * code is in Evil and not here.
 */

#ifdef _WIN32

# include <winsock2.h>

# define pipe_write(fd, buffer, size) send((fd), (char *)(buffer), size, 0)
# define pipe_read(fd, buffer, size)  recv((fd), (char *)(buffer), size, 0)
# define pipe_close(fd)               closesocket(fd)
# define PIPE_FD_INVALID              INVALID_SOCKET
# define PIPE_FD_ERROR                SOCKET_ERROR

#else

# include <unistd.h>
# include <fcntl.h>

# define pipe_write(fd, buffer, size) write((fd), buffer, size)
# define pipe_read(fd, buffer, size)  read((fd), buffer, size)
# define pipe_close(fd)               close(fd)
# define PIPE_FD_INVALID              -1
# define PIPE_FD_ERROR                -1

#endif /* ! _WIN32 */

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
   size_t            already_read;
   void             *passed_data;
   int               message;
   Eina_Bool         delete_me : 1;
};


static Eina_Bool _ecore_pipe_read(void *data, Ecore_Fd_Handler *fd_handler);

/**
 * @addtogroup Ecore_Group Ecore - Main Loop and Job Functions.
 *
 * @{
 */

/**
 * @addtogroup Ecore_Pipe_Group Pipe wrapper
 *
 * These functions wrap the pipe / write / read functions to easily integrate
 * its use into ecore's main loop.
 *
 * The ecore_pipe_add() function creates file descriptors (sockets on
 * Windows) and attach an handle to the ecore main loop. That handle is
 * called when data is read in the pipe. To write data in the pipe,
 * just call ecore_pipe_write(). When you are done, just call
 * ecore_pipe_del().
 *
 * For examples see here:
 * @li @ref tutorial_ecore_pipe_gstreamer_example
 * @li @ref tutorial_ecore_pipe_simple_example
 */


/**
 * Create two file descriptors (sockets on Windows). Add
 * a callback that will be called when the file descriptor that
 * is listened receives data. An event is also put in the event
 * queue when data is received.
 *
 * @param handler The handler called when data is received.
 * @param data    Data to pass to @p handler when it is called.
 * @return        A newly created Ecore_Pipe object if successful.
 *                @c NULL otherwise.
 */
EAPI Ecore_Pipe *
ecore_pipe_add(Ecore_Pipe_Cb handler, const void *data)
{
   Ecore_Pipe *p;
   int         fds[2];

   if (!handler) return NULL;

   p = (Ecore_Pipe *)calloc(1, sizeof(Ecore_Pipe));
   if (!p) return NULL;

   if (pipe(fds))
     {
        free(p);
        return NULL;
     }

   ECORE_MAGIC_SET(p, ECORE_MAGIC_PIPE);
   p->fd_read = fds[0];
   p->fd_write = fds[1];
   p->handler = handler;
   p->data = data;

   fcntl(p->fd_read, F_SETFL, O_NONBLOCK);
   p->fd_handler = ecore_main_fd_handler_add(p->fd_read,
                                          ECORE_FD_READ,
                                          _ecore_pipe_read,
                                          p,
                                          NULL, NULL);
   return p;
}

/**
 * Free an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @param p The Ecore_Pipe object to be freed.
 * @return The pointer to the private data
 */
EAPI void *
ecore_pipe_del(Ecore_Pipe *p)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_del");
        return NULL;
     }
   p->delete_me = EINA_TRUE;
   if (p->handling > 0) return (void *)p->data;
   if (p->fd_handler) ecore_main_fd_handler_del(p->fd_handler);
   if (p->fd_read != PIPE_FD_INVALID) pipe_close(p->fd_read);
   if (p->fd_write != PIPE_FD_INVALID) pipe_close(p->fd_write);
   data = (void *)p->data;
   free(p);
   return data;
}

/**
 * Close the read end of an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @param p The Ecore_Pipe object.
 */
EAPI void
ecore_pipe_read_close(Ecore_Pipe *p)
{
   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_read_close");
        return;
     }
   if (p->fd_handler)
     {
        ecore_main_fd_handler_del(p->fd_handler);
        p->fd_handler = NULL;
     }
   if (p->fd_read != PIPE_FD_INVALID)
     {
        pipe_close(p->fd_read);
        p->fd_read = PIPE_FD_INVALID;
     }
}

/**
 * Stop monitoring if necessary the pipe for reading. See ecore_pipe_thaw()
 * for monitoring it again.
 *
 * @param p The Ecore_Pipe object.
 * @since 1.1
 */
EAPI void
ecore_pipe_freeze(Ecore_Pipe *p)
{
   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_read_freeze");
        return;
     }
   if (p->fd_handler)
     {
        ecore_main_fd_handler_del(p->fd_handler);
        p->fd_handler = NULL;
     }
}

/**
 * Start monitoring again the pipe for reading. See ecore_pipe_freeze() for
 * stopping the monitoring activity. This will not work if
 * ecore_pipe_read_close() was previously called on the same pipe.
 *
 * @param p The Ecore_Pipe object.
 * @since 1.1
 */
EAPI void
ecore_pipe_thaw(Ecore_Pipe *p)
{
   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_read_thaw");
        return;
     }
   if (!p->fd_handler && p->fd_read != PIPE_FD_INVALID)
     {
        p->fd_handler = ecore_main_fd_handler_add(p->fd_read,
                                                  ECORE_FD_READ,
                                                  _ecore_pipe_read,
                                                  p,
                                                  NULL, NULL);
     }
}

/**
 * @brief Wait from another thread on the read side of a pipe.
 *
 * @param p The pipe to watch on.
 * @param message_count The minimal number of message to wait before exiting.
 * @param wait The amount of time in second to wait before exiting.
 * @return the number of message catched during that wait call.
 * @since 1.1
 *
 * Negative value for @p wait means infite wait.
 */
EAPI int
ecore_pipe_wait(Ecore_Pipe *p, int message_count, double wait)
{
   struct timeval tv, *t;
   fd_set rset;
   double end = 0.0;
   double timeout;
   int ret;
   int total = 0;

   if (p->fd_read == PIPE_FD_INVALID)
     return -1;

   FD_ZERO(&rset);
   FD_SET(p->fd_read, &rset);

   if (wait >= 0.0)
     end = ecore_time_get() + wait;
   timeout = wait;

   while (message_count > 0 && (timeout > 0.0 || wait <= 0.0))
     {
        if (wait >= 0.0)
          {
             /* finite() tests for NaN, too big, too small, and infinity.  */
             if ((!ECORE_FINITE(timeout)) || (timeout == 0.0))
               {
                  tv.tv_sec = 0;
                  tv.tv_usec = 0;
               }
             else if (timeout > 0.0)
               {
                  int sec, usec;
#ifdef FIX_HZ
                  timeout += (0.5 / HZ);
                  sec = (int)timeout;
                  usec = (int)((timeout - (double)sec) * 1000000);
#else
                  sec = (int)timeout;
                  usec = (int)((timeout - (double)sec) * 1000000);
#endif
                  tv.tv_sec = sec;
                  tv.tv_usec = usec;
               }
             t = &tv;
          }
        else
          {
             t = NULL;
          }

        ret = main_loop_select(p->fd_read + 1, &rset, NULL, NULL, t);

        if (ret > 0)
          {
             _ecore_pipe_read(p, NULL);
             message_count -= p->message;
             total += p->message;
             p->message = 0;
          }
        else if (ret == 0)
          {
             break;
          }
        else if (errno != EINTR)
          {
             close(p->fd_read);
             p->fd_read = PIPE_FD_INVALID;
             break;
          }

        if (wait >= 0.0)
          timeout = end - ecore_time_get();
     }

   return total;
}

/**
 * Close the write end of an Ecore_Pipe object created with ecore_pipe_add().
 *
 * @param p The Ecore_Pipe object.
 */
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

/**
 * Write on the file descriptor the data passed as parameter.
 *
 * @param p      The Ecore_Pipe object.
 * @param buffer The data to write into the pipe.
 * @param nbytes The size of the @p buffer in bytes
 * @return       Returns EINA_TRUE on a successful write, EINA_FALSE on an error
 */
EAPI Eina_Bool
ecore_pipe_write(Ecore_Pipe *p, const void *buffer, unsigned int nbytes)
{
   ssize_t ret;
   size_t  already_written = 0;
   int     retry = ECORE_PIPE_WRITE_RETRY;

   if (!ECORE_MAGIC_CHECK(p, ECORE_MAGIC_PIPE))
     {
        ECORE_MAGIC_FAIL(p, ECORE_MAGIC_PIPE, "ecore_pipe_write");
        return EINA_FALSE;
     }

   if (p->delete_me) return EINA_FALSE;

   if (p->fd_write == PIPE_FD_INVALID) return EINA_FALSE;

   /* First write the len into the pipe */
   do
     {
        ret = pipe_write(p->fd_write, &nbytes, sizeof(nbytes));
        if (ret == sizeof(nbytes))
          {
             retry = ECORE_PIPE_WRITE_RETRY;
             break;
          }
        else if (ret > 0)
          {
             /* XXX What should we do here? */
             ERR("The length of the data was not written complete"
                 " to the pipe");
             return EINA_FALSE;
          }
        else if (ret == PIPE_FD_ERROR && errno == EPIPE)
          {
             pipe_close(p->fd_write);
             p->fd_write = PIPE_FD_INVALID;
             return EINA_FALSE;
          }
        else if (ret == PIPE_FD_ERROR && errno == EINTR)
          /* try it again */
          ;
        else
          {
             ERR("An unhandled error (ret: %zd errno: %d)"
                 "occurred while writing to the pipe the length",
                 ret, errno);
          }
     }
   while (retry--);

   if (retry != ECORE_PIPE_WRITE_RETRY) return EINA_FALSE;

   /* and now pass the data to the pipe */
   do
     {
        ret = pipe_write(p->fd_write,
                         ((unsigned char *)buffer) + already_written,
                         nbytes - already_written);

        if (ret == (ssize_t)(nbytes - already_written))
          return EINA_TRUE;
        else if (ret >= 0)
          {
             already_written -= ret;
             continue;
          }
        else if (ret == PIPE_FD_ERROR && errno == EPIPE)
          {
             pipe_close(p->fd_write);
             p->fd_write = PIPE_FD_INVALID;
             return EINA_FALSE;
          }
        else if (ret == PIPE_FD_ERROR && errno == EINTR)
          /* try it again */
          ;
        else
          {
             ERR("An unhandled error (ret: %zd errno: %d)"
                 "occurred while writing to the pipe the length",
                 ret, errno);
          }
     }
   while (retry--);

   return EINA_FALSE;
}

/**
 * @}
 */

/**
 * @}
 */

/* Private function */
static void
_ecore_pipe_unhandle(Ecore_Pipe *p)
{
   p->handling--;
   if (p->delete_me)
     {
        ecore_pipe_del(p);
     }
}

static Eina_Bool
_ecore_pipe_read(void *data, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   Ecore_Pipe  *p = (Ecore_Pipe *)data;
   int          i;

   p->handling++;
   for (i = 0; i < 16; i++)
     {
        ssize_t       ret;

        /* if we already have read some data we don't need to read the len
         * but to finish the already started job
         */
        if (p->len == 0)
          {
             /* read the len of the passed data */
             ret = pipe_read(p->fd_read, &p->len, sizeof(p->len));

             /* catch the non error case first */
             /* read amount ok - nothing more to do */
             if (ret == sizeof(p->len))
               ;
             else if (ret > 0)
               {
                  /* we got more data than we asked for - definite error */
                  ERR("Only read %i bytes from the pipe, although"
                      " we need to read %i bytes.", 
                      (int)ret, (int)sizeof(p->len));
                  _ecore_pipe_unhandle(p);
                  return ECORE_CALLBACK_CANCEL;
               }
             else if (ret == 0)
               {
                  /* we got no data even though we had data to read */
                  if (!p->delete_me)
                     p->handler((void *)p->data, NULL, 0);
                  if (p->passed_data) free(p->passed_data);
                  p->passed_data = NULL;
                  p->already_read = 0;
                  p->len = 0;
                  p->message++;
                  pipe_close(p->fd_read);
                  p->fd_read = PIPE_FD_INVALID;
                  p->fd_handler = NULL;
                  _ecore_pipe_unhandle(p);
                  return ECORE_CALLBACK_CANCEL;
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
             else /* ret == PIPE_FD_ERROR is the only other case on Windows */
               {
                  if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                       if (!p->delete_me)
                          p->handler((void *)p->data, NULL, 0);
                       if (p->passed_data) free(p->passed_data);
                       p->passed_data = NULL;
                       p->already_read = 0;
                       p->len = 0;
                       p->message++;
                       pipe_close(p->fd_read);
                       p->fd_read = PIPE_FD_INVALID;
                       p->fd_handler = NULL;
                       _ecore_pipe_unhandle(p);
                       return ECORE_CALLBACK_CANCEL;
                    }
               }
#endif
          }

        /* if somehow we got less than or equal to 0 we got an errnoneous
         * messages so call callback with null and len we got */
        if (p->len <= 0)
          {
             if (!p->delete_me)
                p->handler((void *)p->data, NULL, p->len);
             /* reset all values to 0 */
             if (p->passed_data) free(p->passed_data);
             p->passed_data = NULL;
             p->already_read = 0;
             p->len = 0;
             p->message++;
             _ecore_pipe_unhandle(p);
             return ECORE_CALLBACK_RENEW;
          }

        /* we dont have a buffer to hold the data, so alloc it */
        if (!p->passed_data)
          {
             p->passed_data = malloc(p->len);
             /* alloc failed - error case */
             if (!p->passed_data)
               {
                  if (!p->delete_me)
                     p->handler((void *)p->data, NULL, 0);
                  /* close the pipe */
                  p->already_read = 0;
                  p->len = 0;
                  p->message++;
                  pipe_close(p->fd_read);
                  p->fd_read = PIPE_FD_INVALID;
                  p->fd_handler = NULL;
                  _ecore_pipe_unhandle(p);
                  return ECORE_CALLBACK_CANCEL;
               }
          }

        /* and read the passed data */
        ret = pipe_read(p->fd_read,
                        ((unsigned char *)p->passed_data) + p->already_read,
                        p->len - p->already_read);

        /* catch the non error case first */
        /* if we read enough data to finish the message/buffer */
        if (ret == (ssize_t)(p->len - p->already_read))
          {
             if (!p->delete_me)
                p->handler((void *)p->data, p->passed_data, p->len);
             free(p->passed_data);
             /* reset all values to 0 */
             p->passed_data = NULL;
             p->already_read = 0;
             p->len = 0;
             p->message++;
          }
        else if (ret > 0)
          {
             /* more data left to read */
             p->already_read += ret;
             _ecore_pipe_unhandle(p);
             return ECORE_CALLBACK_RENEW;
          }
        else if (ret == 0)
          {
             /* 0 bytes available when woken up to handle read - error */
             if (!p->delete_me)
                p->handler((void *)p->data, NULL, 0);
             if (p->passed_data) free(p->passed_data);
             p->passed_data = NULL;
             p->already_read = 0;
             p->len = 0;
             p->message++;
             pipe_close(p->fd_read);
             p->fd_read = PIPE_FD_INVALID;
             p->fd_handler = NULL;
             _ecore_pipe_unhandle(p);
             return ECORE_CALLBACK_CANCEL;
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
        else /* ret == PIPE_FD_ERROR is the only other case on Windows */
          {
             if (WSAGetLastError() != WSAEWOULDBLOCK)
               {
                  if (!p->delete_me)
                     p->handler((void *)p->data, NULL, 0);
                  if (p->passed_data) free(p->passed_data);
                  p->passed_data = NULL;
                  p->already_read = 0;
                  p->len = 0;
                  p->message++;
                  pipe_close(p->fd_read);
                  p->fd_read = PIPE_FD_INVALID;
                  p->fd_handler = NULL;
                  _ecore_pipe_unhandle(p);
                  return ECORE_CALLBACK_CANCEL;
               }
             else
               break;
          }
#endif
     }

   _ecore_pipe_unhandle(p);
   return ECORE_CALLBACK_RENEW;
}
