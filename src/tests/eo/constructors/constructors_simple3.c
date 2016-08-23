#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple3.h"

#define MY_CLASS SIMPLE3_CLASS

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   return obj;
}

static Efl_Op_Description op_descs[] = {
     EFL_OBJECT_OP_FUNC_OVERRIDE(efl_constructor, _constructor),
};

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple3",
     EFL_CLASS_TYPE_REGULAR,
     EFL_CLASS_DESCRIPTION_OPS(op_descs),
     0,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple3_class_get, &class_desc, EO_CLASS, NULL);

