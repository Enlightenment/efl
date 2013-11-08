#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple.h"

typedef struct
{
   int a;
   int b;
} Private_Data;

#define MY_CLASS SIMPLE_CLASS

static char *class_var = NULL;

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
EO2_VOID_FUNC_BODYV(simple_##name##_set, EO2_FUNC_CALL(name), int name); \
EO2_FUNC_BODY(simple_##name##_get, int, 0);

_GET_SET_FUNC(a)
_GET_SET_FUNC(b)

extern int my_init_count;

static void
_simple_constructor(Eo *obj, void *class_data, int a)
{
   Private_Data *pd = class_data;

   eo2_do_super(obj, MY_CLASS, eo2_constructor());

   pd->a = a;
   printf("%s %d\n", __func__, pd->a);

   my_init_count++;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo2_do_super(obj, MY_CLASS, eo2_constructor());

   my_init_count++;
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo2_do_super(obj, MY_CLASS, eo2_destructor());

   my_init_count--;
}

static void
_class_constructor(Eo_Class *klass EINA_UNUSED)
{
   class_var = malloc(10);
}

static void
_class_destructor(Eo_Class *klass EINA_UNUSED)
{
   free(class_var);
}

EO2_VOID_FUNC_BODYV(simple_constructor, EO2_FUNC_CALL(a), int a);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC_OVERRIDE(_constructor, eo2_constructor),
     EO2_OP_FUNC_OVERRIDE(_destructor, eo2_destructor),
     EO2_OP_FUNC(_simple_constructor, simple_constructor, "Construct and set A."),
     EO2_OP_FUNC(_a_set, simple_a_set, "Set property a"),
     EO2_OP_FUNC(_a_get, simple_a_get, "Get property a"),
     EO2_OP_FUNC(_b_set, simple_b_set, "Set property b"),
     EO2_OP_FUNC(_b_get, simple_b_get, "Get property b"),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Private_Data),
     _class_constructor,
     _class_destructor
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO2_BASE_CLASS,
      MIXIN_CLASS, NULL);

