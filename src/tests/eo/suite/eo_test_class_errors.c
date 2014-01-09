#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"
#include "eo_error_msgs.h"
#include "eo_test_class_simple.h"

static struct log_ctx ctx;

START_TEST(eo_inherit_errors)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Eo_Class *klass;
   const Eo_Class *klass_mixin;
   const Eo_Class *klass_simple;

   static const Eo_Class_Description class_desc_simple = {
        EO2_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO2_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Eo_Class_Description class_desc_mixin = {
        EO2_VERSION,
        "Mixin",
        EO_CLASS_TYPE_MIXIN,
        EO2_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static Eo_Class_Description class_desc = {
        EO2_VERSION,
        "General",
        EO_CLASS_TYPE_MIXIN,
        EO2_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   klass_mixin = eo_class_new(&class_desc_mixin, NULL, NULL);
   fail_if(!klass_mixin);

   klass_simple = eo_class_new(&class_desc_simple, EO2_BASE_CLASS, NULL);
   fail_if(!klass_simple);

   TEST_EO_ERROR("eo_class_new", "Non-regular classes ('%s') aren't allowed to inherit from regular classes ('%s').");
   klass = eo_class_new(&class_desc, klass_simple, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   class_desc.type = EO_CLASS_TYPE_REGULAR;

   TEST_EO_ERROR("eo_class_new", "Regular classes ('%s') aren't allowed to inherit from non-regular classes ('%s').");
   klass = eo_class_new(&class_desc, klass_mixin, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   (void) klass;
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

START_TEST(eo_inconsistent_mro)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Eo_Class *klass;
   const Eo_Class *klass_mixin;
   const Eo_Class *klass_mixin2;
   const Eo_Class *klass_mixin3;

   static const Eo_Class_Description class_desc_simple = {
        EO2_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO2_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Eo_Class_Description class_desc_mixin = {
        EO2_VERSION,
        "Mixin",
        EO_CLASS_TYPE_MIXIN,
        EO2_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Eo_Class_Description class_desc_mixin2 = {
        EO2_VERSION,
        "Mixin2",
        EO_CLASS_TYPE_MIXIN,
        EO2_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Eo_Class_Description class_desc_mixin3 = {
        EO2_VERSION,
        "Mixin3",
        EO_CLASS_TYPE_MIXIN,
        EO2_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   klass_mixin = eo_class_new(&class_desc_mixin, NULL, NULL);
   fail_if(!klass_mixin);

   klass_mixin2 = eo_class_new(&class_desc_mixin2, klass_mixin, NULL);
   fail_if(!klass_mixin2);

   klass_mixin3 = eo_class_new(&class_desc_mixin3, klass_mixin, NULL);
   fail_if(!klass_mixin3);

   TEST_EO_ERROR("_eo_class_mro_init", "Cannot create a consistent method resolution order for class '%s' because of '%s'.");
   klass = eo_class_new(&class_desc_simple, EO2_BASE_CLASS, klass_mixin, klass_mixin2, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   klass = eo_class_new(&class_desc_simple, EO2_BASE_CLASS, klass_mixin2, klass_mixin, NULL);
   fail_if(!klass);

   klass = eo_class_new(&class_desc_simple, EO2_BASE_CLASS, klass_mixin2, klass_mixin3, NULL);
   fail_if(!klass);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

static void _stub_class_constructor(Eo_Class *klass EINA_UNUSED) {}

START_TEST(eo_bad_interface)
{
   eo_init();
   eina_log_print_cb_set(eo_test_safety_print_cb, &ctx);

   const Eo_Class *klass;

   static Eo_Class_Description class_desc = {
        EO2_VERSION,
        "Interface",
        EO_CLASS_TYPE_INTERFACE,
        EO2_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        10,
        NULL,
        NULL
   };

   TEST_EO_ERROR("eo_class_new", "safety check failed: !desc->data_size is false");
   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   class_desc.data_size = 0;
   class_desc.class_constructor = _stub_class_constructor;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   class_desc.class_constructor = NULL;
   class_desc.class_destructor = _stub_class_constructor;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   class_desc.class_destructor = NULL;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

static void _null_fct(Eo *eo_obj EINA_UNUSED, void *d EINA_UNUSED) { }
void null_fct (void) {}

START_TEST(eo_null_api)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Eo_Class *klass;

   static Eo2_Op_Description op_descs[] = {
        EO2_OP_FUNC(NULL, _null_fct, "NULL API function"),
        EO2_OP_SENTINEL
   };
   static Eo_Class_Description class_desc = {
        EO2_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO2_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   TEST_EO_ERROR("_eo2_class_funcs_set", "Class '%s': NULL API not allowed (%d NULL->%p '%s').");
   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

START_TEST(eo_wrong_override)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Eo_Class *klass;

   static Eo2_Op_Description op_descs[] = {
        EO2_OP_FUNC_OVERRIDE(null_fct, _null_fct),
        EO2_OP_SENTINEL
   };
   static Eo_Class_Description class_desc = {
        EO2_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO2_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   TEST_EO_ERROR("_eo2_class_funcs_set", "Class '%s': Can't find api func description in class hierarchy (%p->%p).");
   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

START_TEST(eo_api_redefined)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Eo_Class *klass;

   static Eo2_Op_Description op_descs[] = {
        EO2_OP_FUNC(null_fct, _null_fct, "API function"),
        EO2_OP_FUNC(null_fct, NULL, "Redefining API function"),
        EO2_OP_SENTINEL
   };
   static Eo_Class_Description class_desc = {
        EO2_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO2_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   TEST_EO_ERROR("_eo2_class_funcs_set", "Class '%s': API previously defined (%d %p->%p '%s').");
   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

START_TEST(eo_dich_func_override)
{
   eo_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Eo_Class *klass;

   static Eo2_Op_Description op_descs[] = {
        EO2_OP_FUNC_OVERRIDE(simple_a_set, _null_fct),
        EO2_OP_FUNC_OVERRIDE(simple_a_set, NULL),
        EO2_OP_SENTINEL
   };
   static Eo_Class_Description class_desc = {
        EO2_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO2_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   TEST_EO_ERROR("_dich_func_set", "Class '%s': Overriding func %p for op %d (%s:'%s') with %p.");
   klass = eo_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   eo_shutdown();
}
END_TEST

void eo_test_class_errors(TCase *tc)
{
   tcase_add_test(tc, eo_inherit_errors);
   tcase_add_test(tc, eo_inconsistent_mro);
   tcase_add_test(tc, eo_bad_interface);
   tcase_add_test(tc, eo_null_api);
   tcase_add_test(tc, eo_wrong_override);
   tcase_add_test(tc, eo_api_redefined);
   tcase_add_test(tc, eo_dich_func_override);
}
