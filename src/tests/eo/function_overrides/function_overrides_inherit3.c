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
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   eo2_do_super(obj, MY_CLASS, simple_a_set(a + 1));
}

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC_OVERRIDE(_a_set, simple_a_set),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Inherit3",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(inherit3_class_get, &class_desc, INHERIT2_CLASS, NULL);

