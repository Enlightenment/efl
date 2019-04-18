#define EFL_BETA_API_SUPPORT

#include <stdio.h>
#include <string.h>

#include <Eina.h>
#include <Eo.h>
#include <Efl_Core.h>

static void _th_read_change(void *data EINA_UNUSED, const Efl_Event *ev);
static void _th_main(void *data EINA_UNUSED, const Efl_Event *ev);
static void _read_change(void *data EINA_UNUSED, const Efl_Event *ev);
static Eina_Value _task_exit(void *data, Eina_Value v, const Eina_Future *dead EINA_UNUSED);

////////////////////////////////////////////////////////////////////////////
//// thread side of code
static void
_th_timeout(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eo *obj = data;

   printf("--- START EXIT [%p]\n", obj);
   efl_threadio_outdata_set(obj, (void *)0x9876);
   efl_loop_quit(obj, eina_value_int_init(99));
}

static void
_th_read_change(void *data EINA_UNUSED, const Efl_Event *ev)
{
   // read input from parent thread/loop status chnaged - read what we can
   Eo *obj = ev->object;
   char buf[4096];
   Eina_Rw_Slice rw_slice = EINA_SLICE_ARRAY(buf);

   while (efl_io_reader_can_read_get(obj))
     {
        Eina_Error err = efl_io_reader_read(obj, &rw_slice);
        if (!err)
          {
             buf[rw_slice.len] = 0;
             printf("--- TH READ [%p] [%s] ok %i bytes '%s'\n", obj, efl_core_command_line_command_get(obj), (int)rw_slice.len, buf);

             char *buf2 = "yes-im-here ";
             Eina_Slice slice = { strlen(buf2), buf2 };
             Eina_Error err = efl_io_writer_write(obj, &slice, NULL);
             if (!err)
               {
                  Eina_Accessor *args_access = efl_core_command_line_command_access(obj);
                  printf("--- TH WRITE [%p] [%s] ok %i bytes\n", obj, efl_core_command_line_command_get(obj), (int)slice.len);
                  void *s = "";
                  eina_accessor_data_get(args_access, 1, &s);
                  if (!strcmp(s, "one"))
                    efl_add(EFL_LOOP_TIMER_CLASS, obj,
                            efl_loop_timer_interval_set(efl_added, 2.0),
                            efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _th_timeout, obj));
                  else
                    efl_add(EFL_LOOP_TIMER_CLASS, obj,
                            efl_loop_timer_interval_set(efl_added, 1.0),
                            efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _th_timeout, obj));
                  eina_accessor_free(args_access);
               }
          }
     }
}

static void
_th_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
   // the "main function" of the thread thayt gets called with arguments
   // just like eflm_main for the main loop
   Eo *obj = ev->object;
   Eina_Accessor *args_access = efl_core_command_line_command_access(obj);
   void *s = "", *ss = "";
   eina_accessor_data_get(args_access, 0, &s);
   eina_accessor_data_get(args_access, 1, &ss);
   printf("--- TH main %p, '%s' '%s' indata=%p\n", obj, s, ss, efl_threadio_indata_get(obj));
   efl_event_callback_add
     (obj, EFL_IO_READER_EVENT_CAN_READ_CHANGED, _th_read_change, NULL);
   if (!strcmp(s, "one"))
     {
        Eina_Array *args = eina_array_new(1);
        eina_array_push(args, eina_stringshare_add("number"));
        eina_array_push(args, eina_stringshare_add("one"));
        Eo *obj2 = efl_add(EFL_THREAD_CLASS, obj,
                           efl_threadio_indata_set(efl_added, (void *)0x1234),
                           efl_core_command_line_command_array_set(efl_added, args),
                           efl_task_flags_set(efl_added, EFL_TASK_FLAGS_USE_STDOUT | EFL_TASK_FLAGS_USE_STDIN),
                           efl_event_callback_add(efl_added, EFL_LOOP_EVENT_ARGUMENTS, _th_main, NULL),
                           efl_event_callback_add(efl_added, EFL_IO_READER_EVENT_CAN_READ_CHANGED, _read_change, NULL),
                           eina_future_then(efl_task_run(efl_added), _task_exit, efl_added)
                          );

        char *buf2 = "hello-out-there2 ";
        Eina_Slice slice = { strlen(buf2), buf2 };
        Eina_Error err = efl_io_writer_write(obj2, &slice, NULL);
        if (!err) printf("--- WRITE [%p] [%s] ok %i bytes\n", obj2, efl_core_command_line_command_get(obj), (int)slice.len);
     }
   eina_accessor_free(args_access);
}

