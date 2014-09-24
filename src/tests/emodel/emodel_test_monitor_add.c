//Compile with:
// gcc -o emodel_test_file emodel_test_file.c `pkg-config --cflags --libs emodel`
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Eio.h>
#include <Ecore.h>
#include <Emodel.h>
#include <eio_model.h>
#include <stdio.h>

#include <check.h>

#define EMODEL_TEST_FILENAME_PATH "/tmp"

Eina_Bool children_added = EINA_FALSE;

static Eina_Bool
_children_added_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
  Emodel_Children_Event* evt = event_info;
  Eo* child = evt->child;
  Eina_Value value_prop;
  const char* str;

  eo_do(child, emodel_property_get("filename", &value_prop));
  str = eina_value_to_string(&value_prop);
  fprintf(stderr, "new children filename %s\n", str);
  if(strcmp(str, "test_file_monitor_add") == 0)
    {
      fprintf(stderr, "is child that we want\n");
      children_added = EINA_TRUE;
      eo_do(obj, emodel_child_del(child));
      ecore_main_loop_quit();
    }
  eina_value_flush(&value_prop);
  return EINA_TRUE;
}

static Eina_Bool
_children_count_cb(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   unsigned int *len = event_info;
   Eina_Accessor *accessor;
   Emodel_Load_Status status;
   Eo *child;
   unsigned int i = 0;

   fprintf(stderr, "Children count number=%d\n", *len);

   /**< get full list */
   eo_do(obj, status = emodel_children_slice_get(0 ,0 ,(Eina_Accessor **)&accessor));
   if(accessor != NULL)
     {
        EINA_ACCESSOR_FOREACH(accessor, i, child) {}
        fprintf(stdout, "Got %d childs from Accessor. status=%d\n", i, status);
     }

   fclose(fopen(EMODEL_TEST_FILENAME_PATH "/test_file_monitor_add", "w+"));

   return EINA_TRUE;
}

START_TEST(emodel_test_test_monitor_add)
{
   Eo *filemodel = NULL;

   fprintf(stderr, "emodel_test_test_monitor_add\n");

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   filemodel = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(EMODEL_TEST_FILENAME_PATH));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   eo_do(filemodel, eo_event_callback_add(EMODEL_EVENT_CHILDREN_COUNT_CHANGED, _children_count_cb, NULL));
   eo_do(filemodel, eo_event_callback_add(EMODEL_EVENT_CHILD_ADDED, _children_added_cb, NULL));

   eo_do(filemodel, emodel_load());

   ecore_main_loop_begin();

   sleep(1); /**< EIO is asynchrounous so I must give some time for deletions to execute */

   ecore_main_loop_iterate(); /**< Give time to unlink file */

   eo_unref(filemodel);

   eio_shutdown();
   ecore_shutdown();
   eina_shutdown();

   fail_if(!children_added);
}
END_TEST

void
emodel_test_monitor_add(TCase *tc)
{
   tcase_add_test(tc, emodel_test_test_monitor_add);
}

