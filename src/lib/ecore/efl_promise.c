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
static const Eo_Class *efl_loop_future_class_get(void);

typedef struct _Efl_Loop_Future_Data Efl_Loop_Future_Data;
typedef struct _Efl_Loop_Future_Callback Efl_Loop_Future_Callback;

struct _Efl_Loop_Future_Callback
{
   EINA_INLIST;

   Eo_Event_Cb success;
   Eo_Event_Cb failure;
   Eo_Event_Cb progress;

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
   Eina_Bool optional : 1;
};

static void
_efl_loop_future_success(Eo_Event *ev, Efl_Loop_Future_Data *pd, void *value)
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
        else
          {
             chain_success.next = NULL;
             chain_success.value = pd->message;
             cb->success((void*) cb->data, ev);
             chain_success.value = value;
          }

        pd->callbacks = eina_inlist_remove(pd->callbacks, pd->callbacks);
        free(cb);
     }
}

static void
_efl_loop_future_failure(Eo_Event *ev, Efl_Loop_Future_Data *pd, Eina_Error error)
{
   Efl_Loop_Future_Callback *cb;
   Efl_Future_Event_Failure chain_fail;

   ev->info = &chain_fail;
   ev->desc = EFL_FUTURE_EVENT_FAILURE;

   chain_fail.error = error;

   EINA_INLIST_FREE(pd->callbacks, cb)
     {
        chain_fail.next = cb->next;

        cb->failure((void*) cb->data, ev);

        pd->callbacks = eina_inlist_remove(pd->callbacks, pd->callbacks);
        free(cb);
     }
}

static void
_efl_loop_future_propagate(Eo *obj, Efl_Loop_Future_Data *pd)
{
   Eo_Event ev;

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
        eo_unref(obj);
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
        eo_del(obj);
     }
}

static void
_efl_loop_future_prepare_events(Efl_Loop_Future_Data *pd, Eina_Bool progress, Eina_Bool optional)
{
   if (!pd->promise) return ;

   if (pd->optional)
     {
        pd->optional = EINA_FALSE;
        ecore_loop_future_unregister(eo_provider_find(pd->self, EFL_LOOP_CLASS), pd->self);
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
                               Eo_Event_Cb success, Eo_Event_Cb failure, Eo_Event_Cb progress, const void *data)
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
                      Eo_Event_Cb success, Eo_Event_Cb failure, Eo_Event_Cb progress, const void *data)
{
   Efl_Loop_Future_Callback *cb;
   Efl_Future *f;

   cb = _efl_loop_future_then_internal(pd, success, failure, progress, data);
   if (!cb) return NULL;

   cb->next = eo_add(EFL_PROMISE_CLASS, obj);
   f = efl_promise_future_get(cb->next);

   _efl_loop_future_prepare_events(pd, !!progress, EINA_FALSE);
   _efl_loop_future_fulfilling(obj, pd);

   return f;
}

static Eina_Bool
_efl_loop_future_internal_then(Efl_Future *f,
                               Eo_Event_Cb success, Eo_Event_Cb failure, Eo_Event_Cb progress, const void *data)
{
   Efl_Loop_Future_Data *pd = eo_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);
   Efl_Loop_Future_Callback *cb;

   cb = _efl_loop_future_then_internal(pd, success, failure, progress, data);
   if (!cb) return EINA_FALSE;

   _efl_loop_future_prepare_events(pd, !!progress, EINA_TRUE);
   _efl_loop_future_fulfilling(f, pd);

   return EINA_FALSE;
}

static void
_efl_loop_future_disconnect(Eo *obj, Efl_Loop_Future_Data *pd)
{
   Eo *promise;

   if (!pd->promise) return ;
   promise = eo_ref(pd->promise->promise);

   // Disconnect from the promise
   pd->promise->futures = eina_list_remove(pd->promise->futures, pd);

   // Notify that there is no more future
   if (!pd->promise->futures)
     {
        eo_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_NONE, NULL);
     }

   // Unreference after propagating the failure
   eo_data_xunref(pd->promise->promise, pd->promise, obj);
   pd->promise = NULL;

   eo_unref(promise);
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
   eo_del_intercept_set(obj, NULL);

   // Trigger events now
   pd = eo_data_scope_get(obj, EFL_LOOP_FUTURE_CLASS);

   if (!pd->promise) return ;
   if (pd->promise->set.future && !pd->promise->set.future_triggered)
     {
        eo_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_SET, obj);
        pd->promise->set.future_triggered = EINA_TRUE;
     }
   if (pd->promise->set.progress && !pd->promise->set.progress_triggered)
     {
        eo_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, obj);
        pd->promise->set.progress_triggered = EINA_TRUE;
     }
}

