#include "Eobj.h"
#include "simple.h"

#include "inherit2.h"
#include "inherit3.h"

#include "config.h"

static Eobj_Class *_my_class = NULL;

static void
_a_set(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   int a;
   a = va_arg(*list, int);
   printf("%s %d\n", eobj_class_name_get(_my_class), a);
   eobj_super_do(obj, SIMPLE_A_SET(a + 1));
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC_DESCRIPTION(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EOBJ_OP_FUNC_DESCRIPTION_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
inherit3_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Class_Description class_desc = {
        "Inherit3",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, INHERIT2_CLASS, NULL);
}
