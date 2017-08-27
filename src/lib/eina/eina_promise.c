#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_private.h"
#include "eina_promise.h"
#include "eina_mempool.h"
#include "eina_promise_private.h"
#include <errno.h>
#include <stdarg.h>
#include <assert.h>

#define EINA_FUTURE_DISPATCHED ((Eina_Future_Cb)(0x01))

#define EFL_MEMPOOL_CHECK_RETURN(_type, _mp, _p)                        \
  if (!eina_mempool_from((_mp), (_p)))                                  \
    {                                                                   \
       ERR("The %s %p is not alive at mempool %p", (_type), (_p), (_mp)); \
       return;                                                          \
    }

#define EFL_MEMPOOL_CHECK_RETURN_VAL(_type, _mp, _p, _val)              \
  if (!eina_mempool_from((_mp), (_p)))                                  \
    {                                                                   \
       ERR("The %s %p is not alive at mempool %p", (_type),(_p), (_mp)); \
       return (_val);                                                   \
    }

#define EFL_MEMPOOL_CHECK_GOTO(_type, _mp, _p, _goto)                   \
  if (!eina_mempool_from((_mp), (_p)))                                  \
    {                                                                   \
       ERR("The %s %p is not alive at mempool %p", (_type), (_p), (_mp)); \
       goto _goto;                                                      \
    }

#define EINA_PROMISE_CHECK_RETURN(_p)                           \
  do {                                                          \
     EINA_SAFETY_ON_NULL_RETURN((_p));                          \
     EFL_MEMPOOL_CHECK_RETURN("promise", _promise_mp, (_p));    \
  } while (0);

#define EINA_PROMISE_CHECK_RETURN_VAL(_p, _val)                         \
  do {                                                                  \
     EINA_SAFETY_ON_NULL_RETURN_VAL((_p), (_val));                      \
     EFL_MEMPOOL_CHECK_RETURN_VAL("promise", _promise_mp, (_p), (_val)); \
  } while (0);

#define EINA_PROMISE_CHECK_GOTO(_p, _goto)                              \
  do {                                                                  \
     EINA_SAFETY_ON_NULL_GOTO((_p), _goto);                             \
     EFL_MEMPOOL_CHECK_GOTO("promise", _promise_mp, (_p), _goto);       \
  } while (0);

#define EINA_FUTURE_CHECK_GOTO(_p, _goto)                               \
  do {                                                                  \
     EINA_SAFETY_ON_NULL_GOTO((_p), _goto);                             \
     EFL_MEMPOOL_CHECK_GOTO("future", _future_mp, (_p), _goto);         \
  } while (0);

#define EINA_FUTURE_CHECK_RETURN(_p)                            \
  do {                                                          \
     EINA_SAFETY_ON_NULL_RETURN((_p));                          \
     EFL_MEMPOOL_CHECK_RETURN("future", _future_mp, (_p));      \
     if (_p->cb == EINA_FUTURE_DISPATCHED)                      \
       {                                                        \
          ERR("Future %p already dispatched", _p);              \
          return;                                               \
       }                                                        \
  } while (0);

#define EINA_FUTURE_CHECK_RETURN_VAL(_p, _val)                         \
  do {                                                                 \
     EINA_SAFETY_ON_NULL_RETURN_VAL((_p), (_val));                     \
     EFL_MEMPOOL_CHECK_RETURN_VAL("future", _future_mp, (_p), (_val)); \
     if (_p->cb == EINA_FUTURE_DISPATCHED)                             \
       {                                                               \
          ERR("Future %p already dispatched", _p);                     \
          return (_val);                                               \
       }                                                               \
  } while (0);

#undef ERR
#define ERR(...) EINA_LOG_DOM_ERR(_promise2_log_dom, __VA_ARGS__)

#undef DBG
#define DBG(...) EINA_LOG_DOM_DBG(_promise2_log_dom, __VA_ARGS__)

#undef INF
#define INF(...) EINA_LOG_DOM_INFO(_promise2_log_dom, __VA_ARGS__)

#undef WRN
#define WRN(...) EINA_LOG_DOM_WARN(_promise2_log_dom, __VA_ARGS__)

#undef CRI
#define CRI(...) EINA_LOG_DOM_CRIT(_promise2_log_dom, __VA_ARGS__)

#define _eina_promise_value_dbg(_msg, _p, _v) __eina_promise_value_dbg(_msg, _p, _v, __LINE__, __FUNCTION__)

struct _Eina_Promise {
   Eina_Future *future;
   Eina_Future_Scheduler *scheduler;
   Eina_Promise_Cancel_Cb cancel;
   const void *data;
};

struct _Eina_Future {
   Eina_Promise *promise;
   Eina_Future *next;
   Eina_Future *prev;
   Eina_Future_Cb cb;
   const void *data;
   Eina_Future **storage;
   Eina_Future_Schedule_Entry *scheduled_entry;
};

static Eina_Mempool *_promise_mp = NULL;
static Eina_Mempool *_future_mp = NULL;
static Eina_List *_pending_futures = NULL;
static int _promise2_log_dom = -1;

static void _eina_promise_cancel(Eina_Promise *p);

