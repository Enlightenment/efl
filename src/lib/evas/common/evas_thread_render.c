#include "evas_common_private.h"

#include <assert.h>

static Eina_Thread evas_thread_worker;
static Eina_Condition evas_thread_queue_condition;
static Eina_Lock evas_thread_queue_lock;
static Eina_Bool evas_thread_queue_ready = EINA_FALSE;
static Eina_Inarray evas_thread_queue;
static Evas_Thread_Command *evas_thread_queue_cache = NULL;
static unsigned int evas_thread_queue_cache_max = 0;

static volatile int evas_thread_exited = 0;
static Eina_Bool exit_thread = EINA_FALSE;
static int init_count = 0;

#define SHUTDOWN_TIMEOUT_RESET (0)
#define SHUTDOWN_TIMEOUT_CHECK (1)
#define SHUTDOWN_TIMEOUT (3000)

static Eina_Bool
_shutdown_timeout(double *time, int mode, int timeout_ms)
{
   struct timeval tv;

   gettimeofday(&tv, NULL);

   if ( mode == SHUTDOWN_TIMEOUT_RESET )
     *time = (tv.tv_sec + tv.tv_usec / 1000000.0) * 1000.0;
   return ((tv.tv_sec + tv.tv_usec / 1000000.0) * 1000.0 - (*time)) > timeout_ms ;
}


static void
evas_thread_queue_append(Evas_Thread_Command_Cb cb, void *data, Eina_Bool do_flush)
{
   Evas_Thread_Command *cmd;

   eina_lock_take(&evas_thread_queue_lock);

   cmd = eina_inarray_grow(&evas_thread_queue, 1);
   if (cmd)
     {
        cmd->cb = cb;
        cmd->data = data;
     }
   else
     {
        ERR("Out of memory allocating thread command.");
        goto out;
     }

   if (do_flush)
     {
        evas_thread_queue_ready = EINA_TRUE;
        eina_condition_signal(&evas_thread_queue_condition);
     }

out:
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
   eina_thread_name_set(eina_thread_self(), "Eevas-thread-wk");
   while (1)
      {
         Evas_Thread_Command *cmd;
         unsigned int len, max;

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
              evas_thread_queue_ready = EINA_FALSE;
              eina_lock_release(&evas_thread_queue_lock);
              continue;
           }

         cmd = evas_thread_queue.members;
         evas_thread_queue.members = evas_thread_queue_cache;
         evas_thread_queue_cache = cmd;

         max = evas_thread_queue.max;
         evas_thread_queue.max = evas_thread_queue_cache_max;
         evas_thread_queue_cache_max = max;

         len = evas_thread_queue.len;
         evas_thread_queue.len = 0;

         evas_thread_queue_ready = EINA_FALSE;

         eina_lock_release(&evas_thread_queue_lock);

         DBG("Evas render thread command queue length: %u", len);

         eina_evlog("+thread", NULL, 0.0, NULL);
         while (len)
           {
              assert(cmd->cb);

              eina_evlog("+thread_do", cmd->data, 0.0, NULL);
              cmd->cb(cmd->data);
              eina_evlog("-thread_do", cmd->data, 0.0, NULL);

              cmd++;
              len--;
           }
         eina_evlog("-thread", NULL, 0.0, NULL);
      }

out:
    /* WRN: add a memory barrier or use a lock if we add more code here */
    evas_thread_exited = 1;
    return NULL;
}

int
evas_thread_init(void)
{
    if (init_count++)
      return init_count;

    exit_thread = EINA_FALSE;
    evas_thread_exited = 0;

    if(!eina_threads_init())
      {
         CRI("Could not init eina threads");
         goto fail_on_eina_thread_init;
      }

    eina_inarray_step_set(&evas_thread_queue, sizeof (Eina_Inarray), sizeof (Evas_Thread_Command), 128);

    if (!eina_lock_new(&evas_thread_queue_lock))
      {
         CRI("Could not create draw thread lock (%m)");
         goto fail_on_lock_creation;
      }
    if (!eina_condition_new(&evas_thread_queue_condition, &evas_thread_queue_lock))
      {
         CRI("Could not create draw thread condition (%m)");
         goto fail_on_cond_creation;
      }

    if (!eina_thread_create(&evas_thread_worker, EINA_THREAD_NORMAL, -1,
                            evas_thread_worker_func, NULL))
      {
         CRI("Could not create draw thread (%m)");
         goto fail_on_thread_creation;
      }

    return init_count;

fail_on_thread_creation:
    evas_thread_worker = 0;
    eina_condition_free(&evas_thread_queue_condition);
fail_on_cond_creation:
    eina_lock_free(&evas_thread_queue_lock);
fail_on_lock_creation:
    eina_threads_shutdown();
fail_on_eina_thread_init:
    exit_thread = EINA_TRUE;
    evas_thread_exited = 1;
    return --init_count;
}

int
evas_thread_shutdown(void)
{
    double to = 0 ;

    if (init_count <= 0)
      {
         ERR("Too many calls to shutdown, ignored.");
         return 0;
      }

    if (--init_count)
      return init_count;

    eina_lock_take(&evas_thread_queue_lock);

    exit_thread = EINA_TRUE;
    eina_condition_signal(&evas_thread_queue_condition);

    eina_lock_release(&evas_thread_queue_lock);
    _shutdown_timeout(&to, SHUTDOWN_TIMEOUT_RESET, SHUTDOWN_TIMEOUT);
    while (!evas_thread_exited && (evas_async_events_process() != -1))
      {
         if(_shutdown_timeout(&to, SHUTDOWN_TIMEOUT_CHECK, SHUTDOWN_TIMEOUT))
           {
              CRI("Timeout shutdown thread. Skipping thread_join. Some resources could be leaked");
              goto timeout_shutdown;
           }
      }

    eina_thread_join(evas_thread_worker);
timeout_shutdown:
    eina_lock_free(&evas_thread_queue_lock);
    eina_condition_free(&evas_thread_queue_condition);

    evas_thread_worker = 0;

    eina_inarray_flush(&evas_thread_queue);
    free(evas_thread_queue_cache);

    eina_threads_shutdown();

    return 0;
}
