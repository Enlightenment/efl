#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_private.h"

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

   unsigned char propagating;

   struct {
      Eina_Bool future : 1;
      Eina_Bool future_triggered : 1;
      Eina_Bool progress : 1;
      Eina_Bool progress_triggered : 1;
   } set;

   Eina_Bool optional : 1;
   Eina_Bool propagated : 1;
   Eina_Bool nodelay : 1;
};

static void
_efl_promise_msg_free(Efl_Promise_Msg *msg)
{
   if (!msg) return ;
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
   Efl_Loop *loop;
   Efl_Promise_Msg  *message;
   Efl_Promise_Data *promise;

#ifndef NDEBUG
   int wref;
#endif
   unsigned char propagating;

   Eina_Bool fulfilled : 1;
   Eina_Bool death : 1;
   Eina_Bool delayed : 1;
   Eina_Bool optional : 1;
};

static void
_efl_loop_future_success(Efl_Event *ev, Efl_Loop_Future_Data *pd, void *value)
{
   Efl_Loop_Future_Callback *cb;
   Efl_Future_Event_Success chain_success;

   ev->info = &chain_success;
   ev->desc = EFL_FUTURE_EVENT_SUCCESS;

   chain_success.value = value;

   pd->propagating++;
   // This is done on purpose, we are using cb as a reminder to the head of the list
   while ((cb = (void*) pd->callbacks))
     {
        // Remove callback early to avoid double execution while
        // doing recursive call
        pd->callbacks = eina_inlist_remove(pd->callbacks, pd->callbacks);

        if (cb->next)
          {
             chain_success.next = cb->next;

             if (cb->success) cb->success((void*) cb->data, ev);
          }
        else
          {
             chain_success.next = NULL;
             chain_success.value = pd->message;
             if (cb->success) cb->success((void*) cb->data, ev);
             chain_success.value = value;
          }

        free(cb);
     }
   pd->propagating--;
}

static void
_efl_loop_future_failure(Efl_Event *ev, Efl_Loop_Future_Data *pd, Eina_Error error)
{
   Efl_Loop_Future_Callback *cb;
   Efl_Future_Event_Failure chain_fail;

   ev->info = &chain_fail;
   ev->desc = EFL_FUTURE_EVENT_FAILURE;

   chain_fail.error = error;

   pd->propagating++;
   // This is done on purpose, we are using cb as a reminder to the head of the list
   while ((cb = (void*) pd->callbacks))
     {
        // Remove callback early to avoid double execution while
        // doing recursive call
        pd->callbacks = eina_inlist_remove(pd->callbacks, pd->callbacks);

        chain_fail.next = cb->next;

        if (cb->failure) cb->failure((void*) cb->data, ev);

        free(cb);
     }
   pd->propagating--;
}

static void
_efl_loop_future_propagate_force(Eo *obj, Efl_Loop_Future_Data *pd)
{
   Efl_Event ev;

   ev.object = obj;

   if (pd->promise->message->error == 0)
     {
        _efl_loop_future_success(&ev, pd, pd->message->value);
     }
   else
     {
        _efl_loop_future_failure(&ev, pd, pd->message->error);
     }
}

