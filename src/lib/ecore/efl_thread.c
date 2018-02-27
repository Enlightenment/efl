#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1

#include <Ecore.h>

#include "ecore_private.h"

#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define MY_CLASS EFL_THREAD_CLASS

typedef struct
{
   const char *name;
   struct {
      int in, out;
      Eo *in_handler, *out_handler;
   } fd, ctrl;
   struct {
      unsigned int argc;
      const char **argv;
   } args;
   Efl_Callback_Array_Item_Full *event_cb;
} Thread_Data;

#define CMD_EXIT   1
#define CMD_EXITED 2

typedef struct
{
   int command;
   int data;
} Control_Data;

typedef struct _Efl_Thread_Data Efl_Thread_Data;

struct _Efl_Thread_Data
{
   struct {
      int in, out;
      Eo *in_handler, *out_handler;
      Eina_Bool can_read : 1;
      Eina_Bool eos_read : 1;
      Eina_Bool can_write : 1;
   } fd, ctrl;
   int read_listeners;
   Eo *loop;
   Thread_Data *thdat;
   Efl_Callback_Array_Item_Full *event_cb;
   Eina_Thread thread;
   Eina_Bool end_sent : 1;
   Eina_Bool exit_read : 1;
   Eina_Bool exit_called : 1;
};

//////////////////////////////////////////////////////////////////////////

static void
_cb_thread_out(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   efl_io_reader_can_read_set(obj, EINA_TRUE);
}

static void
_cb_thread_in(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   efl_io_writer_can_write_set(obj, EINA_TRUE);
}

static void
_cb_thread_ctrl_out(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   Control_Data cmd;
   ssize_t ret;
   Efl_Appthread_Data *ad;

   ad = efl_data_scope_get(obj, EFL_APPTHREAD_CLASS);
   cmd.command = 0;
   cmd.data = 0;
   ret = read(ad->ctrl.out, &cmd, sizeof(Control_Data));
   if (ret == sizeof(Control_Data))
     {
        if (cmd.command == CMD_EXIT)
          {
             efl_event_callback_call(obj, EFL_TASK_EVENT_EXIT, NULL);
             efl_loop_quit(obj, eina_value_int_init(0));
          }
     }
}

static Eina_Value
_efl_loop_arguments_send(void *data, const Eina_Value v,
                         const Eina_Future *dead EINA_UNUSED)

{
   Efl_Loop_Arguments arge;
   Eo *obj = data;
   Eina_Array *arga;
   Eina_Stringshare *s;
   unsigned int argc = efl_task_arg_count_get(obj);
   unsigned int i;

   arga = eina_array_new(argc);
   if (v.type == EINA_VALUE_TYPE_ERROR) goto on_error;

   for (i = 0; i < argc; i++)
     {
        const char *argv = efl_task_arg_value_get(obj, i);
        if (argv)
          eina_array_push(arga, eina_stringshare_add(argv));
     }
   arge.argv = arga;
   arge.initialization = EINA_TRUE;
   efl_event_callback_call(obj,
                           EFL_LOOP_EVENT_ARGUMENTS, &arge);
on_error:
   while ((s = eina_array_pop(arga))) eina_stringshare_del(s);
   eina_array_free(arga);
   return v;
}

static void
_appthread_parent_read_listeners_modify(Efl_Appthread_Data *ad, int mod)
{
   ad->read_listeners += mod;

   if (ad->fd.out_handler)
     {
        if ((ad->read_listeners == 0) && (mod < 0))
          efl_loop_handler_active_set
            (ad->fd.out_handler, EFL_LOOP_HANDLER_FLAGS_NONE);
        else if ((ad->read_listeners == 1) && (mod > 0))
          efl_loop_handler_active_set
            (ad->fd.out_handler, EFL_LOOP_HANDLER_FLAGS_READ);
     }
}

static void
_cb_appthread_event_callback_add(void *data, const Efl_Event *event)
{
   Efl_Appthread_Data *ad = data;
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_IO_READER_EVENT_CAN_READ_CHANGED)
          _appthread_parent_read_listeners_modify(ad, 1);
     }
}

