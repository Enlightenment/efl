#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.h>

#include <eina_private.h>

#include <assert.h>

static int _eina_promise_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_promise_log_dom, __VA_ARGS__)

typedef struct _Eina_Promise_Then_Cb _Eina_Promise_Then_Cb;
typedef struct _Eina_Promise_Progress_Cb _Eina_Promise_Progress_Cb;
typedef struct _Eina_Promise_Cancel_Cb _Eina_Promise_Cancel_Cb;
typedef struct _Eina_Promise_Owner_Progress_Notify_Data _Eina_Promise_Owner_Progress_Notify_Data;
typedef struct _Eina_Promise_Default _Eina_Promise_Default;
typedef struct _Eina_Promise_Default_Owner _Eina_Promise_Default_Owner;
typedef struct _Eina_Promise_Iterator _Eina_Promise_Iterator;
typedef struct _Eina_Promise_Success_Iterator _Eina_Promise_Success_Iterator;

struct _Eina_Promise_Then_Cb
{
   EINA_INLIST;

   Eina_Promise_Cb callback;
   Eina_Promise_Error_Cb error_cb;
   void* data;
};

struct _Eina_Promise_Progress_Cb
{
   EINA_INLIST;

   Eina_Promise_Progress_Cb callback;
   Eina_Promise_Free_Cb free;
   void* data;
};

struct _Eina_Promise_Cancel_Cb
{
   EINA_INLIST;

   Eina_Promise_Default_Cancel_Cb callback;
   Eina_Promise_Free_Cb free;
   void* data;
};

struct _Eina_Promise_Owner_Progress_Notify_Data
{
   EINA_INLIST;

   Eina_Promise_Progress_Notify_Cb callback;
   Eina_Promise_Free_Cb free_cb;
   void* data;
};

struct _Eina_Promise_Default
{
   Eina_Promise vtable;
   Eina_Error error;
   size_t value_size;

   Eina_Inlist *then_callbacks;
   Eina_Inlist *progress_callbacks;
   Eina_Inlist *cancel_callbacks;
   Eina_Inlist *progress_notify_callbacks;
   Eina_Promise_Free_Cb value_free_cb;

   int ref;

   Eina_Bool has_finished : 1;
   Eina_Bool has_errored : 1;
   Eina_Bool is_cancelled : 1;
   Eina_Bool is_manual_then : 1;
   Eina_Bool is_first_then : 1;
   Eina_Bool is_pointer : 1;
};

struct _Eina_Promise_Default_Owner
{
   Eina_Promise_Owner owner_vtable;
   _Eina_Promise_Default promise;

   char value[];
};

#define EINA_PROMISE_GET_OWNER(p) (_Eina_Promise_Default_Owner*)((unsigned char*)p - offsetof(struct _Eina_Promise_Default_Owner, promise))

struct _Eina_Promise_Iterator
{
   struct _Eina_Promise_Success_Iterator
   {
      Eina_Iterator success_iterator_impl;
      unsigned int promise_index;
      unsigned int num_promises;
      unsigned int promises_finished;
      Eina_Promise* promises[];
   } data;
};

typedef struct _Eina_Promise_Race_Value_Type _Eina_Promise_Race_Value_Type;
struct _Eina_Promise_Race_Value_Type
{
   void* value;
   unsigned int promise_index;
   unsigned int num_promises;
   struct _Eina_Promise_Race_Information
   {
      Eina_Promise* promise;
      _Eina_Promise_Default_Owner* self;
   } promises[];
};

static void _eina_promise_free_progress_callback_node(void* node)
{
   _Eina_Promise_Progress_Cb *progress_cb = node;
   if(progress_cb->free)
     progress_cb->free(progress_cb->data);
   free(progress_cb);
}

static void _eina_promise_free_progress_notify_callback_node(void* node)
{
   _Eina_Promise_Owner_Progress_Notify_Data *progress_notify_cb = node;
   if(progress_notify_cb->free_cb)
     progress_notify_cb->free_cb(progress_notify_cb->data);
   free(progress_notify_cb);
}

static void _eina_promise_finish(_Eina_Promise_Default_Owner* promise);
static void _eina_promise_ref(_Eina_Promise_Default* promise);
static void _eina_promise_unref(_Eina_Promise_Default* promise);

static void _eina_promise_iterator_setup(_Eina_Promise_Iterator* iterator, Eina_Array* promises);

static void _eina_promise_free_callback_list(Eina_Inlist** list, void(*free_cb)(void* node))
{
  struct node
  {
    EINA_INLIST;
  } *node;
  Eina_Inlist *list2;
  
  EINA_INLIST_FOREACH_SAFE(*list, list2, node)
    {
      free_cb(node);
    }      
  *list = NULL;
}