static void
_efl_loop_future_propagate(Eo *obj, Efl_Loop_Future_Data *pd)
{
   Efl_Event ev;
   Eina_Bool cancel;

   ev.object = obj;
   cancel = pd->fulfilled && !pd->message;

   // This has to be done early on to avoid recursive success/failure to
   // bypass the fulfilled check.
   pd->fulfilled = EINA_TRUE;

   if (cancel)
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
_efl_loop_future_prepare_events(Efl_Loop_Future_Data *pd, Eina_Bool progress, Eina_Bool optional)
{
   if (!pd->promise) return ;

   if (pd->optional)
     {
        pd->optional = EINA_FALSE;
        ecore_loop_future_unregister(efl_provider_find(pd->self, EFL_LOOP_CLASS), pd->self);
     }

   if (!optional)
     {
        pd->promise->set.future = EINA_TRUE;
        if (progress)
          pd->promise->set.progress = EINA_TRUE;
     }
}

static Efl_Loop_Future_Callback *
_efl_loop_future_then_internal(Efl_Loop_Future_Data *pd,
                               Efl_Event_Cb success, Efl_Event_Cb failure, Efl_Event_Cb progress, const void *data)
{
   Efl_Loop_Future_Callback *cb;

   cb = calloc(1, sizeof (Efl_Loop_Future_Callback));
   if (!cb) return NULL;

   cb->success = success;
   cb->failure = failure;
   cb->progress = progress;
   cb->data = data;
   pd->callbacks = eina_inlist_append(pd->callbacks, EINA_INLIST_GET(cb));

   return cb;
}

static Efl_Future *
_efl_loop_future_then(Eo *obj, Efl_Loop_Future_Data *pd,
                      Efl_Event_Cb success, Efl_Event_Cb failure, Efl_Event_Cb progress, const void *data)
{
   Efl_Loop_Future_Callback *cb;
   Efl_Promise_Data *epd;
   Efl_Future *f;

   cb = _efl_loop_future_then_internal(pd, success, failure, progress, data);
   if (!cb) return NULL;

   efl_ref(obj);

   cb->next = efl_add(EFL_PROMISE_CLASS, obj);
   epd = efl_data_scope_get(cb->next, EFL_PROMISE_CLASS);
   epd->optional = EINA_TRUE;

   f = efl_promise_future_get(cb->next);

   _efl_loop_future_prepare_events(pd, !!progress, EINA_FALSE);
   _efl_loop_future_fulfilling(obj, pd);

   efl_unref(obj);

   return f;
}

static Eina_Bool
_efl_loop_future_internal_then(Efl_Future *f,
                               Efl_Event_Cb success, Efl_Event_Cb failure, Efl_Event_Cb progress, const void *data)
{
   Efl_Loop_Future_Data *pd = efl_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);
   Efl_Loop_Future_Callback *cb;

   cb = _efl_loop_future_then_internal(pd, success, failure, progress, data);
   if (!cb) return EINA_FALSE;

   efl_ref(f);

   _efl_loop_future_prepare_events(pd, !!progress, EINA_TRUE);
   _efl_loop_future_fulfilling(f, pd);

   efl_unref(f);

   return EINA_TRUE;
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
   if (!pd->promise->futures &&
       !pd->promise->message)
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
   // We do allow for calling cancel during the propagation phase
   // as the other proper fix is to wype out all future reference before
   // starting propagating things.
   if (pd->propagating ||
       (pd->promise && pd->promise->propagating))
     {
        efl_ref(obj);

        _efl_loop_future_propagate_force(obj, pd);

        goto disconnect;
     }

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
   efl_ref(obj);

   _efl_loop_future_propagate(obj, pd);

   _efl_loop_future_disconnect(obj, pd);

 disconnect:
   efl_unref(obj);
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

   pd->loop = efl_ref(efl_provider_find(obj, EFL_LOOP_CLASS));

   pd->self = obj;
   pd->optional = EINA_TRUE;

   ecore_loop_future_register(efl_provider_find(obj, EFL_LOOP_CLASS), obj);

   efl_del_intercept_set(obj, _efl_loop_future_intercept);

   return obj;
}

