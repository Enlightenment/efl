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

static Eina_Value
promise_then_accessor(void *data EINA_UNUSED,
                      const Eina_Value v,
                      const Eina_Future *dead_future EINA_UNUSED)
{
  Eo *child = NULL;
  unsigned int i, len;

  fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

  EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
    ;

  ecore_main_loop_quit();

  return EINA_VALUE_EMPTY;
}

static Eina_Value
listing(void *data,
        const Eina_Value v EINA_UNUSED,
        const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Future *future;
   Efl_Model *filemodel = data;
   static unsigned int it = 0;

   if (it++ >= 3)
     {
        fprintf(stderr, "Failed to list any files after 3 attemps.\n");
        ecore_main_loop_quit();
        return eina_value_error_init(ECANCELED);
     }

   if (efl_model_children_count_get(filemodel) == 0)
     {
        future = efl_loop_job(efl_provider_find(filemodel, EFL_LOOP_CLASS));
        future = eina_future_then(future, listing, filemodel);
     }
   else
     {
        future = efl_model_children_slice_get(filemodel, 0, efl_model_children_count_get(filemodel));
        future = eina_future_then(future, &promise_then_accessor, NULL);
     }

   return eina_future_as_value(future);
}

static Eina_Future *listingf = NULL;

static Eina_Value
clearup(void *data EINA_UNUSED,
        const Eina_Value v,
        const Eina_Future *dead_future EINA_UNUSED)
{
   listingf = NULL;

   return v;
}

static void
setup_waiter(Efl_Model *model)
{
   Efl_Loop *loop = efl_provider_find(model, EFL_LOOP_CLASS);

   if (listingf) return ;
   listingf = efl_loop_job(loop);
   listingf = eina_future_then(listingf, listing, model);
   listingf = eina_future_then(listingf, clearup, NULL);
}

static void
_property_changed(void *data EINA_UNUSED, const Efl_Event *event)
{
   // Wait and check is_dir
   Efl_Model_Property_Event *ev = event->info;
   Efl_Model *filemodel = event->object;
   const char *property;
   Eina_Array_Iterator iterator;
   unsigned int i;

   EINA_ARRAY_ITER_NEXT(ev->changed_properties, i, property, iterator)
     if (!strcmp(property, "is_dir"))
       {
          if (efl_model_children_count_get(filemodel) > 0)
            setup_waiter(filemodel);
       }
}

static void
_child_added(void *data EINA_UNUSED, const Efl_Event *event)
{
   // Wait for a child being added to setup a job to list the directory
   Efl_Model *filemodel = event->object;

   if (efl_model_children_count_get(filemodel) > 0)
     setup_waiter(filemodel);
}

EFL_CALLBACKS_ARRAY_DEFINE(model,
                           { EFL_MODEL_EVENT_PROPERTIES_CHANGED, _property_changed },
                           { EFL_MODEL_EVENT_CHILD_ADDED, _child_added })

EFL_START_TEST(eio_model_test_test_file)
{
   Eo *filemodel = NULL;
   Eina_Value *result;
   char *str;

   memset(&reqs, 0, sizeof(struct reqs_t));

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!efl_object_init(), "ERROR: Cannot init EO!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   filemodel = efl_add(EIO_MODEL_CLASS, efl_main_loop_get(),
                       eio_model_path_set(efl_added, EFL_MODEL_TEST_FILENAME_PATH));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_func, NULL);

   result = efl_model_property_get(filemodel, "filename");
   fail_if(!result);
   str = eina_value_to_string(result);
   fail_if(!str);
   free(str);
   eina_value_free(result);

   result = efl_model_property_get(filemodel, "size");
   fail_if(!result);
   eina_value_free(result);

   result = efl_model_property_get(filemodel, "mtime");
   fail_if(!result);
   eina_value_free(result);

   efl_event_callback_array_add(filemodel, model(), NULL);

   ecore_main_loop_begin();

   efl_del(filemodel);

   eio_shutdown();
   ecore_shutdown();
   eina_shutdown();
}
EFL_END_TEST

void
eio_model_test_file(TCase *tc)
{
    tcase_add_test(tc, eio_model_test_test_file);
}
