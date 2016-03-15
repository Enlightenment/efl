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

struct eina_iterator
{
  Eina_Iterator* success_iterator;
  Eina_Iterator* failure_iterator;
};

static void
promise_then(Eo* obj EINA_UNUSED, struct eina_iterator* it_struct)
{
  Eina_Accessor **accessor;
  unsigned int* total;
  char *str;
  Eina_Iterator* iterator = it_struct->success_iterator;
  Eina_Value *name, *size, *mtime;
  
  eina_iterator_next(iterator, &name);
  eina_iterator_next(iterator, &size);
  eina_iterator_next(iterator, &mtime);
  eina_iterator_next(iterator, &accessor);
  eina_iterator_next(iterator, &total);
  
  str = eina_value_to_string(name);
  printf("efl_model_loaded filename %s\n", str);
  free(str);

  str = eina_value_to_string(size);
  printf("efl_model_loaded size %s\n", str);
  free(str);

  str = eina_value_to_string(mtime);
  printf("efl_model_loaded mtime %s\n", str);
  free(str);

  printf("efl_model_loaded count %d\n", (int)*total); fflush(stdout);

  printf("efl_model_loaded accessor %p\n", *accessor); fflush(stdout);

  Eo* child;
  int i = 0;
  EINA_ACCESSOR_FOREACH(*accessor, i, child)
    {
      printf("efl_model_loaded child: %d pointer %p\n", i, child);
    }
  
  ecore_main_loop_quit();
}

static Eina_Bool
_properties_change_cb(void *data EINA_UNUSED, const Eo_Event* event)
{
   const Efl_Model_Property_Event *evt = event->event_info;
   const char *prop;
   Eina_Array_Iterator it;
   unsigned int i;

   fprintf(stdout, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stdout);
   EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it)
     {
   fprintf(stdout, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stdout);
        if (!strcmp(prop, "is_dir"))
          reqs.changed_is_dir = 1;
        else if (!strcmp(prop, "is_lnk"))
          reqs.changed_is_lnk = 1;
        else if (!strcmp(prop, "size"))
          reqs.changed_size = 1;
        else if (!strcmp(prop, "mtime"))
          reqs.changed_mtime = 1;
   fprintf(stdout, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stdout);
     }

   fprintf(stdout, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stdout);
   reqs.properties = 1;
   return EINA_TRUE;
}

static Eina_Bool
_children_count_cb(void *data EINA_UNUSED, const Eo_Event* event)
{
   unsigned int *len = event->event_info;
   unsigned int total;

   fprintf(stdout, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stdout);
   fprintf(stdout, "Children count number=%d\n", *len);
   reqs.children = *len;

   efl_model_children_count_get(event->obj, &total);
   fprintf(stdout, "New total children count number=%d\n", *len);
   fprintf(stdout, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stdout);

   return EINA_TRUE;
}

START_TEST(eio_model_test_test_file)
{
   Eo *filemodel = NULL;
   Eina_Array *properties_list = NULL;
   unsigned int i;

   memset(&reqs, 0, sizeof(struct reqs_t));

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!eo_init(), "ERROR: Cannot init EO!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   filemodel = eo_add(EIO_MODEL_CLASS, NULL, eio_model_path_set(eoid, EFL_MODEL_TEST_FILENAME_PATH));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_func, NULL);

   Ecore_Promise *promises[] = { NULL, NULL, NULL, NULL, NULL, NULL };
   efl_model_property_get(filemodel, "filename", &promises[0]);
   efl_model_property_get(filemodel, "size", &promises[1]);
   efl_model_property_get(filemodel, "mtime", &promises[2]);
   efl_model_children_slice_get(filemodel, 0, 0, &promises[3]);
   efl_model_children_count_get(filemodel, &promises[4]);

   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
   
   ecore_promise_then(ecore_promise_all(eina_carray_iterator_new(&promises[0])),
                      &promise_then, filemodel);

   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_main_loop_begin();

   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
   
   eo_unref(filemodel);

   eio_shutdown();
   ecore_shutdown();
   eina_shutdown();
}
END_TEST

void
eio_model_test_file(TCase *tc)
{
    tcase_add_test(tc, eio_model_test_test_file);
}

