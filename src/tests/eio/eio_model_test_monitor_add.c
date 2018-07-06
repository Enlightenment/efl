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

#include "eio_suite.h"

Eina_Tmpstr* temp_filename = NULL;
const char* tmpdir = NULL;
Eina_Bool children_deleted = EINA_FALSE;

static void
_children_removed_cb(void *data EINA_UNUSED, const Efl_Event* event)
{
   Efl_Model_Children_Event* evt = event->info;
   Eina_Value *path;
   char *str;

   fail_if(evt->child == NULL);
   path = efl_model_property_get(evt->child, "path");
   fail_if(path == NULL);
   str = eina_value_to_string(path);
   fail_if(str == NULL);

   if (temp_filename && strcmp(str, temp_filename) == 0)
     ecore_main_loop_quit();

   free(str);
   eina_value_free(path);
}

static Eina_Value
_children_get(void *data,
              const Eina_Value v,
              const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Model *filemodel = data;
   Eo *child = NULL;
   unsigned int i, len;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *path;
        char *str;

        path = efl_model_property_get(child, "path");
        fail_if(path == NULL);
        str = eina_value_to_string(path);
        fail_if(str == NULL);

        if (strcmp(temp_filename, str) == 0)
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
   eina_future_then(future, _children_get, event->object);
}

static Eina_Value
_create_file(void *data EINA_UNUSED,
             const Eina_Value v,
             const Eina_Future *dead_future EINA_UNUSED)
{
   int fd;

   if((fd = eina_file_mkstemp("prefixXXXXXX.ext", &temp_filename)) > 0)
     {
        close(fd);
     }

   return v;
}

EFL_START_TEST(eio_model_test_test_monitor_add)
{
   Eo *filemodel = NULL;
   Eina_Future* future;

   tmpdir = eina_environment_tmp_get();

   filemodel = efl_add(EIO_MODEL_CLASS,
                       efl_main_loop_get(),
                       eio_model_path_set(efl_added, tmpdir));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   efl_event_callback_add(filemodel, EFL_MODEL_EVENT_CHILD_ADDED, &_children_added_cb, filemodel);
   efl_event_callback_add(filemodel, EFL_MODEL_EVENT_CHILD_REMOVED, &_children_removed_cb, NULL);

   future = efl_model_children_slice_get(filemodel, 0, efl_model_children_count_get(filemodel));

   eina_future_then(future, &_create_file, NULL);

   ecore_main_loop_begin();

   efl_del(filemodel);

   fail_if(!children_deleted);
}
EFL_END_TEST

void
eio_model_test_monitor_add(TCase *tc)
{
   tcase_add_test(tc, eio_model_test_test_monitor_add);
}
