#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#define EFL_NOLEGACY_API_SUPPORT
#include <Efl_Core.h>
#include "efl_app_suite.h"
#include "../efl_check.h"

#define CHAIN_SIZE (3)
#define DEFAULT_ERROR (EFBIG)
#define DEFAULT_MSG ("Future resolve is working!")
#define DEFAULT_INT_VALUE (5466)
#define DEFAULT_INT_VALUE_AS_STRING ("5466")
#define DEFAULT_TIMEOUT (0.1)

#define VALUE_TYPE_CHECK(_v, _type)                     \
  do {                                                  \
     ck_assert_ptr_eq(_v.type, _type);                  \
} while(0)

#define ERROR_CHECK(_v, _errno)                                 \
  do {                                                          \
     Eina_Error _err;                                           \
     VALUE_TYPE_CHECK(_v, EINA_VALUE_TYPE_ERROR);               \
     fail_if(!eina_value_get(&_v, &_err));                      \
     ck_assert_int_eq(_err, _errno);                            \
  } while (0)

typedef struct _PromiseCtx {
   Eina_Promise *p;
   Efl_Object *t;
   Eina_Bool fail;
   Eina_Value *value;
} PromiseCtx;

typedef struct _Easy_Ctx {
   Eina_Bool success_called;
   Eina_Bool error_called;
   Eina_Bool free_called;
   Eina_Bool stop_loop;
} Easy_Ctx;

typedef struct _Race_Ctx {
   Eina_Value value;
   unsigned int success_idx;
   unsigned int failed;
   unsigned int success;
} Race_Ctx;

typedef struct _Race_Future_Ctx {
   Race_Ctx *race_ctx;
   unsigned int idx;
   int value;
} Race_Future_Ctx;

#ifdef EINA_SAFETY_CHECKS

#define LOG_CTX_MULTIPLE_FUNC_CTX_SET(_ctx, ...)                        \
  do {                                                                  \
     struct Func_Ctx tmp[] = { __VA_ARGS__, {NULL, NULL}};              \
                                                                        \
     _ctx.func_ctx = alloca(EINA_C_ARRAY_LENGTH(tmp) * sizeof (struct Func_Ctx)); \
     memcpy(_ctx.func_ctx, tmp, sizeof (tmp));                          \
     _ctx.level = EINA_LOG_LEVEL_ERR;                                   \
     _ctx.did = EINA_FALSE;                                             \
     _ctx.just_fmt = EINA_FALSE;                                        \
     _ctx.func_ctx_idx = 0;                                             \
  } while(0)

#define LOG_CTX_SET(_ctx, _fnc, _msg) LOG_CTX_MULTIPLE_FUNC_CTX_SET(_ctx, {_fnc, _msg})

typedef struct _Log_Ctx {
   struct Func_Ctx {
      const char *fnc;
      const char *msg;
   } *func_ctx;
   int level;
   int func_ctx_idx;
   Eina_Bool did;
   Eina_Bool just_fmt;
} Log_Ctx;

static void
_eina_test_safety_print_cb(const Eina_Log_Domain *d,
                           Eina_Log_Level level,
                           const char *file,
                           const char *fnc, int line,
                           const char *fmt,
                           void *data,
                           va_list args)
{
   Log_Ctx *ctx = data;
   va_list cp_args;
   const char *str;

   va_copy(cp_args, args);
   str = va_arg(cp_args, const char *);
   va_end(cp_args);

   ck_assert_ptr_ne(ctx->func_ctx[ctx->func_ctx_idx].msg, NULL);
   ck_assert_int_eq(level, ctx->level);
   if (ctx->just_fmt)
     ck_assert_str_eq(fmt, ctx->func_ctx[ctx->func_ctx_idx].msg);
   else
     {
        ck_assert_str_eq(fmt, "%s");
        ck_assert_str_eq(ctx->func_ctx[ctx->func_ctx_idx].msg, str);
     }
   ck_assert_str_eq(ctx->func_ctx[ctx->func_ctx_idx].fnc, fnc);
   ctx->did = EINA_TRUE;
   ctx->func_ctx_idx++;

   (void)d;
   (void)file;
   (void)line;
}
#endif

static void
_cancel(void *data, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   PromiseCtx *ctx = data;
   if (ctx->t) efl_del(ctx->t);
   ctx->t = NULL;
   eina_value_free(ctx->value);
   free(ctx);
}

static void
_promise_cancel_test(void *data, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   Eina_Bool *cancel_called = data;
   *cancel_called = EINA_TRUE;
}

static void
_simple_timeout(void *data, const Efl_Event *ev EINA_UNUSED)
{
   PromiseCtx *ctx = data;

   if (ctx->fail) eina_promise_reject(ctx->p, DEFAULT_ERROR);
   else
     {
        Eina_Value v;

        fail_if(!eina_value_copy(ctx->value, &v));
        eina_promise_resolve(ctx->p, v);
        eina_value_free(ctx->value);
     }
   efl_del(ctx->t);
   free(ctx);
}

static Eina_Future_Scheduler *
_future_scheduler_get(void)
{
   return efl_loop_future_scheduler_get(efl_main_loop_get());
}

static PromiseCtx *
_promise_ctx_new(void)
{
   PromiseCtx *ctx;
   ctx = calloc(1, sizeof(PromiseCtx));
   fail_if(!ctx);
   ctx->p = eina_promise_new(_future_scheduler_get(), _cancel, ctx);
   fail_if(!ctx->p);
   return ctx;
}

static Eina_Future *
_future_get(PromiseCtx *ctx, double timeout)
{
   Eina_Future *f;

   f = eina_future_new(ctx->p);
   fail_if(!f);
   ctx->t = efl_add(EFL_LOOP_TIMER_CLASS, efl_main_loop_get(),
               efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _simple_timeout, ctx),
               efl_loop_timer_interval_set(efl_added, timeout));
   fail_if(!ctx->t);
   return f;
}

