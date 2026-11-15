#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#define EFL_BETA_API_SUPPORT
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

typedef struct
{
   Eina_Strbuf *collected;
   int exit_code;
   Eina_Bool exited;
} Exe_Run;

/* Drain, rather than read once. The exit event does not arrive until the
 * child's stdout has been read to its end -- that is how Efl.Exe knows there
 * is nothing left to hand over -- so a handler that takes one chunk per
 * notification and stops leaves the task looking as though it were still
 * running.
 */
static void
_exe_data(void *data, const Efl_Event *ev)
{
   Exe_Run *run = data;

   while (efl_io_reader_can_read_get(ev->object))
     {
        char buf[256];
        Eina_Rw_Slice slice = { sizeof(buf), { buf } };

        if (efl_io_reader_read(ev->object, &slice) != 0) return;
        eina_strbuf_append_length(run->collected, buf, slice.len);
     }
}

/* The exit code is read here rather than after the loop: an Efl.Exe deletes
 * itself once it has exited, the way ecore_exe always did, so this event is
 * the last moment the object exists.
 */
static void
_exe_exited(void *data, const Efl_Event *ev)
{
   Exe_Run *run = data;
   run->exit_code = efl_task_exit_code_get(ev->object);
   run->exited = EINA_TRUE;
   efl_loop_quit(efl_main_loop_get(), eina_value_int_init(0));
}

/* Run something, read what it wrote. The command line comes from
 * Efl.Core.Command_Line, which Efl.Exe implements.
 */
EFL_START_TEST(efl_exe_test_reads_child_output)
{
   Eo *exe;
   Exe_Run run = { eina_strbuf_new(), -1, EINA_FALSE };

   exe = efl_add(EFL_EXE_CLASS, efl_main_loop_get(),
                 efl_core_command_line_command_string_set(efl_added,
                    "/bin/sh -c 'printf hello'"),
                 /* without this the child inherits our stdout and there is
                  * nothing to read */
                 efl_task_flags_set(efl_added, EFL_TASK_FLAGS_USE_STDOUT),
                 efl_event_callback_add(efl_added, EFL_IO_READER_EVENT_CAN_READ_CHANGED,
                                        _exe_data, &run),
                 efl_event_callback_add(efl_added, EFL_TASK_EVENT_EXIT,
                                        _exe_exited, &run));
   fail_if(!exe);
   efl_task_run(exe);

   efl_loop_begin(efl_main_loop_get());

   ck_assert(run.exited);
   ck_assert_str_eq(eina_strbuf_string_get(run.collected), "hello");
   ck_assert_int_eq(run.exit_code, 0);

   /* no efl_del here: the class deletes itself once it has exited */
   eina_strbuf_free(run.collected);
}
EFL_END_TEST

/* A child sees the Efl.Core.Env it was given, and nothing of the parent's. */
EFL_START_TEST(efl_exe_test_env_reaches_the_child)
{
   Eo *exe, *env;
   Exe_Run run = { eina_strbuf_new(), -1, EINA_FALSE };

   /* start from this process's environment, so the child still has a PATH,
      and add one variable to it */
   env = efl_duplicate(efl_env_self());
   efl_core_env_set(env, "EFL_EXE_TEST_VAR", "visible");

   exe = efl_add(EFL_EXE_CLASS, efl_main_loop_get(),
                 efl_core_command_line_command_string_set(efl_added,
                    "/bin/sh -c 'printf %s $EFL_EXE_TEST_VAR'"),
                 efl_task_flags_set(efl_added, EFL_TASK_FLAGS_USE_STDOUT),
                 efl_exe_env_set(efl_added, env),
                 efl_event_callback_add(efl_added, EFL_IO_READER_EVENT_CAN_READ_CHANGED,
                                        _exe_data, &run),
                 efl_event_callback_add(efl_added, EFL_TASK_EVENT_EXIT,
                                        _exe_exited, &run));
   fail_if(!exe);
   efl_task_run(exe);

   efl_loop_begin(efl_main_loop_get());

   ck_assert(run.exited);
   ck_assert_str_eq(eina_strbuf_string_get(run.collected), "visible");

   eina_strbuf_free(run.collected);
   efl_unref(env);
}
EFL_END_TEST

/* Replacing the environment of an Efl.Exe lets go of the previous one. */
EFL_START_TEST(efl_exe_test_env_set_releases_previous)
{
   Eo *exe, *first, *second;
   Eo *first_wref = NULL;

   exe = efl_add(EFL_EXE_CLASS, efl_main_loop_get(),
                 efl_core_command_line_command_string_set(efl_added, "/bin/true"));
   fail_if(!exe);

   first = efl_duplicate(efl_env_self());
   efl_wref_add(first, &first_wref);
   efl_exe_env_set(exe, first);
   efl_unref(first);
   ck_assert_ptr_nonnull(first_wref);

   second = efl_duplicate(efl_env_self());
   efl_exe_env_set(exe, second);
   efl_unref(second);
   ck_assert_ptr_null(first_wref);

   efl_del(exe);
}
EFL_END_TEST

/* The two Efl.Io.Closer properties the class had never answered for. */
EFL_START_TEST(efl_exe_test_closer_properties)
{
   Eo *exe;
   Exe_Run run = { NULL, -1, EINA_FALSE };

   exe = efl_add(EFL_EXE_CLASS, efl_main_loop_get(),
                 efl_core_command_line_command_string_set(efl_added, "/bin/true"),
                 efl_event_callback_add(efl_added, EFL_TASK_EVENT_EXIT,
                                        _exe_exited, &run));
   fail_if(!exe);

   /* both default to true */
   ck_assert(efl_io_closer_close_on_exec_get(exe));
   ck_assert(efl_io_closer_close_on_invalidate_get(exe));

   /* the pipes to the child are not for anything this process execs, so the
    * setter refuses rather than pretending; the safety log is the test working
    */
   ck_assert(!efl_io_closer_close_on_exec_set(exe, EINA_FALSE));
   ck_assert(efl_io_closer_close_on_exec_get(exe));

   efl_io_closer_close_on_invalidate_set(exe, EINA_FALSE);
   ck_assert(!efl_io_closer_close_on_invalidate_get(exe));
   efl_io_closer_close_on_invalidate_set(exe, EINA_TRUE);

   efl_task_run(exe);
   efl_loop_begin(efl_main_loop_get());
   ck_assert(run.exited);
   ck_assert_int_eq(run.exit_code, 0);
}
EFL_END_TEST

void
efl_app_test_efl_exe(TCase *tc)
{
   tcase_add_test(tc, efl_exe_test_reads_child_output);
   tcase_add_test(tc, efl_exe_test_env_reaches_the_child);
   tcase_add_test(tc, efl_exe_test_env_set_releases_previous);
   tcase_add_test(tc, efl_exe_test_closer_properties);
}