static Eo *
_efl_loop_future_eo_base_constructor(Eo *obj, Efl_Loop_Future_Data *pd)
{
   obj = eo_constructor(eo_super(obj, EFL_LOOP_FUTURE_CLASS));

   pd->self = obj;
   pd->optional = EINA_TRUE;

   ecore_loop_future_register(eo_provider_find(obj, EFL_LOOP_CLASS), obj);

   eo_del_intercept_set(obj, _efl_loop_future_intercept);

   return obj;
}

static void
_efl_loop_future_eo_base_destructor(Eo *obj, Efl_Loop_Future_Data *pd)
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

   // Stop the main loop handler that would destroy this optional future
   if (pd->optional)
     {
        pd->optional = EINA_FALSE;
        ecore_loop_future_unregister(eo_provider_find(pd->self, EFL_LOOP_CLASS), pd->self);
     }

   eo_destructor(eo_super(obj, EFL_LOOP_FUTURE_CLASS));

   // Disconnect from the promise
   _efl_loop_future_disconnect(obj, pd);
}

#ifndef NDEBUG
static void
_efl_future_wref_add(Eo *obj, Efl_Loop_Future_Data *pd, Eo **wref)
{
   eo_wref_add(eo_super(obj, EFL_LOOP_FUTURE_CLASS), wref);

   pd->wref++;
}

static void
_efl_future_wref_del(Eo *obj, Efl_Loop_Future_Data *pd, Eo **wref)
{
   eo_wref_del(eo_super(obj, EFL_LOOP_FUTURE_CLASS), wref);

   pd->wref--;
}
#endif

static const Eo_Op_Description _efl_loop_future_op_desc[] = {
  EO_OP_FUNC_OVERRIDE(efl_future_then, _efl_loop_future_then),
  EO_OP_FUNC_OVERRIDE(efl_future_cancel, _efl_loop_future_cancel),
  EO_OP_FUNC_OVERRIDE(eo_constructor, _efl_loop_future_eo_base_constructor),
  EO_OP_FUNC_OVERRIDE(eo_destructor, _efl_loop_future_eo_base_destructor),
#ifndef NDEBUG
  EO_OP_FUNC_OVERRIDE(eo_wref_add, _efl_future_wref_add),
  EO_OP_FUNC_OVERRIDE(eo_wref_del, _efl_future_wref_del),
#endif
};
static const Eo_Class_Description _efl_loop_future_class_desc = {
  EO_VERSION,
  "Efl_Future",
  EO_CLASS_TYPE_REGULAR,
  EO_CLASS_DESCRIPTION_OPS(_efl_loop_future_op_desc),
  NULL,
  sizeof (Efl_Loop_Future_Data),
  NULL,
  NULL
};

EO_DEFINE_CLASS(efl_loop_future_class_get, &_efl_loop_future_class_desc, EFL_FUTURE_CLASS, NULL);

static Efl_Future *
_efl_promise_future_get(Eo *obj, Efl_Promise_Data *pd EINA_UNUSED)
{
   Efl_Future *f;
   Efl_Loop_Future_Data *fd;

   // Build a new future, attach it and return it
   f = eo_add(EFL_LOOP_FUTURE_CLASS, NULL);
   if (!f) return NULL;

   fd = eo_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);

   fd->promise = eo_data_xref(obj, EFL_PROMISE_CLASS, f);
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
   eo_ref(obj);

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
   eo_unref(obj);
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
   eo_ref(obj);

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
   eo_unref(obj);
}

