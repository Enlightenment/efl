#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <pthread.h>

#include "ecore_private.h"
#include "Ecore.h"

typedef struct _Ecore_Pthread_Worker Ecore_Pthread_Worker;
typedef struct _Ecore_Pthread Ecore_Pthread;

struct _Ecore_Pthread_Worker
{
   void (*func_heavy)(void *data);
   void (*func_end)(void *data);

   const void *data;
};

static int _ecore_thread_count_max = 0;
static int _ecore_thread_count = 0;

static int _ecore_thread_init = 0;
static Eina_List *_ecore_thread = NULL;
static int ECORE_THREAD_PIPE_DEL = 0;
static Ecore_Event_Handler *del_handler = NULL;

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef BUILD_PTHREAD
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
_ecore_thread_end(pthread_t *thread)
{
   Ecore_Pipe *p;

   if (pthread_join(*thread, (void**) &p) != 0)
     return ;

   ecore_event_add(ECORE_THREAD_PIPE_DEL, p, _ecore_thread_pipe_free, NULL);
}

static void *
_ecore_thread_worker(Ecore_Pipe *p)
{
   Ecore_Pthread_Worker *work;
   pthread_t *pth;

   pthread_mutex_lock(&_mutex);
   _ecore_thread_count++;
   pthread_mutex_unlock(&_mutex);

 on_error:

   while (_ecore_thread)
     {
	pthread_mutex_lock(&_mutex);

	if (!_ecore_thread)
	  {
	     pthread_mutex_unlock(&_mutex);
	     break;
	  }

	work = eina_list_data_get(_ecore_thread);
	_ecore_thread = eina_list_remove_list(_ecore_thread, _ecore_thread);

	pthread_mutex_unlock(&_mutex);

	work->func_heavy((void*) work->data);

	ecore_pipe_write(p, &work, sizeof (Ecore_Pthread_Worker*));
     }

   pthread_mutex_lock(&_mutex);
   if (_ecore_thread)
     {
	pthread_mutex_unlock(&_mutex);
	goto on_error;
     }
   _ecore_thread_count--;

   pthread_mutex_unlock(&_mutex);

   work = malloc(sizeof (Ecore_Pthread_Worker) + sizeof (pthread_t));
   if (!work) return NULL;

   work->data = (void*) (work + 1);
   work->func_heavy = NULL;
   work->func_end = (void*) _ecore_thread_end;

   pth = (void*) work->data;
   *pth = pthread_self();

   ecore_pipe_write(p, &work, sizeof (Ecore_Pthread_Worker*));

   return p;
}

static void
_ecore_thread_handler(void *data __UNUSED__, void *buffer, unsigned int nbyte)
{
   Ecore_Pthread_Worker *work;

   if (nbyte != sizeof (Ecore_Pthread_Worker*)) return ;

   work = *(Ecore_Pthread_Worker**)buffer;

   work->func_end((void*) work->data);

   free(work);
}
#endif

int
ecore_thread_init(void)
{
   _ecore_thread_init++;

   if (_ecore_thread_init > 1) return _ecore_thread_init;

   _ecore_thread_count_max = eina_cpu_count();
   if (_ecore_thread_count_max <= 0)
     _ecore_thread_count_max = 1;

   ECORE_THREAD_PIPE_DEL = ecore_event_type_new();
#ifdef BUILD_PTHREAD
   del_handler = ecore_event_handler_add(ECORE_THREAD_PIPE_DEL, _ecore_thread_pipe_del, NULL);
#endif
   return _ecore_thread_init;
}

int
ecore_thread_shutdown(void)
{
   _ecore_thread_init--;

   if (!_ecore_thread_init)
     {
	/* FIXME: If function are still running in the background, should we kill them ? */
	ecore_event_handler_del(del_handler);
	del_handler = NULL;
     }

   return _ecore_thread_init;
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
EAPI Eina_Bool
ecore_thread_run(void (*func_heavy)(void *data),
		  void (*func_end)(void *data),
		  const void *data)
{
#ifdef BUILD_PTHREAD
   Ecore_Pthread_Worker *work;
   Ecore_Pipe *p;
   pthread_t thread;

   work = malloc(sizeof (Ecore_Pthread_Worker));
   if (!work) return EINA_FALSE;

   work->func_heavy = func_heavy;
   work->func_end = func_end;
   work->data = data;

   pthread_mutex_lock(&_mutex);
   _ecore_thread = eina_list_append(_ecore_thread, work);

   if (_ecore_thread_count == _ecore_thread_count_max)
     {
	pthread_mutex_unlock(&_mutex);
	return EINA_TRUE;
     }

   pthread_mutex_unlock(&_mutex);

   /* One more thread could be created. */
   p = ecore_pipe_add(_ecore_thread_handler, NULL);

   if (pthread_create(&thread, NULL, (void*) _ecore_thread_worker, p) == 0)
     return EINA_TRUE;

   return EINA_FALSE;
#else
   /*
     If no thread and as we don't want to break app that rely on this
     facility, we will lock the interface until we are done.
    */
   func_heavy(data);
   func_end(data);

   return EINA_TRUE;
#endif
}

