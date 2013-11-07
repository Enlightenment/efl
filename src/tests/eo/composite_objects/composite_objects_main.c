#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "composite_objects_simple.h"
#include "composite_objects_comp.h"

#include "../eunit_tests.h"

static int cb_called = EINA_FALSE;

static Eina_Bool
_a_changed_cb(void *data, Eo *obj, const Eo_Event_Description *desc, void *event_info)
{
   (void) desc;
   (void) obj;
   int new_a = *((int *) event_info);
   printf("%s event_info:'%d' data:'%s'\n", __func__, new_a, (const char *) data);

   cb_called = EINA_TRUE;

   return EO_CALLBACK_CONTINUE;
}

int
main(int argc, char *argv[])
{
   (void) argc;
   (void) argv;
   eo_init();

   Eo *obj = eo2_add(COMP_CLASS, NULL);
   eo2_do(obj, eo2_event_callback_add(EV_A_CHANGED, _a_changed_cb, NULL));

   fail_if(!eo_isa(obj, COMP_CLASS));
   fail_if(!eo_isa(obj, SIMPLE_CLASS));

   int a;
   eo2_do(obj, simple_a_set(1));
   fail_if(!cb_called);

   eo2_do(obj, a = simple_a_get());
   fail_if(a != 1);

   /* disable the callback forwarder, and fail if it's still called. */
   Eo *simple;
   eo2_do(obj, simple = eo2_base_data_get("simple-obj"));
   eo_ref(simple);
   eo2_do(simple, eo2_event_callback_forwarder_del(EV_A_CHANGED, obj));

   cb_called = EINA_FALSE;
   eo2_do(obj, simple_a_set(2));
   fail_if(cb_called);

   fail_if(!eo_composite_is(simple));
   fail_if(!eo_composite_detach(simple, obj));
   fail_if(eo_composite_detach(simple, obj));
   fail_if(eo_composite_is(simple));
   fail_if(!eo_composite_attach(simple, obj));
   fail_if(!eo_composite_is(simple));
   fail_if(eo_composite_attach(simple, obj));

   fail_if(eo_composite_attach(obj, simple));

   eo_unref(simple);
   eo_unref(obj);

   eo_shutdown();
   return 0;
}

