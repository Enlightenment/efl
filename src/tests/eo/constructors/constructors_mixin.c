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
   int a, b;
   eo2_do(obj, a = simple_a_get(), b = simple_b_get());
   printf("%s %d\n", __func__, a + b + x);
}

extern int my_init_count;

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

EAPI EO2_VOID_FUNC_BODYV(mixin_add_and_print, EO2_FUNC_CALL(x), int x);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC(mixin_add_and_print, _add_and_print_set, "Add A + B + param and print it"),
     EO2_OP_FUNC_OVERRIDE(eo2_constructor, _constructor),
     EO2_OP_FUNC_OVERRIDE(eo2_destructor, _destructor),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Mixin",
     EO_CLASS_TYPE_MIXIN,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, EO2_BASE_CLASS, NULL);

