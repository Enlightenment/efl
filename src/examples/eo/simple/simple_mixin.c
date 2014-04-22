#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple_mixin.h"
#include "simple_simple.h"

#define MY_CLASS MIXIN_CLASS

static int
_a_square_get(Eo *obj, void *class_data EINA_UNUSED)
{
   int a = eo_do(obj, simple_a_get());
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
   return a * a;
}

EAPI EO_FUNC_BODY(mixin_a_square_get, int, 0);

static Eo_Op_Description op_desc[] = {
     EO_OP_FUNC(mixin_a_square_get, _a_square_get, "Get the value of A^2"),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Mixin",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(op_desc),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, NULL)