static void
_eina_promise_then_calls(_Eina_Promise_Default_Owner* promise)
{
   _Eina_Promise_Then_Cb* callback;
   Eina_Bool error;

   _eina_promise_ref(&promise->promise);
   error = promise->promise.has_errored;

   EINA_INLIST_FREE(promise->promise.then_callbacks, callback)
     {
       promise->promise.then_callbacks = eina_inlist_remove(promise->promise.then_callbacks, EINA_INLIST_GET(callback));
       if (error)
         {
            if (callback->error_cb)
              (*callback->error_cb)(callback->data, promise->promise.error);
         }
       else if (callback->callback)
         {
            if(promise->promise.is_pointer)
              {
                 char* buffer = promise->value;
                 void** p = (void**)buffer;
                 (*callback->callback)(callback->data, *p);
              }
            else
              (*callback->callback)(callback->data, &promise->value[0]);
         }
       free(callback);
       _eina_promise_unref(&promise->promise);
     }
   _eina_promise_unref(&promise->promise);
}

static void
_eina_promise_cancel_calls(_Eina_Promise_Default_Owner* promise, Eina_Bool call_cancel EINA_UNUSED)
{
   _Eina_Promise_Cancel_Cb* callback;

   _eina_promise_ref(&promise->promise);
   EINA_INLIST_FREE(promise->promise.cancel_callbacks, callback)
     {
        promise->promise.cancel_callbacks = eina_inlist_remove(promise->promise.cancel_callbacks, EINA_INLIST_GET(callback));
        if (callback->callback)
          {
             (*callback->callback)(callback->data, (Eina_Promise_Owner*)promise);
          }
        free(callback);
     }

   if (!promise->promise.is_manual_then)
     {
        _eina_promise_then_calls(promise);
     }
   _eina_promise_unref(&promise->promise);
}

static void
_eina_promise_del(_Eina_Promise_Default_Owner* promise)
{
   if (!promise->promise.has_finished)
     {
        ERR("Promise is being deleted, despite not being finished yet. This will cause intermitent crashes");
     }

   if (promise->promise.value_free_cb)
     {
        if(promise->promise.is_pointer)
          {
             char* buffer = promise->value;
             void** p = (void**)buffer;
             promise->promise.value_free_cb(*p);
          }
        else
          promise->promise.value_free_cb((void*)&promise->value[0]);
     }

   _eina_promise_free_callback_list(&promise->promise.progress_callbacks,
                                    &_eina_promise_free_progress_callback_node);
   _eina_promise_free_callback_list(&promise->promise.progress_notify_callbacks,
                                    &_eina_promise_free_progress_notify_callback_node);
   free(promise);
}

static void *
_eina_promise_owner_buffer_get(_Eina_Promise_Default_Owner* promise)
{
   return &promise->value[0];
}

static void *
_eina_promise_owner_pointer_buffer_get(_Eina_Promise_Default_Owner* promise)
{
   char* buffer = promise->value;
   void** p = (void**)buffer;
   return *p;
}

static void *
_eina_promise_buffer_get(_Eina_Promise_Default const* promise)
{
   _Eina_Promise_Default_Owner const* p = EINA_PROMISE_GET_OWNER(promise);
   return (void*)&p->value[0];
}

static void *
_eina_promise_value_get(_Eina_Promise_Default const* p)
{
   _Eina_Promise_Default_Owner const* promise = EINA_PROMISE_GET_OWNER(p);
   if (p->has_finished && !p->has_errored)
     {
       return (void*)&promise->value[0];
     }
   else
     {
        return NULL;
     }
}

static void *
_eina_promise_pointer_value_get(_Eina_Promise_Default const* p)
{
   _Eina_Promise_Default_Owner const* promise = EINA_PROMISE_GET_OWNER(p);
   if (p->has_finished && !p->has_errored)
     {
        char const* buffer = promise->value;
        void** p = (void**)buffer;
        return *p;
     }
   else
     {
        return NULL;
     }
}

static void
_eina_promise_owner_value_set(_Eina_Promise_Default_Owner* promise, const void* data, Eina_Promise_Free_Cb free)
{
   if (data && promise->promise.value_size)
     {
        memcpy(&promise->value[0], data, promise->promise.value_size);
     }

   promise->promise.value_free_cb = free;

   _eina_promise_finish(promise);
}

static void
_eina_promise_owner_pointer_value_set(_Eina_Promise_Default_Owner* promise, const void* data, Eina_Promise_Free_Cb free)
{
   char* buffer = promise->value;
   void** p = (void**)buffer;
   *p = (void*)data;
   promise->promise.value_free_cb = free;
   _eina_promise_finish(promise);
}

static void
_eina_promise_then(_Eina_Promise_Default* p, Eina_Promise_Cb callback,
		   Eina_Promise_Error_Cb error_cb, void* data)
{
   _Eina_Promise_Default_Owner* promise;
   _Eina_Promise_Then_Cb* cb;
   _Eina_Promise_Owner_Progress_Notify_Data* notify_data;

   promise = EINA_PROMISE_GET_OWNER(p);

   cb = malloc(sizeof(struct _Eina_Promise_Then_Cb));

   cb->callback = callback;
   cb->error_cb = error_cb;
   cb->data = data;
   promise->promise.then_callbacks = eina_inlist_append(promise->promise.then_callbacks, EINA_INLIST_GET(cb));

   EINA_INLIST_FOREACH(promise->promise.progress_notify_callbacks, notify_data)
     {
        (*notify_data->callback)(notify_data->data, &promise->owner_vtable);
     }
   _eina_promise_free_callback_list(&promise->promise.progress_notify_callbacks, &free);
   
   if (!promise->promise.is_first_then)
     {
        _eina_promise_ref(p);
     }
   else
     promise->promise.is_first_then = EINA_FALSE;
   if (promise->promise.has_finished)
     {
        _eina_promise_then_calls(promise);
     }
}

