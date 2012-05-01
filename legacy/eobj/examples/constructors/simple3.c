#include "Eo.h"
#include "mixin.h"
#include "simple3.h"

#include "config.h"

#define MY_CLASS SIMPLE3_CLASS

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   (void) obj;
}

static const Eo_Class_Description class_desc = {
     "Simple3",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _constructor,
     NULL,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple3_class_get, &class_desc, EO_BASE_CLASS, NULL);

