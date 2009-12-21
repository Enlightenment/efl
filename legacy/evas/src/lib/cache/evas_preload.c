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
typedef struct _Evas_Preload_Pthread_Worker Evas_Preload_Pthread_Worker;
typedef struct _Evas_Preload_Pthread_Data Evas_Preload_Pthread_Data;

struct _Evas_Preload_Pthread_Worker
{
   void (*func_heavy)(void *data);
   void (*func_end)(void *data);
   void (*func_cancel)(void *data);

   const void *data;

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
static Eina_List *_evas_preload_thread_data = NULL;
static Eina_List *_evas_preload_thread = NULL;

static LK(_mutex) = PTHREAD_MUTEX_INITIALIZER;

static void
_evas_preload_thread_end(Evas_Preload_Pthread_Data *pth)
{
   Evas_Preload_Pthread_Data *p;

   if (pthread_join(pth->thread, (void**) &p) != 0)
     return ;

   _evas_preload_thread = eina_list_remove(_evas_preload_thread, pth);
}

static void
_evas_preload_thread_done(void *target, Evas_Callback_Type type, void *event_info)
{
   Evas_Preload_Pthread_Worker *work;

   work = event_info;

   if (work->cancel)
     {
	if (work->func_cancel)
	  work->func_cancel((void*) work->data);
     }
   else
     {
	work->func_end((void*) work->data);
     }

   free(work);
}

static void *
_evas_preload_thread_worker(Evas_Preload_Pthread_Data *pth)
{
   Evas_Preload_Pthread_Worker *work;

   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

 on_error:

   while (_evas_preload_thread_data)
     {
	LKL(_mutex);

	if (!_evas_preload_thread_data)
	  {
	     LKU(_mutex);
	     break;
	  }

	work = eina_list_data_get(_evas_preload_thread_data);
	_evas_preload_thread_data = eina_list_remove_list(_evas_preload_thread_data, _evas_preload_thread_data);

	LKU(_mutex);

	work->func_heavy((void*) work->data);

	evas_async_events_put(pth, 0, work, _evas_preload_thread_done);
     }

   LKL(_mutex);
   if (_evas_preload_thread_data)
     {
	LKU(_mutex);
	goto on_error;
     }
   _evas_preload_thread_count--;

   LKU(_mutex);

   work = malloc(sizeof (Evas_Preload_Pthread_Worker));
   if (!work) return NULL;

   work->data = pth;
   work->func_heavy = NULL;
   work->func_end = (void*) _evas_preload_thread_end;
   work->func_cancel = NULL;
   work->cancel = EINA_FALSE;

   evas_async_events_put(pth, 0, work, _evas_preload_thread_done);

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

   EINA_LIST_FREE(_evas_preload_thread_data, work)
     {
	if (work->func_cancel)
	  work->func_cancel((void*)work->data);
	free(work);
     }

   LKU(_mutex);

   EINA_LIST_FREE(_evas_preload_thread, pth)
     {
	Evas_Preload_Pthread_Data *p;

	pthread_cancel(pth->thread);
	pthread_join(pth->thread, (void **) &p);
     }
#endif
}

Evas_Preload_Pthread *
evas_preload_thread_run(void (*func_heavy)(void *data),
			void (*func_end)(void *data),
			void (*func_cancel)(void *data),
			const void *data)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Preload_Pthread_Worker *work;
   Evas_Preload_Pthread_Data *pth;

   work = malloc(sizeof (Evas_Preload_Pthread_Worker));
   if (!work)
     {
	func_cancel(data);
	return NULL;
     }

   work->func_heavy = func_heavy;
   work->func_end = func_end;
   work->func_cancel = func_cancel;
   work->cancel = EINA_FALSE;
   work->data = data;

   LKL(_mutex);
   _evas_preload_thread_data = eina_list_append(_evas_preload_thread_data, work);

   if (_evas_preload_thread_count == _evas_preload_thread_count_max)
     {
	pthread_mutex_unlock(&_mutex);
	return (Evas_Preload_Pthread*) work;
     }

   LKU(_mutex);

   /* One more thread could be created. */
   pth = malloc(sizeof (Evas_Preload_Pthread_Data));
   if (!pth)
     goto on_error;

   if (pthread_create(&pth->thread, NULL, (void*) _evas_preload_thread_worker, pth) == 0)
     {
	LKL(_mutex);
	_evas_preload_thread_count++;
	LKU(_mutex);
	return (Evas_Preload_Pthread*) work;
     }

 on_error:
   if (_evas_preload_thread_count == 0)
     {
	if (work->func_cancel)
	  work->func_cancel((void*) work->data);
	free(work);
     }
   return NULL;
#else
   /*
     If no thread and as we don't want to break app that rely on this
     facility, we will lock the interface until we are done.
    */
   func_heavy((void*) data);
   func_end((void*) data);

   return EINA_TRUE;
#endif
}

Eina_Bool
evas_preload_thread_cancel(Evas_Preload_Pthread *thread)
{
#ifdef BUILD_ASYNC_PRELOAD
   Evas_Preload_Pthread_Worker *work;
   Eina_List *l;

   LKL(_mutex);

   EINA_LIST_FOREACH(_evas_preload_thread_data, l, work)
     if ((void*) work == (void*) thread)
       {
	  _evas_preload_thread_data = eina_list_remove_list(_evas_preload_thread_data, l);

	  LKU(_mutex);

	  if (work->func_cancel)
	    work->func_cancel((void*) work->data);
	  free(work);

	  return EINA_TRUE;
       }

   LKU(_mutex);

   /* Delay the destruction */
   work = (Evas_Preload_Pthread_Worker *) thread;
   work->cancel = EINA_TRUE;
   return EINA_FALSE;
#else
   return EINA_TRUE;
#endif
}
