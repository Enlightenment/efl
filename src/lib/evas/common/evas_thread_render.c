#include <assert.h>

#include "evas_common.h"

static Eina_Thread evas_thread_worker;
static Eina_Condition evas_thread_queue_condition;
static Eina_Lock evas_thread_queue_lock;
static Eina_Bool evas_thread_queue_ready = EINA_FALSE;
static Eina_Inlist *evas_thread_queue = NULL;
static volatile int evas_thread_exited = 0;
static Eina_Bool exit_thread = EINA_FALSE;
static int init_count = 0;

static Evas_Thread_Command *
evas_thread_cmd_new(Evas_Thread_Command_Cb cb, void *data, size_t size)
{
    Evas_Thread_Command *cmd = malloc(sizeof(*cmd) + size);
    if (!cmd)
      {
         ERR("Out of memory allocating thread command.");
         return NULL;
      }

    cmd->cb = cb;
    if (size)
      {
         cmd->data = cmd + 1;
         memcpy(cmd->data, data, size);
      }
    else
      cmd->data = data;

    return cmd;
}

static void
evas_thread_queue_append(Evas_Thread_Command *cmd, Eina_Bool do_flush)
{
    eina_lock_take(&evas_thread_queue_lock);

    evas_thread_queue = eina_inlist_append(evas_thread_queue, EINA_INLIST_GET(cmd));

    if (do_flush)
      {
         evas_thread_queue_ready = EINA_TRUE;
         eina_condition_signal(&evas_thread_queue_condition);
      }

    eina_lock_release(&evas_thread_queue_lock);
}

EAPI void
evas_thread_cmd_enqueue(Evas_Thread_Command_Cb cb, void *data, size_t size)
{
    Evas_Thread_Command *cmd = evas_thread_cmd_new(cb, data, size);
    if (!cmd)
      return;

    evas_thread_queue_append(cmd, EINA_FALSE);
}

EAPI void
evas_thread_queue_flush(Evas_Thread_Command_Cb cb, void *data, size_t size)
{
    Evas_Thread_Command *cmd = evas_thread_cmd_new(cb, data, size);
    if (!cmd)
      return;

    evas_thread_queue_append(cmd, EINA_TRUE);
}

static void*
evas_thread_worker_func(void *data EINA_UNUSED, Eina_Thread thread EINA_UNUSED)
{
    while (1)
      {
         Evas_Thread_Command *cmd;
         Eina_Inlist *queue;

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

         if (!evas_thread_queue)
           {
              ERR("Signaled to find an empty queue. BUG!");
              eina_lock_release(&evas_thread_queue_lock);
              continue;
           }

         queue = evas_thread_queue;
         evas_thread_queue = NULL;
         evas_thread_queue_ready = EINA_FALSE;

         eina_lock_release(&evas_thread_queue_lock);

         while (queue)
           {
              cmd = EINA_INLIST_CONTAINER_GET(queue, Evas_Thread_Command);

              assert(cmd);
              assert(cmd->cb);

              cmd->cb(cmd->data);

              queue = eina_inlist_remove(queue, queue);
              free(cmd);
           }
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

    eina_threads_shutdown();
}
