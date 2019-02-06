#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_error_msgs.h"
#include "eo_test_class_simple.h"

static struct log_ctx ctx;

EFL_START_TEST(eo_pure_virtual_fct_call)
{
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   TEST_EO_ERROR("_efl_object_call_resolve", "in %s:%d: you called a pure virtual func '%s' (%d) of class '%s'.");
   simple_pure_virtual(obj);
   fail_unless(ctx.did);

   efl_unref(obj);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
}
EFL_END_TEST

EFL_START_TEST(eo_api_not_implemented_call)
{
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   TEST_EO_ERROR("simple_no_implementation", "Unable to resolve op for api func %p for obj=%p (%s)");
   simple_no_implementation(obj);
   fail_unless(ctx.did);

   efl_unref(obj);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
}
EFL_END_TEST

EFL_START_TEST(eo_op_not_found_in_super)
{
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   TEST_EO_ERROR("_efl_object_call_resolve", "in %s:%d: func '%s' (%d) could not be resolved for class '%s' for super of '%s'.");
   simple_a_set(efl_super(obj, SIMPLE_CLASS), 10);
   fail_unless(ctx.did);

   efl_unref(obj);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
}
EFL_END_TEST

//the fallback code that will be called

static Eina_Bool fallback_called;

static void
test_func(void)
{
   fallback_called = EINA_TRUE;
}

//implementation of the test function

EAPI void simple_error_test(Eo *obj);
EFL_VOID_FUNC_BODY_FALLBACK(simple_error_test, test_func(););
static void
_test(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{

}

//create a new class to call those things on

static Eina_Bool
_errorcase_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(simple_error_test, _test),
   );

   return efl_class_functions_set(klass, &ops, NULL, NULL);
}

static const Efl_Class_Description errorcase_class_desc = {
     EO_VERSION,
     "Simple errorcase",
     EFL_CLASS_TYPE_REGULAR,
     0,
     _errorcase_class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(simple_errorcase_class_get, &errorcase_class_desc, EO_CLASS, NULL)

EFL_START_TEST(eo_fallbackcall_execute)
{

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);

   fallback_called = EINA_FALSE;
   simple_error_test(NULL);
   ck_assert_int_eq(fallback_called, 1);

   fallback_called = EINA_FALSE;
   simple_error_test(obj);
   ck_assert_int_eq(fallback_called, 1);

}
EFL_END_TEST

void eo_test_call_errors(TCase *tc)
{
   tcase_add_test(tc, eo_pure_virtual_fct_call);
   tcase_add_test(tc, eo_api_not_implemented_call);
   tcase_add_test(tc, eo_op_not_found_in_super);
   tcase_add_test(tc, eo_fallbackcall_execute);
}