static void
_cb_appthread_event_callback_del(void *data, const Efl_Event *event)
{
   Efl_Appthread_Data *ad = data;
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_IO_READER_EVENT_CAN_READ_CHANGED)
          _appthread_parent_read_listeners_modify(ad, -1);
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(_appthread_event_callback_watch,
                           { EFL_EVENT_CALLBACK_ADD, _cb_appthread_event_callback_add },
                           { EFL_EVENT_CALLBACK_DEL, _cb_appthread_event_callback_del });

static void *
_efl_thread_main(void *data, Eina_Thread t)
{
   Efl_Appthread_Data *ad;
   Thread_Data *thdat = data;
   Eo *obj;
   Eina_Value *ret;
   Control_Data cmd;
   unsigned int i;
   int real;
   Efl_Callback_Array_Item_Full *it;
   Eina_Future *job;

   if (thdat->name) eina_thread_name_set(t, thdat->name);
   else eina_thread_name_set(t, "Eflthread");

   obj = efl_add(EFL_APPTHREAD_CLASS, NULL);
   ad = efl_data_scope_get(obj, EFL_APPTHREAD_CLASS);
   efl_event_callback_array_add(obj, _appthread_event_callback_watch(), ad);

   // add handlers for "stdio"
   thdat->fd.in_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, thdat->fd.in),
             efl_event_callback_add
               (efl_added, EFL_LOOP_HANDLER_EVENT_WRITE, _cb_thread_in, obj));
   thdat->fd.out_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, thdat->fd.out),
             efl_event_callback_add
               (efl_added, EFL_LOOP_HANDLER_EVENT_READ, _cb_thread_out, obj));
   // add handlers for control pipes
   thdat->ctrl.in_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, thdat->ctrl.in));
   thdat->ctrl.out_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, thdat->ctrl.out),
             efl_event_callback_add
               (efl_added, EFL_LOOP_HANDLER_EVENT_READ, _cb_thread_ctrl_out, obj),
             efl_loop_handler_active_set
               (efl_added, EFL_LOOP_HANDLER_FLAGS_READ));
   ad->fd.in = thdat->fd.in;
   ad->fd.out = thdat->fd.out;
   ad->ctrl.in = thdat->ctrl.in;
   ad->ctrl.out = thdat->ctrl.out;
   ad->fd.in_handler = thdat->fd.in_handler;
   ad->fd.out_handler = thdat->fd.out_handler;
   ad->ctrl.in_handler = thdat->ctrl.in_handler;
   ad->ctrl.out_handler = thdat->ctrl.out_handler;

   if (thdat->event_cb)
     {
        for (it = thdat->event_cb; it->func; it++)
          efl_event_callback_priority_add(obj, it->desc, it->priority,
                                          it->func, it->user_data);
     }
   for (i = 0; i < thdat->args.argc; i++)
     efl_task_arg_append(obj, thdat->args.argv[i]);
   job = eina_future_then(efl_loop_job(obj), _efl_loop_arguments_send, obj);
   efl_future_Eina_FutureXXX_then(obj, job);

   for (i = 0; i < thdat->args.argc; i++)
     eina_stringshare_del(thdat->args.argv[i]);
   free(thdat->args.argv);
   free(thdat->event_cb);
   thdat->args.argv = NULL;
   thdat->event_cb = NULL;

   ret = efl_loop_begin(obj);
   real = efl_loop_exit_code_process(ret);

   cmd.command = CMD_EXITED;
   cmd.data = real;
   write(thdat->ctrl.in, &cmd, sizeof(Control_Data));

   efl_del(obj);

   thdat->fd.in_handler = NULL;
   thdat->fd.out_handler = NULL;
   thdat->ctrl.in_handler = NULL;
   thdat->ctrl.out_handler = NULL;
   thdat->fd.in = -1;
   thdat->fd.out = -1;
   thdat->ctrl.in = -1;
   thdat->ctrl.out = -1;

   return NULL;
}

//////////////////////////////////////////////////////////////////////////

static Eina_Value
_efl_loop_task_exit(void *data, const Eina_Value v,
                    const Eina_Future *dead EINA_UNUSED)

{
   Eo *obj = data;

   efl_event_callback_call(obj, EFL_TASK_EVENT_EXIT, NULL);
   efl_unref(obj);
   return v;
}

