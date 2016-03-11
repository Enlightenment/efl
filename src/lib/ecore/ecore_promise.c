#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.h>
#include <Ecore.h>
#include <Eo.h>

#include <assert.h>

typedef void(*Ecore_Promise_Free_Cb)(void*);

struct _Ecore_Promise_Then_Cb
{
  EINA_INLIST;

  Ecore_Promise_Cb callback;
  void* data;
};

struct _Ecore_Promise
{
  Eina_Lock lock;
  Eina_Condition condition;
  Eina_Bool has_finished : 1;
  Eina_Bool has_errored : 1;
  Eina_Bool has_pending_call : 1;
  Eina_Bool is_then_calls_manual : 1;
  Eina_Error error;
  size_t value_size;
  int ref;

  struct _Ecore_Promise_Then_Cb then_callbacks;
  Ecore_Promise_Free_Cb free_cb;
  
  char value[];
};

struct _Ecore_Promise_Thread_Data
{
  const void* data;
  Ecore_Promise_Thread_Cb func_blocking;
  Ecore_Promise* promise;
};

typedef struct _Ecore_Promise_Iterator _Ecore_Promise_Iterator;
typedef struct _Ecore_Promise_Success_Iterator _Ecore_Promise_Success_Iterator;
typedef struct _Ecore_Promise_Failure_Iterator _Ecore_Promise_Failure_Iterator;
struct _Ecore_Promise_Iterator
{
   Eina_Iterator* success_iterator;
   Eina_Iterator* failure_iterator;
   struct _Ecore_Promise_Success_Iterator
   {
      Eina_Iterator success_iterator_impl;
      struct _Ecore_Promise_Failure_Iterator
      {
         Eina_Iterator failure_iterator_impl;
         unsigned int promise_index;
         unsigned int num_promises;
         unsigned int promises_finished;
         Ecore_Promise* promises[];
      } data;
   } data;
};

static void _ecore_promise_lock_take(Ecore_Promise* promise);
static void _ecore_promise_lock_release(Ecore_Promise* promise);
static void _ecore_promise_finish(Ecore_Promise* promise);
static void _ecore_promise_then_calls(Ecore_Promise* promise);
static void _ecore_promise_unsafe_ref(Ecore_Promise const* promise);
static void _ecore_promise_unsafe_unref(Ecore_Promise const* promise);
static void _ecore_promise_unsafe_free_unref(Ecore_Promise const* promise);
static Eina_Bool _ecore_promise_unlock_unsafe_free_unref(Ecore_Promise const* promise);

static void _ecore_promise_iterator_setup(_Ecore_Promise_Iterator* iterator, Eina_Array* promises);

static void _ecore_promise_thread_end(void* data, Ecore_Thread* thread EINA_UNUSED)
{
  struct _Ecore_Promise_Thread_Data* p = data;
  _ecore_promise_lock_take(p->promise);
  if(p->promise->has_finished)
    {
      p->promise->has_pending_call = EINA_FALSE;
      if(!_ecore_promise_unlock_unsafe_free_unref(p->promise))
        _ecore_promise_then_calls(p->promise);
    }
  else
    {
      p->promise->is_then_calls_manual = EINA_FALSE;
      p->promise->has_pending_call = EINA_FALSE;
      _ecore_promise_unlock_unsafe_free_unref(p->promise);
    }
  free(data);
}

static void
_ecore_promise_thread_blocking(void* data, Ecore_Thread* thread EINA_UNUSED)
{
  struct _Ecore_Promise_Thread_Data* p = data;
  (p->func_blocking)(p->data, p->promise);
}

