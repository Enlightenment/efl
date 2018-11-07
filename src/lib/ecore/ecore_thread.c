#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef _WIN32
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

# define LK(x)        Eina_Lock x
# define LKI(x)       eina_lock_new(&(x))
# define LKD(x)       eina_lock_free(&(x))
# define LKL(x)       eina_lock_take(&(x))
# define LKU(x)       eina_lock_release(&(x))

# define SLK(x)       Eina_Spinlock x
# define SLKI(x)      eina_spinlock_new(&(x))
# define SLKD(x)      eina_spinlock_free(&(x))
# define SLKL(x)      eina_spinlock_take(&(x))
# define SLKU(x)      eina_spinlock_release(&(x))

# define CD(x)        Eina_Condition x
# define CDI(x, m)    eina_condition_new(&(x), &(m))
# define CDD(x)       eina_condition_free(&(x))
# define CDB(x)       eina_condition_broadcast(&(x))
# define CDW(x, t)    eina_condition_timedwait(&(x), t)

# define LRWK(x)      Eina_RWLock x
# define LRWKI(x)     eina_rwlock_new(&(x));
# define LRWKD(x)     eina_rwlock_free(&(x));
# define LRWKWL(x)    eina_rwlock_take_write(&(x));
# define LRWKRL(x)    eina_rwlock_take_read(&(x));
# define LRWKU(x)     eina_rwlock_release(&(x));

# define PH(x)        Eina_Thread x
# define PHE(x, y)    eina_thread_equal(x, y)
# define PHS()        eina_thread_self()
# define PHC(x, f, d) eina_thread_create(&(x), EINA_THREAD_BACKGROUND, -1, (void *)f, d)
# define PHJ(x)       eina_thread_join(x)

typedef struct _Ecore_Pthread_Worker Ecore_Pthread_Worker;
typedef struct _Ecore_Pthread        Ecore_Pthread;
typedef struct _Ecore_Thread_Data    Ecore_Thread_Data;
typedef struct _Ecore_Thread_Waiter  Ecore_Thread_Waiter;

struct _Ecore_Thread_Waiter
{
   Ecore_Thread_Cb func_cancel;
   Ecore_Thread_Cb func_end;
   Eina_Bool       waiting;
};

struct _Ecore_Thread_Data
{
   void        *data;
   Eina_Free_Cb cb;
};

struct _Ecore_Pthread_Worker
{
   union
   {
      struct
      {
         Ecore_Thread_Cb func_blocking;
      } short_run;
      struct
      {
         Ecore_Thread_Cb        func_heavy;
         Ecore_Thread_Notify_Cb func_notify;

         Ecore_Pthread_Worker  *direct_worker;

         int                    send;
         int                    received;
      } feedback_run;
      struct
      {
         Ecore_Thread_Cb        func_main;
         Ecore_Thread_Notify_Cb func_notify;

         Ecore_Pipe            *send;
         Ecore_Pthread_Worker  *direct_worker;

         struct
         {
            int send;
            int received;
         } from, to;
      } message_run;
   } u;

   Ecore_Thread_Waiter *waiter;
   Ecore_Thread_Cb      func_cancel;
   Ecore_Thread_Cb      func_end;
   PH(self);
   Eina_Hash           *hash;
   CD(cond);
   LK(mutex);

   const void          *data;

   int                  cancel;

   SLK(cancel_mutex);

   Eina_Bool            message_run : 1;
   Eina_Bool            feedback_run : 1;
   Eina_Bool            kill : 1;
   Eina_Bool            reschedule : 1;
   Eina_Bool            no_queue : 1;
};

typedef struct _Ecore_Pthread_Notify Ecore_Pthread_Notify;
struct _Ecore_Pthread_Notify
{
   Ecore_Pthread_Worker *work;
   const void           *user_data;
};

typedef void                       *(*Ecore_Thread_Sync_Cb)(void *data, Ecore_Thread *thread);

typedef struct _Ecore_Pthread_Message Ecore_Pthread_Message;
struct _Ecore_Pthread_Message
{
   union
   {
      Ecore_Thread_Cb      async;
      Ecore_Thread_Sync_Cb sync;
   } u;

   const void *data;

   int         code;

   Eina_Bool   callback : 1;
   Eina_Bool   sync : 1;
};

static int _ecore_thread_count_max = 0;

static void _ecore_thread_handler(void *data);

static int _ecore_thread_count = 0;
static int _ecore_thread_count_no_queue = 0;

static Eina_List *_ecore_running_job = NULL;
static Eina_List *_ecore_pending_job_threads = NULL;
static Eina_List *_ecore_pending_job_threads_feedback = NULL;
static SLK(_ecore_pending_job_threads_mutex);
static SLK(_ecore_running_job_mutex);

static Eina_Hash *_ecore_thread_global_hash = NULL;
static LRWK(_ecore_thread_global_hash_lock);
static LK(_ecore_thread_global_hash_mutex);
static CD(_ecore_thread_global_hash_cond);

static Eina_Bool have_main_loop_thread = 0;

static Eina_Trash *_ecore_thread_worker_trash = NULL;
static int _ecore_thread_worker_count = 0;

static void                 *_ecore_thread_worker(void *);
static Ecore_Pthread_Worker *_ecore_thread_worker_new(void);