static Eina_Future *
_fail_future_get(void)
{
   PromiseCtx *ctx = _promise_ctx_new();
   ctx->fail = EINA_TRUE;
   return _future_get(ctx, DEFAULT_TIMEOUT);
}

static Eina_Future *
_int_future_with_value_and_timeout(int value, double timeout)
{
   PromiseCtx *ctx = _promise_ctx_new();
   ctx->value = eina_value_util_int_new(value);
   fail_if(!ctx->value);
   return _future_get(ctx, timeout);
}

static Eina_Future *
_int_future_get(void)
{
   return _int_future_with_value_and_timeout(DEFAULT_INT_VALUE, DEFAULT_TIMEOUT);
}

static Eina_Future *
_str_future_get(void)
{
   PromiseCtx *ctx = _promise_ctx_new();
   ctx->value = eina_value_util_string_new(DEFAULT_MSG);
   fail_if(!ctx->value);
   return _future_get(ctx, DEFAULT_TIMEOUT);
}

static Eina_Value
_simple_err(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   ERROR_CHECK(v, DEFAULT_ERROR);
   ecore_main_loop_quit();
   return v;
}

static Eina_Value
_simple_ok(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   const char *msg;

   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
   fail_if(!eina_value_get(&v, &msg));
   ck_assert_str_eq(DEFAULT_MSG, msg);
   ecore_main_loop_quit();
   return v;
}

static Eina_Value
_chain_stop(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   int *i = data;
   fail_if(*i != CHAIN_SIZE);
   ecore_main_loop_quit();
   return v;
}

static Eina_Value
_chain_no_error(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Value new_v;
   static int count = DEFAULT_INT_VALUE;
   int current_i;
   int *i = data;

   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_INT);
   fail_if(!eina_value_get(&v, &current_i));
   fail_if(current_i != count++);
   fail_if(!eina_value_setup(&new_v, EINA_VALUE_TYPE_INT));
   fail_if(!eina_value_set(&new_v, count));
   (*i)++;
   return new_v;
}

static Eina_Value
_chain_error(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   int *i = data;

   ERROR_CHECK(v, DEFAULT_ERROR);
   (*i)++;
   return v;
}

static Eina_Value
_cancel_cb(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Value new_v;
   int *cancel_count = data;

   fail_if(!eina_value_setup(&new_v, EINA_VALUE_TYPE_INT));
   ERROR_CHECK(v, ECANCELED);
   (*cancel_count)++;
   /* Although this function returns an INT Eina_Value, the next
      _cancel_cb must receive a EINA_VALUE_TYPE_ERROR as ECANCELED */
   return new_v;
}

static Eina_Value
_inner_resolve(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Value new_v;
   fail_if(!eina_value_setup(&new_v, EINA_VALUE_TYPE_STRING));
   fail_if(!eina_value_set(&new_v, DEFAULT_MSG));
   eina_promise_resolve(data, new_v);
   return v;
}

static Eina_Value
_inner_fail(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   eina_promise_reject(data, DEFAULT_ERROR);
   return v;
}

static void
_inner_promise_cancel(void *data EINA_UNUSED, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   //This must never happen...
   fail_if(EINA_FALSE);
}

static Eina_Value
_future_promise_create(void *data, const Eina_Value v EINA_UNUSED, const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Promise *p;

   p = eina_promise_new(_future_scheduler_get(), _inner_promise_cancel, NULL);
   fail_if(!p);
   eina_future_then(_str_future_get(),
                    data ? _inner_fail : _inner_resolve,
                    p);
   return eina_promise_as_value(p);
}

static Eina_Value
_inner_future_last(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   if (data)
     ERROR_CHECK(v, DEFAULT_ERROR);
   else
     {
        const char *msg;
        VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
        fail_if(!eina_value_get(&v, &msg));
        ck_assert_str_eq(DEFAULT_MSG, msg);
     }
   ecore_main_loop_quit();
   return v;
}

static Eina_Value
_convert_check(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   const char *number;
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
   fail_if(!eina_value_get(&v, &number));
   ck_assert_str_eq(DEFAULT_INT_VALUE_AS_STRING, number);
   ecore_main_loop_quit();
   return v;
}

static Eina_Value
_easy_success(void *data, const Eina_Value v)
{
   Easy_Ctx *ctx = data;
   const char *msg;

   ctx->success_called = EINA_TRUE;
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
   fail_if(!eina_value_get(&v, &msg));
   ck_assert_str_eq(DEFAULT_MSG, msg);
   return v;
}

static Eina_Value
_easy_error(void *data, const Eina_Error err)
{
   Eina_Value v;
   Easy_Ctx *ctx = data;
   fail_if(err != EINVAL);
   fail_if(!eina_value_setup(&v, EINA_VALUE_TYPE_ERROR));
   fail_if(!eina_value_set(&v, err));
   ctx->error_called = EINA_TRUE;
   return v;
}

static void
_easy_free(void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Easy_Ctx *ctx = data;
   ctx->free_called = EINA_TRUE;
   if (ctx->stop_loop) ecore_main_loop_quit();
}

static Eina_Value
_all_cb(void *data, const Eina_Value array, const Eina_Future *dead EINA_UNUSED)
{
   unsigned int len, i, *expected_len = data;

   VALUE_TYPE_CHECK(array, EINA_VALUE_TYPE_ARRAY);
   len = eina_value_array_count(&array);
   fail_if(len != *expected_len);

   for (i = 0; i < len; i++)
     {
        Eina_Value v;

        fail_if(!eina_value_array_get(&array, i, &v));
        if (i % 2 == 0)
          {
             const char *msg;
             VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
             fail_if(!eina_value_get(&v, &msg));
             ck_assert_str_eq(DEFAULT_MSG, msg);
          }
        else
          {
             int ivalue = 0;
             VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_INT);
             fail_if(!eina_value_get(&v, &ivalue));
             fail_if(ivalue != DEFAULT_INT_VALUE);
          }
        eina_value_flush(&v);
     }
   ecore_main_loop_quit();
   return array;
}

