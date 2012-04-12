#include "Eobj.h"
#include "mixin.h"
#include "simple.h"

#include "config.h"

EAPI Eobj_Op SIMPLE_BASE_ID = 0;

typedef struct
{
   int a;
   int b;
} Private_Data;

static Eobj_Class *_my_class = NULL;

#define _GET_SET_FUNC(name) \
static void \
_##name##_get(Eobj *obj __UNUSED__, void *class_data, va_list *list) \
{ \
   Private_Data *pd = class_data; \
   int *name; \
   name = va_arg(*list, int *); \
   *name = pd->name; \
   printf("%s %d\n", __func__, pd->name); \
} \
static void \
_##name##_set(Eobj *obj __UNUSED__, void *class_data, va_list *list) \
{ \
   Private_Data *pd = class_data; \
   int name; \
   name = va_arg(*list, int); \
   pd->name = name; \
   printf("%s %d\n", __func__, pd->name); \
}

_GET_SET_FUNC(a)
_GET_SET_FUNC(b)

static void
_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC_DESCRIPTION(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _a_set),
        EOBJ_OP_FUNC_DESCRIPTION(SIMPLE_ID(SIMPLE_SUB_ID_A_GET), _a_get),
        EOBJ_OP_FUNC_DESCRIPTION(SIMPLE_ID(SIMPLE_SUB_ID_B_SET), _b_set),
        EOBJ_OP_FUNC_DESCRIPTION(SIMPLE_ID(SIMPLE_SUB_ID_B_GET), _b_get),
        EOBJ_OP_FUNC_DESCRIPTION_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

const Eobj_Class *
simple_class_get(void)
{
   if (_my_class) return _my_class;

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(SIMPLE_SUB_ID_A_SET, "i", "Set property A"),
        EOBJ_OP_DESCRIPTION(SIMPLE_SUB_ID_A_GET, "i", "Get property A"),
        EOBJ_OP_DESCRIPTION(SIMPLE_SUB_ID_B_SET, "i", "Set property B"),
        EOBJ_OP_DESCRIPTION(SIMPLE_SUB_ID_B_GET, "i", "Get property B"),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Class_Description class_desc = {
        "Simple",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(&SIMPLE_BASE_ID, op_desc, SIMPLE_SUB_ID_LAST),
        NULL,
        sizeof(Private_Data),
        NULL,
        NULL,
        _class_constructor,
        NULL
   };

   return _my_class = eobj_class_new(&class_desc, EOBJ_CLASS_BASE, MIXIN_CLASS, NULL);
}