static void
_eina_promise_owner_error_set(_Eina_Promise_Default_Owner* promise, Eina_Error error)
{
   promise->promise.error = error;
   promise->promise.has_errored = EINA_TRUE;

   _eina_promise_finish(promise);
}

static void
_eina_promise_finish(_Eina_Promise_Default_Owner* promise)
{
   promise->promise.has_finished = EINA_TRUE;
   if (!promise->promise.is_manual_then)
     {
        _eina_promise_then_calls(promise);
     }
}

static Eina_Error
_eina_promise_error_get(_Eina_Promise_Default const* promise)
{
   if (promise->has_errored)
     {
        return promise->error;
     }
   else
     {
        return 0;
     }
}

static Eina_Bool
_eina_promise_pending_is(_Eina_Promise_Default const* promise)
{
   return !promise->has_finished;
}

static Eina_Bool
_eina_promise_owner_pending_is(_Eina_Promise_Default_Owner const* promise)
{
   return !promise->promise.has_finished;
}

static Eina_Bool
_eina_promise_owner_cancelled_is(_Eina_Promise_Default_Owner const* promise)
{
   return promise->promise.is_cancelled;
}

static void
_eina_promise_progress_cb_add(_Eina_Promise_Default* promise, Eina_Promise_Progress_Cb callback, void* data,
                              Eina_Promise_Free_Cb free_cb)
{
   _Eina_Promise_Progress_Cb* cb;
   _Eina_Promise_Owner_Progress_Notify_Data* notify_data;
   _Eina_Promise_Default_Owner* owner = EINA_PROMISE_GET_OWNER(promise);

   cb = malloc(sizeof(struct _Eina_Promise_Progress_Cb));
   cb->callback = callback;
   cb->data = data;
   cb->free = free_cb;
   promise->progress_callbacks = eina_inlist_append(promise->progress_callbacks, EINA_INLIST_GET(cb));

   EINA_INLIST_FOREACH(owner->promise.progress_notify_callbacks, notify_data)
     {
       (*notify_data->callback)(notify_data->data, &owner->owner_vtable);
     }
   _eina_promise_free_callback_list(&owner->promise.progress_notify_callbacks,
                                    &_eina_promise_free_progress_notify_callback_node);
}

static void
_eina_promise_cancel(_Eina_Promise_Default* promise)
{
   _Eina_Promise_Default_Owner* owner = EINA_PROMISE_GET_OWNER(promise);

   if (!owner->promise.is_cancelled)
     {
        owner->promise.is_cancelled = EINA_TRUE;
        owner->promise.has_finished = EINA_TRUE;
        owner->promise.has_errored = EINA_TRUE;
        owner->promise.error = EINA_ERROR_PROMISE_CANCEL;
        _eina_promise_cancel_calls(owner, EINA_TRUE);
     }
}

static size_t
_eina_promise_value_size_get(_Eina_Promise_Default_Owner const* promise)
{
   return promise->promise.value_size;
}

static size_t
_eina_promise_owner_value_size_get(_Eina_Promise_Default_Owner const* promise)
{
   return promise->promise.value_size;
}

static void
_eina_promise_ref(_Eina_Promise_Default* p)
{
   ++p->ref;
}

static void
_eina_promise_unref(_Eina_Promise_Default* p)
{
   if (p->ref == 1 && p->has_finished)
     {
        _eina_promise_del(EINA_PROMISE_GET_OWNER(p));
     }
   else
     {
        --p->ref;
     }
}

static Eina_Promise *
_eina_promise_owner_promise_get(_Eina_Promise_Default_Owner* p)
{
   return &p->promise.vtable;
}

void
eina_promise_owner_default_cancel_cb_add(Eina_Promise_Owner* p,
					 Eina_Promise_Default_Cancel_Cb callback, void* data,
					 Eina_Promise_Free_Cb free_cb)
{
   _Eina_Promise_Default_Owner *promise;
   _Eina_Promise_Cancel_Cb *cb;

   promise = (_Eina_Promise_Default_Owner *)p;

   cb = malloc(sizeof(struct _Eina_Promise_Cancel_Cb));

   cb->callback = callback;
   cb->free = free_cb;
   cb->data = data;
   promise->promise.cancel_callbacks = eina_inlist_append(promise->promise.cancel_callbacks, EINA_INLIST_GET(cb));

   if (promise->promise.is_cancelled)
     {
        _eina_promise_cancel_calls(promise, EINA_TRUE);
     }
}

