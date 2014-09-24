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
#define EMODEL_MAX_TEST_CHILDS 16

/**
 * The following test works however
 * it is going to rename (move) the original directory to
 * new one so '/tmp' as root dir doesn't work , you'll need to use
 * '/tmp/some_other_dir' as root instead.
 */
//#define _RUN_LOCAL_TEST

struct reqs_t {
   /* property change */
   int changed_is_dir;
   int changed_is_lnk;
   int changed_size;
   int changed_mtime;
   int changed_icon;

   /* properties list */
   int proplist_filename;
   int proplist_path;
   int proplist_icon;
   int proplist_mtime;
   int proplist_is_dir;
   int proplist_is_lnk;
   int proplist_size;

   /* misc tests for data or propeties */
   int properties;
   int children;
   int child_add;
   int child_del;
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
_load_status_cb(void *data EINA_UNUSED, Eo *obj, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Emodel_Load *st = event_info;
   printf("Load CHANGE\n");

   if (st->status & EMODEL_LOAD_STATUS_LOADED_CHILDREN)
     printf("Children is Loaded\n");

   if (st->status & EMODEL_LOAD_STATUS_LOADED_PROPERTIES)
     printf("Properties is Loaded\n");

   if ((st->status & EMODEL_LOAD_STATUS_LOADED) == EMODEL_LOAD_STATUS_LOADED)
     {
        Eina_Accessor *accessor;
        Eina_Value value_prop;
        Eo *child;
        Emodel_Load_Status status;
        unsigned int total, i;
        char *str;

        printf("Model is Loaded\n");
        eo_do(obj, status = emodel_property_get("filename", &value_prop));
        str = eina_value_to_string(&value_prop);
        printf("emodel_loaded filename %s, status=%d\n", str, status);
        eina_value_flush(&value_prop);
        free(str);

        eo_do(obj, status = emodel_property_get("size", &value_prop));
        str = eina_value_to_string(&value_prop);
        printf("emodel_loaded size %s, status=%d\n", str, status);
        eina_value_flush(&value_prop);
        free(str);

        eo_do(obj, status = emodel_property_get("mtime", &value_prop));
        str = eina_value_to_string(&value_prop);
        printf("emodel_loaded mtime %s, status=%d\n", str, status);
        eina_value_flush(&value_prop);
        free(str);

        eo_do(obj, emodel_children_count_get(&total));
        printf("emodel_test count %d\n", (int)total);

        /**< get full list */
        eo_do(obj, status = emodel_children_slice_get(0 ,0 ,(Eina_Accessor **)&accessor));
        EINA_ACCESSOR_FOREACH(accessor, i, child)
          {
             //XXX: check if there is memleak
             eo_do(child, status = emodel_property_get("filename", &value_prop));
             str = eina_value_to_string(&value_prop);
             printf("(full) %d emodel_children_get filename %s\n", i, str);
             eina_value_flush(&value_prop);
             free(str);
          }

        i = 0;
        eina_accessor_free(accessor);
        eo_do(obj, status = emodel_children_slice_get(5 ,5 ,(Eina_Accessor **)&accessor));

        EINA_ACCESSOR_FOREACH(accessor, i, child)
          {
             //XXX: check if there is memleak
             eo_do(child, status = emodel_property_get("filename", &value_prop));
             str = eina_value_to_string(&value_prop);
             printf("(slice) %d emodel_chidlren_property_set filename %s\n", i, str);
             eina_value_flush(&value_prop);
             free(str);
             eo_unref(child);
          }
        eina_accessor_free(accessor);
        ecore_main_loop_quit();
     }
   return EINA_TRUE;
}

static Eina_Bool
_properties_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   const Emodel_Property_Event *evt = (Emodel_Property_Event *)event_info;
   Emodel_Property_Pair *pair = NULL;
   Eina_List *l = NULL;

