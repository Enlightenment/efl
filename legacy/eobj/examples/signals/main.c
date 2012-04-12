#include "Eobj.h"
#include "simple.h"

#include "../eunit_tests.h"

static int cb_count = 0;

static Eina_Bool
_a_changed_cb(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info)
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
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);
   Simple_Public_Data *pd = eobj_data_get(obj, SIMPLE_CLASS);

   /* The order of these two is undetermined. */
   eobj_event_callback_priority_add(obj, SIG_A_CHANGED, EOBJ_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 2);
   eobj_event_callback_priority_add(obj, SIG_A_CHANGED, EOBJ_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, (void *) 1);
   /* This will be called afterwards. */
   eobj_event_callback_priority_add(obj, SIG_A_CHANGED, EOBJ_CALLBACK_PRIORITY_DEFAULT, _a_changed_cb, (void *) 3);
   /* This will never be called because the previous callback returns NULL. */
   eobj_event_callback_priority_add(obj, SIG_A_CHANGED, EOBJ_CALLBACK_PRIORITY_AFTER, _a_changed_cb, (void *) 4);

   eobj_do(obj, SIMPLE_A_SET(1));

   fail_if(cb_count != 3);

   eobj_event_callback_del_full(obj, SIG_A_CHANGED, _a_changed_cb, (void *) 3);
   fail_if(pd->cb_count != 3);
   eobj_event_callback_del_full(obj, SIG_A_CHANGED, _a_changed_cb, (void *) 4);
   fail_if(pd->cb_count != 2);
   eobj_event_callback_del(obj, SIG_A_CHANGED, _a_changed_cb);
   fail_if(pd->cb_count != 1);
   eobj_event_callback_del(obj, SIG_A_CHANGED, _a_changed_cb);
   fail_if(pd->cb_count != 0);

   eobj_unref(obj);
   eobj_shutdown();
   return 0;
}

