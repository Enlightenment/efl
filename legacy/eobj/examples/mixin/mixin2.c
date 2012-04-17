#include "Eobj.h"
#include "mixin.h"
#include "mixin2.h"
#include "simple.h"

#include "config.h"

static const Eobj_Class *_my_class = NULL;

static void
_add_and_print_set(Eobj *obj, void *class_data __UNUSED__, va_list *list)
{
   int a, b, x;
   eobj_do(obj, SIMPLE_A_GET(&a), SIMPLE_B_GET(&b));
   x = va_arg(*list, const int);
   printf("%s %d\n", eobj_class_name_get(eobj_class_get(obj)), a + b + x);
   eobj_do_super(obj, MIXIN_ADD_AND_PRINT(x));
}

static void
_constructor(Eobj *obj, void *class_data __UNUSED__)
{
   eobj_constructor_super(obj);
}

static void
_destructor(Eobj *obj, void *class_data __UNUSED__)
{
   eobj_destructor_super(obj);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(MIXIN_ID(MIXIN_SUB_ID_ADD_AND_SET), _add_and_print_set),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
mixin2_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Class_Description class_desc = {
        "Mixin2",
        EOBJ_CLASS_TYPE_MIXIN,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        _constructor,
        _destructor,
        _class_constructor,
        NULL
   };

   _my_class = eobj_class_new(&class_desc, MIXIN_CLASS, NULL);

   return _my_class;
}