static void
_ecore_promise_then_calls(Ecore_Promise* promise)
{
  _ecore_promise_lock_take(promise);
  struct _Ecore_Promise_Then_Cb then_callbacks = promise->then_callbacks;
  memset(&promise->then_callbacks, 0, sizeof(promise->then_callbacks));
  promise->has_pending_call = EINA_FALSE;
  _ecore_promise_lock_release(promise);
  
  struct _Ecore_Promise_Then_Cb* callback;

  if(then_callbacks.callback)
    {
       (*then_callbacks.callback)(then_callbacks.data, &promise->value[0]);
       _ecore_promise_unsafe_free_unref(promise);
    }

  if(EINA_INLIST_GET(&then_callbacks)->next)
    {
       Eina_Inlist* list2;
       EINA_INLIST_FOREACH_SAFE(EINA_INLIST_GET(&then_callbacks)->next, list2, callback)
         {
            if(callback->callback)
              {
                 (*callback->callback)(callback->data, promise);
              }
            _ecore_promise_unsafe_free_unref(promise);
         }
    }
}

Ecore_Promise* ecore_promise_thread_run(Ecore_Promise_Thread_Cb func_blocking, const void* data, size_t value_size)
{
  struct _Ecore_Promise_Thread_Data *new_data = malloc(sizeof(struct _Ecore_Promise_Thread_Data));
  new_data->data = data;
  new_data->func_blocking = func_blocking;
  new_data->promise = ecore_promise_add(value_size);
  new_data->promise->is_then_calls_manual = EINA_TRUE;
  new_data->promise->has_pending_call = EINA_TRUE;
  ecore_promise_ref(new_data->promise);
  ecore_thread_run(&_ecore_promise_thread_blocking, &_ecore_promise_thread_end, NULL, new_data);
  return new_data->promise;
}

Ecore_Promise* ecore_promise_add(int value_size)
{
  Ecore_Promise* p = malloc(sizeof(Ecore_Promise) + value_size);
  eina_lock_new(&p->lock);
  eina_condition_new(&p->condition, &p->lock);
  p->has_finished = p->has_errored = p->has_pending_call = p->is_then_calls_manual = EINA_FALSE;
  p->ref = 1;
  memset(&p->then_callbacks, 0, sizeof(p->then_callbacks));
  p->value_size = value_size;
  p->free_cb = NULL;
  return p;
}

static void _ecore_promise_del(Ecore_Promise* promise)
{
  if(promise->free_cb)
    promise->free_cb((void*)&promise->value[0]);
  
  eina_lock_free(&promise->lock);
  eina_condition_free(&promise->condition);
}

void* ecore_promise_buffer_get(Ecore_Promise* promise)
{
  return &promise->value[0];
}

void* ecore_promise_value_get(Ecore_Promise const* promise)
{
  _ecore_promise_lock_take((Ecore_Promise*)promise);
  _ecore_promise_unsafe_ref(promise);
  while(!promise->has_finished)
  {
     eina_condition_wait((Eina_Condition*)&promise->condition);
  }

  void* v = (void*)(promise->value_size && !promise->has_errored ? &promise->value[0] : NULL);
  _ecore_promise_unsafe_unref(promise);
  _ecore_promise_lock_release((Ecore_Promise*)promise);
  return v;
}

void ecore_promise_value_set(Ecore_Promise* promise, void* data)
{
  _ecore_promise_lock_take(promise);
  if(data && promise->value_size)
    {
      memcpy(&promise->value[0], data, promise->value_size);
    }

  _ecore_promise_finish(promise);
}

static void _ecore_promise_all_compose_then_cb(Ecore_Promise* promise, void* value EINA_UNUSED)
{
   _ecore_promise_lock_take(promise);
   _Ecore_Promise_Iterator* iterator = (_Ecore_Promise_Iterator*)promise->value;

   if(++iterator->data.data.promises_finished == iterator->data.data.num_promises)
     {
        _ecore_promise_finish(promise);
     }
   else
     _ecore_promise_lock_release(promise);
}

static void _ecore_promise_all_free(_Ecore_Promise_Iterator* value)
{
  unsigned i = 0;
  eina_iterator_free(value->success_iterator);
  /* eina_iterator_free(value->failure_iterator); */
  
  for(;i != value->data.data.num_promises; ++i)
    {
       ecore_promise_unref(value->data.data.promises[i]);
    }
}

