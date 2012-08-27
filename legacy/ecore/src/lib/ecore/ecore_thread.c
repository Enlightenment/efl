
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

#ifdef EFL_HAVE_THREADS

# define LK(x) Eina_Lock x
# define LKI(x) eina_lock_new(&(x))
# define LKD(x) eina_lock_free(&(x))
# define LKL(x) eina_lock_take(&(x))
# define LKU(x) eina_lock_release(&(x))

# define CD(x) Eina_Condition x
# define CDI(x, m) eina_condition_new(&(x), &(m))
# define CDD(x) eina_condition_free(&(x))
# define CDB(x) eina_condition_broadcast(&(x))
# define CDW(x, t) eina_condition_timedwait(&(x), t)

# define LRWK(x) Eina_RWLock x
# define LRWKI(x) eina_rwlock_new(&(x));
# define LRWKD(x) eina_rwlock_free(&(x));
# define LRWKWL(x) eina_rwlock_take_write(&(x));
# define LRWKRL(x) eina_rwlock_take_read(&(x));
# define LRWKU(x) eina_rwlock_release(&(x));

# ifdef EFL_HAVE_POSIX_THREADS
#  include <pthread.h>
#  ifdef __linux__
#   include <sched.h>
#   include <sys/resource.h>
#   include <unistd.h>
#   include <sys/syscall.h>
#   include <errno.h>
#  endif

#  define PH(x)        pthread_t x
#  define PHE(x, y)    pthread_equal(x, y)
#  define PHS()        pthread_self()
#  define PHC(x, f, d) pthread_create(&(x), NULL, (void *)f, d)
#  define PHJ(x)       pthread_join(x, NULL)
#  define PHA(x)       pthread_cancel(x)

# else /* EFL_HAVE_WIN32_THREADS */

#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  undef WIN32_LEAN_AND_MEAN

typedef struct
{
   HANDLE thread;
   void  *val;
} win32_thread;

static Eina_List    *_ecore_thread_win32_threads = NULL;
static Eina_Lock     _ecore_thread_win32_lock;

#  define PH(x)     win32_thread * x
#  define PHE(x, y) ((x) == (y))

static win32_thread *
_ecore_thread_win32_self()
{
   win32_thread *t;
   Eina_List *l;

   LKL(_ecore_thread_win32_lock);
   EINA_LIST_FOREACH(_ecore_thread_win32_threads, l, t)
     if (t->thread == GetCurrentThread())
       {
          LKU(_ecore_thread_win32_lock);
          return t;
       }

   LKU(_ecore_thread_win32_lock);
   return NULL;
}

#  define PHS()     _ecore_thread_win32_self()

static int
_ecore_thread_win32_create(win32_thread         **x,
                           LPTHREAD_START_ROUTINE f,
                           void                  *d)
{
   win32_thread *t;

   t = (win32_thread *)calloc(1, sizeof(win32_thread));
   if (!t)
     return -1;

   LKL(_ecore_thread_win32_lock);
   (t)->thread = CreateThread(NULL, 0, f, d, 0, NULL);
   if (!t->thread)
     {
        free(t);
        LKU(_ecore_thread_win32_lock);
        return -1;
     }
   t->val = d;
   *x = t;
   _ecore_thread_win32_threads = eina_list_append(_ecore_thread_win32_threads, t);
   LKU(_ecore_thread_win32_lock);

   return 0;
}

#  define PHC(x, f, d) _ecore_thread_win32_create(&(x), (LPTHREAD_START_ROUTINE)f, d)

static int
_ecore_thread_win32_join(win32_thread *x,
                         void        **res)
{
   if (!PHE(x, PHS()))
     {
        WaitForSingleObject(x->thread, INFINITE);
        CloseHandle(x->thread);
     }
   if (res) *res = x->val;
   _ecore_thread_win32_threads = eina_list_remove(_ecore_thread_win32_threads, x);
   free(x);

   return 0;
}

#  define PHJ(x) _ecore_thread_win32_join(x, NULL)
#  define PHA(x)    TerminateThread(x->thread, 0)

# endif

#endif

typedef struct _Ecore_Pthread_Worker Ecore_Pthread_Worker;
typedef struct _Ecore_Pthread        Ecore_Pthread;
typedef struct _Ecore_Thread_Data    Ecore_Thread_Data;