static void
_eina_promise_owner_progress(_Eina_Promise_Default_Owner* promise, void* data)
{
   _Eina_Promise_Progress_Cb* callback;
   Eina_Inlist* list2;

   EINA_INLIST_FOREACH_SAFE(promise->promise.progress_callbacks, list2, callback)
     {
        if(callback->callback)
          (*callback->callback)(callback->data, data);
     }
}

static void
_eina_promise_owner_progress_notify(_Eina_Promise_Default_Owner* promise, Eina_Promise_Progress_Notify_Cb notify,
                                    void* data, Eina_Promise_Free_Cb free_cb)
{
   _Eina_Promise_Owner_Progress_Notify_Data* cb
     = malloc(sizeof(struct _Eina_Promise_Owner_Progress_Notify_Data));

   cb->callback = notify;
   cb->free_cb = free_cb;
   cb->data = data;
   promise->promise.progress_notify_callbacks =
     eina_inlist_append(promise->promise.progress_notify_callbacks, EINA_INLIST_GET(cb));
}

Eina_Promise_Owner *
eina_promise_value_add(int value_size)
{
   _Eina_Promise_Default_Owner* p;

   p = malloc(sizeof(_Eina_Promise_Default_Owner) + value_size);
   p->promise.vtable.version = EINA_PROMISE_VERSION;
   p->promise.vtable.then = EINA_FUNC_PROMISE_THEN(_eina_promise_then);
   p->promise.vtable.value_get = EINA_FUNC_PROMISE_VALUE_GET(_eina_promise_value_get);
   p->promise.vtable.error_get = EINA_FUNC_PROMISE_ERROR_GET(_eina_promise_error_get);
   p->promise.vtable.pending_is = EINA_FUNC_PROMISE_PENDING_IS(_eina_promise_pending_is);
   p->promise.vtable.progress_cb_add = EINA_FUNC_PROMISE_PROGRESS_CB_ADD(_eina_promise_progress_cb_add);
   p->promise.vtable.cancel = EINA_FUNC_PROMISE_CANCEL(_eina_promise_cancel);
   p->promise.vtable.ref = EINA_FUNC_PROMISE_REF(_eina_promise_ref);
   p->promise.vtable.unref = EINA_FUNC_PROMISE_UNREF(_eina_promise_unref);
   p->promise.vtable.value_size_get = EINA_FUNC_PROMISE_VALUE_SIZE_GET(_eina_promise_value_size_get);
   p->promise.vtable.buffer_get = EINA_FUNC_PROMISE_BUFFER_GET(_eina_promise_buffer_get);
   EINA_MAGIC_SET(&p->promise.vtable, EINA_MAGIC_PROMISE);
   
   p->promise.has_finished = p->promise.has_errored =
     p->promise.is_cancelled = p->promise.is_manual_then = p->promise.is_pointer = EINA_FALSE;
   p->promise.is_first_then = EINA_TRUE;
   p->promise.ref = 1;
   memset(&p->promise.then_callbacks, 0, sizeof(p->promise.then_callbacks));
   memset(&p->promise.progress_callbacks, 0, sizeof(p->promise.progress_callbacks));
   memset(&p->promise.progress_notify_callbacks, 0, sizeof(p->promise.progress_notify_callbacks));
   memset(&p->promise.cancel_callbacks, 0, sizeof(p->promise.cancel_callbacks));
   p->promise.value_size = value_size;
   p->promise.value_free_cb = NULL;
   p->promise.error = 0;

   p->owner_vtable.version = EINA_PROMISE_VERSION;
   p->owner_vtable.value_set = EINA_FUNC_PROMISE_OWNER_VALUE_SET(_eina_promise_owner_value_set);
   p->owner_vtable.error_set = EINA_FUNC_PROMISE_OWNER_ERROR_SET(_eina_promise_owner_error_set);
   p->owner_vtable.buffer_get = EINA_FUNC_PROMISE_OWNER_BUFFER_GET(_eina_promise_owner_buffer_get);
   p->owner_vtable.value_size_get = EINA_FUNC_PROMISE_OWNER_VALUE_SIZE_GET(_eina_promise_owner_value_size_get);
   p->owner_vtable.promise_get = EINA_FUNC_PROMISE_OWNER_PROMISE_GET(_eina_promise_owner_promise_get);
   p->owner_vtable.pending_is = EINA_FUNC_PROMISE_OWNER_PENDING_IS(_eina_promise_owner_pending_is);
   p->owner_vtable.cancelled_is = EINA_FUNC_PROMISE_OWNER_CANCELLED_IS(_eina_promise_owner_cancelled_is);
   p->owner_vtable.progress = EINA_FUNC_PROMISE_OWNER_PROGRESS(_eina_promise_owner_progress);
   p->owner_vtable.progress_notify = EINA_FUNC_PROMISE_OWNER_PROGRESS_NOTIFY(_eina_promise_owner_progress_notify);
   EINA_MAGIC_SET(&p->owner_vtable, EINA_MAGIC_PROMISE_OWNER);

   return &p->owner_vtable;
}