typedef struct _Race_Result {
   Eina_Value value;
   unsigned int index;
} Race_Result;

static Eina_Value_Struct_Member RACE_STRUCT_MEMBERS[] = {
  EINA_VALUE_STRUCT_MEMBER(NULL, Race_Result, value),
  EINA_VALUE_STRUCT_MEMBER(NULL, Race_Result, index),
  EINA_VALUE_STRUCT_MEMBER_SENTINEL
};

static const Eina_Value_Struct_Desc RACE_STRUCT_DESC = {
  .version = EINA_VALUE_STRUCT_DESC_VERSION,
  .ops = NULL,
  .members = RACE_STRUCT_MEMBERS,
  .member_count = 2,
  .size = sizeof(Race_Result)
};

EAPI const Eina_Value_Struct_Desc *EINA_PROMISE_RACE_STRUCT_DESC = &RACE_STRUCT_DESC;

static inline void
__eina_promise_value_dbg(const char *msg,
                         const Eina_Promise *p,
                         const Eina_Value v,
                         int line,
                         const char *fname)
{
   if (EINA_UNLIKELY(eina_log_domain_level_check(_promise2_log_dom,
                                                 EINA_LOG_LEVEL_DBG)))
     {
        if (!v.type)
          {
             eina_log_print(_promise2_log_dom, EINA_LOG_LEVEL_DBG,
                            __FILE__, fname, line, "%s: %p with no value",
                            msg, p);
          }
        else
          {
             char *str = eina_value_to_string(&v);
             eina_log_print(_promise2_log_dom, EINA_LOG_LEVEL_DBG,
                            __FILE__, fname, line,
                            "%s: %p - Value Type: %s Content: %s", msg, p,
                            eina_value_type_name_get(eina_value_type_get(&v)),
                            str);
             free(str);
          }
     }
}

static Eina_Bool
_promise_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eina_Promise **tmem = mem;
   *tmem = NULL;
   return EINA_TRUE;
}

static Eina_Bool
_promise_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eina_Promise **tmem = mem;
   if (*tmem)
     {
        _eina_promise_cancel(*tmem);
        *tmem = NULL;
     }
   return EINA_TRUE;
}

static void
_promise_replace(Eina_Promise **dst, Eina_Promise * const *src)
{
   if (*src == *dst) return;
   if (*dst) _eina_promise_cancel(*dst);
   *dst = *src;
}

static Eina_Bool
_promise_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   Eina_Promise **dst = mem;
   Eina_Promise **src = va_arg(args, Eina_Promise **);
   _promise_replace(dst, src);
   return EINA_TRUE;
}

static Eina_Bool
_promise_pset(const Eina_Value_Type *type EINA_UNUSED,
              void *mem, const void *ptr)
{
   Eina_Promise **dst = mem;
   Eina_Promise * const *src = ptr;
   _promise_replace(dst, src);
   return EINA_TRUE;
}

static Eina_Bool
_promise_pget(const Eina_Value_Type *type EINA_UNUSED,
              const void *mem, void *ptr)
{
   Eina_Promise * const *src = mem;
   Eina_Promise **dst = ptr;
   *dst = *src;
   return EINA_TRUE;
}

static Eina_Bool
_promise_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   Eina_Promise * const *p = type_mem;

   if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
       convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[128];
        snprintf(buf, sizeof(buf), "Promise %p (cancel: %p, data: %p)",
                 *p, (*p)->cancel, (*p)->data);
        other_mem = buf;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   return EINA_FALSE;
}

static const Eina_Value_Type EINA_VALUE_TYPE_PROMISE2 = {
  .version = EINA_VALUE_TYPE_VERSION,
  .value_size = sizeof(Eina_Promise *),
  .name = "Eina_Promise",
  .setup = _promise_setup,
  .flush = _promise_flush,
  .copy = NULL,
  .compare = NULL,
  .convert_to = _promise_convert_to,
  .convert_from = NULL,
  .vset = _promise_vset,
  .pset = _promise_pset,
  .pget = _promise_pget
};

static Eina_Promise *
_eina_value_promise2_steal(Eina_Value *value)
{
   Eina_Promise **p, *r;
   /*
     Do not use eina_value_flush()/eina_value_pset() in here,
     otherwise it would cancel the promise!
   */
   p = eina_value_memory_get(value);
   r = *p;
   *p = NULL;
   return r;
}

static Eina_Future *
_eina_future_free(Eina_Future *f)
{
   DBG("Free future %p", f);
   Eina_Future *next = f->next;
   Eina_Future *prev = f->prev;
   if (next) next->prev = NULL;
   if (prev) prev->next = NULL;
   eina_mempool_free(_future_mp, f);
   return next;
}

static void
_eina_promise_unlink(Eina_Promise *p)
{
   if (p->future)
     {
        DBG("Unliking promise %p and future %p", p, p->future);
        p->future->promise = NULL;
        p->future = NULL;
     }
}

static void
_eina_promise_link(Eina_Promise *p, Eina_Future *f)
{
   assert(f != NULL);
   if (p) p->future = f;
   f->promise = p;
   DBG("Linking future %p with promise %p", f, p);
}

