#include "Eobj.h"
#include "simple.h"

static Eina_Bool
_a_changed_cb(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info)
{
   /* FIXME: Actually print it. */
   (void) desc;
   (void) obj;
   int new_a = *((int *) event_info);
   printf("%s event_info:'%d' data:'%s'\n", __func__, new_a, (const char *) data);

   /* Fix data is NULL, stop. */
   return !!data;
}

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);

   /* The order of these two is undetermined. */
   eobj_event_callback_priority_add(obj, SIG_A_CHANGED, EOBJ_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, "CALLED");
   eobj_event_callback_priority_add(obj, SIG_A_CHANGED, EOBJ_CALLBACK_PRIORITY_BEFORE, _a_changed_cb, "CALLED2");
   /* This will be called afterwards. */
   eobj_event_callback_priority_add(obj, SIG_A_CHANGED, EOBJ_CALLBACK_PRIORITY_DEFAULT, _a_changed_cb, NULL);
   /* This will never be called because the previous callback returns NULL. */
   eobj_event_callback_priority_add(obj, SIG_A_CHANGED, EOBJ_CALLBACK_PRIORITY_AFTER, _a_changed_cb, "NOT CALLED");

   eobj_do(obj, SIMPLE_A_SET(1));

   eobj_event_callback_del_full(obj, SIG_A_CHANGED, _a_changed_cb, NULL);

   eobj_unref(obj);
   eobj_shutdown();
   return 0;
}