EAPI Eina_Promise_Owner *
eina_promise_add()
{
   _Eina_Promise_Default_Owner* p;

   p = malloc(sizeof(_Eina_Promise_Default_Owner) + sizeof(void*));
   p->promise.vtable.version = EINA_PROMISE_VERSION;
   p->promise.vtable.then = EINA_FUNC_PROMISE_THEN(_eina_promise_then);
   p->promise.vtable.value_get = EINA_FUNC_PROMISE_VALUE_GET(_eina_promise_pointer_value_get);
   p->promise.vtable.error_get = EINA_FUNC_PROMISE_ERROR_GET(_eina_promise_error_get);
   p->promise.vtable.pending_is = EINA_FUNC_PROMISE_PENDING_IS(_eina_promise_pending_is);
   p->promise.vtable.progress_cb_add = EINA_FUNC_PROMISE_PROGRESS_CB_ADD(_eina_promise_progress_cb_add);
   p->promise.vtable.cancel = EINA_FUNC_PROMISE_CANCEL(_eina_promise_cancel);
   p->promise.vtable.ref = EINA_FUNC_PROMISE_REF(_eina_promise_ref);
   p->promise.vtable.unref = EINA_FUNC_PROMISE_UNREF(_eina_promise_unref);
   p->promise.vtable.value_size_get = EINA_FUNC_PROMISE_VALUE_SIZE_GET(_eina_promise_value_size_get);
   p->promise.vtable.buffer_get = EINA_FUNC_PROMISE_BUFFER_GET(_eina_promise_buffer_get);
   EINA_MAGIC_SET(&p->promise.vtable, EINA_MAGIC_PROMISE);

   p->promise.has_finished = p->promise.has_errored =
     p->promise.is_cancelled = p->promise.is_manual_then = EINA_FALSE;
   p->promise.is_first_then = p->promise.is_pointer = EINA_TRUE;
   p->promise.ref = 1;
   memset(&p->promise.then_callbacks, 0, sizeof(p->promise.then_callbacks));
   memset(&p->promise.progress_callbacks, 0, sizeof(p->promise.progress_callbacks));
   memset(&p->promise.progress_notify_callbacks, 0, sizeof(p->promise.progress_notify_callbacks));
   memset(&p->promise.cancel_callbacks, 0, sizeof(p->promise.cancel_callbacks));
   p->promise.value_size = 0;
   p->promise.value_free_cb = NULL;
   p->promise.error = 0;

   p->owner_vtable.version = EINA_PROMISE_VERSION;
   p->owner_vtable.value_set = EINA_FUNC_PROMISE_OWNER_VALUE_SET(_eina_promise_owner_pointer_value_set);
   p->owner_vtable.error_set = EINA_FUNC_PROMISE_OWNER_ERROR_SET(_eina_promise_owner_error_set);
   p->owner_vtable.buffer_get = EINA_FUNC_PROMISE_OWNER_BUFFER_GET(_eina_promise_owner_pointer_buffer_get);
   p->owner_vtable.value_size_get = EINA_FUNC_PROMISE_OWNER_VALUE_SIZE_GET(_eina_promise_owner_value_size_get);
   p->owner_vtable.promise_get = EINA_FUNC_PROMISE_OWNER_PROMISE_GET(_eina_promise_owner_promise_get);
   p->owner_vtable.pending_is = EINA_FUNC_PROMISE_OWNER_PENDING_IS(_eina_promise_owner_pending_is);
   p->owner_vtable.cancelled_is = EINA_FUNC_PROMISE_OWNER_CANCELLED_IS(_eina_promise_owner_cancelled_is);
   p->owner_vtable.progress = EINA_FUNC_PROMISE_OWNER_PROGRESS(_eina_promise_owner_progress);
   p->owner_vtable.progress_notify = EINA_FUNC_PROMISE_OWNER_PROGRESS_NOTIFY(_eina_promise_owner_progress_notify);
   EINA_MAGIC_SET(&p->owner_vtable, EINA_MAGIC_PROMISE_OWNER);
   
   return &p->owner_vtable;
}

void
eina_promise_owner_default_manual_then_set(Eina_Promise_Owner* owner, Eina_Bool is_manual)
{
   _Eina_Promise_Default_Owner* p = (_Eina_Promise_Default_Owner*)owner;

   p->promise.is_manual_then = is_manual;
}

void
eina_promise_owner_default_call_then(Eina_Promise_Owner* promise)
{
   _Eina_Promise_Default_Owner* owner = (_Eina_Promise_Default_Owner*)promise;

   _eina_promise_then_calls(owner);
}

static void
_eina_promise_all_compose_then_cb(_Eina_Promise_Default_Owner* promise, void* value EINA_UNUSED, Eina_Promise* promise_then EINA_UNUSED)
{
   _Eina_Promise_Iterator* iterator;

   if (!promise->promise.has_finished)
     {
        iterator = (_Eina_Promise_Iterator*)promise->value;
        if (++iterator->data.promises_finished == iterator->data.num_promises)
          {
             _eina_promise_finish(promise);
          }
     }
}

