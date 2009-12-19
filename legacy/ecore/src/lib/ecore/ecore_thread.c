#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef EFL_HAVE_PTHREAD
# include <pthread.h>
#endif

#include "ecore_private.h"
#include "Ecore.h"

#ifdef EFL_HAVE_PTHREAD
typedef struct _Ecore_Pthread_Worker Ecore_Pthread_Worker;
typedef struct _Ecore_Pthread_Data Ecore_Pthread_Data;
typedef struct _Ecore_Pthread Ecore_Pthread;

struct _Ecore_Pthread_Worker
{
   void (*func_heavy)(void *data);
   void (*func_end)(void *data);
   void (*func_cancel)(void *data);

   const void *data;

   Eina_Bool cancel : 1;
};

struct _Ecore_Pthread_Data
{
   Ecore_Pipe *p;
   pthread_t thread;
};
#endif

static int _ecore_thread_count_max = 0;
static int ECORE_THREAD_PIPE_DEL = 0;

#ifdef EFL_HAVE_PTHREAD
static int _ecore_thread_count = 0;
static Eina_List *_ecore_thread_data = NULL;
static Eina_List *_ecore_thread = NULL;
static Ecore_Event_Handler *del_handler = NULL;

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

static void
_ecore_thread_pipe_free(void *data __UNUSED__, void *event)
{
   Ecore_Pipe *p = event;

   ecore_pipe_del(p);
}

static int
_ecore_thread_pipe_del(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   /* This is a hack to delay pipe destruction until we are out of it's internal loop. */
   return 0;
}

static void
_ecore_thread_end(Ecore_Pthread_Data *pth)
{
   Ecore_Pipe *p;

   if (pthread_join(pth->thread, (void**) &p) != 0)
     return ;

   _ecore_thread = eina_list_remove(_ecore_thread, pth);

   ecore_event_add(ECORE_THREAD_PIPE_DEL, pth->p, _ecore_thread_pipe_free, NULL);
}

static void *
_ecore_thread_worker(Ecore_Pthread_Data *pth)
{
   Ecore_Pthread_Worker *work;

   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

   pthread_mutex_lock(&_mutex);
   _ecore_thread_count++;
   pthread_mutex_unlock(&_mutex);

 on_error:

   while (_ecore_thread_data)
     {
	pthread_mutex_lock(&_mutex);

	if (!_ecore_thread_data)
	  {
	     pthread_mutex_unlock(&_mutex);
	     break;
	  }

	work = eina_list_data_get(_ecore_thread_data);
	_ecore_thread_data = eina_list_remove_list(_ecore_thread_data, _ecore_thread_data);

	pthread_mutex_unlock(&_mutex);

	work->func_heavy((void*) work->data);

	ecore_pipe_write(pth->p, &work, sizeof (Ecore_Pthread_Worker*));
     }

   pthread_mutex_lock(&_mutex);
   if (_ecore_thread_data)
     {
	pthread_mutex_unlock(&_mutex);
	goto on_error;
     }
   _ecore_thread_count--;

   pthread_mutex_unlock(&_mutex);

   work = malloc(sizeof (Ecore_Pthread_Worker));
   if (!work) return NULL;

   work->data = pth;
   work->func_heavy = NULL;
   work->func_end = (void*) _ecore_thread_end;
   work->func_cancel = NULL;
   work->cancel = EINA_FALSE;

   ecore_pipe_write(pth->p, &work, sizeof (Ecore_Pthread_Worker*));

   return pth->p;
}

