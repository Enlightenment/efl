#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple6.h"

#define MY_CLASS SIMPLE6_CLASS

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());

   eo_error_set(obj);
}

static Eo_Op_Description op_descs [] = {
     EO_OP_FUNC_OVERRIDE(eo_destructor, _destructor),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple6",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple6_class_get, &class_desc, EO_BASE_CLASS, NULL);

