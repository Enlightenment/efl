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
   eo_do_super(obj, MY_CLASS, eo_constructor());
}

static Eo_Op_Description op_descs [] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple7",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple7_class_get, &class_desc, SIMPLE2_CLASS, NULL);

