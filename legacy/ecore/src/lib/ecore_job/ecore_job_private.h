#ifndef _ECORE_JOB_PRIVATE_H
#define _ECORE_JOB_PRIVATE_H

#define ECORE_MAGIC_JOB             0x76543210

typedef struct _Ecore_Job Ecore_Job;

struct _Ecore_Job
{
   ECORE_MAGIC;
   Ecore_Event  *event;
   void        (*func) (void *data);
   void         *data;
};

#endif
