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
EFL_VOID_FUNC_BODYV(simple_##name##_set, EFL_FUNC_CALL(name), int name); \
EFL_FUNC_BODY(simple_##name##_get, int, 0);

_GET_SET_FUNC(a)
_GET_SET_FUNC(b)

extern int my_init_count;

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   my_init_count++;

   return efl_constructor(efl_super(obj, MY_CLASS));
}

static Eo*
_finalize(Eo *obj, void *class_data EINA_UNUSED)
{
   Eo *ret;
   Private_Data *pd = class_data;

   ret = efl_finalize(efl_super(obj, MY_CLASS));

   if (pd->a < 0)
     {
        return NULL;
     }

   return ret;
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));

   my_init_count--;
}

static void
_class_constructor(Efl_Class *klass EINA_UNUSED)
{
   class_var = malloc(10);
}

static void
_class_destructor(Efl_Class *klass EINA_UNUSED)
{
   free(class_var);
}

EFL_VOID_FUNC_BODYV(simple_constructor, EFL_FUNC_CALL(a), int a);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_constructor, _constructor),
         EFL_OBJECT_OP_FUNC(efl_destructor, _destructor),
         EFL_OBJECT_OP_FUNC(efl_finalize, _finalize),
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_get, _a_get),
         EFL_OBJECT_OP_FUNC(simple_b_set, _b_set),
         EFL_OBJECT_OP_FUNC(simple_b_get, _b_get),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Private_Data),
     _class_initializer,
     _class_constructor,
     _class_destructor
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS,
      MIXIN_CLASS, NULL);

