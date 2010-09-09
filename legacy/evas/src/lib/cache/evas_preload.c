#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef BUILD_ASYNC_PRELOAD
# include <pthread.h>
#endif

#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

#ifdef BUILD_ASYNC_PRELOAD

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
#endif

static int _evas_preload_thread_count_max = 0;

#ifdef BUILD_ASYNC_PRELOAD
static int _evas_preload_thread_count = 0;
static Eina_List *_evas_preload_thread_workers = NULL;
static Eina_List *_evas_preload_thread = NULL;

static LK(_mutex) = PTHREAD_MUTEX_INITIALIZER;

static void
_evas_preload_thread_end(void *data)
{
   Evas_Preload_Pthread_Data *pth = data;
   Evas_Preload_Pthread_Data *p;

   if (pthread_join(pth->thread, (void **)&p) != 0)
     {
        free(p);
        return;
     }

   _evas_preload_thread = eina_list_remove(_evas_preload_thread, pth);
}

static void
_evas_preload_thread_done(void *target __UNUSED__, Evas_Callback_Type type __UNUSED__, void *event_info)
{
   Evas_Preload_Pthread_Worker *work;

   work = event_info;

   if (work->cancel)
     {
	if (work->func_cancel) work->func_cancel(work->data);
     }
   else
     {
	work->func_end(work->data);
     }

   free(work);
}

static void *
_evas_preload_thread_worker(void *data)
{
   Evas_Preload_Pthread_Data *pth = data;
   Evas_Preload_Pthread_Worker *work;
   
   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
   
on_error:
   
   for (;;)
     {
	LKL(_mutex);

	if (!_evas_preload_thread_workers)
	  {
	     LKU(_mutex);
	     break;
	  }

	work = eina_list_data_get(_evas_preload_thread_workers);
	_evas_preload_thread_workers = eina_list_remove_list
           (_evas_preload_thread_workers, _evas_preload_thread_workers);
	LKU(_mutex);
	work->func_heavy(work->data);
	evas_async_events_put(pth, 0, work, _evas_preload_thread_done);
     }

   LKL(_mutex);
   if (_evas_preload_thread_workers)
     {
	LKU(_mutex);
	goto on_error;
     }
   _evas_preload_thread_count--;

   LKU(_mutex);
   return pth;
}
#endif

void
_evas_preload_thread_init(void)
{
   _evas_preload_thread_count_max = eina_cpu_count();
   if (_evas_preload_thread_count_max <= 0)
      _evas_preload_thread_count_max = 1;
}

void
_evas_preload_thread_shutdown(void)
{
   /* FIXME: If function are still running in the background, should we kill them ? */
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Preload_Pthread_Worker *work;
   Evas_Preload_Pthread_Data *pth;

   /* Force processing of async events. */
   evas_async_events_process();

   LKL(_mutex);

   EINA_LIST_FREE(_evas_preload_thread_workers, work)
     {
	if (work->func_cancel) work->func_cancel(work->data);
	free(work);
     }
   
   LKU(_mutex);
   
   EINA_LIST_FREE(_evas_preload_thread, pth)
     {
	Evas_Preload_Pthread_Data *p;
        
	pthread_cancel(pth->thread);
	pthread_join(pth->thread, (void **)&p);
        free(p);
     }
#endif
}

Evas_Preload_Pthread_Worker *
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
   _evas_preload_thread_workers = eina_list_append
      (_evas_preload_thread_workers, work);

   if (_evas_preload_thread_count == _evas_preload_thread_count_max)
     {
        LKU(_mutex);
	return work;
     }

   LKU(_mutex);

   /* One more thread could be created. */
   pth = malloc(sizeof(Evas_Preload_Pthread_Data));
   if (!pth) goto on_error;

   if (!pthread_create(&pth->thread, NULL, _evas_preload_thread_worker, pth))
     {
#ifdef __linux__
        struct sched_param param;

        /* lower priority of async loader threads so they use up "bg cpu"
         * as it was really intended to work.
         * yes - this may fail if not root. there is no portable way to do
         * this so try - if it fails. meh. nothnig to be done.
         */
        memset(&param, 0, sizeof(param));
        param.sched_priority = sched_get_priority_max(SCHED_RR);
        if (pthread_setschedparam(pth->thread, SCHED_RR, &param) != 0)
          {
             int newp;

             errno = 0;
             newp = getpriority(PRIO_PROCESS, 0);
             if (errno == 0)
               {
                  newp += 5;
                  if (newp > 19) newp = 19;
                  setpriority(PRIO_PROCESS, pth->thread, newp);
               }
          }
#endif

	LKL(_mutex);
	_evas_preload_thread_count++;
	LKU(_mutex);
	return work;
     }

 on_error:
   if (_evas_preload_thread_count == 0)
     {
	if (work->func_cancel) work->func_cancel(work->data);
	free(work);
     }
   return NULL;
#else
   /*
     If no thread and as we don't want to break app that rely on this
     facility, we will lock the interface until we are done.
    */
   func_heavy(data);
   func_end(data);

   return NULL;
#endif
}

Eina_Bool
evas_preload_thread_cancel(Evas_Preload_Pthread_Worker *wk)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Preload_Pthread_Worker *work;
   Eina_List *l;

   if (!wk) return EINA_TRUE;
   
   LKL(_mutex);
   EINA_LIST_FOREACH(_evas_preload_thread_workers, l, work)
     {
        if (work != wk) continue;

        _evas_preload_thread_workers = eina_list_remove_list
           (_evas_preload_thread_workers, l);
        LKU(_mutex);
        if (work->func_cancel) work->func_cancel(work->data);
        free(work);
        return EINA_TRUE;
     }

   LKU(_mutex);

   /* Delay the destruction */
   wk->cancel = EINA_TRUE;
   return EINA_FALSE;
#else
   return EINA_TRUE;
#endif
}
