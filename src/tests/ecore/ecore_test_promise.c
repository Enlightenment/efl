#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_suite.h"
#include <time.h>

void promised_thread(const void* data EINA_UNUSED, Eina_Promise_Owner* promise, Ecore_Thread* thread EINA_UNUSED)
{
  eina_promise_owner_value_set(promise, NULL, NULL);
}

void promise_callback(void* data EINA_UNUSED, void* value EINA_UNUSED)
{
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise)
{
   Eina_Promise* promise;

   ecore_init();

   ecore_thread_promise_run(&promised_thread, NULL, NULL, &promise);
   eina_promise_then(promise, &promise_callback, NULL, NULL);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

void promise_error_thread(const void* data EINA_UNUSED, Eina_Promise_Owner* promise, Ecore_Thread* thread EINA_UNUSED)
{
  eina_promise_owner_error_set(promise, EINA_ERROR_OUT_OF_MEMORY);
}

void promise_error_callback(void* data EINA_UNUSED, Eina_Error error)
{
  ck_assert(error == EINA_ERROR_OUT_OF_MEMORY);
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_error)
{
   Eina_Promise* promise;

   ecore_init();

   ecore_thread_promise_run(&promise_error_thread, NULL, NULL, &promise);
   eina_promise_then(promise, NULL, &promise_error_callback, NULL);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_promise_all)
{
   Eina_Promise* promise;
   Eina_Promise* first[2] = { NULL, NULL };

   ecore_init();

   ecore_thread_promise_run(&promised_thread, NULL, NULL, &first[0]);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));
   eina_promise_then(promise, &promise_callback, NULL, NULL);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

void promise_callback2(void* data, void* value EINA_UNUSED)
{
  if(++(*(int*)data) == 2)
    ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_all_then_then)
{
   Eina_Promise* promise;
   Eina_Promise* first[2] = { NULL, NULL };

   ecore_init();

   int i = 0;

   ecore_thread_promise_run(&promised_thread, NULL, NULL, &first[0]);
   eina_promise_then(first[0], &promise_callback2, NULL, &i);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));
   eina_promise_then(promise, &promise_callback2, NULL, &i);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

struct sync_data
{
  Eina_Lock lock;
  Eina_Condition cond;
  Eina_Bool var;
};

void promised_exit_thread(struct sync_data* data EINA_UNUSED, Eina_Promise_Owner* promise, Ecore_Thread* thread EINA_UNUSED)
{
  eina_promise_owner_value_set(promise, NULL, NULL);
  eina_lock_take(&data->lock);
  data->var = EINA_TRUE;
  eina_condition_broadcast(&data->cond);
  eina_lock_release(&data->lock);
}

static void _ecore_test_promise_then_after_thread_finished_main_cb()
{
   Eina_Promise* promise;
   Ecore_Thread* thread;
   struct sync_data data;

   data.var = EINA_FALSE;
   eina_lock_new(&data.lock);
   eina_condition_new(&data.cond, &data.lock);

   thread = ecore_thread_promise_run((Ecore_Thread_Promise_Cb)&promised_exit_thread, NULL, &data, &promise);

   eina_lock_take(&data.lock);
   while(!data.var)
     {
       eina_condition_wait(&data.cond);
     }
   eina_lock_release(&data.lock);
   eina_promise_then(promise, &promise_callback, NULL, NULL);

   while(ecore_thread_wait(thread, 1.0) != EINA_TRUE); // join thread
}

START_TEST(ecore_test_promise_then_after_thread_finished)
{
   ecore_init();

   ecore_job_add(&_ecore_test_promise_then_after_thread_finished_main_cb, NULL);
   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

static void _ecore_test_promise_then_after_thread_finished_all_main_cb()
{
   Eina_Promise* first[] = {NULL, NULL};
   Ecore_Thread* thread;
   Eina_Promise* promise;
   struct sync_data data;

   data.var = EINA_FALSE;
   eina_lock_new(&data.lock);
   eina_condition_new(&data.cond, &data.lock);

   thread = ecore_thread_promise_run((Ecore_Thread_Promise_Cb)&promised_exit_thread, NULL, &data, &first[0]);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));

   eina_lock_take(&data.lock);
   while(!data.var)
     {
       eina_condition_wait(&data.cond);
     }
   eina_lock_release(&data.lock);
   eina_promise_then(promise, &promise_callback, NULL, NULL);

   while(ecore_thread_wait(thread, 1.0) != EINA_TRUE); // join thread
}

