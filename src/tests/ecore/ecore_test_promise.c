#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_suite.h"
#include <time.h>

void promised_thread(const void* data EINA_UNUSED, Ecore_Promise* promise)
{
  fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
  ecore_promise_value_set(promise, NULL);
}

void promise_callback(void* data EINA_UNUSED, void* value EINA_UNUSED)
{
  fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
   Ecore_Promise* promise = ecore_promise_thread_run(&promised_thread, NULL, 0);
   ecore_promise_then(promise, &promise_callback, NULL);
   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

void promise_error_thread(const void* data EINA_UNUSED, Ecore_Promise* promise)
{
  fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
  ecore_promise_error_set(promise, EINA_ERROR_OUT_OF_MEMORY);
}

void promise_error_callback(void* data EINA_UNUSED, void* value EINA_UNUSED)
{
  fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_error)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
   Ecore_Promise* promise = ecore_promise_thread_run(&promise_error_thread, NULL, 0);
   ecore_promise_then(promise, &promise_error_callback, NULL);
   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_promise_all)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
   Ecore_Promise* first[2] = { ecore_promise_thread_run(&promised_thread, NULL, 0), NULL };
   Ecore_Promise* promise = ecore_promise_all(eina_carray_iterator_new((void**)&first[0]));
   ecore_promise_then(promise, &promise_callback, NULL);
   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

void promise_callback2(void* data, void* value EINA_UNUSED)
{
  fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
  if(++(*(int*)data) == 2)
    ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_all_then_then)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   int i = 0;
   
   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
   Ecore_Promise* first[2] = { ecore_promise_thread_run(&promised_thread, NULL, 0), NULL };
   ecore_promise_then(first[0], &promise_callback2, &i);
   Ecore_Promise* promise = ecore_promise_all(eina_carray_iterator_new((void**)&first[0]));
   ecore_promise_then(promise, &promise_callback2, &i);
   fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

struct sync_data
{
  Eina_Lock lock;
  Eina_Condition cond;
  Eina_Bool var;
};

void promised_exit_thread(struct sync_data* data EINA_UNUSED, Ecore_Promise* promise)
{
  fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); fflush(stderr);
  ecore_promise_value_set(promise, NULL);
  eina_lock_take(&data->lock);
  data->var = EINA_TRUE;
  eina_condition_broadcast(&data->cond);
  eina_lock_release(&data->lock);
}

static void _ecore_test_promise_then_after_thread_finished_main_cb()
{
   struct sync_data data;
   data.var = EINA_FALSE;
   eina_lock_new(&data.lock);
   eina_condition_new(&data.cond, &data.lock);

   Ecore_Promise* promise = ecore_promise_thread_run((Ecore_Promise_Thread_Cb)&promised_exit_thread, &data, 0);

   eina_lock_take(&data.lock);
   while(!data.var)
     {
       eina_condition_wait(&data.cond);
     }
   eina_lock_release(&data.lock);
   ecore_promise_then(promise, &promise_callback, NULL);
}

START_TEST(ecore_test_promise_then_after_thread_finished)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_job_add(&_ecore_test_promise_then_after_thread_finished_main_cb, NULL);
   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

static void _ecore_test_promise_then_after_thread_finished_all_main_cb()
{
   struct sync_data data;
   data.var = EINA_FALSE;
   eina_lock_new(&data.lock);
   eina_condition_new(&data.cond, &data.lock);

   Ecore_Promise* first[] = {ecore_promise_thread_run((Ecore_Promise_Thread_Cb)&promised_exit_thread, &data, 0), NULL};
   Ecore_Promise* promise = ecore_promise_all(eina_carray_iterator_new((void**)&first[0]));

   eina_lock_take(&data.lock);
   while(!data.var)
     {
       eina_condition_wait(&data.cond);
     }
   eina_lock_release(&data.lock);
   ecore_promise_then(promise, &promise_callback, NULL);
}

START_TEST(ecore_test_promise_then_after_thread_finished_all)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_job_add(&_ecore_test_promise_then_after_thread_finished_all_main_cb, NULL);
   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

void promised_block_thread(const void* data EINA_UNUSED, Ecore_Promise* promise)
{
   struct timespec v = {.tv_sec = 1, .tv_nsec = 0}, rem;
   if(nanosleep(&v, &rem) == -1 && errno == EINTR)
     do
       {
         v = rem;
       }
     while(nanosleep(&v, &rem) == -1 && errno == EINTR);

  int r = 10;
  ecore_promise_value_set(promise, &r);
}

static void
_ecore_test_promise_blocking_get_quit_cb(void* data EINA_UNUSED)
{
  ecore_main_loop_quit();
}

