#include "Eo.h"
#include "mixin.h"
#include "simple.h"

#include "config.h"

EAPI Eo_Op MIXIN_BASE_ID = 0;

#define MY_CLASS MIXIN_CLASS

static void
_add_and_print_set(const Eo *obj, const void *class_data EINA_UNUSED, va_list *list)
{
   int a, b, x;
   eo_query(obj, simple_a_get(&a), simple_b_get(&b));
   x = va_arg(*list, const int);
   printf("%s %d\n", __func__, a + b + x);
}

extern int my_init_count;

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_constructor_super(obj);

   my_init_count++;
}

static void
_destructor(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_destructor_super(obj);

   my_init_count--;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC_CONST(MIXIN_ID(MIXIN_SUB_ID_ADD_AND_SET), _add_and_print_set),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION_CONST(MIXIN_SUB_ID_ADD_AND_SET, "i", "Add A + B + param and print it"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     "Mixin",
     EO_CLASS_TYPE_MIXIN,
     EO_CLASS_DESCRIPTION_OPS(&MIXIN_BASE_ID, op_desc, MIXIN_SUB_ID_LAST),
     NULL,
     0,
     _constructor,
     _destructor,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(mixin_class_get, &class_desc, NULL, NULL);