static void
_eina_promise_cancel(Eina_Promise *p)
{
   DBG("Cancelling promise: %p, data: %p, future: %p", p, p->data, p->future);
   _eina_promise_unlink(p);
   p->cancel((void *)p->data, p);
   eina_mempool_free(_promise_mp, p);
}

static void
_eina_value_safe_flush(Eina_Value v)
{
   //FIXME: Should we move this check to eina_value?
   if (v.type) eina_value_flush(&v);
}

static void
_eina_promise_value_steal_and_link(Eina_Value value, Eina_Future *f)
{
   Eina_Promise *p = _eina_value_promise2_steal(&value);
   DBG("Promise %p stolen from value", p);
   eina_value_flush(&value);
   if (f) _eina_promise_link(p, f);
   else _eina_promise_unlink(p);
}

static Eina_Value
_eina_future_cb_dispatch(Eina_Future *f, const Eina_Value value)
{
   Eina_Future_Cb cb = f->cb;

   f->cb = EINA_FUTURE_DISPATCHED;
   if (f->storage) *f->storage = NULL;

   if (EINA_UNLIKELY(eina_log_domain_level_check(_promise2_log_dom, EINA_LOG_LEVEL_DBG)))
     {
        if (!value.type) DBG("Distach cb: %p, data: %p with no value", cb, f->data);
        else
          {
             char *str = eina_value_to_string(&value);
             DBG("Dispatch cb: %p, data: %p, value type: %s content: '%s'",
                 cb, f->data,
                 eina_value_type_name_get(eina_value_type_get(&value)), str);
             free(str);
          }
     }
   return cb((void *)f->data, value, f);
}

static Eina_Value
_eina_future_dispatch_internal(Eina_Future **f,
                               const Eina_Value value)
{
   Eina_Value next_value = EINA_VALUE_EMPTY;

   assert(value.type != &EINA_VALUE_TYPE_PROMISE2);
   while ((*f) && (!(*f)->cb)) *f = _eina_future_free(*f);
   if (!*f)
     {
        _eina_promise_value_dbg("No future to deliver value", NULL, value);
        return value;
     }
   next_value = _eina_future_cb_dispatch(*f, value);
   *f = _eina_future_free(*f);
   return next_value;
}

static Eina_Bool
_eina_value_is(const Eina_Value v1, const Eina_Value v2)
{
   if (v1.type != v2.type) return EINA_FALSE;
   //Both types are NULL at this point... so they are equal...
   if (!v1.type) return EINA_TRUE;
   return !memcmp(eina_value_memory_get(&v1),
                  eina_value_memory_get(&v2),
                  v1.type->value_size);
}

static void
_eina_future_dispatch(Eina_Future *f, Eina_Value value)
{
    Eina_Value next_value = _eina_future_dispatch_internal(&f, value);
    if (!_eina_value_is(next_value, value)) _eina_value_safe_flush(value);
    if (!f)
      {
         if (next_value.type == &EINA_VALUE_TYPE_PROMISE2)
           {
              DBG("There are no more futures, but next_value is a promise setting p->future to NULL.");
              _eina_promise_value_steal_and_link(next_value, NULL);
           }
         else _eina_value_safe_flush(next_value);
         return;
      }

    if (next_value.type == &EINA_VALUE_TYPE_PROMISE2)
      {
         if (EINA_UNLIKELY(eina_log_domain_level_check(_promise2_log_dom, EINA_LOG_LEVEL_DBG)))
           {
              Eina_Promise *p = NULL;

              eina_value_pget(&next_value, &p);
              DBG("Future %p will wait for a new promise %p", f, p);
           }
         _eina_promise_value_steal_and_link(next_value, f);
      }
    else _eina_future_dispatch(f, next_value);
 }

static void
_scheduled_entry_cb(Eina_Future *f, Eina_Value value)
{
   _pending_futures = eina_list_remove(_pending_futures, f);
   f->scheduled_entry = NULL;
   _eina_future_dispatch(f, value);
}

void
eina_future_schedule_entry_recall(Eina_Future_Schedule_Entry *entry)
{
   entry->scheduler->recall(entry);
}

static void
_eina_future_cancel(Eina_Future *f, int err)
{
   Eina_Value value = EINA_VALUE_EMPTY;

   DBG("Cancelling future %p, cb: %p data: %p with error: %d - msg: '%s'",
       f, f->cb, f->data, err, eina_error_msg_get(err));

   for (; f->prev != NULL; f = f->prev)
     {
        assert(f->promise == NULL); /* intermediate futures shouldn't have a promise */
        assert(f->scheduled_entry == NULL); /* intermediate futures shouldn't have pending dispatch */
     }

   if (f->scheduled_entry)
     {
        eina_future_schedule_entry_recall(f->scheduled_entry);
        f->scheduled_entry = NULL;
        _pending_futures = eina_list_remove(_pending_futures, f);
     }

   if (f->promise)
     {
        _eina_promise_cancel(f->promise);
        f->promise = NULL;
     }

   eina_value_setup(&value, EINA_VALUE_TYPE_ERROR);
   eina_value_set(&value, err);

   while (f)
     {
        if (f->cb)
          {
             Eina_Value r = _eina_future_cb_dispatch(f, value);
             if (!_eina_value_is(value, r)) _eina_value_safe_flush(r);
          }
        f = _eina_future_free(f);
     }
   eina_value_flush(&value);
}

