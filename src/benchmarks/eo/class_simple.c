#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "class_simple.h"

#define MY_CLASS SIMPLE_CLASS

static void
_other_call(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, Eo *other, int times)
{
   if (times > 0)
     {
        eo_do(other, simple_other_call(obj, times-1));
     }
}

EAPI EO_VOID_FUNC_BODYV(simple_other_call, EO_FUNC_CALL(other, times), Eo *other, int times);

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   pd->a = a;
}

EAPI EO_VOID_FUNC_BODYV(simple_a_set, EO_FUNC_CALL(a), int a);

static Eo_Op_Description op_desc[] = {
     EO_OP_FUNC(simple_a_set, _a_set),
     EO_OP_FUNC(simple_other_call, _other_call),
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_desc),
     NULL,
     sizeof(Simple_Public_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL)