static PH(get_main_loop_thread) (void)
{
   static PH(main_loop_thread);
   static pid_t main_loop_pid;
   pid_t pid = getpid();

   if (pid != main_loop_pid)
     {
        main_loop_pid = pid;
        main_loop_thread = PHS();
        have_main_loop_thread = 1;
     }

   return main_loop_thread;
}

static void
_ecore_thread_worker_free(Ecore_Pthread_Worker *worker)
{
   SLKD(worker->cancel_mutex);
   CDD(worker->cond);
   LKD(worker->mutex);

   if (_ecore_thread_worker_count > ((_ecore_thread_count_max + 1) * 16))
     {
        _ecore_thread_worker_count--;
        free(worker);
        return;
     }

   eina_trash_push(&_ecore_thread_worker_trash, worker);
}

static void
_ecore_thread_data_free(void *data)
{
   Ecore_Thread_Data *d = data;

   if (d->cb) d->cb(d->data);
   free(d);
}

void
_ecore_thread_join(void *data)
{
   PH(thread) = (uintptr_t)data;
   DBG("joining thread=%" PRIu64, (uint64_t)thread);
   PHJ(thread);
}

static void
_ecore_thread_kill(Ecore_Pthread_Worker *work)
{
   if (work->cancel)
     {
        if (work->func_cancel)
          work->func_cancel((void *)work->data, (Ecore_Thread *)work);
     }
   else
     {
        if (work->func_end)
          work->func_end((void *)work->data, (Ecore_Thread *)work);
     }

   if (work->feedback_run)
     {
        if (work->u.feedback_run.direct_worker)
          _ecore_thread_worker_free(work->u.feedback_run.direct_worker);
     }
   if (work->hash)
     eina_hash_free(work->hash);
   _ecore_thread_worker_free(work);
}

static void
_ecore_thread_handler(void *data)
{
   Ecore_Pthread_Worker *work = data;

   if (work->feedback_run)
     {
        if (work->u.feedback_run.send != work->u.feedback_run.received)
          {
             work->kill = EINA_TRUE;
             return;
          }
     }

   _ecore_thread_kill(work);
}

#if 0
static void
_ecore_nothing_handler(void *data EINA_UNUSED, void *buffer EINA_UNUSED, unsigned int nbyte EINA_UNUSED)
{
}

#endif

static void
_ecore_notify_handler(void *data)
{
   Ecore_Pthread_Notify *notify = data;
   Ecore_Pthread_Worker *work = notify->work;
   void *user_data = (void *)notify->user_data;

   work->u.feedback_run.received++;

   if (work->u.feedback_run.func_notify)
     work->u.feedback_run.func_notify((void *)work->data, (Ecore_Thread *)work, user_data);

   /* Force reading all notify event before killing the thread */
   if (work->kill && work->u.feedback_run.send == work->u.feedback_run.received)
     {
        _ecore_thread_kill(work);
     }

   free(notify);
}

static void
_ecore_message_notify_handler(void *data)
{
   Ecore_Pthread_Notify *notify = data;
   Ecore_Pthread_Worker *work = notify->work;
   Ecore_Pthread_Message *user_data = (void *)notify->user_data;
   Eina_Bool delete = EINA_TRUE;

   work->u.message_run.from.received++;

   if (!user_data->callback)
     {
        if (work->u.message_run.func_notify)
          work->u.message_run.func_notify((void *)work->data, (Ecore_Thread *)work, (void *)user_data->data);
     }
   else
     {
        if (user_data->sync)
          {
             user_data->data = user_data->u.sync((void *)user_data->data, (Ecore_Thread *)work);
             user_data->callback = EINA_FALSE;
             user_data->code = INT_MAX;
             ecore_pipe_write(work->u.message_run.send, &user_data, sizeof (Ecore_Pthread_Message *));

             delete = EINA_FALSE;
          }
        else
          {
             user_data->u.async((void *)user_data->data, (Ecore_Thread *)work);
          }
     }

   if (delete)
     {
        free(user_data);
     }

   /* Force reading all notify event before killing the thread */
   if (work->kill && work->u.message_run.from.send == work->u.message_run.from.received)
     {
        _ecore_thread_kill(work);
     }
   free(notify);
}

static void
_ecore_short_job_cleanup(void *data)
{
   Ecore_Pthread_Worker *work = data;

   DBG("cleanup work=%p, thread=%" PRIu64, work, (uint64_t)work->self);

   SLKL(_ecore_running_job_mutex);
   _ecore_running_job = eina_list_remove(_ecore_running_job, work);
   SLKU(_ecore_running_job_mutex);

   if (work->reschedule)
     {
        work->reschedule = EINA_FALSE;

        SLKL(_ecore_pending_job_threads_mutex);
        _ecore_pending_job_threads = eina_list_append(_ecore_pending_job_threads, work);
        SLKU(_ecore_pending_job_threads_mutex);
     }
   else
     {
        ecore_main_loop_thread_safe_call_async(_ecore_thread_handler, work);
     }
}

