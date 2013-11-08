#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"
#include "eo_test_class_simple.h"

/* The Max level to consider when working with the print cb. */
#define _EINA_LOG_MAX 2

struct log_ctx {
   const char *msg;
   const char *fnc;
   Eina_Bool did;
   int expected_level;
};

static struct log_ctx ctx;

static void
_eo_test_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
{
   struct log_ctx *myctx = data;

   if (level > _EINA_LOG_MAX)
      return;

   ck_assert_int_eq(level, myctx->expected_level);
   if (myctx->msg)
      ck_assert_str_eq(myctx->msg, fmt);
   ck_assert_str_eq(myctx->fnc, fnc);
   myctx->did = EINA_TRUE;

#ifdef SHOW_LOG
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
#else
   (void)d;
   (void)file;
   (void)line;
#endif
}

static void
_eo_test_safety_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
{
   struct log_ctx *myctx = data;
   va_list cp_args;
   const char *str;

   if (level > _EINA_LOG_MAX)
      return;

   va_copy(cp_args, args);
   str = va_arg(cp_args, const char *);
   va_end(cp_args);

   ck_assert_int_eq(level, myctx->expected_level);
   ck_assert_str_eq(fmt, "%s");
   ck_assert_str_eq(myctx->msg, str);
   ck_assert_str_eq(myctx->fnc, fnc);
   myctx->did = EINA_TRUE;

#ifdef SHOW_LOG
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
#else
   (void)d;
   (void)file;
   (void)line;
#endif
}

#define TEST_EO_ERROR(fn, _msg)                  \
  ctx.msg = _msg;                                \
  ctx.fnc = fn;                                  \
  ctx.did = EINA_FALSE;                          \
  ctx.expected_level = EINA_LOG_LEVEL_ERR

START_TEST(eo_inherit_errors)
{
   eo_init();
   eina_log_print_cb_set(_eo_test_print_cb, &ctx);

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
   eina_log_print_cb_set(_eo_test_print_cb, &ctx);

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
   eina_log_print_cb_set(_eo_test_safety_print_cb, &ctx);

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

void eo_test_class_errors(TCase *tc)
{
   tcase_add_test(tc, eo_inherit_errors);
   tcase_add_test(tc, eo_inconsistent_mro);
   tcase_add_test(tc, eo_bad_interface);
}
