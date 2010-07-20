#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#ifdef EFL_HAVE_PTHREAD
# include <pthread.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"

typedef struct _Ecore_Pthread_Worker Ecore_Pthread_Worker;
typedef struct _Ecore_Pthread Ecore_Pthread;

struct _Ecore_Pthread_Worker
{
   union {
      struct {
	 void (*func_blocking)(void *data);
      } short_run;
      struct {
	 void (*func_heavy)(Ecore_Thread *thread, void *data);
	 void (*func_notify)(Ecore_Thread *thread, void *msg_data, void *data);

	 Ecore_Pipe *notify;

#ifdef EFL_HAVE_PTHREAD
	 pthread_t self;
#endif
      } long_run;
   } u;

   void (*func_cancel)(void *data);
   void (*func_end)(void *data);

   const void *data;

   Eina_Bool cancel : 1;
   Eina_Bool long_run : 1;
};

#ifdef EFL_HAVE_PTHREAD
typedef struct _Ecore_Pthread_Data Ecore_Pthread_Data;

struct _Ecore_Pthread_Data
{
   Ecore_Pipe *p;
   void *data;
   pthread_t thread;
};
#endif

static int _ecore_thread_count_max = 0;
static int ECORE_THREAD_PIPE_DEL = 0;

#ifdef EFL_HAVE_PTHREAD
static int _ecore_thread_count = 0;
static Eina_List *_ecore_thread = NULL;
static Eina_List *_ecore_thread_data = NULL;
static Eina_List *_ecore_long_thread_data = NULL;
static Ecore_Event_Handler *del_handler = NULL;

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;

static void
_ecore_thread_pipe_free(void *data __UNUSED__, void *event)
{
   Ecore_Pipe *p = event;

   ecore_pipe_del(p);
}

static Eina_Bool
_ecore_thread_pipe_del(void *data __UNUSED__, int type __UNUSED__, void *event __UNUSED__)
{
   /* This is a hack to delay pipe destruction until we are out of it's internal loop. */
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_thread_end(Ecore_Pthread_Data *pth)
{
   Ecore_Pipe *p;

   if (pthread_join(pth->thread, (void **) &p) != 0)
     return ;

   _ecore_thread = eina_list_remove(_ecore_thread, pth);

   ecore_event_add(ECORE_THREAD_PIPE_DEL, pth->p, _ecore_thread_pipe_free, NULL);
}

static void
_ecore_thread_handler(void *data __UNUSED__, void *buffer, unsigned int nbyte)
{
   Ecore_Pthread_Worker *work;

   if (nbyte != sizeof (Ecore_Pthread_Worker *)) return ;

   work = *(Ecore_Pthread_Worker **)buffer;

   if (work->cancel)
     {
	if (work->func_cancel)
	  work->func_cancel((void *) work->data);
     }
   else
     {
	if (work->func_end)
	  work->func_end((void *) work->data);
     }

   if (work->long_run) ecore_pipe_del(work->u.long_run.notify);
   free(work);
}

static void
_ecore_notify_handler(void *data, void *buffer, unsigned int nbyte)
{
   Ecore_Pthread_Worker *work = data;
   void *user_data;

   if (nbyte != sizeof (Ecore_Pthread_Worker *)) return ;

   user_data = *(void **)buffer;

   if (work->u.long_run.func_notify)
     work->u.long_run.func_notify((Ecore_Thread *) work, user_data, (void *) work->data);
}

static void
_ecore_short_job(Ecore_Pipe *end_pipe)
{
   Ecore_Pthread_Worker *work;

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

	work->u.short_run.func_blocking((void *) work->data);

	ecore_pipe_write(end_pipe, &work, sizeof (Ecore_Pthread_Worker *));
     }
}

static void
_ecore_long_job(Ecore_Pipe *end_pipe, pthread_t thread)
{
   Ecore_Pthread_Worker *work;

   while (_ecore_long_thread_data)
     {
	pthread_mutex_lock(&_mutex);

	if (!_ecore_long_thread_data)
	  {
	     pthread_mutex_unlock(&_mutex);
	     break;
	  }

	work = eina_list_data_get(_ecore_long_thread_data);
	_ecore_long_thread_data = eina_list_remove_list(_ecore_long_thread_data, _ecore_long_thread_data);

	pthread_mutex_unlock(&_mutex);

	work->u.long_run.self = thread;
	work->u.long_run.func_heavy((Ecore_Thread *) work, (void *) work->data);

	ecore_pipe_write(end_pipe, &work, sizeof (Ecore_Pthread_Worker *));
     }
}

