#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple3.h"

#define MY_CLASS SIMPLE3_CLASS

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   (void) obj;
}

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC_OVERRIDE(eo2_constructor, _constructor),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Simple3",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple3_class_get, &class_desc, EO2_BASE_CLASS, NULL);