static void
_eina_future_schedule(Eina_Promise *p,
                      Eina_Future *f,
                      Eina_Value value)
{
   f->scheduled_entry = p->scheduler->schedule(p->scheduler,
                                               _scheduled_entry_cb,
                                               f, value);
   EINA_SAFETY_ON_NULL_GOTO(f->scheduled_entry, err);
   assert(f->scheduled_entry->scheduler != NULL);
   _pending_futures = eina_list_append(_pending_futures, f);
   DBG("The promise %p schedule the future %p with cb: %p and data: %p",
       p, f, f->cb, f->data);
   return;
 err:
   _eina_future_cancel(p->future, ENOMEM);
   _eina_value_safe_flush(value);
}

static void
_eina_promise_deliver(Eina_Promise *p,
                      Eina_Value value)
{
   if (p->future)
     {
        Eina_Future *f = p->future;
        _eina_promise_unlink(p);
        if (value.type == &EINA_VALUE_TYPE_PROMISE2) _eina_promise_value_steal_and_link(value, f);
        else _eina_future_schedule(p, f, value);
     }
   else
     {
        DBG("Promise %p has no future", p);
        _eina_value_safe_flush(value);
     }
   eina_mempool_free(_promise_mp, p);
}

Eina_Bool
eina_promise_init(void)
{
   const char *choice = getenv("EINA_MEMPOOL");
   if ((!choice) || (!choice[0])) choice = "chained_mempool";

   RACE_STRUCT_MEMBERS[0].type = EINA_VALUE_TYPE_VALUE;
   RACE_STRUCT_MEMBERS[1].type = EINA_VALUE_TYPE_UINT;

   _promise2_log_dom = eina_log_domain_register("eina_promise", EINA_COLOR_CYAN);
   if (_promise2_log_dom < 0)
     {
        EINA_LOG_ERR("Could not creae the Eina_Promise domain");
        return EINA_FALSE;
     }

   //FIXME: Is 512 too high?
   _promise_mp = eina_mempool_add(choice, "Eina_Promise",
                                  NULL, sizeof(Eina_Promise), 512);
   EINA_SAFETY_ON_NULL_GOTO(_promise_mp, err_promise);

   _future_mp = eina_mempool_add(choice, "Eina_Future",
                                 NULL, sizeof(Eina_Future), 512);
   EINA_SAFETY_ON_NULL_GOTO(_future_mp, err_future);

   return EINA_TRUE;

 err_future:
   eina_mempool_del(_promise_mp);
   _promise_mp = NULL;
 err_promise:
   eina_log_domain_unregister(_promise2_log_dom);
   _promise2_log_dom = -1;
   return EINA_FALSE;
}

Eina_Bool
eina_promise_shutdown(void)
{
   while (_pending_futures) _eina_future_cancel(_pending_futures->data, ECANCELED);
   eina_mempool_del(_future_mp);
   eina_mempool_del(_promise_mp);
   eina_log_domain_unregister(_promise2_log_dom);
   _promise2_log_dom = -1;
   _promise_mp = NULL;
   _future_mp = NULL;
   return EINA_TRUE;
}

EAPI Eina_Value
eina_promise_as_value(Eina_Promise *p)
{
   Eina_Value v = EINA_VALUE_EMPTY;
   Eina_Bool r;
   EINA_PROMISE_CHECK_RETURN_VAL(p, v);
   r = eina_value_setup(&v, &EINA_VALUE_TYPE_PROMISE2);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_setup);
   r = eina_value_pset(&v, &p);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_pset);
   DBG("Created value from promise %p", p);
   return v;

 err_pset:
   eina_value_flush(&v);
   memset(&v, 0, sizeof(Eina_Value));
 err_setup:
   if (p->future) _eina_future_cancel(p->future, ENOMEM);
   else _eina_promise_cancel(p);
   return v;
}

static void
_eina_promise_clean_dispatch(Eina_Promise *p, Eina_Value v)
{
   Eina_Future *f = p->future;

   if (f)
     {
        _eina_promise_value_dbg("Clean contenxt - Resolving promise", p, v);
        _eina_promise_unlink(p);
        _eina_future_dispatch(f, v);
     }
   eina_mempool_free(_promise_mp, p);
}

static Eina_Value
_future_proxy(void *data, const Eina_Value v,
              const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Value copy;
   //We're in a safe context (from mainloop), so we can avoid scheduling a new dispatch
   if (!v.type) copy = v;
   else if (!eina_value_copy(&v, &copy))
     {
        ERR("Value cannot be copied - unusable with Eina_Future: %p (%s)", v.type, v.type->name);
        eina_value_setup(&copy, EINA_VALUE_TYPE_ERROR);
        eina_value_set(&copy, ENOTSUP);
     }
   _eina_promise_clean_dispatch(data, copy);
   return v;
}

