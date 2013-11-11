#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "access_simple.h"
#include "access_simple_protected.h"
#include "access_inherit.h"

#define MY_CLASS INHERIT_CLASS

static void
_prot_print(Eo *obj, void *class_data EINA_UNUSED)
{
   Simple_Protected_Data *pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   printf("%s %d\n", __func__, pd->protected_x1);
}

EAPI EO2_VOID_FUNC_BODY(inherit_prot_print);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC(inherit_prot_print, _prot_print, "Print protected var x1."),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Inherit",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(inherit_class_get, &class_desc, SIMPLE_CLASS, NULL)
