#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>

#include "eina_suite.h"

struct ctx {
   int a, b;
};

#define VAL_A 1234
#define VAL_B 4567
#define RETVAL (void*)8901
#define CANCELVAL (void*)0xCA
#define COUNT 100

#ifdef EINA_SAFETY_CHECKS
struct log_ctx {
   const char *dom;
   const char *msg;
   const char *fnc;
   int level;
   Eina_Bool did;
   Eina_Bool just_fmt;
};

/* tests should not output on success, just uncomment this for debugging */
//#define SHOW_LOG 1

static void
_eina_test_safety_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
{
   struct log_ctx *ctx = data;
   va_list cp_args;
   const char *str;

   // Avoid checking non-coro messages for errors.
   if (d && !strcmp(d->name, ctx->dom))
     {
        va_copy(cp_args, args);
        str = va_arg(cp_args, const char *);
        va_end(cp_args);

        if (ctx->just_fmt)
          ck_assert_str_eq(fmt, ctx->msg);
        else
          {
             ck_assert_str_eq(fmt, "%s");
             ck_assert_str_eq(ctx->msg, str);
          }
        ck_assert_int_eq(level, ctx->level);
        ck_assert_str_eq(ctx->fnc, fnc);
        ctx->did = EINA_TRUE;

#ifdef SHOW_LOG
        eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
#else
     }
   else
     {
        eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
     }
   (void)d;
   (void)file;
   (void)line;
#endif
}
#endif


static const void *
coro_func_noyield(void *data, Eina_Bool canceled, Eina_Coro *coro EINA_UNUSED)
{
    EINA_LOG_DBG("entered");
   struct ctx *ctx = data;

   ck_assert_ptr_ne(ctx, NULL);
   ck_assert_int_eq(ctx->a, VAL_A);

   if (canceled) return CANCELVAL;

   ctx->b = VAL_B;

   EINA_LOG_DBG("returning normally");
   return RETVAL;
}

