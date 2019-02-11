#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple7.h"
#include "constructors_simple2.h"

#include "../eunit_tests.h"

#define MY_CLASS SIMPLE7_CLASS

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   /* FIXME: Actually test it. */
   return efl_constructor(efl_super(obj, MY_CLASS));
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
     "Simple7",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple7_class_get, &class_desc, SIMPLE2_CLASS, NULL);

