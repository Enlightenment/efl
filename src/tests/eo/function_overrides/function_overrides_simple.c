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

static Eina_Bool
_a_print(Eo *obj EINA_UNUSED, void *class_data)
{
   Simple_Public_Data *pd = class_data;
   printf("Print %s %d\n", eo_class_name_get(MY_CLASS), pd->a);

   return EINA_TRUE;
}

static Eina_Bool
_class_print(Eo_Class *klass, void *class_data EINA_UNUSED)
{
   printf("Print %s-%s\n", eo_class_name_get(klass), eo_class_name_get(MY_CLASS));
   Eina_Bool called = EINA_FALSE;
   eo2_do_super(klass, MY_CLASS, called = simple_class_print());
   fail_if(called);

   eo2_do_super(klass, MY_CLASS, called = simple_class_print2());
   fail_if(called);

   return EINA_TRUE;
}

static Eina_Bool
_class_print2(Eo_Class *klass, void *class_data EINA_UNUSED)
{
   printf("Print %s-%s\n", eo_class_name_get(klass), eo_class_name_get(MY_CLASS));

   return EINA_TRUE;
}

EAPI EO2_VOID_FUNC_BODYV(simple_a_set, EO2_FUNC_CALL(a), int a);
EAPI EO2_FUNC_BODY(simple_a_print, Eina_Bool, EINA_FALSE);
EAPI EO2_FUNC_BODY(simple_class_print, Eina_Bool, EINA_FALSE);
EAPI EO2_FUNC_BODY(simple_class_print2, Eina_Bool, EINA_FALSE);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC(simple_a_set, _a_set, "Set property A"),
     EO2_OP_FUNC(simple_a_print, _a_print, "Print property A"),
     EO2_OP_FUNC(simple_class_print, _class_print, "Print class name."),
     EO2_OP_FUNC(simple_class_print2, _class_print2, "Print2 class name."),
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

