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

static void
promise_then_count(void *data EINA_UNUSED, Efl_Event const *event)
{
  int *total = ((Efl_Future_Event_Success*)event->info)->value;
  ck_assert_ptr_ne(total, NULL);
  printf("efl_model_loaded count %d\n", *total); fflush(stdout);
  ecore_main_loop_quit();
}

static void
promise_then_accessor(void *data EINA_UNUSED, Efl_Event const* event)
{
  Eina_Accessor *accessor = ((Efl_Future_Event_Success*)event->info)->value;
  ck_assert_ptr_ne(accessor, NULL);
  printf("efl_model_loaded accessor %p\n", accessor); fflush(stdout);

  Eo* child;
  int i = 0;
  EINA_ACCESSOR_FOREACH(accessor, i, child)
    {
      printf("efl_model_loaded child: %d pointer %p\n", i, child);
    }

  ecore_main_loop_quit();
}

static void
promise_then_value(void *user EINA_UNUSED, Efl_Event const* event)
{
  Eina_Value* value = ((Efl_Future_Event_Success*)event->info)->value;
  ck_assert_ptr_ne(value, NULL);
  char *str = eina_value_to_string(value);

  ck_assert_ptr_ne(str, NULL);
  printf("efl_model_loaded property: %s\n", str);
  free(str);

  ecore_main_loop_quit();
}

static void
error_promise_then(void* data EINA_UNUSED, Efl_Event const* event EINA_UNUSED)
{
  ck_abort_msg(0, "Error Promise cb");
  ecore_main_loop_quit();
}

START_TEST(eio_model_test_test_file)
{
   Eo *filemodel = NULL;

   memset(&reqs, 0, sizeof(struct reqs_t));

   fail_if(!eina_init(), "ERROR: Cannot init Eina!\n");
   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!efl_object_init(), "ERROR: Cannot init EO!\n");
   fail_if(!eio_init(), "ERROR: Cannot init EIO!\n");

   filemodel = efl_add(EIO_MODEL_CLASS, ecore_main_loop_get(), eio_model_path_set(efl_added, EFL_MODEL_TEST_FILENAME_PATH));
   fail_if(!filemodel, "ERROR: Cannot init model!\n");

   handler = ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, exit_func, NULL);

   Efl_Future *future;

   future = efl_model_property_get(filemodel, "filename");
   efl_future_then(future, &promise_then_value, &error_promise_then, NULL, NULL);
   ecore_main_loop_begin();

   future = efl_model_property_get(filemodel, "size");
   efl_future_then(future, &promise_then_value, &error_promise_then, NULL, NULL);
   ecore_main_loop_begin();

   future = efl_model_property_get(filemodel, "mtime");
   efl_future_then(future, &promise_then_value, &error_promise_then, NULL, NULL);
   ecore_main_loop_begin();

   future = efl_model_children_slice_get(filemodel, 0, 0);
   efl_future_then(future, &promise_then_accessor, &error_promise_then, NULL, NULL);
   ecore_main_loop_begin();

   future = efl_model_children_count_get(filemodel);
   efl_future_then(future, &promise_then_count, &error_promise_then, NULL, NULL);
   ecore_main_loop_begin();

   efl_unref(filemodel);

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

