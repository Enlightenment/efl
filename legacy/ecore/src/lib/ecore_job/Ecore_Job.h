#ifndef _ECORE_JOB_H
#define _ECORE_JOB_H

#ifdef __cplusplus
extern "C" {
#endif
   
#ifndef _ECORE_JOB_PRIVATE_H
typedef void Ecore_Job; /**< A job handle */
#endif

Ecore_Job *ecore_job_add(void (*func) (void *data), const void *data);    
void      *ecore_job_del(Ecore_Job *job);

#ifdef __cplusplus
}
#endif

#endif