static void
_thread_exit_eval(Eo *obj, Efl_Thread_Data *pd)
{
   if ((pd->fd.out == -1) && /*(pd->fd.in == -1) &&*/
       (pd->exit_read) && (!pd->exit_called))
     {
        Eina_Future *job;
        Eo *loop = efl_provider_find(obj, EFL_LOOP_CLASS);

        pd->exit_called = EINA_TRUE;
        efl_ref(obj);
        job = eina_future_then(efl_loop_job(loop), _efl_loop_task_exit, obj);
        efl_future_Eina_FutureXXX_then(loop, job);
     }
}

static void
_cb_thread_parent_out(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   efl_io_reader_can_read_set(obj, EINA_TRUE);
}

static void
_cb_thread_parent_in(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   efl_io_writer_can_write_set(obj, EINA_TRUE);
}

static void
_cb_thread_parent_ctrl_out(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *obj = data;
   Control_Data cmd;
   ssize_t ret;
   Efl_Thread_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   if (!pd) return;

   cmd.command = 0;
   cmd.data = 0;
   ret = read(pd->ctrl.out, &cmd, sizeof(Control_Data));
   if (ret == sizeof(Control_Data))
     {
        if (cmd.command == CMD_EXITED)
          {
             if (!pd->exit_read)
               {
                  Efl_Task_Data *td = efl_data_scope_get(obj, EFL_TASK_CLASS);

                  if (td) td->exit_code = cmd.data;
                  pd->exit_read = EINA_TRUE;
                  _thread_exit_eval(obj, pd);
               }
          }
     }
}

//////////////////////////////////////////////////////////////////////////

static void
_thread_parent_read_listeners_modify(Efl_Thread_Data *pd, int mod)
{
   pd->read_listeners += mod;

   if (pd->fd.out_handler)
     {
        if ((pd->read_listeners == 0) && (mod < 0))
          efl_loop_handler_active_set
            (pd->fd.out_handler, EFL_LOOP_HANDLER_FLAGS_NONE);
        else if ((pd->read_listeners == 1) && (mod > 0))
          efl_loop_handler_active_set
            (pd->fd.out_handler, EFL_LOOP_HANDLER_FLAGS_READ);
     }
}

static void
_cb_event_callback_add(void *data, const Efl_Event *event)
{
   Efl_Thread_Data *pd = data;
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_EVENT_ARGUMENTS)
          {
             Efl_Callback_Array_Item_Full *event_cb, *it;
             int num;

             num = 0;
             if (pd->event_cb)
               {
                  for (it = pd->event_cb; it->func; it++) num++;
               }
             num++;
             event_cb = realloc(pd->event_cb, (num + 1) * sizeof(Efl_Callback_Array_Item_Full));
             if (event_cb)
               {
                  pd->event_cb = event_cb;
                  event_cb[num - 1] = array[i];
                  event_cb[num].desc = NULL;
                  event_cb[num].priority = 0;
                  event_cb[num].func = NULL;
                  event_cb[num].user_data = NULL;
               }
          }
        else if (array[i].desc == EFL_IO_READER_EVENT_CAN_READ_CHANGED)
          _thread_parent_read_listeners_modify(pd, 1);
     }
}

static void
_cb_event_callback_del(void *data, const Efl_Event *event)
{
   Efl_Thread_Data *pd = data;
   const Efl_Callback_Array_Item_Full *array = event->info;
   int i;

   for (i = 0; array[i].desc != NULL; i++)
     {
        if (array[i].desc == EFL_LOOP_EVENT_ARGUMENTS)
          {
             Efl_Callback_Array_Item_Full *it;

             if (pd->event_cb)
               {
                  Eina_Bool shuffle_down = EINA_FALSE;

                  for (it = pd->event_cb; it->func; it++)
                    {
                       if ((it->desc == array[i].desc) &&
                           (it->priority == array[i].priority) &&
                           (it->func == array[i].func) &&
                           (it->user_data == array[i].user_data))
                         shuffle_down = EINA_TRUE;
                       if (shuffle_down) it[0] = it[1];
                    }
               }
          }
        else if (array[i].desc == EFL_IO_READER_EVENT_CAN_READ_CHANGED)
          _thread_parent_read_listeners_modify(pd, -1);
     }
}

EFL_CALLBACKS_ARRAY_DEFINE(_event_callback_watch,
                           { EFL_EVENT_CALLBACK_ADD, _cb_event_callback_add },
                           { EFL_EVENT_CALLBACK_DEL, _cb_event_callback_del });

//////////////////////////////////////////////////////////////////////////