struct _Ecore_Thread_Data
{
   void        *data;
   Eina_Free_Cb cb;
};

struct _Ecore_Pthread_Worker
{
   union {
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
      struct {
         Ecore_Thread_Cb func_main;
         Ecore_Thread_Notify_Cb func_notify;

         Ecore_Pipe            *send;
         Ecore_Pthread_Worker  *direct_worker;

         struct {
            int send;
            int received;
         } from, to;
      } message_run;
   } u;

   Ecore_Thread_Cb func_cancel;
   Ecore_Thread_Cb func_end;
#ifdef EFL_HAVE_THREADS
                   PH(self);
   Eina_Hash      *hash;
                   CD(cond);
                   LK(mutex);
#endif

   const void     *data;

   int cancel;

#ifdef EFL_HAVE_THREADS
   LK(cancel_mutex);
#endif

   Eina_Bool message_run : 1;
   Eina_Bool feedback_run : 1;
   Eina_Bool kill : 1;
   Eina_Bool reschedule : 1;
   Eina_Bool no_queue : 1;
};

#ifdef EFL_HAVE_THREADS
typedef struct _Ecore_Pthread_Notify Ecore_Pthread_Notify;
struct _Ecore_Pthread_Notify
{
   Ecore_Pthread_Worker *work;
   const void *user_data;
};

typedef void *(*Ecore_Thread_Sync_Cb)(void* data, Ecore_Thread *thread);

typedef struct _Ecore_Pthread_Message Ecore_Pthread_Message;
struct _Ecore_Pthread_Message
{
   union {
      Ecore_Thread_Cb async;
      Ecore_Thread_Sync_Cb sync;
   } u;

   const void *data;

   int code;

   Eina_Bool callback : 1;
   Eina_Bool sync : 1;
};

#endif

static int _ecore_thread_count_max = 0;

#ifdef EFL_HAVE_THREADS

static void _ecore_thread_handler(void *data);

static int _ecore_thread_count = 0;

static Eina_List *_ecore_running_job = NULL;
static Eina_List *_ecore_pending_job_threads = NULL;
static Eina_List *_ecore_pending_job_threads_feedback = NULL;
static LK(_ecore_pending_job_threads_mutex);
static LK(_ecore_running_job_mutex);

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
   LKD(worker->cancel_mutex);
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

