#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"
#include "eo_error_msgs.h"
#include "eo_test_class_simple.h"

static struct log_ctx ctx;

START_TEST(eo_pure_virtual_fct_call)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   TEST_EO_ERROR("_eo_call_resolve", "in %s:%d: you called a pure virtual func '%s' (%d) of class '%s'.");
   eo_do(obj, simple_pure_virtual());
   fail_unless(ctx.did);

   eo_unref(obj);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   eo_shutdown();
}
END_TEST

START_TEST(eo_api_not_implemented_call)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   TEST_EO_ERROR("_eo_api_op_id_get", "Unable to resolve op for api func %p");
   eo_do(obj, simple_no_implementation());
   fail_unless(ctx.did);

   eo_unref(obj);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   eo_shutdown();
}
END_TEST

START_TEST(eo_op_not_found_in_super)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   TEST_EO_ERROR("_eo_call_resolve", "in %s:%d: func '%s' (%d) could not be resolved for class '%s' for super of '%s'.");
   eo_do_super(obj, SIMPLE_CLASS, simple_a_set(10));
   fail_unless(ctx.did);

   eo_unref(obj);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   eo_shutdown();
}
END_TEST

void eo_test_call_errors(TCase *tc)
{
   tcase_add_test(tc, eo_pure_virtual_fct_call);
   tcase_add_test(tc, eo_api_not_implemented_call);
   tcase_add_test(tc, eo_op_not_found_in_super);
}
