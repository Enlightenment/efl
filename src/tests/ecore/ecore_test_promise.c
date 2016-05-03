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

   ecore_thread_promise_run(&promised_thread, NULL, NULL, 0, &promise);
   eina_promise_then(promise, &promise_callback, NULL, NULL);

   ecore_main_loop_begin();

   ecore_shutdown();
}
END_TEST

void promise_error_thread(const void* data EINA_UNUSED, Eina_Promise_Owner* promise, Ecore_Thread* thread EINA_UNUSED)
{
  eina_promise_owner_error_set(promise, EINA_ERROR_OUT_OF_MEMORY);
}

void promise_error_callback(void* data EINA_UNUSED, Eina_Error const* error)
{
  ck_assert(*error == EINA_ERROR_OUT_OF_MEMORY);
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_error)
{
   Eina_Promise* promise;

   ecore_init();

   ecore_thread_promise_run(&promise_error_thread, NULL, NULL, 0, &promise);
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

   ecore_thread_promise_run(&promised_thread, NULL, NULL, 0, &first[0]);
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

   ecore_thread_promise_run(&promised_thread, NULL, NULL, 0, &first[0]);
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

   thread = ecore_thread_promise_run((Ecore_Thread_Promise_Cb)&promised_exit_thread, NULL, &data, 0, &promise);

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

   thread = ecore_thread_promise_run((Ecore_Thread_Promise_Cb)&promised_exit_thread, NULL, &data, 0, &first[0]);
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

void promised_block_thread(const void* data EINA_UNUSED, Eina_Promise_Owner* promise, Ecore_Thread* thread EINA_UNUSED)
{
   struct timespec v = {.tv_sec = 1, .tv_nsec = 0}, rem;
   if(nanosleep(&v, &rem) == -1 && errno == EINTR)
     do
       {
         v = rem;
       }
     while(nanosleep(&v, &rem) == -1 && errno == EINTR);

  int r = 10;
  eina_promise_owner_value_set(promise, &r, NULL);
}

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

   promise_owner = eina_promise_default_add(0);

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

   promise_owner = eina_promise_default_add(0);
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

   owner = eina_promise_default_add(0);
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

   owner = eina_promise_default_add(0);
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

static void _cancel_callback(const void* data, Eina_Promise_Owner* promise EINA_UNUSED, Ecore_Thread* thread EINA_UNUSED)
{
  _condition_var* v = (void*)data;
  
  eina_lock_take(&v->lock);
  v->boolean = EINA_TRUE;
  eina_condition_broadcast(&v->condvar);
  eina_lock_release(&v->lock);
}

static void _cancel_promise_callback(void* data EINA_UNUSED, Eina_Error const* value)
{
  ck_assert(!!value);
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_cancel_promise)
{
   _condition_var v = {.boolean = EINA_FALSE};
   Eina_Promise* promise;
   
   ecore_init();

   eina_lock_new(&v.lock);
   eina_condition_new(&v.condvar, &v.lock);

   ecore_thread_promise_run(&promise_cancel_thread, &_cancel_callback, &v, 0, &promise);
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

   ecore_thread_promise_run(&promise_progress_thread, NULL, NULL, 0, &promise);

   eina_promise_progress_cb_add(promise, &_progress_callback, NULL);

   ecore_main_loop_begin();

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
}
