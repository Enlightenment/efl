#include "Eo.h"
#include "simple.h"

#include "../eunit_tests.h"

static int cb_count = 0;

static Eina_Bool
_a_changed_cb(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   (void) desc;
   (void) obj;
   int new_a = *((int *) event_info);
   printf("%s event_info:'%d' data:'%d'\n", __func__, new_a, (int) data);

   cb_count++;

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
   Simple_Public_Data *pd = eo_data_get(obj, SIMPLE_CLASS);

   /* The order of these two is undetermined. */
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2);
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1);
   /* This will be called afterwards. */
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_DEFAULT, _a_changed_cb, (void *) 3);
   /* This will never be called because the previous callback returns NULL. */
   eo_event_callback_priority_add(obj, EV_A_CHANGED, EO_CALLBACK_PRIORITY_AFTER, _a_changed_cb, (void *) 4);

   eo_do(obj, simple_a_set(1));

   fail_if(cb_count != 3);

   eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 3);
   fail_if(pd->cb_count != 3);

   fail_if(eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 12));
   fail_if(pd->cb_count != 3);

   fail_if(4 != (int) eo_event_callback_del(obj, EV_A_CHANGED, _a_changed_cb, (void *) 4));
   fail_if(pd->cb_count != 2);
   eo_event_callback_del_lazy(obj, EV_A_CHANGED, _a_changed_cb);
   fail_if(pd->cb_count != 1);

   fail_if(eo_event_callback_del_lazy(obj, EV_A_CHANGED, NULL));
   fail_if(pd->cb_count != 1);
   fail_if(eo_event_callback_del(obj, EV_A_CHANGED, NULL, 0));
   fail_if(pd->cb_count != 1);

   eo_event_callback_del_lazy(obj, EV_A_CHANGED, _a_changed_cb);
   fail_if(pd->cb_count != 0);

   fail_if(eo_event_callback_del_lazy(obj, EV_A_CHANGED, _a_changed_cb));
   fail_if(pd->cb_count != 0);

   eo_unref(obj);
   eo_shutdown();
   return 0;
}

