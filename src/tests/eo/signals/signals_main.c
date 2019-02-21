#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "signals_simple.h"

#include "../eunit_tests.h"

static int cb_count = 0;

static void
_null_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
}

static void
_a_changed_cb(void *data, const Efl_Event *event)
{
   int new_a = *((int *) event->info);
   printf("%s event_info:'%d' data:'%d'\n", __func__, new_a, (int) (intptr_t) data);

   cb_count++;

   efl_event_callback_priority_add(event->object, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _null_cb, (void *) 23423);
   efl_event_callback_del(event->object, EV_A_CHANGED, _null_cb, (void *) 23423);

   /* Stop as we reached the 3rd one. */
   if (cb_count == 3)
      efl_event_callback_stop(event->object);
}

static Eina_Bool inside = EINA_FALSE;
static int called = 0;

static void
_restart_1_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   fprintf(stderr, "restart 1 inside: %i\n", inside);
   fail_if(!inside);
   called++;
   efl_event_callback_stop(event->object);
}

static void
_restart_2_cb(void *data, const Efl_Event *event)
{
   fprintf(stderr, "restart 2 inside: %i\n", inside);
   fail_if(inside);

   inside = EINA_TRUE;
   efl_event_callback_legacy_call(event->object, event->desc, data);
   inside = EINA_FALSE;

   called++;

   fprintf(stderr, "restart 2 exit inside: %i (%i)\n", inside, called);
   efl_event_callback_stop(event->object);
}

static void
_restart_3_cb(void *data, const Efl_Event *event)
{
   fprintf(stderr, "restart 3 inside: %i\n", inside);
   fail_if(!inside);

   fprintf(stderr, "restart 3 exit inside: %i (%i)\n", inside, called);
   efl_event_callback_stop(event->object);

   inside = EINA_TRUE;
   efl_event_callback_call(event->object, event->desc, data);
   inside = EINA_FALSE;

   called++;
}

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   efl_object_init();

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Simple_Public_Data *pd = efl_data_scope_get(obj, SIMPLE_CLASS);

   /* The order of these two is undetermined. */
   efl_event_callback_priority_add(obj, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2);
   efl_event_callback_priority_add(obj, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1);
   /* This will be called afterwards. */
   efl_event_callback_priority_add(obj, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_DEFAULT, _a_changed_cb, (void *) 3);
   /* This will never be called because the previous callback returns NULL. */
   efl_event_callback_priority_add(obj, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_AFTER, _a_changed_cb, (void *) 4);

   simple_a_set(obj, 1);

   fail_if(cb_count != 3);

   efl_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 3);
   fail_if(pd->cb_count != 3);

   efl_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 12);
   fail_if(pd->cb_count != 3);

   efl_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 4);
   fail_if(pd->cb_count != 2);

   efl_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 2);
   fail_if(pd->cb_count != 1);

   efl_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 1);
   fail_if(pd->cb_count != 0);

   /* Freeze/thaw. */
   int fcount = 0;
   cb_count = 0;
   efl_event_callback_priority_add(obj, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1);
   fail_if(pd->cb_count != 1);

   fcount = efl_event_freeze_count_get(obj);
   fail_if(fcount != 0);

   efl_event_freeze(obj);
   fcount = efl_event_freeze_count_get(obj);
   fail_if(fcount != 1);

   efl_event_freeze(obj);
   fcount = efl_event_freeze_count_get(obj);
   fail_if(fcount != 2);

   efl_event_callback_priority_add(obj, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2);
   fail_if(pd->cb_count != 2);

   simple_a_set(obj, 2);
   fail_if(cb_count != 0);
   efl_event_thaw(obj);
   fcount = efl_event_freeze_count_get(obj);
   fail_if(fcount != 1);

   efl_event_thaw(obj);
   fcount = efl_event_freeze_count_get(obj);
   fail_if(fcount != 0);

   simple_a_set(obj, 3);
   fail_if(cb_count != 2);

   cb_count = 0;
   efl_event_thaw(obj);
   fcount = efl_event_freeze_count_get(obj);
   fail_if(fcount != 0);

   efl_event_freeze(obj);
   fcount = efl_event_freeze_count_get(obj);
   fail_if(fcount != 1);

   simple_a_set(obj, 2);
   fail_if(cb_count != 0);
   efl_event_thaw(obj);
   fcount = efl_event_freeze_count_get(obj);
   fail_if(fcount != 0);

   efl_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 1);
   fail_if(pd->cb_count != 1);
   efl_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 2);
   fail_if(pd->cb_count != 0);

   /* Global Freeze/thaw. */
   fcount = 0;
   cb_count = 0;
   pd->cb_count = 0;
   efl_event_callback_priority_add(obj, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1);
   fail_if(pd->cb_count != 1);

   fcount = efl_event_global_freeze_count_get();
   fail_if(fcount != 0);

   efl_event_global_freeze();
   fcount = efl_event_global_freeze_count_get();
   fail_if(fcount != 1);

   efl_event_global_freeze();
   fcount = efl_event_global_freeze_count_get();
   fail_if(fcount != 2);

   efl_event_callback_priority_add(obj, EV_A_CHANGED, EFL_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2);
   fail_if(pd->cb_count != 2);

   simple_a_set(obj, 2);
   fail_if(cb_count != 0);
   efl_event_global_thaw();
   fcount = efl_event_global_freeze_count_get();
   fail_if(fcount != 1);

   efl_event_global_thaw();
   fcount = efl_event_global_freeze_count_get();
   fail_if(fcount != 0);

   simple_a_set(obj, 3);
   fail_if(cb_count != 2);

   cb_count = 0;
   efl_event_global_thaw();
   fcount = efl_event_global_freeze_count_get();
   fail_if(fcount != 0);

   efl_event_global_freeze();
   fcount = efl_event_global_freeze_count_get();
   fail_if(fcount != 1);

   simple_a_set(obj, 2);
   fail_if(cb_count != 0);
   efl_event_global_thaw();
   fcount = efl_event_global_freeze_count_get();
   fail_if(fcount != 0);

   efl_event_callback_priority_add(obj, EV_RESTART, EFL_CALLBACK_PRIORITY_DEFAULT, _restart_1_cb, NULL);
   efl_event_callback_priority_add(obj, EV_RESTART, EFL_CALLBACK_PRIORITY_BEFORE, _restart_3_cb, NULL);
   efl_event_callback_priority_add(obj, EV_RESTART, EFL_CALLBACK_PRIORITY_BEFORE, _restart_2_cb, NULL);
   efl_event_callback_legacy_call(obj, EV_RESTART, NULL);
   fail_if(inside);
   fail_if(called != 3);

   efl_unref(obj);
   efl_object_shutdown();
   return 0;
}
