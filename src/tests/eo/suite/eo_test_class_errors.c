#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_error_msgs.h"
#include "eo_test_class_simple.h"

static struct log_ctx ctx;

START_TEST(eo_inherit_errors)
{
   efl_object_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Efl_Class *klass;
   const Efl_Class *klass_mixin;
   const Efl_Class *klass_simple;

   static const Efl_Class_Description class_desc_simple = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        EFL_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Efl_Class_Description class_desc_mixin = {
        EO_VERSION,
        "Mixin",
        EFL_CLASS_TYPE_MIXIN,
        EFL_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "General",
        EFL_CLASS_TYPE_MIXIN,
        EFL_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   klass_mixin = efl_class_new(&class_desc_mixin, NULL, NULL);
   fail_if(!klass_mixin);

   klass_simple = efl_class_new(&class_desc_simple, EO_CLASS, NULL);
   fail_if(!klass_simple);

   TEST_EO_ERROR("efl_class_new", "Non-regular classes ('%s') aren't allowed to inherit from regular classes ('%s').");
   klass = efl_class_new(&class_desc, klass_simple, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   class_desc.type = EFL_CLASS_TYPE_REGULAR;

   TEST_EO_ERROR("efl_class_new", "Regular classes ('%s') aren't allowed to inherit from non-regular classes ('%s').");
   klass = efl_class_new(&class_desc, klass_mixin, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   (void) klass;
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   efl_object_shutdown();
}
END_TEST

START_TEST(eo_inconsistent_mro)
{
   efl_object_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Efl_Class *klass;
   const Efl_Class *klass_mixin;
   const Efl_Class *klass_mixin2;
   const Efl_Class *klass_mixin3;

   static const Efl_Class_Description class_desc_simple = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        EFL_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Efl_Class_Description class_desc_mixin = {
        EO_VERSION,
        "Mixin",
        EFL_CLASS_TYPE_MIXIN,
        EFL_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Efl_Class_Description class_desc_mixin2 = {
        EO_VERSION,
        "Mixin2",
        EFL_CLASS_TYPE_MIXIN,
        EFL_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Efl_Class_Description class_desc_mixin3 = {
        EO_VERSION,
        "Mixin3",
        EFL_CLASS_TYPE_MIXIN,
        EFL_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   klass_mixin = efl_class_new(&class_desc_mixin, NULL, NULL);
   fail_if(!klass_mixin);

   klass_mixin2 = efl_class_new(&class_desc_mixin2, klass_mixin, NULL);
   fail_if(!klass_mixin2);

   klass_mixin3 = efl_class_new(&class_desc_mixin3, klass_mixin, NULL);
   fail_if(!klass_mixin3);

   TEST_EO_ERROR("_eo_class_mro_init", "Cannot create a consistent method resolution order for class '%s' because of '%s'.");
   klass = efl_class_new(&class_desc_simple, EO_CLASS, klass_mixin, klass_mixin2, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   klass = efl_class_new(&class_desc_simple, EO_CLASS, klass_mixin2, klass_mixin, NULL);
   fail_if(!klass);

   klass = efl_class_new(&class_desc_simple, EO_CLASS, klass_mixin2, klass_mixin3, NULL);
   fail_if(!klass);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   efl_object_shutdown();
}
END_TEST

static void _stub_class_constructor(Efl_Class *klass EINA_UNUSED) {}

START_TEST(eo_bad_interface)
{
   efl_object_init();
   eina_log_print_cb_set(eo_test_safety_print_cb, &ctx);

   const Efl_Class *klass;

   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Interface",
        EFL_CLASS_TYPE_INTERFACE,
        EFL_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        10,
        NULL,
        NULL
   };

   TEST_EO_ERROR("efl_class_new", "safety check failed: !desc->data_size is false");
   klass = efl_class_new(&class_desc, NULL, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   class_desc.data_size = 0;
   class_desc.class_constructor = _stub_class_constructor;

   klass = efl_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   class_desc.class_constructor = NULL;
   class_desc.class_destructor = _stub_class_constructor;

   klass = efl_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   class_desc.class_destructor = NULL;

   klass = efl_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   efl_object_shutdown();
}
END_TEST

static void _null_fct(Eo *eo_obj EINA_UNUSED, void *d EINA_UNUSED) { }
void null_fct (void) {}

START_TEST(eo_null_api)
{
   efl_object_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Efl_Class *klass;

   static Efl_Op_Description op_descs[] = {
        EFL_OBJECT_OP_FUNC(NULL, _null_fct),
   };
   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        EFL_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   TEST_EO_ERROR("_eo_class_funcs_set", "Class '%s': NULL API not allowed (NULL->%p '%s').");
   klass = efl_class_new(&class_desc, NULL, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   efl_object_shutdown();
}
END_TEST

START_TEST(eo_wrong_override)
{
   efl_object_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Efl_Class *klass;

   static Efl_Op_Description op_descs[] = {
        EFL_OBJECT_OP_FUNC_OVERRIDE(null_fct, _null_fct),
   };
   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        EFL_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   TEST_EO_ERROR("_eo_class_funcs_set", "Class '%s': Can't find api func description in class hierarchy (%p->%p) (%s).");
   klass = efl_class_new(&class_desc, NULL, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   efl_object_shutdown();
}
END_TEST

START_TEST(eo_api_redefined)
{
   efl_object_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Efl_Class *klass;

   static Efl_Op_Description op_descs[] = {
        EFL_OBJECT_OP_FUNC(null_fct, _null_fct),
        EFL_OBJECT_OP_FUNC(null_fct, NULL),
   };
   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        EFL_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   TEST_EO_ERROR("_eo_class_funcs_set", "Class '%s': API previously defined (%p->%p '%s').");
   klass = efl_class_new(&class_desc, NULL, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   efl_object_shutdown();
}
END_TEST

START_TEST(eo_dich_func_override)
{
   efl_object_init();
   eina_log_print_cb_set(eo_test_print_cb, &ctx);

   const Efl_Class *klass;

   static Efl_Op_Description op_descs[] = {
        EFL_OBJECT_OP_FUNC_OVERRIDE(simple_a_set, _null_fct),
        EFL_OBJECT_OP_FUNC_OVERRIDE(simple_a_set, NULL),
   };
   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        EFL_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   TEST_EO_ERROR("_vtable_func_set", "Class '%s': Overriding already set func %p for op %d (%s) with %p.");
   klass = efl_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(klass);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   efl_object_shutdown();
}
END_TEST

void eo_test_class_errors(TCase *tc)
{
   tcase_add_test(tc, eo_inherit_errors);
   tcase_add_test(tc, eo_inconsistent_mro);
   tcase_add_test(tc, eo_bad_interface);
#ifndef _WIN32
   /* This test is not relevant for WIN32. */
   tcase_add_test(tc, eo_null_api);
#endif
   tcase_add_test(tc, eo_wrong_override);
   tcase_add_test(tc, eo_api_redefined);
   tcase_add_test(tc, eo_dich_func_override);
}