static void
_ecore_short_job(PH(thread))
{
   Ecore_Pthread_Worker *work;
   int cancel;

   SLKL(_ecore_pending_job_threads_mutex);

   if (!_ecore_pending_job_threads)
     {
        SLKU(_ecore_pending_job_threads_mutex);
        return;
     }

   work = eina_list_data_get(_ecore_pending_job_threads);
   _ecore_pending_job_threads = eina_list_remove_list(_ecore_pending_job_threads,
                                                      _ecore_pending_job_threads);
   SLKU(_ecore_pending_job_threads_mutex);

   SLKL(_ecore_running_job_mutex);
   _ecore_running_job = eina_list_append(_ecore_running_job, work);
   SLKU(_ecore_running_job_mutex);

   SLKL(work->cancel_mutex);
   cancel = work->cancel;
   SLKU(work->cancel_mutex);
   work->self = thread;

   EINA_THREAD_CLEANUP_PUSH(_ecore_short_job_cleanup, work);
   if (!cancel)
     work->u.short_run.func_blocking((void *)work->data, (Ecore_Thread *)work);
   eina_thread_cancellable_set(EINA_FALSE, NULL);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
}

static void
_ecore_feedback_job_cleanup(void *data)
{
   Ecore_Pthread_Worker *work = data;

   DBG("cleanup work=%p, thread=%" PRIu64, work, (uint64_t)work->self);

   SLKL(_ecore_running_job_mutex);
   _ecore_running_job = eina_list_remove(_ecore_running_job, work);
   SLKU(_ecore_running_job_mutex);

   if (work->reschedule)
     {
        work->reschedule = EINA_FALSE;

        SLKL(_ecore_pending_job_threads_mutex);
        _ecore_pending_job_threads_feedback = eina_list_append(_ecore_pending_job_threads_feedback, work);
        SLKU(_ecore_pending_job_threads_mutex);
     }
   else
     {
        ecore_main_loop_thread_safe_call_async(_ecore_thread_handler, work);
     }
}

static void
_ecore_feedback_job(PH(thread))
{
   Ecore_Pthread_Worker *work;
   int cancel;

   SLKL(_ecore_pending_job_threads_mutex);

   if (!_ecore_pending_job_threads_feedback)
     {
        SLKU(_ecore_pending_job_threads_mutex);
        return;
     }

   work = eina_list_data_get(_ecore_pending_job_threads_feedback);
   _ecore_pending_job_threads_feedback = eina_list_remove_list(_ecore_pending_job_threads_feedback,
                                                               _ecore_pending_job_threads_feedback);
   SLKU(_ecore_pending_job_threads_mutex);
   SLKL(_ecore_running_job_mutex);
   _ecore_running_job = eina_list_append(_ecore_running_job, work);
   SLKU(_ecore_running_job_mutex);

   SLKL(work->cancel_mutex);
   cancel = work->cancel;
   SLKU(work->cancel_mutex);
   work->self = thread;

   EINA_THREAD_CLEANUP_PUSH(_ecore_feedback_job_cleanup, work);
   if (!cancel)
     work->u.feedback_run.func_heavy((void *)work->data, (Ecore_Thread *)work);
   eina_thread_cancellable_set(EINA_FALSE, NULL);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);
}

static void
_ecore_direct_worker_cleanup(void *data)
{
   Ecore_Pthread_Worker *work = data;

   DBG("cleanup work=%p, thread=%" PRIu64 " (should join)", work, (uint64_t)work->self);

   SLKL(_ecore_pending_job_threads_mutex);
   _ecore_thread_count_no_queue--;
   ecore_main_loop_thread_safe_call_async(_ecore_thread_handler, work);

   ecore_main_loop_thread_safe_call_async((Ecore_Cb)_ecore_thread_join,
                                          (void *)(intptr_t)PHS());
   SLKU(_ecore_pending_job_threads_mutex);
}

static void *
_ecore_direct_worker(Ecore_Pthread_Worker *work)
{
   eina_thread_cancellable_set(EINA_FALSE, NULL);
   eina_thread_name_set(eina_thread_self(), "Ethread-feedback");
   work->self = PHS();

   EINA_THREAD_CLEANUP_PUSH(_ecore_direct_worker_cleanup, work);
   if (work->message_run)
     work->u.message_run.func_main((void *)work->data, (Ecore_Thread *)work);
   else
     work->u.feedback_run.func_heavy((void *)work->data, (Ecore_Thread *)work);
   eina_thread_cancellable_set(EINA_FALSE, NULL);
   EINA_THREAD_CLEANUP_POP(EINA_TRUE);

   return NULL;
}

static void
_ecore_thread_worker_cleanup(void *data EINA_UNUSED)
{
   DBG("cleanup thread=%" PRIuPTR " (should join)", PHS());
   SLKL(_ecore_pending_job_threads_mutex);
   _ecore_thread_count--;
   ecore_main_loop_thread_safe_call_async((Ecore_Cb)_ecore_thread_join,
                                          (void *)(intptr_t)PHS());
   SLKU(_ecore_pending_job_threads_mutex);
}

static void *
_ecore_thread_worker(void *data EINA_UNUSED)
{
   eina_thread_cancellable_set(EINA_FALSE, NULL);
   EINA_THREAD_CLEANUP_PUSH(_ecore_thread_worker_cleanup, NULL);
restart:

   /* these 2 are cancellation points as user cb may enable */
   _ecore_short_job(PHS());
   _ecore_feedback_job(PHS());

   /* from here on, cancellations are guaranteed to be disabled */

   /* FIXME: Check if there is feedback running task todo, and switch to feedback run handler. */
   eina_thread_name_set(eina_thread_self(), "Ethread-worker");

   SLKL(_ecore_pending_job_threads_mutex);
   if (_ecore_pending_job_threads || _ecore_pending_job_threads_feedback)
     {
        SLKU(_ecore_pending_job_threads_mutex);
        goto restart;
     }
   SLKU(_ecore_pending_job_threads_mutex);

   /* Sleep a little to prevent premature death */
#ifdef _WIN32
   Sleep(1); /* around 50ms */
#else
   usleep(50);
#endif

   SLKL(_ecore_pending_job_threads_mutex);
   if (_ecore_pending_job_threads || _ecore_pending_job_threads_feedback)
     {
        SLKU(_ecore_pending_job_threads_mutex);
        goto restart;
     }
   SLKU(_ecore_pending_job_threads_mutex);

   EINA_THREAD_CLEANUP_POP(EINA_TRUE);

   return NULL;
}

