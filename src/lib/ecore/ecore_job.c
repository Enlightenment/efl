#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>

#include "Ecore.h"
#include "ecore_private.h"

static Eina_Bool _ecore_job_event_handler(void *data,
                                          int   type,
                                          void *ev);
static void _ecore_job_event_free(void *data,
                                  void *ev);

static int ecore_event_job_type = 0;
static Ecore_Event_Handler *_ecore_job_handler = NULL;

struct _Ecore_Job
{
   Ecore_Event *event;
   Ecore_Cb     func;
   void        *data;
};

void
_ecore_job_init(void)
{
   ecore_event_job_type = ecore_event_type_new();
   _ecore_job_handler = ecore_event_handler_add(ecore_event_job_type, _ecore_job_event_handler, NULL);
}

void
_ecore_job_shutdown(void)
{
   ecore_event_handler_del(_ecore_job_handler);
   _ecore_job_handler = NULL;
}

EAPI Ecore_Job *
ecore_job_add(Ecore_Cb    func,
              const void *data)
{
   Ecore_Job *job;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   if (!func)
     {
        ERR("Callback function must be set up for an Ecore_Job object");
        return NULL;
     }

   job = calloc(1, sizeof (Ecore_Job));
   if (!job) return NULL;

   job->event = ecore_event_add(ecore_event_job_type, job, _ecore_job_event_free, job);
   if (!job->event)
     {
        ERR("No event was assigned to Ecore_Job '%p'", job);
        free(job);
        return NULL;
     }
   job->func = func;
   job->data = (void *)data;

   return job;
}

EAPI void *
ecore_job_del(Ecore_Job *job)
{
   void *data;

   if (!job) return NULL;

   EINA_MAIN_LOOP_CHECK_RETURN_VAL(NULL);

   data = job->data;
   ecore_event_del(job->event);

   return data;
}

static Eina_Bool
_ecore_job_event_handler(void *data EINA_UNUSED,
                         int   type EINA_UNUSED,
                         void *ev)
{
   Ecore_Job *job;

   job = ev;
   job->func(job->data);
   return ECORE_CALLBACK_DONE;
}

static void
_ecore_job_event_free(void *data,
                      void *job EINA_UNUSED)
{
   free(data);
}
