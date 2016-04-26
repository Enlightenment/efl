#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "signals_simple.h"

#include "../eunit_tests.h"

static int cb_count = 0;

static Eina_Bool
_null_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_a_changed_cb(void *data, const Eo_Event *event)
{
   int new_a = *((int *) event->info);
   printf("%s event_info:'%d' data:'%d'\n", __func__, new_a, (int) (intptr_t) data);

   cb_count++;

   eo_event_callback_priority_add(event->obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _null_cb, (void *) 23423);
   eo_event_callback_del(event->obj, EV_A_CHANGED, _null_cb, (void *) 23423);

   /* Stop as we reached the 3rd one. */
   return (cb_count != 3);
}

static Eina_Bool inside = EINA_FALSE;
static int called = 0;

static Eina_Bool
_restart_1_cb(void *data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   fprintf(stderr, "restart 1 inside: %i\n", inside);
   fail_if(!inside);
   called++;
   return EINA_FALSE;
}

static Eina_Bool
_restart_2_cb(void *data, const Eo_Event *event)
{
   fprintf(stderr, "restart 2 inside: %i\n", inside);
   fail_if(inside);

   inside = EINA_TRUE;
   eo_event_callback_call(event->obj, event->desc, data);
   inside = EINA_FALSE;

   called++;

   fprintf(stderr, "restart 2 exit inside: %i (%i)\n", inside, called);
   return EINA_FALSE;
}

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   Simple_Public_Data *pd = eo_data_scope_get(obj, SIMPLE_CLASS);

   /* The order of these two is undetermined. */
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2);
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1);
   /* This will be called afterwards. */
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_DEFAULT, _a_changed_cb, (void *) 3);
   /* This will never be called because the previous callback returns NULL. */
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_AFTER, _a_changed_cb, (void *) 4);

   simple_a_set(obj, 1);

   fail_if(cb_count != 3);

   eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 3);
   fail_if(pd->cb_count != 3);

   eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 12);
   fail_if(pd->cb_count != 3);

   eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 4);
   fail_if(pd->cb_count != 2);

   eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 2);
   fail_if(pd->cb_count != 1);

   eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 1);
   fail_if(pd->cb_count != 0);

   /* Freeze/thaw. */
   int fcount = 0;
   cb_count = 0;
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1);
   fail_if(pd->cb_count != 1);

   fcount = eo_event_freeze_count_get(obj);
   fail_if(fcount != 0);

   eo_event_freeze(obj);
   fcount = eo_event_freeze_count_get(obj);
   fail_if(fcount != 1);

   eo_event_freeze(obj);
   fcount = eo_event_freeze_count_get(obj);
   fail_if(fcount != 2);

   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2);
   fail_if(pd->cb_count != 2);

   simple_a_set(obj, 2);
   fail_if(cb_count != 0);
   eo_event_thaw(obj);
   fcount = eo_event_freeze_count_get(obj);
   fail_if(fcount != 1);

   eo_event_thaw(obj);
   fcount = eo_event_freeze_count_get(obj);
   fail_if(fcount != 0);

   simple_a_set(obj, 3);
   fail_if(cb_count != 2);

   cb_count = 0;
   eo_event_thaw(obj);
   fcount = eo_event_freeze_count_get(obj);
   fail_if(fcount != 0);

   eo_event_freeze(obj);
   fcount = eo_event_freeze_count_get(obj);
   fail_if(fcount != 1);

   simple_a_set(obj, 2);
   fail_if(cb_count != 0);
   eo_event_thaw(obj);
   fcount = eo_event_freeze_count_get(obj);
   fail_if(fcount != 0);

   eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 1);
   fail_if(pd->cb_count != 1);
   eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 2);
   fail_if(pd->cb_count != 0);

   /* Global Freeze/thaw. */
   fcount = 0;
   cb_count = 0;
   pd->cb_count = 0;
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1);
   fail_if(pd->cb_count != 1);

   fcount = eo_event_global_freeze_count_get(EO_CLASS);
   fail_if(fcount != 0);

   eo_event_global_freeze(EO_CLASS);
   fcount = eo_event_global_freeze_count_get(EO_CLASS);
   fail_if(fcount != 1);

   eo_event_global_freeze(EO_CLASS);
   fcount = eo_event_global_freeze_count_get(EO_CLASS);
   fail_if(fcount != 2);

   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2);
   fail_if(pd->cb_count != 2);

   simple_a_set(obj, 2);
   fail_if(cb_count != 0);
   eo_event_global_thaw(EO_CLASS);
   fcount = eo_event_global_freeze_count_get(EO_CLASS);
   fail_if(fcount != 1);

   eo_event_global_thaw(EO_CLASS);
   fcount = eo_event_global_freeze_count_get(EO_CLASS);
   fail_if(fcount != 0);

   simple_a_set(obj, 3);
   fail_if(cb_count != 2);

   cb_count = 0;
   eo_event_global_thaw(EO_CLASS);
   fcount = eo_event_global_freeze_count_get(EO_CLASS);
   fail_if(fcount != 0);

   eo_event_global_freeze(EO_CLASS);
   fcount = eo_event_global_freeze_count_get(EO_CLASS);
   fail_if(fcount != 1);

   simple_a_set(obj, 2);
   fail_if(cb_count != 0);
   eo_event_global_thaw(EO_CLASS);
   fcount = eo_event_global_freeze_count_get(EO_CLASS);
   fail_if(fcount != 0);

   eo_event_callback_priority_add(obj, EV_RESTART, EO_CALLBACK_PRIORITY_DEFAULT, _restart_1_cb, NULL);
   eo_event_callback_priority_add(obj, EV_RESTART, EO_CALLBACK_PRIORITY_BEFORE, _restart_2_cb, NULL);
   eo_event_callback_call(obj, EV_RESTART, NULL);
   fail_if(inside);
   fail_if(called != 2);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}
