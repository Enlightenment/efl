#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin.h"
#include "simple7.h"
#include "simple2.h"

#include "../eunit_tests.h"

#define MY_CLASS SIMPLE7_CLASS

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   fail_if(eo_do_super(obj, eo_constructor()));
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple7",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(simple7_class_get, &class_desc, SIMPLE2_CLASS, NULL);

