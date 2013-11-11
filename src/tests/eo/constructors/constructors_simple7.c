#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple7.h"
#include "constructors_simple2.h"

#include "../eunit_tests.h"

#define MY_CLASS SIMPLE7_CLASS

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   /* FIXME: Actually test it. */
   eo2_do_super(obj, MY_CLASS, eo2_constructor());
}

static Eo2_Op_Description op_descs [] = {
     EO2_OP_FUNC_OVERRIDE(eo2_constructor, _constructor),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Simple7",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple7_class_get, &class_desc, SIMPLE2_CLASS, NULL);