static Eina_Value
_future_all_count(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   unsigned int *futures_called = data;
   (*futures_called)++;
   return v;
}

static Eina_Value
_race_cb(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Race_Future_Ctx *future_ctx = data;
   Race_Ctx *ctx = future_ctx->race_ctx;

   if (v.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;
        eina_value_get(&v, &err);
        fail_if(err != ECANCELED);
        ctx->failed++;
     }
   else if (v.type == EINA_VALUE_TYPE_INT)
     {
        int i;
        fail_if(!eina_value_get(&v, &i));
        fail_if(future_ctx->value != i);
        ctx->success++;
        ctx->success_idx = future_ctx->idx;
        fail_if(!eina_value_copy(&v, &ctx->value));
     }
   else fail_if(EINA_TRUE); //This is not supposed to happen!
   free(future_ctx);
   return v;
}

static Eina_Value
_race_end_cb(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Race_Ctx *ctx = data;
   unsigned int idx;
   Eina_Value_Struct *st;
   Eina_Value r;

   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRUCT);

   st = eina_value_memory_get(&v);
   fail_if(!st);
   fail_if(st->desc != EINA_PROMISE_RACE_STRUCT_DESC);
   fail_if(!eina_value_struct_get(&v, "index", &idx));
   fail_if(idx != ctx->success_idx);
   fail_if(!eina_value_struct_get(&v, "value", &r));
   fail_if(eina_value_compare(&r, &ctx->value));
   eina_value_flush(&r);
   ecore_main_loop_quit();
   return v;
}

static Eina_Value
_promise_empty_done(void *data, const Eina_Value value, const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Bool *pdone = data;

   if (!value.type) *pdone = EINA_TRUE;

   ecore_main_loop_quit();

   return value;
}

EFL_START_TEST(efl_test_timeout)
{
   Eina_Future *f;
   Eina_Bool done = EINA_FALSE;

   fail_if(!ecore_init());
   f = eina_future_then(efl_loop_timeout(efl_main_loop_get(), 0.0001),
                        _promise_empty_done, &done);
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();

   fail_unless(done);
}
EFL_END_TEST

EFL_START_TEST(efl_test_job)
{
   Eina_Future *f;
   Eina_Bool done = EINA_FALSE;

   fail_if(!ecore_init());
   f = eina_future_then(efl_loop_job(efl_main_loop_get()),
                        _promise_empty_done, &done);
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();

   fail_unless(done);
}
EFL_END_TEST

EFL_START_TEST(efl_test_idle)
{
   Eina_Future *f;
   Eina_Bool done = EINA_FALSE;

   fail_if(!ecore_init());
   f = eina_future_then(efl_loop_idle(efl_main_loop_get()),
                        _promise_empty_done, &done);
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();

   fail_unless(done);
}
EFL_END_TEST


