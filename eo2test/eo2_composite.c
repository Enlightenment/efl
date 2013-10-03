#include "Eo.h"
#include "eo2_simple.h"
#include "eo2_composite.h"

static int
_get(Eo *obj, void *obj_data EINA_UNUSED)
{
   int ret = 0;

   eo2_do_super(obj, EO2_COMPOSITE_CLASS, ret = eo2_get());

   return ret;
}

static void
_constructor(Eo *obj, void *obj_data EINA_UNUSED)
{
   eo2_do_super(obj, EO2_COMPOSITE_CLASS, eo2_constructor());

   Eo *simple = eo2_add(EO2_SIMPLE_CLASS, obj);
   eo_composite_attach(simple, obj);
   eo2_do(simple, eo2_event_callback_forwarder_add(EO2_EV_X_CHANGED, obj));

   eo2_do(obj, eo2_base_data_set("simple-obj", simple, NULL));

   eo_unref(simple);
}

static Eo2_Op_Description op_descs [] = {
       EO2_OP_FUNC_OVERRIDE(_constructor, eo2_constructor),
       EO2_OP_FUNC_OVERRIDE(_get, eo2_get),
       EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Eo2 Composite",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(eo2_composite_class_get, &class_desc,
                EO2_BASE_CLASS, EO2_SIMPLE_CLASS, NULL);
