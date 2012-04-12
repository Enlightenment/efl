#include "Eobj.h"
#include "simple.h"
#include "comp.h"

#include "../eunit_tests.h"

static int cb_called = EINA_FALSE;

static Eina_Bool
_a_changed_cb(void *data, Eobj *obj, const Eobj_Event_Description *desc, void *event_info)
{
   (void) desc;
   (void) obj;
   int new_a = *((int *) event_info);
   printf("%s event_info:'%d' data:'%s'\n", __func__, new_a, (const char *) data);

   cb_called = EINA_TRUE;

   return EINA_TRUE;
}

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eobj_init();

   Eobj *obj = eobj_add(COMP_CLASS, NULL);
   eobj_event_callback_add(obj, SIG_A_CHANGED, _a_changed_cb, NULL);

   int a;
   eobj_do(obj, SIMPLE_A_SET(1));
   fail_if(!cb_called);

   eobj_do(obj, SIMPLE_A_GET(&a));
   fail_if(a != 1);

   /* disable the callback forwarder, and fail if it's still called. */
   Eobj *simple = eobj_generic_data_get(obj, "simple-obj");
   eobj_ref(simple);
   eobj_event_callback_forwarder_del(simple, SIG_A_CHANGED, obj);

   cb_called = EINA_FALSE;
   eobj_do(obj, SIMPLE_A_SET(2));
   fail_if(cb_called);

   fail_if(!eobj_composite_is(simple));
   eobj_composite_object_detach(obj, simple);
   fail_if(eobj_composite_is(simple));
   eobj_composite_object_attach(obj, simple);
   fail_if(!eobj_composite_is(simple));

   eobj_unref(simple);
   eobj_unref(obj);

   eobj_shutdown();
   return 0;
}