static void
_efl_promise_progress_set(Eo *obj, Efl_Promise_Data *pd, void *p)
{
   Efl_Loop_Future_Data *f;
   Eina_List *l, *ln;
   Efl_Future_Event_Progress chain_progress;
   Eo_Event ev;

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

static Eo_Base *
_efl_promise_eo_base_constructor(Eo *obj, Efl_Promise_Data *pd)
{
   pd->promise = obj;

   return eo_constructor(eo_super(obj, EFL_PROMISE_CLASS));
}

static void
_efl_promise_eo_base_destructor(Eo *obj, Efl_Promise_Data *pd)
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

   eo_destructor(eo_super(obj, EFL_PROMISE_CLASS));
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

   Eina_Bool failed : 1;
   Eina_Bool progress_triggered : 1;
   Eina_Bool future_triggered : 1;
   Eina_Bool done : 1;
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
   eo_del(all->promise);
   all->promise = NULL;
}

static void
_efl_promise_all_die(void *data, const Eo_Event *ev EINA_UNUSED)
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
_efl_promise_race_die(void *data, const Eo_Event *ev EINA_UNUSED)
{
   Efl_Promise_All *all = data;

   eina_array_flush(&all->members);
   free(all);
}

static void
_efl_future_set(void *data, const Eo_Event *ev EINA_UNUSED)
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
             Efl_Loop_Future_Data *pd = eo_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);

             if (!pd->promise->set.future && !pd->promise->set.future_triggered)
               {
                  eo_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_SET, f);
                  pd->promise->set.future_triggered = EINA_TRUE;
                  pd->promise->set.future = EINA_TRUE;
               }
          }
     }
}

static void
_efl_future_progress_set(void *data, const Eo_Event *ev EINA_UNUSED)
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
             Efl_Loop_Future_Data *pd = eo_data_scope_get(f, EFL_LOOP_FUTURE_CLASS);

             if (!pd->promise->set.progress && !pd->promise->set.progress_triggered)
               {
                  eo_event_callback_call(pd->promise->promise, EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, f);
                  pd->promise->set.progress_triggered = EINA_TRUE;
                  pd->promise->set.progress = EINA_TRUE;
               }
          }
     }
}

static void
_efl_all_future_none(void *data, const Eo_Event *ev EINA_UNUSED)
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
_efl_race_future_none(void *data, const Eo_Event *ev EINA_UNUSED)
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
        if (!f) efl_future_cancel(f);
     }

   // No one is listening to this promise anyway
   _efl_promise_all_free(all);
}

static Eina_Bool
_efl_accessor_all_get_at(Efl_Accessor_All *ac, unsigned int pos, void **data)
{
   Efl_Future_All *fa;

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
   eo_unref(ac->promise);
   free(ac);
}

static void
_then_all(void *data, const Eo_Event *ev)
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

   if (done)
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
        ac->promise = eo_ref(all->promise);

        efl_promise_value_set(all->promise, &ac->accessor, EINA_FREE_CB(eina_accessor_free));
     }
}

static void
_fail_all(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Failure *fail = ev->info;
   Efl_Promise_All *all = data;
   Efl_Future_All *fa;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (all->failed) return ;
   all->failed = EINA_TRUE;

   eo_ref(all->promise);

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

   efl_promise_failed(all->promise, fail->error);
   _efl_promise_all_free(all);
   eo_unref(all->promise);
}

static void
_progress(void *data, const Eo_Event *ev)
{
   Efl_Promise_All *all = data;
   Efl_Future_Event_Progress *p = ev->info;
   void *item;
   Efl_Future_All_Progress a;
   Eina_Array_Iterator iterator;
   unsigned int i;

   a.inprogress = ev->object;
   a.progress = p->progress;

   EINA_ARRAY_ITER_NEXT(&all->members, i, item, iterator)
     {
        Efl_Future *f = all->future_get(item);

        if (f == a.inprogress) break ;
     }
   a.index = i;

   efl_promise_progress_set(all->promise, &a);
}

EO_CALLBACKS_ARRAY_DEFINE(efl_all_callbacks,
                          { EFL_PROMISE_EVENT_FUTURE_SET, _efl_future_set },
                          { EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, _efl_future_progress_set },
                          { EFL_PROMISE_EVENT_FUTURE_NONE, _efl_all_future_none },
                          { EO_EVENT_DEL, _efl_promise_all_die });

