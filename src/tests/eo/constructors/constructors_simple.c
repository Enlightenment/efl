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
EO_VOID_FUNC_BODYV(simple_##name##_set, EO_FUNC_CALL(name), int name); \
EO_FUNC_BODY(simple_##name##_get, int, 0);

_GET_SET_FUNC(a)
_GET_SET_FUNC(b)

extern int my_init_count;

static void
_simple_constructor(Eo *obj, void *class_data, int a)
{
   Private_Data *pd = class_data;

   eo_do_super(obj, MY_CLASS, eo_constructor());

   pd->a = a;
   printf("%s %d\n", __func__, pd->a);

   my_init_count++;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());

   my_init_count++;
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_destructor());

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

EO_VOID_FUNC_BODYV(simple_constructor, EO_FUNC_CALL(a), int a);

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
     EO_OP_FUNC_OVERRIDE(eo_destructor, _destructor),
     EO_OP_FUNC(simple_constructor, _simple_constructor, "Construct and set A."),
     EO_OP_FUNC(simple_a_set, _a_set, "Set property a"),
     EO_OP_FUNC(simple_a_get, _a_get, "Get property a"),
     EO_OP_FUNC(simple_b_set, _b_set, "Set property b"),
     EO_OP_FUNC(simple_b_get, _b_get, "Get property b"),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Private_Data),
     _class_constructor,
     _class_destructor
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_BASE_CLASS,
      MIXIN_CLASS, NULL);