START_TEST(ecore_test_promise_then_after_thread_finished_all)
{
   ecore_init();

   ecore_job_add(&_ecore_test_promise_then_after_thread_finished_all_main_cb, NULL);
   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

static void
_ecore_test_promise_normal_lifetime_cb(void* data EINA_UNUSED, void* value EINA_UNUSED)
{
  ecore_main_loop_quit();
}

static void
_ecore_test_promise_normal_lifetime_value_set_cb(void* promise_owner)
{
   eina_promise_owner_value_set((Eina_Promise_Owner*)promise_owner, NULL, NULL);
}

START_TEST(ecore_test_promise_normal_lifetime)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* promise;
   
   ecore_init();

   promise_owner = eina_promise_add();

   promise = eina_promise_owner_promise_get(promise_owner);

   eina_promise_then(promise, &_ecore_test_promise_normal_lifetime_cb, NULL, NULL);
   ecore_job_add(_ecore_test_promise_normal_lifetime_value_set_cb, promise_owner);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_promise_normal_lifetime_all)
{
   Eina_Promise_Owner* promise_owner;
   Eina_Promise* first[2] = {NULL, NULL};
   Eina_Promise* promise;
   
   ecore_init();

   promise_owner = eina_promise_add();
   first[0] = eina_promise_owner_promise_get(promise_owner);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));
   
   eina_promise_then(promise, &_ecore_test_promise_normal_lifetime_cb, NULL, NULL);
   ecore_job_add(_ecore_test_promise_normal_lifetime_value_set_cb, promise_owner);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

