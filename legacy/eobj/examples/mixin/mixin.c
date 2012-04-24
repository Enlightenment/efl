#include "Eobj.h"
#include "mixin.h"
#include "simple.h"

#include "config.h"

EAPI Eobj_Op MIXIN_BASE_ID = 0;

#define MY_CLASS MIXIN_CLASS

static void
_ab_sum_get(const Eobj *obj, const void *class_data EINA_UNUSED, va_list *list)
{
   int a, b;
   eobj_query(obj, SIMPLE_A_GET(&a), SIMPLE_B_GET(&b));
   int *sum = va_arg(*list, int *);
   if (sum)
      *sum = a + b;
   printf("%s %s\n", eobj_class_name_get(MY_CLASS), __func__);
}

static void
_constructor(Eobj *obj, void *class_data EINA_UNUSED)
{
   eobj_constructor_super(obj);
}

static void
_destructor(Eobj *obj, void *class_data EINA_UNUSED)
{
   eobj_destructor_super(obj);
}

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC_CONST(MIXIN_ID(MIXIN_SUB_ID_AB_SUM_GET), _ab_sum_get),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}


static const Eobj_Op_Description op_desc[] = {
     EOBJ_OP_DESCRIPTION_CONST(MIXIN_SUB_ID_AB_SUM_GET, "i", "Get the sum of a and b."),
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

EOBJ_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, NULL)