static void
_efl_loop_future_efl_object_destructor(Eo *obj, Efl_Loop_Future_Data *pd)
{
   Eo *promise = NULL;

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

   // Stop the main loop handler that would destroy this optional future
   if (pd->optional)
     {
        pd->optional = EINA_FALSE;
        ecore_loop_future_unregister(efl_provider_find(pd->self, EFL_LOOP_CLASS), pd->self);
     }

   if (pd->promise) promise = efl_ref(pd->promise->promise);

   efl_destructor(efl_super(obj, EFL_LOOP_FUTURE_CLASS));

   // Disconnect from the promise
   _efl_loop_future_disconnect(obj, pd);

   efl_unref(promise);

   efl_unref(pd->loop);
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

static Efl_Object *
_efl_loop_future_efl_object_provider_find(Eo *obj EINA_UNUSED, Efl_Loop_Future_Data *pd, const Efl_Object *klass)
{
   Efl_Object *r = NULL;

   if (pd->loop) r = efl_provider_find(pd->loop, klass);
   if (r) return r;

   return efl_provider_find(efl_super(obj, EFL_LOOP_FUTURE_CLASS), klass);
}

static Eina_Bool
_efl_loop_future_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
#ifndef NDEBUG
                  EFL_OBJECT_OP_FUNC(efl_wref_add, _efl_future_wref_add),
                  EFL_OBJECT_OP_FUNC(efl_wref_del, _efl_future_wref_del),
#endif
                  EFL_OBJECT_OP_FUNC(efl_provider_find, _efl_loop_future_efl_object_provider_find),
                  EFL_OBJECT_OP_FUNC(efl_future_then, _efl_loop_future_then),
                  EFL_OBJECT_OP_FUNC(efl_future_cancel, _efl_loop_future_cancel),
                  EFL_OBJECT_OP_FUNC(efl_constructor, _efl_loop_future_efl_object_constructor),
                  EFL_OBJECT_OP_FUNC(efl_destructor, _efl_loop_future_efl_object_destructor));

   return efl_class_functions_set(klass, &ops, NULL);
};

static const Efl_Class_Description _efl_loop_future_class_desc = {
  EO_VERSION,
  "Efl.Future",
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
   f = efl_add(EFL_LOOP_FUTURE_CLASS, obj);
   if (!f) return NULL;

   fd = efl_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);

   fd->promise = efl_data_xref(obj, EFL_PROMISE_CLASS, f);
   fd->promise->futures = eina_list_append(fd->promise->futures, fd);

   // The promise has already been fulfilled, prepare the propagation
   if (fd->promise->message &&
       fd->promise->propagated)
     {
        fd->message = fd->promise->message;
        EINA_REFCOUNT_REF(fd->message);

        _efl_loop_future_propagate(f, fd);
     }

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
_efl_promise_propagate(Eo *obj, Efl_Promise_Data *pd)
{
   Efl_Loop_Future_Data *f;
   Eina_List *l, *ln;

   pd->propagated = EINA_TRUE;

   // By triggering this message, we are likely going to kill all future
   // And a user of the promise may want to attach an event handler on the promise
   // and destroy it, so delay that to after the loop is done.
   efl_ref(obj);

   // Send it to all futures
   pd->propagating++;
   EINA_LIST_FOREACH_SAFE(pd->futures, l, ln, f)
     {
        EINA_REFCOUNT_REF(pd->message);
        f->message = pd->message;

        // Trigger the callback
        _efl_loop_future_propagate(f->self, f);
     }
   pd->propagating--;

   // Now, we may die.
   efl_unref(obj);
}

void
ecore_loop_promise_fulfill(Eo *obj)
{
   Efl_Promise_Data *pd = efl_data_scope_get(obj, EFL_PROMISE_CLASS);

   _efl_promise_propagate(obj, pd);
}

static void
_efl_promise_value_set(Eo *obj, Efl_Promise_Data *pd, void *v, Eina_Free_Cb free_cb)
{
   Efl_Promise_Msg *message;

   if (pd->message)
     {
        ERR("This promise has already been fulfilled. You can can't set a value twice nor can you set a value after it has been cancelled.");
        return ;
     }

   // Create a refcounted structure where refcount == number of future + one
   message = _efl_promise_message_new(pd);
   if (!message) return ;

   message->value = v;
   message->free_cb = free_cb;

   EINA_REFCOUNT_INIT(message);
   pd->message = message;

   if (pd->nodelay)
     _efl_promise_propagate(obj, pd);
   else
     ecore_loop_promise_register(efl_provider_find(obj, EFL_LOOP_CLASS), obj);
}

