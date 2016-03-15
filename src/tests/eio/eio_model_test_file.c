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

#define EFL_MODEL_TEST_FILENAME_PATH "/tmp"
#define EFL_MODEL_MAX_TEST_CHILDS 16

struct reqs_t {
   /* property change */
   int changed_is_dir;
   int changed_is_lnk;
   int changed_size;
   int changed_mtime;

   /* properties list */
   int proplist_filename;
   int proplist_path;
   int proplist_mtime;
   int proplist_is_dir;
   int proplist_is_lnk;
   int proplist_size;

   /* misc tests for data or propeties */
   int properties;
   int children;
   int child_add;
   int child_del;

   /* load status */
  int properties_loaded;
};

static struct reqs_t reqs;
static Ecore_Event_Handler *handler;

static Eina_Bool
 exit_func(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *ev)
 {
    Ecore_Event_Signal_Exit *e;

    e = (Ecore_Event_Signal_Exit *)ev;
    if (e->interrupt)      fprintf(stdout, "Exit: interrupt\n");
    else if (e->quit)      fprintf(stdout, "Exit: quit\n");
    else if (e->terminate) fprintf(stdout, "Exit: terminate\n");
    ecore_main_loop_quit();
    return ECORE_CALLBACK_CANCEL;
 }

static Eina_Bool
_load_status_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Efl_Model_Load *st = event->event_info;
   printf("Load CHANGE\n");

   if (st->status & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     printf("Children is Loaded\n");

   if (st->status & EFL_MODEL_LOAD_STATUS_LOADED_PROPERTIES)
     {
        fprintf(stderr, "Properties are Loaded\n"); fflush(stderr);
        if(!reqs.properties_loaded)
          ecore_main_loop_quit();
        reqs.properties_loaded = 1;
     }

   if ((st->status & EFL_MODEL_LOAD_STATUS_LOADED) == EFL_MODEL_LOAD_STATUS_LOADED)
     {
        Eina_Accessor *accessor;
        const Eina_Value *value_prop;
        Efl_Model_Load_Status status;
        unsigned int total;
        char *str;

        printf("Model is Loaded\n");
        status = efl_model_property_get(event->obj, "filename", &value_prop);
        str = eina_value_to_string(value_prop);
        printf("efl_model_loaded filename %s, status=%d\n", str, status);
        free(str);

        status = efl_model_property_get(event->obj, "size", &value_prop);
        str = eina_value_to_string(value_prop);
        printf("efl_model_loaded size %s, status=%d\n", str, status);
        free(str);

        status = efl_model_property_get(event->obj, "mtime", &value_prop);
        str = eina_value_to_string(value_prop);
        printf("efl_model_loaded mtime %s, status=%d\n", str, status);
        free(str);

        efl_model_children_count_get(event->obj, &total);
        printf("efl_model_test count %d\n", (int)total);

        /**< get full list */
        status = efl_model_children_slice_get(event->obj, 0, 0, (Eina_Accessor **)&accessor);
        eina_accessor_free(accessor);
        ecore_main_loop_quit();
     }
   return EINA_TRUE;
}

static Eina_Bool
_properties_change_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   const Efl_Model_Property_Event *evt = (Efl_Model_Property_Event *)event->event_info;
   const char *prop;
   Eina_Array_Iterator it;
   unsigned int i;

   EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it)
     {
        if (!strcmp(prop, "is_dir"))
          reqs.changed_is_dir = 1;
        else if (!strcmp(prop, "is_lnk"))
          reqs.changed_is_lnk = 1;
        else if (!strcmp(prop, "size"))
          reqs.changed_size = 1;
        else if (!strcmp(prop, "mtime"))
          reqs.changed_mtime = 1;
     }

   reqs.properties = 1;
   return EINA_TRUE;
}

static Eina_Bool
_children_count_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   unsigned int *len = (unsigned int *)event->event_info;
   unsigned int total;

   fprintf(stdout, "Children count number=%d\n", *len);
   reqs.children = *len;

   efl_model_children_count_get(event->obj, &total);
   fprintf(stdout, "New total children count number=%d\n", *len);

   return EINA_TRUE;
}

START_TEST(eio_model_test_test_file)
{
   Eo *filemodel = NULL;
   const Eina_Value *value_prop;
   Efl_Model_Load_Status status;
   Eina_Array *properties_list = NULL;
   Eina_Array_Iterator iterator;
   char *str;
   unsigned int i;

   memset(&reqs, 0, sizeof(struct reqs_t));

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   filemodel = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(eo_self, EFL_MODEL_TEST_FILENAME_PATH));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   eo_event_callback_add(filemodel, EFL_MODEL_BASE_EVENT_LOAD_STATUS, _load_status_cb, NULL);
   eo_event_callback_add(filemodel, EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, _properties_change_cb, NULL);
   eo_event_callback_add(filemodel, EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED, _children_count_cb, NULL);

   efl_model_load(filemodel);

   handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_func, NULL);

   ecore_main_loop_begin();

   status = efl_model_property_get(filemodel, "filename", &value_prop);
   str = eina_value_to_string(value_prop);
   printf("efl_model_test filename %s, load status %d\n", str, status);

   free(str);

   i = 0;
   efl_model_properties_get(filemodel, &properties_list);
   EINA_ARRAY_ITER_NEXT(properties_list, i, str, iterator)
     {
        fprintf(stdout, "Returned property list %d: %s\n", i, str);
        if(!strcmp(str, "filename"))
          reqs.proplist_filename = 1;
        else if(!strcmp(str, "path"))
          reqs.proplist_path = 1;
        else if(!strcmp(str, "mtime"))
          reqs.proplist_mtime = 1;
        else if(!strcmp(str, "is_dir"))
          reqs.proplist_is_dir = 1;
        else if(!strcmp(str, "is_lnk"))
          reqs.proplist_is_lnk = 1;
        else if(!strcmp(str, "size"))
          reqs.proplist_size = 1;
     }

   ecore_main_loop_begin();

   eo_unref(filemodel);
   ecore_shutdown();
   eina_shutdown();
   eio_shutdown();
}
END_TEST

void
eio_model_test_file(TCase *tc)
{
    tcase_add_test(tc, eio_model_test_test_file);
}
