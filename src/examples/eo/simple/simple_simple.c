#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "simple_simple.h"

EAPI Eo_Op SIMPLE_BASE_ID = 0;

typedef struct
{
   int a;
} Private_Data;

#define MY_CLASS SIMPLE_CLASS

static void
_a_get(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   const Private_Data *pd = class_data;
   int *a;
   a = va_arg(*list, int *);
   *a = pd->a;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
}

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   int a;
   a = va_arg(*list, int);
   pd->a = a;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
}

static void
_a_power_3_get(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   const Private_Data *pd = class_data;
   int *ret;
   ret = va_arg(*list, int *);
   if (ret)
      *ret = pd->a * pd->a * pd->a;
   printf("%s %s\n", eo_class_name_get(MY_CLASS), __func__);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_GET), _a_get),
        EO_OP_FUNC(INTERFACE_ID(INTERFACE_SUB_ID_A_POWER_3_GET), _a_power_3_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(SIMPLE_SUB_ID_A_SET, "Set property A"),
     EO_OP_DESCRIPTION(SIMPLE_SUB_ID_A_GET, "Get property A"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&SIMPLE_BASE_ID, op_desc, SIMPLE_SUB_ID_LAST),
     NULL,
     sizeof(Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, INTERFACE_CLASS, MIXIN_CLASS, NULL);