static void
_eina_promise_all_compose_error_then_cb(_Eina_Promise_Default_Owner* promise, Eina_Error error, Eina_Promise* promise_then EINA_UNUSED)
{
   if (!promise->promise.has_finished)
     {
        promise->promise.has_finished = promise->promise.has_errored = EINA_TRUE;
        promise->promise.error = error;
        _eina_promise_finish(promise);
     }
}

static void
_eina_promise_all_free(_Eina_Promise_Iterator* value)
{
   unsigned i = 0;

   for (;i != value->data.num_promises; ++i)
     {
        eina_promise_unref(value->data.promises[i]);
     }
}

Eina_Promise *
eina_promise_all(Eina_Iterator* it)
{
   _Eina_Promise_Default_Owner *promise;
   Eina_Promise* current;
   Eina_Array* promises;
   Eina_Promise **cur_promise, **last;
   _Eina_Promise_Iterator* internal_it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);
   
   promises = eina_array_new(20);

   EINA_ITERATOR_FOREACH(it, current)
     {
        eina_array_push(promises, current);
     }

   eina_iterator_free(it);

   promise = (_Eina_Promise_Default_Owner*)
     eina_promise_value_add(sizeof(_Eina_Promise_Iterator) +
                            sizeof(_Eina_Promise_Default_Owner*)*eina_array_count_get(promises));
   internal_it = (_Eina_Promise_Iterator*)&promise->value[0];
   _eina_promise_iterator_setup(internal_it, promises);
   eina_array_free(promises);

   promise->promise.value_free_cb = (Eina_Promise_Free_Cb)&_eina_promise_all_free;

   cur_promise = internal_it->data.promises;
   last = internal_it->data.promises + internal_it->data.num_promises;
   for (;cur_promise != last; ++cur_promise)
     {
        eina_promise_ref(*cur_promise); // We need to keep the value alive until this promise is freed
        eina_promise_then(*cur_promise, (Eina_Promise_Cb)&_eina_promise_all_compose_then_cb,
                          (Eina_Promise_Error_Cb)&_eina_promise_all_compose_error_then_cb, promise);
     }

   return &promise->promise.vtable;
}

static Eina_Bool
_eina_promise_iterator_next(_Eina_Promise_Success_Iterator *it, void **data)
{
   if (it->promise_index == it->num_promises)
     return EINA_FALSE;

   if (eina_promise_error_get(it->promises[it->promise_index]))
     {
        return EINA_FALSE;
     }
   else
     {
        *data = eina_promise_value_get(it->promises[it->promise_index++]);
        return EINA_TRUE;
     }
}

static void **
_eina_promise_iterator_get_container(_Eina_Promise_Success_Iterator *it)
{
   return (void**)it->promises;
}

static void
_eina_promise_iterator_free(_Eina_Promise_Success_Iterator *it EINA_UNUSED)
{
}

static void
_eina_promise_iterator_setup(_Eina_Promise_Iterator* it, Eina_Array* promises_array)
{
   Eina_Promise** promises;

   it->data.num_promises = eina_array_count_get(promises_array);
   it->data.promise_index = 0;
   it->data.promises_finished = 0;
   promises = (Eina_Promise**)promises_array->data;

   memcpy(&it->data.promises[0], promises, it->data.num_promises*sizeof(Eina_Promise*));

   EINA_MAGIC_SET(&it->data.success_iterator_impl, EINA_MAGIC_ITERATOR);

   it->data.success_iterator_impl.version = EINA_ITERATOR_VERSION;
   it->data.success_iterator_impl.next = FUNC_ITERATOR_NEXT(_eina_promise_iterator_next);
   it->data.success_iterator_impl.get_container = FUNC_ITERATOR_GET_CONTAINER(
      _eina_promise_iterator_get_container);
   it->data.success_iterator_impl.free = FUNC_ITERATOR_FREE(_eina_promise_iterator_free);
}

static void
_eina_promise_progress_notify_fulfilled(void* data, Eina_Promise_Owner* p EINA_UNUSED)
{
  Eina_Promise_Owner* owner = data;
  eina_promise_owner_value_set(owner, NULL, NULL);
}

EAPI Eina_Error EINA_ERROR_PROMISE_NO_NOTIFY;
EAPI Eina_Error EINA_ERROR_PROMISE_CANCEL;
EAPI Eina_Error EINA_ERROR_PROMISE_NULL;

static void
_eina_promise_progress_notify_failed(void* data)
{
  Eina_Promise_Owner* owner = data;
  if(eina_promise_owner_pending_is(owner))
    eina_promise_owner_error_set(owner, EINA_ERROR_PROMISE_NO_NOTIFY);
}

EAPI Eina_Promise*
eina_promise_progress_notification(Eina_Promise_Owner* promise)
{
  Eina_Promise_Owner* owner;

  owner = eina_promise_value_add(0);

  eina_promise_owner_progress_notify(promise, &_eina_promise_progress_notify_fulfilled, owner,
                                     &_eina_promise_progress_notify_failed);

  return eina_promise_owner_promise_get(owner);
}

