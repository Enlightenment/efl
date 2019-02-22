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

#define APPTHREAD_CLASS EFL_APPTHREAD_CLASS

typedef struct
{
   const char *name;
   struct {
      int in, out;
      Eo *in_handler, *out_handler;
   } fd, ctrl;
   Eina_Array *argv;
   Efl_Callback_Array_Item_Full *event_cb;
   void *indata, *outdata;
} Thread_Data;

#define CMD_EXIT        1
#define CMD_EXITED      2
#define CMD_CALL        3
#define CMD_CALL_SYNC   4

typedef struct
{
   Eina_Semaphore sem;
   void *data;
} Control_Reply;

typedef struct
{
   union {
      struct {
         int command;
         int data;
         void *ptr[4];
      } d;
      unsigned char b[64];
   };
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
   Eina_Promise *promise;
   Eo *loop;
   Thread_Data *thdat;
   Efl_Callback_Array_Item_Full *event_cb;
   Eina_Thread thread;
   Eina_Bool end_sent : 1;
   Eina_Bool exit_read : 1;
   Eina_Bool exit_called : 1;
   Eina_Bool run : 1;
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

   ad = efl_data_scope_get(obj, APPTHREAD_CLASS);
   memset(&cmd, 0, sizeof(cmd));
   ret = read(ad->ctrl.out, &cmd, sizeof(Control_Data));
   if (ret == sizeof(Control_Data))
     {
        if (cmd.d.command == CMD_EXIT)
          {
             efl_event_callback_call(obj, EFL_LOOP_EVENT_QUIT, NULL);
             efl_loop_quit(obj, eina_value_int_init(0));
          }
        else if (cmd.d.command == CMD_CALL)
          {
             EFlThreadIOCall func = cmd.d.ptr[0];
             void *data = cmd.d.ptr[1];
             Eina_Free_Cb free_func = cmd.d.ptr[2];
             if (func)
               {
                  Efl_Event event = { obj, NULL, NULL };

                  func(data, &event);
               }
             if (free_func) free_func(data);
          }
        else if (cmd.d.command == CMD_CALL_SYNC)
          {
             EFlThreadIOCallSync func = cmd.d.ptr[0];
             void *data = cmd.d.ptr[1];
             Eina_Free_Cb free_func = cmd.d.ptr[2];
             Control_Reply *rep = cmd.d.ptr[3];
             if (func)
               {
                  Efl_Event event = { obj, NULL, NULL };

                  rep->data = func(data, &event);
               }
             if (free_func) free_func(data);
             eina_semaphore_release(&(rep->sem), 1);
          }
     }
}

static Eina_Value
_efl_loop_arguments_send(Eo *obj, void *data EINA_UNUSED, const Eina_Value v)

