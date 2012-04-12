#include "Eobj.h"
#include "simple.h"

#include "inherit.h"

static Eobj_Class *_my_class = NULL;

const Eobj_Class *
inherit_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Class_Description class_desc = {
        "Inherit",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, SIMPLE_CLASS, NULL);
}
