#include "Eobj.h"
#include "mixin.h"
#include "simple3.h"

static Eobj_Class *_my_class = NULL;

static void
_constructor(Eobj *obj)
{
   (void) obj;
}

static void
_destructor(Eobj *obj)
{
   eobj_destructor_super(obj);
}

const Eobj_Class *
simple3_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Class_Description class_desc = {
        "Simple3",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        _constructor,
        _destructor,
        NULL,
        NULL
   };

   _my_class = eobj_class_new(&class_desc, EOBJ_CLASS_BASE, NULL);
   return _my_class;
}
