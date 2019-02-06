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
   printf("%s %d\n", efl_class_name_get(MY_CLASS), a);
   pd->a = a;
}

static Eina_Bool
_a_print(Eo *obj EINA_UNUSED, void *class_data)
{
   Simple_Public_Data *pd = class_data;
   printf("Print %s %d\n", efl_class_name_get(MY_CLASS), pd->a);

   return EINA_TRUE;
}

static Eina_Bool
_class_print(Efl_Class *klass, void *class_data EINA_UNUSED)
{
   printf("Print %s-%s\n", efl_class_name_get(klass), efl_class_name_get(MY_CLASS));
   Eina_Bool called = EINA_FALSE;
   called = simple_class_print(efl_super(klass, MY_CLASS));
   fail_if(called);

   called = simple_class_print2(efl_super(klass, MY_CLASS));
   fail_if(called);

   return EINA_TRUE;
}

static Eina_Bool
_class_print2(Efl_Class *klass, void *class_data EINA_UNUSED)
{
   printf("Print %s-%s\n", efl_class_name_get(klass), efl_class_name_get(MY_CLASS));

   return EINA_TRUE;
}

EAPI EFL_VOID_FUNC_BODYV(simple_a_set, EFL_FUNC_CALL(a), int a);
EAPI EFL_FUNC_BODY(simple_a_print, Eina_Bool, EINA_FALSE);
EAPI EFL_FUNC_BODY_CONST(simple_class_print, Eina_Bool, EINA_FALSE);
EAPI EFL_FUNC_BODY_CONST(simple_class_print2, Eina_Bool, EINA_FALSE);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
         EFL_OBJECT_OP_FUNC(simple_a_print, _a_print),
         EFL_OBJECT_OP_FUNC(simple_class_print, _class_print),
         EFL_OBJECT_OP_FUNC(simple_class_print2, _class_print2),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Simple_Public_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_class_get, &class_desc, EO_CLASS, NULL);

