#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_error_msgs.h"
#include "eo_test_class_simple.h"

static struct log_ctx ctx;

const Efl_Class *klass;

static void _destructor_unref(Eo *obj, void *class_data EINA_UNUSED)
{
   efl_destructor(efl_super(obj, klass));

   /* this triggers an eo stack overflow if not correctly protected */
   efl_unref(obj);
}

static Eina_Bool
_destructor_unref_class_initializer(Efl_Class *klass2)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_destructor, _destructor_unref),
   );

   return efl_class_functions_set(klass2, &ops, NULL);
}

EFL_START_TEST(efl_destructor_unref)
{
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        0,
        _destructor_unref_class_initializer,
        NULL,
        NULL
   };

   klass = efl_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   Eo *obj = efl_add_ref(klass, NULL);
   fail_if(!obj);

   TEST_EO_CRITICAL("efl_unref", "Obj:%s@%p. User refcount (%d) < 0. Too many unrefs.");
   efl_unref(obj);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

}
EFL_END_TEST

EFL_START_TEST(efl_destructor_double_del)
{
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        0,
        NULL,
        NULL,
        NULL
   };

   klass = efl_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   Eo *obj = efl_add_ref(klass, NULL);
   efl_manual_free_set(obj, EINA_TRUE);
   fail_if(!obj);

   TEST_EO_CRITICAL("efl_unref", "Obj:%s@%p. User refcount (%d) < 0. Too many unrefs.");
   efl_unref(obj);
   efl_unref(obj);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

}
EFL_END_TEST

void eo_test_class_behaviour_errors(TCase *tc)
{
   tcase_add_test(tc, efl_destructor_unref);
   tcase_add_test(tc, efl_destructor_double_del);
}