static void
_efl_promise_failed_set(Eo *obj, Efl_Promise_Data *pd, Eina_Error err)
{
   Efl_Promise_Msg *message;

   if (pd->message)
     {
        ERR("This promise has already been fulfilled. You can can't set a value twice nor can you set a value after it has been cancelled.");
        return ;
     }
   // Create a refcounted structure where refcount == number of future + one
   message = _efl_promise_message_new(pd);
   if (!message) return ;

   message->error = err;

   EINA_REFCOUNT_INIT(message);
   pd->message = message;

   if (pd->nodelay)
     _efl_promise_propagate(obj, pd);
   else
     ecore_loop_promise_register(efl_provider_find(obj, EFL_LOOP_CLASS), obj);
}

static void
_efl_promise_progress_set(Eo *obj, Efl_Promise_Data *pd, const void *p)
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

                  if (cb->progress) cb->progress((void*) cb->data, &ev);
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
   pd->nodelay = EINA_TRUE;

   // Unref refcounted structure
   if (!pd->message && pd->futures)
     {
        if (!pd->optional) ERR("This promise has not been fulfilled. Forcefully cancelling %p.", obj);
        efl_promise_failed_set(obj, EINA_ERROR_FUTURE_CANCEL);
     }

   if (pd->message &&
       !pd->propagated)
     {
        ecore_loop_promise_unregister(efl_provider_find(obj, EFL_LOOP_CLASS), obj);
        _efl_promise_propagate(obj, pd);
     }

   if (pd->message)
     {
        EINA_REFCOUNT_UNREF(pd->message)
          _efl_promise_msg_free(pd->message);
        pd->message = NULL;
     }

   efl_destructor(efl_super(obj, EFL_PROMISE_CLASS));
}

static void
_efl_promise_connect_then(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *success = ev->info;
   Efl_Promise_Data *pd = data;
   // This is a trick due to the fact we are using internal function call to register this functions
   Efl_Promise_Msg *d = success->value;

   EINA_REFCOUNT_REF(d);
   pd->message = d;

   _efl_promise_propagate(pd->promise, pd);

   efl_unref(pd->promise);
}

static void
_efl_promise_connect_fail(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Failure *fail = ev->info;
   Efl_Promise_Data *pd = data;

   efl_promise_failed_set(pd->promise, fail->error);

   efl_unref(pd->promise);
}

static Eina_Bool
_efl_promise_connect(Eo *obj, Efl_Promise_Data *pd, Efl_Future *f)
{
   // We have to keep a reference on the promise to avoid it dying before the future
   efl_ref(obj);

   return _efl_loop_future_internal_then(f, _efl_promise_connect_then, _efl_promise_connect_fail, NULL, pd);
}

typedef struct _Efl_Promise_Composite Efl_Promise_All;
typedef struct _Efl_Future_All Efl_Future_All;
typedef struct _Efl_Accessor_All Efl_Accessor_All;

typedef struct _Efl_Promise_Composite Efl_Promise_Race;

struct _Efl_Accessor_All
{
   Eina_Accessor accessor;

   Efl_Promise_All *all;
   Efl_Promise *promise;
};

struct _Efl_Future_All
{
   Efl_Future *f;

   Efl_Promise_Msg *d;
};

struct _Efl_Promise_Composite
{
   Eina_Array members;

   Efl_Promise *promise;

   Efl_Future *(*future_get)(void *item);

   Eina_Error error;

   Eina_Bool failed : 1;
   Eina_Bool progress_triggered : 1;
   Eina_Bool future_triggered : 1;
   Eina_Bool done : 1;
   Eina_Bool building : 1;
};