static void *
_ecore_direct_worker(Ecore_Pthread_Worker *work)
{
   Ecore_Pthread_Data *pth;

   pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
   pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

   pth = malloc(sizeof (Ecore_Pthread_Data));
   if (!pth) return NULL;

   pth->p = ecore_pipe_add(_ecore_thread_handler, NULL);
   if (!pth->p)
     {
	free(pth);
	return NULL;
     }
   pth->thread = pthread_self();

   work->u.long_run.self = pth->thread;
   work->u.long_run.func_heavy((Ecore_Thread *) work, (void *) work->data);

   ecore_pipe_write(pth->p, &work, sizeof (Ecore_Pthread_Worker *));

   work = malloc(sizeof (Ecore_Pthread_Worker));
   if (!work)
     {
	ecore_pipe_del(pth->p);
	free(pth);
	return NULL;
     }

   work->data = pth;
   work->u.short_run.func_blocking = NULL;
   work->func_end = (void *) _ecore_thread_end;
   work->func_cancel = NULL;
   work->cancel = EINA_FALSE;
   work->long_run = EINA_FALSE;

   ecore_pipe_write(pth->p, &work, sizeof (Ecore_Pthread_Worker *));

   return pth->p;
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
   if (_ecore_thread_data) _ecore_short_job(pth->p);
   if (_ecore_long_thread_data) _ecore_long_job(pth->p, pth->thread);

   /* FIXME: Check if there is long running task todo, and switch to long run handler. */

   pthread_mutex_lock(&_mutex);
   if (_ecore_thread_data)
     {
	pthread_mutex_unlock(&_mutex);
	goto on_error;
     }
   if (_ecore_long_thread_data)
     {
	pthread_mutex_unlock(&_mutex);
	goto on_error;
     }

   _ecore_thread_count--;

   pthread_mutex_unlock(&_mutex);

   work = malloc(sizeof (Ecore_Pthread_Worker));
   if (!work) return NULL;

   work->data = pth;
   work->u.short_run.func_blocking = NULL;
   work->func_end = (void *) _ecore_thread_end;
   work->func_cancel = NULL;
   work->cancel = EINA_FALSE;
   work->long_run = EINA_FALSE;

   ecore_pipe_write(pth->p, &work, sizeof (Ecore_Pthread_Worker *));

   return pth->p;
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
	  work->func_cancel((void *)work->data);
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

/**
 * @brief Run some blocking code in a parrallel thread to avoid locking the main loop.
 * @param func_blocking The function that should run in another thread.
 * @param func_end The function that will be called in the main loop if the thread terminate correctly.
 * @param func_cancel The function that will be called in the main loop if the thread is cancelled.
 * @param data User context data to pass to all callback.
 * @return A reference to the newly created thread instance, or NULL if it failed.
 *
 * ecore_thread_run provide a facility for easily managing blocking task in a
 * parallel thread. You should provide three function. The first one, func_blocking,
 * that will do the blocking work in another thread (so you should not use the
 * EFL in it except Eina if you are carefull). The second one, func_end,
 * that will be called in Ecore main loop when func_blocking is done. So you
 * can use all the EFL inside this function. The last one, func_cancel, will
 * be called in the main loop if the thread is cancelled or could not run at all.
 *
 * Be aware, that you can't make assumption on the result order of func_end
 * after many call to ecore_thread_run, as we start as much thread as the
 * host CPU can handle.
 */
EAPI Ecore_Thread *
ecore_thread_run(void (*func_blocking)(void *data),
		 void (*func_end)(void *data),
		 void (*func_cancel)(void *data),
		 const void *data)
{
#ifdef EFL_HAVE_PTHREAD
   Ecore_Pthread_Worker *work;
   Ecore_Pthread_Data *pth = NULL;

   if (!func_blocking) return NULL;

   work = malloc(sizeof (Ecore_Pthread_Worker));
   if (!work)
     {
        func_cancel((void *) data);
	return NULL;
     }

   work->u.short_run.func_blocking = func_blocking;
   work->func_end = func_end;
   work->func_cancel = func_cancel;
   work->cancel = EINA_FALSE;
   work->long_run = EINA_FALSE;
   work->data = data;

   pthread_mutex_lock(&_mutex);
   _ecore_thread_data = eina_list_append(_ecore_thread_data, work);

   if (_ecore_thread_count == _ecore_thread_count_max)
     {
	pthread_mutex_unlock(&_mutex);
	return (Ecore_Thread *) work;
     }

   pthread_mutex_unlock(&_mutex);

   /* One more thread could be created. */
   pth = malloc(sizeof (Ecore_Pthread_Data));
   if (!pth) goto on_error;

   pth->p = ecore_pipe_add(_ecore_thread_handler, NULL);
   if (!pth->p) goto on_error;

   if (pthread_create(&pth->thread, NULL, (void *) _ecore_thread_worker, pth) == 0)
     return (Ecore_Thread *) work;

 on_error:
   if (pth)
     {
	if (pth->p) ecore_pipe_del(pth->p);
	free(pth);
     }

   if (_ecore_thread_count == 0)
     {
	if (work->func_cancel)
	  work->func_cancel((void *) work->data);
	free(work);
	work = NULL;
     }
   return (Ecore_Thread *) work;
#else
   /*
     If no thread and as we don't want to break app that rely on this
     facility, we will lock the interface until we are done.
    */
   func_blocking((void *)data);
   func_end((void *)data);

   return NULL;
#endif
}

/**
 * @brief Cancel a running thread.
 * @param thread The thread to cancel.
 * @return Will return EINA_TRUE if the thread has been cancelled,
 *         EINA_FALSE if it is pending.
 *
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
     if ((void *) work == (void *) thread)
       {
	  _ecore_thread_data = eina_list_remove_list(_ecore_thread_data, l);

	  pthread_mutex_unlock(&_mutex);

	  if (work->func_cancel)
	    work->func_cancel((void *) work->data);
	  free(work);

	  return EINA_TRUE;
       }

   pthread_mutex_unlock(&_mutex);

   /* Delay the destruction */
   ((Ecore_Pthread_Worker *)thread)->cancel = EINA_TRUE;
   return EINA_FALSE;
#else
   return EINA_TRUE;
#endif
}