EFL_START_TEST(efl_test_promise_future_success)
{
   Eina_Future *f;
   fail_if(!ecore_init());
   f = eina_future_then(_str_future_get(),
                        _simple_ok, NULL);
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_failure)
{
   Eina_Future *f;
   fail_if(!ecore_init());
   f = eina_future_then(_fail_future_get(),
                        _simple_err, NULL);
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_chain_no_error)
{
   Eina_Future *f;
   static int i = 0;

   fail_if(!ecore_init());
   f = eina_future_chain(_int_future_get(),
                         {.cb = _chain_no_error, .data = &i},
                         {.cb = _chain_no_error, .data = &i},
                         {.cb = _chain_no_error, .data = &i},
                         {.cb = _chain_stop, .data = &i});
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_chain_error)
{
   Eina_Future *f;
   static int i = 0;

   fail_if(!ecore_init());
   f = eina_future_chain(_fail_future_get(),
                         {.cb = _chain_error, .data = &i},
                         {.cb = _chain_error, .data = &i},
                         {.cb = _chain_error, .data = &i},
                         {.cb = _chain_stop, .data = &i});
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_cancel)
{
   fail_if(!ecore_init());
   int i;

   for (i = 0; i < 3; i++)
     {
        Eina_Promise *p;
        Eina_Future *first, *last, *middle;
        int cancel_count = 0;
        Eina_Bool cancel_called = EINA_FALSE;

        p = eina_promise_new(_future_scheduler_get(), _promise_cancel_test, &cancel_called);
        fail_if(!p);
        first = eina_future_new(p);
        fail_if(!first);
        if (i == 2)
          {
             Eina_Future *f;
             last = NULL;
             f = eina_future_then(first, _cancel_cb, &cancel_count);
             fail_if(!f);
             middle = eina_future_then(f, _cancel_cb, &cancel_count);
             fail_if(!middle);
             f = eina_future_then(middle, _cancel_cb, &cancel_count);
             fail_if(!f);
          }
        else
          {
             middle = NULL;
             last = eina_future_chain(first,
                                      {.cb = _cancel_cb, .data = &cancel_count},
                                      {.cb = _cancel_cb, .data = &cancel_count},
                                      {.cb = _cancel_cb, .data = &cancel_count});
             fail_if(!last);
          }
        if (i == 0)
          eina_future_cancel(last);
        else if (i == 1)
          eina_future_cancel(first);
        else
          eina_future_cancel(middle);
        fail_if(cancel_count != CHAIN_SIZE);
        fail_if(!cancel_called);
     }
   ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_inner_promise)
{
   Eina_Future *f;

   fail_if(!ecore_init());
   f = eina_future_chain(_str_future_get(),
                         {.cb = _future_promise_create, .data = NULL},
                         {.cb = _inner_future_last, .data = NULL});
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_inner_promise_fail)
{
   Eina_Future *f;
   void *data =(void *) 0x01;

   fail_if(!ecore_init());
   f = eina_future_chain(_str_future_get(),
                         {.cb = _future_promise_create, .data = data},
                         {.cb = _inner_future_last, .data = data});
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_implicit_cancel)
{
   Eina_Promise *p;
   Eina_Future *f;
   int cancel_count = 0;
   Eina_Bool cancel_called = EINA_FALSE;
   Eina_Value v = EINA_VALUE_EMPTY;

   fail_if(!ecore_init());

   p = eina_promise_new(_future_scheduler_get(), _promise_cancel_test, &cancel_called);
   fail_if(!p);
   f = eina_future_new(p);
   fail_if(!f);
   f = eina_future_chain(f,
                         {.cb = _cancel_cb, .data = &cancel_count},
                         {.cb = _cancel_cb, .data = &cancel_count},
                         {.cb = _cancel_cb, .data = &cancel_count});
   fail_if(!f);
   eina_promise_resolve(p, v);
   /*
     The promise was resolved, but the mainloop is not running.
     Since ecore_shutdown() will be called all the futures must be cancelled
   */
   ck_assert_int_eq(ecore_shutdown(), 0);
   //All the futures were cancelled at this point
   fail_if(cancel_count != CHAIN_SIZE);
   //Cancel should not be called, since we called eina_promise_resolve()
   fail_if(cancel_called);
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_convert)
{
   Eina_Future *f;

   fail_if(!ecore_init());
   f = eina_future_chain(_int_future_get(),
                         eina_future_cb_convert_to(EINA_VALUE_TYPE_STRING),
                         { .cb = _convert_check, .data = NULL });
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();

}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_easy)
{
   Eina_Future *f;
   Easy_Ctx easy1 = { 0 };
   Easy_Ctx easy2 = { 0 };
   Easy_Ctx easy3 = { 0 };

   easy3.stop_loop = EINA_TRUE;
   fail_if(!ecore_init());
   f = eina_future_then_from_desc(_str_future_get(),
                                  eina_future_cb_easy(_easy_success,
                                                      _easy_error,
                                                      _easy_free,
                                                      EINA_VALUE_TYPE_STRING,
                                                      &easy1));
   fail_if(!f);
   f = eina_future_then_easy(f, _easy_success, _easy_error,
                             _easy_free, NULL, &easy2);
   fail_if(!f);
   f = eina_future_chain_easy(f, {_easy_success, _easy_error,
          _easy_free, EINA_VALUE_TYPE_INT, &easy3});
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
   fail_if(!(easy1.success_called && !easy1.error_called && easy1.free_called));
   fail_if(!(easy2.success_called && !easy2.error_called && easy2.free_called));
   fail_if(!(!easy3.success_called && easy3.error_called && easy3.free_called));
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_all)
{
   Eina_Future *futures[11];
   unsigned int i, futures_called = 0, len = EINA_C_ARRAY_LENGTH(futures);

   fail_if(!ecore_init());
   for (i = 0; i < len - 1; i++)
     {
        Eina_Future *f;
        if (i % 2 == 0)
          f = _str_future_get();
        else
          f = _int_future_get();
        fail_if(!f);
        futures[i] = eina_future_then(f, _future_all_count, &futures_called);
        fail_if(!futures[i]);
     }

   futures[--len] = EINA_FUTURE_SENTINEL;
   fail_if(!eina_future_then(eina_future_all_array(futures), _all_cb, &len));
   ecore_main_loop_begin();
   ecore_shutdown();
   fail_if(futures_called != len);
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_future_race)
{
   Race_Ctx ctx = { 0 };
   Eina_Future *futures[11];
   unsigned int i, len = EINA_C_ARRAY_LENGTH(futures);
   double timeouts[10] = {
     2.0, 1.0, 0.5, 0.1, 4.5, 2.3, 5.6, 1.0, 0.5, 0.3
   };

   srand(time(NULL));
   fail_if(!ecore_init());
   for (i = 0; i < len - 1; i++)
     {
        Race_Future_Ctx *future_ctx = calloc(1, sizeof(Race_Future_Ctx));
        fail_if(!future_ctx);
        future_ctx->race_ctx = &ctx;
        future_ctx->idx = i;
        future_ctx->value = rand() % RAND_MAX;
        futures[i] = eina_future_then(_int_future_with_value_and_timeout(future_ctx->value, timeouts[i]),
                                      _race_cb, future_ctx);
        fail_if(!futures[i]);
     }

   futures[--len] = EINA_FUTURE_SENTINEL;
   fail_if(!eina_future_then(eina_future_race_array(futures),
                             _race_end_cb, &ctx));
   ecore_main_loop_begin();
   eina_value_flush(&ctx.value);
   ecore_shutdown();
   fail_if(ctx.success != 1);
   fail_if(ctx.failed != (len - 1));
}
EFL_END_TEST

static Eina_Value
_eo_future1_ok(Eo *eo EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   const char *number;

   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
   fail_if(!eina_value_get(&v, &number));
   ck_assert_str_eq(DEFAULT_INT_VALUE_AS_STRING, number);
   return v;
}

static Eina_Value
_eo_future1_err(Eo *eo EINA_UNUSED, void *data EINA_UNUSED, Eina_Error err)
{
   //Should not happen
   fail_if(EINA_TRUE);
   return eina_value_error_init(err);
}

static Eina_Value
_eo_future2_ok(Eo *eo EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   //Should not happen
   fail_if(EINA_TRUE);
   return v;
}