static void
_ecore_test_promise_immediate_set_lifetime_cb(void* data EINA_UNUSED, void* value EINA_UNUSED)
{
   ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_immediate_set_lifetime)
{
   Eina_Promise_Owner* owner;
   Eina_Promise* promise;
   
   ecore_init();

   owner = eina_promise_add();
   promise = eina_promise_owner_promise_get(owner);

   eina_promise_owner_value_set(owner, NULL, NULL);
   eina_promise_then(promise, &_ecore_test_promise_immediate_set_lifetime_cb, NULL, NULL);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_promise_immediate_set_lifetime_all)
{
   Eina_Promise_Owner* owner;
   Eina_Promise* first[2] = {NULL, NULL};
   Eina_Promise* promise;

   ecore_init();

   owner = eina_promise_add();
   first[0] = eina_promise_owner_promise_get(owner);
   promise = eina_promise_all(eina_carray_iterator_new((void**)&first[0]));

   eina_promise_owner_value_set(owner, NULL, NULL);
   eina_promise_then(promise, &_ecore_test_promise_immediate_set_lifetime_cb, NULL, NULL);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

struct _condition_var
{
  Eina_Lock lock;
  Eina_Condition condvar;
  Eina_Bool boolean;
};
typedef struct _condition_var _condition_var;

static void promise_cancel_thread(const void* data, Eina_Promise_Owner* promise EINA_UNUSED, Ecore_Thread* thread EINA_UNUSED)
{
  _condition_var* v = (void*)data;

  eina_lock_take(&v->lock);
  while(!v->boolean)
    eina_condition_wait(&v->condvar);
  eina_lock_release(&v->lock);
}

static void _cancel_callback(const void* data, Eina_Promise_Owner* promise, Ecore_Thread* thread EINA_UNUSED)
{
  _condition_var* v = (void*)data;
  
  eina_lock_take(&v->lock);
  v->boolean = EINA_TRUE;
  eina_condition_broadcast(&v->condvar);
  eina_lock_release(&v->lock);

  eina_promise_owner_value_set(promise, NULL, NULL);
}

static void _cancel_promise_callback(void* data EINA_UNUSED, Eina_Error value)
{
  ck_assert(value == EINA_ERROR_PROMISE_CANCEL);
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_cancel_promise)
{
   _condition_var v = {.boolean = EINA_FALSE};
   Eina_Promise* promise;
   
   ecore_init();

   eina_lock_new(&v.lock);
   eina_condition_new(&v.condvar, &v.lock);

   ecore_thread_promise_run(&promise_cancel_thread, &_cancel_callback, &v, &promise);
   eina_promise_then(promise, NULL, &_cancel_promise_callback, NULL);

   eina_promise_cancel(promise);

   ecore_main_loop_begin();


   eina_condition_free(&v.condvar);
   eina_lock_free(&v.lock);
   ecore_shutdown();
}
END_TEST

static void promise_progress_thread(const void* data EINA_UNUSED,
                                    Eina_Promise_Owner* promise, Ecore_Thread* thread EINA_UNUSED)
{
  void* v = (void*)1;
  eina_promise_owner_progress(promise, v);
  // Release the promise to avoid leaks
  eina_promise_owner_value_set(promise, NULL, NULL);
}

static void _progress_callback(void* data EINA_UNUSED, void* value)
{
  ck_assert(value == (void*)1);
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_progress_promise)
{
   Eina_Promise* promise;

   ecore_init();

   ecore_thread_promise_run(&promise_progress_thread, NULL, NULL, &promise);

   eina_promise_progress_cb_add(promise, &_progress_callback, NULL, NULL);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

typedef struct _Future_Ok Future_Ok;
struct _Future_Ok
{
   Eina_Bool then : 1;
   Eina_Bool cancel : 1;
   Eina_Bool progress : 1;
};

static void
_then(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Success *s = ev->info;
   int *value = s->value;
   Future_Ok *fo = data;

   fail_if(*value != 42);
   fo->then = EINA_TRUE;
}

static void
_cancel(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Failure *f = ev->info;
   Future_Ok *fo = data;

   fail_if(f->error != EINA_ERROR_FUTURE_CANCEL);
   fo->cancel = EINA_TRUE;
}

static void
_progress(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Progress *p = ev->info;
   int *value = p->progress;
   Future_Ok *fo = data;

   fail_if(*value != 7);
   fo->progress = EINA_TRUE;
}

static void
_death(void *data, const Eo_Event *ev EINA_UNUSED)
{
   Eina_Bool *death = data;

   *death = EINA_TRUE;
}

// Test value set after then
START_TEST(efl_test_promise_future_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   fail_if(!fo.then || fo.cancel || !fo.progress);
   fail_if(!deadf || deadp);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);
   eo_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   efl_promise_value_set(p, &value, NULL);

   fail_if(fo.then || !fo.cancel || !fo.progress);
   fail_if(!deadf || deadp);
   fail_if(!none);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

// Test value set before then
START_TEST(efl_test_promise_before_future_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   fail_if(deadp || !deadf);
   fail_if(!fo.then || fo.cancel || fo.progress);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_before_future_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);
   eo_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo));

   fail_if(deadp || !deadf);

   efl_promise_value_set(p, &value, NULL);

   fail_if(fo.then || !fo.cancel || fo.progress);
   fail_if(!none);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

static void
_chain_then(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Success *s = ev->info;
   int *v = s->value;
   Future_Ok *fo = data;

   fo->then = EINA_TRUE;

   fail_if(*v != 42);

   efl_promise_value_set(s->next, v, NULL);
}

static void
_chain_fail(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Failure *f = ev->info;
   Future_Ok *fo = data;

   fo->cancel = EINA_TRUE;

   efl_promise_failed(f->next, f->error);
}

static void
_chain_progress(void *data, const Eo_Event *ev)
{
   Efl_Future_Event_Progress *p = ev->info;
   Future_Ok *fo = data;

   fo->progress = EINA_TRUE;

   efl_promise_progress_set(p->next, p->progress);
}

