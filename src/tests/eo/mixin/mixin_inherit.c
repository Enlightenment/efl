#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_simple.h"
#include "mixin_mixin4.h"
#include "mixin_inherit.h"

#define MY_CLASS INHERIT_CLASS

static int
_a_get(Eo *obj, void *class_data EINA_UNUSED)
{
   int ret;
   eo2_do_super(obj, MY_CLASS, ret = simple_a_get());
   printf("%s %d\n", __func__, ret);

   return ret;
}

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC_OVERRIDE(simple_a_get, _a_get),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Inherit",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(inherit_class_get, &class_desc, SIMPLE_CLASS, MIXIN4_CLASS, NULL);
