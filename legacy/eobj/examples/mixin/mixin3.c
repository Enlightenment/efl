#include "Eo.h"
#include "mixin.h"
#include "mixin3.h"
#include "simple.h"

#include "config.h"

#include "../eunit_tests.h"

#define MY_CLASS MIXIN3_CLASS

static void
_ab_sum_get(const Eo *obj, const void *class_data EINA_UNUSED, va_list *list)
{
   /* This cast is just a hack for the test. */
   Mixin3_Public_Data *pd = (Mixin3_Public_Data *) class_data;
   int *sum = va_arg(*list, int *);
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
   eo_query_super(obj, mixin_ab_sum_get(sum));

   ++*sum;
   pd->count += 3;

     {
        int _a, _b;
        eo_query(obj, simple_a_get(&_a), simple_b_get(&_b));
        fail_if(*sum != _a + _b + 2);
     }
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_constructor_super(obj);
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_destructor_super(obj);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC_CONST(MIXIN_ID(MIXIN_SUB_ID_AB_SUM_GET), _ab_sum_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description class_desc = {
     "Mixin3",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     sizeof(Mixin3_Public_Data),
     _constructor,
     _destructor,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(mixin3_class_get, &class_desc, MIXIN_CLASS, NULL);

