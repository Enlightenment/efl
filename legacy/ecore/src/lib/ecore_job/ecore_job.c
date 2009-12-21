/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_job_private.h"
#include "Ecore_Job.h"

int _ecore_job_log_dom = -1;
static int _ecore_job_event_handler(void *data, int type, void *ev);
static void _ecore_job_event_free(void *data, void *ev);
    
static int ecore_event_job_type = 0;
static int _ecore_job_init_count = 0;
static Ecore_Event_Handler* _ecore_job_handler = NULL;

EAPI int
ecore_job_init(void)
{
   if (++_ecore_job_init_count != 1)
     return _ecore_job_init_count;
   _ecore_job_log_dom = eina_log_domain_register("EcoreJob", ECORE_DEFAULT_LOG_COLOR);
   if(_ecore_job_log_dom < 0)
     {
       EINA_LOG_ERR("Impossible to create a log domain for the Ecore Job module.");
       return _ecore_job_init_count--;
     }
   if (!ecore_init())
     return --_ecore_job_init_count;

   ecore_event_job_type = ecore_event_type_new();
   _ecore_job_handler = ecore_event_handler_add(ecore_event_job_type, _ecore_job_event_handler, NULL);

   return _ecore_job_init_count;
}

EAPI int
ecore_job_shutdown(void)
{
   if (--_ecore_job_init_count != 0)
     return _ecore_job_init_count;

   ecore_event_handler_del(_ecore_job_handler);
   _ecore_job_handler = NULL;
   eina_log_domain_unregister(_ecore_job_log_dom);
   _ecore_job_log_dom = -1;
   ecore_shutdown();
   
   return _ecore_job_init_count;
}

/**
 * Add a job to the event queue.
 * @param   func The function to call when the job gets handled.
 * @param   data Data pointer to be passed to the job function when the job is
 *               handled.
 * @return  The handle of the job.  @c NULL is returned if the job could not be
 *          added to the queue.
 * @ingroup Ecore_Job_Group
 * @note    Once the job has been executed, the job handle is invalid.
 */
EAPI Ecore_Job *
ecore_job_add(void (*func) (void *data), const void *data)
{
   Ecore_Job *job;
   
   if (!func) return NULL;

   job = calloc(1, sizeof(Ecore_Job));
   if (!job) return NULL;
   ECORE_MAGIC_SET(job, ECORE_MAGIC_JOB);
   job->event = ecore_event_add(ecore_event_job_type, job, _ecore_job_event_free, NULL);
   if (!job->event)
     {
	free(job);
	return NULL;
     }
   job->func = func;
   job->data = (void *)data;
   return job;
}

/**
 * Delete a queued job that has not yet been executed.
 * @param   job  Handle of the job to delete.
 * @return  The data pointer that was to be passed to the job.
 * @ingroup Ecore_Job_Group
 */
EAPI void *
ecore_job_del(Ecore_Job *job)
{
   void *data;
   
   if (!ECORE_MAGIC_CHECK(job, ECORE_MAGIC_JOB))
     {
	ECORE_MAGIC_FAIL(job, ECORE_MAGIC_JOB,
			 "ecore_job_del");
	return NULL;
     }
   data = job->data;
   ECORE_MAGIC_SET(job, ECORE_MAGIC_NONE);
   ecore_event_del(job->event);
   return data;
}

static int
_ecore_job_event_handler(void *data __UNUSED__, int type __UNUSED__, void *ev)
{
   Ecore_Job *job;
   
   job = ev;
   job->func(job->data);
   return 0;
}

static void
_ecore_job_event_free(void *data __UNUSED__, void *ev)
{
   free(ev);
}