static void
_proxy_cancel(void *data EINA_UNUSED, const Eina_Promise *dead_ptr EINA_UNUSED)
{
}

static Eina_Future_Scheduler *
_scheduler_get(Eina_Future *f)
{
   for (; f->prev != NULL; f = f->prev);
   assert(f->promise != NULL);
   return f->promise->scheduler;
}

EAPI Eina_Value
eina_future_as_value(Eina_Future *f)
{
   Eina_Value v = EINA_VALUE_EMPTY;
   Eina_Promise *p;
   Eina_Future *r_future;

   EINA_FUTURE_CHECK_RETURN_VAL(f, v);
   p = eina_promise_new(_scheduler_get(f), _proxy_cancel, NULL);
   EINA_SAFETY_ON_NULL_GOTO(p, err_promise);
   r_future = eina_future_then(f, _future_proxy, p);
   //If eina_future_then() fails f will be cancelled
   EINA_SAFETY_ON_NULL_GOTO(r_future, err_future);

   v = eina_promise_as_value(p);
   if (v.type == &EINA_VALUE_TYPE_PROMISE2)
     {
        DBG("Creating future proxy for future: %p - promise %p", f, p);
        return v;
     }

   //The promise was freed by eina_promise_as_value()
   ERR("Could not create a Eina_Value for future %p", f);
   //Futures will be unlinked
   _eina_future_free(r_future);
   _eina_future_cancel(f, ENOMEM);
   return v;

 err_future:
   /*
     Do not cancel the _eina_future_cancel(f,..) here, it
     was already canceled by eina_future_then().
   */
   _eina_promise_cancel(p);
   return v;

 err_promise:
   _eina_future_cancel(f, ENOMEM);
   return v;
}

EAPI Eina_Promise *
eina_promise_new(Eina_Future_Scheduler *scheduler,
                 Eina_Promise_Cancel_Cb cancel_cb, const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cancel_cb, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(scheduler, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(scheduler->schedule, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(scheduler->recall, NULL);

   Eina_Promise *p = eina_mempool_calloc(_promise_mp, sizeof(Eina_Promise));
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, NULL);
   p->cancel = cancel_cb;
   p->data = data;
   p->scheduler = scheduler;
   DBG("Creating new promise - Promise:%p, cb: %p, data:%p", p,
       p->cancel, p->data);
   return p;
}

EAPI void
eina_future_cancel(Eina_Future *f)
{
   EINA_FUTURE_CHECK_RETURN(f);
   _eina_future_cancel(f, ECANCELED);
}

EAPI void
eina_promise_resolve(Eina_Promise *p, Eina_Value value)
{
   EINA_PROMISE_CHECK_GOTO(p, err);
   _eina_promise_value_dbg("Resolve promise", p, value);
   _eina_promise_deliver(p, value);
   return;
 err:
   _eina_value_safe_flush(value);
}

EAPI void
eina_promise_reject(Eina_Promise *p, Eina_Error err)
{
   Eina_Value value;
   Eina_Bool r;

   EINA_PROMISE_CHECK_RETURN(p);
   r = eina_value_setup(&value, EINA_VALUE_TYPE_ERROR);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_setup);
   r = eina_value_set(&value, err);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_set);
   DBG("Reject promise %p - Error msg: '%s' - Error code: %d", p,
       eina_error_msg_get(err), err);
   _eina_promise_deliver(p, value);
   return;

 err_set:
   eina_value_flush(&value);
 err_setup:
   if (p->future) _eina_future_cancel(p->future, ENOMEM);
   else _eina_promise_cancel(p);
}

static void
_fake_future_dispatch(const Eina_Future_Desc desc, int err)
{
   /*
     This function is used to dispatch the Eina_Future_Cb in case,
     the future creation fails. By calling the Eina_Future_Cb
     the user has a chance to free allocated resources.
    */
   Eina_Value v, r;

   if (!desc.cb) return;
   eina_value_setup(&v, EINA_VALUE_TYPE_ERROR);
   eina_value_set(&v, err);
   //Since the future was not created the dead_ptr is NULL.
   r = desc.cb((void *)desc.data, v, NULL);
   _eina_value_safe_flush(r);
   eina_value_flush(&v);
}

static Eina_Future *
_eina_future_new(Eina_Promise *p, const Eina_Future_Desc desc)
{
   Eina_Future *f;

   f = eina_mempool_calloc(_future_mp, sizeof(Eina_Future));
   EINA_SAFETY_ON_NULL_GOTO(f, err_future);
   _eina_promise_link(p, f);
   f->cb = desc.cb;
   f->data = desc.data;
   if (desc.storage)
     {
        *desc.storage = f;
        f->storage = desc.storage;
     }
   DBG("Creating new future - Promise:%p, Future:%p, cb: %p, data: %p ",
       p, f, f->cb, f->data);
   return f;

 err_future:
   _fake_future_dispatch(desc, ENOMEM);
   if (p) _eina_promise_cancel(p);
   return NULL;
}

