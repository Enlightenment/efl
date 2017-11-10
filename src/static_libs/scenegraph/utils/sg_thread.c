#include"sg_thread.h"

static void
thread_receive_event(SG_Thread *obj EINA_UNUSED, SG_Thread_Event *e, Ecore_Thread *thread EINA_UNUSED)
{
   printf("message type %d processed callback received \n", e->type);
}
static void
thread_feedback_cb(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED, void *msg)
{
   SG_Thread *obj = data;
   SG_Thread_Event *e = msg;

   obj->fn->receive_event(obj, e, thread);
}

static void
thread_callback(SG_Thread *obj EINA_UNUSED, SG_Thread_Event *e, Ecore_Thread *thread)
{
   if (ecore_thread_check(thread)) return;

   if (e->callback)
     {
        SG_Thread_Event *e_copy = malloc(sizeof(SG_Thread_Event));

        e_copy->type   = e->type;
        e_copy->result = e->result;
        e_copy->data   = e->data;
        e_copy->callback = e->callback;
        ecore_thread_feedback(thread, e_copy);
     }
}

static void
thread_process_event(SG_Thread *obj, SG_Thread_Event *e, Ecore_Thread *thread)
{
   switch (e->type)
      {
         default:
           printf("Message type %d processed \n", e->type);
           break;
      }
   thread_callback(obj, e, thread);
}

static void
thread_post_event(SG_Thread *obj, SG_Thread_Event *e)
{
   void *ref;
   SG_Thread_Event *e_copy;

   if (!obj->m_queue) return;

   e_copy = eina_thread_queue_send(obj->m_queue, sizeof (SG_Thread_Event), &ref);
   e_copy->type = e->type;
   e_copy->data = e->data;
   e_copy->callback = e->callback;
   eina_thread_queue_send_done(obj->m_queue, ref);
}

static void
thread_exec(SG_Thread *obj, Ecore_Thread *thread)
{
   Eina_Thread_Queue *queue = obj->m_queue;

   for (;;)
     {
        void *ref;
        SG_Thread_Event *e, e_copy;

        if (ecore_thread_check(thread))
          {
             break;
          }
        // wait for message to avilable
        e = eina_thread_queue_wait(queue, &ref);

        if (!e) break;

        // copy the message
        e_copy.type = e->type;
        e_copy.data = e->data;
        e_copy.callback = e->callback;
        eina_thread_queue_wait_done(queue, ref);

        // if the thread is canceled
        if (ecore_thread_check(thread))
         {
            break;
         }

        // do message processing
        obj->fn->process_event(obj, &e_copy, thread);
     }
}

static void
thread_run(SG_Thread *obj, Ecore_Thread *thread)
{
   thread_exec(obj, thread);
}

static void
thread_run_cb(void *data, Ecore_Thread *thread)
{
   SG_Thread *obj = data;

   obj->fn->run(obj, thread);
}

static void
thread_cancel(SG_Thread *obj)
{
   if (obj->m_thread)
     ecore_thread_cancel(obj->m_thread);
   // free the message queue
  if (obj->m_queue)
    eina_thread_queue_free(obj->m_queue);
   obj->m_thread = NULL;
   obj->m_queue = NULL;
}

static void
thread_start(SG_Thread* obj)
{
   if (obj->m_thread) return;
   obj->m_queue = eina_thread_queue_new();
   obj->m_thread = ecore_thread_feedback_run(thread_run_cb, thread_feedback_cb, NULL, NULL,
                                             obj, EINA_TRUE);
}

static void
thread_dtr(SG_Thread* obj)
{
   thread_cancel(obj);
}

static SG_Thread_Func SG_THREAD_VTABLE =
{
   thread_dtr,
   thread_cancel,
   thread_start,
   thread_post_event,
   thread_run,
   thread_process_event,
   thread_receive_event,
   thread_callback
};

void
sg_thread_ctr(SG_Thread* obj)
{
   obj->fn = &SG_THREAD_VTABLE;
}

SG_Thread* sg_thread_create(void)
{
   SG_Thread* obj = calloc(1, sizeof(SG_Thread));
   sg_thread_ctr(obj);

   return obj;
}

void sg_thread_destroy(SG_Thread *obj)
{
   if (obj)
     {
        obj->fn->dtr(obj);
        free(obj);
     }
}
