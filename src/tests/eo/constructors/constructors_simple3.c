#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple3.h"

#define MY_CLASS SIMPLE3_CLASS

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   return obj;
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_constructor, _constructor),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple3",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple3_class_get, &class_desc, EO_CLASS, NULL);

