#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_mixin.h"
#include "mixin_mixin3.h"
#include "mixin_simple.h"

#include "../eunit_tests.h"

#define MY_CLASS MIXIN3_CLASS

static int
_ab_sum_get(Eo *obj, void *class_data EINA_UNUSED)
{
   /* This cast is just a hack for the test. */
   Mixin3_Public_Data *pd = (Mixin3_Public_Data *) class_data;
   int sum = 0;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
   eo_do_super(obj, MY_CLASS, sum = mixin_ab_sum_get());

   ++sum;
   pd->count += 3;

     {
        int _a = 0, _b = 0;
        eo_do(obj, _a = simple_a_get(), _b = simple_b_get());
        fail_if(sum != _a + _b + 2);
     }

   return sum;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
     EO_OP_FUNC_OVERRIDE(eo_destructor, _destructor),
     EO_OP_FUNC_OVERRIDE(mixin_ab_sum_get, _ab_sum_get),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Mixin3",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Mixin3_Public_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(mixin3_class_get, &class_desc, MIXIN_CLASS, NULL);

