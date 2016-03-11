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
   printf("%s %d\n", eo_class_name_get(MY_CLASS), a);
   simple_a_print(obj);
   simple_a_set(eo_super(obj, MY_CLASS), a + 1);

   Eina_Bool called = EINA_FALSE;
   called = simple_a_print(eo_super(obj, MY_CLASS));
   fail_if(!called);
}

static Eina_Bool
_print(Eo *obj, void *class_data EINA_UNUSED)
{
   Eina_Bool called = EINA_FALSE;
   printf("Hey\n");
   called = inherit2_print(eo_super(obj, MY_CLASS));
   fail_if(called);

   return EINA_TRUE;
}

static Eina_Bool
_print2(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED)
{
   printf("Hey2\n");

   return EINA_TRUE;
}

static Eina_Bool
_class_print(Eo_Class *klass, void *data EINA_UNUSED)
{
   Eina_Bool called = EINA_FALSE;
   printf("Print %s-%s\n", eo_class_name_get(klass), eo_class_name_get(MY_CLASS));
   called = simple_class_print(eo_super(klass, MY_CLASS));
   fail_if(!called);

   called = simple_class_print2(eo_super(klass, MY_CLASS));
   fail_if(!called);

   return EINA_TRUE;
}

EAPI EO_FUNC_BODY(inherit2_print, _EO_EMPTY_HOOK, _EO_EMPTY_HOOK, Eina_Bool, EINA_FALSE);
EAPI EO_FUNC_BODY(inherit2_print2, _EO_EMPTY_HOOK, _EO_EMPTY_HOOK, Eina_Bool, EINA_FALSE);

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC(inherit2_print, _print),
     EO_OP_FUNC(inherit2_print2, _print2),
     EO_OP_CLASS_FUNC_OVERRIDE(simple_class_print, _class_print),
     EO_OP_FUNC_OVERRIDE(simple_a_set, _a_set),
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Inherit2",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(inherit2_class_get, &class_desc, INHERIT_CLASS, NULL);