static Efl_Future *
_efl_promise_all_future_get(void *item)
{
   Efl_Future_All *fa = item;

   return fa->f;
}

static Efl_Future *
_efl_promise_race_future_get(void *item)
{
   return item;
}

static void
_efl_promise_all_free(Efl_Promise_All *all)
{
   Efl_Future_All *fa;
   Eina_Array_Iterator iterator;
   unsigned int i;

   EINA_ARRAY_ITER_NEXT(&all->members, i, fa, iterator)
     {
        if (fa->d)
          EINA_REFCOUNT_UNREF(fa->d)
            _efl_promise_msg_free(fa->d);
     }
   efl_del(all->promise);
   all->promise = NULL;
}

static void
_efl_promise_all_die(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Promise_All *all = data;
   Efl_Future_All *fa;

   while ((fa = eina_array_pop(&all->members)))
     {
        EINA_REFCOUNT_UNREF(fa->d)
          _efl_promise_msg_free(fa->d);
        assert(fa->f == NULL);
        free(fa);
     }
   eina_array_flush(&all->members);
   free(all);
}

static void
_efl_promise_race_die(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Promise_All *all = data;

   eina_array_flush(&all->members);
   free(all);
}

static void
_efl_future_set(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Promise_All *all = data;
   void *item;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (all->future_triggered) return ;
   all->future_triggered = EINA_TRUE;

   // Propagate set on demand
   EINA_ARRAY_ITER_NEXT(&all->members, i, item, iterator)
     {
        Efl_Future *f = all->future_get(item);

        if (f)
          {
             Efl_Loop_Future_Data *pd = efl_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);

             if (!pd->promise->set.future && !pd->promise->set.future_triggered)
               {
                  efl_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_SET, f);
                  pd->promise->set.future_triggered = EINA_TRUE;
                  pd->promise->set.future = EINA_TRUE;
               }
          }
     }
}

static void
_efl_future_progress_set(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Promise_All *all = data;
   void *item;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (all->progress_triggered) return ;
   all->progress_triggered = 1;

   // Propagate progress set
   EINA_ARRAY_ITER_NEXT(&all->members, i, item, iterator)
     {
        Efl_Future *f = all->future_get(item);

        if (f)
          {
             Efl_Loop_Future_Data *pd = efl_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);

             if (!pd->promise->set.progress && !pd->promise->set.progress_triggered)
               {
                  efl_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, f);
                  pd->promise->set.progress_triggered = EINA_TRUE;
                  pd->promise->set.progress = EINA_TRUE;
               }
          }
     }
}

static void
_efl_all_future_none(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Promise_All *all = data;
   Efl_Future_All *fa;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (all->failed) return ;
   all->failed = EINA_TRUE;

   // Trigger cancel on all future
   EINA_ARRAY_ITER_NEXT(&all->members, i, fa, iterator)
     {
        if (!fa->d && fa->f) efl_future_cancel(fa->f);
     }

   // No one is listening to this promise anyway
   _efl_promise_all_free(all);
}

static void
_efl_race_future_none(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Promise_Race *all = data;
   Efl_Future *f;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (all->failed) return ;
   all->failed = EINA_TRUE;

   // Trigger cancel on all future
   EINA_ARRAY_ITER_NEXT(&all->members, i, f, iterator)
     {
        efl_future_cancel(f);
     }

   // No one is listening to this promise anyway
   _efl_promise_all_free(all);
}

static Eina_Bool
_efl_accessor_all_get_at(Efl_Accessor_All *ac, unsigned int pos, void **data)
{
   Efl_Future_All *fa;

   if (eina_array_count(&ac->all->members) <= pos)
     return EINA_FALSE;

   fa = eina_array_data_get(&ac->all->members, pos);
   if (!fa) return EINA_FALSE;

   *data = fa->d->value;
   return EINA_TRUE;
}