static void
_ecore_thread_join(PH(thread))
{
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
_ecore_nothing_handler(void *data __UNUSED__, void *buffer __UNUSED__, unsigned int nbyte __UNUSED__)
{
}
#endif

static void
_ecore_notify_handler(void *data)
{
   Ecore_Pthread_Notify *notify = data;
   Ecore_Pthread_Worker *work = notify->work;
   void *user_data = (void*) notify->user_data;

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
   Ecore_Pthread_Message *user_data = (void *) notify->user_data;
   Eina_Bool delete = EINA_TRUE;

   work->u.message_run.from.received++;

   if (!user_data->callback)
     {
        if (work->u.message_run.func_notify)
          work->u.message_run.func_notify((void *) work->data, (Ecore_Thread *) work, (void *) user_data->data);
     }
   else
     {
        if (user_data->sync)
          {
             user_data->data = user_data->u.sync((void*) user_data->data, (Ecore_Thread *) work);
             user_data->callback = EINA_FALSE;
             user_data->code = INT_MAX;
             ecore_pipe_write(work->u.message_run.send, &user_data, sizeof (Ecore_Pthread_Message *));

             delete = EINA_FALSE;
          }
        else
          {
             user_data->u.async((void*) user_data->data, (Ecore_Thread *) work);
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
_ecore_short_job(PH(thread))
{
   Ecore_Pthread_Worker *work;
   int cancel;

   LKL(_ecore_pending_job_threads_mutex);
   
   if (!_ecore_pending_job_threads)
     {
        LKU(_ecore_pending_job_threads_mutex);
        return;
     }
   
   work = eina_list_data_get(_ecore_pending_job_threads);
   _ecore_pending_job_threads = eina_list_remove_list(_ecore_pending_job_threads,
                                                      _ecore_pending_job_threads);
   LKU(_ecore_pending_job_threads_mutex);

   LKL(_ecore_running_job_mutex);
   _ecore_running_job = eina_list_append(_ecore_running_job, work);
   LKU(_ecore_running_job_mutex);
   
   LKL(work->cancel_mutex);
   cancel = work->cancel;
   LKU(work->cancel_mutex);
   work->self = thread;
   if (!cancel)
     work->u.short_run.func_blocking((void *) work->data, (Ecore_Thread*) work);

   LKL(_ecore_running_job_mutex);
   _ecore_running_job = eina_list_remove(_ecore_running_job, work);
   LKU(_ecore_running_job_mutex);
   
   if (work->reschedule)
     {
        work->reschedule = EINA_FALSE;
        
        LKL(_ecore_pending_job_threads_mutex);
        _ecore_pending_job_threads = eina_list_append(_ecore_pending_job_threads, work);
        LKU(_ecore_pending_job_threads_mutex);
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
   
   LKL(_ecore_pending_job_threads_mutex);
   
   if (!_ecore_pending_job_threads_feedback)
     {
        LKU(_ecore_pending_job_threads_mutex);
        return;
     }
   
   work = eina_list_data_get(_ecore_pending_job_threads_feedback);
   _ecore_pending_job_threads_feedback = eina_list_remove_list(_ecore_pending_job_threads_feedback,
                                                               _ecore_pending_job_threads_feedback);
   LKU(_ecore_pending_job_threads_mutex);
   LKL(_ecore_running_job_mutex);
   _ecore_running_job = eina_list_append(_ecore_running_job, work);
   LKU(_ecore_running_job_mutex);
   
   LKL(work->cancel_mutex);
   cancel = work->cancel;
   LKU(work->cancel_mutex);
   work->self = thread;
   if (!cancel)
     work->u.feedback_run.func_heavy((void *) work->data, (Ecore_Thread *) work);

   LKL(_ecore_running_job_mutex);
   _ecore_running_job = eina_list_remove(_ecore_running_job, work);
   LKU(_ecore_running_job_mutex);

   if (work->reschedule)
     {
        work->reschedule = EINA_FALSE;
        
        LKL(_ecore_pending_job_threads_mutex);
        _ecore_pending_job_threads_feedback = eina_list_append(_ecore_pending_job_threads_feedback, work);
        LKU(_ecore_pending_job_threads_mutex);
     }
   else
     {
        ecore_main_loop_thread_safe_call_async(_ecore_thread_handler, work);
     }
}

static void *
_ecore_direct_worker(Ecore_Pthread_Worker *work)
{
#ifdef EFL_POSIX_THREADS
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

   eina_sched_prio_drop();

   work->self = PHS();
   if (work->message_run)
     work->u.message_run.func_main((void *) work->data, (Ecore_Thread *) work);
   else
     work->u.feedback_run.func_heavy((void *) work->data, (Ecore_Thread *) work);

   ecore_main_loop_thread_safe_call_async(_ecore_thread_handler, work);

   ecore_main_loop_thread_safe_call_async((Ecore_Cb) _ecore_thread_join, 
					  (void*) PHS());

   return NULL;
}

static void *
_ecore_thread_worker(void *data __UNUSED__)
{
#ifdef EFL_POSIX_THREADS
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

   eina_sched_prio_drop();

restart:
   _ecore_short_job(PHS());
   _ecore_feedback_job(PHS());

   /* FIXME: Check if there is feedback running task todo, and switch to feedback run handler. */

   LKL(_ecore_pending_job_threads_mutex);
   if (_ecore_pending_job_threads || _ecore_pending_job_threads_feedback)
     {
        LKU(_ecore_pending_job_threads_mutex);
        goto restart;
     }
   LKU(_ecore_pending_job_threads_mutex);

   /* Sleep a little to prevent premature death */
#ifdef _WIN32
   Sleep(1); /* around 50ms */
#else
   usleep(50);
#endif

   LKL(_ecore_pending_job_threads_mutex);
   if (_ecore_pending_job_threads || _ecore_pending_job_threads_feedback)
     {
        LKU(_ecore_pending_job_threads_mutex);
        goto restart;
     }
   _ecore_thread_count--;

   ecore_main_loop_thread_safe_call_async((Ecore_Cb) _ecore_thread_join,
					  (void*) PHS());
   LKU(_ecore_pending_job_threads_mutex);

   return NULL;
}

#endif

static Ecore_Pthread_Worker *
_ecore_thread_worker_new(void)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Pthread_Worker *result;

   result = eina_trash_pop(&_ecore_thread_worker_trash);

   if (!result) 
     {
       result = calloc(1, sizeof(Ecore_Pthread_Worker));
       _ecore_thread_worker_count++;
     }

   LKI(result->cancel_mutex);
   LKI(result->mutex);
   CDI(result->cond, result->mutex);

   return result;
#else
   return malloc(sizeof (Ecore_Pthread_Worker));
#endif
}

void
_ecore_thread_init(void)
{
   _ecore_thread_count_max = eina_cpu_count();
   if (_ecore_thread_count_max <= 0)
     _ecore_thread_count_max = 1;

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_WIN32_THREADS
   LKI(_ecore_thread_win32_lock);
# endif
   LKI(_ecore_pending_job_threads_mutex);
   LRWKI(_ecore_thread_global_hash_lock);
   LKI(_ecore_thread_global_hash_mutex);
   LKI(_ecore_running_job_mutex);
   CDI(_ecore_thread_global_hash_cond, _ecore_thread_global_hash_mutex);
#endif
}

void
_ecore_thread_shutdown(void)
{
   /* FIXME: If function are still running in the background, should we kill them ? */
#ifdef EFL_HAVE_THREADS
    Ecore_Pthread_Worker *work;
    Eina_List *l;
    Eina_Bool test;
    int iteration = 0;

    LKL(_ecore_pending_job_threads_mutex);

    EINA_LIST_FREE(_ecore_pending_job_threads, work)
      {
         if (work->func_cancel)
           work->func_cancel((void *)work->data, (Ecore_Thread *) work);
         free(work);
      }

    EINA_LIST_FREE(_ecore_pending_job_threads_feedback, work)
      {
         if (work->func_cancel)
           work->func_cancel((void *)work->data, (Ecore_Thread *) work);
         free(work);
      }

    LKU(_ecore_pending_job_threads_mutex);
    LKL(_ecore_running_job_mutex);

    EINA_LIST_FOREACH(_ecore_running_job, l, work)
      ecore_thread_cancel((Ecore_Thread*) work);

    LKU(_ecore_running_job_mutex);

    do
      {
	 LKL(_ecore_pending_job_threads_mutex);
	 if (_ecore_thread_count > 0)
	   {
	      test = EINA_TRUE;
	   }
	 else
	   {
	      test = EINA_FALSE;
	   }
	 LKU(_ecore_pending_job_threads_mutex);
	 iteration++;
	 if (test) usleep(50000);
      }
    while (test == EINA_TRUE && iteration < 20);

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

    LKD(_ecore_pending_job_threads_mutex);
    LRWKD(_ecore_thread_global_hash_lock);
    LKD(_ecore_thread_global_hash_mutex);
    LKD(_ecore_running_job_mutex);
    CDD(_ecore_thread_global_hash_cond);
# ifdef EFL_HAVE_WIN32_THREADS
   LKU(_ecore_thread_win32_lock);
# endif
#endif
}

EAPI Ecore_Thread *
ecore_thread_run(Ecore_Thread_Cb func_blocking,
                 Ecore_Thread_Cb func_end,
                 Ecore_Thread_Cb func_cancel,
                 const void     *data)
{
   Ecore_Pthread_Worker *work;
   Eina_Bool tried = EINA_FALSE;
#ifdef EFL_HAVE_THREADS
   PH(thread);
#endif

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

#ifdef EFL_HAVE_THREADS
   work->self = 0;
   work->hash = NULL;

   LKL(_ecore_pending_job_threads_mutex);
   _ecore_pending_job_threads = eina_list_append(_ecore_pending_job_threads, work);

   if (_ecore_thread_count == _ecore_thread_count_max)
     {
        LKU(_ecore_pending_job_threads_mutex);
        return (Ecore_Thread *)work;
     }

   LKU(_ecore_pending_job_threads_mutex);

   /* One more thread could be created. */
   eina_threads_init();

   LKL(_ecore_pending_job_threads_mutex);

 retry:
   if (PHC(thread, _ecore_thread_worker, NULL) == 0)
     {
        _ecore_thread_count++;
	LKU(_ecore_pending_job_threads_mutex);
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
          work->func_cancel((void *) work->data, (Ecore_Thread *) work);

        CDD(work->cond);
        LKD(work->mutex);
        LKD(work->cancel_mutex);
        free(work);
        work = NULL;
     }
   LKU(_ecore_pending_job_threads_mutex);

   eina_threads_shutdown();

   return (Ecore_Thread *)work;
#else
   /*
      If no thread and as we don't want to break app that rely on this
      facility, we will lock the interface until we are done.
    */
   do {
        /* Handle reschedule by forcing it here. That would mean locking the app,
         * would be better with an idler, but really to complex for a case where
         * thread should really exist.
         */
          work->reschedule = EINA_FALSE;

          func_blocking((void *)data, (Ecore_Thread *)work);
          if (work->cancel == EINA_FALSE) func_end((void *)data, (Ecore_Thread *)work);
          else func_cancel((void *)data, (Ecore_Thread *)work);
     } while (work->reschedule == EINA_TRUE);

   free(work);

   return NULL;
#endif
}

EAPI Eina_Bool
ecore_thread_cancel(Ecore_Thread *thread)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Pthread_Worker *volatile work = (Ecore_Pthread_Worker *)thread;
   Eina_List *l;
   int cancel;

   if (!work)
     return EINA_TRUE;
   LKL(work->cancel_mutex);
   cancel = work->cancel;
   LKU(work->cancel_mutex);
   if (cancel)
     return EINA_FALSE;

   if (work->feedback_run)
     {
        if (work->kill)
          return EINA_TRUE;
        if (work->u.feedback_run.send != work->u.feedback_run.received)
          goto on_exit;
     }

   LKL(_ecore_pending_job_threads_mutex);

   if ((have_main_loop_thread) &&
       (PHE(get_main_loop_thread(), PHS())))
     {
        if (!work->feedback_run)
          EINA_LIST_FOREACH(_ecore_pending_job_threads, l, work)
            {
               if ((void *)work == (void *)thread)
                 {
                    _ecore_pending_job_threads = eina_list_remove_list(_ecore_pending_job_threads, l);

                    LKU(_ecore_pending_job_threads_mutex);

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

                    LKU(_ecore_pending_job_threads_mutex);

                    if (work->func_cancel)
                      work->func_cancel((void *)work->data, (Ecore_Thread *)work);
                    free(work);

                    return EINA_TRUE;
                 }
            }
     }

   LKU(_ecore_pending_job_threads_mutex);

   work = (Ecore_Pthread_Worker *)thread;

   /* Delay the destruction */
 on_exit:
   LKL(work->cancel_mutex);
   work->cancel = EINA_TRUE;
   LKU(work->cancel_mutex);

   return EINA_FALSE;
#else
   (void) thread;
   return EINA_TRUE;
#endif
}

EAPI Eina_Bool
ecore_thread_check(Ecore_Thread *thread)
{
   Ecore_Pthread_Worker *volatile worker = (Ecore_Pthread_Worker *) thread;
   int cancel;

   if (!worker) return EINA_TRUE;
#ifdef EFL_HAVE_THREADS
   LKL(worker->cancel_mutex);
#endif
   cancel = worker->cancel;
   /* FIXME: there is an insane bug driving me nuts here. I don't know if
    it's a race condition, some cache issue or some alien attack on our software.
    But ecore_thread_check will only work correctly with a printf, all the volatile,
    lock and even usleep don't help here... */
   /* fprintf(stderr, "wc: %i\n", cancel); */
#ifdef EFL_HAVE_THREADS
   LKU(worker->cancel_mutex);
#endif
   return cancel;
}

EAPI Ecore_Thread *
ecore_thread_feedback_run(Ecore_Thread_Cb        func_heavy,
                          Ecore_Thread_Notify_Cb func_notify,
                          Ecore_Thread_Cb        func_end,
                          Ecore_Thread_Cb        func_cancel,
                          const void            *data,
                          Eina_Bool              try_no_queue)
{
#ifdef EFL_HAVE_THREADS
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
        if (PHC(t, _ecore_direct_worker, worker) == 0)
          return (Ecore_Thread *)worker;
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

   LKL(_ecore_pending_job_threads_mutex);
   _ecore_pending_job_threads_feedback = eina_list_append(_ecore_pending_job_threads_feedback, worker);

   if (_ecore_thread_count == _ecore_thread_count_max)
     {
        LKU(_ecore_pending_job_threads_mutex);
        return (Ecore_Thread *)worker;
     }

   LKU(_ecore_pending_job_threads_mutex);

   /* One more thread could be created. */
   eina_threads_init();

   LKL(_ecore_pending_job_threads_mutex);
 retry:
   if (PHC(thread, _ecore_thread_worker, NULL) == 0)
     {
        _ecore_thread_count++;
	LKU(_ecore_pending_job_threads_mutex);
        return (Ecore_Thread *)worker;
     }
   if (!tried)
     {
        _ecore_main_call_flush();
	tried = EINA_TRUE;
	goto retry;
     }
   LKU(_ecore_pending_job_threads_mutex);

   eina_threads_shutdown();

on_error:
   LKL(_ecore_pending_job_threads_mutex);
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
   LKU(_ecore_pending_job_threads_mutex);

   return (Ecore_Thread *)worker;
#else
   Ecore_Pthread_Worker worker;

   (void)try_no_queue;

   /*
      If no thread and as we don't want to break app that rely on this
      facility, we will lock the interface until we are done.
    */
   worker.u.feedback_run.func_heavy = func_heavy;
   worker.u.feedback_run.func_notify = func_notify;
   worker.u.feedback_run.send = 0;
   worker.u.feedback_run.received = 0;
   worker.func_cancel = func_cancel;
   worker.func_end = func_end;
   worker.data = data;
   worker.cancel = EINA_FALSE;
   worker.feedback_run = EINA_TRUE;
   worker.message_run = EINA_FALSE;
   worker.kill = EINA_FALSE;

   do {
        worker.reschedule = EINA_FALSE;

        func_heavy((void *)data, (Ecore_Thread *)&worker);

        if (worker.cancel) func_cancel((void *)data, (Ecore_Thread *)&worker);
        else func_end((void *)data, (Ecore_Thread *)&worker);
     } while (worker.reschedule == EINA_TRUE);

   return NULL;
#endif
}

EAPI Eina_Bool
ecore_thread_feedback(Ecore_Thread *thread,
                      const void   *data)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;

   if (!worker) return EINA_FALSE;

#ifdef EFL_HAVE_THREADS
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

        msg = malloc(sizeof (Ecore_Pthread_Message*));
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
#else
   worker->u.feedback_run.func_notify((void *)worker->data, thread, (void *)data);

   return EINA_TRUE;
#endif
}

#if 0
EAPI Ecore_Thread *
ecore_thread_message_run(Ecore_Thread_Cb func_main,
			 Ecore_Thread_Notify_Cb func_notify,
			 Ecore_Thread_Cb func_end,
			 Ecore_Thread_Cb func_cancel,
			 const void *data)
{
#ifdef EFL_HAVE_THREADS
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

  if (PHC(t, _ecore_direct_worker, worker) == 0)
    return (Ecore_Thread*) worker;

  eina_threads_shutdown();

  if (worker->u.message_run.direct_worker) _ecore_thread_worker_free(worker->u.message_run.direct_worker);
  if (worker->u.message_run.send) ecore_pipe_del(worker->u.message_run.send);

  CDD(worker->cond);
  LKD(worker->mutex);
#else
  /* Note: This type of thread can't and never will work without thread support */
  WRN("ecore_thread_message_run called, but threads disable in Ecore, things will go wrong. Starting now !");
# warning "You disabled threads support in ecore, I hope you know what you are doing !"
#endif

  func_cancel((void *) data, NULL);

  return NULL;
}
#endif

EAPI Eina_Bool
ecore_thread_reschedule(Ecore_Thread *thread)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;

   if (!worker) return EINA_FALSE;

#ifdef EFL_HAVE_THREADS
   if (!PHE(worker->self, PHS())) return EINA_FALSE;
#endif

   worker->reschedule = EINA_TRUE;
   return EINA_TRUE;
}

EAPI int
ecore_thread_active_get(void)
{
#ifdef EFL_HAVE_THREADS
   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   return _ecore_thread_count;
#else
   return 0;
#endif
}

EAPI int
ecore_thread_pending_get(void)
{
#ifdef EFL_HAVE_THREADS
   int ret;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   LKL(_ecore_pending_job_threads_mutex);
   ret = eina_list_count(_ecore_pending_job_threads);
   LKU(_ecore_pending_job_threads_mutex);
   return ret;
#else
   return 0;
#endif
}

EAPI int
ecore_thread_pending_feedback_get(void)
{
#ifdef EFL_HAVE_THREADS
   int ret;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   LKL(_ecore_pending_job_threads_mutex);
   ret = eina_list_count(_ecore_pending_job_threads_feedback);
   LKU(_ecore_pending_job_threads_mutex);
   return ret;
#else
   return 0;
#endif
}

EAPI int
ecore_thread_pending_total_get(void)
{
#ifdef EFL_HAVE_THREADS
   int ret;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(0);
   LKL(_ecore_pending_job_threads_mutex);
   ret = eina_list_count(_ecore_pending_job_threads) + eina_list_count(_ecore_pending_job_threads_feedback);
   LKU(_ecore_pending_job_threads_mutex);
   return ret;
#else
   return 0;
#endif
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
   if (num > (16 * eina_cpu_count())) num = 16 * eina_cpu_count();

   _ecore_thread_count_max = num;
}

EAPI void
ecore_thread_max_reset(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
   _ecore_thread_count_max = eina_cpu_count();
}

EAPI int
ecore_thread_available_get(void)
{
#ifdef EFL_HAVE_THREADS
   int ret;

   LKL(_ecore_pending_job_threads_mutex);
   ret = _ecore_thread_count_max - _ecore_thread_count;
   LKU(_ecore_pending_job_threads_mutex);
   return ret;
#else
   return 0;
#endif
}

EAPI Eina_Bool
ecore_thread_local_data_add(Ecore_Thread *thread,
                            const char   *key,
                            void         *value,
                            Eina_Free_Cb  cb,
                            Eina_Bool     direct)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Data *d;
   Eina_Bool ret;
#endif

   if ((!thread) || (!key) || (!value))
     return EINA_FALSE;
#ifdef EFL_HAVE_THREADS
   if (!PHE(worker->self, PHS())) return EINA_FALSE;

   if (!worker->hash)
     worker->hash = eina_hash_string_small_new(_ecore_thread_data_free);

   if (!worker->hash)
     return EINA_FALSE;

   if (!(d = malloc(sizeof(Ecore_Thread_Data))))
     return EINA_FALSE;

   d->data = value;
   d->cb = cb;

   if (direct)
     ret = eina_hash_direct_add(worker->hash, key, d);
   else
     ret = eina_hash_add(worker->hash, key, d);
   CDB(worker->cond);
   return ret;
#else
   (void) cb;
   (void) direct;
   return EINA_FALSE;
#endif
}