/**
 * @brief Tell if a thread was canceled or not.
 * @param thread The thread to test.
 * @return EINA_TRUE if the thread is cancelled,
 *         EINA_FALSE if it is not.
 *
 * You can use this function in main loop and in the thread.
 */
EAPI Eina_Bool
ecore_thread_check(Ecore_Thread *thread)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *) thread;

   if (!worker) return EINA_TRUE;
   return worker->cancel;
}

/**
 * @brief Run some heavy code in a parrallel thread to avoid locking the main loop.
 * @param func_heavy The function that should run in another thread.
 * @param func_notify The function that will receive the data send by func_heavy in the main loop.
 * @param func_end The function that will be called in the main loop if the thread terminate correctly.
 * @param func_cancel The function that will be called in the main loop if the thread is cancelled.
 * @param data User context data to pass to all callback.
 * @param try_no_queue If you wan't to run outside of the thread pool.
 * @return A reference to the newly created thread instance, or NULL if it failed.
 *
 * ecore_long_run provide a facility for easily managing heavy task in a
 * parallel thread. You should provide four functions. The first one, func_heavy,
 * that will do the heavy work in another thread (so you should not use the
 * EFL in it except Eina and Eet if you are carefull). The second one, func_notify,
 * will receive the data send from the thread function (func_heavy) by ecore_thread_notify
 * in the main loop (and so, can use all the EFL). Tje third, func_end,
 * that will be called in Ecore main loop when func_heavy is done. So you
 * can use all the EFL inside this function. The last one, func_cancel, will
 * be called in the main loop also, if the thread is cancelled or could not run at all.
 *
 * Be aware, that you can't make assumption on the result order of func_end
 * after many call to ecore_long_run, as we start as much thread as the
 * host CPU can handle.
 *
 * If you set try_no_queue, it will try to run outside of the thread pool, this can bring
 * the CPU down, so be carefull with that. Of course if it can't start a new thread, it will
 * try to use one from the pool.
 */