static void
_ecore_test_promise_blocking_get_main_cb(void* data EINA_UNUSED)
{
   Ecore_Promise* promise = ecore_promise_thread_run(&promised_block_thread, NULL, sizeof(int));
   const void* value = ecore_promise_value_get(promise);
   ck_assert(*(int*)value == 10);

   ecore_job_add(&_ecore_test_promise_blocking_get_quit_cb, NULL);
}

START_TEST(ecore_test_promise_blocking_get)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_job_add(&_ecore_test_promise_blocking_get_main_cb, NULL);
   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

static void
_ecore_test_promise_blocking_get_all_value_get_cb(Ecore_Promise* promise, Ecore_Thread* thread EINA_UNUSED)
{
   Eina_Iterator** iterator = ecore_promise_value_get(promise);
   int* v;
   ck_assert(eina_iterator_next(*iterator, (void**)&v));
   ck_assert(*v == 10);
   ecore_main_loop_quit();
}

static void
_ecore_test_promise_blocking_get_all_main_cb(void* data EINA_UNUSED)
{
   Ecore_Promise* first[2] = {ecore_promise_thread_run(&promised_block_thread, NULL, sizeof(int)), NULL};
   Ecore_Promise* promise = ecore_promise_all(eina_carray_iterator_new((void**)&first[0]));

   ecore_thread_run((Ecore_Thread_Cb)&_ecore_test_promise_blocking_get_all_value_get_cb, NULL, NULL, promise);
}

START_TEST(ecore_test_promise_blocking_get_all)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   ecore_job_add(&_ecore_test_promise_blocking_get_all_main_cb, NULL);
   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

static void
_ecore_test_promise_normal_lifetime_cb(void* data EINA_UNUSED, void* value EINA_UNUSED)
{
  ecore_main_loop_quit();
}

START_TEST(ecore_test_promise_normal_lifetime)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   Ecore_Promise* promise = ecore_promise_add(0);

   ecore_promise_then(promise, &_ecore_test_promise_normal_lifetime_cb, NULL);
   ecore_promise_value_set(promise, NULL);
   
   ecore_main_loop_begin();

   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_promise_normal_lifetime_all)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   Ecore_Promise* first[2] = {ecore_promise_add(0), NULL};
   Ecore_Promise* promise = ecore_promise_all(eina_carray_iterator_new((void**)&first[0]));

   ecore_promise_then(promise, &_ecore_test_promise_normal_lifetime_cb, NULL);
   ecore_promise_value_set(promise, NULL);
   
   ecore_main_loop_begin();

   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
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
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   Ecore_Promise* promise = ecore_promise_add(0);

   ecore_promise_value_set(promise, NULL);
   ecore_promise_then(promise, &_ecore_test_promise_immediate_set_lifetime_cb, NULL);

   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_promise_immediate_set_lifetime_all)
{
   ecore_init();
   fprintf(stderr, "%s:%d %s ---------- BEGIN test\n", __FILE__, __LINE__, __func__); fflush(stderr);

   Ecore_Promise* first[2] = {ecore_promise_add(0), NULL};
   Ecore_Promise* promise = ecore_promise_all(eina_carray_iterator_new((void**)&first[0]));

   ecore_promise_value_set(first[0], NULL);
   ecore_promise_then(promise, &_ecore_test_promise_immediate_set_lifetime_cb, NULL);

   ecore_main_loop_begin();
   
   fprintf(stderr, "%s:%d %s ---------- END test\n", __FILE__, __LINE__, __func__); fflush(stderr);
   ecore_shutdown();
}
END_TEST

void ecore_test_ecore_promise(TCase *tc EINA_UNUSED)
{
   tcase_add_test(tc, ecore_test_promise);
   tcase_add_test(tc, ecore_test_promise_error);
   tcase_add_test(tc, ecore_test_promise_all);
   tcase_add_test(tc, ecore_test_promise_all_then_then);
   tcase_add_test(tc, ecore_test_promise_then_after_thread_finished);
   tcase_add_test(tc, ecore_test_promise_then_after_thread_finished_all);
   tcase_add_test(tc, ecore_test_promise_blocking_get);
   tcase_add_test(tc, ecore_test_promise_blocking_get_all);
   tcase_add_test(tc, ecore_test_promise_normal_lifetime);
   tcase_add_test(tc, ecore_test_promise_normal_lifetime_all);
   tcase_add_test(tc, ecore_test_promise_immediate_set_lifetime);
   tcase_add_test(tc, ecore_test_promise_immediate_set_lifetime_all);
}
