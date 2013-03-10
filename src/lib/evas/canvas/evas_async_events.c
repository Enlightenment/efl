#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef _MSC_VER
# include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>

#include "evas_common.h"
#include "evas_private.h"

typedef struct _Evas_Event_Async	Evas_Event_Async;

struct _Evas_Event_Async
{
   const void		    *target;
   void			    *event_info;
   Evas_Async_Events_Put_Cb  func;
   Evas_Callback_Type	     type;
};

static int _fd_write = -1;
static int _fd_read = -1;
static pid_t _fd_pid = 0;

static Eina_Lock async_lock;
static Eina_Inarray async_queue;
static Evas_Event_Async *async_queue_cache = NULL;
static unsigned int async_queue_cache_max = 0;

static int _init_evas_event = 0;

Eina_Bool
_evas_fd_close_on_exec(int fd)
{
#ifdef HAVE_FCNTL
   int flags;

   flags = fcntl(fd, F_GETFD);
   if (flags == -1)
     return EINA_FALSE;

   flags |= FD_CLOEXEC;
   if (fcntl(fd, F_SETFD, flags) == -1)
     return EINA_FALSE;
   return EINA_TRUE;
#else
   (void) fd;
   return EINA_FALSE;
#endif
}

int
evas_async_events_init(void)
{
   int filedes[2];

   _init_evas_event++;
   if (_init_evas_event > 1) return _init_evas_event;

   _fd_pid = getpid();

   if (pipe(filedes) == -1)
     {
	_init_evas_event = 0;
	return 0;
     }

   _evas_fd_close_on_exec(filedes[0]);
   _evas_fd_close_on_exec(filedes[1]);

   _fd_read = filedes[0];
   _fd_write = filedes[1];

#ifdef HAVE_FCNTL
   fcntl(_fd_read, F_SETFL, O_NONBLOCK);
#endif

   eina_lock_new(&async_lock);
   eina_inarray_step_set(&async_queue, sizeof (Eina_Inarray), sizeof (Evas_Event_Async), 16);

   return _init_evas_event;
}

int
evas_async_events_shutdown(void)
{
   _init_evas_event--;
   if (_init_evas_event > 0) return _init_evas_event;

   eina_lock_free(&async_lock);
   eina_inarray_flush(&async_queue);
   free(async_queue_cache);

   close(_fd_read);
   close(_fd_write);
   _fd_read = -1;
   _fd_write = -1;

   return _init_evas_event;
}

static void
_evas_async_events_fork_handle(void)
{
   int i, count = _init_evas_event;

   if (getpid() == _fd_pid) return;
   for (i = 0; i < count; i++) evas_async_events_shutdown();
   for (i = 0; i < count; i++) evas_async_events_init();
}

EAPI int
evas_async_events_fd_get(void)
{
   _evas_async_events_fork_handle();
   return _fd_read;
}

static int
_evas_async_events_process_single(void)
{
   int ret, wakeup;

   ret = read(_fd_read, &wakeup, sizeof(int));
   if (ret < 0)
     {
        switch (errno)
          {
           case EBADF:
           case EINVAL:
           case EIO:
           case EISDIR:
              _fd_read = -1;
          }

        return ret;
     }

   if (ret == sizeof(int))
     {
        Evas_Event_Async *ev;
        unsigned int len, max;
        int nr;

        eina_lock_take(&async_lock);

        ev = async_queue.members;
        async_queue.members = async_queue_cache;
        async_queue_cache = ev;

        max = async_queue.max;
        async_queue.max = async_queue_cache_max;
        async_queue_cache_max = max;

        len = async_queue.len;
        async_queue.len = 0;

        eina_lock_release(&async_lock);

        DBG("Evas async events queue length: %u", len);
        nr = len;

        while (len > 0)
          {
             if (ev->func) ev->func((void *)ev->target, ev->type, ev->event_info);
             ev++;
             len--;
          }

        return nr;
     }

   return 0;
}

EAPI int
evas_async_events_process(void)
{
   int nr, count = 0;

   if (_fd_read == -1) return 0;

   _evas_async_events_fork_handle();

   while ((nr = _evas_async_events_process_single()) > 0) count += nr;

   evas_cache_image_wakeup();

   return count;
}

static void
_evas_async_events_fd_blocking_set(Eina_Bool blocking)
{
#ifdef HAVE_FCNTL
   long flags = fcntl(_fd_read, F_GETFL);

   if (blocking) flags &= ~O_NONBLOCK;
   else flags |= O_NONBLOCK;

   fcntl(_fd_read, F_SETFL, flags);
#else
   (void) blocking;
#endif
}

int
evas_async_events_process_blocking(void)
{
   int ret;

   _evas_async_events_fork_handle();

   _evas_async_events_fd_blocking_set(EINA_TRUE);
   ret = _evas_async_events_process_single();
   evas_cache_image_wakeup(); /* FIXME: is this needed ? */
   _evas_async_events_fd_blocking_set(EINA_FALSE);

   return ret;
}

EAPI Eina_Bool
evas_async_events_put(const void *target, Evas_Callback_Type type, void *event_info, Evas_Async_Events_Put_Cb func)
{
   Evas_Event_Async *ev;
   unsigned int count;
   Eina_Bool ret;

   if (!func) return EINA_FALSE;
   if (_fd_write == -1) return EINA_FALSE;

   _evas_async_events_fork_handle();

   eina_lock_take(&async_lock);

   count = async_queue.len;
   ev = eina_inarray_grow(&async_queue, 1);
   if (!ev)
     {
        eina_lock_release(&async_lock);
        return EINA_FALSE;
     }

   ev->func = func;
   ev->target = target;
   ev->type = type;
   ev->event_info = event_info;

   eina_lock_release(&async_lock);

   if (count == 0)
     {
        int wakeup = 1;
        ssize_t check;

        do
          {
             check = write(_fd_write, &wakeup, sizeof (int));
          }
        while ((check != sizeof (int)) &&
               ((errno == EINTR) || (errno == EAGAIN)));

        if (check == sizeof (int)) ret = EINA_TRUE;
        else
          {
             ret = EINA_FALSE;

             switch (errno)
               {
                case EBADF:
                case EINVAL:
                case EIO:
                case EPIPE:
                   _fd_write = -1;
               }
          }
     }
   else ret = EINA_TRUE;

   evas_cache_image_wakeup();

   return ret;
}