START_TEST(coro_noyield)
{
   EINA_LOG_DBG("coro_noyield started");
   Eina_Coro *coro;
   struct ctx ctx = {
     .a = VAL_A,
     .b = 0,
   };
   void *result = NULL;
   int i = 0;

   eina_init();

   EINA_LOG_DBG("Creating coro");
   coro = eina_coro_new(coro_func_noyield, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   EINA_LOG_DBG("Looping");
   while (eina_coro_run(&coro, &result, NULL))
     {
        i++;
        ck_assert_int_le(i, 1);
     }
   ck_assert_ptr_eq(coro, NULL);

   ck_assert_int_eq(ctx.b, VAL_B);
   ck_assert_ptr_eq(result, RETVAL);

   eina_shutdown();
   EINA_LOG_DBG("coro_noyield finished");
}
END_TEST

static const void *
coro_func_yield(void *data, Eina_Bool canceled, Eina_Coro *coro)
{
   struct ctx *ctx = data;
   char buf[256] = "hi there";
   int i;

   ck_assert_ptr_ne(ctx, NULL);
   ck_assert_int_eq(ctx->a, VAL_A);

   if (canceled) return CANCELVAL;

   ctx->b = 1;
   eina_coro_yield_or_return(coro, CANCELVAL);

   ctx->b = 2;
   eina_coro_yield_or_return(coro, CANCELVAL);

   for (i = 0; i < COUNT; i++) {
      ctx->b = i * 10;
      /* have some stuff on stack and write to it, so we validate
       * non-thread based solutions are really saving their stack.
       */
      snprintf(buf, sizeof(buf), "b=%d -----------------", ctx->b);
      eina_coro_yield_or_return(coro, CANCELVAL);
   }

   ctx->b = VAL_B;

   return RETVAL;
}

START_TEST(coro_yield)
{
   Eina_Coro *coro;
   struct ctx ctx = {
     .a = VAL_A,
     .b = 0,
   };
   Eina_Bool r;
   void *result = NULL;
   int i = 0;

   eina_init();

   coro = eina_coro_new(coro_func_yield, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   r = eina_coro_run(&coro, NULL, NULL);
   ck_assert_int_eq(r, EINA_TRUE);
   ck_assert_int_eq(ctx.b, 1);

   r = eina_coro_run(&coro, NULL, NULL);
   ck_assert_int_eq(r, EINA_TRUE);
   ck_assert_int_eq(ctx.b, 2);

   while (eina_coro_run(&coro, &result, NULL))
     {
        ck_assert_int_eq(ctx.b, i * 10);
        i++;
        ck_assert_int_le(i, COUNT);
        /* change caller's stack to guarantee coroutine stack is
         * being properly persisted.
         */
        memset(alloca(10), 0xff, 10);
     }
   ck_assert_ptr_eq(coro, NULL);
   ck_assert_int_eq(i, COUNT);

   ck_assert_int_eq(ctx.b, VAL_B);
   ck_assert_ptr_eq(result, RETVAL);

   eina_shutdown();
}
END_TEST

START_TEST(coro_cancel)
{
   Eina_Coro *coro;
   struct ctx ctx = {
     .a = VAL_A,
     .b = 0,
   };
   Eina_Bool r;

   eina_init();

   // cancel before it runs
   coro = eina_coro_new(coro_func_yield, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   ck_assert_ptr_eq(eina_coro_cancel(&coro), CANCELVAL);
   ck_assert_ptr_eq(coro, NULL);

   // cancel after single run
   coro = eina_coro_new(coro_func_yield, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   r = eina_coro_run(&coro, NULL, NULL);
   ck_assert_int_eq(r, EINA_TRUE);
   ck_assert_int_eq(ctx.b, 1);

   ck_assert_ptr_eq(eina_coro_cancel(&coro), CANCELVAL);
   ck_assert_ptr_eq(coro, NULL);
   ck_assert_int_eq(ctx.b, 1); // it's yielding after setting b=1

   eina_shutdown();
}
END_TEST

static Eina_Bool
_coro_hook_enter_success(void *data, const Eina_Coro *coro EINA_UNUSED)
{
   int *i = data;
   (*i) += 1;
   return EINA_TRUE;
}

static void
_coro_hook_exit(void *data, const Eina_Coro *coro EINA_UNUSED)
{
   int *i = data;
   (*i) += 100;
}

static void
_main_hook_enter(void *data, const Eina_Coro *coro EINA_UNUSED)
{
   int *i = data;
   (*i) += 10;
}

static Eina_Bool
_main_hook_exit_success(void *data, const Eina_Coro *coro EINA_UNUSED)
{
   int *i = data;
   (*i) += 1000;
   return EINA_TRUE;
}

START_TEST(coro_hook)
{
   Eina_Coro *coro;
   struct ctx ctx = {
     .a = VAL_A,
     .b = 0,
   };
   void *result = NULL;
   int i = 0, hooks_result = 0;

   eina_init();

   fail_unless(eina_coro_hook_add(_coro_hook_enter_success,
                                  _coro_hook_exit,
                                  _main_hook_enter,
                                  _main_hook_exit_success,
                                  &hooks_result));

   coro = eina_coro_new(coro_func_noyield, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   while (eina_coro_run(&coro, &result, NULL))
     {
        i++;
        ck_assert_int_le(i, 1);
     }
   ck_assert_ptr_eq(coro, NULL);

   ck_assert_int_eq(ctx.b, VAL_B);
   ck_assert_ptr_eq(result, RETVAL);

   fail_unless(eina_coro_hook_del(_coro_hook_enter_success,
                                  _coro_hook_exit,
                                  _main_hook_enter,
                                  _main_hook_exit_success,
                                  &hooks_result));

   ck_assert_int_eq(hooks_result, 1111);

   eina_shutdown();
}
END_TEST

#ifdef EINA_SAFETY_CHECKS
START_TEST(coro_new_null)
{
   Eina_Coro *coro;

   eina_init();

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   struct log_ctx lctx;

#define TEST_MAGIC_SAFETY(_dom, fn, _msg)              \
   lctx.dom = _dom;                              \
   lctx.msg = _msg;                              \
   lctx.fnc = fn;                                \
   lctx.just_fmt = EINA_FALSE;                   \
   lctx.level = EINA_LOG_LEVEL_ERR;              \
   lctx.did = EINA_FALSE

   eina_log_print_cb_set(_eina_test_safety_print_cb, &lctx);

   TEST_MAGIC_SAFETY("eina_safety", "eina_coro_new", "safety check failed: func == NULL");

   coro = eina_coro_new(NULL, NULL, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_eq(coro, NULL);

   fail_unless(lctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
#undef TEST_MAGIC_SAFETY

   eina_shutdown();
}
END_TEST

START_TEST(coro_yield_incorrect)
{
   Eina_Coro *coro;
   struct ctx ctx = {
     .a = VAL_A,
     .b = 0,
   };
   void *result = NULL;
   int i = 0;

   eina_init();

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   struct log_ctx lctx;

#define TEST_MAGIC_SAFETY(_dom, fn, _msg)        \
   lctx.dom = _dom;                              \
   lctx.msg = _msg;                              \
   lctx.fnc = fn;                                \
   lctx.just_fmt = EINA_TRUE;                    \
   lctx.level = EINA_LOG_LEVEL_CRITICAL;         \
   lctx.did = EINA_FALSE

   eina_log_print_cb_set(_eina_test_safety_print_cb, &lctx);

   coro = eina_coro_new(coro_func_noyield, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   TEST_MAGIC_SAFETY("eina_coro", "eina_coro_yield", "must be called from coroutine! coro=%p {func=%p data=%p turn=%s threads={%p%c %p%c} awaiting=%p}");
   fail_if(eina_coro_yield(coro));
   fail_unless(lctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
#undef TEST_MAGIC_SAFETY

   while (eina_coro_run(&coro, &result, NULL))
     {
        i++;
        ck_assert_int_le(i, 1);
     }
   ck_assert_ptr_eq(coro, NULL);

   ck_assert_int_eq(ctx.b, VAL_B);
   ck_assert_ptr_eq(result, RETVAL);

   eina_shutdown();
}
END_TEST

static const void *
coro_func_run_incorrect(void *data, Eina_Bool canceled, Eina_Coro *coro)
{
#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   struct log_ctx lctx;

#define TEST_MAGIC_SAFETY(_dom, fn, _msg)        \
   lctx.dom = _dom;                              \
   lctx.msg = _msg;                              \
   lctx.fnc = fn;                                \
   lctx.just_fmt = EINA_TRUE;                    \
   lctx.level = EINA_LOG_LEVEL_CRITICAL;         \
   lctx.did = EINA_FALSE

   eina_log_print_cb_set(_eina_test_safety_print_cb, &lctx);

   TEST_MAGIC_SAFETY("eina_coro", "eina_coro_run", "must be called from main thread! coro=%p {func=%p data=%p turn=%s threads={%p%c %p%c} awaiting=%p}");
   fail_if(eina_coro_run(&coro, NULL, NULL));
   fail_unless(lctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
#undef TEST_MAGIC_SAFETY

   return coro_func_noyield(data, canceled, coro);
}

START_TEST(coro_run_incorrect)
{
   Eina_Coro *coro;
   struct ctx ctx = {
     .a = VAL_A,
     .b = 0,
   };
   void *result = NULL;
   int i = 0;

   eina_init();

   coro = eina_coro_new(coro_func_run_incorrect, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   while (eina_coro_run(&coro, &result, NULL))
     {
        i++;
        ck_assert_int_le(i, 1);
     }
   ck_assert_ptr_eq(coro, NULL);

   ck_assert_int_eq(ctx.b, VAL_B);
   ck_assert_ptr_eq(result, RETVAL);

   eina_shutdown();
}
END_TEST

static Eina_Bool
_coro_hook_enter_failed(void *data, const Eina_Coro *coro EINA_UNUSED)
{
   int *i = data;
   (*i) += 2;
   return EINA_FALSE;
}

 static Eina_Bool
_main_hook_exit_failed(void *data, const Eina_Coro *coro EINA_UNUSED)
{
   int *i = data;
   (*i) += 2000;
   return EINA_FALSE;
}

START_TEST(coro_hook_failed)
{
   Eina_Coro *coro;
   struct ctx ctx = {
     .a = VAL_A,
     .b = 0,
   };
   void *result = NULL;
   int i = 0, hooks_result = 0;

   eina_init();

#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   struct log_ctx lctx;

#define TEST_MAGIC_SAFETY(_dom, fn, _msg)        \
   lctx.dom = _dom;                              \
   lctx.msg = _msg;                              \
   lctx.fnc = fn;                                \
   lctx.just_fmt = EINA_TRUE;                    \
   lctx.level = EINA_LOG_LEVEL_ERR;              \
   lctx.did = EINA_FALSE

   eina_log_print_cb_set(_eina_test_safety_print_cb, &lctx);

   fail_unless(eina_coro_hook_add(_coro_hook_enter_failed,
                                  _coro_hook_exit,
                                  _main_hook_enter,
                                  _main_hook_exit_success,
                                  &hooks_result));

   coro = eina_coro_new(coro_func_noyield, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   TEST_MAGIC_SAFETY("eina_coro", "_eina_coro_hooks_coro_enter", "failed hook enter=%p data=%p for coroutine coro=%p {func=%p data=%p turn=%s threads={%p%c %p%c} awaiting=%p}");

   while (eina_coro_run(&coro, &result, NULL))
     {
        i++;
        ck_assert_int_le(i, 1);
     }
   ck_assert_ptr_eq(coro, NULL);

   ck_assert_int_eq(ctx.b, 0);
   ck_assert_ptr_eq(result, CANCELVAL);

   fail_unless(lctx.did);

   fail_unless(eina_coro_hook_del(_coro_hook_enter_failed,
                                  _coro_hook_exit,
                                  _main_hook_enter,
                                  _main_hook_exit_success,
                                  &hooks_result));

   ck_assert_int_eq(hooks_result, 1112);

   // now fail main exit
   hooks_result = 0;

   fail_unless(eina_coro_hook_add(_coro_hook_enter_success,
                                  _coro_hook_exit,
                                  _main_hook_enter,
                                  _main_hook_exit_failed,
                                  &hooks_result));

   coro = eina_coro_new(coro_func_noyield, &ctx, EINA_CORO_STACK_SIZE_DEFAULT);
   ck_assert_ptr_ne(coro, NULL);

   TEST_MAGIC_SAFETY("eina_coro", "_eina_coro_hooks_main_exit", "failed hook exit=%p data=%p for main routine coro=%p {func=%p data=%p turn=%s threads={%p%c %p%c} awaiting=%p}");

   while (eina_coro_run(&coro, &result, NULL))
     {
        i++;
        ck_assert_int_le(i, 1);
     }
   ck_assert_ptr_eq(coro, NULL);

   ck_assert_int_eq(ctx.b, 0);
   ck_assert_ptr_eq(result, CANCELVAL);

   fail_unless(lctx.did);

   fail_unless(eina_coro_hook_del(_coro_hook_enter_success,
                                  _coro_hook_exit,
                                  _main_hook_enter,
                                  _main_hook_exit_failed,
                                  &hooks_result));

   ck_assert_int_eq(hooks_result, 2111);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
#undef TEST_MAGIC_SAFETY

   eina_shutdown();
}
END_TEST
#endif

void
eina_test_coro(TCase *tc)
{
   tcase_add_test(tc, coro_noyield);
   tcase_add_test(tc, coro_yield);
   // coro_await is tested in ecore_suite, so it hooks into the main loop
   tcase_add_test(tc, coro_cancel);
   tcase_add_test(tc, coro_hook);

#ifdef EINA_SAFETY_CHECKS
   tcase_add_test(tc, coro_new_null);
   tcase_add_test(tc, coro_yield_incorrect);
   tcase_add_test(tc, coro_run_incorrect);
   tcase_add_test(tc, coro_hook_failed);
#endif
}
