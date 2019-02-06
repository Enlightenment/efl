#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_test_class_singleton.h"
#include "eo_test_class_simple.h"

#define MY_CLASS SINGLETON_CLASS

static Eo *singleton_obj = NULL;

static Eo *
_singleton_efl_constructor(Eo *eo_obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   if (!singleton_obj)
     {
        singleton_obj = efl_add_ref(SIMPLE_CLASS, NULL);
     }
   else
     {
        efl_ref(singleton_obj);
     }

   return singleton_obj;
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_constructor, _singleton_efl_constructor),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Singleton",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(singleton_class_get, &class_desc, EO_CLASS, NULL)