// Test chained then
START_TEST(efl_test_promise_future_chain_success)
{
   Efl_Promise *p;
   Efl_Future *f1, *f2;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf1 = EINA_FALSE, deadf2 = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f1 = efl_promise_future_get(p);
   fail_if(!f1);

   eo_event_callback_add(f1, EO_EVENT_DEL, _death, &deadf1);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);

   f2 = efl_future_then(f1, _chain_then, _chain_fail, _chain_progress, &fo1);
   fail_if(!f2);

   eo_event_callback_add(f2, EO_EVENT_DEL, _death, &deadf2);

   fail_if(!efl_future_then(f2, _then, _cancel, _progress, &fo2));

   fail_if(deadp || deadf1 || deadf2);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   fail_if(!fo1.then || fo1.cancel || !fo1.progress);
   fail_if(!fo2.then || fo2.cancel || !fo2.progress);
   fail_if(!deadf1 || !deadf2 || deadp);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_chain_cancel)
{
   Efl_Promise *p;
   Efl_Future *f1, *f2;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf1 = EINA_FALSE, deadf2 = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f1, efl_promise_future_get(p));
   fail_if(!f1);

   eo_event_callback_add(f1, EO_EVENT_DEL, _death, &deadf1);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);
   eo_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   f2 = efl_future_then(f1, _chain_then, _chain_fail, _chain_progress, &fo1);
   fail_if(!f2);

   eo_event_callback_add(f2, EO_EVENT_DEL, _death, &deadf2);

   fail_if(!efl_future_then(f2, _then, _cancel, _progress, &fo2));

   fail_if(deadp || deadf1 || deadf2);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f1);

   efl_promise_value_set(p, &value, NULL);

   fail_if(fo1.then || !fo1.cancel || !fo1.progress);
   fail_if(fo2.then || !fo2.cancel || !fo2.progress);
   fail_if(!deadf1 || !deadf2 || deadp);
   fail_if(!none);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

// Test value set after multi then
START_TEST(efl_test_promise_future_multi_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);

   eo_ref(f);
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));
   eo_unref(f);

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   fail_if(!fo1.then || fo1.cancel || !fo1.progress);
   fail_if(!fo2.then || fo2.cancel || !fo2.progress);
   fail_if(!deadf || deadp);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_multi_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);
   eo_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   efl_promise_value_set(p, &value, NULL);

   fail_if(fo1.then || !fo1.cancel || !fo1.progress);
   fail_if(fo2.then || !fo2.cancel || !fo2.progress);
   fail_if(!deadf || deadp);
   fail_if(!none);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

// Test value set before multi then
START_TEST(efl_test_promise_before_future_multi_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, NULL);

   eo_ref(f);
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));
   eo_unref(f);

   fail_if(deadp || !deadf);
   fail_if(!fo1.then || fo1.cancel || fo1.progress);
   fail_if(!fo2.then || fo2.cancel || fo2.progress);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_before_future_multi_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Future_Ok fo1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo2 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok fo3 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);
   eo_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   eo_ref(f);
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo1));
   fail_if(!efl_future_then(f, _then, _cancel, _progress, &fo2));
   eo_unref(f);

   fail_if(efl_future_then(f, _then, _cancel, _progress, &fo3));

   fail_if(deadp || !deadf);

   efl_promise_value_set(p, &value, NULL);

   fail_if(fo1.then || !fo1.cancel || fo1.progress);
   fail_if(fo2.then || !fo2.cancel || fo2.progress);
   fail_if(fo3.then || fo3.cancel || fo3.progress);
   fail_if(!none);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

static Eina_Bool cleanup = EINA_FALSE;

static void
_cleanup_called(void *s EINA_UNUSED)
{
   cleanup = EINA_TRUE;
}

// Test optional value set without then
START_TEST(efl_test_promise_future_optional_success)
{
   Efl_Promise *p;
   Efl_Future *f;
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE;
   int progress = 7;
   int value = 42;

   cleanup = EINA_FALSE;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   f = efl_promise_future_get(p);
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_promise_value_set(p, &value, _cleanup_called);

   fail_if(deadf || deadp);

   ecore_main_loop_iterate();

   fail_if(!deadf);

   eo_del(p);

   fail_if(!deadp || !cleanup);

   ecore_shutdown();
}
END_TEST

START_TEST(efl_test_promise_future_optional_cancel)
{
   Efl_Promise *p;
   Efl_Future *f;
   Eina_Bool deadf = EINA_FALSE, deadp = EINA_FALSE, none = EINA_FALSE;
   int progress = 7;
   int value = 42;

   cleanup = EINA_FALSE;

   ecore_init();

   p = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);

   efl_future_use(&f, efl_promise_future_get(p));
   fail_if(!f);

   eo_event_callback_add(f, EO_EVENT_DEL, _death, &deadf);
   eo_event_callback_add(p, EO_EVENT_DEL, _death, &deadp);
   eo_event_callback_add(p, EFL_PROMISE_EVENT_FUTURE_NONE, _death, &none);

   fail_if(deadp || deadf);

   efl_promise_progress_set(p, &progress);
   efl_future_cancel(f);

   efl_promise_value_set(p, &value, _cleanup_called);

   fail_if(deadf || deadp);
   fail_if(!none);

   ecore_main_loop_iterate();

   fail_if(!deadf || deadp);

   eo_del(p);

   fail_if(!deadp);

   ecore_shutdown();
}
END_TEST