EAPI Eina_Future *
eina_future_new(Eina_Promise *p)
{
   static const Eina_Future_Desc desc = {
     .cb = NULL,
     .data = NULL
   };

   EINA_PROMISE_CHECK_RETURN_VAL(p, NULL);
   EINA_SAFETY_ON_TRUE_GOTO(p->future != NULL, err_has_future);

   return _eina_future_new(p, desc);

 err_has_future:
   //_eina_future_cancel() will also cancel the promise
   _eina_future_cancel(p->future, EINVAL);
   return NULL;
}

static Eina_Future *
_eina_future_then(Eina_Future *prev, const Eina_Future_Desc desc)
{
   Eina_Future *next = _eina_future_new(NULL, desc);
   EINA_SAFETY_ON_NULL_GOTO(next, err_next);
   next->prev = prev;
   prev->next = next;
   DBG("Linking futures - Prev:%p Next:%p", prev, next);
   return next;

 err_next:
   //_fake_future_dispatch() already called by _eina_future_new()
   _eina_future_cancel(prev, ENOMEM);
   return NULL;
}

EAPI Eina_Future *
eina_future_then_from_desc(Eina_Future *prev, const Eina_Future_Desc desc)
{
   EINA_FUTURE_CHECK_GOTO(prev, err_future);
   EINA_SAFETY_ON_TRUE_GOTO(prev->next != NULL, err_next);
   return _eina_future_then(prev, desc);

 err_next:
   _eina_future_cancel(prev->next, EINVAL);
 err_future:
   _fake_future_dispatch(desc, EINVAL);
   return NULL;
}

EAPI Eina_Future *
eina_future_chain_array(Eina_Future *prev, const Eina_Future_Desc descs[])
{
   Eina_Future *f = prev;
   ssize_t i = -1;
   int err = ENOMEM;

   EINA_FUTURE_CHECK_GOTO(prev, err_prev);
   EINA_SAFETY_ON_TRUE_GOTO(prev->next != NULL, err_next);

   for (i = 0; descs[i].cb; i++)
     {
        f = _eina_future_then(f, descs[i]);
        /*
          If _eina_future_then() fails the whole chain will be cancelled by it.
          All we need to do is free the remaining descs..
        */
        EINA_SAFETY_ON_NULL_GOTO(f, err_prev);
     }

   return f;

 err_next:
   err = EINVAL;
   _eina_future_cancel(f, err);
 err_prev:
   /*
     If i > 0 we'll start to dispatch fake futures
     at i + 1, since the descs[i] was already freed
     by _eina_future_then()
    */
   for (i = i + 1; descs[i].cb; i++)
     _fake_future_dispatch(descs[i], err);
   return NULL;
}

EAPI Eina_Future *
eina_future_chain_easy_array(Eina_Future *prev, const Eina_Future_Cb_Easy_Desc descs[])
{
   size_t i = -1;
   Eina_Future *f = prev;
   int err = ENOMEM;

   EINA_FUTURE_CHECK_GOTO(prev, err_prev);
   EINA_SAFETY_ON_TRUE_GOTO(prev->next != NULL, err_next);

   for (i = 0; descs[i].success || descs[i].error || descs[i].free || descs[i].success_type; i++)
     {
        Eina_Future_Desc fdesc = eina_future_cb_easy_from_desc(descs[i]);
        f = _eina_future_then(f, fdesc);
        EINA_SAFETY_ON_NULL_GOTO(f, err_prev);
     }

   return f;

 err_next:
   err = EINVAL;
   _eina_future_cancel(f, err);
 err_prev:
   /*
     If i > 0 we'll start to dispatch fake futures
     at i + 1, since the descs[i] was already freed
     by _eina_future_then()
    */
   for (i = i + 1; descs[i].error || descs[i].free; i++)
     {
        if (descs[i].error)
          {
             Eina_Value v = descs[i].error((void *)descs[i].data, err);
             _eina_value_safe_flush(v);
          }
        if (descs[i].free) descs[i].free((void *)descs[i].data, NULL);
     }
   return NULL;
}

static Eina_Value
_eina_future_cb_console(void *data,
                        const Eina_Value value,
                        const Eina_Future *dead_future EINA_UNUSED)
{
   Eina_Future_Cb_Console_Desc *c = data;
   const char *prefix = c ? c->prefix : NULL;
   const char *suffix = c ? c->suffix : NULL;
   const char *content = "no value";
   char *str = NULL;

   if (value.type)
     {
        str = eina_value_to_string(&value);
        content = str;
     }

   if (!prefix) prefix = "";
   if (!suffix) suffix = "\n";
   printf("%s%s%s", prefix, content, suffix);
   free(str);
   if (c) {
      free((void *)c->prefix);
      free((void *)c->suffix);
      free(c);
   }
   return value;
}

EAPI Eina_Future_Desc
eina_future_cb_console_from_desc(const Eina_Future_Cb_Console_Desc desc)
{
   Eina_Future_Cb_Console_Desc *c;
   Eina_Future_Desc fdesc = {
     .cb = _eina_future_cb_console,
     .data = NULL
   };

   if (desc.prefix || desc.suffix) {
      fdesc.data = c = calloc(1, sizeof(Eina_Future_Cb_Console_Desc));
      EINA_SAFETY_ON_NULL_GOTO(c, exit);
      c->prefix = desc.prefix ? strdup(desc.prefix) : NULL;
      c->suffix = desc.suffix ? strdup(desc.suffix) : NULL;
   }
 exit:
   return fdesc;
}

