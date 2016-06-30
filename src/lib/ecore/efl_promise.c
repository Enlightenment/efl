#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

// FIXME: handle self destruction when back in the main loop

typedef struct _Efl_Promise_Data Efl_Promise_Data;
typedef struct _Efl_Promise_Msg Efl_Promise_Msg;

struct _Efl_Promise_Msg
{
   EINA_REFCOUNT;

   void *value;
   Eina_Free_Cb free_cb;

   Eina_Error error;
};

struct _Efl_Promise_Data
{
   Efl_Promise *promise;
   Efl_Promise_Msg *message;
   Eina_List *futures;

   struct {
      Eina_Bool future : 1;
      Eina_Bool future_triggered : 1;
      Eina_Bool progress : 1;
      Eina_Bool progress_triggered : 1;
   } set;
};

static void
_efl_promise_msg_free(Efl_Promise_Msg *msg)
{
   if (msg->free_cb)
     msg->free_cb(msg->value);
   free(msg);
}

#define EFL_LOOP_FUTURE_CLASS efl_loop_future_class_get()
static const Efl_Class *efl_loop_future_class_get(void);

typedef struct _Efl_Loop_Future_Data Efl_Loop_Future_Data;
typedef struct _Efl_Loop_Future_Callback Efl_Loop_Future_Callback;

struct _Efl_Loop_Future_Callback
{
   EINA_INLIST;

   Efl_Event_Cb success;
   Efl_Event_Cb failure;
   Efl_Event_Cb progress;

   Efl_Promise *next;

   const void *data;
};

struct _Efl_Loop_Future_Data
{
   Eina_Inlist *callbacks;

   Efl_Future *self;
   Efl_Promise_Msg  *message;
   Efl_Promise_Data *promise;

#ifndef NDEBUG
   int wref;
#endif

   Eina_Bool fulfilled : 1;
   Eina_Bool death : 1;
   Eina_Bool delayed : 1;
};

static void
_efl_loop_future_success(Efl_Event *ev, Efl_Loop_Future_Data *pd, void *value)
{
   Efl_Loop_Future_Callback *cb;
   Efl_Future_Event_Success chain_success;

   ev->info = &chain_success;
   ev->desc = EFL_FUTURE_EVENT_SUCCESS;

   chain_success.value = value;

   EINA_INLIST_FREE(pd->callbacks, cb)
     {
        if (cb->next)
          {
             chain_success.next = cb->next;

             cb->success((void*) cb->data, ev);
          }

        pd->callbacks = eina_inlist_remove(pd->callbacks, pd->callbacks);
        free(cb);
     }
}

static void
_efl_loop_future_failure(Efl_Event *ev, Efl_Loop_Future_Data *pd, Eina_Error error)
{
   Efl_Loop_Future_Callback *cb;
   Efl_Future_Event_Failure chain_fail;

   ev->info = &chain_fail;
   ev->desc = EFL_FUTURE_EVENT_FAILURE;

   chain_fail.error = error;

   EINA_INLIST_FREE(pd->callbacks, cb)
     {
        if (cb->next)
          {
             chain_fail.next = cb->next;

             cb->failure((void*) cb->data, ev);
          }

        pd->callbacks = eina_inlist_remove(pd->callbacks, pd->callbacks);
        free(cb);
     }
}

static void
_efl_loop_future_propagate(Eo *obj, Efl_Loop_Future_Data *pd)
{
   Efl_Event ev;

   ev.object = obj;

   if (pd->fulfilled &&
       !pd->message)
     {
        _efl_loop_future_failure(&ev, pd, EINA_ERROR_FUTURE_CANCEL);
     }
   else if (pd->message->error == 0)
     {
        _efl_loop_future_success(&ev, pd, pd->message->value);
     }
   else
     {
        _efl_loop_future_failure(&ev, pd, pd->message->error);
     }
   pd->fulfilled = EINA_TRUE;

   if (!pd->delayed)
     {
        pd->delayed = EINA_TRUE;
        efl_unref(obj);
     }
}

static void
_efl_loop_future_fulfilling(Eo *obj, Efl_Loop_Future_Data *pd)
{
   if (pd->fulfilled)
     {
        _efl_loop_future_propagate(obj, pd);
     }

   if (!pd->death)
     {
        pd->death = EINA_TRUE;
        efl_del(obj);
     }
}

static void
_efl_loop_future_prepare_events(Efl_Loop_Future_Data *pd, Eina_Bool progress)
{
   if (!pd->promise) return ;

   pd->promise->set.future = EINA_TRUE;
   if (progress)
     pd->promise->set.progress = EINA_TRUE;
}

