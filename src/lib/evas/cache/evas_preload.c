#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#ifdef _WIN32
# include <Evil.h>
#endif
#ifdef __linux__
# include <sys/syscall.h>
#endif
#include "evas_common_private.h"
#include "evas_private.h"
#include "Evas.h"

#include "Ecore.h"

typedef struct _Evas_Preload_Pthread Evas_Preload_Pthread;
typedef void (*_evas_preload_pthread_func)(void *data);

struct _Evas_Preload_Pthread
{
   EINA_INLIST;

   Ecore_Thread *thread;

   _evas_preload_pthread_func func_heavy;
   _evas_preload_pthread_func func_end;
   _evas_preload_pthread_func func_cancel;
   void *data;
};

static Eina_Inlist *works = NULL;

static void
_evas_preload_thread_work_free(Evas_Preload_Pthread *work)
{
   works = eina_inlist_remove(works, EINA_INLIST_GET(work));

   free(work);
}

static void
_evas_preload_thread_success(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Evas_Preload_Pthread *work = data;

   work->func_end(work->data);

    _evas_preload_thread_work_free(work);
}

static void
_evas_preload_thread_fail(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Evas_Preload_Pthread *work = data;

   if (work->func_cancel) work->func_cancel(work->data);

   _evas_preload_thread_work_free(work);
}

static void
_evas_preload_thread_worker(void *data, Ecore_Thread *thread)
{
   Evas_Preload_Pthread *work = data;

   work->thread = thread;

   work->func_heavy(work->data);
}

void
_evas_preload_thread_init(void)
{
}

void
_evas_preload_thread_shutdown(void)
{
   Evas_Preload_Pthread *work;

   EINA_INLIST_FOREACH(works, work)
     ecore_thread_cancel(work->thread);

   while (works)
     {
        work = (Evas_Preload_Pthread*) works;
        if (!ecore_thread_wait(work->thread, 1))
          {
             ERR("Can not wait any longer on Evas thread to be done during shutdown. This might lead to a crash.");
             works = eina_inlist_remove(works, works);
          }
     }
}

Evas_Preload_Pthread *
evas_preload_thread_run(void (*func_heavy) (void *data),
                        void (*func_end) (void *data),
                        void (*func_cancel) (void *data),
                        const void *data)
{
   Evas_Preload_Pthread *work;

   work = malloc(sizeof(Evas_Preload_Pthread));
   if (!work)
     {
        func_cancel((void *)data);
        return NULL;
     }

   work->func_heavy = func_heavy;
   work->func_end = func_end;
   work->func_cancel = func_cancel;
   work->data = (void *)data;

   work->thread = ecore_thread_run(_evas_preload_thread_worker,
                                   _evas_preload_thread_success,
                                   _evas_preload_thread_fail,
                                   work);
   if (!work->thread)
     return NULL;

   works = eina_inlist_prepend(works, EINA_INLIST_GET(work));

   return work;
}

Eina_Bool
evas_preload_thread_cancel(Evas_Preload_Pthread *work)
{
   return ecore_thread_cancel(work->thread);
}

Eina_Bool
evas_preload_thread_cancelled_is(Evas_Preload_Pthread *work)
{
   if (!work) return EINA_FALSE;
   return ecore_thread_check(work->thread);
}

Eina_Bool
evas_preload_pthread_wait(Evas_Preload_Pthread *work, double wait)
{
   Eina_Bool r;

   if (!work) return EINA_TRUE;

   ecore_thread_main_loop_begin();
   r = ecore_thread_wait(work->thread, wait);
   ecore_thread_main_loop_end();

   return r;
}
