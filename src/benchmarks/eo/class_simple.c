#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "class_simple.h"

#define MY_CLASS SIMPLE_CLASS

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   pd->a = a;
}

EAPI EO_VOID_FUNC_BODYV(simple_a_set, EO_FUNC_CALL(a), int a);

static Eo_Op_Description op_desc[] = {
     EO_OP_FUNC(simple_a_set, _a_set, "Set property A"),
     EO_OP_SENTINEL
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

