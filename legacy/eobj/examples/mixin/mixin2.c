#include "Eobj.h"
#include "mixin.h"
#include "mixin2.h"
#include "simple.h"

#include "config.h"

#include "../eunit_tests.h"

#define MY_CLASS MIXIN2_CLASS

static void
_ab_sum_get(Eobj *obj, void *class_data, va_list *list)
{
   Mixin2_Public_Data *pd = class_data;
   int *sum = va_arg(*list, int *);
   printf("%s %s\n", eobj_class_name_get(MY_CLASS), __func__);
   eobj_do_super(obj, MIXIN_AB_SUM_GET(sum));

   ++*sum;
   pd->count += 2;

     {
        int _a, _b;
        eobj_do(obj, SIMPLE_A_GET(&_a), SIMPLE_B_GET(&_b));
        fail_if(*sum != _a + _b + 1);
     }
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
        EOBJ_OP_FUNC(MIXIN_ID(MIXIN_SUB_ID_AB_SUM_GET), _ab_sum_get),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

static const Eobj_Class_Description class_desc = {
     "Mixin2",
     EOBJ_CLASS_TYPE_MIXIN,
     EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     sizeof(Mixin2_Public_Data),
     _constructor,
     _destructor,
     _class_constructor,
     NULL
};

EOBJ_DEFINE_CLASS(mixin2_class_get, &class_desc, MIXIN_CLASS, NULL);