static Efl_Promise *
_efl_accessor_all_get_container(Efl_Accessor_All *ac)
{
   return ac->all->promise;
}

static void
_efl_accessor_all_free(Efl_Accessor_All *ac)
{
   _efl_promise_all_free(ac->all);
   efl_unref(ac->promise);
   free(ac);
}

static void
_real_then_all(Efl_Promise_All *all)
{
   Efl_Accessor_All *ac;

   ac = calloc(1, sizeof (Efl_Accessor_All));
   if (!ac) return ; // We do now the promise and all here

   EINA_MAGIC_SET(&ac->accessor, EINA_MAGIC_ACCESSOR);

   ac->accessor.version = EINA_ACCESSOR_VERSION;
   ac->accessor.get_at = FUNC_ACCESSOR_GET_AT(_efl_accessor_all_get_at);
   ac->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(_efl_accessor_all_get_container);
   ac->accessor.free = FUNC_ACCESSOR_FREE(_efl_accessor_all_free);
   ac->all = all;
   ac->promise = efl_ref(all->promise);

   efl_promise_value_set(all->promise, &ac->accessor, EINA_FREE_CB(eina_accessor_free));
}

static void
_then_all(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *success = ev->info;
   Efl_Promise_All *all = data;
   Efl_Future_All *fa;
   // This is a trick due to the fact we are using internal function call to register this functions
   Efl_Promise_Msg *d = success->value;
   Eina_Array_Iterator iterator;
   unsigned int i;
   Eina_Bool done = EINA_TRUE;

   // Only when all value are received can we propagate the success
   EINA_ARRAY_ITER_NEXT(&all->members, i, fa, iterator)
     {
        if (fa->f == ev->object)
          {
             fa->d = d;
             EINA_REFCOUNT_REF(fa->d);
          }
        done &= !!fa->d;
     }

   if (all->building) return ;

   if (done)
     {
        _real_then_all(all);
     }
}

static void
_real_fail_all(Efl_Promise_All *all, Eina_Error error)
{
   efl_promise_failed_set(all->promise, error);
   _efl_promise_all_free(all);
}

static void
_fail_all(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Failure *fail = ev->info;
   Efl_Promise_All *all = data;
   Efl_Future_All *fa;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (all->failed) return ;
   all->failed = EINA_TRUE;

   if (all->building)
     {
        all->error = fail->error;
        return ;
     }

   efl_ref(all->promise);

   // In case of one fail, the entire promise will fail and
   // all remaining future will be cancelled
   EINA_ARRAY_ITER_NEXT(&all->members, i, fa, iterator)
     {
        if (fa->d)
          {
             EINA_REFCOUNT_UNREF(fa->d)
               _efl_promise_msg_free(fa->d);
             fa->d = NULL;
          }
        else
          {
             efl_future_cancel(fa->f);
          }
     }

   _real_fail_all(all, fail->error);
   efl_unref(all->promise);
}

static void
_progress(void *data, const Efl_Event *ev)
{
   Efl_Promise_All *all = data;
   Efl_Future_Event_Progress *p = ev->info;
   void *item;
   Efl_Future_All_Progress a;
   Eina_Array_Iterator iterator;
   unsigned int i;

   a.inprogress = ev->object;
   a.progress = (Efl_Object *) p->progress;

   EINA_ARRAY_ITER_NEXT(&all->members, i, item, iterator)
     {
        Efl_Future *f = all->future_get(item);

        if (f == a.inprogress) break ;
     }
   a.index = i;

   efl_promise_progress_set(all->promise, &a);
}

EFL_CALLBACKS_ARRAY_DEFINE(efl_all_callbacks,
                           { EFL_PROMISE_EVENT_FUTURE_SET, _efl_future_set },
                           { EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, _efl_future_progress_set },
                           { EFL_PROMISE_EVENT_FUTURE_NONE, _efl_all_future_none },
                           { EFL_EVENT_DEL, _efl_promise_all_die });

