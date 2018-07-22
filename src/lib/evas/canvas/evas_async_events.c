#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <errno.h>

#ifdef _WIN32
# include <winsock2.h>
#endif /* ! _WIN32 */

#include <fcntl.h>

#include "evas_common_private.h"
#include "evas_private.h"
#include "Ecore.h"

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

static Eina_Bool _write_error = EINA_TRUE;
static Eina_Bool _read_error = EINA_TRUE;

static Eina_Spinlock async_lock;
static Eina_Inarray async_queue;
static Evas_Event_Async *async_queue_cache = NULL;
static unsigned int async_queue_cache_max = 0;

static Ecore_Pipe *_async_pipe = NULL;
static int _event_count = 0;


static int _init_evas_event = 0;
static const int wakeup = 1;

static void _evas_async_events_fd_blocking_set(Eina_Bool blocking EINA_UNUSED);

static void
_async_events_pipe_read_cb(void *data EINA_UNUSED, void *buf, unsigned int len)
{
   if (wakeup != *(int*)buf || sizeof(int) != len)
     return;

   Evas_Event_Async *ev;
   unsigned int ln, max;
   int nr;

   eina_spinlock_take(&async_lock);

   ev = async_queue.members;
   async_queue.members = async_queue_cache;
   async_queue_cache = ev;

   max = async_queue.max;
   async_queue.max = async_queue_cache_max;
   async_queue_cache_max = max;

   ln = async_queue.len;
   async_queue.len = 0;

   eina_spinlock_release(&async_lock);

   DBG("Evas async events queue length: %u", len);
   nr = ln;

   while (ln > 0)
    {
       if (ev->func) ev->func((void *)ev->target, ev->type, ev->event_info);
       ev++;
       ln--;
    }
   _event_count += nr;

   _evas_async_events_fd_blocking_set(EINA_FALSE);
}

static void
_evas_async_events_fork_handle(void *data EINA_UNUSED)
{
   ecore_pipe_del(_async_pipe);
   _async_pipe = ecore_pipe_add(_async_events_pipe_read_cb, NULL);
}

int
evas_async_events_init(void)
{

   if (_init_evas_event++)
     return _init_evas_event;

   ecore_fork_reset_callback_add(_evas_async_events_fork_handle, NULL);

   _async_pipe = ecore_pipe_add(_async_events_pipe_read_cb, NULL);
   if ( !_async_pipe )
     {
        _init_evas_event = 0;
        return 0;
     }

   _read_error = EINA_FALSE;
   _write_error = EINA_FALSE;

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
   if (--_init_evas_event)
     return _init_evas_event;

   eina_condition_free(&_thread_cond);
   eina_lock_free(&_thread_mutex);
   eina_condition_free(&_thread_feedback_cond);
   eina_lock_free(&_thread_feedback_mutex);
   eina_spinlock_free(&_thread_id_lock);

   free(async_queue_cache);
   async_queue_cache = NULL;
   async_queue_cache_max = 0;

   eina_spinlock_free(&async_lock);
   eina_inarray_flush(&async_queue);

   ecore_fork_reset_callback_del(_evas_async_events_fork_handle, NULL);

   ecore_pipe_del(_async_pipe);
   _read_error = EINA_TRUE;
   _write_error = EINA_TRUE;

   return _init_evas_event;
}

EAPI int
evas_async_events_fd_get(void)
{
   return -1;
}

EAPI int
evas_async_events_process(void)
{
   int count = 0;

   if (_read_error) return -1;

   _event_count = 0;
   while (ecore_pipe_wait(_async_pipe, 1, 0.0))
     count = _event_count;

   return count;
}

static void
_evas_async_events_fd_blocking_set(Eina_Bool blocking)
{
#ifdef HAVE_FCNTL
   int _fd_read = ecore_pipe_read_fd(_async_pipe);
   long flags = fcntl(_fd_read, F_GETFL);

   if (blocking) flags &= ~O_NONBLOCK;
   else flags |= O_NONBLOCK;

   if (fcntl(_fd_read, F_SETFL, flags) < 0) ERR("cannot set fd flags");
#else
   (void) blocking;
#endif
}

EAPI int
evas_async_events_process_blocking(void)
{
   int ret;
   if (_read_error) return -1;

   _evas_async_events_fd_blocking_set(EINA_TRUE);

   _event_count = 0;
   ecore_pipe_wait(_async_pipe, 1, 0.0);
   ret = _event_count;

   _evas_async_events_fd_blocking_set(EINA_FALSE);

   return ret;
}

EAPI Eina_Bool
evas_async_events_put(const void *target, Evas_Callback_Type type, void *event_info, Evas_Async_Events_Put_Cb func)
{
   Evas_Event_Async *ev;
   unsigned int count;
   Eina_Bool ret = EINA_TRUE;;

   if (!func) return EINA_FALSE;
   if (_write_error) return EINA_FALSE;

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
        if (!ecore_pipe_write(_async_pipe, &wakeup, sizeof(int)))
          {
             ret = EINA_FALSE;
             _write_error = EINA_TRUE;
          }
     }

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