Ecore_Promise* ecore_promise_all(Eina_Iterator* it)
{
  Ecore_Promise* current, *promise;
  Eina_Array* promises;

  promises = eina_array_new(20);

  EINA_ITERATOR_FOREACH(it, current)
    {
      eina_array_push(promises, current);
    }
  
  promise = ecore_promise_add(sizeof(_Ecore_Promise_Iterator) + sizeof(Ecore_Promise*)*eina_array_count_get(promises));
  //promise->is_then_calls_manual = EINA_TRUE;
  promise->free_cb = (Ecore_Promise_Free_Cb)_ecore_promise_all_free;
  _Ecore_Promise_Iterator* internal_it = ecore_promise_buffer_get(promise);
  _ecore_promise_iterator_setup(internal_it, promises);
  eina_array_free(promises);

  {
    Ecore_Promise** cur_promise = internal_it->data.data.promises, ** last =
      internal_it->data.data.promises + internal_it->data.data.num_promises;
    for(;cur_promise != last; ++cur_promise)
      {
         ecore_promise_ref(*cur_promise);
         ecore_promise_then(*cur_promise, (Ecore_Promise_Cb)&_ecore_promise_all_compose_then_cb, promise);
      }
  }

  return promise;
}

void ecore_promise_then(Ecore_Promise* promise, Ecore_Promise_Cb callback, void* data)
{
  _ecore_promise_lock_take(promise);
  _ecore_promise_unsafe_ref(promise);
  if(!promise->then_callbacks.callback && !EINA_INLIST_GET(&promise->then_callbacks)->next)
    {
      promise->then_callbacks.callback = callback;
      promise->then_callbacks.data = data;
    }
  else
    {
      struct _Ecore_Promise_Then_Cb* p = malloc(sizeof(struct _Ecore_Promise_Then_Cb));
      p->callback = callback;
      p->data = data;
      Eina_Inlist* l = eina_inlist_append(EINA_INLIST_GET(&promise->then_callbacks), EINA_INLIST_GET(p));
      (void)l;
    }
  if(promise->has_finished && !promise->has_pending_call)
    {
       promise->has_pending_call = EINA_TRUE;
       _ecore_promise_lock_release(promise);
       ecore_job_add((Ecore_Cb)&_ecore_promise_then_calls, promise);
    }
  else
    _ecore_promise_lock_release(promise);
}

EAPI Eina_Error ecore_promise_error_get(Ecore_Promise const* promise)
{
  _ecore_promise_lock_take((Ecore_Promise*)promise);
  if(promise->has_errored)
    {
       Eina_Error error = promise->error;
       _ecore_promise_lock_release((Ecore_Promise*)promise);
       return error;
    }
  else
    {
       _ecore_promise_lock_release((Ecore_Promise*)promise);
       return 0;
    }
}

EAPI void ecore_promise_error_set(Ecore_Promise* promise, Eina_Error error)
{
  _ecore_promise_lock_take(promise);
  promise->error = error;
  promise->has_errored = EINA_TRUE;
  
  _ecore_promise_finish(promise);
}

static void
_ecore_promise_finish(Ecore_Promise* promise)
{
  promise->has_finished = EINA_TRUE;
  eina_condition_broadcast(&promise->condition);
  _ecore_promise_unsafe_unref(promise);
  if(!promise->is_then_calls_manual && !promise->has_pending_call)
    {
       promise->has_pending_call = EINA_TRUE;
       _ecore_promise_lock_release(promise);
       ecore_job_add((Ecore_Cb)&_ecore_promise_then_calls, promise);
    }
  else
    _ecore_promise_lock_release(promise);
}

static Eina_Bool
_ecore_promise_iterator_next(_Ecore_Promise_Success_Iterator *it, void **data)
{
   if(it->data.promise_index == it->data.num_promises)
     return EINA_FALSE;
   
   if(ecore_promise_error_get(it->data.promises[it->data.promise_index]))
     {
        return EINA_FALSE;
     }
   else
     {
        *data = ecore_promise_value_get(it->data.promises[it->data.promise_index++]);
        return EINA_TRUE;
     }
}

