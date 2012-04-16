#include "Eobj.h"
#include "simple.h"

#include "config.h"

EAPI Eobj_Op SIMPLE_BASE_ID = 0;

static Eobj_Class *_my_class = NULL;

static void
_a_set(Eobj *obj __UNUSED__, void *class_data, va_list *list)
{
   Simple_Public_Data *pd = class_data;
   int a;
   a = va_arg(*list, int);
   printf("%s %d\n", eobj_class_name_get(_my_class), a);
   pd->a = a;
}

static void
_a_print(Eobj *obj __UNUSED__, void *class_data, va_list *list)
{
   Simple_Public_Data *pd = class_data;
   (void) list;
   printf("Print %s %d\n", eobj_class_name_get(_my_class), pd->a);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EOBJ_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_PRINT), _a_print),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
simple_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(SIMPLE_SUB_ID_A_SET, "i", "Set property A"),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Class_Description class_desc = {
        "Simple",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(&SIMPLE_BASE_ID, op_desc, SIMPLE_SUB_ID_LAST),
        NULL,
        sizeof(Simple_Public_Data),
        NULL,
        NULL,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, EOBJ_CLASS_BASE, NULL);
}