// Race implementation
static void
_eina_promise_race_free(_Eina_Promise_Race_Value_Type* value)
{
   unsigned i = 0;
   for (;i != value->num_promises; ++i)
     {
        eina_promise_unref(value->promises[i].promise);
     }
}

static void
_eina_promise_race_compose_then_cb(struct _Eina_Promise_Race_Information* info, void* value EINA_UNUSED, Eina_Promise* promise_then EINA_UNUSED)
{
   _Eina_Promise_Default_Owner* race_promise;
   _Eina_Promise_Race_Value_Type *race_value;

   race_promise = info->self;
   race_value = (_Eina_Promise_Race_Value_Type*)race_promise->value;

   if (!race_promise->promise.has_finished)
     {
        race_value->value = value;
        race_value->promise_index =  info - &race_value->promises[0];
        _eina_promise_finish(race_promise);
     }
}

static void
_eina_promise_race_compose_error_then_cb(struct _Eina_Promise_Race_Information* info, Eina_Error error, Eina_Promise* promise_then EINA_UNUSED)
{
   _Eina_Promise_Default_Owner* race_promise;
   _Eina_Promise_Race_Value_Type *race_value;

   race_promise = info->self;
   race_value = (_Eina_Promise_Race_Value_Type*)race_promise->value;

   if (!race_promise->promise.has_finished)
     {
        race_value->promise_index =  info - &race_value->promises[0];
        eina_promise_owner_error_set(&race_promise->owner_vtable, error);
     }
}

Eina_Promise *
eina_promise_race(Eina_Iterator* it)
{
   _Eina_Promise_Default_Owner *promise;
   Eina_Promise* current;
   Eina_Array* promises;
   struct _Eina_Promise_Race_Information *cur_promise, *last;
   _Eina_Promise_Race_Value_Type *value;
   int num_promises;

   EINA_SAFETY_ON_NULL_RETURN_VAL(it, NULL);

   promises = eina_array_new(20);

   EINA_ITERATOR_FOREACH(it, current)
     {
        eina_array_push(promises, current);
     }

   eina_iterator_free(it);

   num_promises = eina_array_count_get(promises);
   promise = (_Eina_Promise_Default_Owner*)
     eina_promise_value_add(sizeof(_Eina_Promise_Race_Value_Type) +
                            sizeof(struct _Eina_Promise_Race_Information)*num_promises);
   value = eina_promise_owner_buffer_get((Eina_Promise_Owner*)promise);
   value->value = NULL;
   value->promise_index = -1;
   value->num_promises = num_promises;

   promise->promise.value_free_cb = (Eina_Promise_Free_Cb)&_eina_promise_race_free;

   cur_promise = value->promises;
   last = value->promises + value->num_promises;
   for (int i = 0;cur_promise != last; ++cur_promise, ++i)
     {
        cur_promise->promise = eina_array_data_get(promises, i);
        cur_promise->self = promise;
        eina_promise_then(cur_promise->promise, (Eina_Promise_Cb)&_eina_promise_race_compose_then_cb,
                          (Eina_Promise_Error_Cb)&_eina_promise_race_compose_error_then_cb, cur_promise);
        eina_promise_ref(cur_promise->promise); // We need to keep the value alive until this promise is freed
     }

   eina_array_free(promises);

   return &promise->promise.vtable;
}

// API functions
EAPI void
eina_promise_then(Eina_Promise* promise, Eina_Promise_Cb callback,
		  Eina_Promise_Error_Cb error_cb, void* data)
{
   if(!promise)
     {
        if(!error_cb)
          {
            ERR("eina_promise_the with NULL promise and no error callback.");
            eina_error_set(EINA_ERROR_PROMISE_NULL);
          }
        else
          error_cb(data, EINA_ERROR_PROMISE_NULL);
        return;
     }
   else
     promise->then(promise, callback, error_cb, data);
}

#define _EINA_PROMISE_NULL_CHECK(promise, v)    \
  if(!promise)                                  \
    {                                           \
      eina_error_set(EINA_ERROR_PROMISE_NULL);  \
      return v;                                 \
    }
#define EINA_MAGIC_CHECK_PROMISE(promise)               \
  do if(!EINA_MAGIC_CHECK(promise, EINA_MAGIC_PROMISE))    \
       {EINA_MAGIC_FAIL(promise, EINA_MAGIC_PROMISE);} while(0)
#define EINA_MAGIC_CHECK_PROMISE_OWNER(promise)               \
  do {if(!EINA_MAGIC_CHECK(promise, EINA_MAGIC_PROMISE_OWNER))  \
      {EINA_MAGIC_FAIL(promise, EINA_MAGIC_PROMISE_OWNER);} } while(0)

EAPI void
eina_promise_owner_value_set(Eina_Promise_Owner* promise, const void* value, Eina_Promise_Free_Cb free)
{
   _EINA_PROMISE_NULL_CHECK(promise, );
   EINA_MAGIC_CHECK_PROMISE_OWNER(promise);
   promise->value_set(promise, value, free);
}