static void**
_ecore_promise_iterator_get_container(_Ecore_Promise_Success_Iterator *it)
{
   return (void**)it->data.promises;
}

static void
_ecore_promise_iterator_free(_Ecore_Promise_Success_Iterator *it EINA_UNUSED)
{
}

static void _ecore_promise_iterator_setup(_Ecore_Promise_Iterator* it, Eina_Array* promises_array)
{
   Ecore_Promise** promises;

   it->success_iterator = &it->data.success_iterator_impl;
   it->failure_iterator = &it->data.data.failure_iterator_impl;
   it->data.data.num_promises = eina_array_count_get(promises_array);
   it->data.data.promise_index = 0;
   promises = (Ecore_Promise**)promises_array->data;

   memcpy(&it->data.data.promises[0], promises, it->data.data.num_promises*sizeof(Ecore_Promise*));

   EINA_MAGIC_SET(&it->data.success_iterator_impl, EINA_MAGIC_ITERATOR);
   EINA_MAGIC_SET(&it->data.data.failure_iterator_impl, EINA_MAGIC_ITERATOR);

   it->data.success_iterator_impl.version = EINA_ITERATOR_VERSION;
   it->data.success_iterator_impl.next = FUNC_ITERATOR_NEXT(_ecore_promise_iterator_next);
   it->data.success_iterator_impl.get_container = FUNC_ITERATOR_GET_CONTAINER(
      _ecore_promise_iterator_get_container);
   it->data.success_iterator_impl.free = FUNC_ITERATOR_FREE(_ecore_promise_iterator_free);
}

EAPI int ecore_promise_value_size_get(Ecore_Promise const* promise)
{
  return promise->value_size;
}

static void _ecore_promise_lock_take(Ecore_Promise* promise)
{
  eina_lock_take(&promise->lock);
}

static void _ecore_promise_lock_release(Ecore_Promise* promise)
{
  eina_lock_release(&promise->lock);
}

static void _ecore_promise_unsafe_ref(Ecore_Promise const* promise)
{
  Ecore_Promise* p = (Ecore_Promise*)promise;
  ++p->ref;
}

static void _ecore_promise_free_cb(Ecore_Promise* promise)
{
  _ecore_promise_lock_take(promise);
  _ecore_promise_unlock_unsafe_free_unref(promise);
}

static void _ecore_promise_unsafe_unref(Ecore_Promise const* promise)
{
  Ecore_Promise* p = (Ecore_Promise*)promise;
  if(p->ref == 1 && !p->has_pending_call)
    {
      ecore_job_add((Ecore_Cb)_ecore_promise_free_cb, p);
    }
  else
    --p->ref;
}

static void _ecore_promise_unsafe_free_unref(Ecore_Promise const* promise)
{
  Ecore_Promise* p = (Ecore_Promise*)promise;
  if(--p->ref == 0)
    {
      assert(!p->has_pending_call);
      _ecore_promise_del(p);
    }
}

static Eina_Bool _ecore_promise_unlock_unsafe_free_unref(Ecore_Promise const* promise)
{
  Ecore_Promise* p = (Ecore_Promise*)promise;
  if(--p->ref == 0)
    {
       assert(!p->has_pending_call);
       _ecore_promise_lock_release((Ecore_Promise*)promise);
       _ecore_promise_del(p);
       return EINA_TRUE;
    }
  else
    {
       _ecore_promise_lock_release((Ecore_Promise*)promise);
       return EINA_FALSE;
    }
}

EAPI void ecore_promise_ref(Ecore_Promise* promise)
{
  _ecore_promise_lock_take(promise);
  _ecore_promise_unsafe_ref(promise);
  _ecore_promise_lock_release(promise);
}

EAPI void ecore_promise_unref(Ecore_Promise* promise)
{
  _ecore_promise_lock_take(promise);
  _ecore_promise_unsafe_unref(promise);
  _ecore_promise_lock_release(promise);
}

void _ecore_promise_init()
{
  _eo_ecore_promise_add = &ecore_promise_add;
}

void _ecore_promise_shutdown()
{
}
