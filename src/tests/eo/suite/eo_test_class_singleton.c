#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo_test_class_singleton.h"
#include "eo_test_class_simple.h"

#define MY_CLASS SINGLETON_CLASS

static Eo *singleton_obj = NULL;

static Eo *
_singleton_eo_constructor(Eo *eo_obj EINA_UNUSED, void *_pd EINA_UNUSED)
{
   if (!singleton_obj)
     {
        singleton_obj = eo_add(SIMPLE_CLASS, NULL);
     }
   else
     {
        eo_ref(singleton_obj);
     }

   return singleton_obj;
}

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _singleton_eo_constructor),
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Singleton",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(singleton_class_get, &class_desc, EO_CLASS, NULL)
