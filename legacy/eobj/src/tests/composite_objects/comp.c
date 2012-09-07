#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"
#include "comp.h"

#include "../eunit_tests.h"

EAPI Eo_Op COMP_BASE_ID = 0;

#define MY_CLASS COMP_CLASS

static void
_a_get(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   int *a;
   a = va_arg(*list, int *);
   eo_do_super(obj, simple_a_get(a));
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());

   Eo *simple = eo_add(SIMPLE_CLASS, obj);
   eo_composite_attach(simple, obj);
   eo_do(simple, eo_event_callback_forwarder_add(EV_A_CHANGED, obj));

   fail_if(eo_composite_is(obj));
   fail_if(!eo_composite_is(simple));

   eo_do(obj, eo_base_data_set("simple-obj", simple, NULL));

   eo_unref(simple);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_GET), _a_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Comp",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(comp_class_get, &class_desc, EO_BASE_CLASS,
      SIMPLE_CLASS, NULL);