static Ecore_Pthread_Worker *
_ecore_thread_worker_new(void)
{
   Ecore_Pthread_Worker *result;

   result = eina_trash_pop(&_ecore_thread_worker_trash);

   if (!result)
     {
        result = calloc(1, sizeof(Ecore_Pthread_Worker));
        _ecore_thread_worker_count++;
     }
   else
     {
        memset(result, 0, sizeof(Ecore_Pthread_Worker));
     }

   SLKI(result->cancel_mutex);
   LKI(result->mutex);
   CDI(result->cond, result->mutex);

   return result;
}

void
_ecore_thread_init(void)
{
   _ecore_thread_count_max = eina_cpu_count() * 4;
   if (_ecore_thread_count_max <= 0)
     _ecore_thread_count_max = 1;

   SLKI(_ecore_pending_job_threads_mutex);
   LRWKI(_ecore_thread_global_hash_lock);
   LKI(_ecore_thread_global_hash_mutex);
   SLKI(_ecore_running_job_mutex);
   CDI(_ecore_thread_global_hash_cond, _ecore_thread_global_hash_mutex);
}

void
_ecore_thread_shutdown(void)
{
   /* FIXME: If function are still running in the background, should we kill them ? */
   Ecore_Pthread_Worker *work;
   Eina_List *l;
   Eina_Bool test;
   int iteration = 0;

   SLKL(_ecore_pending_job_threads_mutex);

   EINA_LIST_FREE(_ecore_pending_job_threads, work)
     {
        if (work->func_cancel)
          work->func_cancel((void *)work->data, (Ecore_Thread *)work);
        free(work);
     }

   EINA_LIST_FREE(_ecore_pending_job_threads_feedback, work)
     {
        if (work->func_cancel)
          work->func_cancel((void *)work->data, (Ecore_Thread *)work);
        free(work);
     }

   SLKU(_ecore_pending_job_threads_mutex);
   SLKL(_ecore_running_job_mutex);

   EINA_LIST_FOREACH(_ecore_running_job, l, work)
     ecore_thread_cancel((Ecore_Thread *)work);

   SLKU(_ecore_running_job_mutex);

   do
     {
        SLKL(_ecore_pending_job_threads_mutex);
        if (_ecore_thread_count + _ecore_thread_count_no_queue > 0)
          {
             test = EINA_TRUE;
          }
        else
          {
             test = EINA_FALSE;
          }
        SLKU(_ecore_pending_job_threads_mutex);
        iteration++;
        if (test)
          {
             _ecore_main_call_flush();
             usleep(1000);
          }
     } while (test == EINA_TRUE && iteration < 50);

   if (iteration == 20 && _ecore_thread_count > 0)
     {
        ERR("%i of the child thread are still running after 1s. This can lead to a segv. Sorry.", _ecore_thread_count);
     }

   if (_ecore_thread_global_hash)
     eina_hash_free(_ecore_thread_global_hash);
   have_main_loop_thread = 0;

   while ((work = eina_trash_pop(&_ecore_thread_worker_trash)))
     {
        free(work);
     }

   SLKD(_ecore_pending_job_threads_mutex);
   LRWKD(_ecore_thread_global_hash_lock);
   LKD(_ecore_thread_global_hash_mutex);
   SLKD(_ecore_running_job_mutex);
   CDD(_ecore_thread_global_hash_cond);
}

EAPI Ecore_Thread *
ecore_thread_run(Ecore_Thread_Cb func_blocking,
                 Ecore_Thread_Cb func_end,
                 Ecore_Thread_Cb func_cancel,
                 const void *data)
{
   Ecore_Pthread_Worker *work;
   Eina_Bool tried = EINA_FALSE;
   PH(thread);

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (!func_blocking) return NULL;

   work = _ecore_thread_worker_new();
   if (!work)
     {
        if (func_cancel)
          func_cancel((void *)data, NULL);
        return NULL;
     }

   work->u.short_run.func_blocking = func_blocking;
   work->func_end = func_end;
   work->func_cancel = func_cancel;
   work->cancel = EINA_FALSE;
   work->feedback_run = EINA_FALSE;
   work->message_run = EINA_FALSE;
   work->kill = EINA_FALSE;
   work->reschedule = EINA_FALSE;
   work->no_queue = EINA_FALSE;
   work->data = data;

   work->self = 0;
   work->hash = NULL;

   SLKL(_ecore_pending_job_threads_mutex);
   _ecore_pending_job_threads = eina_list_append(_ecore_pending_job_threads, work);

   if (_ecore_thread_count == _ecore_thread_count_max)
     {
        SLKU(_ecore_pending_job_threads_mutex);
        return (Ecore_Thread *)work;
     }

   SLKU(_ecore_pending_job_threads_mutex);

   /* One more thread could be created. */
   eina_threads_init();

   SLKL(_ecore_pending_job_threads_mutex);

retry:
   if (PHC(thread, _ecore_thread_worker, NULL))
     {
        _ecore_thread_count++;
        SLKU(_ecore_pending_job_threads_mutex);
        return (Ecore_Thread *)work;
     }
   if (!tried)
     {
        _ecore_main_call_flush();
        tried = EINA_TRUE;
        goto retry;
     }

   if (_ecore_thread_count == 0)
     {
        _ecore_pending_job_threads = eina_list_remove(_ecore_pending_job_threads, work);

        if (work->func_cancel)
          work->func_cancel((void *)work->data, (Ecore_Thread *)work);

        _ecore_thread_worker_free(work);
        work = NULL;
     }
   SLKU(_ecore_pending_job_threads_mutex);

   eina_threads_shutdown();

   return (Ecore_Thread *)work;
}

