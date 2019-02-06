#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_mixin.h"
#include "mixin_simple.h"

#define MY_CLASS MIXIN_CLASS

static int
_ab_sum_get(Eo *obj, void *class_data EINA_UNUSED)
{
   int a = 0, b = 0;
   a = simple_a_get(obj);
   b = simple_b_get(obj);
   printf("%s %s\n", efl_class_name_get(MY_CLASS), __func__);
   return a + b;
}

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   return efl_constructor(efl_super(obj, MY_CLASS));
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EAPI EFL_FUNC_BODY(mixin_ab_sum_get, int, 0);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_constructor, _constructor),
         EFL_OBJECT_OP_FUNC(efl_destructor, _destructor),
         EFL_OBJECT_OP_FUNC(mixin_ab_sum_get, _ab_sum_get),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Mixin",
     EFL_CLASS_TYPE_MIXIN,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, EO_CLASS, NULL)

