#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"
#include "eo_error_msgs.h"
#include "eo_test_class_simple.h"

static struct log_ctx ctx;

const Eo_Class *klass;

static void _destructor_unref(Eo *obj, void *class_data EINA_UNUSED)
{
   eo_do_super(obj, klass, eo_destructor());

   /* this triggers an eo stack overflow if not correctly protected */
   eo_unref(obj);
}

START_TEST(eo_destructor_unref)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   static Eo_Op_Description op_descs [] = {
        EO_OP_FUNC_OVERRIDE(eo_destructor, _destructor_unref),
   };

   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   klass = eo_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   Eo *obj = eo_add(klass, NULL);
   fail_if(!obj);

   TEST_EO_ERROR("_eo_unref", "Object %p deletion already triggered. You wrongly call eo_unref() within a destructor.");
   eo_unref(obj);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

START_TEST(eo_destructor_double_del)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   klass = eo_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   Eo *obj = eo_add(klass, NULL);
   eo_manual_free_set(obj, EINA_TRUE);
   fail_if(!obj);

   TEST_EO_ERROR("_eo_unref", "Object %p already destructed.");
   eo_del(obj);
   eo_del(obj);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

void eo_test_class_behaviour_errors(TCase *tc)
{
   tcase_add_test(tc, eo_destructor_unref);
   tcase_add_test(tc, eo_destructor_double_del);
}