static int value[] = { 42, 7, 3 };

static void
_then_all(void *data, const Eo_Event *ev)
{
   Future_Ok *fo = data;
   Efl_Future_Event_Success *s = ev->info;
   Eina_Accessor *ac = s->value;
   int *i;

   fail_if(!eina_accessor_data_get(ac, 0, (void**) &i));
   fail_if(i != &value[0]);
   fail_if(!eina_accessor_data_get(ac, 1, (void**) &i));
   fail_if(i != &value[1]);
   fail_if(!eina_accessor_data_get(ac, 2, (void**) &i));
   fail_if(i != &value[2]);

   fo->then = EINA_TRUE;
}

START_TEST(efl_test_promise_all)
{
   Efl_Promise *p1, *p2, *p3;
   Efl_Future *all = NULL, *f1;
   Future_Ok donea = { EINA_FALSE, EINA_FALSE, EINA_FALSE };
   Future_Ok donep1 = { EINA_FALSE, EINA_FALSE, EINA_FALSE };

   ecore_init();

   p1 = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   p2 = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   p3 = eo_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p1 || !p2 || !p3);

   f1 = eo_ref(efl_promise_future_get(p1));
   fail_if(!efl_future_then(f1, _then, _cancel, _progress, &donep1));
   efl_future_use(&all, efl_future_all(f1, efl_promise_future_get(p2), efl_promise_future_get(p3)));
   eo_unref(f1);

   fail_if(!efl_future_then(all, _then_all, _cancel, _progress, &donea));
   fail_if(!all);

   efl_promise_value_set(p1, &value[0], NULL);
   fail_if(!donep1.then || donep1.cancel || donep1.progress);
   fail_if(donea.then || donea.cancel || donea.progress);

   efl_promise_value_set(p2, &value[1], NULL);
   efl_promise_value_set(p3, &value[2], NULL);

   fail_if(!donea.then || donea.cancel || donea.progress);
   fail_if(all);

   eo_del(p1);
   eo_del(p2);
   eo_del(p3);

   ecore_shutdown();
}
END_TEST

void ecore_test_ecore_promise(TCase *tc)
{
   tcase_add_test(tc, ecore_test_promise);
   tcase_add_test(tc, ecore_test_promise_error);
   tcase_add_test(tc, ecore_test_promise_all);
   tcase_add_test(tc, ecore_test_promise_all_then_then);
   tcase_add_test(tc, ecore_test_promise_then_after_thread_finished);
   tcase_add_test(tc, ecore_test_promise_then_after_thread_finished_all);
   tcase_add_test(tc, ecore_test_promise_normal_lifetime);
   tcase_add_test(tc, ecore_test_promise_normal_lifetime_all);
   tcase_add_test(tc, ecore_test_promise_immediate_set_lifetime);
   tcase_add_test(tc, ecore_test_promise_immediate_set_lifetime_all);
   tcase_add_test(tc, ecore_test_promise_cancel_promise);
   tcase_add_test(tc, ecore_test_promise_progress_promise);

   tcase_add_test(tc, efl_test_promise_future_success);
   tcase_add_test(tc, efl_test_promise_future_cancel);
   tcase_add_test(tc, efl_test_promise_future_chain_success);
   tcase_add_test(tc, efl_test_promise_future_chain_cancel);
   tcase_add_test(tc, efl_test_promise_before_future_success);
   tcase_add_test(tc, efl_test_promise_before_future_cancel);
   tcase_add_test(tc, efl_test_promise_future_multi_success);
   tcase_add_test(tc, efl_test_promise_future_multi_cancel);
   tcase_add_test(tc, efl_test_promise_before_future_multi_success);
   tcase_add_test(tc, efl_test_promise_before_future_multi_cancel);
   tcase_add_test(tc, efl_test_promise_future_optional_success);
   tcase_add_test(tc, efl_test_promise_future_optional_cancel);
   tcase_add_test(tc, efl_test_promise_all);
}