EOLIAN static Efl_Object *
_efl_thread_efl_object_constructor(Eo *obj, Efl_Thread_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_event_callback_array_add(obj, _event_callback_watch(), pd);
   pd->fd.in = -1;
   pd->fd.out = -1;
   pd->fd.can_write = EINA_TRUE;
   pd->ctrl.in = -1;
   pd->ctrl.out = -1;
   return obj;
}

EOLIAN static void
_efl_thread_efl_object_destructor(Eo *obj, Efl_Thread_Data *pd)
{
   if (pd->thdat)
     {
/* we probably shouldn't do this... this simply has to orphan threads if they
 * lose their parent. this stops shutdown from blocking.
        // if exit response not read yet, read until fetched
        if (!pd->exit_read)
          {
             Control_Data cmd;
             ssize_t ret;

             // if it hasn't been asked to exit... ask it
             if (!pd->end_sent) efl_task_end(obj);
             cmd.command = 0;
             cmd.data = 0;
             ret = read(pd->ctrl.out, &cmd, sizeof(Control_Data));
             while (ret == sizeof(Control_Data))
               {
                  if (cmd.command == CMD_EXITED)
                    {
                       if (!pd->exit_read)
                         {
                            Efl_Task_Data *td = efl_data_scope_get(obj, EFL_TASK_CLASS);

                            if (td) td->exit_code = cmd.data;
                            pd->exit_read = EINA_TRUE;
                            efl_event_callback_call(obj, EFL_TASK_EVENT_EXIT, NULL);
                            break;
                         }
                    }
                  ret = read(pd->ctrl.out, &cmd, sizeof(Control_Data));
               }
          }
 */
        // stop and wait for thread to exit/join here
        eina_thread_join(pd->thread);
        efl_del(pd->fd.in_handler);
        efl_del(pd->fd.out_handler);
        efl_del(pd->ctrl.in_handler);
        efl_del(pd->ctrl.out_handler);
        close(pd->fd.in);
        close(pd->fd.out);
        close(pd->ctrl.in);
        close(pd->ctrl.out);
        pd->fd.in_handler = NULL;
        pd->fd.out_handler = NULL;
        pd->ctrl.in_handler = NULL;
        pd->ctrl.out_handler = NULL;
        pd->fd.in = -1;
        pd->fd.out = -1;
        pd->ctrl.in = -1;
        pd->ctrl.out = -1;
        free(pd->thdat);
     }
   free(pd->event_cb);
   pd->event_cb = NULL;
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_thread_efl_object_parent_set(Eo *obj, Efl_Thread_Data *pd, Efl_Object *parent)
{
   efl_parent_set(efl_super(obj, MY_CLASS), parent);
   pd->loop = efl_provider_find(parent, EFL_LOOP_CLASS);
}

EOLIAN static Eina_Bool
_efl_thread_efl_task_run(Eo *obj, Efl_Thread_Data *pd)
{
   Eina_Thread_Priority pri;
   Thread_Data *thdat;
   const char *name;
   int pipe_to_thread[2];
   int pipe_from_thread[2];
   unsigned int argc, i, num;
   Efl_Callback_Array_Item_Full *it;

   thdat = calloc(1, sizeof(Thread_Data));
   if (!thdat) return EINA_FALSE;
   thdat->fd.in = -1;
   thdat->fd.out = -1;
   thdat->ctrl.in = -1;
   thdat->ctrl.out = -1;

   if (pipe(pipe_to_thread) != 0)
     {
        ERR("Can't create to_thread pipe");
        free(thdat);
        return EINA_FALSE;
     }
   if (pipe(pipe_from_thread) != 0)
     {
        ERR("Can't create from_thread pipe");
        close(pipe_to_thread[0]);
        close(pipe_to_thread[1]);
        free(thdat);
        return EINA_FALSE;
     }
   thdat->fd.in  = pipe_from_thread[1]; // write - input to parent
   thdat->fd.out = pipe_to_thread  [0]; // read - output from parent
   pd->fd.in     = pipe_to_thread  [1]; // write - input to child
   pd->fd.out    = pipe_from_thread[0]; // read - output from child
   eina_file_close_on_exec(pd->fd.in, EINA_TRUE);
   eina_file_close_on_exec(pd->fd.out, EINA_TRUE);
   eina_file_close_on_exec(thdat->fd.in, EINA_TRUE);
   eina_file_close_on_exec(thdat->fd.out, EINA_TRUE);
   fcntl(pd->fd.in, F_SETFL, O_NONBLOCK);
   fcntl(pd->fd.out, F_SETFL, O_NONBLOCK);
   fcntl(thdat->fd.in, F_SETFL, O_NONBLOCK);
   fcntl(thdat->fd.out, F_SETFL, O_NONBLOCK);
   pd->fd.in_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, pd->fd.in),
             efl_event_callback_add
               (efl_added, EFL_LOOP_HANDLER_EVENT_WRITE, _cb_thread_parent_in, obj));
   pd->fd.out_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, pd->fd.out),
             efl_event_callback_add
               (efl_added, EFL_LOOP_HANDLER_EVENT_READ, _cb_thread_parent_out, obj));
   if (pd->read_listeners > 0)
     efl_loop_handler_active_set(pd->fd.out_handler, EFL_LOOP_HANDLER_FLAGS_READ);

   if (pipe(pipe_to_thread) != 0)
     {
        ERR("Can't create to_thread control pipe");
        efl_del(pd->fd.in_handler);
        efl_del(pd->fd.out_handler);
        close(thdat->fd.in);
        close(thdat->fd.out);
        close(pd->fd.in);
        close(pd->fd.out);
        pd->fd.in_handler = NULL;
        pd->fd.out_handler = NULL;
        pd->fd.in = -1;
        pd->fd.out = -1;
        free(thdat);
        return EINA_FALSE;
     }
   if (pipe(pipe_from_thread) != 0)
     {
        ERR("Can't create from_thread control pipe");
        efl_del(pd->fd.in_handler);
        efl_del(pd->fd.out_handler);
        close(pipe_to_thread[0]);
        close(pipe_to_thread[1]);
        close(thdat->fd.in);
        close(thdat->fd.out);
        close(pd->fd.in);
        close(pd->fd.out);
        pd->fd.in_handler = NULL;
        pd->fd.out_handler = NULL;
        pd->fd.in = -1;
        pd->fd.out = -1;
        free(thdat);
        return EINA_FALSE;
     }
   thdat->ctrl.in  = pipe_from_thread[1]; // write - input to parent
   thdat->ctrl.out = pipe_to_thread  [0]; // read - output from parent
   pd->ctrl.in     = pipe_to_thread  [1]; // write - input to child
   pd->ctrl.out    = pipe_from_thread[0]; // read - output from child
   // yes - these are blocking because we write and read very little
   eina_file_close_on_exec(pd->ctrl.in, EINA_TRUE);
   eina_file_close_on_exec(pd->ctrl.out, EINA_TRUE);
   eina_file_close_on_exec(thdat->ctrl.in, EINA_TRUE);
   eina_file_close_on_exec(thdat->ctrl.out, EINA_TRUE);
   pd->ctrl.in_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, pd->ctrl.in));
   pd->ctrl.out_handler =
     efl_add(EFL_LOOP_HANDLER_CLASS, obj,
             efl_loop_handler_fd_set(efl_added, pd->ctrl.out),
             efl_event_callback_add
               (efl_added, EFL_LOOP_HANDLER_EVENT_READ, _cb_thread_parent_ctrl_out, obj),
             efl_loop_handler_active_set
               (efl_added, EFL_LOOP_HANDLER_FLAGS_READ));

   switch (efl_task_priority_get(obj))
     {
      case EFL_TASK_PRIORITY_BACKGROUND:
        pri = EINA_THREAD_IDLE;
        break;
      case EFL_TASK_PRIORITY_HIGH:
      case EFL_TASK_PRIORITY_ULTRA:
        pri = EINA_THREAD_URGENT;
        break;
      case EFL_TASK_PRIORITY_LOW:
        pri = EINA_THREAD_BACKGROUND;
        break;
      case EFL_TASK_PRIORITY_NORMAL:
      default:
        pri = EINA_THREAD_NORMAL;
        break;
     }
   name = efl_name_get(obj);
   if (name) thdat->name = eina_stringshare_add(name);

   argc = efl_task_arg_count_get(obj);
   if (argc > 0)
     {
        thdat->args.argc = argc;
        thdat->args.argv = malloc(argc * sizeof(char *));
        if (thdat->args.argv)
          {
             for (i = 0; i < argc; i++)
               {
                  const char *argv = efl_task_arg_value_get(obj, i);
                  if (argv)
                    thdat->args.argv[i] = eina_stringshare_add(argv);
                  else
                    thdat->args.argv[i] = NULL;
               }
          }
        // XXX: if malloc fails?
     }
   if (pd->event_cb)
     {
        num = 0;
        for (it = pd->event_cb; it->func; it++) num++;
        thdat->event_cb = malloc((num + 1) * sizeof(Efl_Callback_Array_Item_Full));
        if (thdat->event_cb)
          memcpy(thdat->event_cb, pd->event_cb,
                 (num + 1) * sizeof(Efl_Callback_Array_Item_Full));
        // XXX: if malloc fails?
     }

   // env data - ignore as other end will share same env

   if (!eina_thread_create(&(pd->thread), pri, -1, _efl_thread_main, thdat))
     {
        for (i = 0; i < thdat->args.argc; i++)
          eina_stringshare_del(thdat->args.argv[i]);
        free(thdat->args.argv);
        efl_del(pd->fd.in_handler);
        efl_del(pd->fd.out_handler);
        efl_del(pd->ctrl.in_handler);
        efl_del(pd->ctrl.out_handler);
        close(pd->fd.in);
        close(pd->fd.out);
        close(pd->ctrl.in);
        close(pd->ctrl.out);
        close(thdat->fd.in);
        close(thdat->fd.out);
        close(thdat->ctrl.in);
        close(thdat->ctrl.out);
        free(thdat);
        pd->fd.in_handler = NULL;
        pd->fd.out_handler = NULL;
        pd->ctrl.in_handler = NULL;
        pd->ctrl.out_handler = NULL;
        pd->fd.in = -1;
        pd->fd.out = -1;
        pd->ctrl.in = -1;
        pd->ctrl.out = -1;
        return EINA_FALSE;
     }
   pd->thdat = thdat;
   return EINA_TRUE;
}