static Eina_Value
_eina_future_cb_convert_to(void *data, const Eina_Value src,
                           const Eina_Future *dead_future EINA_UNUSED)
{
    const Eina_Value_Type *type = data;
    Eina_Value dst = EINA_VALUE_EMPTY;

    if (!type) return src; // pass thru

    if (!eina_value_setup(&dst, type))
      {
         eina_value_setup(&dst, EINA_VALUE_TYPE_ERROR);
         eina_value_set(&dst, ENOMEM);
      }
    else if (src.type) eina_value_convert(&src, &dst);
    // otherwise leave initial value for empty

    return dst;
}

EAPI Eina_Future_Desc
eina_future_cb_convert_to(const Eina_Value_Type *type)
{
   return (Eina_Future_Desc){.cb = _eina_future_cb_convert_to, .data = type};
}

EAPI void *
eina_promise_data_get(const Eina_Promise *p)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, NULL);
   return (void *)p->data;
}

static Eina_Value
_eina_future_cb_easy(void *data, const Eina_Value value,
                     const Eina_Future *dead_future)
{
    Eina_Future_Cb_Easy_Desc *d = data;
    Eina_Value ret = EINA_VALUE_EMPTY;
    if (!d)
      {
         if (eina_value_setup(&ret, EINA_VALUE_TYPE_ERROR)) eina_value_set(&ret, ENOMEM);
         return ret;
      }
    EASY_FUTURE_DISPATCH(ret, value, dead_future, d, (void*)d->data);
    free(d);
    return ret;
}

EAPI Eina_Future_Desc
eina_future_cb_easy_from_desc(const Eina_Future_Cb_Easy_Desc desc)
{
   Eina_Future_Cb_Easy_Desc *d = calloc(1, sizeof(Eina_Future_Cb_Easy_Desc));
   EINA_SAFETY_ON_NULL_GOTO(d, end);
   *d = desc;
 end:
   return (Eina_Future_Desc){ .cb = _eina_future_cb_easy, .data = d };
}

typedef struct _Base_Ctx {
   Eina_Promise *promise;
   Eina_Future **futures;
   unsigned int futures_len;
} Base_Ctx;

typedef struct _All_Promise2_Ctx {
   Base_Ctx base;
   Eina_Value values;
   unsigned int processed;
} All_Promise2_Ctx;

typedef struct _Race_Promise2_Ctx {
   Base_Ctx base;
   Eina_Bool dispatching;
} Race_Promise2_Ctx;

static void
_base_ctx_clean(Base_Ctx *ctx)
{
   unsigned int i;
   for (i = 0; i < ctx->futures_len; i++)
     if (ctx->futures[i]) _eina_future_cancel(ctx->futures[i], ECANCELED);
   free(ctx->futures);
}

static void
_all_promise2_ctx_free(All_Promise2_Ctx *ctx)
{
   _base_ctx_clean(&ctx->base);
   eina_value_flush(&ctx->values);
   free(ctx);
}

static void
_all_promise2_cancel(void *data, const Eina_Promise *dead EINA_UNUSED)
{
   _all_promise2_ctx_free(data);
}

static void
_race_promise2_ctx_free(Race_Promise2_Ctx *ctx)
{
   _base_ctx_clean(&ctx->base);
   free(ctx);
}

static void
_race_promise2_cancel(void *data, const Eina_Promise *dead EINA_UNUSED)
{
   _race_promise2_ctx_free(data);
}

static Eina_Bool
_future_unset(Base_Ctx *ctx, unsigned int *pos, const Eina_Future *dead_ptr)
{
   unsigned int i;

   for (i = 0; i < ctx->futures_len; i++)
     {
        if (ctx->futures[i] == dead_ptr)
          {
             ctx->futures[i] = NULL;
             *pos = i;
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

static Eina_Value
_race_then_cb(void *data, const Eina_Value v,
              const Eina_Future *dead_ptr)
{
   Race_Promise2_Ctx *ctx = data;
   Eina_Promise *p = ctx->base.promise;
   Eina_Bool found, r;
   Eina_Value result;
   unsigned int i;

   //This is not allowed!
   assert(v.type != &EINA_VALUE_TYPE_PROMISE2);
   found = _future_unset(&ctx->base, &i, dead_ptr);
   assert(found);

   if (ctx->dispatching) return EINA_VALUE_EMPTY;
   ctx->dispatching = EINA_TRUE;

   //By freeing the race_ctx all the other futures will be cancelled.
   _race_promise2_ctx_free(ctx);

   r = eina_value_struct_setup(&result, &RACE_STRUCT_DESC);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_setup);
   r = eina_value_struct_set(&result, "value", v);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_set);
   r = eina_value_struct_set(&result, "index", i);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_set);
   //We're in a safe context (from mainloop), so we can avoid scheduling a new dispatch
   _eina_promise_clean_dispatch(p, result);
   return v;

 err_set:
   eina_value_flush(&result);
 err_setup:
   eina_value_setup(&result, EINA_VALUE_TYPE_ERROR);
   eina_value_set(&result, ENOMEM);
   _eina_promise_clean_dispatch(p, result);
   return v;
}

