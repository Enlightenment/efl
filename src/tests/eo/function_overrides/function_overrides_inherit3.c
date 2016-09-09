#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "function_overrides_simple.h"
#include "function_overrides_inherit2.h"
#include "function_overrides_inherit3.h"

#define MY_CLASS INHERIT3_CLASS

static void
_a_set(Eo *obj, void *class_data EINA_UNUSED, int a)
{
   printf("%s %d\n", efl_class_name_get(MY_CLASS), a);
   simple_a_set(efl_super(obj, MY_CLASS), a + 1);
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Inherit3",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(inherit3_class_get, &class_desc, INHERIT2_CLASS, NULL);