static Efl_Future *
_efl_loop_future_then(Eo *obj, Efl_Loop_Future_Data *pd,
                      Efl_Event_Cb success, Efl_Event_Cb failure, Efl_Event_Cb progress, const void *data)
{
   Efl_Loop_Future_Callback *cb;
   Efl_Future *f;

   cb = calloc(1, sizeof (Efl_Loop_Future_Callback));
   if (!cb) return NULL;

   cb->success = success;
   cb->failure = failure;
   cb->progress = progress;
   cb->data = data;
   cb->next = efl_add(EFL_PROMISE_CLASS, obj);

   f = efl_promise_future_get(cb->next);

   pd->callbacks = eina_inlist_append(pd->callbacks, EINA_INLIST_GET(cb));

   _efl_loop_future_prepare_events(pd, !!progress);
   _efl_loop_future_fulfilling(obj, pd);

   return f;
}

static void
_efl_loop_future_disconnect(Eo *obj, Efl_Loop_Future_Data *pd)
{
   Eo *promise;

   if (!pd->promise) return ;
   promise = efl_ref(pd->promise->promise);

   // Disconnect from the promise
   pd->promise->futures = eina_list_remove(pd->promise->futures, pd);

   // Notify that there is no more future
   if (!pd->promise->futures)
     {
        efl_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_NONE, NULL);
     }

   // Unreference after propagating the failure
   efl_data_xunref(pd->promise->promise, pd->promise, obj);
   pd->promise = NULL;

   efl_unref(promise);
}

static void
_efl_loop_future_cancel(Eo *obj, Efl_Loop_Future_Data *pd)
{
   // Check state
   if (pd->fulfilled)
     {
        ERR("Triggering cancel on an already fulfilled Efl.Future.");
        return;
     }

#ifndef NDEBUG
   if (!pd->wref)
     {
        WRN("Calling cancel should be only done on a weak reference. Look at efl_future_use.");
     }
#endif

   pd->fulfilled = EINA_TRUE;

   // Trigger failure
   _efl_loop_future_propagate(obj, pd);

   _efl_loop_future_disconnect(obj, pd);
}

static void
_efl_loop_future_intercept(Eo *obj)
{
   Efl_Loop_Future_Data *pd;

   // Just delay object death
   efl_del_intercept_set(obj, NULL);

   // Trigger events now
   pd = efl_data_scope_get(obj, EFL_LOOP_FUTURE_CLASS);

   if (!pd->promise) return ;
   if (pd->promise->set.future && !pd->promise->set.future_triggered)
     {
        efl_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_SET, obj);
        pd->promise->set.future_triggered = EINA_TRUE;
     }
   if (pd->promise->set.progress && !pd->promise->set.progress_triggered)
     {
        efl_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, obj);
        pd->promise->set.progress_triggered = EINA_TRUE;
     }
}

static Eo *
_efl_loop_future_efl_object_constructor(Eo *obj, Efl_Loop_Future_Data *pd)
{
   obj = efl_constructor(efl_super(obj, EFL_LOOP_FUTURE_CLASS));

   pd->self = obj;

   efl_del_intercept_set(obj, _efl_loop_future_intercept);

   return obj;
}

static void
_efl_loop_future_efl_object_destructor(Eo *obj, Efl_Loop_Future_Data *pd)
{
   if (!pd->fulfilled)
     {
        ERR("Lost reference to a future without fulfilling it. Forcefully cancelling it.");
        _efl_loop_future_propagate(obj, pd);
     }
#ifndef NDEBUG
   else if (pd->callbacks)
     {
        ERR("Found referenced callbacks while destroying the future.");
        _efl_loop_future_propagate(obj, pd);
     }
#endif

   // Cleanup message if needed
   if (pd->message)
     {
        EINA_REFCOUNT_UNREF(pd->message)
          _efl_promise_msg_free(pd->message);
        pd->message = NULL;
     }

   efl_destructor(efl_super(obj, EFL_LOOP_FUTURE_CLASS));

   // Disconnect from the promise
   _efl_loop_future_disconnect(obj, pd);
}

#ifndef NDEBUG
static void
_efl_future_wref_add(Eo *obj, Efl_Loop_Future_Data *pd, Eo **wref)
{
   efl_wref_add(efl_super(obj, EFL_LOOP_FUTURE_CLASS), wref);

   pd->wref++;
}

static void
_efl_future_wref_del(Eo *obj, Efl_Loop_Future_Data *pd, Eo **wref)
{
   efl_wref_del(efl_super(obj, EFL_LOOP_FUTURE_CLASS), wref);

   pd->wref--;
}
#endif

static Eina_Bool
_efl_loop_future_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
                  EFL_OBJECT_OP_FUNC(efl_future_then, _efl_loop_future_then),
                  EFL_OBJECT_OP_FUNC(efl_future_cancel, _efl_loop_future_cancel),
                  EFL_OBJECT_OP_FUNC(efl_constructor, _efl_loop_future_efl_object_constructor),
                  EFL_OBJECT_OP_FUNC(efl_destructor, _efl_loop_future_efl_object_destructor));

   return efl_class_functions_set(klass, &ops);
};

static const Efl_Class_Description _efl_loop_future_class_desc = {
  EO_VERSION,
  "Efl_Future",
  EFL_CLASS_TYPE_REGULAR,
  sizeof (Efl_Loop_Future_Data),
  _efl_loop_future_class_initializer,
  NULL,
  NULL
};

