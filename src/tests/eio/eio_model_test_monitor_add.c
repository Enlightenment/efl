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

void _children_removed_get_value_cb(void* data EINA_UNUSED, Efl_Event const* event)
{
  Efl_Future_Event_Success* info = event->info;
  Eina_Value* value = info->value;
  char* filename = eina_value_to_string(value);
  if(temp_filename && strcmp(filename, temp_filename) == 0)
    ecore_main_loop_quit();
  free(filename);

  ecore_main_loop_quit();
}

static void
_children_removed_cb(void *data EINA_UNUSED, const Efl_Event* event)
{
   if(children_deleted)
     {
        Efl_Model_Children_Event* evt = event->info;
        Efl_Future* future;

        future = efl_model_property_get(evt->child, "path");

        efl_future_then(future, _children_removed_get_value_cb, NULL, NULL, NULL);
     }
}

struct _added_get_value_data
{
  Eo *object, *child;
};
typedef struct _added_get_value_data added_get_value_data;

void _children_added_get_value_cb(void* d, Efl_Event const* event)
{
  Efl_Future_Event_Success* info = event->info;
  Eina_Value* value = info->value;
  added_get_value_data* data = d;

   char* filename = eina_value_to_string(value);

   if(temp_filename && strcmp(temp_filename, filename) == 0)
     {
        children_deleted = EINA_TRUE;
        efl_model_child_del(data->object, data->child);
     }
   free(filename);
}

static void
_children_added_cb(void *d EINA_UNUSED, const Efl_Event* event)
{
   Efl_Model_Children_Event* evt = event->info;
   Efl_Future* future;
   
   future = efl_model_property_get(evt->child, "path");

   added_get_value_data* data = malloc(sizeof(added_get_value_data));
   data->object = event->object;
   data->child = evt->child;
   efl_future_then(future, _children_added_get_value_cb, NULL, NULL, data);
}

static void
_create_file(void *data EINA_UNUSED, Efl_Event const* event EINA_UNUSED)
{
   int fd;
   if((fd = eina_file_mkstemp("prefixXXXXXX.ext", &temp_filename)) > 0)
     {
        close(fd);
     }
}


static void
_create_file_error(void *data EINA_UNUSED, Efl_Event const* event EINA_UNUSED)
{
   ck_abort_msg(0, "Error Promise cb called in Create file");
   ecore_main_loop_quit();
}

START_TEST(eio_model_test_test_monitor_add)
{
   Eo *filemodel = NULL;

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!efl_object_init(), "ERROR: Cannot init EO!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   tmpdir = eina_environment_tmp_get();

   filemodel = efl_add(EIO_MODEL_CLASS, ecore_main_loop_get(), eio_model_path_set(efl_added, tmpdir));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   efl_event_callback_add(filemodel, EFL_MODEL_EVENT_CHILD_ADDED, &_children_added_cb, filemodel);
   efl_event_callback_add(filemodel, EFL_MODEL_EVENT_CHILD_REMOVED, &_children_removed_cb, NULL);

   Efl_Future* future;
   future = efl_model_children_slice_get(filemodel, 0, 0);

   efl_future_then(future, &_create_file, &_create_file_error, NULL, NULL);

   ecore_main_loop_begin();

   efl_unref(filemodel);

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

