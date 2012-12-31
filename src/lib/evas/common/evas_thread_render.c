#include <assert.h>

#include "evas_common.h"

static Eina_Thread evas_thread_worker;
static Eina_Condition evas_thread_queue_condition;
static Eina_Lock evas_thread_queue_lock;
static Eina_Bool evas_thread_queue_ready = EINA_FALSE;
static Eina_Inarray evas_thread_queue;

static Evas_Thread_Command *evas_thread_queue_cache = NULL;
static int evas_thread_queue_cache_max = 0;

static volatile int evas_thread_exited = 0;
static Eina_Bool exit_thread = EINA_FALSE;
static int init_count = 0;

static void
evas_thread_queue_append(Evas_Thread_Command_Cb cb, void *data, Eina_Bool do_flush)
{
   Evas_Thread_Command *cmd;

   eina_lock_take(&evas_thread_queue_lock);

   if (evas_thread_queue.members == NULL)
     {
        evas_thread_queue.members = evas_thread_queue_cache;
        evas_thread_queue.len = 0;
        evas_thread_queue.max = evas_thread_queue_cache_max;
        evas_thread_queue_cache = NULL;
        evas_thread_queue_cache_max = 0;
     }

   cmd = eina_inarray_grow(&evas_thread_queue, 1);
   if (cmd)
     {
        cmd->cb = cb;
        cmd->data = data;
     }
   else
     {
        ERR("Out of memory allocating thread command.");
     }

   if (do_flush)
     {
        evas_thread_queue_ready = EINA_TRUE;
        eina_condition_signal(&evas_thread_queue_condition);
     }

   eina_lock_release(&evas_thread_queue_lock);
}

EAPI void
evas_thread_cmd_enqueue(Evas_Thread_Command_Cb cb, void *data)
{
    evas_thread_queue_append(cb, data, EINA_FALSE);
}

EAPI void
evas_thread_queue_flush(Evas_Thread_Command_Cb cb, void *data)
{
    evas_thread_queue_append(cb, data, EINA_TRUE);
}

static void*
evas_thread_worker_func(void *data EINA_UNUSED, Eina_Thread thread EINA_UNUSED)
{
    while (1)
      {
         Evas_Thread_Command *head;
         Evas_Thread_Command *cmd;
         int len;
         int max;

         eina_lock_take(&evas_thread_queue_lock);

         while (!evas_thread_queue_ready)
           {
              if (exit_thread)
                {
                   eina_lock_release(&evas_thread_queue_lock);
                   goto out;
                }
              eina_condition_wait(&evas_thread_queue_condition);
           }

         if (!eina_inarray_count(&evas_thread_queue))
           {
              ERR("Signaled to find an empty queue. BUG!");
              eina_lock_release(&evas_thread_queue_lock);
              continue;
           }

         head = evas_thread_queue.members;
         evas_thread_queue.members = NULL;
         max = evas_thread_queue.max; evas_thread_queue.max = 0;
         len = evas_thread_queue.len; evas_thread_queue.len = 0;
         
         evas_thread_queue_ready = EINA_FALSE;

         eina_lock_release(&evas_thread_queue_lock);

         cmd = head;
         while (len)
           {
              assert(cmd->cb);

              cmd->cb(cmd->data);

              cmd++;
              len--;
           }

         evas_thread_queue_cache = head;
         evas_thread_queue_cache_max = max;
      }

out:
    /* WRN: add a memory barrier or use a lock if we add more code here */
    evas_thread_exited = 1;
    return NULL;
}

void
evas_thread_init(void)
{
    if (init_count++) return;

    eina_threads_init();

    eina_inarray_step_set(&evas_thread_queue, sizeof (Eina_Inarray), sizeof (Evas_Thread_Command), 128);

    if (!eina_lock_new(&evas_thread_queue_lock))
      CRIT("Could not create draw thread lock");
    if (!eina_condition_new(&evas_thread_queue_condition, &evas_thread_queue_lock))
      CRIT("Could not create draw thread condition");
    if (!eina_thread_create(&evas_thread_worker, EINA_THREAD_NORMAL, 0,
          evas_thread_worker_func, NULL))
      CRIT("Could not create draw thread");
}

void
evas_thread_shutdown(void)
{
    assert(init_count);

    if (--init_count)
      return;

    eina_lock_take(&evas_thread_queue_lock);

    exit_thread = EINA_TRUE;
    eina_condition_signal(&evas_thread_queue_condition);

    eina_lock_release(&evas_thread_queue_lock);

    while (!evas_thread_exited)
      evas_async_events_process();

    eina_thread_join(evas_thread_worker);
    eina_lock_free(&evas_thread_queue_lock);
    eina_condition_free(&evas_thread_queue_condition);

    eina_inarray_flush(&evas_thread_queue);

    eina_threads_shutdown();
}
