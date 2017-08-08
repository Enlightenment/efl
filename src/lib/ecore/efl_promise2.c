#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Ecore.h"
#include "efl_promise2.h"
#include "ecore_private.h"
#include "efl_promise2_priv.h"

struct _Efl_Promise2 {
   Eina_Free_Cb cancel;
   Efl_Future2 *future;
   const void *data;
};

struct _Efl_Future2 {
   Efl_Future2_Cb success;
   Efl_Future2_Cb error;
   Eina_Free_Cb free;
   Efl_Promise2 *promise;
   Efl_Future2 *next;
   Efl_Future2 *prev;
   Eina_Value *pending_value;
   const void *data;
   Ecore_Event *event;
   Eina_Bool reject;
};

static Eina_Mempool *_promise_mp = NULL;
static Eina_Mempool *_future_mp = NULL;
static Eina_List *_pending_futures = NULL;
static int FUTURE_EVENT_ID = -1;
static Ecore_Event_Handler *_future_event_handler = NULL;

static void
_efl_future2_dispatch(Efl_Future2 *f, Eina_Value *value, Eina_Bool rejected)
{
   while (f)
     {
        if (((!rejected && f->success) || (rejected && f->error)))
          break;
        Efl_Future2 *n = f->next;
        Eina_Free_Cb free_cb = f->free;
        const void *data = f->data;
        if (n) n->prev = NULL;
        f->next = NULL;
        eina_mempool_free(_future_mp, f);
        if (free_cb) free((void *)data);
        f = n;
     }

   if (!f)
     {
        eina_value_free(value);
        return;
     }

   Eina_Value *next_value = rejected ? f->error((void *)f->data, value) :
     f->success((void *)f->data, value);
   if (value != next_value) eina_value_free(value);
   if (f->free) f->free((void *)f->data);

   Efl_Future2 *next_future = f->next;
   if (next_future) next_future->prev = NULL;
   eina_mempool_free(_future_mp, f);

   if (!next_future)
     {
        eina_value_free(next_value);
        return;
     }

   if (next_value && next_value->type == EINA_VALUE_TYPE_PROMISE2)
     {
        Efl_Promise2 *p = eina_value_promise2_steal(next_value);
        eina_value_free(next_value);
        f->reject = rejected;
        p->future = next_future;
     }
   else if (((next_value && next_value->type == EINA_VALUE_TYPE_ERROR) || (rejected)))
     _efl_future2_dispatch(next_future, next_value, EINA_TRUE);
   else
     _efl_future2_dispatch(next_future, next_value, EINA_FALSE);
}

static Eina_Bool
_efl_future2_handler_cb(void *data EINA_UNUSED,
                        int type EINA_UNUSED,
                        void *event)
{
   Efl_Future2 *f = event;
   _efl_future2_dispatch(f, f->pending_value, f->reject);
   _pending_futures = eina_list_remove(_pending_futures, f);
   return ECORE_CALLBACK_DONE;
}

static void
_dummy_free(void *user_data EINA_UNUSED, void *func_data EINA_UNUSED)
{
}

static Eina_Bool
_efl_future2_schedule(Efl_Future2 *f, Eina_Value *value, Eina_Bool reject)
{
   EINA_SAFETY_ON_TRUE_RETURN_VAL(f->event != NULL, EINA_FALSE);

   f->reject = reject;
   if (value && value->type == EINA_VALUE_TYPE_PROMISE2)
     {
        Efl_Promise2 *p = eina_value_promise2_steal(value);
        eina_value_free(value);
        p->future = f;
     }
   else
     {
        f->event = ecore_event_add(FUTURE_EVENT_ID, f, _dummy_free, NULL);
        EINA_SAFETY_ON_NULL_RETURN_VAL(f->event, EINA_FALSE);
        f->pending_value = value;
        _pending_futures = eina_list_append(_pending_futures, f);
     }
   return EINA_TRUE;
}

