#include "Eobj.h"
#include "mixin.h"
#include "simple5.h"

#include "config.h"

#define MY_CLASS SIMPLE5_CLASS

static void
_destructor(Eobj *obj, void *class_data EINA_UNUSED)
{
   (void) obj;
}

static const Eobj_Class_Description class_desc = {
     "Simple5",
     EOBJ_CLASS_TYPE_REGULAR,
     EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     _destructor,
     NULL,
     NULL
};

EOBJ_DEFINE_CLASS(simple5_class_get, &class_desc, EOBJ_BASE_CLASS, NULL);

