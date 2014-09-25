#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "composite_objects_simple.h"
#include "composite_objects_comp.h"

#include "../eunit_tests.h"

#define MY_CLASS COMP_CLASS

static int
_a_get(Eo *obj, void *class_data EINA_UNUSED)
{
   int a = 0;
   eo_do_super(obj, MY_CLASS, a = simple_a_get());

   return a;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());

   Eo *simple = eo_add(SIMPLE_CLASS, obj);
   eo_composite_attach(simple, obj);
   eo_do(simple, eo_event_callback_forwarder_add(EV_A_CHANGED, obj));

   fail_if(eo_composite_is(obj));
   fail_if(!eo_composite_is(simple));

   eo_do(obj, eo_key_data_set("simple-obj", simple, NULL));
}

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
     EO_OP_FUNC_OVERRIDE(simple_a_get, _a_get),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Comp",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(comp_class_get, &class_desc, EO_CLASS,
      SIMPLE_CLASS, NULL);

