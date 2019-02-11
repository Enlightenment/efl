#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple5.h"

#define MY_CLASS SIMPLE5_CLASS

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   (void) obj;
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_destructor, _destructor),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple5",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple5_class_get, &class_desc, EO_CLASS, NULL);