EFL_DEFINE_CLASS(efl_loop_future_class_get, &_efl_loop_future_class_desc, EFL_FUTURE_CLASS, NULL);

static Efl_Future *
_efl_promise_future_get(Eo *obj, Efl_Promise_Data *pd EINA_UNUSED)
{
   Efl_Future *f;
   Efl_Loop_Future_Data *fd;

   // Build a new future, attach it and return it
   f = efl_add(EFL_LOOP_FUTURE_CLASS, NULL);
   if (!f) return NULL;

   fd = efl_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);

   fd->promise = efl_data_xref(obj, EFL_PROMISE_CLASS, f);
   fd->promise->futures = eina_list_append(fd->promise->futures, fd);

   return f;
}

static Efl_Promise_Msg *
_efl_promise_message_new(Efl_Promise_Data *pd)
{
   Efl_Promise_Msg *message;

   message = calloc(1, sizeof (Efl_Promise_Msg));
   if (!message) return NULL;

   EINA_REFCOUNT_INIT(message);
   pd->message = message;

   return message;
}

static void
_efl_promise_value_set(Eo *obj, Efl_Promise_Data *pd, void *v, Eina_Free_Cb free_cb)
{
   Efl_Promise_Msg *message;
   Efl_Loop_Future_Data *f;
   Eina_List *l, *ln;

   if (pd->message)
     {
        ERR("This promise has already been fulfilled. You can can't set a value twice nor can you set a value after it has been cancelled.");
        return ;
     }

   // By triggering this message, we are likely going to kill all future
   // And a user of the promise may want to attach an event handler on the promise
   // and destroy it, so delay that to after the loop is done.
   efl_ref(obj);

   // Create a refcounted structure where refcount == number of future + one
   message = _efl_promise_message_new(pd);
   if (!message) return ;

   message->value = v;
   message->free_cb = free_cb;

   EINA_REFCOUNT_INIT(message);
   pd->message = message;

   // Send it to all futures
   EINA_LIST_FOREACH_SAFE(pd->futures, l, ln, f)
     {
        EINA_REFCOUNT_REF(message);
        f->message = message;

        // Trigger the callback
        _efl_loop_future_propagate(f->self, f);
     }

   // Now, we may die.
   efl_unref(obj);
}

static void
_efl_promise_failed(Eo *obj, Efl_Promise_Data *pd, Eina_Error err)
{
   Efl_Promise_Msg *message;
   Efl_Loop_Future_Data *f;
   Eina_List *l, *ln;

   if (pd->message)
     {
        ERR("This promise has already been fulfilled. You can can't set a value twice nor can you set a value after it has been cancelled.");
        return ;
     }

   // By triggering this message, we are likely going to kill all future
   // And a user of the promise may want to attach an event handler on the promise
   // and destroy it, so delay that to after the loop is done.
   efl_ref(obj);

   // Create a refcounted structure where refcount == number of future + one
   message = _efl_promise_message_new(pd);
   if (!message) return ;

   message->error = err;

   EINA_REFCOUNT_INIT(message);
   pd->message = message;

   // Send it to each future
   EINA_LIST_FOREACH_SAFE(pd->futures, l, ln, f)
     {
        EINA_REFCOUNT_REF(message);
        f->message = message;

        // Trigger the callback
        _efl_loop_future_propagate(f->self, f);
     }

   // Now, we may die.
   efl_unref(obj);
}

static void
_efl_promise_progress_set(Eo *obj, Efl_Promise_Data *pd, void *p)
{
   Efl_Loop_Future_Data *f;
   Eina_List *l, *ln;
   Efl_Future_Event_Progress chain_progress;
   Efl_Event ev;

   chain_progress.progress = p;

   ev.object = obj;
   ev.info = &chain_progress;
   ev.desc = EFL_FUTURE_EVENT_PROGRESS;

   EINA_LIST_FOREACH_SAFE(pd->futures, l, ln, f)
     {
        Efl_Loop_Future_Callback *cb;

        EINA_INLIST_FOREACH(f->callbacks, cb)
          {
             if (cb->next)
               {
                  chain_progress.next = cb->next;

                  cb->progress((void*) cb->data, &ev);
               }
          }
     }
}

static Efl_Object *
_efl_promise_efl_object_constructor(Eo *obj, Efl_Promise_Data *pd)
{
   pd->promise = obj;

   return efl_constructor(efl_super(obj, EFL_PROMISE_CLASS));
}

static void
_efl_promise_efl_object_destructor(Eo *obj, Efl_Promise_Data *pd)
{
   // Unref refcounted structure
   if (!pd->message && pd->futures)
     {
        ERR("This promise has not been fulfilled. Forcefully cancelling %p.", obj);
        efl_promise_failed(obj, EINA_ERROR_FUTURE_CANCEL);
     }

   if (pd->message)
     {
        EINA_REFCOUNT_UNREF(pd->message)
          _efl_promise_msg_free(pd->message);
        pd->message = NULL;
     }

   efl_destructor(efl_super(obj, EFL_PROMISE_CLASS));
}

#include "efl_promise.eo.c"
