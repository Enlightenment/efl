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
   int a = 0, b = 0;
   eo_do(obj, a = simple_a_get(), b = simple_b_get());
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
   return a + b;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

EAPI EO_FUNC_BODY(mixin_ab_sum_get, int, 0);

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
     EO_OP_FUNC_OVERRIDE(eo_destructor, _destructor),
     EO_OP_FUNC(mixin_ab_sum_get, _ab_sum_get, "Get the sum of a and b."),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Mixin",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, EO_CLASS, NULL)

