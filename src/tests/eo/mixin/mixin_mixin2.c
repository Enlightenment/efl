#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_mixin.h"
#include "mixin_mixin2.h"
#include "mixin_simple.h"

#include "../eunit_tests.h"

#define MY_CLASS MIXIN2_CLASS

static int
_ab_sum_get(Eo *obj, void *class_data)
{
   /* This cast is a hack just for the tests... */
   Mixin2_Public_Data *pd = (Mixin2_Public_Data *) class_data;
   int sum = 0;
   printf("%s %s\n", efl_class_name_get(MY_CLASS), __func__);
   sum = mixin_ab_sum_get(efl_super(obj, MY_CLASS));

   ++sum;
   pd->count += 2;

     {
        int _a = 0, _b = 0;
        _a = simple_a_get(obj);
        _b = simple_b_get(obj);
        fail_if(sum != _a + _b + 1);
     }

   return sum;
}

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   return obj;
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_constructor, _constructor),
         EFL_OBJECT_OP_FUNC(efl_destructor, _destructor),
         EFL_OBJECT_OP_FUNC(mixin_ab_sum_get, _ab_sum_get),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Mixin2",
     EFL_CLASS_TYPE_MIXIN,
     sizeof(Mixin2_Public_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(mixin2_class_get, &class_desc, MIXIN_CLASS, NULL);