static Eina_Value
_eo_future2_err(Eo *eo EINA_UNUSED, void *data EINA_UNUSED, Eina_Error err)
{
   Eina_Value v;

   fail_if(err != EINVAL);
   fail_if(!eina_value_setup(&v, EINA_VALUE_TYPE_INT));
   fail_if(!eina_value_set(&v, DEFAULT_INT_VALUE));
   return v;
}

static void
_eo_future_free(Eo *eo, void *data EINA_UNUSED, const Eina_Future *dead EINA_UNUSED)
{
   int *free_called = efl_key_data_get(eo, "free_called");
   (*free_called)++;
}

static Eina_Value
_eo_chain_stop(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   int ivalue = 0;
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_INT);
   fail_if(!eina_value_get(&v, &ivalue));
   ck_assert_int_eq(ivalue, DEFAULT_INT_VALUE);
   ecore_main_loop_quit();
   return v;
}

EFL_START_TEST(efl_test_promise_eo)
{
   Eina_Future *f;
   Eo *obj;
   int free_called = 0;

   fail_if(!efl_object_init());
   fail_if(!ecore_init());

   //Use a random object..
   obj = efl_add_ref(EFL_IO_BUFFER_CLASS, NULL);
   fail_if(!obj);
   efl_key_data_set(obj, "free_called", &free_called);
   f = eina_future_chain(_int_future_get(),
                         eina_future_cb_convert_to(EINA_VALUE_TYPE_STRING),
                         efl_future_cb(obj, _eo_future1_ok, _eo_future1_err, _eo_future_free, EINA_VALUE_TYPE_STRING),
                         efl_future_cb(obj, _eo_future2_ok, _eo_future2_err, _eo_future_free, EINA_VALUE_TYPE_INT),
                         { .cb = _eo_chain_stop });
   fail_if(!f);
   ecore_main_loop_begin();
   efl_unref(obj);
   ecore_shutdown();
   efl_object_shutdown();
   ck_assert_int_eq(free_called, 2);
}
EFL_END_TEST

static Eina_Value
_eo_future_link_success(Eo *eo EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   //This should never happen
   fail_if(EINA_TRUE);
   return v;
}

static Eina_Value
_eo_future_link_err(Eo *eo, void *data EINA_UNUSED, Eina_Error err)
{
   int *err_called = efl_key_data_get(eo, "err_called");
   Eina_Value v;

   fail_if(!err_called);
   ck_assert_int_eq(err, ECANCELED);
   fail_if(!eina_value_setup(&v, EINA_VALUE_TYPE_ERROR));
   fail_if(!eina_value_set(&v, err));
   (*err_called)++;
   return v;
}

static Eina_Value
_eo_link_chain_end(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   int *err_called = data;
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_ERROR);
   ERROR_CHECK(v, ECANCELED);
   (*err_called)++;
   return v;
}

EFL_START_TEST(efl_test_promise_eo_link)
{
   Eina_Future *f;
   Eo *obj;
   int err_called = 0;

   fail_if(!efl_object_init());
   fail_if(!ecore_init());

   //Use a random object..
   obj = efl_add_ref(EFL_IO_BUFFER_CLASS, NULL);
   fail_if(!obj);

   efl_key_data_set(obj, "err_called", &err_called);
   fail_if(!efl_key_data_get(obj, "err_called"));
   f = efl_future_chain(obj, _int_future_get(),
                        {.success = _eo_future_link_success, .error = _eo_future_link_err},
                        {.success = _eo_future_link_success, .error = _eo_future_link_err},
                        {.success = _eo_future_link_success, .error = _eo_future_link_err},
                        {.success = _eo_future_link_success, .error = _eo_future_link_err},
                        {.success = _eo_future_link_success, .error = _eo_future_link_err});
   fail_if(!f);
   f = eina_future_then(f, _eo_link_chain_end, &err_called);
   fail_if(!f);
   /*
     Since the mainloop is not running and the object is deleted the whole chain must be cancelled.
   */
   efl_unref(obj);
   ecore_shutdown();
   efl_object_shutdown();
   ck_assert_int_eq(err_called, 6);
}
EFL_END_TEST

static Eina_Value
_err_ignored(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *f EINA_UNUSED)
{
   //Must be NULL since the error must be ignored.
   VALUE_TYPE_CHECK(v, NULL);
   ecore_main_loop_quit();
   return v;
}

EFL_START_TEST(efl_test_promise_future_ignore_error)
{
   Eina_Future *f;

   fail_if(!ecore_init());
   f = _fail_future_get();
   fail_if(!f);
   eina_future_chain(f, eina_future_cb_ignore_error(DEFAULT_ERROR),
                     {.cb = _err_ignored});
   ecore_main_loop_begin();
   ecore_shutdown();
}
EFL_END_TEST

#define PROMISE_LOG_DOMAIN_STR ("promise_test_domain")

typedef struct _Promise_Log_Ctx {
   Eina_Future_Cb_Log_Desc dbg;
   Eina_Future_Cb_Log_Desc crit;
   Eina_Future_Cb_Log_Desc warn;
   Eina_Future_Cb_Log_Desc info;
   Eina_Future_Cb_Log_Desc err;
   Eina_Bool dbg_log_ok;
   Eina_Bool crit_log_ok;
   Eina_Bool warn_log_ok;
   Eina_Bool info_log_ok;
   Eina_Bool err_log_ok;
} Promise_Log_Ctx;

static Eina_Value
_log_quit(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   int ivalue;

   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_INT);
   fail_if(!eina_value_get(&v, &ivalue));
   fail_if(ivalue != DEFAULT_INT_VALUE);
   ecore_main_loop_quit();
   return v;
}

