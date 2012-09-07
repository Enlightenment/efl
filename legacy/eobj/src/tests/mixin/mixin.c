#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin.h"
#include "simple.h"

EAPI Eo_Op MIXIN_BASE_ID = 0;

#define MY_CLASS MIXIN_CLASS

static void
_ab_sum_get(Eo *obj, void *class_data EINA_UNUSED, va_list *list)
{
   int a, b;
   eo_do(obj, simple_a_get(&a), simple_b_get(&b));
   int *sum = va_arg(*list, int *);
   if (sum)
      *sum = a + b;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_destructor());
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC(MIXIN_ID(MIXIN_SUB_ID_AB_SUM_GET), _ab_sum_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}


static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(MIXIN_SUB_ID_AB_SUM_GET, "Get the sum of a and b."),
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

