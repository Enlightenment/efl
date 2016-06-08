#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>

#include <assert.h>

#include <ecore_private.h>

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
   Eina_Promise promise_vtable;
   Eina_Promise* eina_promise;
   _Ecore_Thread_Data thread_callback_data;
   int ref_count;
   int then_count;
};
typedef struct  _Ecore_Thread_Promise_Owner _Ecore_Thread_Promise_Owner;

#define ECORE_PROMISE_GET_OWNER(p) (_Ecore_Thread_Promise_Owner*)((unsigned char*)p - offsetof(struct _Ecore_Thread_Promise_Owner, promise_vtable))

static void _ecore_promise_ref_update(_Ecore_Thread_Promise_Owner* p)
{
  if(p->ref_count < 0)
    {
       ERR("Reference count is negative for promise %p\n", p);
    }
  if(!p->ref_count)
    {
       p->eina_promise->unref(p->eina_promise);
       if (!p->then_count) // Whether we still own the initial eina_promise ref
         p->eina_promise->unref(p->eina_promise);
       p->eina_promise = NULL;
       p->eina_owner = NULL;
       p->thread_callback_data.thread = NULL;
       free(p);
    }
}

static void _ecore_promise_thread_release_ref(void* data, void* value EINA_UNUSED)
{
   _Ecore_Thread_Promise_Owner* p = data;
   p->ref_count -= p->then_count;
   _ecore_promise_ref_update(p);
}

