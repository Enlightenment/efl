#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_job_private.h"
#include "Ecore_Job.h"

static void _ecore_job_free(void *data, void *ev);
static int _ecore_job_event_handler(void *data, int type, void *ev);
    
static int ecore_event_job_type = 0;

/**
 * Add a job to the event queue
 * @param func The function to be called when the job gets handled
 * @param data The data to be passed to the job function
 * @return A job handle
 * 
 * Add a job to the queue to be executed by the event system later on and
 * return a pointer to the job handle. When the job is to be executed, the 
 * function @p func is called and passed the pointer @p data. once the job has
 * been executed the job handle is no longer valid.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Job *
ecore_job_add(void (*func) (void *data), const void *data)
{
   Ecore_Job *job;
   
   if (!func) return NULL;
   if (!ecore_event_job_type)
     {
	ecore_event_job_type = ecore_event_type_new();
	ecore_event_handler_add(ecore_event_job_type, _ecore_job_event_handler, NULL);
     }
   job = calloc(1, sizeof(Ecore_Job));
   if (!job) return NULL;
   ECORE_MAGIC_SET(job, ECORE_MAGIC_JOB);
   job->event = ecore_event_add(ecore_event_job_type, job, _ecore_job_free, NULL);
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
 * Delete a queued job
 * @param job The job to delete
 * @return The data pointer to be passed to the job
 * 
 * This removed a job from the queue (if it hasn't been executed yet) and
 * returns the data pointer that was to be passed to it.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
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

static void
_ecore_job_free(void *data, void *ev)
{
   Ecore_Job *job;
   
   job = ev;
   free(job);
}

static int
_ecore_job_event_handler(void *data, int type, void *ev)
{
   Ecore_Job *job;
   
   job = ev;
   job->func(job->data);
   return 0;
}
