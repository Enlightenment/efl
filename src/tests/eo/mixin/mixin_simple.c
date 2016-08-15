#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "mixin_mixin.h"
#include "mixin_mixin2.h"
#include "mixin_mixin3.h"
#include "mixin_simple.h"

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

static Efl_Op_Description op_descs[] = {
     EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
     EFL_OBJECT_OP_FUNC(simple_a_get, _a_get),
     EFL_OBJECT_OP_FUNC(simple_b_set, _b_set),
     EFL_OBJECT_OP_FUNC(simple_b_get, _b_get),
};

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     EFL_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Private_Data),
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS,
      MIXIN3_CLASS, MIXIN2_CLASS, NULL);

