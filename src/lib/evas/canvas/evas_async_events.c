#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef _MSC_VER
# include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>

#include "evas_common_private.h"
#include "evas_private.h"

typedef struct _Evas_Event_Async	Evas_Event_Async;
struct _Evas_Event_Async
{
   const void		    *target;
   void			    *event_info;
   Evas_Async_Events_Put_Cb  func;
   Evas_Callback_Type	     type;
};

typedef struct _Evas_Safe_Call Evas_Safe_Call;
struct _Evas_Safe_Call
{
   Eina_Condition c;
   Eina_Lock      m;

   int            current_id;
};

static Eina_Lock _thread_mutex;
static Eina_Condition _thread_cond;

static Eina_Lock _thread_feedback_mutex;
static Eina_Condition _thread_feedback_cond;

static int _thread_loop = 0;

static Eina_Spinlock _thread_id_lock;
static int _thread_id = -1;
static int _thread_id_max = 0;
static int _thread_id_update = 0;

static int _fd_write = -1;
static int _fd_read = -1;
static pid_t _fd_pid = 0;

static Eina_Spinlock async_lock;
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
   if (fcntl(_fd_read, F_SETFL, O_NONBLOCK) < 0) ERR("Can't set NONBLOCK on async fd");
#endif

   eina_spinlock_new(&async_lock);
   eina_inarray_step_set(&async_queue, sizeof (Eina_Inarray), sizeof (Evas_Event_Async), 16);

   eina_lock_new(&_thread_mutex);
   eina_condition_new(&_thread_cond, &_thread_mutex);

   eina_lock_new(&_thread_feedback_mutex);
   eina_condition_new(&_thread_feedback_cond, &_thread_feedback_mutex);

   eina_spinlock_new(&_thread_id_lock);

   return _init_evas_event;
}

int
evas_async_events_shutdown(void)
{
   _init_evas_event--;
   if (_init_evas_event > 0) return _init_evas_event;

   eina_condition_free(&_thread_cond);
   eina_lock_free(&_thread_mutex);
   eina_condition_free(&_thread_feedback_cond);
   eina_lock_free(&_thread_feedback_mutex);
   eina_spinlock_free(&_thread_id_lock);

   eina_spinlock_free(&async_lock);
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

        eina_spinlock_take(&async_lock);

        ev = async_queue.members;
        async_queue.members = async_queue_cache;
        async_queue_cache = ev;

        max = async_queue.max;
        async_queue.max = async_queue_cache_max;
        async_queue_cache_max = max;

        len = async_queue.len;
        async_queue.len = 0;

        eina_spinlock_release(&async_lock);

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

EAPI int
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

   eina_spinlock_take(&async_lock);

   count = async_queue.len;
   ev = eina_inarray_grow(&async_queue, 1);
   if (!ev)
     {
        eina_spinlock_release(&async_lock);
        return EINA_FALSE;
     }

   ev->func = func;
   ev->target = target;
   ev->type = type;
   ev->event_info = event_info;

   eina_spinlock_release(&async_lock);

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

static void
_evas_thread_main_loop_lock(void *target EINA_UNUSED,
                            Evas_Callback_Type type EINA_UNUSED,
                            void *event_info)
{
   Evas_Safe_Call *call = event_info;

   eina_lock_take(&_thread_mutex);

   eina_lock_take(&call->m);
   _thread_id = call->current_id;
   eina_condition_broadcast(&call->c);
   eina_lock_release(&call->m);

   while (_thread_id_update != _thread_id)
     eina_condition_wait(&_thread_cond);
   eina_lock_release(&_thread_mutex);

   eina_main_loop_define();

   eina_lock_take(&_thread_feedback_mutex);

   _thread_id = -1;

   eina_condition_broadcast(&_thread_feedback_cond);
   eina_lock_release(&_thread_feedback_mutex);

   eina_condition_free(&call->c);
   eina_lock_free(&call->m);
   free(call);
}                           

EAPI int
evas_thread_main_loop_begin(void)
{
   Evas_Safe_Call *order;

   if (eina_main_loop_is())
     {
        return ++_thread_loop;
     }

   order = malloc(sizeof (Evas_Safe_Call));
   if (!order) return -1;

   eina_spinlock_take(&_thread_id_lock);
   order->current_id = ++_thread_id_max;
   if (order->current_id < 0)
     {
        _thread_id_max = 0;
        order->current_id = ++_thread_id_max;
     }
   eina_spinlock_release(&_thread_id_lock);

   eina_lock_new(&order->m);
   eina_condition_new(&order->c, &order->m);

   evas_async_events_put(NULL, 0, order, _evas_thread_main_loop_lock);

   eina_lock_take(&order->m);
   while (order->current_id != _thread_id)
     eina_condition_wait(&order->c);
   eina_lock_release(&order->m);

   eina_main_loop_define();

   _thread_loop = 1;

   return _thread_loop;
}

EAPI int
evas_thread_main_loop_end(void)
{
   int current_id;

   if (_thread_loop == 0)
     abort();

   /* until we unlock the main loop, this thread has the main loop id */
   if (!eina_main_loop_is())
     {
        ERR("Not in a locked thread !");
        return -1;
     }

   _thread_loop--;
   if (_thread_loop > 0)
     return _thread_loop;

   current_id = _thread_id;

   eina_lock_take(&_thread_mutex);
   _thread_id_update = _thread_id;
   eina_condition_broadcast(&_thread_cond);
   eina_lock_release(&_thread_mutex);

   eina_lock_take(&_thread_feedback_mutex);
   while (current_id == _thread_id && _thread_id != -1)
     eina_condition_wait(&_thread_feedback_cond);
   eina_lock_release(&_thread_feedback_mutex);

   return 0;
}