   EINA_LIST_FOREACH(evt->changed_properties, l, pair)
     {
        char *str;
        str = eina_value_to_string(&pair->value);
        fprintf(stdout, "Received changed property=%s, value=%s\n",
                pair->property, str);
        free(str);
        if(!strcmp(pair->property, "is_dir"))
          reqs.changed_is_dir = 1;
        if(!strcmp(pair->property, "is_lnk"))
          reqs.changed_is_lnk = 1;
        if(!strcmp(pair->property, "size"))
          reqs.changed_size = 1;
        if(!strcmp(pair->property, "mtime"))
          reqs.changed_mtime = 1;
        if(!strcmp(pair->property, "icon"))
          reqs.changed_icon = 1;
     }

   reqs.properties = 1;
   return EINA_TRUE;
}

static Eina_Bool
_children_count_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   unsigned int *len = (unsigned int *)event_info;
   unsigned int total;

   fprintf(stdout, "Children count number=%d\n", *len);
   reqs.children = *len;

   eo_do(obj, emodel_children_count_get(&total));
   fprintf(stdout, "New total children count number=%d\n", *len);

   return EINA_TRUE;
}

START_TEST(emodel_test_test_file)
{
   Eo *filemodel = NULL;
   Eina_Value value_prop;
   Emodel_Load_Status status;
#ifdef _RUN_LOCAL_TEST
   Eina_Value nameset_value;
#endif
   Eina_List *properties_list;
   Eina_List *l;
   char *str;
   int i;

   memset(&reqs, -1, sizeof(struct reqs_t));

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   filemodel = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(EMODEL_TEST_FILENAME_PATH));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   eo_do(filemodel, eo_event_callback_add(EMODEL_EVENT_LOAD_STATUS, _load_status_cb, NULL));
   eo_do(filemodel, eo_event_callback_add(EMODEL_EVENT_PROPERTIES_CHANGED, _properties_cb, NULL));
   eo_do(filemodel, eo_event_callback_add(EMODEL_EVENT_CHILDREN_COUNT_CHANGED, _children_count_cb, NULL));

   eo_do(filemodel, emodel_load());

   handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_func, NULL);

   eina_value_setup(&value_prop, EINA_VALUE_TYPE_STRING);

   eo_do(filemodel, status = emodel_property_get("filename", &value_prop));
   str = eina_value_to_string(&value_prop);
   printf("emodel_test filename %s, load status %d\n", str, status);

   eina_value_flush(&value_prop);
   free(str);

   i = 0;
   eo_do(filemodel, emodel_properties_list_get(&properties_list));
   EINA_LIST_FOREACH((Eina_List *)properties_list, l, str)
     {
        fprintf(stdout, "Returned property list %d: %s\n", i++, str);
        if(!strcmp(str, "filename"))
          reqs.proplist_filename = 1;
        else if(!strcmp(str, "path"))
          reqs.proplist_path = 1;
        else if(!strcmp(str, "icon"))
          reqs.proplist_icon = 1;
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

#ifdef _RUN_LOCAL_TEST
   eina_value_setup(&nameset_value, EINA_VALUE_TYPE_STRING);
   eina_value_setup(&value_prop, EINA_VALUE_TYPE_STRING);
   eina_value_set(&nameset_value, "/tmp/emodel_test");
   eo_do(filemodel, emodel_property_set("path", nameset_value));
   eina_value_flush(&nameset_value);
   //emodel_property_get("path", &value_prop);
   eo_do(filemodel, status = emodel_property_get("path", &value_prop));
   eina_value_flush(&value_prop);
#endif

   sleep(1); /**< EIO is asynchrounous so I must give some time for deletions to execute */
   eo_unref(filemodel);
   ecore_shutdown();
   eina_shutdown();
   eio_shutdown();
}
END_TEST

void
emodel_test_file(TCase *tc)
{
   /* tcase_add_test(tc, emodel_test_test_file); */
}