////////////////////////////////////////////////////////////////////////////
//// main loop side of code
static void
_read_change(void *data EINA_UNUSED, const Efl_Event *ev)
{
   // read output from thread status chnaged - read what we can
   Eo *obj = ev->object;
   char buf[4096];
   Eina_Rw_Slice rw_slice = EINA_SLICE_ARRAY(buf);

   while (efl_io_reader_can_read_get(obj))
     {
        Eina_Error err = efl_io_reader_read(obj, &rw_slice);
        if (!err)
          {
             buf[rw_slice.len] = 0;
             printf("--- READ [%p] [%s] ok %i bytes '%s'\n", obj, efl_core_command_line_command_get(obj), (int)rw_slice.len, buf);
          }
     }
}

static Eina_Value
_task_exit(void *data, Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   // called when the task says it has completed and exited.
   // all output to read has stopped
   Eo *obj = data;
   printf("--- [%p] EXITED exit_code=%i outdata=%p\n", obj, efl_task_exit_code_get(obj), efl_threadio_outdata_get(obj));
   efl_del(obj);
   return v;
}

////////////////////////////////////////////////////////////////////////////
// just main loop input handling
static void
_stdin_read_change(void *data EINA_UNUSED, const Efl_Event *ev)
{
   // read output from thread status chnaged - read what we can
   Eo *obj = ev->object;
   char buf[4096];
   Eina_Rw_Slice rw_slice = EINA_SLICE_ARRAY(buf);

   while (efl_io_reader_can_read_get(obj))
     {
        Eina_Error err = efl_io_reader_read(obj, &rw_slice);
        if (!err)
          {
             buf[rw_slice.len] = 0;
             printf("--- STDIN READ [%p] [%s] ok %i bytes '%s'\n", obj, efl_core_command_line_command_get(obj), (int)rw_slice.len, buf);
          }
     }
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eo *app = ev->object;
   int threads = 2, i;
   Eina_Accessor *args_access = efl_core_command_line_command_access(app);
   void *s;

   const Efl_Version *v = efl_app_build_efl_version_get(app);
   printf("--- EFL %i.%i.%i\n", v->major, v->minor, v->micro);
   s = NULL;
   eina_accessor_data_get(args_access, 2, &s);
   if (s && (!strcmp(s, "-stdinwatch")))
     efl_event_callback_add(app, EFL_IO_READER_EVENT_CAN_READ_CHANGED,
                            _stdin_read_change, NULL);
   s = NULL;
   eina_accessor_data_get(args_access, 1, &s);
   if (s) threads = atoi(s);
   for (i = 0; i < threads; i++)
     {
        Eina_Array *args = eina_array_new(1);
        eina_array_push(args, eina_stringshare_add("number"));
        eina_array_push(args, eina_stringshare_add("one"));
        Eo *obj = efl_add(EFL_THREAD_CLASS, app,
                          efl_threadio_indata_set(efl_added, (void *)0x5678),
                          efl_core_command_line_command_array_set(efl_added, args),
                          efl_task_flags_set(efl_added, EFL_TASK_FLAGS_USE_STDOUT | EFL_TASK_FLAGS_USE_STDIN),
                          efl_event_callback_add(efl_added, EFL_LOOP_EVENT_ARGUMENTS, _th_main, NULL),
                          efl_event_callback_add(efl_added, EFL_IO_READER_EVENT_CAN_READ_CHANGED, _read_change, NULL),
                          eina_future_then(efl_task_run(efl_added), _task_exit, efl_added)
                         );

        char *buf2 = "hello-out-there ";
        Eina_Slice slice = { strlen(buf2), buf2 };
        Eina_Error err = efl_io_writer_write(obj, &slice, NULL);
        if (!err) printf("--- WRITE [%p] [%s] ok %i bytes\n", obj, efl_core_command_line_command_get(obj), (int)slice.len);
     }
}
EFL_MAIN()