EAPI Efl_Future *
efl_future_all_internal(Efl_Future *f1, ...)
{
   Efl_Promise_All *all;
   Efl_Future_All *fa;
   Efl_Future *fn;
   Eina_Array_Iterator iterator;
   unsigned int i;
   va_list args;

   if (!f1) return NULL;

   all = calloc(1, sizeof (Efl_Promise_All));
   if (!all) return NULL;

   eina_array_step_set(&all->members, sizeof (Eina_Array), 8);
   all->future_get = _efl_promise_all_future_get;
   all->promise = eo_add(EFL_PROMISE_CLASS, NULL);
   if (!all->promise) goto on_error;

   fa = calloc(1, sizeof (Efl_Future_All));
   if (!fa) goto on_error;
   efl_future_use(&fa->f, f1);
   eina_array_push(&all->members, fa);

   va_start(args, f1);

   while ((fn = va_arg(args, Efl_Future *)))
     {
        fa = calloc(1, sizeof (Efl_Future_All));
        if (!fa)
          {
             va_end(args);
             goto on_error;
          }
        eina_array_push(&all->members, fa);
        efl_future_use(&fa->f, fn);
     }

   va_end(args);

   EINA_ARRAY_ITER_NEXT(&all->members, i, fa, iterator)
     _efl_loop_future_internal_then(fa->f, _then_all, _fail_all, _progress, all);

   eo_event_callback_array_add(all->promise, efl_all_callbacks(), all);

   return efl_promise_future_get(all->promise);

 on_error:
   _efl_promise_all_die(all, NULL);
   return NULL;
}

static void
_then_race(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Success *success = ev->info;
   Efl_Promise_Race *race = data;
   // This is a trick due to the fact we are using internal function call to register this functions
   Efl_Promise_Msg *d = success->value;
   Efl_Future *f;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (race->done) return ;
   race->done = EINA_TRUE;

   eo_ref(race->promise);

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
   eo_unref(race->promise);
}

static void
_fail_race(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Failure *fail = ev->info;
   Efl_Promise_Race *race = data;
   Efl_Future *f;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (race->done) return ;
   race->done = EINA_TRUE;

   eo_ref(race->promise);

   EINA_ARRAY_ITER_NEXT(&race->members, i, f, iterator)
     {
        // To avoid double cancel/success
        eina_array_data_set(&race->members, i, NULL);

        if (f != ev->object)
          {
             efl_future_cancel(f);
          }
     }

   efl_promise_failed(race->promise, fail->error);
   _efl_promise_all_free(race);
   eo_unref(race->promise);
}

EO_CALLBACKS_ARRAY_DEFINE(efl_race_callbacks,
                          { EFL_PROMISE_EVENT_FUTURE_SET, _efl_future_set },
                          { EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, _efl_future_progress_set },
                          { EFL_PROMISE_EVENT_FUTURE_NONE, _efl_race_future_none },
                          { EO_EVENT_DEL, _efl_promise_race_die });

EAPI Efl_Future *
efl_future_race_internal(Efl_Future *f1, ...)
{
   Efl_Promise_Race *race;
   Efl_Future *fn;
   va_list args;
   Eina_Array_Iterator iterator;
   unsigned int i;

   if (!f1) return NULL;

   race = calloc(1, sizeof (Efl_Promise_Race));
   if (!race) return NULL;

   eina_array_step_set(&race->members, sizeof (Eina_Array), 8);
   race->future_get = _efl_promise_race_future_get;
   race->promise = eo_add(EFL_PROMISE_CLASS, NULL);
   if (!race->promise)
     {
        free(race);
        return NULL;
     }

   eina_array_push(&race->members, f1);

   va_start(args, f1);

   while ((fn = va_arg(args, Efl_Future *)))
     {
        eina_array_push(&race->members, fn);
     }

   EINA_ARRAY_ITER_NEXT(&race->members, i, fn, iterator)
     _efl_loop_future_internal_then(fn, _then_race, _fail_race, _progress, race);

   eo_event_callback_array_add(race->promise, efl_race_callbacks(), race);

   return efl_promise_future_get(race->promise);
}

#include "efl_promise.eo.c"
