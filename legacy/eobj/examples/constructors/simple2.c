#include "Eo.h"
#include "mixin.h"
#include "simple2.h"

#include "config.h"

#define MY_CLASS SIMPLE2_CLASS

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_constructor_super(obj);

   eo_error_set(obj);
}

static const Eo_Class_Description class_desc = {
     "Simple2",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _constructor,
     NULL,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple2_class_get, &class_desc, EO_BASE_CLASS, NULL);

