#include "Eobj.h"
#include "mixin.h"
#include "simple.h"
#include "simple4.h"

static Eobj_Class *_my_class = NULL;

const Eobj_Class *
simple4_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Class_Description class_desc = {
        "Simple4",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   _my_class = eobj_class_new(&class_desc, SIMPLE_CLASS, MIXIN_CLASS, NULL);
   return _my_class;
}
