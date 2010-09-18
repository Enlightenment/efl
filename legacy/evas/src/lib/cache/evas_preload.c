#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef BUILD_ASYNC_PRELOAD
# include <pthread.h>
# include <sched.h>
# ifdef __linux__
# include <sys/syscall.h>
# endif
#endif

#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

static int _threads_max = 0;

#ifdef BUILD_ASYNC_PRELOAD

typedef struct _Evas_Preload_Pthread_Worker Evas_Preload_Pthread_Worker;
typedef struct _Evas_Preload_Pthread_Data Evas_Preload_Pthread_Data;

typedef void (*_evas_preload_pthread_func)(void *data);

struct _Evas_Preload_Pthread_Worker
{
   _evas_preload_pthread_func func_heavy;
   _evas_preload_pthread_func func_end;
   _evas_preload_pthread_func func_cancel;
   void *data;
   Eina_Bool cancel : 1;
};

struct _Evas_Preload_Pthread_Data
{
   pthread_t thread;
};

static int _threads_count = 0;
static Eina_List *_workers = NULL;

static LK(_mutex) = PTHREAD_MUTEX_INITIALIZER;

static void
_evas_preload_thread_end(void *data)
{
   Evas_Preload_Pthread_Data *pth = data;
   Evas_Preload_Pthread_Data *p = NULL;

   if (pthread_join(pth->thread, (void **)&p) != 0) free(p);
}

static void
_evas_preload_thread_done(void *target __UNUSED__, Evas_Callback_Type type __UNUSED__, void *event_info)
{
   Evas_Preload_Pthread_Worker *work = event_info;
   if (work->cancel)
     {
	if (work->func_cancel) work->func_cancel(work->data);
     }
   else
      work->func_end(work->data);
   free(work);
}

/* Lower priority of current thread.
 *
 * It's used by worker threads so they use up "bg cpu" as it was really intended
 * to work. If current thread is running with real-time priority, we decrease
 * our priority by 5. This is done in a portable way.  Otherwise we are
 * running with SCHED_OTHER policy and there's no portable way to set the nice
 * level on current thread. In Linux, it does work and it's the only one that is
 * implemented.
 */
static void
_evas_preload_thread_pri_drop(void)
{
   struct sched_param param;
   int pol, prio, ret;
   pid_t tid;
   pthread_t pthread_id;

   pthread_id = pthread_self();
   ret = pthread_getschedparam(pthread_id, &pol, &param);
   if (ret)
     {
        ERR("Unable to query sched parameters");
        return;
     }

   if (EINA_UNLIKELY(pol == SCHED_RR || pol == SCHED_FIFO))
     {
        prio = sched_get_priority_max(pol);
        param.sched_priority += 5;
        if (prio > 0 && param.sched_priority > prio)
           param.sched_priority = prio;

        pthread_setschedparam(pthread_id, pol, &param);
     }
#ifdef __linux__
   else
     {
        tid = syscall(SYS_gettid);
        errno = 0;
        prio = getpriority(PRIO_PROCESS, tid);
        if (errno == 0)
          {
             prio += 5;
             if (prio > 19)
                prio = 19;

             setpriority(PRIO_PROCESS, tid, prio);
          }
     }
#endif
}

static void *
_evas_preload_thread_worker(void *data)
{
   Evas_Preload_Pthread_Data *pth = data;
   Evas_Preload_Pthread_Worker *work;
   
   _evas_preload_thread_pri_drop();
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
on_error:
   for (;;)
     {
	LKL(_mutex);
	if (!_workers)
	  {
	     LKU(_mutex);
	     break;
	  }

	work = eina_list_data_get(_workers);
	_workers = eina_list_remove_list(_workers, _workers);
	LKU(_mutex);
        
	if (work->func_heavy) work->func_heavy(work->data);
	evas_async_events_put(pth, 0, work, _evas_preload_thread_done);
     }

   LKL(_mutex);
   if (_workers)
     {
	LKU(_mutex);
	goto on_error;
     }
   _threads_count--;
   LKU(_mutex);

   // dummy worker to wake things up
   work = malloc(sizeof(Evas_Preload_Pthread_Worker));
   if (!work) return NULL;

   work->data = pth;
   work->func_heavy = NULL;
   work->func_end = (_evas_preload_pthread_func) _evas_preload_thread_end;
   work->func_cancel = NULL;
   work->cancel = EINA_FALSE;

   evas_async_events_put(pth, 0, work, _evas_preload_thread_done);
   return pth;
}
#endif

void
_evas_preload_thread_init(void)
{
   _threads_max = eina_cpu_count();
   if (_threads_max < 1) _threads_max = 1;
}

void
_evas_preload_thread_shutdown(void)
{
   /* FIXME: If function are still running in the background, should we kill them ? */
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Preload_Pthread_Worker *work;

   /* Force processing of async events. */
   evas_async_events_process();
   LKL(_mutex);
   EINA_LIST_FREE(_workers, work)
     {
	if (work->func_cancel) work->func_cancel(work->data);
	free(work);
     }
   LKU(_mutex);
#endif
}

Evas_Preload_Pthread *
evas_preload_thread_run(void (*func_heavy) (void *data),
			void (*func_end) (void *data),
			void (*func_cancel) (void *data),
			const void *data)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Preload_Pthread_Worker *work;
   Evas_Preload_Pthread_Data *pth;

   work = malloc(sizeof(Evas_Preload_Pthread_Worker));
   if (!work)
     {
	func_cancel((void *)data);
	return NULL;
     }

   work->func_heavy = func_heavy;
   work->func_end = func_end;
   work->func_cancel = func_cancel;
   work->cancel = EINA_FALSE;
   work->data = (void *)data;

   LKL(_mutex);
   _workers = eina_list_append(_workers, work);
   if (_threads_count == _threads_max)
     {
	LKU(_mutex);
	return (Evas_Preload_Pthread *)work;
     }
   LKU(_mutex);

   /* One more thread could be created. */
   pth = malloc(sizeof(Evas_Preload_Pthread_Data));
   if (!pth) goto on_error;
   
   if (pthread_create(&pth->thread, NULL, _evas_preload_thread_worker, pth) == 0)
     {
	LKL(_mutex);
	_threads_count++;
	LKU(_mutex);
	return (Evas_Preload_Pthread*)work;
     }

 on_error:
   LKL(_mutex);
   if (_threads_count == 0)
     {
	LKU(_mutex);
	if (work->func_cancel) work->func_cancel(work->data);
	free(work);
        return NULL;
     }
   LKU(_mutex);
   return NULL;
#else
   /*
    If no thread and as we don't want to break app that rely on this
    facility, we will lock the interface until we are done.
    */
   func_heavy((void *)data);
   func_end((void *)data);
   return (void *)1;
#endif
}

Eina_Bool
evas_preload_thread_cancel(Evas_Preload_Pthread *thread)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Preload_Pthread_Worker *work;
   Eina_List *l;

   if (!thread) return EINA_TRUE;
   LKL(_mutex);
   EINA_LIST_FOREACH(_workers, l, work)
     {
        if (work == (Evas_Preload_Pthread_Worker *)thread)
          {
             _workers = eina_list_remove_list(_workers, l);
             LKU(_mutex);
             if (work->func_cancel) work->func_cancel(work->data);
             free(work);
             return EINA_TRUE;
          }
     }
   LKU(_mutex);
   
   /* Delay the destruction */
   work = (Evas_Preload_Pthread_Worker *)thread;
   work->cancel = EINA_TRUE;
   return EINA_FALSE;
#else
   return EINA_TRUE;
#endif
}
