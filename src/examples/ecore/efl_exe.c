#include <stdio.h>
#include <string.h>

#include <Eina.h>
#include <Eo.h>
#include <Efl_Core.h>

static void _read_change(void *data EINA_UNUSED, const Efl_Event *ev);
static void _task_exit(void *data EINA_UNUSED, const Efl_Event *ev);

static void
_read_change(void *data EINA_UNUSED, const Efl_Event *ev)
{
   // read output from exe status changed - read what we can
   Eo *obj = ev->object;
   char buf[4096];
   Eina_Rw_Slice rw_slice = EINA_SLICE_ARRAY(buf);

   while (efl_io_reader_can_read_get(obj))
     {
        Eina_Error err = efl_io_reader_read(obj, &rw_slice);
        if (!err)
          {
             buf[rw_slice.len - 1] = 0;
             printf("--- READ [%p] [%s] ok %i bytes '%s'\n", obj, efl_core_command_line_command_get(obj), (int)rw_slice.len, buf);
          }
     }
}

static void
_task_exit(void *data EINA_UNUSED, const Efl_Event *ev)
{
   // called when the task says it has completed and exited.
   // all output to read has stopped
   Eo *obj = ev->object;
   printf("--- [%p] EXITED exit_code=%i\n", obj, efl_task_exit_code_get(obj));
   efl_loop_quit(efl_provider_find(obj, EFL_LOOP_CLASS), eina_value_int_init(99));
   // exe auto deleted at this point like efl threads. more convenient as
   // you don't need to remember to delete them yourself if launching
   // lots of commands - this is how ecore_exe worked. so listen to the
   // exit event (or del event) if you care about this... or ref it to keep
   // it around longer.
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eo *app = ev->object;

   const Efl_Version *v = efl_app_build_efl_version_get(app);
   printf("--- EFL %i.%i.%i\n", v->major, v->minor, v->micro);
   Eina_Array *args = eina_array_new(1);
   eina_array_push(args, eina_stringshare_add("./efl_exe_test.sh"));
   Eo *env = efl_duplicate(efl_env_self());
   efl_core_env_set(env, "BLAH", "blahvalue");
   Eo *obj = efl_add(EFL_EXE_CLASS, app,
                     efl_core_command_line_command_array_set(efl_added, args),
                     efl_exe_env_set(efl_added, env),
                     efl_task_flags_set(efl_added, EFL_TASK_FLAGS_USE_STDOUT | EFL_TASK_FLAGS_USE_STDIN),
                     efl_event_callback_add(efl_added, EFL_IO_READER_EVENT_CAN_READ_CHANGED, _read_change, NULL),
                     efl_event_callback_add(efl_added, EFL_TASK_EVENT_EXIT, _task_exit, NULL),
                     efl_task_run(efl_added)
                    );
   efl_unref(env);

   char *buf2 = "sample-input\n";
   Eina_Slice slice = { strlen(buf2), .mem = buf2 };
   Eina_Error err = efl_io_writer_write(obj, &slice, NULL);
   if (!err) printf("--- WRITE [%p] [%s] ok %i bytes\n", obj, efl_core_command_line_command_get(obj), (int)slice.len);
}
EFL_MAIN()