EOLIAN static void
_efl_thread_efl_task_end(Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
{
   if (pd->end_sent) return;
   if (pd->thdat)
     {
        Control_Data cmd;

        pd->end_sent = EINA_TRUE;
        cmd.command = CMD_EXIT;
        cmd.data = 0;
        write(pd->ctrl.in, &cmd, sizeof(Control_Data));
     }
}

EOLIAN static Eina_Error
_efl_thread_efl_io_closer_close(Eo *obj, Efl_Thread_Data *pd)
{
   if (!pd->thdat) return 0;
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_io_closer_closed_get(obj), EBADF);
   efl_io_writer_can_write_set(obj, EINA_FALSE);
   efl_io_reader_can_read_set(obj, EINA_FALSE);
   efl_io_reader_eos_set(obj, EINA_TRUE);
   if (pd->fd.in >= 0) close(pd->fd.in);
   if (pd->fd.out >= 0) close(pd->fd.out);
   if (pd->fd.in_handler) efl_del(pd->fd.in_handler);
   if (pd->fd.out_handler) efl_del(pd->fd.out_handler);
   pd->fd.in = -1;
   pd->fd.out = -1;
   pd->fd.in_handler = NULL;
   pd->fd.out_handler = NULL;
   return 0;
}

EOLIAN static Eina_Bool
_efl_thread_efl_io_closer_closed_get(Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
{
   if ((pd->fd.in == -1) && (pd->fd.out == -1)) return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN static Eina_Error
_efl_thread_efl_io_reader_read(Eo *obj, Efl_Thread_Data *pd, Eina_Rw_Slice *rw_slice)
{
   ssize_t r;

   errno = 0;
   if (pd->fd.out == -1) goto err;

   do
     {
        errno = 0;
        r = read(pd->fd.out, rw_slice->mem, rw_slice->len);
        if (r == -1)
          {
             if (errno == EINTR) continue;
             goto err;
          }
     }
   while (r == -1);

   rw_slice->len = r;
   if (r == 0)
     {
        efl_io_reader_can_read_set(obj, EINA_FALSE);
        efl_io_reader_eos_set(obj, EINA_TRUE);
        close(pd->fd.out);
        pd->fd.out = -1;
        efl_del(pd->fd.out_handler);
        pd->fd.out_handler = NULL;
        _thread_exit_eval(obj, pd);
        return EPIPE;
     }
   return 0;
err:
   if ((pd->fd.out != -1) && (errno != EAGAIN))
     {
        close(pd->fd.out);
        pd->fd.out = -1;
        efl_del(pd->fd.out_handler);
        pd->fd.out_handler = NULL;
     }
   rw_slice->len = 0;
   rw_slice->mem = NULL;
   efl_io_reader_can_read_set(obj, EINA_FALSE);
   _thread_exit_eval(obj, pd);
   return EINVAL;
}

EOLIAN static void
_efl_thread_efl_io_reader_can_read_set(Eo *obj, Efl_Thread_Data *pd, Eina_Bool can_read)
{
   Eina_Bool old = efl_io_reader_can_read_get(obj);
   if (old == can_read) return;
   pd->fd.can_read = can_read;
   if (!pd->fd.out_handler) return;
   if (can_read)
     efl_loop_handler_active_set(pd->fd.out_handler, 0);
   else
     efl_loop_handler_active_set(pd->fd.out_handler,
                                 EFL_LOOP_HANDLER_FLAGS_READ);
   efl_event_callback_call(obj, EFL_IO_READER_EVENT_CAN_READ_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_thread_efl_io_reader_can_read_get(Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
{
   return pd->fd.can_read;
}

EOLIAN static void
_efl_thread_efl_io_reader_eos_set(Eo *obj, Efl_Thread_Data *pd, Eina_Bool is_eos)
{
   Eina_Bool old = efl_io_reader_eos_get(obj);
   if (old == is_eos) return;

   pd->fd.eos_read = is_eos;
   if (!is_eos) return;
   if (pd->fd.out_handler)
     efl_loop_handler_active_set(pd->fd.out_handler, 0);
   efl_event_callback_call(obj, EFL_IO_READER_EVENT_EOS, NULL);
}

EOLIAN static Eina_Bool
_efl_thread_efl_io_reader_eos_get(Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
{
   return pd->fd.eos_read;
}

EOLIAN static Eina_Error
_efl_thread_efl_io_writer_write(Eo *obj, Efl_Thread_Data *pd, Eina_Slice *slice, Eina_Slice *remaining)
{
   ssize_t r;

   errno = 0;
   if (pd->fd.in == -1) goto err;

   do
     {
        errno = 0;
        r = write(pd->fd.in, slice->mem, slice->len);
        if (r == -1)
          {
             if (errno == EINTR) continue;
             goto err;
          }
     }
   while (r == -1);

   if (remaining)
     {
        remaining->len = slice->len - r;
        remaining->bytes = slice->bytes + r;
     }
   slice->len = r;

   if ((slice) && (slice->len > 0))
     efl_io_writer_can_write_set(obj, EINA_FALSE);
   if (r == 0)
     {
        close(pd->fd.in);
        pd->fd.in = -1;
        efl_del(pd->fd.in_handler);
        pd->fd.in_handler = NULL;
        _thread_exit_eval(obj, pd);
        return EPIPE;
     }
   return 0;
err:
   if ((pd->fd.in != -1) && (errno != EAGAIN))
     {
        close(pd->fd.in);
        pd->fd.in = -1;
        efl_del(pd->fd.in_handler);
        pd->fd.in_handler = NULL;
     }
   if (remaining) *remaining = *slice;
   slice->len = 0;
   slice->mem = NULL;
   efl_io_writer_can_write_set(obj, EINA_FALSE);
   _thread_exit_eval(obj, pd);
   return EINVAL;
}

EOLIAN static void
_efl_thread_efl_io_writer_can_write_set(Eo *obj, Efl_Thread_Data *pd, Eina_Bool can_write)
{
   Eina_Bool old = efl_io_writer_can_write_get(obj);
   if (old == can_write) return;
   pd->fd.can_write = can_write;
   if (can_write)
     efl_loop_handler_active_set(pd->fd.in_handler, 0);
   else
     efl_loop_handler_active_set(pd->fd.in_handler,
                                 EFL_LOOP_HANDLER_FLAGS_WRITE);
   efl_event_callback_call(obj, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, NULL);
}

EOLIAN static Eina_Bool
_efl_thread_efl_io_writer_can_write_get(Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
{
   return pd->fd.can_write;
}

//////////////////////////////////////////////////////////////////////////

#include "efl_thread.eo.c"