static inline Efl_Promise_All *
_efl_future_all_new(Eo *provider)
{
   Efl_Promise_All *all;
   Eo *loop;

   loop = efl_provider_find(provider, EFL_LOOP_CLASS);
   if (!loop) return NULL;

   all = calloc(1, sizeof (Efl_Promise_All));
   if (!all) return NULL;

   eina_array_step_set(&all->members, sizeof (Eina_Array), 8);
   all->future_get = _efl_promise_all_future_get;
   all->promise = efl_add(EFL_PROMISE_CLASS, loop);
   if (!all->promise) goto on_error;

   all->building = EINA_TRUE;

   return all;

 on_error:
   free(all);
   return NULL;
}

static inline Efl_Future *
_efl_future_all_done(Efl_Promise_All *all)
{
   Efl_Future *r;
   Efl_Future_All *fa;
   Eina_Array_Iterator iterator;
   unsigned int i;

   EINA_ARRAY_ITER_NEXT(&all->members, i, fa, iterator)
     _efl_loop_future_internal_then(fa->f, _then_all, _fail_all, _progress, all);

   efl_event_callback_array_add(all->promise, efl_all_callbacks(), all);

   r = efl_promise_future_get(all->promise);

   all->building = EINA_FALSE;

   if (all->failed)
     {
        _real_fail_all(all, all->error);
     }
   else
     {
        Eina_Bool done = EINA_TRUE;

        EINA_ARRAY_ITER_NEXT(&all->members, i, fa, iterator)
          done &= !!fa->d;

        if (done) _real_then_all(all);
     }

   return r;
}

static Eina_Bool
_efl_future_all_append(Efl_Promise_All *all, Efl_Future *fn)
{
   Efl_Future_All *fa;

   fa = calloc(1, sizeof (Efl_Future_All));
   if (!fa) return EINA_FALSE;
   efl_future_use(&fa->f, fn);
   eina_array_push(&all->members, fa);

   return EINA_TRUE;
}

EAPI Efl_Future *
efl_future_all_internal(Efl_Future *f1, ...)
{
   Efl_Promise_All *all;
   Efl_Future *fn;
   va_list args;

   if (!f1) return NULL;

   all = _efl_future_all_new(f1);
   if (!all) return NULL;

   if (!_efl_future_all_append(all, f1))
     goto on_error;

   va_start(args, f1);
   while ((fn = va_arg(args, Efl_Future *)))
     {
        if (!_efl_future_all_append(all, fn))
          {
             va_end(args);
             goto on_error;
          }
     }
   va_end(args);

   return _efl_future_all_done(all);

 on_error:
   _efl_promise_all_die(all, NULL);
   return NULL;
}

EAPI Efl_Future *
efl_future_iterator_all(Eina_Iterator *it)
{
   Efl_Promise_All *all = NULL;
   Efl_Future *fn;

   if (!it) return NULL;

   EINA_ITERATOR_FOREACH(it, fn)
     {
        if (!all) all = _efl_future_all_new(fn);
        if (!all) goto on_error;
        if (!_efl_future_all_append(all, fn))
          goto on_error;
     }
   eina_iterator_free(it);

   return _efl_future_all_done(all);

 on_error:
   eina_iterator_free(it);
   if (all) _efl_promise_all_die(all, NULL);
   return NULL;
}

