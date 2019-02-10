#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_simple.h"
#include "mixin_mixin4.h"
#include "mixin_inherit.h"

#define MY_CLASS INHERIT_CLASS

static int
_a_get(Eo *obj, void *class_data EINA_UNUSED)
{
   int ret = 0;
   ret = simple_a_get(efl_super(obj, MY_CLASS));
   printf("%s %d\n", __func__, ret);

   return ret;
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_a_get, _a_get),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Inherit",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(inherit_class_get, &class_desc, SIMPLE_CLASS, MIXIN4_CLASS, NULL);