EAPI void *
ecore_thread_local_data_set(Ecore_Thread *thread,
                            const char   *key,
                            void         *value,
                            Eina_Free_Cb  cb)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Data *d, *r;
   void *ret;
#endif

   if ((!thread) || (!key) || (!value))
     return NULL;
#ifdef EFL_HAVE_THREADS
   if (!PHE(worker->self, PHS())) return NULL;

   if (!worker->hash)
     worker->hash = eina_hash_string_small_new(_ecore_thread_data_free);

   if (!worker->hash)
     return NULL;

   if (!(d = malloc(sizeof(Ecore_Thread_Data))))
     return NULL;

   d->data = value;
   d->cb = cb;

   r = eina_hash_set(worker->hash, key, d);
   CDB(worker->cond);
   ret = r->data;
   free(r);
   return ret;
#else
   (void) cb;
   return NULL;
#endif
}

EAPI void *
ecore_thread_local_data_find(Ecore_Thread *thread,
                             const char   *key)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
   Ecore_Thread_Data *d;
#endif

   if ((!thread) || (!key))
     return NULL;
#ifdef EFL_HAVE_THREADS
   if (!PHE(worker->self, PHS())) return NULL;

   if (!worker->hash)
     return NULL;

   d = eina_hash_find(worker->hash, key);
   if (d)
     return d->data;
   return NULL;
