#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "function_overrides_simple.h"

#include "../eunit_tests.h"

#define MY_CLASS SIMPLE_CLASS

Eina_Bool class_print_called = EINA_FALSE;
Eina_Bool class_print2_called = EINA_FALSE;

static void
_a_set(Eo *obj EINA_UNUSED, void *class_data, int a)
{
   Simple_Public_Data *pd = class_data;
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   pd->a = a;
}

static void
_a_print(Eo *obj EINA_UNUSED, void *class_data)
{
   Simple_Public_Data *pd = class_data;
   printf("Print %s %d\n", eo_class_name_get(MY_CLASS), pd->a);
   pd->a_print_called = EINA_TRUE;
}

static void
_class_print(Eo_Class *klass)
{
   printf("Print %s-%s\n", eo_class_name_get(klass), eo_class_name_get(MY_CLASS));
   class_print_called = EINA_FALSE;
   eo2_do_super(klass, MY_CLASS, simple_class_print());
   fail_if(class_print_called);

   class_print2_called = EINA_FALSE;
   eo2_do_super(klass, MY_CLASS, simple_class_print2());
   fail_if(class_print2_called);

   class_print_called = EINA_TRUE;
}

static void
_class_print2(Eo_Class *klass)
{
   printf("Print %s-%s\n", eo_class_name_get(klass), eo_class_name_get(MY_CLASS));
   class_print2_called = EINA_TRUE;
}

EAPI EO2_VOID_FUNC_BODYV(simple_a_set, EO2_FUNC_CALL(a), int a);
EAPI EO2_VOID_FUNC_BODY(simple_a_print);
EAPI EO2_VOID_CLASS_FUNC_BODY(simple_class_print);
EAPI EO2_VOID_CLASS_FUNC_BODY(simple_class_print2);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC(_a_set, simple_a_set, "Set property A"),
     EO2_OP_FUNC(_a_print, simple_a_print, "Print property A"),
     EO2_OP_CLASS_FUNC(_class_print, simple_class_print, "Print class name."),
     EO2_OP_CLASS_FUNC(_class_print2, simple_class_print2, "Print2 class name."),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Simple_Public_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO2_BASE_CLASS, NULL);