EAPI Eina_Bool
ecore_thread_cancel(Ecore_Thread *thread)
{
   Ecore_Pthread_Worker *volatile work = (Ecore_Pthread_Worker *)thread;
   Eina_List *l;
   int cancel;

   if (!work)
     return EINA_TRUE;
   SLKL(work->cancel_mutex);
   cancel = work->cancel;
   SLKU(work->cancel_mutex);
   if (cancel)
     return EINA_FALSE;

   if (work->feedback_run)
     {
        if (work->kill)
          return EINA_TRUE;
        if (work->u.feedback_run.send != work->u.feedback_run.received)
          goto on_exit;
     }

   SLKL(_ecore_pending_job_threads_mutex);

   if ((have_main_loop_thread) &&
       (PHE(get_main_loop_thread(), PHS())))
     {
        if (!work->feedback_run)
          EINA_LIST_FOREACH(_ecore_pending_job_threads, l, work)
            {
               if ((void *)work == (void *)thread)
                 {
                    _ecore_pending_job_threads = eina_list_remove_list(_ecore_pending_job_threads, l);

                    SLKU(_ecore_pending_job_threads_mutex);

                    if (work->func_cancel)
                      work->func_cancel((void *)work->data, (Ecore_Thread *)work);
                    free(work);

                    return EINA_TRUE;
                 }
            }
        else
          EINA_LIST_FOREACH(_ecore_pending_job_threads_feedback, l, work)
            {
               if ((void *)work == (void *)thread)
                 {
                    _ecore_pending_job_threads_feedback = eina_list_remove_list(_ecore_pending_job_threads_feedback, l);

                    SLKU(_ecore_pending_job_threads_mutex);

                    if (work->func_cancel)
                      work->func_cancel((void *)work->data, (Ecore_Thread *)work);
                    free(work);

                    return EINA_TRUE;
                 }
            }
     }

   SLKU(_ecore_pending_job_threads_mutex);

   work = (Ecore_Pthread_Worker *)thread;

   /* Delay the destruction */
on_exit:
   eina_thread_cancel(work->self); /* noop unless eina_thread_cancellable_set() was used by user */
   SLKL(work->cancel_mutex);
   work->cancel = EINA_TRUE;
   SLKU(work->cancel_mutex);

   return EINA_FALSE;
}

static void
_ecore_thread_wait_reset(Ecore_Thread_Waiter *waiter,
                         Ecore_Pthread_Worker *worker)
{
   worker->func_cancel = waiter->func_cancel;
   worker->func_end = waiter->func_end;
   worker->waiter = NULL;

   waiter->func_end = NULL;
   waiter->func_cancel = NULL;
   waiter->waiting = EINA_FALSE;
}

static void
_ecore_thread_wait_cancel(void *data EINA_UNUSED, Ecore_Thread *thread)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Waiter *waiter = worker->waiter;

   if (waiter->func_cancel) waiter->func_cancel(data, thread);
   _ecore_thread_wait_reset(waiter, worker);
}

static void
_ecore_thread_wait_end(void *data EINA_UNUSED, Ecore_Thread *thread)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Waiter *waiter = worker->waiter;

   if (waiter->func_end) waiter->func_end(data, thread);
   _ecore_thread_wait_reset(waiter, worker);
}

EAPI Eina_Bool
ecore_thread_wait(Ecore_Thread *thread, double wait)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Waiter waiter;

   if (!thread) return EINA_TRUE;

   waiter.func_end = worker->func_end;
   waiter.func_cancel = worker->func_cancel;
   waiter.waiting = EINA_TRUE;

   // Now trick the thread to call the wrapper function
   worker->waiter = &waiter;
   worker->func_cancel = _ecore_thread_wait_cancel;
   worker->func_end = _ecore_thread_wait_end;

   while (waiter.waiting == EINA_TRUE)
     {
        double start, end;

        start = ecore_time_get();
        _ecore_main_call_flush();
        ecore_main_loop_thread_safe_call_wait(0.0001);
        end = ecore_time_get();

        wait -= end - start;

        if (wait <= 0) break;
     }

   if (waiter.waiting == EINA_FALSE)
     {
        return EINA_TRUE;
     }
   else
     {
        _ecore_thread_wait_reset(&waiter, worker);
        return EINA_FALSE;
     }
}