#else
   return NULL;
#endif
}

EAPI Eina_Bool
ecore_thread_local_data_del(Ecore_Thread *thread,
                            const char   *key)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *)thread;
#endif

   if ((!thread) || (!key))
     return EINA_FALSE;
#ifdef EFL_HAVE_THREADS
   if (!PHE(worker->self, PHS())) return EINA_FALSE;

   if (!worker->hash)
     return EINA_FALSE;
   return eina_hash_del_by_key(worker->hash, key);
#else
   return EINA_TRUE;
#endif
}

EAPI Eina_Bool
ecore_thread_global_data_add(const char  *key,
                             void        *value,
                             Eina_Free_Cb cb,
                             Eina_Bool    direct)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Thread_Data *d;
   Eina_Bool ret;
#endif

   if ((!key) || (!value))
     return EINA_FALSE;
#ifdef EFL_HAVE_THREADS
   LRWKWL(_ecore_thread_global_hash_lock);
   if (!_ecore_thread_global_hash)
     _ecore_thread_global_hash = eina_hash_string_small_new(_ecore_thread_data_free);
   LRWKU(_ecore_thread_global_hash_lock);

   if (!(d = malloc(sizeof(Ecore_Thread_Data))))
     return EINA_FALSE;

   d->data = value;
   d->cb = cb;

   if (!_ecore_thread_global_hash)
     return EINA_FALSE;
   LRWKWL(_ecore_thread_global_hash_lock);
   if (direct)
     ret = eina_hash_direct_add(_ecore_thread_global_hash, key, d);
   else
     ret = eina_hash_add(_ecore_thread_global_hash, key, d);
   LRWKU(_ecore_thread_global_hash_lock);
   CDB(_ecore_thread_global_hash_cond);
   return ret;