{
   Efl_Loop_Arguments arge;
   Eina_Array *arga;
   Eina_Stringshare *s;
   Eina_Accessor *accessor;
   const char *argv;
   int i = 0;

   accessor = efl_core_command_line_command_access(obj);
   if (accessor)
     {
        arga = eina_array_new(10);

        EINA_ACCESSOR_FOREACH(accessor, i, argv)
          {
             eina_array_push(arga, eina_stringshare_add(argv));
          }
        arge.argv = arga;
     }
   else arge.argv = NULL;
   arge.initialization = EINA_TRUE;
   efl_event_callback_call(obj,
                           EFL_LOOP_EVENT_ARGUMENTS, &arge);
   if (accessor)
     {
        while ((s = eina_array_pop(arga))) eina_stringshare_del(s);
        eina_array_free(arga);
        eina_accessor_free(accessor);
     }

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
   int real;
   Efl_Callback_Array_Item_Full *it;

   if (thdat->name) eina_thread_name_set(t, thdat->name);
   else eina_thread_name_set(t, "Eflthread");

   obj = efl_add_ref(APPTHREAD_CLASS, NULL);
   ad = efl_data_scope_get(obj, APPTHREAD_CLASS);
   efl_threadio_indata_set(obj, thdat->indata);
   efl_threadio_outdata_set(obj, thdat->outdata);
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
   ad->thdat = thdat;

   if (thdat->event_cb)
     {
        for (it = thdat->event_cb; it->func; it++)
          efl_event_callback_priority_add(obj, it->desc, it->priority,
                                          it->func, it->user_data);
     }
   if (thdat->argv) efl_core_command_line_command_array_set(obj, thdat->argv);
   thdat->argv = NULL;
   efl_future_then(obj, efl_loop_job(obj),
                   .success = _efl_loop_arguments_send);
   free(thdat->event_cb);
   thdat->event_cb = NULL;

   ret = efl_loop_begin(obj);
   real = efl_loop_exit_code_process(ret);
   thdat->outdata = efl_threadio_outdata_get(obj);

   memset(&cmd, 0, sizeof(cmd));
   cmd.d.command = CMD_EXITED;
   cmd.d.data = real;
   write(thdat->ctrl.in, &cmd, sizeof(Control_Data));

   efl_unref(obj);

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

static void
_thread_exit_eval(Eo *obj, Efl_Thread_Data *pd)
{
   if ((pd->fd.out == -1) && /*(pd->fd.in == -1) &&*/
       (pd->exit_read) && (!pd->exit_called))
     {
        pd->exit_called = EINA_TRUE;
        if (pd->thdat) efl_threadio_outdata_set(obj, pd->thdat->outdata);
        if (pd->promise)
          {
             Eina_Promise *p = pd->promise;
             int exit_code = efl_task_exit_code_get(obj);
             if ((exit_code != 0) && (!(efl_task_flags_get(obj) &
                                        EFL_TASK_FLAGS_NO_EXIT_CODE_ERROR)))
               eina_promise_reject(p, exit_code + 1000000);
             else eina_promise_resolve(p, eina_value_int_init(exit_code));
          }
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

   memset(&cmd, 0, sizeof(cmd));
   ret = read(pd->ctrl.out, &cmd, sizeof(Control_Data));
   if (ret == sizeof(Control_Data))
     {
        if (cmd.d.command == CMD_EXITED)
          {
             if (!pd->exit_read)
               {
                  Efl_Task_Data *td = efl_data_scope_get(obj, EFL_TASK_CLASS);

                  if (td) td->exit_code = cmd.d.data;
                  pd->exit_read = EINA_TRUE;
                  _thread_exit_eval(obj, pd);
               }
          }
        else if (cmd.d.command == CMD_CALL)
          {
             EFlThreadIOCall func = cmd.d.ptr[0];
             void *data = cmd.d.ptr[1];
             Eina_Free_Cb free_func = cmd.d.ptr[2];
             if (func)
               {
                  Efl_Event event = { obj, NULL, NULL };

                  func(data, &event);
               }
             if (free_func) free_func(data);
          }
        else if (cmd.d.command == CMD_CALL_SYNC)
          {
             EFlThreadIOCallSync func = cmd.d.ptr[0];
             void *data = cmd.d.ptr[1];
             Eina_Free_Cb free_func = cmd.d.ptr[2];
             Control_Reply *rep = cmd.d.ptr[3];
             if (func)
               {
                  Efl_Event event = { obj, NULL, NULL };

                  rep->data = func(data, &event);
               }
             if (free_func) free_func(data);
             eina_semaphore_release(&(rep->sem), 1);
          }
     }
}

//////////////////////////////////////////////////////////////////////////

static Eina_Value
_run_cancel_cb(Efl_Loop_Consumer *consumer, void *data EINA_UNUSED, Eina_Error error)
{
   if (error == ECANCELED) efl_task_end(consumer);

   return eina_value_error_init(error);
}

static void
_run_clean_cb(Efl_Loop_Consumer *consumer EINA_UNUSED,void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Thread_Data *pd = data;

   pd->promise = NULL;
}

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
   if (pd->promise)
     ERR("Thread being destroyed while real worker has  not exited yet.");
   if (pd->thdat)
     {
        eina_thread_join(pd->thread);
//        efl_del(pd->fd.in_handler);
//        efl_del(pd->fd.out_handler);
//        efl_del(pd->ctrl.in_handler);
//        efl_del(pd->ctrl.out_handler);
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

EOLIAN static Eina_Future *
_efl_thread_efl_task_run(Eo *obj, Efl_Thread_Data *pd)
{
   Eina_Thread_Priority pri;
   Thread_Data *thdat;
   const char *name;
   int pipe_to_thread[2];
   int pipe_from_thread[2];
   unsigned int num;
   Efl_Callback_Array_Item_Full *it;
   Efl_Task_Data *td = efl_data_scope_get(obj, EFL_TASK_CLASS);

   if (pd->run) return NULL;
   if (!td) return NULL;
   thdat = calloc(1, sizeof(Thread_Data));
   if (!thdat) return NULL;
   thdat->fd.in = -1;
   thdat->fd.out = -1;
   thdat->ctrl.in = -1;
   thdat->ctrl.out = -1;

   thdat->indata = efl_threadio_indata_get(obj);
   thdat->outdata = efl_threadio_outdata_get(obj);

   // input/output pipes
   if (td->flags & EFL_TASK_FLAGS_USE_STDIN)
     {
        if (pipe(pipe_to_thread) != 0)
          {
             ERR("Can't create to_thread pipe");
             free(thdat);
             return NULL;
          }
     }
   if (td->flags & EFL_TASK_FLAGS_USE_STDOUT)
     {
        if (pipe(pipe_from_thread) != 0)
          {
             ERR("Can't create from_thread pipe");
             close(pipe_to_thread[0]);
             close(pipe_to_thread[1]);
             free(thdat);
             return NULL;
          }
     }
   if (td->flags & EFL_TASK_FLAGS_USE_STDIN)
     {
        thdat->fd.in  = pipe_from_thread[1]; // write - input to parent
        pd->fd.out    = pipe_from_thread[0]; // read - output from child
        eina_file_close_on_exec(thdat->fd.in, EINA_TRUE);
        eina_file_close_on_exec(pd->fd.out, EINA_TRUE);
        if (fcntl(thdat->fd.in, F_SETFL, O_NONBLOCK) < 0)
          ERR("can't set pipe to NONBLOCK");
        if (fcntl(pd->fd.out, F_SETFL, O_NONBLOCK) < 0)
          ERR("can't set pipe to NONBLOCK");
        pd->fd.out_handler =
          efl_add(EFL_LOOP_HANDLER_CLASS, obj,
                  efl_loop_handler_fd_set(efl_added, pd->fd.out),
                  efl_event_callback_add
                    (efl_added, EFL_LOOP_HANDLER_EVENT_READ, _cb_thread_parent_out, obj));
        if (pd->read_listeners > 0)
          efl_loop_handler_active_set(pd->fd.out_handler, EFL_LOOP_HANDLER_FLAGS_READ);
     }
   if (td->flags & EFL_TASK_FLAGS_USE_STDOUT)
     {
        pd->fd.in     = pipe_to_thread  [1]; // write - input to child
        thdat->fd.out = pipe_to_thread  [0]; // read - output from parent
        eina_file_close_on_exec(pd->fd.in, EINA_TRUE);
        eina_file_close_on_exec(thdat->fd.out, EINA_TRUE);
        if (fcntl(thdat->fd.out, F_SETFL, O_NONBLOCK) < 0)
          ERR("can't set pipe to NONBLOCK");
        if (fcntl(pd->fd.in, F_SETFL, O_NONBLOCK) < 0)
          ERR("can't set pipe to NONBLOCK");
        pd->fd.in_handler =
          efl_add(EFL_LOOP_HANDLER_CLASS, obj,
                  efl_loop_handler_fd_set(efl_added, pd->fd.in),
                  efl_event_callback_add
                    (efl_added, EFL_LOOP_HANDLER_EVENT_WRITE, _cb_thread_parent_in, obj));
     }

   // control pipes
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
        return NULL;
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
        return NULL;
     }
   thdat->ctrl.in  = pipe_from_thread[1]; // write - input to parent
   thdat->ctrl.out = pipe_to_thread  [0]; // read - output from parent
   pd->ctrl.in     = pipe_to_thread  [1]; // write - input to child
   pd->ctrl.out    = pipe_from_thread[0]; // read - output from child
   if (fcntl(thdat->ctrl.in, F_SETFL, O_NONBLOCK) < 0)
     ERR("can't set pipe to NONBLOCK");
   if (fcntl(thdat->ctrl.out, F_SETFL, O_NONBLOCK) < 0)
     ERR("can't set pipe to NONBLOCK");
   if (fcntl(pd->ctrl.in, F_SETFL, O_NONBLOCK) < 0)
     ERR("can't set pipe to NONBLOCK");
   if (fcntl(pd->ctrl.out, F_SETFL, O_NONBLOCK) < 0)
     ERR("can't set pipe to NONBLOCK");
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

   {
      Eina_Accessor *acc;
      int i = 0;
      const char *argv;

      acc = efl_core_command_line_command_access(obj);
      if (acc)
        {
           thdat->argv = eina_array_new(1);
           EINA_ACCESSOR_FOREACH(acc, i, argv)
             {
                eina_array_push(thdat->argv, eina_stringshare_add(argv));
             }
        }

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
        while (eina_array_count(thdat->argv)) eina_stringshare_del(eina_array_pop(thdat->argv));
        eina_array_free(thdat->argv);
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
        return NULL;
     }
   pd->thdat = thdat;
   pd->run = EINA_TRUE;
   pd->promise = efl_loop_promise_new(obj);
   return efl_future_then(obj, eina_future_new(pd->promise),
                          .data = pd, .error = _run_cancel_cb, .free = _run_clean_cb);
}

EOLIAN static void
_efl_thread_efl_task_end(Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
{
   if (pd->end_sent) return;
   if (pd->thdat)
     {
        Control_Data cmd;

        pd->end_sent = EINA_TRUE;
        memset(&cmd, 0, sizeof(cmd));
        cmd.d.command = CMD_EXIT;
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
_efl_thread_efl_io_closer_closed_get(const Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
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
   efl_event_callback_call(obj, EFL_IO_READER_EVENT_CAN_READ_CHANGED, (void*) (uintptr_t) can_read);
}

EOLIAN static Eina_Bool
_efl_thread_efl_io_reader_can_read_get(const Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
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
_efl_thread_efl_io_reader_eos_get(const Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
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
   efl_event_callback_call(obj, EFL_IO_WRITER_EVENT_CAN_WRITE_CHANGED, (void*) (uintptr_t) can_write);
}

EOLIAN static Eina_Bool
_efl_thread_efl_io_writer_can_write_get(const Eo *obj EINA_UNUSED, Efl_Thread_Data *pd)
{
   return pd->fd.can_write;
}

void
_appthread_threadio_call(Eo *obj EINA_UNUSED, Efl_Appthread_Data *pd,
                         void *func_data, EFlThreadIOCall func, Eina_Free_Cb func_free_cb)
{
   Thread_Data *thdat = pd->thdat;
   Control_Data cmd;

   memset(&cmd, 0, sizeof(cmd));
   cmd.d.command = CMD_CALL;
   cmd.d.ptr[0] = func;
   cmd.d.ptr[1] = func_data;
   cmd.d.ptr[2] = func_free_cb;
   write(thdat->ctrl.in, &cmd, sizeof(Control_Data));
}

EOLIAN static void
_efl_thread_efl_threadio_call(Eo *obj EINA_UNUSED, Efl_Thread_Data *pd,
                              void *func_data, EFlThreadIOCall func, Eina_Free_Cb func_free_cb)
{
   Control_Data cmd;

   memset(&cmd, 0, sizeof(cmd));
   cmd.d.command = CMD_CALL;
   cmd.d.ptr[0] = func;
   cmd.d.ptr[1] = func_data;
   cmd.d.ptr[2] = func_free_cb;
   write(pd->ctrl.in, &cmd, sizeof(Control_Data));
}

void *
_appthread_threadio_call_sync(Eo *obj EINA_UNUSED, Efl_Appthread_Data *pd,
                              void *func_data, EFlThreadIOCallSync func, Eina_Free_Cb func_free_cb)
{
   Thread_Data *thdat = pd->thdat;
   Control_Data cmd;
   Control_Reply *rep;
   void *data;

   memset(&cmd, 0, sizeof(cmd));
   cmd.d.command = CMD_CALL_SYNC;
   cmd.d.ptr[0] = func;
   cmd.d.ptr[1] = func_data;
   cmd.d.ptr[2] = func_free_cb;
   rep = malloc(sizeof(Control_Reply));
   if (!rep) return NULL;
   cmd.d.ptr[3] = rep;
   rep->data = NULL;
   eina_semaphore_new(&(rep->sem), 0);
   write(thdat->ctrl.in, &cmd, sizeof(Control_Data));
   eina_semaphore_lock(&(rep->sem));
   data = rep->data;
   free(rep);
   return data;
}

EOLIAN static void *
_efl_thread_efl_threadio_call_sync(Eo *obj EINA_UNUSED, Efl_Thread_Data *pd,
                                   void *func_data, EFlThreadIOCallSync func, Eina_Free_Cb func_free_cb)
{
   Control_Data cmd;
   Control_Reply *rep;
   void *data;

   memset(&cmd, 0, sizeof(cmd));
   cmd.d.command = CMD_CALL_SYNC;
   cmd.d.ptr[0] = func;
   cmd.d.ptr[1] = func_data;
   cmd.d.ptr[2] = func_free_cb;
   rep = malloc(sizeof(Control_Reply));
   if (!rep) return NULL;
   cmd.d.ptr[3] = rep;
   rep->data = NULL;
   eina_semaphore_new(&(rep->sem), 0);
   write(pd->ctrl.in, &cmd, sizeof(Control_Data));
   eina_semaphore_lock(&(rep->sem));
   data = rep->data;
   free(rep);
   return data;
}

//////////////////////////////////////////////////////////////////////////

#include "efl_thread.eo.c"
