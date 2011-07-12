/*
 * gcc -o ecore_thread_example ecore_thread_example.c `pkg-config --cflags --libs ecore`
 */
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>

typedef struct
{
   Ecore_Thread *thread_3;
   int msgs_received;
   int max_msgs;
   Eina_Lock mutex;
   Eina_Condition condition;
} App_Data;

typedef struct
{
   Eina_List *list;
} Thread_Data;

typedef struct
{
   char *name;
   char *base;
   Eina_Lock mutex;
} Feedback_Thread_Data;

typedef struct
{
   int all_done;
   Eina_List *list;
} App_Msg;

static void
_local_data_free(void *data)
{
   Thread_Data *td = data;
   char *str;

   EINA_LIST_FREE(td->list, str)
     {
        printf("Freeing string: %s\n", str);
        free(str);
     }
   free(td);
}

static void
_short_job(void *data, Ecore_Thread *th)
{
   Thread_Data *td;
   int i;

   td = ecore_thread_local_data_find(th, "data");
   if (!td)
     {
        td = calloc(1, sizeof(Thread_Data));
        if (!td)
          {
             ecore_thread_cancel(th);
             return;
          }
        ecore_thread_local_data_add(th, "data", td, _local_data_free,
                                    EINA_FALSE);
     }

   for (i = 0; i < 10; i++)
     {
        char buf[200];

        if (ecore_thread_check(th))
          {
             ecore_thread_local_data_del(th, "data");
             break;
          }

        snprintf(buf, sizeof(buf), "Thread %p: String number %d", th, i);
        td->list = eina_list_append(td->list, strdup(buf));
        sleep(1);
     }
}

static void
_feedback_job(void *data, Ecore_Thread *th)
{
   time_t t;
   int i, count;
   Feedback_Thread_Data *ftd = NULL;
   DIR *dir;
   App_Msg *msg;

   count = (int)ecore_thread_global_data_find("count");
   for (i = 0; i < count; i++)
     {
        char buf[32];
        snprintf(buf, sizeof(buf), "data%d", i);
        ftd = ecore_thread_global_data_find(buf);
        if (!ftd)
          continue;
        if (eina_lock_take_try(&ftd->mutex))
          break;
        else
          ftd = NULL;
     }
   if (!ftd)
     return;

   dir = opendir(ftd->base);
   if (!dir)
     goto the_end;

   msg = calloc(1, sizeof(App_Msg));

   t = time(NULL);
   while (time(NULL) < t + 2)
     {
        struct dirent entry, *result;

        if (readdir_r(dir, &entry, &result))
          break;
        if (!result)
          break;

        if (strlen(result->d_name) >= 10)
          msg->list = eina_list_append(msg->list,
                                       strdup(result->d_name));
     }

   closedir(dir);
   ecore_thread_feedback(th, msg);

the_end:
   ecore_thread_global_data_del(ftd->name);
   free(ftd->name);
   free(ftd->base);
   eina_lock_release(&ftd->mutex);
   eina_lock_free(&ftd->mutex);
   free(ftd);
   ecore_thread_reschedule(th);
}

static void
_out_of_pool_job(void *data, Ecore_Thread *th)
{
   App_Data *ad = data;
   App_Msg *msg;

   while (1)
     {
        int msgs;
        eina_condition_wait(&ad->condition);
        msgs = ad->msgs_received;
        eina_lock_release(&ad->mutex);
        if (msgs == ad->max_msgs)
          {
             msg = calloc(1, sizeof(App_Msg));
             msg->all_done = 1;
             ecore_thread_feedback(th, msg);
             return;
          }
     }
}

static void
_print_status(void)
{
   int active, pending_total, pending_feedback, pending_short, available;

   active = ecore_thread_active_get();
   pending_total = ecore_thread_pending_total_get();
   pending_feedback = ecore_thread_pending_feedback_get();
   pending_short = ecore_thread_pending_get();
   available = ecore_thread_available_get();

   printf("Status:\n\t* Active threads: %d\n"
          "\t* Available threads: %d\n"
          "\t* Pending short jobs: %d\n"
          "\t* Pending feedback jobs: %d\n"
          "\t* Pending total: %d\n", active, available, pending_short,
          pending_feedback, pending_total);
}

static void
_feedback_job_msg_cb(void *data, Ecore_Thread *th, void *msg_data)
{
   App_Data *ad = data;
   App_Msg *msg = msg_data;
   char *str;

   if (msg->all_done)
     {
        ecore_main_loop_quit();
        free(msg);
        return;
     }

   _print_status();

   if (!msg->list)
     printf("Received an empty list from thread %p\n", th);
   else
     {
        int i = 0;
        printf("Received %d elements from threads %p (printing first 5):\n",
               eina_list_count(msg->list), th);
        EINA_LIST_FREE(msg->list, str)
          {
             if (i <= 5)
               printf("\t%s\n", str);
             free(str);
             i++;
          }
     }

   eina_lock_take(&ad->mutex);
   ad->msgs_received++;
   eina_condition_signal(&ad->condition);
   eina_lock_release(&ad->mutex);

   free(msg);
}

static void
_thread_end_cb(void *data, Ecore_Thread *th)
{
   App_Data *ad = data;

   printf("Normal termination for thread %p.\n", th);
   if (th == ad->thread_3)
     ad->thread_3 = NULL;
}