static void
_log_test(const Eina_Log_Domain *d,
          Eina_Log_Level level,
          const char *file, const char *fnc, int line,
          const char *fmt, void *data, va_list args)
{
   Promise_Log_Ctx *ctx = data;
   Eina_Bool *log_ok;
   Eina_Future_Cb_Log_Desc *desc;
   va_list cpy;
   const char *prefix, *suffix, *value;

   if (strcmp(d->name, PROMISE_LOG_DOMAIN_STR))
     return;

   switch (level)
     {
      case EINA_LOG_LEVEL_DBG:
         log_ok = &ctx->dbg_log_ok;
         desc = &ctx->dbg;
         break;
      case EINA_LOG_LEVEL_CRITICAL:
         log_ok = &ctx->crit_log_ok;
         desc = &ctx->crit;
         break;
      case EINA_LOG_LEVEL_WARN:
         log_ok = &ctx->warn_log_ok;
         desc = &ctx->warn;
         break;
      case EINA_LOG_LEVEL_INFO:
         log_ok = &ctx->info_log_ok;
         desc = &ctx->info;
         break;
      default:
         log_ok = &ctx->err_log_ok;
         desc = &ctx->err;
     }

   ck_assert_str_eq(fnc, desc->func);
   ck_assert_str_eq(file, desc->file);
   ck_assert_str_eq(fmt, "%s%s%s");
   ck_assert_int_eq(desc->line, line);
   //FIXME: REmove this check
   ck_assert_int_eq(desc->level, level);
   va_copy(cpy, args);
   prefix = va_arg(cpy, const char *);
   value = va_arg(cpy, const char *);
   suffix = va_arg(cpy, const char *);
   ck_assert_str_eq(prefix, desc->prefix ? desc->prefix : "");
   ck_assert_str_eq(suffix, desc->suffix ? desc->suffix : "");
   ck_assert_str_eq(value, DEFAULT_INT_VALUE_AS_STRING);
   va_end(cpy);
   *log_ok = EINA_TRUE;
}

EFL_START_TEST(efl_test_promise_log)
{
   Promise_Log_Ctx ctx = { 0 };
   Eina_Future *f;
   int domain;

   fail_if(!ecore_init());

   domain = eina_log_domain_register(PROMISE_LOG_DOMAIN_STR, EINA_COLOR_CYAN);
   fail_if(domain < 0);
   eina_log_domain_level_set(PROMISE_LOG_DOMAIN_STR, EINA_LOG_LEVEL_DBG);
   ctx.dbg = (Eina_Future_Cb_Log_Desc){"dbg prefix:", " dbg suffix", __FILE__,
                                       __FUNCTION__, EINA_LOG_LEVEL_DBG,
                                       domain, __LINE__};
   ctx.crit = (Eina_Future_Cb_Log_Desc){NULL, NULL, __FILE__,
                                       __FUNCTION__, EINA_LOG_LEVEL_CRITICAL,
                                       domain, __LINE__};
   ctx.warn = (Eina_Future_Cb_Log_Desc){"warn prefix:", NULL, __FILE__,
                                       __FUNCTION__, EINA_LOG_LEVEL_WARN,
                                        domain, __LINE__};
   ctx.err = (Eina_Future_Cb_Log_Desc){NULL, " err suffix", __FILE__,
                                       __FUNCTION__, EINA_LOG_LEVEL_ERR,
                                       domain, __LINE__};
   ctx.info = (Eina_Future_Cb_Log_Desc){"info prefix:", " info suffix",
                                        __FILE__, __FUNCTION__, EINA_LOG_LEVEL_INFO,
                                        domain, __LINE__};
   eina_log_print_cb_set(_log_test, &ctx);
   f = eina_future_chain(_int_future_get(),
                         eina_future_cb_log_from_desc(ctx.dbg),
                         eina_future_cb_log_from_desc(ctx.crit),
                         eina_future_cb_log_from_desc(ctx.warn),
                         eina_future_cb_log_from_desc(ctx.err),
                         eina_future_cb_log_from_desc(ctx.info),
                         { _log_quit });
   fail_if(!f);
   ecore_main_loop_begin();
   eina_log_domain_unregister(domain);
   ecore_shutdown();
   fail_if(!ctx.dbg_log_ok);
   fail_if(!ctx.crit_log_ok);
   fail_if(!ctx.warn_log_ok);
   fail_if(!ctx.err_log_ok);
   fail_if(!ctx.info_log_ok);
}
EFL_END_TEST

#ifdef EINA_SAFETY_CHECKS

static void
_dummy_cancel(void *data EINA_UNUSED, const Eina_Promise *dead EINA_UNUSED)
{
}

EFL_START_TEST(efl_test_promise_null)
{
   Log_Ctx ctx = { 0 };
   Eina_Promise *p;

   fail_if(!ecore_init());

   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);
   LOG_CTX_SET(ctx, "eina_promise_new", "safety check failed: scheduler == NULL");
   p = eina_promise_new(NULL, _dummy_cancel, NULL);
   ck_assert_ptr_eq(p, NULL);
   fail_unless(ctx.did);

   LOG_CTX_SET(ctx, "eina_promise_new", "safety check failed: cancel_cb == NULL");
   p = eina_promise_new(_future_scheduler_get(), NULL, NULL);
   ck_assert_ptr_eq(p, NULL);
   fail_unless(ctx.did);

   ecore_shutdown();
   eina_log_print_cb_set(NULL, NULL);
}
EFL_END_TEST

