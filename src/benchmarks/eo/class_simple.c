#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "class_simple.h"

#define MY_CLASS SIMPLE_CLASS

EOAPI const Efl_Event_Description _SIMPLE_FOO = EFL_EVENT_DESCRIPTION("foo");
EOAPI const Efl_Event_Description _SIMPLE_BAR = EFL_EVENT_DESCRIPTION("bar");

static void
_other_call(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, Eo *other, int times)
{
   if (times > 0)
     {
        simple_other_call(other, obj, times-1);
     }
}

EAPI EFL_VOID_FUNC_BODYV(simple_other_call, EFL_FUNC_CALL(other, times), Eo *other, int times);

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   pd->a = a;
}

EAPI EFL_VOID_FUNC_BODYV(simple_a_set, EFL_FUNC_CALL(a), int a);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
         EFL_OBJECT_OP_FUNC(simple_other_call, _other_call),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Simple_Public_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL)