EAPI Ecore_Thread *
ecore_long_run(void (*func_heavy)(Ecore_Thread *thread, void *data),
	       void (*func_notify)(Ecore_Thread *thread, void *msg_data, void *data),
	       void (*func_end)(void *data),
	       void (*func_cancel)(void *data),
	       const void *data,
	       Eina_Bool try_no_queue)
{

#ifdef EFL_HAVE_PTHREAD
   Ecore_Pthread_Worker *worker;
   Ecore_Pthread_Data *pth = NULL;

   if (!func_heavy) return NULL;

   worker = malloc(sizeof (Ecore_Pthread_Worker));
   if (!worker) goto on_error;

   worker->u.long_run.func_heavy = func_heavy;
   worker->u.long_run.func_notify = func_notify;
   worker->func_cancel = func_cancel;
   worker->func_end = func_end;
   worker->data = data;
   worker->cancel = EINA_FALSE;
   worker->long_run = EINA_TRUE;

   worker->u.long_run.notify = ecore_pipe_add(_ecore_notify_handler, worker);

   if (!try_no_queue)
     {
	pthread_t t;

	if (pthread_create(&t, NULL, (void *) _ecore_direct_worker, worker) == 0)
	  return (Ecore_Thread *) worker;
     }

   pthread_mutex_lock(&_mutex);
   _ecore_long_thread_data = eina_list_append(_ecore_long_thread_data, worker);

   if (_ecore_thread_count == _ecore_thread_count_max)
     {
	pthread_mutex_unlock(&_mutex);
	return (Ecore_Thread *) worker;
     }

   pthread_mutex_unlock(&_mutex);

   /* One more thread could be created. */
   pth = malloc(sizeof (Ecore_Pthread_Data));
   if (!pth) goto on_error;

   pth->p = ecore_pipe_add(_ecore_thread_handler, NULL);
   if (pth->p) goto on_error;

   if (pthread_create(&pth->thread, NULL, (void *) _ecore_thread_worker, pth) == 0)
     return (Ecore_Thread *) worker;

 on_error:
   if (pth)
     {
	if (pth->p) ecore_pipe_del(pth->p);
	free(pth);
     }

   if (_ecore_thread_count == 0)
     {
	if (func_cancel) func_cancel((void *) data);

	if (worker)
	  {
	     ecore_pipe_del(worker->u.long_run.notify);
	     free(worker);
	     worker = NULL;
	  }
     }

   return (Ecore_Thread *) worker;
#else
   Ecore_Pthread_Worker worker;

   (void) try_no_queue;

   /*
     If no thread and as we don't want to break app that rely on this
     facility, we will lock the interface until we are done.
    */
   worker.u.long_run.func_heavy = func_heavy;
   worker.u.long_run.func_notify = func_notify;
   worker.u.long_run.notify = NULL;
   worker.func_cancel = func_cancel;
   worker.func_end = func_end;
   worker.data = data;
   worker.cancel = EINA_FALSE;
   worker.long_run = EINA_TRUE;

   func_heavy((Ecore_Thread *) &worker, (void *)data);

   if (worker.cancel) func_cancel((void *)data);
   else func_end((void *)data);

   return NULL;
#endif
}

/**
 * @brief Send data to main loop from worker thread.
 * @param thread The current Ecore_Thread context to send data from
 * @param data Data to be transmitted to the main loop
 * @return EINA_TRUE if data was successfully send to main loop,
 *         EINA_FALSE if anything goes wrong.
 *
 * After a succesfull call, the data should be considered owned
 * by the main loop.
 *
 * You should use this function only in the func_heavy call.
 */
EAPI Eina_Bool
ecore_thread_notify(Ecore_Thread *thread, const void *data)
{
   Ecore_Pthread_Worker *worker = (Ecore_Pthread_Worker *) thread;

   if (!worker) return EINA_FALSE;
   if (!worker->long_run) return EINA_FALSE;

#ifdef EFL_HAVE_PTHREAD
   if (worker->u.long_run.self != pthread_self()) return EINA_FALSE;

   ecore_pipe_write(worker->u.long_run.notify, &data, sizeof (void *));

   return EINA_TRUE;
#else
   worker->u.long_run.func_notify(thread, (void*) data, (void*) worker->data);

   return EINA_TRUE;
#endif
}