EAPI Eina_Bool
ecore_thread_check(Ecore_Thread *thread)
{
   Ecore_Pthread_Worker *volatile worker = (Ecore_Pthread_Worker *)thread;
   int cancel;

   if (!worker) return EINA_TRUE;
   SLKL(worker->cancel_mutex);

   cancel = worker->cancel;
   /* FIXME: there is an insane bug driving me nuts here. I don't know if
      it's a race condition, some cache issue or some alien attack on our software.
      But ecore_thread_check will only work correctly with a printf, all the volatile,
      lock and even usleep don't help here... */
   /* fprintf(stderr, "wc: %i\n", cancel); */
   SLKU(worker->cancel_mutex);
   return cancel;
}

EAPI Ecore_Thread *
ecore_thread_feedback_run(Ecore_Thread_Cb func_heavy,
                          Ecore_Thread_Notify_Cb func_notify,
                          Ecore_Thread_Cb func_end,
                          Ecore_Thread_Cb func_cancel,
                          const void *data,
                          Eina_Bool try_no_queue)
{
   Ecore_Pthread_Worker *worker;
   Eina_Bool tried = EINA_FALSE;
   PH(thread);

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (!func_heavy) return NULL;

   worker = _ecore_thread_worker_new();
   if (!worker) goto on_error;

   worker->u.feedback_run.func_heavy = func_heavy;
   worker->u.feedback_run.func_notify = func_notify;
   worker->hash = NULL;
   worker->func_cancel = func_cancel;
   worker->func_end = func_end;
   worker->data = data;
   worker->cancel = EINA_FALSE;
   worker->message_run = EINA_FALSE;
   worker->feedback_run = EINA_TRUE;
   worker->kill = EINA_FALSE;
   worker->reschedule = EINA_FALSE;
   worker->self = 0;

   worker->u.feedback_run.send = 0;
   worker->u.feedback_run.received = 0;

   worker->u.feedback_run.direct_worker = NULL;

   if (try_no_queue)
     {
        PH(t);

        worker->u.feedback_run.direct_worker = _ecore_thread_worker_new();
        worker->no_queue = EINA_TRUE;

        eina_threads_init();

retry_direct:
        if (PHC(t, _ecore_direct_worker, worker))
          {
             SLKL(_ecore_pending_job_threads_mutex);
             _ecore_thread_count_no_queue++;
             SLKU(_ecore_pending_job_threads_mutex);
             return (Ecore_Thread *)worker;
          }
        if (!tried)
          {
             _ecore_main_call_flush();
             tried = EINA_TRUE;
             goto retry_direct;
          }

        if (worker->u.feedback_run.direct_worker)
          {
             _ecore_thread_worker_free(worker->u.feedback_run.direct_worker);
             worker->u.feedback_run.direct_worker = NULL;
          }

        eina_threads_shutdown();
     }

   worker->no_queue = EINA_FALSE;

   SLKL(_ecore_pending_job_threads_mutex);
   _ecore_pending_job_threads_feedback = eina_list_append(_ecore_pending_job_threads_feedback, worker);

   if (_ecore_thread_count == _ecore_thread_count_max)
     {
        SLKU(_ecore_pending_job_threads_mutex);
        return (Ecore_Thread *)worker;
     }

   SLKU(_ecore_pending_job_threads_mutex);

   /* One more thread could be created. */
   eina_threads_init();

   SLKL(_ecore_pending_job_threads_mutex);
retry:
   if (PHC(thread, _ecore_thread_worker, NULL))
     {
        _ecore_thread_count++;
        SLKU(_ecore_pending_job_threads_mutex);
        return (Ecore_Thread *)worker;
     }
   if (!tried)
     {
        _ecore_main_call_flush();
        tried = EINA_TRUE;
        goto retry;
     }
   SLKU(_ecore_pending_job_threads_mutex);

   eina_threads_shutdown();

on_error:
   SLKL(_ecore_pending_job_threads_mutex);
   if (_ecore_thread_count == 0)
     {
        _ecore_pending_job_threads_feedback = eina_list_remove(_ecore_pending_job_threads_feedback,
                                                               worker);

        if (func_cancel) func_cancel((void *)data, NULL);

        if (worker)
          {
             CDD(worker->cond);
             LKD(worker->mutex);
             free(worker);
             worker = NULL;
          }
     }
   SLKU(_ecore_pending_job_threads_mutex);

   return (Ecore_Thread *)worker;
}

EAPI Eina_Bool
ecore_thread_feedback(Ecore_Thread *thread,
                      const void *data)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;

   if (!worker) return EINA_FALSE;

   if (!PHE(worker->self, PHS())) return EINA_FALSE;

   if (worker->feedback_run)
     {
        Ecore_Pthread_Notify *notify;

        notify = malloc(sizeof (Ecore_Pthread_Notify));
        if (!notify) return EINA_FALSE;

        notify->user_data = data;
        notify->work = worker;
        worker->u.feedback_run.send++;

        ecore_main_loop_thread_safe_call_async(_ecore_notify_handler, notify);
     }
   else if (worker->message_run)
     {
        Ecore_Pthread_Message *msg;
        Ecore_Pthread_Notify *notify;

        msg = malloc(sizeof (Ecore_Pthread_Message));
        if (!msg) return EINA_FALSE;
        msg->data = data;
        msg->callback = EINA_FALSE;
        msg->sync = EINA_FALSE;

        notify = malloc(sizeof (Ecore_Pthread_Notify));
        if (!notify)
          {
             free(msg);
             return EINA_FALSE;
          }
        notify->work = worker;
        notify->user_data = msg;

        worker->u.message_run.from.send++;
        ecore_main_loop_thread_safe_call_async(_ecore_message_notify_handler, notify);
     }
   else
     return EINA_FALSE;

   return EINA_TRUE;
}

