#ifndef _ECORE_JOB_PRIVATE_H
#define _ECORE_JOB_PRIVATE_H

#define ECORE_MAGIC_JOB             0x76543210

extern int _ecore_job_log_dom;
#ifdef ECORE_JOB_DEFAULT_LOG_COLOR
# undef ECORE_JOB_DEFAULT_LOG_COLOR
#endif
#define ECORE_JOB_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_job_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_job_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_job_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_job_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_job_log_dom, __VA_ARGS__)

typedef struct _Ecore_Job Ecore_Job;

struct _Ecore_Job
{
   ECORE_MAGIC;
   Ecore_Event  *event;
   void        (*func) (void *data);
   void         *data;
};

#endif
