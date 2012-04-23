#include "Eobj.h"
#include "mixin.h"
#include "simple2.h"

#include "config.h"

#define MY_CLASS SIMPLE2_CLASS

static void
_constructor(Eobj *obj, void *class_data EINA_UNUSED)
{
   eobj_constructor_super(obj);

   eobj_constructor_error_set(obj);
}

static const Eobj_Class_Description class_desc = {
     "Simple2",
     EOBJ_CLASS_TYPE_REGULAR,
     EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _constructor,
     NULL,
     NULL,
     NULL
};

EOBJ_DEFINE_CLASS(simple2_class_get, &class_desc, EOBJ_BASE_CLASS, NULL);