#if 0
EAPI Ecore_Thread *
ecore_thread_message_run(Ecore_Thread_Cb func_main,
                         Ecore_Thread_Notify_Cb func_notify,
                         Ecore_Thread_Cb func_end,
                         Ecore_Thread_Cb func_cancel,
                         const void *data)
{
   Ecore_Pthread_Worker *worker;
   PH(t);

   if (!func_main) return NULL;

   worker = _ecore_thread_worker_new();
   if (!worker) return NULL;

   worker->u.message_run.func_main = func_main;
   worker->u.message_run.func_notify = func_notify;
   worker->u.message_run.direct_worker = _ecore_thread_worker_new();
   worker->u.message_run.send = ecore_pipe_add(_ecore_nothing_handler, worker);
   worker->u.message_run.from.send = 0;
   worker->u.message_run.from.received = 0;
   worker->u.message_run.to.send = 0;
   worker->u.message_run.to.received = 0;

   ecore_pipe_freeze(worker->u.message_run.send);

   worker->func_cancel = func_cancel;
   worker->func_end = func_end;
   worker->hash = NULL;
   worker->data = data;

   worker->cancel = EINA_FALSE;
   worker->message_run = EINA_TRUE;
   worker->feedback_run = EINA_FALSE;
   worker->kill = EINA_FALSE;
   worker->reschedule = EINA_FALSE;
   worker->no_queue = EINA_FALSE;
   worker->self = 0;

   eina_threads_init();

   if (PHC(t, _ecore_direct_worker, worker))
     return (Ecore_Thread *)worker;

   eina_threads_shutdown();

   if (worker->u.message_run.direct_worker) _ecore_thread_worker_free(worker->u.message_run.direct_worker);
   if (worker->u.message_run.send) ecore_pipe_del(worker->u.message_run.send);

   CDD(worker->cond);
   LKD(worker->mutex);

   func_cancel((void *)data, NULL);

   return NULL;
}

#endif

EAPI Eina_Bool
ecore_thread_reschedule(Ecore_Thread *thread)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;

   if (!worker) return EINA_FALSE;

   if (!PHE(worker->self, PHS())) return EINA_FALSE;

   worker->reschedule = EINA_TRUE;
   return EINA_TRUE;
}

EAPI int
ecore_thread_active_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return _ecore_thread_count;
}

EAPI int
ecore_thread_pending_get(void)
{
   int ret;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   SLKL(_ecore_pending_job_threads_mutex);
   ret = eina_list_count(_ecore_pending_job_threads);
   SLKU(_ecore_pending_job_threads_mutex);
   return ret;
}

EAPI int
ecore_thread_pending_feedback_get(void)
{
   int ret;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   SLKL(_ecore_pending_job_threads_mutex);
   ret = eina_list_count(_ecore_pending_job_threads_feedback);
   SLKU(_ecore_pending_job_threads_mutex);
   return ret;
}

EAPI int
ecore_thread_pending_total_get(void)
{
   int ret;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   SLKL(_ecore_pending_job_threads_mutex);
   ret = eina_list_count(_ecore_pending_job_threads) + eina_list_count(_ecore_pending_job_threads_feedback);
   SLKU(_ecore_pending_job_threads_mutex);
   return ret;
}

EAPI int
ecore_thread_max_get(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return _ecore_thread_count_max;
}

EAPI void
ecore_thread_max_set(int num)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   if (num < 1) return;
   /* avoid doing something hilarious by blocking dumb users */
   if (num > (32 * eina_cpu_count())) num = 32 * eina_cpu_count();

   _ecore_thread_count_max = num;
}

EAPI void
ecore_thread_max_reset(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _ecore_thread_count_max = eina_cpu_count() * 4;
}

EAPI int
ecore_thread_available_get(void)
{
   int ret;

   SLKL(_ecore_pending_job_threads_mutex);
   ret = _ecore_thread_count_max - _ecore_thread_count;
   SLKU(_ecore_pending_job_threads_mutex);
   return ret;
}

EAPI Eina_Bool
ecore_thread_local_data_add(Ecore_Thread *thread,
                            const char *key,
                            void *value,
                            Eina_Free_Cb cb,
                            Eina_Bool direct)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Data *d;
   Eina_Bool ret;

   if ((!thread) || (!key) || (!value))
     return EINA_FALSE;

   LKL(worker->mutex);
   if (!worker->hash)
     worker->hash = eina_hash_string_small_new(_ecore_thread_data_free);
   LKU(worker->mutex);

   if (!worker->hash)
     return EINA_FALSE;

   if (!(d = malloc(sizeof(Ecore_Thread_Data))))
     return EINA_FALSE;

   d->data = value;
   d->cb = cb;

   LKL(worker->mutex);
   if (direct)
     ret = eina_hash_direct_add(worker->hash, key, d);
   else
     ret = eina_hash_add(worker->hash, key, d);
   LKU(worker->mutex);
   CDB(worker->cond);
   return ret;
}

