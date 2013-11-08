#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_mixin.h"
#include "mixin_simple.h"

#define MY_CLASS MIXIN_CLASS

static int
_ab_sum_get(Eo *obj, void *class_data EINA_UNUSED)
{
   int a, b;
   eo2_do(obj, a = simple_a_get(), b = simple_b_get());
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
   return a + b;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo2_do_super(obj, MY_CLASS, eo2_constructor());
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo2_do_super(obj, MY_CLASS, eo2_destructor());
}

EAPI EO2_FUNC_BODY(mixin_ab_sum_get, int, 0);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC_OVERRIDE(_constructor, eo2_constructor),
     EO2_OP_FUNC_OVERRIDE(_destructor, eo2_destructor),
     EO2_OP_FUNC(_ab_sum_get, mixin_ab_sum_get, "Get the sum of a and b."),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Mixin",
     EO_CLASS_TYPE_MIXIN,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, EO2_BASE_CLASS, NULL)

