#include "Eobj.h"
#include "mixin.h"
#include "simple.h"

EAPI Eobj_Op MIXIN_BASE_ID = 0;

static Eobj_Class *_my_class = NULL;

static void
_add_and_print_set(Eobj *obj, Eobj_Op op, va_list *list)
{
   (void) op;
   int a, b, x;
   eobj_do(obj, SIMPLE_A_GET(&a), SIMPLE_B_GET(&b));
   x = va_arg(*list, const int);
   printf("%s %d\n", __func__, a + b + x);
}

static void
_constructor(Eobj *obj)
{
   eobj_constructor_super(obj);
}

static void
_destructor(Eobj *obj)
{
   eobj_destructor_super(obj);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC_DESCRIPTION(MIXIN_ID(MIXIN_SUB_ID_ADD_AND_SET), _add_and_print_set),
        EOBJ_OP_FUNC_DESCRIPTION_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
mixin_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(MIXIN_SUB_ID_ADD_AND_SET, "i", "Add A + B + param and print it"),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Class_Description class_desc = {
        "Mixin",
        EOBJ_CLASS_TYPE_MIXIN,
        EOBJ_CLASS_DESCRIPTION_OPS(&MIXIN_BASE_ID, op_desc, MIXIN_SUB_ID_LAST),
        NULL,
        0,
        _constructor,
        _destructor,
        _class_constructor,
        NULL
   };

   _my_class = eobj_class_new(&class_desc, EOBJ_CLASS_BASE, NULL);

   return _my_class;
}
