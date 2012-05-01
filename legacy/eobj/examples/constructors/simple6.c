#include "Eo.h"
#include "mixin.h"
#include "simple6.h"

#include "config.h"

#define MY_CLASS SIMPLE6_CLASS

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_constructor_super(obj);

   eo_constructor_error_set(obj);
}

static const Eo_Class_Description class_desc = {
     "Simple6",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     _destructor,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple6_class_get, &class_desc, EO_BASE_CLASS, NULL);