EFL_START_TEST(efl_test_promise_reject_resolve_null)
{
   Log_Ctx ctx = { 0 };
   Eina_Value v;

   fail_if(!ecore_init());

   LOG_CTX_SET(ctx, "eina_promise_resolve", "safety check failed: (p) == NULL");
   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);
   eina_value_setup(&v, EINA_VALUE_TYPE_INT);
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_INT);
   eina_promise_resolve(NULL, v);
   fail_unless(ctx.did);

   LOG_CTX_SET(ctx, "eina_promise_reject", "safety check failed: (p) == NULL");
   eina_promise_reject(NULL, DEFAULT_ERROR);
   fail_unless(ctx.did);
   ecore_shutdown();
   eina_log_print_cb_set(NULL, NULL);
}
EFL_END_TEST

static Eina_Value
_future_null_cb(void *data, const Eina_Value v, const Eina_Future *dead)
{
   int err;
   int *cb_called = data;

   ck_assert_ptr_eq(dead, NULL);
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_ERROR);
   fail_if(!eina_value_get(&v, &err));
   ck_assert_int_eq(err, EINVAL);
   (*cb_called)++;
   return v;
}

static Eina_Value
_future_easy_null_success(void *data EINA_UNUSED, const Eina_Value v)
{
   //Should not happen
   fail_if(EINA_TRUE);
   return v;
}

static Eina_Value
_future_easy_null_err(void *data, Eina_Error err)
{
   int *cb_called = data;
   ck_assert_int_eq(err, EINVAL);
   (*cb_called)++;
   return eina_value_error_init(err);
}

static void
_future_easy_null_free(void *data, const Eina_Future *dead)
{
   int *cb_called = data;
   ck_assert_ptr_eq(dead, NULL);
   (*cb_called)++;
}

EFL_START_TEST(efl_test_future_null)
{
   Eina_Future *f;
   Log_Ctx ctx = { 0 };
   int cb_called = 0;
   int easy_cb_calls = 0;

   fail_if(!ecore_init());

   LOG_CTX_SET(ctx, "eina_future_then_from_desc", "safety check failed: (prev) == NULL");
   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);
   f = eina_future_then(NULL, _future_null_cb, &cb_called);
   ck_assert_ptr_eq(f, NULL);

   ck_assert_int_eq(cb_called, 1);

   cb_called = 0;
   LOG_CTX_SET(ctx, "eina_future_chain_array", "safety check failed: (prev) == NULL");
   f = eina_future_chain(NULL,
                         eina_future_cb_easy(_future_easy_null_success,
                                             _future_easy_null_err,
                                             _future_easy_null_free,
                                             NULL, &easy_cb_calls),
                         {_future_null_cb, &cb_called},
                         {_future_null_cb, &cb_called},
                         {_future_null_cb, &cb_called},
                         {_future_null_cb, &cb_called},
                         {_future_null_cb, &cb_called});
   ck_assert_ptr_eq(f, NULL);
   ck_assert_int_eq(cb_called, 5);
   ck_assert_int_eq(easy_cb_calls, 2);

   easy_cb_calls = 0;
   LOG_CTX_SET(ctx, "eina_future_chain_easy_array", "safety check failed: (prev) == NULL");
   f = eina_future_chain_easy(NULL,
                              {_future_easy_null_success,
                                   _future_easy_null_err,
                                   _future_easy_null_free,
                                   NULL, &easy_cb_calls});
   ck_assert_ptr_eq(f, NULL);
   ck_assert_int_eq(easy_cb_calls, 2);
   ecore_shutdown();
   eina_log_print_cb_set(NULL, NULL);
}
EFL_END_TEST

static Eina_Value
_future_race_all_null_cb(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   int err;
   int *cb_called = data;

   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_ERROR);
   fail_if(!eina_value_get(&v, &err));
   ck_assert_int_eq(err, ENOMEM);
   (*cb_called)++;
   return v;
}

EFL_START_TEST(efl_test_future_all_null)
{
   Log_Ctx ctx = { 0 };
   unsigned i, len;
   int cb_called = 0;
   Eina_Future *futures[11] = { 0 }, *f;

   fail_if(!ecore_init());

   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);

   len = EINA_C_ARRAY_LENGTH(futures);
   len--;
   for (i = 0; i < len; i++)
     {
        futures[i] = eina_future_then(_int_future_get(),
                                      _future_race_all_null_cb, &cb_called);
        fail_if(!futures[i]);
     }

   LOG_CTX_MULTIPLE_FUNC_CTX_SET(ctx,
                                 {"promise_proxy_of_future_array_create", "safety check failed: array[i] == NULL"},
                                 {"eina_promise_all_array", "safety check failed: r is false"});
   //The last future is NULL, which may cause the cancel.
   f = eina_future_all_array(futures);
   ck_assert_ptr_eq(f, NULL);
   ecore_shutdown();
   ck_assert_int_eq(cb_called, len);
   eina_log_print_cb_set(NULL, NULL);
}
EFL_END_TEST

EFL_START_TEST(efl_test_future_race_null)
{
   Log_Ctx ctx = { 0 };
   unsigned i, len;
   int cb_called = 0;
   Eina_Future *futures[11] = { 0 }, *f;

   fail_if(!ecore_init());

   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);

   len = EINA_C_ARRAY_LENGTH(futures);
   len--;
   for (i = 0; i < len; i++)
     {
        futures[i] = eina_future_then(_int_future_get(),
                                      _future_race_all_null_cb, &cb_called);
        fail_if(!futures[i]);
     }

   LOG_CTX_MULTIPLE_FUNC_CTX_SET(ctx,
                                 {"promise_proxy_of_future_array_create", "safety check failed: array[i] == NULL"},
                                 {"eina_promise_race_array", "safety check failed: r is false"});
   //The last future is NULL, which may cause the cancel.
   f = eina_future_race_array(futures);
   ck_assert_ptr_eq(f, NULL);
   ecore_shutdown();
   ck_assert_int_eq(cb_called, len);
   eina_log_print_cb_set(NULL, NULL);
}
EFL_END_TEST

#endif

const char *test_strings[] = { "another", "string" };

