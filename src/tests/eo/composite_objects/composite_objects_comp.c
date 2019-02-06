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
   a = simple_a_get(efl_super(obj, MY_CLASS));

   return a;
}

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   Eo *simple = efl_add(SIMPLE_CLASS, obj);
   efl_composite_attach(obj, simple);
   efl_event_callback_forwarder_add(simple, EV_A_CHANGED, obj);

   fail_if(efl_composite_part_is(obj));
   fail_if(!efl_composite_part_is(simple));

   efl_key_data_set(obj, "simple-obj", simple);

   return obj;
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_constructor, _constructor),
         EFL_OBJECT_OP_FUNC(simple_a_get, _a_get),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Comp",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(comp_class_get, &class_desc, EO_CLASS,
      SIMPLE_CLASS, NULL);

