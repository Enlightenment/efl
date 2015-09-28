#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple2.h"

#define MY_CLASS SIMPLE2_CLASS

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());

   return NULL;
}

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple2",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple2_class_get, &class_desc, EO_CLASS, NULL);

