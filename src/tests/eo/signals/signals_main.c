#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "signals_simple.h"

#include "../eunit_tests.h"

static int cb_count = 0;

static Eina_Bool
_null_cb(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   (void) desc;
   (void) obj;
   (void) data;
   (void) event_info;
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_a_changed_cb(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   (void) desc;
   (void) obj;
   int new_a = *((int *) event_info);
   printf("%s event_info:'%d' data:'%d'\n", __func__, new_a, (int) (intptr_t) data);

   cb_count++;

   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _null_cb, (void *) 23423));
   eo_do(obj, eo_event_callback_del(EV_A_CHANGED, _null_cb, (void *) 23423));

   /* Stop as we reached the 3rd one. */
   return (cb_count != 3);
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
   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2));
   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1));
   /* This will be called afterwards. */
   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_DEFAULT, _a_changed_cb, (void *) 3));
   /* This will never be called because the previous callback returns NULL. */
   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_AFTER, _a_changed_cb, (void *) 4));

   eo_do(obj, simple_a_set(1));

   fail_if(cb_count != 3);

   eo_do(obj, eo_event_callback_del(EV_A_CHANGED, _a_changed_cb, (void *) 3));
   fail_if(pd->cb_count != 3);

   eo_do(obj, eo_event_callback_del(EV_A_CHANGED, _a_changed_cb, (void *) 12));
   fail_if(pd->cb_count != 3);

   eo_do(obj, eo_event_callback_del(EV_A_CHANGED, _a_changed_cb, (void *) 4));
   fail_if(pd->cb_count != 2);

   eo_do(obj, eo_event_callback_del(EV_A_CHANGED, _a_changed_cb, (void *) 2));
   fail_if(pd->cb_count != 1);

   eo_do(obj, eo_event_callback_del(EV_A_CHANGED, _a_changed_cb, (void *) 1));
   fail_if(pd->cb_count != 0);

   /* Freeze/thaw. */
   int fcount = 0;
   cb_count = 0;
   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1));
   fail_if(pd->cb_count != 1);

   eo_do(obj, fcount = eo_event_freeze_get());
   fail_if(fcount != 0);

   eo_do(obj, eo_event_freeze());
   eo_do(obj, fcount = eo_event_freeze_get());
   fail_if(fcount != 1);

   eo_do(obj, eo_event_freeze());
   eo_do(obj, fcount = eo_event_freeze_get());
   fail_if(fcount != 2);

   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2));
   fail_if(pd->cb_count != 1);

   eo_do(obj, simple_a_set(2));
   fail_if(cb_count != 0);
   eo_do(obj, eo_event_thaw());
   eo_do(obj, fcount = eo_event_freeze_get());
   fail_if(fcount != 1);

   eo_do(obj, eo_event_thaw());
   eo_do(obj, fcount = eo_event_freeze_get());
   fail_if(fcount != 0);

   eo_do(obj, simple_a_set(3));
   fail_if(cb_count != 2);

   cb_count = 0;
   eo_do(obj, eo_event_thaw());
   eo_do(obj, fcount = eo_event_freeze_get());
   fail_if(fcount != 0);

   eo_do(obj, eo_event_freeze());
   eo_do(obj, fcount = eo_event_freeze_get());
   fail_if(fcount != 1);

   eo_do(obj, simple_a_set(2));
   fail_if(cb_count != 0);
   eo_do(obj, eo_event_thaw());
   eo_do(obj, fcount = eo_event_freeze_get());
   fail_if(fcount != 0);

   eo_do(obj, eo_event_callback_del(EV_A_CHANGED, _a_changed_cb, (void *) 1));
   fail_if(pd->cb_count != 0);
   eo_do(obj, eo_event_callback_del(EV_A_CHANGED, _a_changed_cb, (void *) 2));
   fail_if(pd->cb_count != -1);

   /* Global Freeze/thaw. */
   fcount = 0;
   cb_count = 0;
   pd->cb_count = 0;
   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1));
   fail_if(pd->cb_count != 1);

   eo_do(EO_CLASS, fcount = eo_event_global_freeze_get());
   fail_if(fcount != 0);

   eo_do(EO_CLASS, eo_event_global_freeze());
   eo_do(EO_CLASS, fcount = eo_event_global_freeze_get());
   fail_if(fcount != 1);

   eo_do(EO_CLASS, eo_event_global_freeze());
   eo_do(EO_CLASS, fcount = eo_event_global_freeze_get());
   fail_if(fcount != 2);

   eo_do(obj, eo_event_callback_priority_add(EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2));
   fail_if(pd->cb_count != 1);

   eo_do(obj, simple_a_set(2));
   fail_if(cb_count != 0);
   eo_do(EO_CLASS, eo_event_global_thaw());
   eo_do(EO_CLASS, fcount = eo_event_global_freeze_get());
   fail_if(fcount != 1);

   eo_do(EO_CLASS, eo_event_global_thaw());
   eo_do(EO_CLASS, fcount = eo_event_global_freeze_get());
   fail_if(fcount != 0);

   eo_do(obj, simple_a_set(3));
   fail_if(cb_count != 2);

   cb_count = 0;
   eo_do(EO_CLASS, eo_event_global_thaw());
   eo_do(EO_CLASS, fcount = eo_event_global_freeze_get());
   fail_if(fcount != 0);

   eo_do(EO_CLASS, eo_event_global_freeze());
   eo_do(EO_CLASS, fcount = eo_event_global_freeze_get());
   fail_if(fcount != 1);

   eo_do(obj, simple_a_set(2));
   fail_if(cb_count != 0);
   eo_do(EO_CLASS, eo_event_global_thaw());
   eo_do(EO_CLASS, fcount = eo_event_global_freeze_get());
   fail_if(fcount != 0);


   eo_unref(obj);
   eo_shutdown();
   return 0;
}