Eina_Bool
efl_promise2_init(void)
{
   const char *choice;

   choice = getenv("EINA_MEMPOOL");
   if ((!choice) || (!choice[0]))
     choice = "chained_mempool";

   _promise_mp = eina_mempool_add(choice, "Efl_Promise2",
                                  NULL, sizeof(Efl_Promise2), 16);
   EINA_SAFETY_ON_NULL_RETURN_VAL(_promise_mp, EINA_FALSE);

   _future_mp = eina_mempool_add(choice, "Efl_Future2",
                                 NULL, sizeof(Efl_Future2), 16);
   EINA_SAFETY_ON_NULL_GOTO(_future_mp, err_future);

   FUTURE_EVENT_ID = ecore_event_type_new();

   _future_event_handler = ecore_event_handler_add(FUTURE_EVENT_ID,
                                                   _efl_future2_handler_cb,
                                                   NULL);
   EINA_SAFETY_ON_NULL_GOTO(_future_event_handler, err_handler);

   return EINA_TRUE;

 err_handler:
   eina_mempool_del(_future_mp);
   _future_mp = NULL;
 err_future:
   eina_mempool_del(_promise_mp);
   _promise_mp = NULL;
   return EINA_FALSE;
}

void
efl_promise2_shutdown(void)
{
   Efl_Future2 *f;

   EINA_LIST_FREE(_pending_futures, f)
     _efl_future2_dispatch(f, f->pending_value, f->reject);

   ecore_event_handler_del(_future_event_handler);
   eina_mempool_del(_future_mp);
   eina_mempool_del(_promise_mp);
   _promise_mp = NULL;
   _future_mp = NULL;
   _future_event_handler = NULL;
   FUTURE_EVENT_ID = -1;
}

EAPI Eina_Value *
efl_promise2_as_value(Efl_Promise2 *p)
{
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_PROMISE2);
   EINA_SAFETY_ON_NULL_RETURN_VAL(v, NULL);
   eina_value_pset(v, p);
   return v;
}

EAPI Eina_Value *
efl_future2_as_value(Efl_Future2 *f)
{
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_FUTURE2);
   EINA_SAFETY_ON_NULL_RETURN_VAL(v, NULL);
   eina_value_pset(v, f);
   return v;
}

EAPI Efl_Promise2 *
efl_promise2_new(Eina_Free_Cb cancel_cb, const void *data)
{
   Efl_Promise2 *p = eina_mempool_calloc(_promise_mp, sizeof(Efl_Promise2));
   EINA_SAFETY_ON_NULL_RETURN_VAL(p, NULL);
   p->cancel = cancel_cb;
   p->data = data;
   return p;
}

EAPI void
efl_promise2_cancel(Efl_Promise2 *p)
{
   if (p->cancel)
     p->cancel((void *)p->data);
   if (p->future)
     {
        if (p->future->event)
          ecore_event_del(p->future->event);
        p->future->promise = NULL;
     }
   eina_mempool_free(_promise_mp, p);
}

EAPI Eina_Bool
efl_promise2_resolve(Efl_Promise2 *p, Eina_Value *value)
{
   Eina_Bool r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(p, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(p->future, EINA_FALSE);
   r = _efl_future2_schedule(p->future, value, EINA_FALSE);
   eina_mempool_free(_promise_mp, p);
   return r;
}

EAPI Eina_Bool
efl_promise2_reject(Efl_Promise2 *p, Eina_Value *value)
{
   Eina_Bool r;

   EINA_SAFETY_ON_NULL_RETURN_VAL(p, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(p->future, EINA_FALSE);
   r = _efl_future2_schedule(p->future, value, EINA_TRUE);
   eina_mempool_free(_promise_mp, p);
   return r;
}

EAPI Efl_Future2 *
efl_future2_new(Efl_Promise2 *p,
                Efl_Future2_Cb success,
                Efl_Future2_Cb error,
                Eina_Free_Cb free_cb,
                const void *data)
{
   Efl_Future2 *f;

   EINA_SAFETY_ON_TRUE_RETURN_VAL(p && p->future, NULL);

   f = eina_mempool_calloc(_future_mp, sizeof(Efl_Future2));
   EINA_SAFETY_ON_NULL_RETURN_VAL(f, NULL);
   f->promise = p;
   if (p)
     p->future = f;
   f->success = success;
   f->error = error;
   f->free = free_cb;
   f->data = data;
   return f;
}

EAPI Efl_Future2 *
efl_future2_then(Efl_Future2 *prev, Efl_Future2_Cb success,
                 Efl_Future2_Cb error, Eina_Free_Cb free_cb,
                 const void *data)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(prev, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(prev->next != NULL, NULL);

   Efl_Future2 *next = efl_future2_new(NULL, success, error, free_cb, data);
   next->prev = prev;
   prev->next = next;
   return next;
}
