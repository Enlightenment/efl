#include "Eobj.h"
#include "simple.h"
#include "simple_protected.h"

#include "inherit.h"

#include "config.h"

EAPI Eobj_Op INHERIT_BASE_ID = 0;

static Eobj_Class *_my_class = NULL;

static void
_prot_print(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   Simple_Protected_Data *pd = eobj_data_get(obj, SIMPLE_CLASS);
   (void) list;
   printf("%s %d\n", __func__, pd->protected_x1);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(INHERIT_ID(INHERIT_SUB_ID_PROT_PRINT), _prot_print),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
inherit_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(INHERIT_SUB_ID_PROT_PRINT, "", "Print protected var x1."),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Class_Description class_desc = {
        "Inherit",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(&INHERIT_BASE_ID, op_desc, INHERIT_SUB_ID_LAST),
        NULL,
        0,
        NULL,
        NULL,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, SIMPLE_CLASS, NULL);
}
