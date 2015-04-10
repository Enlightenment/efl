//Compile with:
// gcc -o emodel_test_file emodel_test_file.c `pkg-config --cflags --libs emodel`
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Eio.h>
#include <Ecore.h>
#include <Efl.h>
#include <eio_model.h>
#include <stdio.h>

#include <check.h>

Eina_Bool children_added = EINA_FALSE;
Eina_Tmpstr* temp_filename = NULL;
const char* tmpdir = NULL;

static Eina_Bool
_load_monitor_status_cb(void *data, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
  Efl_Model_Load* st = event_info;
  Eo* parent = data;
  const Eina_Value* value_prop = NULL;
  const char* str = NULL;

  if (!(st->status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES))
    return EINA_TRUE;

  eo_do(obj, efl_model_property_get("path", &value_prop));
  fail_if(!value_prop, "ERROR: Cannot get property!\n");

  str = eina_value_to_string(value_prop);
  fail_if(!str, "ERROR: Cannot convert value to string!\n");
  fprintf(stderr, "new children filename %s\n", str);
  if(strcmp(str, temp_filename) == 0)
    {
      fprintf(stderr, "is child that we want\n");
      eo_do(obj, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _load_monitor_status_cb, data));
      children_added = EINA_TRUE;
      eo_do(parent, efl_model_child_del(obj));
    }

    return EINA_FALSE;
}

static Eina_Bool
_children_removed_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void* event_info EINA_UNUSED)
{
  if(children_added)
    {
       Efl_Model_Children_Event* evt = event_info;

       Eina_Bool b;
       eo_do(evt->child, b = efl_model_load_status_get() & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES);
       if(b)
         {
            const Eina_Value* value_prop = NULL;
            const char* str = NULL;

            eo_do(evt->child, efl_model_property_get("path", &value_prop));
            fail_if(!value_prop, "ERROR: Cannot get property!\n");

            str = eina_value_to_string(value_prop);
            fail_if(!str, "ERROR: Cannot convert value to string!\n");
            if(strcmp(str, temp_filename) == 0)
              ecore_main_loop_quit();
         }
    }
  return EINA_TRUE;
}

static Eina_Bool
_children_added_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
  Efl_Model_Children_Event* evt = event_info;
  if (evt == NULL)
    return EINA_TRUE;

  eo_do(evt->child, eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS, _load_monitor_status_cb, obj));
  eo_do(evt->child, efl_model_load());

  return EINA_TRUE;
}

static Eina_Bool
_children_count_cb(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   unsigned int *len = event_info;
   Eina_Accessor *accessor;
   Efl_Model_Load_Status status;
   Eo *child;
   unsigned int i = 0;
   int fd = 0;

   fprintf(stderr, "Children count number=%d\n", *len);

   /**< get full list */
   eo_do(obj, status = efl_model_children_slice_get(0 ,0 ,(Eina_Accessor **)&accessor));
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

#ifndef HAVE_EVIL
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        tmpdir = getenv("TMPDIR");
        if (!tmpdir) tmpdir = getenv("XDG_RUNTIME_DIR");
     }
   if (!tmpdir) tmpdir = "/tmp";
#else
   tmpdir = (char *)evil_tmpdir_get();
#endif /* ! HAVE_EVIL */
   
   filemodel = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(tmpdir));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   eo_do(filemodel, eo_event_callback_add(EFL_MODEL_BASE_EVENT_CHILD_ADDED, _children_added_cb, NULL));
   eo_do(filemodel, eo_event_callback_add(EFL_MODEL_BASE_EVENT_CHILD_REMOVED, _children_removed_cb, NULL));
   eo_do(filemodel, eo_event_callback_add(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, _children_count_cb, NULL));

   eo_do(filemodel, efl_model_load());

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

