#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin.h"
#include "mixin3.h"
#include "simple.h"

#include "../eunit_tests.h"

#define MY_CLASS MIXIN3_CLASS

static void
_ab_sum_get(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   /* This cast is just a hack for the test. */
   Mixin3_Public_Data *pd = (Mixin3_Public_Data *) class_data;
   int *sum = va_arg(*list, int *);
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
   eo_do_super(obj, mixin_ab_sum_get(sum));

   ++*sum;
   pd->count += 3;

     {
        int _a, _b;
        eo_do(obj, simple_a_get(&_a), simple_b_get(&_b));
        fail_if(*sum != _a + _b + 2);
     }
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_destructor());
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC(MIXIN_ID(MIXIN_SUB_ID_AB_SUM_GET), _ab_sum_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Mixin3",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     sizeof(Mixin3_Public_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(mixin3_class_get, &class_desc, MIXIN_CLASS, NULL);

