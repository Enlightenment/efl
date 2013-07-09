
#include "eo_simple.h"

EAPI Eo_Op SIMPLE_BASE_ID = 0;

typedef struct
{
   int a;
   int b;
} Private_Data;

#define MY_CLASS SIMPLE_CLASS

static void
_set(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   pd->a = va_arg(*list, int);
   pd->b = va_arg(*list, int);
}

static void
_get(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   const Private_Data *pd = class_data;
   int *a;
   a = va_arg(*list, int *);
   *a = pd->a;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_SET), _set),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_GET), _get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(SIMPLE_SUB_ID_SET, "Set properties"),
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

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_BASE_CLASS, NULL);
