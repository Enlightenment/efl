#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple5.h"

#define MY_CLASS SIMPLE5_CLASS

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   (void) obj;
}

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_destructor, _destructor),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple5",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple5_class_get, &class_desc, EO_BASE_CLASS, NULL);

