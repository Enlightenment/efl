#include <Efl_Core.h>
#include <stdlib.h>
#include <errno.h>

typedef struct _Ctx {
   Eina_Promise *p;
   Eina_Bool should_fail;
   Eina_Future *timer;
   Eina_Value *value;
} Ctx;

typedef struct _Inner_Promise_Ctx {
   Eina_Future *future;
   Eina_Promise *promise;
} Inner_Promise_Ctx;

#define DEFAULT_MSG "the simple example is working!"

#define VALUE_TYPE_CHECK(_v, _type)                                     \
  if (_v.type != _type)                                                 \
    {                                                                   \
       fprintf(stderr, "Value type is not '%s' - received '%s'\n",      \
               _type->name, _v.type->name);                             \
       return _v;                                                       \
    }

static Eina_Value
_timeout(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Ctx *ctx = data;
   if (v.type == EINA_VALUE_TYPE_ERROR) goto end;
   if (ctx->should_fail) eina_promise_reject(ctx->p, ENETDOWN);
   else
     {
        Eina_Value tmp;
        eina_value_copy(ctx->value, &tmp);
        eina_promise_resolve(ctx->p, tmp);
     }
 end:
   if (ctx->value) eina_value_free(ctx->value);
   ctx->timer = NULL;
   free(ctx);
   return v;
}

static void
_promise_cancel(void *data, const Eina_Promise *dead EINA_UNUSED)
{
   Ctx *ctx = data;
   if (ctx->timer) eina_future_cancel(ctx->timer);
}

static Ctx *
_promise_ctx_new(Efl_Loop *loop, Eina_Value *v)
{
   Ctx *ctx;
   ctx = calloc(1, sizeof(Ctx));
   EINA_SAFETY_ON_NULL_GOTO(ctx, err_ctx);
   ctx->p = efl_loop_promise_new(loop, _promise_cancel, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->p, err_timer);
   ctx->value = v;
   return ctx;
 err_timer:
   free(ctx);
 err_ctx:
   eina_value_free(v);
   return NULL;
}

static Eina_Future *
_future_get(Ctx *ctx, Efl_Loop *loop)
{
   Eina_Future *f;

   f = eina_future_new(ctx->p);
   EINA_SAFETY_ON_NULL_GOTO(f, err_future);
   ctx->timer = eina_future_then(efl_loop_timeout(loop, 0.1), _timeout, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->timer, err_timer);
   return f;

 err_timer:
   eina_future_cancel(f);
 err_future:
   return NULL;
}

static Eina_Future *
_fail_future_get(Efl_Loop *loop)
{
   Ctx *ctx = _promise_ctx_new(loop, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   ctx->should_fail = EINA_TRUE;
   return _future_get(ctx, loop);
}

static Eina_Future *
_str_future_get(Efl_Loop *loop)
{
   Eina_Value *v = eina_value_util_string_new(DEFAULT_MSG);
   EINA_SAFETY_ON_NULL_RETURN_VAL(v, NULL);
   Ctx *ctx = _promise_ctx_new(loop, v);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return _future_get(ctx, loop);
}

static Eina_Future *
_int_future_get(Efl_Loop *loop, int i)
{
   Eina_Value *v = eina_value_util_int_new(i);
   EINA_SAFETY_ON_NULL_RETURN_VAL(v, NULL);
   Ctx *ctx = _promise_ctx_new(loop, v);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   return _future_get(ctx, loop);
}

static Eina_Value
_simple_ok(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
   return v;
}

static Eina_Value
_alternate_error_cb(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Bool *should_fail = data;
   Eina_Value new_v = EINA_VALUE_EMPTY;

   if (*should_fail)
     {
        *should_fail = EINA_FALSE;
        eina_value_setup(&new_v, EINA_VALUE_TYPE_ERROR);
        eina_value_set(&new_v, ENETDOWN);
        printf("Received succes from the previous future - Generating error for the next future...\n");
     }
   else
     {
        *should_fail = EINA_TRUE;
        Eina_Error err;
        VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_ERROR);
        eina_value_get(&v, &err);
        printf("Received error from the previous future - value: %s. Send success\n",
               eina_error_msg_get(err));
     }
   return new_v;
}

static void
_alternate_error(Efl_Loop *loop)
{
   static Eina_Bool should_fail = EINA_TRUE;

   eina_future_chain(_str_future_get(loop),
                     {.cb = _alternate_error_cb, .data = &should_fail},
                     {.cb = _alternate_error_cb, .data = &should_fail},
                     {.cb = _alternate_error_cb, .data = &should_fail},
                     {.cb = _alternate_error_cb, .data = &should_fail});

}

static Eina_Value
_simple_err(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_ERROR);
   return v;
}

static void
_simple(Efl_Loop *loop)
{
   eina_future_chain(_str_future_get(loop),
                     eina_future_cb_console("Expecting the following message: "DEFAULT_MSG ". Got: ", NULL),
                     { .cb = _simple_ok, .data = NULL });
   eina_future_chain(_fail_future_get(loop),
                     eina_future_cb_console("Expectig network down error. Got: ", NULL),
                     { .cb = _simple_err, .data = NULL });
}

static Eina_Value
_chain_no_errors_cb(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   int count;
   Eina_Value new_v;

   eina_value_setup(&new_v, EINA_VALUE_TYPE_INT);
   if (!v.type)
     count = 1;
   else
     {
        VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_INT);
        eina_value_get(&v, &count);
     }
   eina_value_set(&new_v, count * 2);
   return new_v;
}

