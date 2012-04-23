#include "Eobj.h"
#include "mixin.h"
#include "simple3.h"

#include "config.h"

#define MY_CLASS SIMPLE3_CLASS

static void
_constructor(Eobj *obj, void *class_data EINA_UNUSED)
{
   (void) obj;
}

static const Eobj_Class_Description class_desc = {
     "Simple3",
     EOBJ_CLASS_TYPE_REGULAR,
     EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _constructor,
     NULL,
     NULL,
     NULL
};

EOBJ_DEFINE_CLASS(simple3_class_get, &class_desc, EOBJ_BASE_CLASS, NULL);

