#ifndef _ECORE_JOB_H
#define _ECORE_JOB_H

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

/**
 * @file
 * @brief Functions for dealing with Ecore jobs.
 */

#ifdef __cplusplus
extern "C" {
#endif
   
#ifndef _ECORE_JOB_PRIVATE_H
typedef void Ecore_Job; /**< A job handle */
#endif

EAPI Ecore_Job *ecore_job_add(void (*func) (void *data), const void *data);    
EAPI void      *ecore_job_del(Ecore_Job *job);

#ifdef __cplusplus
}
#endif

#endif