static Eina_Value
_all_then_cb(void *data, const Eina_Value v,
             const Eina_Future *dead_ptr)
{
   All_Promise2_Ctx *ctx = data;
   unsigned int i = 0;
   Eina_Bool found;

   //This is not allowed!
   assert(v.type != &EINA_VALUE_TYPE_PROMISE2);

   found = _future_unset(&ctx->base, &i, dead_ptr);
   assert(found);

   ctx->processed++;
   eina_value_array_set(&ctx->values, i, v);
   if (ctx->processed == ctx->base.futures_len)
     {
        //We're in a safe context (from mainloop), so we can avoid scheduling a new dispatch
        _eina_promise_clean_dispatch(ctx->base.promise, ctx->values);
        ctx->values = EINA_VALUE_EMPTY; /* flushed in _eina_promise_clean_dispatch() */
        _all_promise2_ctx_free(ctx);
     }
   return v;
}

static void
_future2_array_cancel(Eina_Future *array[])
{
   size_t i;
   for (i = 0; array[i]; i++) _eina_future_cancel(array[i], ENOMEM);
}

static Eina_Bool
promise_proxy_of_future_array_create(Eina_Future *array[],
                                     Base_Ctx *ctx,
                                     Eina_Promise_Cancel_Cb cancel_cb,
                                     Eina_Future_Cb future_cb)
{
   unsigned int i;

   //Count how many futures...
   for (i = 0; array[i]; i++);

   EINA_SAFETY_ON_FALSE_RETURN_VAL(i > 0, EINA_FALSE);

   ctx->promise = eina_promise_new(_scheduler_get(array[0]), cancel_cb, ctx);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx->promise, EINA_FALSE);

   ctx->futures_len = i;
   ctx->futures = calloc(ctx->futures_len, sizeof(Eina_Future *));
   EINA_SAFETY_ON_NULL_GOTO(ctx->futures, err_futures);

   for (i = 0; i < ctx->futures_len; i++)
     {
        ctx->futures[i] = eina_future_then(array[i], future_cb, ctx);
        //Futures will be cancelled by the caller...
        EINA_SAFETY_ON_NULL_GOTO(ctx->futures[i], err_then);
     }
   return EINA_TRUE;

 err_then:
   //This will also unlink the prev future...
   while (i >= 1) _eina_future_free(ctx->futures[--i]);
   free(ctx->futures);
   ctx->futures = NULL;
 err_futures:
   ctx->futures_len = 0;
   eina_mempool_free(_promise_mp, ctx->promise);
   ctx->promise = NULL;
   return EINA_FALSE;
}

EAPI Eina_Promise *
eina_promise_all_array(Eina_Future *array[])
{
   All_Promise2_Ctx *ctx;
   unsigned int i;
   Eina_Bool r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(array, NULL);
   ctx = calloc(1, sizeof(All_Promise2_Ctx));
   EINA_SAFETY_ON_NULL_GOTO(ctx, err_ctx);
   r = eina_value_array_setup(&ctx->values, EINA_VALUE_TYPE_VALUE, 0);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_array);
   r = promise_proxy_of_future_array_create(array, &ctx->base,
                                            _all_promise2_cancel, _all_then_cb);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_promise);

   for (i = 0; i < ctx->base.futures_len; i++)
     {
        Eina_Bool r;
        Eina_Value v;

        //Stub values...
        r = eina_value_setup(&v, EINA_VALUE_TYPE_INT);
        EINA_SAFETY_ON_FALSE_GOTO(r, err_stub);
        r = eina_value_array_append(&ctx->values, v);
        eina_value_flush(&v);
        EINA_SAFETY_ON_FALSE_GOTO(r, err_stub);
     }

   return ctx->base.promise;

 err_stub:
   for (i = 0; i < ctx->base.futures_len; i++) _eina_future_free(ctx->base.futures[i]);
   free(ctx->base.futures);
   eina_mempool_free(_promise_mp, ctx->base.promise);
 err_promise:
   eina_value_flush(&ctx->values);
 err_array:
   free(ctx);
 err_ctx:
   _future2_array_cancel(array);
   return NULL;
}

EAPI Eina_Promise *
eina_promise_race_array(Eina_Future *array[])
{
   Race_Promise2_Ctx *ctx;
   Eina_Bool r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(array, NULL);
   ctx = calloc(1, sizeof(Race_Promise2_Ctx));
   EINA_SAFETY_ON_NULL_GOTO(ctx, err_ctx);
   r = promise_proxy_of_future_array_create(array, &ctx->base,
                                            _race_promise2_cancel,
                                            _race_then_cb);
   EINA_SAFETY_ON_FALSE_GOTO(r, err_promise);

   return ctx->base.promise;

 err_promise:
   free(ctx);
 err_ctx:
   _future2_array_cancel(array);
   return NULL;
}