static Eina_Value
test_generate_int(Eo *o, void *data, const Eina_Value value)
{
   const Eina_Value empty = EINA_VALUE_EMPTY;

   ck_assert_int_eq(memcmp(&value, &empty, sizeof (Eina_Value)), 0);
   ck_assert_ptr_eq(data, test_strings[0]);
   ck_assert_ptr_eq(o, efl_main_loop_get());
   return eina_value_int_init(42);
}

static Eina_Value
test_got_int(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value value)
{
   int i = 0;
   ck_assert_ptr_eq(eina_value_type_get(&value), EINA_VALUE_TYPE_INT);
   eina_value_get(&value, &i);
   ck_assert_int_eq(i, 42);

   return eina_value_error_init(EAGAIN);
}

static Eina_Value
test_error_not_reached(Eo *o EINA_UNUSED, void *data EINA_UNUSED, Eina_Error error EINA_UNUSED)
{
   abort();

   return eina_value_error_init(EAGAIN);
}

static Eina_Value
test_not_reached(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value value)
{
   abort();

   return value;
}

static Eina_Value
test_failure(Eo *o, void *data EINA_UNUSED, Eina_Error error)
{
   ck_assert_int_eq(error, EAGAIN);
   ck_assert_ptr_eq(o, efl_main_loop_get());

   return eina_value_error_init(error);
}

static Eina_Value
test_end_future(Eo *o, void *data, Eina_Error error)
{
   ck_assert_ptr_eq(o, efl_main_loop_get());
   ck_assert_ptr_eq(data, test_strings[1]);
   ck_assert_int_eq(error, EAGAIN);

   return eina_value_error_init(error);
}

static void
test_cleanup(Eo *o, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   ck_assert_ptr_eq(o, efl_main_loop_get());
   ck_assert_ptr_eq(data, test_strings[1]);
   efl_loop_quit(o, eina_value_error_init(EAGAIN));
}

static void
efl_main_test(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Future *f;

   f = efl_future_chain(ev->object, efl_loop_idle(ev->object),
                        { .success = test_generate_int, .data = test_strings[0] },
                        { .success_type = EINA_VALUE_TYPE_INT, .success = test_got_int , .error = test_error_not_reached },
                        { .success = test_not_reached, .error = test_failure });
   efl_future_then(ev->object, f, .success = test_not_reached, .error = test_end_future, .data = test_strings[1], .free = test_cleanup);
}


EFL_START_TEST(efl_test_future_then)
{
   char *argv[] = { "Future" };
   Eina_Value *ret = NULL;
   Eina_Error err = 0;

   ecore_init();
   efl_event_callback_add(efl_main_loop_get(), EFL_LOOP_EVENT_ARGUMENTS, efl_main_test, NULL);
   ecore_init_ex(1, argv);
   ret = efl_loop_begin(efl_main_loop_get());
   ecore_shutdown_ex();

   ck_assert_ptr_eq(eina_value_type_get(ret), EINA_VALUE_TYPE_ERROR);
   eina_value_get(ret, &err);
   ck_assert_int_eq(err, EAGAIN);
   ecore_shutdown();
}
EFL_END_TEST

static void
promise_init(void)
{
   /* enable ecore init count manipulation for these tests */
   ecore_shutdown();
}

static void
promise_shutdown(void)
{
   /* enable ecore init count manipulation for these tests */
   _efl_startup_time = ecore_time_unix_get();
   _EFL_APP_VERSION_SET();
   ecore_init();
}

void efl_app_test_promise(TCase *tc)
{
   tcase_add_checked_fixture(tc, promise_init, promise_shutdown);
   tcase_add_test(tc, efl_test_promise_future_convert);
   tcase_add_test(tc, efl_test_promise_future_easy);
   tcase_add_test(tc, efl_test_promise_future_all);
   tcase_add_test(tc, efl_test_promise_future_race);
   tcase_add_test(tc, efl_test_promise_future_ignore_error);
   tcase_add_test(tc, efl_test_promise_future_success);
   tcase_add_test(tc, efl_test_promise_future_failure);
   tcase_add_test(tc, efl_test_future_then);
}

void efl_app_test_promise_2(TCase *tc)
{
   tcase_add_checked_fixture(tc, promise_init, promise_shutdown);
   tcase_add_test(tc, efl_test_promise_future_chain_no_error);
   tcase_add_test(tc, efl_test_promise_future_chain_error);
   tcase_add_test(tc, efl_test_promise_future_cancel);
   tcase_add_test(tc, efl_test_promise_future_implicit_cancel);
   tcase_add_test(tc, efl_test_promise_future_inner_promise);
   tcase_add_test(tc, efl_test_promise_future_inner_promise_fail);
}

void efl_app_test_promise_3(TCase *tc)
{
   tcase_add_checked_fixture(tc, promise_init, promise_shutdown);
   tcase_add_test(tc, efl_test_timeout);
   tcase_add_test(tc, efl_test_job);
   tcase_add_test(tc, efl_test_idle);
   tcase_add_test(tc, efl_test_promise_log);
   //FIXME: We should move this to EO tests, however they depend on Ecore...
   tcase_add_test(tc, efl_test_promise_eo);
   tcase_add_test(tc, efl_test_promise_eo_link);
}

void efl_app_test_promise_safety(TCase *tc)
{
#ifdef EINA_SAFETY_CHECKS
   tcase_add_checked_fixture(tc, promise_init, promise_shutdown);
   tcase_add_test(tc, efl_test_promise_null);
   tcase_add_test(tc, efl_test_promise_reject_resolve_null);
   tcase_add_test(tc, efl_test_future_null);
   tcase_add_test(tc, efl_test_future_all_null);
   tcase_add_test(tc, efl_test_future_race_null);
#endif
}