static void
_then_race(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Success *ev_success = ev->info;
   Efl_Promise_Race *race = data;
   // This is a trick due to the fact we are using internal function call to register this functions
   Efl_Promise_Msg *d = ev_success->value;
   Efl_Future *f;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (race->done) return ;
   race->done = EINA_TRUE;

   efl_ref(race->promise);

   EINA_ARRAY_ITER_NEXT(&race->members, i, f, iterator)
     {
        // To avoid double cancel/success
        eina_array_data_set(&race->members, i, NULL);

        if (f == ev->object)
          {
             Efl_Future_Race_Success *success = calloc(1, sizeof (Efl_Future_Race_Success));

             if (!success) continue ;
             success->winner = f;
             success->value = d->value;
             success->index = i;

             efl_promise_value_set(race->promise, success, free);
          }
        else
          {
             efl_future_cancel(f);
          }
     }

   _efl_promise_all_free(race);
   efl_unref(race->promise);
}

static void
_fail_race(void *data, const Efl_Event *ev)
{
   Efl_Future_Event_Failure *fail = ev->info;
   Efl_Promise_Race *race = data;
   Efl_Future *f;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (race->done) return ;
   race->done = EINA_TRUE;

   efl_ref(race->promise);

   EINA_ARRAY_ITER_NEXT(&race->members, i, f, iterator)
     {
        // To avoid double cancel/success
        eina_array_data_set(&race->members, i, NULL);

        if (f != ev->object)
          {
             efl_future_cancel(f);
          }
     }

   efl_promise_failed_set(race->promise, fail->error);
   _efl_promise_all_free(race);
   efl_unref(race->promise);
}

EFL_CALLBACKS_ARRAY_DEFINE(efl_race_callbacks,
                           { EFL_PROMISE_EVENT_FUTURE_SET, _efl_future_set },
                           { EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, _efl_future_progress_set },
                           { EFL_PROMISE_EVENT_FUTURE_NONE, _efl_race_future_none },
                           { EFL_EVENT_DEL, _efl_promise_race_die });

static Efl_Promise_Race *
_efl_future_race_new(Eo *provider)
{
   Efl_Promise_Race *race;
   Eo *loop;

   loop = efl_provider_find(provider, EFL_LOOP_CLASS);
   if (!loop) return NULL;

   race = calloc(1, sizeof (Efl_Promise_Race));
   if (!race) return NULL;

   eina_array_step_set(&race->members, sizeof (Eina_Array), 8);
   race->future_get = _efl_promise_race_future_get;
   race->promise = efl_add(EFL_PROMISE_CLASS, loop);
   if (!race->promise) goto on_error;

   return race;

 on_error:
   free(race);
   return NULL;
}

static inline Efl_Future *
_efl_future_race_done(Efl_Promise_Race *race)
{
   Efl_Future *r;
   Efl_Future *fn;
   Eina_Array_Iterator iterator;
   unsigned int i;

   r = efl_promise_future_get(race->promise);

   EINA_ARRAY_ITER_NEXT(&race->members, i, fn, iterator)
     _efl_loop_future_internal_then(fn, _then_race, _fail_race, _progress, race);

   efl_event_callback_array_add(race->promise, efl_race_callbacks(), race);

   return r;
}

EAPI Efl_Future *
efl_future_race_internal(Efl_Future *f1, ...)
{
   Efl_Promise_Race *race;
   Efl_Future *fn;
   va_list args;

   if (!f1) return NULL;

   race = _efl_future_race_new(f1);
   if (!race) return NULL;

   eina_array_push(&race->members, f1);

   va_start(args, f1);

   while ((fn = va_arg(args, Efl_Future *)))
     {
        eina_array_push(&race->members, fn);
     }

   return _efl_future_race_done(race);
}

EAPI Efl_Future *
efl_future_iterator_race(Eina_Iterator *it)
{
   Efl_Promise_Race *race = NULL;
   Efl_Future *fn;

   if (!it) return NULL;

   EINA_ITERATOR_FOREACH(it, fn)
     {
        if (!race) race = _efl_future_race_new(fn);
        if (!race) goto on_error;
        eina_array_push(&race->members, fn);
     }
   eina_iterator_free(it);

   return _efl_future_race_done(race);

 on_error:
   eina_iterator_free(it);
   return NULL;
}

#include "efl_promise.eo.c"