static void _ecore_promise_thread_end(void* data, Ecore_Thread* thread EINA_UNUSED)
{
   _Ecore_Thread_Promise_Owner* p = data;
   if(!eina_promise_owner_pending_is(p->eina_owner))
     {
        eina_promise_owner_default_manual_then_set(p->eina_owner, EINA_FALSE);
        eina_promise_owner_default_call_then(p->eina_owner);
        p->ref_count -= p->then_count;
        _ecore_promise_ref_update(p);
     }
   else
     {
        eina_promise_owner_default_manual_then_set(p->eina_owner, EINA_FALSE);
        // This then will steal the ref to the eina_promise, working
        // refcount-wise like a normal then on the thread promise.
        p->ref_count++;
        p->then_count++;
        eina_promise_then(p->eina_promise, &_ecore_promise_thread_release_ref,
                          (Eina_Promise_Error_Cb)&_ecore_promise_thread_release_ref, p);
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

static void _ecore_promise_cancel_cb(void* data, Eina_Promise_Owner* promise EINA_UNUSED)
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
   return &promise->promise_vtable;
}
static void _ecore_thread_promise_owner_progress(_Ecore_Thread_Promise_Owner* promise, void* data)
{
   ecore_thread_feedback(promise->thread_callback_data.thread, data);
}
static void _ecore_thread_promise_owner_progress_notify(_Ecore_Thread_Promise_Owner* promise,
                                                        Eina_Promise_Progress_Notify_Cb progress_cb,
                                                        void* data, Eina_Promise_Free_Cb free_cb)
{
   promise->eina_owner->progress_notify(promise->eina_owner, progress_cb, data, free_cb);
}

static void _ecore_promise_then(Eina_Promise* promise, Eina_Promise_Cb callback,
                                Eina_Promise_Error_Cb error_cb, void* data)
{
   _Ecore_Thread_Promise_Owner* v = ECORE_PROMISE_GET_OWNER(promise);
   v->eina_promise->then(v->eina_promise, callback, error_cb, data);
   v->ref_count++;
   v->then_count++;
}
static void* _ecore_promise_value_get(Eina_Promise const* promise)
{
   _Ecore_Thread_Promise_Owner* v = ECORE_PROMISE_GET_OWNER(promise);
   return v->eina_promise->value_get(v->eina_promise);
}
static Eina_Error _ecore_promise_error_get(Eina_Promise const* promise)
{
   _Ecore_Thread_Promise_Owner* v = ECORE_PROMISE_GET_OWNER(promise);
   return v->eina_promise->error_get(v->eina_promise);
}
static Eina_Bool _ecore_promise_pending_is(Eina_Promise const* promise)
{
   _Ecore_Thread_Promise_Owner* v = ECORE_PROMISE_GET_OWNER(promise);
   return v->eina_promise->pending_is(v->eina_promise);
}
static void _ecore_promise_progress_cb_add(Eina_Promise const* promise, Eina_Promise_Progress_Cb callback, void* data,
                                           Eina_Promise_Free_Cb free_cb)
{
   _Ecore_Thread_Promise_Owner* v = ECORE_PROMISE_GET_OWNER(promise);
   v->eina_promise->progress_cb_add(v->eina_promise, callback, data, free_cb);
}
static void _ecore_promise_cancel(Eina_Promise const* promise)
{
   _Ecore_Thread_Promise_Owner* v = ECORE_PROMISE_GET_OWNER(promise);
   v->eina_promise->cancel(v->eina_promise);
}
static void _ecore_promise_ref(Eina_Promise const* promise)
{
   _Ecore_Thread_Promise_Owner* v = ECORE_PROMISE_GET_OWNER(promise);
   ++v->ref_count;   
}
static void _ecore_promise_unref(Eina_Promise const* promise)
{
   _Ecore_Thread_Promise_Owner* v = ECORE_PROMISE_GET_OWNER(promise);
   --v->ref_count;   
}

Ecore_Thread* ecore_thread_promise_run(Ecore_Thread_Promise_Cb func_blocking,
                                       Ecore_Thread_Promise_Cb func_cancel,
                                       const void* data,
                                       Eina_Promise** promise)
{
   _Ecore_Thread_Promise_Owner* priv;

   priv = malloc(sizeof(_Ecore_Thread_Promise_Owner));

   priv->eina_owner = eina_promise_add();

   priv->owner_vtable.version = EINA_PROMISE_VERSION;
   priv->owner_vtable.value_set = EINA_FUNC_PROMISE_OWNER_VALUE_SET(&_ecore_promise_owner_value_set);
   priv->owner_vtable.error_set = EINA_FUNC_PROMISE_OWNER_ERROR_SET(&_ecore_promise_owner_error_set);
   priv->owner_vtable.promise_get = EINA_FUNC_PROMISE_OWNER_PROMISE_GET(&_ecore_thread_promise_owner_promise_get);
   priv->owner_vtable.pending_is = EINA_FUNC_PROMISE_OWNER_PENDING_IS(&_ecore_promise_owner_pending_is);
   priv->owner_vtable.cancelled_is = EINA_FUNC_PROMISE_OWNER_CANCELLED_IS(&_ecore_promise_owner_cancelled_is);
   priv->owner_vtable.progress = EINA_FUNC_PROMISE_OWNER_PROGRESS(&_ecore_thread_promise_owner_progress);
   priv->owner_vtable.progress_notify = EINA_FUNC_PROMISE_OWNER_PROGRESS_NOTIFY(&_ecore_thread_promise_owner_progress_notify);
   EINA_MAGIC_SET(&priv->owner_vtable, EINA_MAGIC_PROMISE_OWNER);

   priv->promise_vtable.then = EINA_FUNC_PROMISE_THEN(&_ecore_promise_then);
   priv->promise_vtable.value_get = EINA_FUNC_PROMISE_VALUE_GET(&_ecore_promise_value_get);
   priv->promise_vtable.error_get = EINA_FUNC_PROMISE_ERROR_GET(&_ecore_promise_error_get);
   priv->promise_vtable.pending_is = EINA_FUNC_PROMISE_PENDING_IS(&_ecore_promise_pending_is);
   priv->promise_vtable.progress_cb_add = EINA_FUNC_PROMISE_PROGRESS_CB_ADD(&_ecore_promise_progress_cb_add);
   priv->promise_vtable.cancel = EINA_FUNC_PROMISE_CANCEL(&_ecore_promise_cancel);
   priv->promise_vtable.ref = EINA_FUNC_PROMISE_REF(&_ecore_promise_ref);
   priv->promise_vtable.unref = EINA_FUNC_PROMISE_UNREF(&_ecore_promise_unref);
   EINA_MAGIC_SET(&priv->promise_vtable, EINA_MAGIC_PROMISE);
   
   priv->thread_callback_data.data = data;
   priv->thread_callback_data.func_blocking = func_blocking;
   priv->thread_callback_data.func_cancel = func_cancel;
   eina_promise_owner_default_manual_then_set(priv->eina_owner, EINA_TRUE);

   priv->eina_promise = priv->eina_owner->promise_get(priv->eina_owner);
   priv->eina_promise->ref(priv->eina_promise);
   priv->ref_count = 0;
   priv->then_count = 0;

   if(func_cancel)
     eina_promise_owner_default_cancel_cb_add(priv->eina_owner, &_ecore_promise_cancel_cb, priv, NULL);
   priv->thread_callback_data.thread =
     ecore_thread_feedback_run(&_ecore_promise_thread_blocking, &_ecore_promise_thread_notify,
                               &_ecore_promise_thread_end, &_ecore_promise_thread_cancel, priv,
                               EINA_FALSE);
   if(promise)
     *promise = &priv->promise_vtable;
   return priv->thread_callback_data.thread;
}
