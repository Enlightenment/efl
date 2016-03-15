//Compile with:
// gcc -o emodel_test_file emodel_test_file.c `pkg-config --cflags --libs emodel`
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <Eo.h>
#include <Ecore.h>
#include <Efl.h>
#include <Eio.h>
#include <eio_model.h>

#include "eio_suite.h"

Eina_Bool children_added = EINA_FALSE;
Eina_Tmpstr* temp_filename = NULL;
const char* tmpdir = NULL;

static Eina_Bool
_load_monitor_status_cb(void *data, const Eo_Event *event)
{
  Efl_Model_Load* st = event->event_info;
  Eo* parent = data;
  const Eina_Value* value_prop = NULL;
  const char* str = NULL;

  if (!(st->status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
    return EINA_TRUE;

  efl_model_property_get(event->obj, "path", &value_prop);
  fail_if(!value_prop, "ERROR: Cannot get property!\n");

  str = eina_value_to_string(value_prop);
  fail_if(!str, "ERROR: Cannot convert value to string!\n");
  fprintf(stderr, "new children filename %s\n", str);
  if(temp_filename && strcmp(str, temp_filename) == 0)
    {
      fprintf(stderr, "is child that we want\n");
      eo_event_callback_del(event->obj, EFL_MODEL_BASE_EVENT_LOAD_STATUS, _load_monitor_status_cb, data);
      children_added = EINA_TRUE;
      efl_model_child_del(parent, event->obj);
    }

    return EINA_FALSE;
}

static Eina_Bool
_children_removed_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
  if(children_added)
    {
       Efl_Model_Children_Event* evt = event->event_info;

       Eina_Bool b;
       b = efl_model_load_status_get(evt->child);
       if(b)
         {
            const Eina_Value* value_prop = NULL;
            const char* str = NULL;

            efl_model_property_get(evt->child, "path", &value_prop);
            fail_if(!value_prop, "ERROR: Cannot get property!\n");

            str = eina_value_to_string(value_prop);
            fail_if(!str, "ERROR: Cannot convert value to string!\n");
            if(temp_filename && strcmp(str, temp_filename) == 0)
              ecore_main_loop_quit();
         }
    }
  return EINA_TRUE;
}

static Eina_Bool
_children_added_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
  Efl_Model_Children_Event* evt = event->event_info;
  if (evt == NULL)
    return EINA_TRUE;

  eo_event_callback_add(evt->child, EFL_MODEL_BASE_EVENT_LOAD_STATUS, _load_monitor_status_cb, event->obj);
  efl_model_load(evt->child);

  return EINA_TRUE;
}

static Eina_Bool
_children_count_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   unsigned int *len = event->event_info;
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   Eo *child;
   unsigned int i = 0;
   int fd = 0;

   fprintf(stderr, "Children count number=%d\n", *len);

   /**< get full list */
   status = efl_model_children_slice_get(event->obj, 0, 0, (Eina_Accessor **)&accessor);
   if(accessor != NULL)
     {
        EINA_ACCESSOR_FOREACH(accessor, i, child) {}
        fprintf(stdout, "Got %d childs from Accessor. status=%d\n", i, status);
     }

   if((fd = eina_file_mkstemp("prefixXXXXXX.ext", &temp_filename)) > 0)
     {
       close(fd);
     }

   return EINA_TRUE;
}

START_TEST(eio_model_test_test_monitor_add)
{
   Eo *filemodel = NULL;

   fprintf(stderr, "efl_model_test_test_monitor_add\n");

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   tmpdir = eina_environment_tmp_get();

   filemodel = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(eo_self, tmpdir));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   eo_event_callback_add(filemodel, EFL_MODEL_BASE_EVENT_CHILD_ADDED, _children_added_cb, NULL);
   eo_event_callback_add(filemodel, EFL_MODEL_BASE_EVENT_CHILD_REMOVED, _children_removed_cb, NULL);
   eo_event_callback_add(filemodel, EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, _children_count_cb, NULL);

   efl_model_load(filemodel);

   ecore_main_loop_begin();

   eo_unref(filemodel);

   eio_shutdown();
   ecore_shutdown();
   eina_shutdown();

   fail_if(!children_added);
}
END_TEST

void
eio_model_test_monitor_add(TCase *tc)
{
   tcase_add_test(tc, eio_model_test_test_monitor_add);
}
