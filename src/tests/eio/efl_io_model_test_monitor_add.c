//Compile with:
// gcc -o emodel_test_file emodel_test_file.c `pkg-config --cflags --libs emodel`
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Eio.h>
#include <Ecore.h>
#include <Efl.h>
#include <Ecore_File.h>
#include <stdio.h>

#include "eio_suite.h"
#include "eio_test_common.h"

/* A directory of this test's own.
 *
 * It used to watch eina_environment_tmp_get() -- /tmp itself -- and wait for
 * its own file to be reported among everything else happening in there. That
 * is fine on an idle machine and a lottery during a parallel test run, where
 * a dozen other suites are making and removing temporary files and
 * directories in the same place: the event this test is waiting for has to
 * find its way through all of theirs, and when it does not, ecore_main_loop_begin
 * never returns and the whole suite dies on a timeout with nothing to say
 * about why. It was the one suite-level hang anybody had seen.
 *
 * Watching somewhere nobody else touches makes it about the monitor again.
 */
static Eina_Tmpstr *test_dirname = NULL;
Eina_Tmpstr* temp_filename = NULL;
Eina_Bool children_deleted = EINA_FALSE;
static Ecore_Timer *give_up = NULL;

// This will ensure that the child have the time to be automatically destroyed
static Eina_Value
_delayed_quit(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   ecore_main_loop_quit();

   return v;
}

static Eina_Value
_children_got(Eo *o, void *data EINA_UNUSED, const Eina_Value v)
{
   Eo *child = NULL;
   unsigned int i, len;
   Eina_Value r = (Eina_Value) v;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *path;
        char *str;

        path = efl_model_property_get(child, "path");
        fail_if(path == NULL);
        str = eina_value_to_string(path);
        fail_if(str == NULL);

        if (temp_filename && strcmp(str, temp_filename) == 0)
          r = eina_future_as_value(efl_future_then(efl_loop_get(o),
                                                   efl_loop_job(efl_loop_get(o)),
                                                   .success = _delayed_quit));


        free(str);
        eina_value_free(path);
     }

   return r;
}

static Eina_Value
_children_failed(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Error err)
{
   if (err != ECANCELED)
     ck_abort_msg( "Failed to get the child with '%s'.\n", eina_error_msg_get(err));
   return eina_value_error_init(err);
}

static void
_children_removed_cb(void *data EINA_UNUSED, const Efl_Event* event)
{
   Efl_Model_Children_Event* evt = event->info;
   Eina_Value *path;
   char *str;

   if (!evt->child)
     {
        Eina_Future *f;

        // Last time we can fetch the object
        f = efl_future_then(event->object,
                            efl_model_children_slice_get(event->object, evt->index, 1),
                            .success = _children_got,
                            .error = _children_failed,
                            .success_type = EINA_VALUE_TYPE_ARRAY);
        fail_if(f == NULL);
        return;
     }

   fail_if(evt->child == NULL);
   path = efl_model_property_get(evt->child, "path");
   fail_if(path == NULL);
   str = eina_value_to_string(path);
   fail_if(str == NULL);

   if (temp_filename && strcmp(str, temp_filename) == 0)
     efl_future_then(efl_loop_get(evt->child),
                     efl_loop_job(efl_loop_get(evt->child)),
                     .success = _delayed_quit);

   free(str);
   eina_value_free(path);
}

static Eina_Bool started_up = EINA_FALSE;

/* Waiting for an event that never comes is a failure, not a hang: say so
   here rather than leaving the suite to be killed from outside. */
static Eina_Bool
_give_up_cb(void *data EINA_UNUSED)
{
   give_up = NULL;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

/* The file the monitor is supposed to notice, made once the model is up and
   listening rather than from the first thing the listing happens to report --
   an empty directory reports nothing, and there would be nothing to start
   from. */
static Eina_Value
_make_watched_file(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   char templatename[PATH_MAX];
   int fd;

   snprintf(templatename, sizeof(templatename), "%s/prefixXXXXXX.ext",
            (const char *) test_dirname);
   fd = eina_file_mkstemp(templatename, &temp_filename);
   ck_assert_int_gt(fd, 0);
   close(fd);

   started_up = EINA_TRUE;

   return v;
}

static Eina_Value
_children_get(void *data,
              const Eina_Value v,
              const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Model *filemodel = data;
   Eo *child = NULL;
   unsigned int i, len;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err = 0;

        fail_if(!eina_value_error_get(&v, &err));
        fail_if(err != ECANCELED);
        return v;
     }

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *path;
        char *str;

        path = efl_model_property_get(child, "path");
        fail_if(path == NULL);
        str = eina_value_to_string(path);
        fail_if(str == NULL);

        if (started_up && strcmp(temp_filename, str) == 0)
          {
             children_deleted = EINA_TRUE;
             efl_model_child_del(filemodel, child);
          }
        free(str);
        eina_value_free(path);
     }

   return v;
}

static void
_children_added_cb(void *d EINA_UNUSED, const Efl_Event* event)
{
   Efl_Model_Children_Event* evt = event->info;
   Eina_Future *future;

   future = efl_model_children_slice_get(event->object, evt->index, 1);
   eina_future_then(future, _children_get, event->object, NULL);
}

EFL_START_TEST(efl_io_model_test_test_monitor_add)
{
   Eo *filemodel = NULL;

   test_dirname = get_eio_test_file_tmp_dir();
   fail_if(test_dirname == NULL);

   started_up = EINA_FALSE;
   children_deleted = EINA_FALSE;
   temp_filename = NULL;

   filemodel = efl_add(EFL_IO_MODEL_CLASS,
                       efl_main_loop_get(),
                       efl_io_model_path_set(efl_added, test_dirname));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-extra-args"
   fail_if(!filemodel, "ERROR: Cannot init model!\n");
#pragma GCC diagnostic pop

   efl_event_callback_add(filemodel, EFL_MODEL_EVENT_CHILD_ADDED, &_children_added_cb, filemodel);
   efl_event_callback_add(filemodel, EFL_MODEL_EVENT_CHILD_REMOVED, &_children_removed_cb, NULL);

   /* Asking for the count is what starts the listing and puts the monitor in
      place; the file goes in once that has happened. */
   efl_model_children_count_get(filemodel);
   efl_future_then(filemodel, efl_loop_job(efl_loop_get(filemodel)),
                   .success = _make_watched_file);

   give_up = ecore_timer_add(60.0, _give_up_cb, NULL);

   ecore_main_loop_begin();

   ecore_timer_del(give_up);
   give_up = NULL;

   efl_del(filemodel);

   fail_if(!started_up, "the file the monitor should have seen was never made");
   fail_if(!children_deleted, "the monitor never reported the file being added");

   if (temp_filename) unlink(temp_filename);
   ecore_file_recursive_rm(test_dirname);
   eina_tmpstr_del(test_dirname);
   test_dirname = NULL;
}
EFL_END_TEST

void
efl_io_model_test_monitor_add(TCase *tc)
{
   tcase_add_test(tc, efl_io_model_test_test_monitor_add);
}