EAPI void
eina_promise_owner_error_set(Eina_Promise_Owner* promise, Eina_Error error)
{
   _EINA_PROMISE_NULL_CHECK(promise, );
   EINA_MAGIC_CHECK_PROMISE_OWNER(promise);
   promise->error_set(promise, error);
}

EAPI void *
eina_promise_value_get(Eina_Promise const* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, NULL);
   EINA_MAGIC_CHECK_PROMISE(promise);
   return promise->value_get(promise);
}

EAPI Eina_Error
eina_promise_error_get(Eina_Promise const* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, EINA_ERROR_PROMISE_NULL);
   EINA_MAGIC_CHECK_PROMISE(promise);
   return promise->error_get(promise);
}

EAPI Eina_Bool
eina_promise_pending_is(Eina_Promise const* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, EINA_FALSE);
   EINA_MAGIC_CHECK_PROMISE(promise);
   return promise->pending_is(promise);
}

EAPI void
eina_promise_progress_cb_add(Eina_Promise* promise, Eina_Promise_Progress_Cb callback, void* data,
                             Eina_Promise_Free_Cb free_cb)
{
   _EINA_PROMISE_NULL_CHECK(promise, );
   EINA_MAGIC_CHECK_PROMISE(promise);
   promise->progress_cb_add(promise, callback, data, free_cb);
}

EAPI void
eina_promise_cancel(Eina_Promise* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, );
   EINA_MAGIC_CHECK_PROMISE(promise);
   promise->cancel(promise);
}

EAPI void
eina_promise_ref(Eina_Promise* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, );
   EINA_MAGIC_CHECK_PROMISE(promise);
   promise->ref(promise);
}

EAPI void
eina_promise_unref(Eina_Promise* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, );
   EINA_MAGIC_CHECK_PROMISE(promise);
   promise->unref(promise);
}

EAPI void *
eina_promise_owner_buffer_get(Eina_Promise_Owner* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, NULL);
   EINA_MAGIC_CHECK_PROMISE(promise);
   return promise->buffer_get(promise);
}

EAPI void *
eina_promise_buffer_get(Eina_Promise* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, NULL);
   EINA_MAGIC_CHECK_PROMISE(promise);
   return promise->buffer_get(promise);
}

EAPI size_t
eina_promise_value_size_get(Eina_Promise const* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, 0);
   EINA_MAGIC_CHECK_PROMISE(promise);
   return promise->value_size_get(promise);
}

EAPI Eina_Promise *
eina_promise_owner_promise_get(Eina_Promise_Owner* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, NULL);
   EINA_MAGIC_CHECK_PROMISE_OWNER(promise);
   return promise->promise_get(promise);
}

EAPI Eina_Bool
eina_promise_owner_pending_is(Eina_Promise_Owner const* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, EINA_FALSE);
   EINA_MAGIC_CHECK_PROMISE_OWNER(promise);
   return promise->pending_is(promise);
}

EAPI Eina_Bool
eina_promise_owner_cancelled_is(Eina_Promise_Owner const* promise)
{
   _EINA_PROMISE_NULL_CHECK(promise, EINA_FALSE);
   EINA_MAGIC_CHECK_PROMISE_OWNER(promise);
   return promise->cancelled_is(promise);
}

EAPI void
eina_promise_owner_progress(Eina_Promise_Owner const* promise, void* progress)
{
   _EINA_PROMISE_NULL_CHECK(promise, );
   EINA_MAGIC_CHECK_PROMISE_OWNER(promise);
   promise->progress(promise, progress);
}

EAPI void
eina_promise_owner_progress_notify(Eina_Promise_Owner* promise, Eina_Promise_Progress_Notify_Cb progress_cb,
                                   void* data, Eina_Promise_Free_Cb free_cb)
{
   _EINA_PROMISE_NULL_CHECK(promise, );
   EINA_MAGIC_CHECK_PROMISE_OWNER(promise);
   promise->progress_notify(promise, progress_cb, data, free_cb);
}

static const char EINA_ERROR_PROMISE_NO_NOTIFY_STR[] = "Out of memory";
static const char EINA_ERROR_PROMISE_CANCEL_STR[] = "Promise cancelled";
static const char EINA_ERROR_PROMISE_NULL_STR[] = "NULL promise";

Eina_Bool eina_promise_init()
{
   EINA_ERROR_PROMISE_NO_NOTIFY = eina_error_msg_static_register(EINA_ERROR_PROMISE_NO_NOTIFY_STR);
   EINA_ERROR_PROMISE_CANCEL = eina_error_msg_static_register(EINA_ERROR_PROMISE_CANCEL_STR);
   EINA_ERROR_PROMISE_NULL = eina_error_msg_static_register(EINA_ERROR_PROMISE_NULL_STR);

   _eina_promise_log_dom = eina_log_domain_register("eina_promise",
                                                    EINA_LOG_COLOR_DEFAULT);
   if (_eina_promise_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_promise");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eina_Bool eina_promise_shutdown()
{
   eina_log_domain_unregister(_eina_promise_log_dom);
   _eina_promise_log_dom = -1;
   return EINA_TRUE;
}
