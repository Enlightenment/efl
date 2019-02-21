#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple6.h"

#define MY_CLASS SIMPLE6_CLASS

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_destructor, _destructor),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple6",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple6_class_get, &class_desc, EO_CLASS, NULL);

