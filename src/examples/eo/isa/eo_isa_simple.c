#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_isa_simple.h"

typedef struct
{
   int a;
} Private_Data;

#define MY_CLASS SIMPLE_CLASS

static int
_a_get(Eo *obj EINA_UNUSED, void *class_data)
{
   const Private_Data *pd = class_data;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
   return pd->a;
}

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Private_Data *pd = class_data;
   pd->a = a;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
}

static int
_a_power_3_get(Eo *obj EINA_UNUSED, void *class_data)
{
   const Private_Data *pd = class_data;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
   return pd->a * pd->a * pd->a;
}

EAPI EO_FUNC_BODY(simple_a_get, int, 0);
EAPI EO_VOID_FUNC_BODYV(simple_a_set, EO_FUNC_CALL(a), int a);

static Eo_Op_Description op_desc[] = {
     EO_OP_FUNC(simple_a_set, _a_set, "Set property A"),
     EO_OP_FUNC(simple_a_get, _a_get, "Get property A"),
     EO_OP_FUNC_OVERRIDE(interface_a_power_3_get, _a_power_3_get),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_desc),
     NULL,
     sizeof(Private_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, INTERFACE_CLASS, MIXIN_CLASS, NULL);
