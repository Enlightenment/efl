#include "Eobj.h"
#include "mixin.h"
#include "simple5.h"

#include "config.h"

static Eobj_Class *_my_class = NULL;

static void
_destructor(Eobj *obj, void *class_data __UNUSED__)
{
   (void) obj;
}

const Eobj_Class *
simple5_class_get(void)
{
   if (_my_class) return _my_class;

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

   _my_class = eobj_class_new(&class_desc, EOBJ_CLASS_BASE, NULL);
   return _my_class;
}
