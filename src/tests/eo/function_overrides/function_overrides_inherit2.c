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
   eo2_do(obj, simple_a_print());
   eo2_do_super(obj, MY_CLASS, simple_a_set(a + 1));

   Simple_Public_Data *pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   pd->a_print_called = EINA_FALSE;
   eo2_do_super(obj, MY_CLASS, simple_a_print());
   fail_if(!pd->a_print_called);
}

Eina_Bool inherit_print_called = EINA_FALSE;
Eina_Bool inherit2_print_called = EINA_FALSE;

static void
_print(Eo *obj, void *class_data EINA_UNUSED)
{
   printf("Hey\n");
   inherit2_print_called = EINA_FALSE;
   eo2_do_super(obj, MY_CLASS, inherit2_print());
   fail_if(inherit2_print_called);
   inherit_print_called = EINA_TRUE;
}

static void
_print2(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED)
{
   printf("Hey2\n");
   inherit2_print_called = EINA_TRUE;
}

static void
_class_print(Eo_Class *klass, void *data EINA_UNUSED)
{
   printf("Print %s-%s\n", eo_class_name_get(klass), eo_class_name_get(MY_CLASS));
   class_print_called = EINA_FALSE;
   eo2_do_super(klass, MY_CLASS, simple_class_print());
   fail_if(!class_print_called);

   class_print2_called = EINA_FALSE;
   eo2_do_super(klass, MY_CLASS, simple_class_print2());
   fail_if(!class_print2_called);
}

EAPI EO2_VOID_FUNC_BODY(inherit2_print);
EAPI EO2_VOID_FUNC_BODY(inherit2_print2);

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC(_print, inherit2_print, "Print hey"),
     EO2_OP_FUNC(_print2, inherit2_print2, "Print hey2"),
     EO2_OP_CLASS_FUNC_OVERRIDE(_class_print, simple_class_print),
     EO2_OP_FUNC_OVERRIDE(_a_set, simple_a_set),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Inherit2",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(inherit2_class_get, &class_desc, INHERIT_CLASS, NULL);

