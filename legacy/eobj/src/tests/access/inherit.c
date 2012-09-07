#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple.h"
#include "simple_protected.h"

#include "inherit.h"

EAPI Eo_Op INHERIT_BASE_ID = 0;

#define MY_CLASS INHERIT_CLASS

static void
_prot_print(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   Simple_Protected_Data *pd = eo_data_get(obj, SIMPLE_CLASS);
   (void) list;
   printf("%s %d\n", __func__, pd->protected_x1);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(INHERIT_ID(INHERIT_SUB_ID_PROT_PRINT), _prot_print),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(INHERIT_SUB_ID_PROT_PRINT, "Print protected var x1."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Inherit",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&INHERIT_BASE_ID, op_desc, INHERIT_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(inherit_class_get, &class_desc, SIMPLE_CLASS, NULL)
