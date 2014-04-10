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
   eo_do(obj, a = simple_a_get(), b = simple_b_get());
   printf("%s %d\n", __func__, a + b + x);
}

extern int my_init_count;

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

EAPI EO_VOID_FUNC_BODYV(mixin_add_and_print, EO_FUNC_CALL(x), int x);

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC(mixin_add_and_print, _add_and_print_set, "Add A + B + param and print it"),
     EO_OP_FUNC_OVERRIDE(eo_constructor, _constructor),
     EO_OP_FUNC_OVERRIDE(eo_destructor, _destructor),
     EO_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Mixin",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, EO_CLASS, NULL);