static void
_thread_cancel_cb(void *data, Ecore_Thread *th)
{
   App_Data *ad = data;

   printf("Thread %p got cancelled.\n", th);
   if (th == ad->thread_3)
     ad->thread_3 = NULL;
}

static Eina_Bool
_cancel_timer_cb(void *data)
{
   App_Data *ad = data;

   if (ad->thread_3 && !ecore_thread_check(ad->thread_3))
     ecore_thread_cancel(ad->thread_3);

   return EINA_FALSE;
}

static Eina_Bool
_status_timer_cb(void *data)
{
   _print_status();

   return EINA_TRUE;
}

static const Ecore_Getopt optdesc = {
   "ecore_thread_example",
   NULL,
   "0.0",
   "(C) 2011 Enlightenment",
   "Public domain?",
   "Example program for Ecore_Thread",
   0,
   {
      ECORE_GETOPT_STORE_INT('t', "threads", "Max number of threads to run"),
      ECORE_GETOPT_STORE_INT('m', "msgs", "Max number of messages to receive"),
      ECORE_GETOPT_APPEND_METAVAR('p', "path", "Add path for feedback job",
                                  "STRING", ECORE_GETOPT_TYPE_STR),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

int
main(int argc, char *argv[])
{
   int i, max_threads = 0, max_msgs = 0;
   Eina_Bool opt_quit = EINA_FALSE;
   Eina_List *path_list = NULL;
   App_Data appdata;
   Ecore_Getopt_Value values[] = {
        ECORE_GETOPT_VALUE_INT(max_threads),
        ECORE_GETOPT_VALUE_INT(max_msgs),
        ECORE_GETOPT_VALUE_LIST(path_list),
        ECORE_GETOPT_VALUE_BOOL(opt_quit),
        ECORE_GETOPT_VALUE_NONE
   };

   ecore_init();

   i = ecore_thread_max_get();
   printf("Initial max threads: %d\n", i);

   memset(&appdata, 0, sizeof(App_Data));
   appdata.max_msgs = 1;

   if (ecore_getopt_parse(&optdesc, values, argc, argv) < 0)
     {
        printf("Argument parsing failed\n");
        return 1;
     }

   if (opt_quit)
     return 0;

   if (max_threads)
     {
        ecore_thread_max_set(max_threads);
        printf("Max threads: %d\n", ecore_thread_max_get());
     }
   if (max_msgs)
     appdata.max_msgs = max_msgs;

   if (!path_list)
     {
        Feedback_Thread_Data *ftd;
        ecore_thread_global_data_add("count", (void *)3, NULL, EINA_FALSE);
        ftd = calloc(1, sizeof(Feedback_Thread_Data));
        ftd->name = strdup("data0");
        ftd->base = strdup("/usr/bin");
        eina_lock_new(&ftd->mutex);
        ecore_thread_global_data_add(ftd->name, ftd, NULL, EINA_TRUE);
        ftd = calloc(1, sizeof(Feedback_Thread_Data));
        ftd->name = strdup("data1");
        ftd->base = strdup("/usr/lib");
        eina_lock_new(&ftd->mutex);
        ecore_thread_global_data_add(ftd->name, ftd, NULL, EINA_TRUE);
        ftd = calloc(1, sizeof(Feedback_Thread_Data));
        ftd->name = strdup("data2");
        ftd->base = strdup("/usr/share");
        eina_lock_new(&ftd->mutex);
        ecore_thread_global_data_add(ftd->name, ftd, NULL, EINA_TRUE);
     }
   else
     {
        Feedback_Thread_Data *ftd;
        char *str;
        ecore_thread_global_data_add("count",
                                     (void *)eina_list_count(path_list), NULL,
                                     EINA_FALSE);
        i = 0;
        EINA_LIST_FREE(path_list, str)
          {
             char buf[32];
             snprintf(buf, sizeof(buf), "data%d", i);
             ftd = calloc(1, sizeof(Feedback_Thread_Data));
             ftd->name = strdup(buf);
             ftd->base = strdup(str);
             eina_lock_new(&ftd->mutex);
             ecore_thread_global_data_add(ftd->name, ftd, NULL, EINA_TRUE);
             free(str);
             i++;
          }
     }

   eina_lock_new(&appdata.mutex);
   eina_condition_new(&appdata.condition, &appdata.mutex);

   ecore_thread_feedback_run(_out_of_pool_job, _feedback_job_msg_cb, NULL,
                             NULL, &appdata, EINA_TRUE);

   ecore_thread_run(_short_job, _thread_end_cb, _thread_cancel_cb, &appdata);
   ecore_thread_feedback_run(_feedback_job, _feedback_job_msg_cb,
                             _thread_end_cb, _thread_cancel_cb, &appdata,
                             EINA_FALSE);
   appdata.thread_3 = ecore_thread_run(_short_job, _thread_end_cb,
                                       _thread_cancel_cb, &appdata);
   ecore_thread_feedback_run(_feedback_job, _feedback_job_msg_cb,
                             _thread_end_cb, _thread_cancel_cb, &appdata,
                             EINA_FALSE);

   ecore_timer_add(1.0, _cancel_timer_cb, &appdata);
   ecore_timer_add(2.0, _status_timer_cb, NULL);

   _print_status();

   ecore_main_loop_begin();

   eina_condition_free(&appdata.condition);
   eina_lock_free(&appdata.mutex);

   ecore_shutdown();

   return 0;
}