EAPI void *
ecore_thread_local_data_set(Ecore_Thread *thread,
                            const char *key,
                            void *value,
                            Eina_Free_Cb cb)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Data *d, *r;
   void *ret;

   if ((!thread) || (!key) || (!value))
     return NULL;

   LKL(worker->mutex);
   if (!worker->hash)
     worker->hash = eina_hash_string_small_new(_ecore_thread_data_free);
   LKU(worker->mutex);

   if (!worker->hash)
     return NULL;

   if (!(d = malloc(sizeof(Ecore_Thread_Data))))
     return NULL;

   d->data = value;
   d->cb = cb;

   LKL(worker->mutex);
   r = eina_hash_set(worker->hash, key, d);
   LKU(worker->mutex);
   CDB(worker->cond);

   if (r)
     {
        ret = r->data;
        free(r);
        return ret;
     }
   return NULL;
}

EAPI void *
ecore_thread_local_data_find(Ecore_Thread *thread,
                             const char *key)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Data *d;

   if ((!thread) || (!key))
     return NULL;

   if (!worker->hash)
     return NULL;

   LKL(worker->mutex);
   d = eina_hash_find(worker->hash, key);
   LKU(worker->mutex);
   if (d)
     return d->data;
   return NULL;
}

EAPI Eina_Bool
ecore_thread_local_data_del(Ecore_Thread *thread,
                            const char *key)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Eina_Bool r;

   if ((!thread) || (!key))
     return EINA_FALSE;

   if (!worker->hash)
     return EINA_FALSE;

   LKL(worker->mutex);
   r = eina_hash_del_by_key(worker->hash, key);
   LKU(worker->mutex);
   return r;
}

EAPI Eina_Bool
ecore_thread_global_data_add(const char *key,
                             void *value,
                             Eina_Free_Cb cb,
                             Eina_Bool direct)
{
   Ecore_Thread_Data *d;
   Eina_Bool ret;

   if ((!key) || (!value))
     return EINA_FALSE;

   LRWKWL(_ecore_thread_global_hash_lock);
   if (!_ecore_thread_global_hash)
     _ecore_thread_global_hash = eina_hash_string_small_new(_ecore_thread_data_free);
   LRWKU(_ecore_thread_global_hash_lock);

   if (!(d = malloc(sizeof(Ecore_Thread_Data))))
     return EINA_FALSE;

   d->data = value;
   d->cb = cb;

   if (!_ecore_thread_global_hash)
     {
        free(d);
        return EINA_FALSE;
     }

   LRWKWL(_ecore_thread_global_hash_lock);
   if (direct)
     ret = eina_hash_direct_add(_ecore_thread_global_hash, key, d);
   else
     ret = eina_hash_add(_ecore_thread_global_hash, key, d);
   LRWKU(_ecore_thread_global_hash_lock);
   CDB(_ecore_thread_global_hash_cond);
   return ret;
}

EAPI void *
ecore_thread_global_data_set(const char *key,
                             void *value,
                             Eina_Free_Cb cb)
{
   Ecore_Thread_Data *d, *r;
   void *ret;

   if ((!key) || (!value))
     return NULL;

   LRWKWL(_ecore_thread_global_hash_lock);
   if (!_ecore_thread_global_hash)
     _ecore_thread_global_hash = eina_hash_string_small_new(_ecore_thread_data_free);
   LRWKU(_ecore_thread_global_hash_lock);

   if (!_ecore_thread_global_hash)
     return NULL;

   if (!(d = malloc(sizeof(Ecore_Thread_Data))))
     return NULL;

   d->data = value;
   d->cb = cb;

   LRWKWL(_ecore_thread_global_hash_lock);
   r = eina_hash_set(_ecore_thread_global_hash, key, d);
   LRWKU(_ecore_thread_global_hash_lock);
   CDB(_ecore_thread_global_hash_cond);

   if (r)
     {
        ret = r->data;
        free(r);
        return ret;
     }
   return NULL;
}

EAPI void *
ecore_thread_global_data_find(const char *key)
{
   Ecore_Thread_Data *ret;

   if (!key)
     return NULL;

   if (!_ecore_thread_global_hash) return NULL;

   LRWKRL(_ecore_thread_global_hash_lock);
   ret = eina_hash_find(_ecore_thread_global_hash, key);
   LRWKU(_ecore_thread_global_hash_lock);
   if (ret)
     return ret->data;
   return NULL;
}

EAPI Eina_Bool
ecore_thread_global_data_del(const char *key)
{
   Eina_Bool ret;

   if (!key)
     return EINA_FALSE;

   if (!_ecore_thread_global_hash)
     return EINA_FALSE;

   LRWKWL(_ecore_thread_global_hash_lock);
   ret = eina_hash_del_by_key(_ecore_thread_global_hash, key);
   LRWKU(_ecore_thread_global_hash_lock);
   return ret;
}

EAPI void *
ecore_thread_global_data_wait(const char *key,
                              double seconds)
{
   double tm = 0;
   Ecore_Thread_Data *ret = NULL;

   if (!key)
     return NULL;

   if (seconds > 0)
     tm = ecore_time_get() + seconds;

   while (1)
     {
        LRWKRL(_ecore_thread_global_hash_lock);
        if (_ecore_thread_global_hash)
          ret = eina_hash_find(_ecore_thread_global_hash, key);
        LRWKU(_ecore_thread_global_hash_lock);
        if ((ret) ||
            (!EINA_DBL_EQ(seconds, 0.0)) ||
            ((seconds > 0) && (tm <= ecore_time_get())))
          break;
        LKL(_ecore_thread_global_hash_mutex);
        CDW(_ecore_thread_global_hash_cond, tm - ecore_time_get());
        LKU(_ecore_thread_global_hash_mutex);
     }
   if (ret) return ret->data;
   return NULL;
}

