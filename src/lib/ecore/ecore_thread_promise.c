#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>

#include <assert.h>

struct _Ecore_Thread_Data
{
   Ecore_Thread_Promise_Cb func_blocking;
   Ecore_Thread_Promise_Cb func_cancel;
   void const* data;
   Ecore_Thread* thread;
};
typedef struct _Ecore_Thread_Data _Ecore_Thread_Data;

struct _Ecore_Thread_Promise_Owner
{
   Eina_Promise_Owner owner_vtable;
   Eina_Promise_Owner* eina_owner;
   _Ecore_Thread_Data thread_callback_data;
};
typedef struct  _Ecore_Thread_Promise_Owner _Ecore_Thread_Promise_Owner;

static void _ecore_promise_thread_end(void* data, Ecore_Thread* thread EINA_UNUSED)
{
   _Ecore_Thread_Promise_Owner* p = data;
   if(!eina_promise_owner_pending_is(p->eina_owner))
     {
        eina_promise_owner_default_manual_then_set(p->eina_owner, EINA_FALSE);
        eina_promise_owner_default_call_then(p->eina_owner);
     }
   else
     {
        eina_promise_owner_default_manual_then_set(p->eina_owner, EINA_FALSE);
     }
}

static void
_ecore_promise_thread_blocking(void* data, Ecore_Thread* thread EINA_UNUSED)
{
   _Ecore_Thread_Promise_Owner* promise = data;
   (promise->thread_callback_data.func_blocking)(promise->thread_callback_data.data, &promise->owner_vtable, thread);
}

static void _ecore_promise_thread_notify(void* data, Ecore_Thread* thread EINA_UNUSED, void* msg_data)
{
   _Ecore_Thread_Promise_Owner* promise = data;
   eina_promise_owner_progress(promise->eina_owner, msg_data);
}

static void _ecore_promise_cancel(void* data, Eina_Promise_Owner* promise EINA_UNUSED)
{
   _Ecore_Thread_Promise_Owner* priv = data;
   (priv->thread_callback_data.func_cancel)(priv->thread_callback_data.data, &priv->owner_vtable,
                                            priv->thread_callback_data.thread);
}

static void _ecore_promise_thread_cancel(void* data, Ecore_Thread* thread EINA_UNUSED)
{
   _Ecore_Thread_Promise_Owner* owner = data;
   Eina_Promise* promise;

   promise = eina_promise_owner_promise_get(owner->eina_owner);
   eina_promise_cancel(promise);
}

static void* _ecore_promise_owner_buffer_get(_Ecore_Thread_Promise_Owner* promise)
{
   return promise->eina_owner->buffer_get(promise->eina_owner);
}
static size_t _ecore_promise_owner_value_size_get(_Ecore_Thread_Promise_Owner const* promise)
{
   return promise->eina_owner->value_size_get(promise->eina_owner);
}
static void _ecore_promise_owner_value_set(_Ecore_Thread_Promise_Owner* promise, void* data, Eina_Promise_Free_Cb free)
{
   promise->eina_owner->value_set(promise->eina_owner, data, free);
}
static void _ecore_promise_owner_error_set(_Ecore_Thread_Promise_Owner* promise, Eina_Error error)
{
   promise->eina_owner->error_set(promise->eina_owner, error);
}
static Eina_Bool _ecore_promise_owner_pending_is(_Ecore_Thread_Promise_Owner const* promise)
{
   return promise->eina_owner->pending_is(promise->eina_owner);
}
static Eina_Bool _ecore_promise_owner_cancelled_is(_Ecore_Thread_Promise_Owner const* promise)
{
   return promise->eina_owner->cancelled_is(promise->eina_owner);
}
static Eina_Promise* _ecore_thread_promise_owner_promise_get(_Ecore_Thread_Promise_Owner* promise)
{
   return promise->eina_owner->promise_get(promise->eina_owner);
}
static void _ecore_thread_promise_owner_progress(_Ecore_Thread_Promise_Owner* promise, void* data)
{
   ecore_thread_feedback(promise->thread_callback_data.thread, data);
}

Ecore_Thread* ecore_thread_promise_run(Ecore_Thread_Promise_Cb func_blocking,
                                       Ecore_Thread_Promise_Cb func_cancel,
                                       const void* data, size_t value_size,
                                       Eina_Promise** promise)
{
   _Ecore_Thread_Promise_Owner* priv;

   priv = malloc(sizeof(_Ecore_Thread_Promise_Owner));

   priv->eina_owner = eina_promise_default_add(value_size);

   priv->owner_vtable.version = EINA_PROMISE_VERSION;
   priv->owner_vtable.value_set = EINA_FUNC_PROMISE_OWNER_VALUE_SET(&_ecore_promise_owner_value_set);
   priv->owner_vtable.error_set = EINA_FUNC_PROMISE_OWNER_ERROR_SET(&_ecore_promise_owner_error_set);
   priv->owner_vtable.buffer_get = EINA_FUNC_PROMISE_OWNER_BUFFER_GET(&_ecore_promise_owner_buffer_get);
   priv->owner_vtable.value_size_get = EINA_FUNC_PROMISE_OWNER_VALUE_SIZE_GET(&_ecore_promise_owner_value_size_get);
   priv->owner_vtable.promise_get = EINA_FUNC_PROMISE_OWNER_PROMISE_GET(&_ecore_thread_promise_owner_promise_get);
   priv->owner_vtable.pending_is = EINA_FUNC_PROMISE_OWNER_PENDING_IS(&_ecore_promise_owner_pending_is);
   priv->owner_vtable.cancelled_is = EINA_FUNC_PROMISE_OWNER_CANCELLED_IS(&_ecore_promise_owner_cancelled_is);
   priv->owner_vtable.progress = EINA_FUNC_PROMISE_OWNER_PROGRESS(&_ecore_thread_promise_owner_progress);

   priv->thread_callback_data.data = data;
   priv->thread_callback_data.func_blocking = func_blocking;
   priv->thread_callback_data.func_cancel = func_cancel;
   eina_promise_owner_default_manual_then_set(priv->eina_owner, EINA_TRUE);
   if(func_cancel)
     eina_promise_owner_default_cancel_cb_add(priv->eina_owner, &_ecore_promise_cancel, priv, NULL);
   priv->thread_callback_data.thread =
     ecore_thread_feedback_run(&_ecore_promise_thread_blocking, &_ecore_promise_thread_notify,
                               &_ecore_promise_thread_end, &_ecore_promise_thread_cancel, priv,
                               EINA_FALSE);
   if(promise)
     *promise = priv->eina_owner->promise_get(priv->eina_owner);
   return priv->thread_callback_data.thread;
}
