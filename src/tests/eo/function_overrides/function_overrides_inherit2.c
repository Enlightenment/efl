#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "function_overrides_simple.h"
#include "function_overrides_inherit.h"
#include "function_overrides_inherit2.h"

#include "../eunit_tests.h"

#define MY_CLASS INHERIT2_CLASS

static void
_a_set(Eo *obj, void *class_data EINA_UNUSED, int a)
{
   printf("%s %d\n", efl_class_name_get(MY_CLASS), a);
   simple_a_print(obj);
   simple_a_set(efl_super(obj, MY_CLASS), a + 1);

   Eina_Bool called = EINA_FALSE;
   called = simple_a_print(efl_super(obj, MY_CLASS));
   fail_if(!called);
}

static Eina_Bool
_print(Eo *obj, void *class_data EINA_UNUSED)
{
   Eina_Bool called = EINA_FALSE;
   printf("Hey\n");
   called = inherit2_print(efl_super(obj, MY_CLASS));
   fail_if(called);

   return EINA_TRUE;
}

static Eina_Bool
_print2(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED)
{
   printf("Hey2\n");

   return EINA_TRUE;
}

EAPI EFL_FUNC_BODY(inherit2_print, Eina_Bool, EINA_FALSE);
EAPI EFL_FUNC_BODY(inherit2_print2, Eina_Bool, EINA_FALSE);

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(inherit2_print, _print),
         EFL_OBJECT_OP_FUNC(inherit2_print2, _print2),
         EFL_OBJECT_OP_FUNC(simple_a_set, _a_set),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Inherit2",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(inherit2_class_get, &class_desc, INHERIT_CLASS, NULL);

