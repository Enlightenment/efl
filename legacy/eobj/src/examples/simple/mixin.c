#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin.h"
#include "simple.h"

EAPI Eo_Op MIXIN_BASE_ID = 0;

#define MY_CLASS MIXIN_CLASS

static void
_a_square_get(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   int a;
   eo_do(obj, simple_a_get(&a));
   int *ret = va_arg(*list, int *);
   if (ret)
      *ret = a * a;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(MIXIN_ID(MIXIN_SUB_ID_A_SQUARE_GET), _a_square_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}


static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(MIXIN_SUB_ID_A_SQUARE_GET, "Get the value of A^2"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Mixin",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(&MIXIN_BASE_ID, op_desc, MIXIN_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, NULL)

