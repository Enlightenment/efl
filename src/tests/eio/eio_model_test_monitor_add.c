//Compile with:
// gcc -o emodel_test_file emodel_test_file.c `pkg-config --cflags --libs emodel`
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Eio.h>
#include <Ecore.h>
#include <Efl.h>
#include <stdio.h>

#include <check.h>

Eina_Tmpstr* temp_filename = NULL;
const char* tmpdir = NULL;
Eina_Bool children_deleted = EINA_FALSE;

struct _pair
{
  Eo *parent, *child;
};

static void
_children_removed_cb(void *data EINA_UNUSED, const Eo_Event* event)
{
   fprintf(stderr, __FILE__ ":%d %s\n", __LINE__, __func__);
   if(children_deleted)
     {
        Efl_Model_Children_Event* evt = event->info;

        Eina_Promise* promise;
        promise = efl_model_property_get(evt->child, "path");
        Eina_Value const* value = eina_promise_value_get(promise);
        char* filename = eina_value_to_string(value);

        if(temp_filename && strcmp(filename, temp_filename) == 0)
             ecore_main_loop_quit();
        free(filename);
        eina_promise_unref(promise);
     }
}

static void
_children_added_cb(void *data EINA_UNUSED, const Eo_Event* event)
{
   fprintf(stderr, __FILE__ ":%d %s\n", __LINE__, __func__);
   Efl_Model_Children_Event* evt = event->info;

   Eina_Promise* promise;
   promise = efl_model_property_get(evt->child, "path");
   Eina_Value const* value = eina_promise_value_get(promise);
   char* filename = eina_value_to_string(value);

   if(temp_filename && strcmp(temp_filename, filename) == 0)
     {
        children_deleted = EINA_TRUE;
        efl_model_child_del(event->object, evt->child);
     }
   free(filename);

   eina_promise_unref(promise);
}

static void
_create_file(void *data EINA_UNUSED, void* value EINA_UNUSED)
{
   int fd;
   if((fd = eina_file_mkstemp("prefixXXXXXX.ext", &temp_filename)) > 0)
     {
        fprintf(stderr, __FILE__ ":%d %s\n", __LINE__, __func__);
        close(fd);
     }
}


static void
_create_file_error(void *data EINA_UNUSED, Eina_Error value EINA_UNUSED)
{
   ck_abort_msg(0, "Error Promise cb called in Create file");
   ecore_main_loop_quit();
}

START_TEST(eio_model_test_test_monitor_add)
{
   Eo *filemodel = NULL;

   fprintf(stderr, "efl_model_test_test_monitor_add\n");

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!eo_init(), "ERROR: Cannot init EO!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   tmpdir = eina_environment_tmp_get();

   filemodel = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(eo_self, tmpdir));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   efl_event_callback_add(filemodel, EFL_MODEL_EVENT_CHILD_ADDED, &_children_added_cb, filemodel);
   efl_event_callback_add(filemodel, EFL_MODEL_EVENT_CHILD_REMOVED, &_children_removed_cb, NULL);

   Eina_Promise* promise;
   promise = efl_model_children_slice_get(filemodel, 0, 0);

   eina_promise_then(promise, &_create_file, &_create_file_error, NULL);

   ecore_main_loop_begin();

   eo_unref(filemodel);

   eio_shutdown();
   ecore_shutdown();
   eina_shutdown();

   fail_if(!children_deleted);
}
END_TEST

void
eio_model_test_monitor_add(TCase *tc)
{
   tcase_add_test(tc, eio_model_test_test_monitor_add);
}

