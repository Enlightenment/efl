#include "Eobj.h"
#include "simple.h"

#include "inherit.h"
#include "inherit2.h"

#include "config.h"

#include "../eunit_tests.h"

EAPI Eobj_Op INHERIT2_BASE_ID = 0;

static Eobj_Class *_my_class = NULL;

static void
_a_set(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   int a;
   a = va_arg(*list, int);
   printf("%s %d\n", eobj_class_name_get(_my_class), a);
   eobj_do(obj, SIMPLE_A_PRINT());
   eobj_super_do(obj, SIMPLE_A_SET(a + 1));
}

static void
_print(Eobj *obj, void *class_data __UNUSED__, va_list *list __UNUSED__)
{
   printf("Hey\n");
   fail_if(eobj_super_do(obj, INHERIT2_PRINT()));
}

static void
_print2(Eobj *obj __UNUSED__, void *class_data __UNUSED__, va_list *list __UNUSED__)
{
   printf("Hey2\n");
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC_DESCRIPTION(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EOBJ_OP_FUNC_DESCRIPTION(INHERIT2_ID(INHERIT2_SUB_ID_PRINT), _print),
        EOBJ_OP_FUNC_DESCRIPTION(INHERIT2_ID(INHERIT2_SUB_ID_PRINT2), _print2),
        EOBJ_OP_FUNC_DESCRIPTION_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
inherit2_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(INHERIT2_SUB_ID_PRINT, "", "Print hey"),
        EOBJ_OP_DESCRIPTION(INHERIT2_SUB_ID_PRINT2, "", "Print hey2"),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Class_Description class_desc = {
        "Inherit2",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(&INHERIT2_BASE_ID, op_desc, INHERIT2_SUB_ID_LAST),
        NULL,
        0,
        NULL,
        NULL,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, INHERIT_CLASS, NULL);
}
