#include "eo_error_msgs.h"

void
eo_test_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
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

void
eo_test_safety_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
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
