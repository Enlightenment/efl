#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "interface_interface.h"
#include "interface_interface2.h"
#include "interface_simple.h"

typedef struct
{
   int a;
   int b;
} Private_Data;

#define MY_CLASS SIMPLE_CLASS

#define _GET_SET_FUNC(name) \
static int \
_##name##_get(Eo *obj EINA_UNUSED, void *class_data) \
{ \
   const Private_Data *pd = class_data; \
   printf("%s %d\n", __func__, pd->name); \
   return pd->name; \
} \
static void \
_##name##_set(Eo *obj EINA_UNUSED, void *class_data, int name) \
{ \
   Private_Data *pd = class_data; \
   pd->name = name; \
   printf("%s %d\n", __func__, pd->name); \
} \
EFL_VOID_FUNC_BODYV(simple_##name##_set, EFL_FUNC_CALL(name), int name); \
EFL_FUNC_BODY(simple_##name##_get, int, 0);

_GET_SET_FUNC(a)
_GET_SET_FUNC(b)

static int
_ab_sum_get(Eo *obj, void *class_data EINA_UNUSED)
{
   int a = 0, b = 0;
   a = simple_a_get(obj);
   b = simple_b_get(obj);
   printf("%s %s\n", efl_class_name_get(MY_CLASS), __func__);
   return a + b;
}

static int
_ab_sum_get2(Eo *obj, void *class_data EINA_UNUSED)
{
   int a = 0, b = 0;
   a = simple_a_get(obj);
   b = simple_b_get(obj);
   printf("%s %s\n", efl_class_name_get(MY_CLASS), __func__);
   return a + b + 1;
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_get, _a_get),
         EFL_OBJECT_OP_FUNC(simple_b_set, _b_set),
         EFL_OBJECT_OP_FUNC(simple_b_get, _b_get),
         EFL_OBJECT_OP_FUNC(interface_ab_sum_get, _ab_sum_get),
         EFL_OBJECT_OP_FUNC(interface2_ab_sum_get2, _ab_sum_get2),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Private_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, INTERFACE2_CLASS, NULL);
