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
   Simple_Protected_Data *pd = efl_data_scope_get(obj, SIMPLE_CLASS);
   printf("%s %d\n", __func__, pd->protected_x1);
}

EAPI EFL_VOID_FUNC_BODY(inherit_prot_print);

static Efl_Op_Description op_descs[] = {
     EFL_OBJECT_OP_FUNC(inherit_prot_print, _prot_print),
};

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Inherit",
     EFL_CLASS_TYPE_REGULAR,
     EFL_CLASS_DESCRIPTION_OPS(op_descs),
     0,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(inherit_class_get, &class_desc, SIMPLE_CLASS, NULL)