static void
_ecore_thread_handler(void *data __UNUSED__, void *buffer, unsigned int nbyte)
{
   Ecore_Pthread_Worker *work;

   if (nbyte != sizeof (Ecore_Pthread_Worker*)) return ;

   work = *(Ecore_Pthread_Worker**)buffer;

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
#endif

void
_ecore_thread_init(void)
{
   _ecore_thread_count_max = eina_cpu_count();
   if (_ecore_thread_count_max <= 0)
     _ecore_thread_count_max = 1;

   ECORE_THREAD_PIPE_DEL = ecore_event_type_new();
#ifdef EFL_HAVE_PTHREAD
   del_handler = ecore_event_handler_add(ECORE_THREAD_PIPE_DEL, _ecore_thread_pipe_del, NULL);
#endif
}

void
_ecore_thread_shutdown(void)
{
   /* FIXME: If function are still running in the background, should we kill them ? */
#ifdef EFL_HAVE_PTHREAD
   Ecore_Pthread_Worker *work;
   Ecore_Pthread_Data *pth;

   pthread_mutex_lock(&_mutex);

   EINA_LIST_FREE(_ecore_thread_data, work)
     {
	if (work->func_cancel)
	  work->func_cancel((void*)work->data);
	free(work);
     }

   pthread_mutex_unlock(&_mutex);

   EINA_LIST_FREE(_ecore_thread, pth)
     {
	Ecore_Pipe *p;

	pthread_cancel(pth->thread);
	pthread_join(pth->thread, (void **) &p);

	ecore_pipe_del(pth->p);
     }

   ecore_event_handler_del(del_handler);
   del_handler = NULL;
#endif
}

/*
 * ecore_thread_run provide a facility for easily managing heavy task in a
 * parallel thread. You should provide two function, the first one, func_heavy,
 * that will do the heavy work in another thread (so you should not use the
 * EFL in it except Eina if you are carefull), and the second one, func_end,
 * that will be called in Ecore main loop when func_heavy is done. So you
 * can use all the EFL inside this function.
 *
 * Be aware, that you can't make assumption on the result order of func_end
 * after many call to ecore_thread_run, as we start as much thread as the
 * host CPU can handle.
 */
EAPI Ecore_Thread *
ecore_thread_run(void (*func_heavy)(void *data),
		 void (*func_end)(void *data),
		 void (*func_cancel)(void *data),
		 const void *data)
{
#ifdef EFL_HAVE_PTHREAD
   Ecore_Pthread_Worker *work;
   Ecore_Pthread_Data *pth;

   work = malloc(sizeof (Ecore_Pthread_Worker));
   if (!work) return NULL;

   work->func_heavy = func_heavy;
   work->func_end = func_end;
   work->func_cancel = func_cancel;
   work->cancel = EINA_FALSE;
   work->data = data;

   pthread_mutex_lock(&_mutex);
   _ecore_thread_data = eina_list_append(_ecore_thread_data, work);

   if (_ecore_thread_count == _ecore_thread_count_max)
     {
	pthread_mutex_unlock(&_mutex);
	return (Ecore_Thread*) work;
     }

   pthread_mutex_unlock(&_mutex);

   /* One more thread could be created. */
   pth = malloc(sizeof (Ecore_Pthread_Data));
   if (!pth)
     goto on_error;

   pth->p = ecore_pipe_add(_ecore_thread_handler, NULL);

   if (pthread_create(&pth->thread, NULL, (void*) _ecore_thread_worker, pth) == 0)
     return (Ecore_Thread*) work;

 on_error:
   if (_ecore_thread_count == 0)
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
   func_heavy((void *)data);
   func_end((void *)data);

   return NULL;
#endif
}

/*
 * ecore_thread_cancel give the possibility to cancel a task still running. It
 * will return EINA_FALSE, if the destruction is delayed or EINA_TRUE if it is
 * cancelled after this call.
 *
 * You should use this function only in the main loop.
 *
 * func_end, func_cancel will destroy the handler, so don't use it after.
 * And if ecore_thread_cancel return EINA_TRUE, you should not use Ecore_Thread also.
 */
EAPI Eina_Bool
ecore_thread_cancel(Ecore_Thread *thread)
{
#ifdef EFL_HAVE_PTHREAD
   Ecore_Pthread_Worker *work;
   Eina_List *l;

   pthread_mutex_lock(&_mutex);

   EINA_LIST_FOREACH(_ecore_thread_data, l, work)
     if ((void*) work == (void*) thread)
       {
	  _ecore_thread_data = eina_list_remove_list(_ecore_thread_data, l);

	  if (work->func_cancel)
	    work->func_cancel((void*) work->data);
	  free(work);

	  return EINA_TRUE;
       }

   pthread_mutex_unlock(&_mutex);

   /* Delay the destruction */
   work->cancel = EINA_TRUE;
   return EINA_FALSE;
#else
   return EINA_FALSE;
#endif
}
