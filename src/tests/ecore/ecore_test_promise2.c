#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Eina.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <Eo.h>
#include "ecore_suite.h"

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
   Ecore_Timer *t;
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

static void
_cancel(void *data, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   PromiseCtx *ctx = data;
   if (ctx->t) ecore_timer_del(ctx->t);
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

static Eina_Bool
_simple_timeout(void *data)
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
   free(ctx);
   return EINA_FALSE;
}

static Eina_Future_Scheduler *
_future_scheduler_get(void)
{
   return efl_loop_future_scheduler_get(ecore_main_loop_get());
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
   ctx->t = ecore_timer_add(timeout, _simple_timeout, ctx);
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
      _cancel_cb must receive a EINA_VALYE_TYPE_ERROR as ECANCELED */
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

START_TEST(efl_test_promise_future_success)
{
   Eina_Future *f;
   fail_if(!ecore_init());
   f = eina_future_then(_str_future_get(),
                        _simple_ok, NULL);
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_failure)
{
   Eina_Future *f;
   fail_if(!ecore_init());
   f = eina_future_then(_fail_future_get(),
                        _simple_err, NULL);
   fail_if(!f);
   ecore_main_loop_begin();
   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_chain_no_error)
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
END_TEST

START_TEST(efl_test_promise_future_chain_error)
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
END_TEST

START_TEST(efl_test_promise_future_cancel)
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
END_TEST

START_TEST(efl_test_promise_future_inner_promise)
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
END_TEST

START_TEST(efl_test_promise_future_inner_promise_fail)
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
END_TEST

START_TEST(efl_test_promise_future_implicit_cancel)
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
   ecore_shutdown();
   //All the futures were cancelled at this point
   fail_if(cancel_count != CHAIN_SIZE);
   //Cancel should not be called, since we called eina_promise_resolve()
   fail_if(cancel_called);
}
END_TEST

START_TEST(efl_test_promise_future_convert)
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
END_TEST

START_TEST(efl_test_promise_future_easy)
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
END_TEST

START_TEST(efl_test_promise_future_all)
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

   futures[--len] = NULL;
   fail_if(!eina_future_then(eina_future_all_array(futures), _all_cb, &len));
   ecore_main_loop_begin();
   ecore_shutdown();
   fail_if(futures_called != len);
}
END_TEST

START_TEST(efl_test_promise_future_race)
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

   futures[--len] = NULL;
   fail_if(!eina_future_then(eina_future_race_array(futures),
                             _race_end_cb, &ctx));
   ecore_main_loop_begin();
   eina_value_flush(&ctx.value);
   ecore_shutdown();
   fail_if(ctx.success != 1);
   fail_if(ctx.failed != (len - 1));
}
END_TEST

static Eina_Value
_eo_future1_ok(Eo *eo EINA_UNUSED, const Eina_Value v)
{
   const char *number;

   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
   fail_if(!eina_value_get(&v, &number));
   ck_assert_str_eq(DEFAULT_INT_VALUE_AS_STRING, number);
   return v;
}

static Eina_Value
_eo_future1_err(Eo *eo EINA_UNUSED, Eina_Error err EINA_UNUSED)
{
   //Should not happen
   fail_if(EINA_TRUE);
}

static Eina_Value
_eo_future2_ok(Eo *eo EINA_UNUSED, const Eina_Value v)
{
   //Should not happen
   fail_if(EINA_TRUE);
   return v;
}

static Eina_Value
_eo_future2_err(Eo *eo EINA_UNUSED, Eina_Error err)
{
   Eina_Value v;

   fail_if(err != EINVAL);
   fail_if(!eina_value_setup(&v, EINA_VALUE_TYPE_INT));
   fail_if(!eina_value_set(&v, DEFAULT_INT_VALUE));
   return v;
}

static void
_eo_future_free(Eo *eo, const Eina_Future *dead EINA_UNUSED)
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

START_TEST(efl_test_promise_eo)
{
   Eina_Future *f;
   Eo *obj;
   int free_called = 0;

   fail_if(!efl_object_init());
   fail_if(!ecore_init());

   //Use a random object..
   obj = efl_add(EFL_IO_BUFFER_CLASS, NULL);
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
END_TEST

static Eina_Value
_eo_future_link_success(Eo *eo EINA_UNUSED, const Eina_Value v)
{
   //This should never happen
   fail_if(EINA_TRUE);
   return v;
}

static Eina_Value
_eo_future_link_err(Eo *eo, Eina_Error err)
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

START_TEST(efl_test_promise_eo_link)
{
   Eina_Future *f;
   Eo *obj;
   int err_called = 0;

   fail_if(!efl_object_init());
   fail_if(!ecore_init());

   //Use a random object..
   obj = efl_add(EFL_IO_BUFFER_CLASS, NULL);
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
END_TEST

void ecore_test_ecore_promise2(TCase *tc)
{
   tcase_add_test(tc, efl_test_promise_future_success);
   tcase_add_test(tc, efl_test_promise_future_failure);
   tcase_add_test(tc, efl_test_promise_future_chain_no_error);
   tcase_add_test(tc, efl_test_promise_future_chain_error);
   tcase_add_test(tc, efl_test_promise_future_cancel);
   tcase_add_test(tc, efl_test_promise_future_implicit_cancel);
   tcase_add_test(tc, efl_test_promise_future_inner_promise);
   tcase_add_test(tc, efl_test_promise_future_inner_promise_fail);
   tcase_add_test(tc, efl_test_promise_future_convert);
   tcase_add_test(tc, efl_test_promise_future_easy);
   tcase_add_test(tc, efl_test_promise_future_all);
   tcase_add_test(tc, efl_test_promise_future_race);
   //FIXME: We should move this to EO tests, however they depend on Ecore...
   tcase_add_test(tc, efl_test_promise_eo);
   tcase_add_test(tc, efl_test_promise_eo_link);
}
