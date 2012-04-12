#include "Eobj.h"
#include "simple.h"

#include "comp.h"
#include "../eunit_tests.h"

EAPI Eobj_Op COMP_BASE_ID = 0;

static Eobj_Class *_my_class = NULL;

static void
_a_get(Eobj *obj, Eobj_Op op, va_list *list)
{
   (void) op;
   int *a;
   a = va_arg(*list, int *);
   eobj_super_do(obj, SIMPLE_A_GET(a));
}

static void
_constructor(Eobj *obj)
{
   eobj_constructor_super(obj);

   Eobj *simple = eobj_add(SIMPLE_CLASS, obj);
   eobj_composite_object_attach(obj, simple);
   eobj_event_callback_forwarder_add(simple, SIG_A_CHANGED, obj);

   fail_if(eobj_composite_is(obj));
   fail_if(!eobj_composite_is(simple));

   eobj_unref(simple);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC_DESCRIPTION(SIMPLE_ID(SIMPLE_SUB_ID_A_GET), _a_get),
        EOBJ_OP_FUNC_DESCRIPTION_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
comp_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Class_Description class_desc = {
        "Comp",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        _constructor,
        NULL,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, EOBJ_CLASS_BASE,
         SIMPLE_CLASS, NULL);
}