static void
_chain_no_errors(Efl_Loop *loop)
{
   eina_future_chain(_int_future_get(loop, 0),
                     eina_future_cb_console("Expecting no value. Got: ", NULL),
                     {.cb = _chain_no_errors_cb, .data = NULL},
                     eina_future_cb_console("Expecting number 2. Got: ", NULL),
                     {.cb = _chain_no_errors_cb, .data = NULL},
                     eina_future_cb_console("Expecting number 4. Got: ", NULL),
                     {.cb = _chain_no_errors_cb, .data = NULL},
                     eina_future_cb_console("Expecting number 8. Got: ", NULL),
                     {.cb = _chain_no_errors_cb, .data = NULL},
                     eina_future_cb_console("Expecting number 16. Got: ", NULL));
}

static Eina_Value
_chain_with_error_cb(void *data EINA_UNUSED, const Eina_Value v EINA_UNUSED, const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Value err;
   eina_value_setup(&err, EINA_VALUE_TYPE_ERROR);
   eina_value_set(&err, E2BIG);
   return err;
}

static void
_chain_with_error(Efl_Loop *loop)
{
   eina_future_chain(_int_future_get(loop, 0),
                     { _chain_with_error_cb, NULL },
                     eina_future_cb_console("Expecting argument list too long. Got: ", NULL),
                     { .cb = _simple_err, .data = NULL });
}

static Eina_Value
_delayed_resolve(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Inner_Promise_Ctx *ctx = data;
   Eina_Value new_v;
   eina_value_setup(&new_v, EINA_VALUE_TYPE_STRING);
   eina_value_set(&new_v, "Hello from inner future");
   eina_promise_resolve(ctx->promise, new_v);
   free(ctx);
   return v;
}

static Eina_Value
_delayed_reject(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   Inner_Promise_Ctx *ctx = data;
   eina_promise_reject(ctx->promise, ENETDOWN);
   free(ctx);
   return v;
}

static void
_inner_promise_cancel(void *data, const Eina_Promise *dead EINA_UNUSED)
{
   Inner_Promise_Ctx *ctx = data;
   eina_future_cancel(ctx->future);
   free(ctx);
}

static Eina_Value
_chain_inner_cb(void *data, const Eina_Value v, const Eina_Future *dead_future)
{
   Inner_Promise_Ctx *ctx;
   Eina_Value r;
   int s = 0;

   eina_value_int_get(&v, &s);

   ctx = calloc(1, sizeof(Inner_Promise_Ctx));
   EINA_SAFETY_ON_NULL_GOTO(ctx, err);
   ctx->promise = eina_promise_continue_new(dead_future, _inner_promise_cancel, ctx);
   EINA_SAFETY_ON_NULL_GOTO(ctx->promise, err);

   printf("Creating a new promise inside the future cb\n");
   ctx->future = eina_future_then(_int_future_get(data, 0),
                                  !s ? _delayed_resolve : _delayed_reject,
                                  ctx);
   return eina_promise_as_value(ctx->promise);

 err:
   eina_value_setup(&r, EINA_VALUE_TYPE_ERROR);
   eina_value_set(&r, ENOMEM);
   free(ctx);
   return r;
}

static Eina_Value
_chain_inner_last_cb(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_STRING);
   return v;
}

static void
_chain_inner_no_errors(Efl_Loop *loop)
{
   eina_future_chain(_int_future_get(loop, 0),
                     { .cb = _chain_inner_cb, .data = loop },
                     eina_future_cb_console("Expecting message: 'Hello from inner future'. Got: ", NULL),
                     { .cb = _chain_inner_last_cb, .data = NULL });
}

static Eina_Value
_err_inner_chain(void *data, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_ERROR);
   efl_loop_quit(data, EINA_VALUE_EMPTY);
   return v;
}

static void
_chain_inner_errors(Efl_Loop *loop)
{

   eina_future_chain(_int_future_get(loop, 1),
                     { .cb = _chain_inner_cb, .data = loop },
                     eina_future_cb_console("Expection network down error. Got: ", NULL),
                     { .cb = _err_inner_chain, .data = loop });
}

static Eina_Value
_canceled_cb(void *data EINA_UNUSED, const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   VALUE_TYPE_CHECK(v, EINA_VALUE_TYPE_ERROR);
   return v;
}

static void
_future_cancel(Efl_Loop *loop)
{
   Eina_Future *f;

   f = eina_future_chain(_int_future_get(loop, 0),
                         eina_future_cb_console("Expecting cancelled operation error.  Got: ", NULL),
                         { .cb = _canceled_cb, .data = NULL },
                         eina_future_cb_console("Expecting cancelled operation error.  Got: ", NULL),
                         { .cb = _canceled_cb, .data = NULL },
                         eina_future_cb_console("Expecting cancelled operation error.  Got: ", NULL),
                         { .cb = _canceled_cb, .data = NULL },
                         eina_future_cb_console("Expecting cancelled operation error.  Got: ", NULL),
                         { .cb = _canceled_cb, .data = NULL },
                         eina_future_cb_console("Expecting cancelled operation error.  Got: ", NULL),
                         { .cb = _canceled_cb, .data = NULL },
                         eina_future_cb_console("Expecting cancelled operation error.  Got: ", NULL));
   eina_future_cancel(f);
}

EAPI_MAIN void
efl_main(void *data EINA_UNUSED,
         const Efl_Event *ev)
{
   _simple(ev->object);
   _alternate_error(ev->object);
   _chain_no_errors(ev->object);
   _chain_with_error(ev->object);
   _chain_inner_no_errors(ev->object);
   _chain_inner_errors(ev->object);
   _future_cancel(ev->object);
}

EFL_MAIN();
