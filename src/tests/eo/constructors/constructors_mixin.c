#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "constructors_mixin.h"
#include "constructors_simple.h"

#define MY_CLASS MIXIN_CLASS

static void
_add_and_print_set(Eo *obj, void *class_data EINA_UNUSED, int x)
{
   int a = 0, b = 0;
   a = simple_a_get(obj);
   b = simple_b_get(obj);
   printf("%s %d\n", __func__, a + b + x);
}

extern int my_init_count;

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   my_init_count++;

   return efl_constructor(efl_super(obj, MY_CLASS));
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));

   my_init_count--;
}

EAPI EFL_VOID_FUNC_BODYV(mixin_add_and_print, EFL_FUNC_CALL(x), int x);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(mixin_add_and_print, _add_and_print_set),
         EFL_OBJECT_OP_FUNC(efl_constructor, _constructor),
         EFL_OBJECT_OP_FUNC(efl_destructor, _destructor),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Mixin",
     EFL_CLASS_TYPE_MIXIN,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, EO_CLASS, NULL);