#else
   (void) cb;
   (void) direct;
   return EINA_TRUE;
#endif
}

EAPI void *
ecore_thread_global_data_set(const char  *key,
                             void        *value,
                             Eina_Free_Cb cb)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Thread_Data *d, *r;
   void *ret;
#endif

   if ((!key) || (!value))
     return NULL;
#ifdef EFL_HAVE_THREADS
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

   ret = r->data;
   free(r);
   return ret;
#else
   (void) cb;
   return NULL;
#endif
}

EAPI void *
ecore_thread_global_data_find(const char *key)
{
#ifdef EFL_HAVE_THREADS
   Ecore_Thread_Data *ret;
#endif

   if (!key)
     return NULL;
#ifdef EFL_HAVE_THREADS
   if (!_ecore_thread_global_hash) return NULL;

   LRWKRL(_ecore_thread_global_hash_lock);
   ret = eina_hash_find(_ecore_thread_global_hash, key);
   LRWKU(_ecore_thread_global_hash_lock);
   if (ret)
     return ret->data;
   return NULL;
#else
   return NULL;
#endif
}

EAPI Eina_Bool
ecore_thread_global_data_del(const char *key)
{
#ifdef EFL_HAVE_THREADS
   Eina_Bool ret;
#endif

   if (!key)
     return EINA_FALSE;
#ifdef EFL_HAVE_THREADS
   if (!_ecore_thread_global_hash)
     return EINA_FALSE;

   LRWKWL(_ecore_thread_global_hash_lock);
   ret = eina_hash_del_by_key(_ecore_thread_global_hash, key);
   LRWKU(_ecore_thread_global_hash_lock);
   return ret;
#else
   return EINA_TRUE;
#endif
}

EAPI void *
ecore_thread_global_data_wait(const char *key,
                              double      seconds)
{
#ifdef EFL_HAVE_THREADS
   double tm = 0;
   Ecore_Thread_Data *ret = NULL;
#endif

   if (!key)
     return NULL;
#ifdef EFL_HAVE_THREADS
   if (!_ecore_thread_global_hash)
     return NULL;
   if (seconds > 0)
     tm = ecore_time_get() + seconds;

   while (1)
     {
        LRWKRL(_ecore_thread_global_hash_lock);
        ret = eina_hash_find(_ecore_thread_global_hash, key);
        LRWKU(_ecore_thread_global_hash_lock);
        if ((ret) || (!seconds) || ((seconds > 0) && (tm <= ecore_time_get())))
          break;
        LKL(_ecore_thread_global_hash_mutex);
        CDW(_ecore_thread_global_hash_cond, tm);
        LKU(_ecore_thread_global_hash_mutex);
     }
   if (ret) return ret->data;
   return NULL;
#else
   (void) seconds;
   return NULL;
#endif
}

