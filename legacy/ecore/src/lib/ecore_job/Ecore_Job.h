/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _ECORE_JOB_H
#define _ECORE_JOB_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_JOB_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_ECORE_JOB_BUILD */
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
#endif /* ! _WIN32 */

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

EAPI int        ecore_job_init(void);
EAPI int        ecore_job_shutdown(void);
EAPI Ecore_Job *ecore_job_add(void (*func) (void *data), const void *data);
EAPI void      *ecore_job_del(Ecore_Job *job);

#ifdef __cplusplus
}
#endif

#endif
