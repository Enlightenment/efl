#ifndef SG_THREAD_H
#define SG_THREAD_H

#include <Eina.h>
#include <Ecore.h>

typedef struct _SG_Thread_Func                  SG_Thread_Func;
typedef struct _SG_Thread                       SG_Thread;
typedef struct _SG_Thread_Event                 SG_Thread_Event;

// enable sub classing
void sg_thread_ctr(SG_Thread* obj);

// api
SG_Thread   *sg_thread_create(void);
void         sg_thread_destroy(SG_Thread *);


struct _SG_Thread_Event
{
   Eina_Thread_Queue_Msg  header; // ignore
   int                    type;
   Eina_Bool              callback;
   void                  *data;
   void                  *result;
};

struct _SG_Thread_Func
{
   void                (*dtr)(SG_Thread* obj);
   void                (*cancel)(SG_Thread* obj);
   void                (*start)(SG_Thread* obj);
   void                (*post_event)(SG_Thread* obj, SG_Thread_Event *e);
   void                (*run)(SG_Thread* obj, Ecore_Thread *thread); /* pure virtual */ // this function runs on the thread
   void                (*process_event)(SG_Thread* obj, SG_Thread_Event *e, Ecore_Thread *thread); /* pure virtual */ // this function runs on the thread
   void                (*receive_event)(SG_Thread* obj, SG_Thread_Event *e, Ecore_Thread *thread); /* pure virtual */ // this function runs on the caller thread
   void                (*callback)(SG_Thread* obj, SG_Thread_Event *e, Ecore_Thread *thread); /* private function only used by derive class */
};

#define SG_THREAD_CLASS               \
   SG_Thread_Func        *fn;         \
   Eina_Thread_Queue     *m_queue;    \
   Ecore_Thread          *m_thread;



struct _SG_Thread
{
   SG_THREAD_CLASS
};

#endif //SG_THREAD_H