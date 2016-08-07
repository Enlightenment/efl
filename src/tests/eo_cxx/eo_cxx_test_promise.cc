#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>
#include <Eo.hh>
#include <Ecore.hh>

#include "eo_cxx_suite.h"

START_TEST(eo_cxx_future_construct_and_destroy)
{
   Efl_Promise *p;
   Efl_Future *f;

   ecore_init();

   {
      p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
      fail_if(!p);
  
      f = efl_promise_future_get(p);
      fail_if(!f);

      efl::shared_future<int> future(efl_ref(f));
   }
   ecore_shutdown();
}
END_TEST

START_TEST(eo_cxx_future_wait)
{
   Efl_Promise *p;
   Efl_Future *f;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);
  
   f = efl_promise_future_get(p);
   fail_if(!f);

   {
      efl::shared_future<int> future(efl_ref(f));

      std::thread thread([&]
                         {
                            efl::ecore::main_loop_thread_safe_call_sync([]{}); // wait for ecore_main_loop_begin() call to start
                            efl::ecore::main_loop_thread_safe_call_async
                              ([&]
                               {
                                  int* i = static_cast<int*>(malloc(sizeof(int)));
                                  *i = 5;
                                  efl_promise_value_set(p, i, & ::free);
                               });
                         
                            future.wait();
                            efl::ecore::main_loop_thread_safe_call_sync([] { ecore_main_loop_quit(); });
                         });

      ecore_main_loop_begin();
   
      thread.join();
   }
   ecore_shutdown();
}
END_TEST

START_TEST(eo_cxx_future_get)
{
   Efl_Promise *p;
   Efl_Future *f;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);
  
   f = efl_promise_future_get(p);
   fail_if(!f);

   {
      efl::shared_future<int> future(efl_ref(f));

      std::thread thread([&]
                         {
                            efl::ecore::main_loop_thread_safe_call_sync([]{}); // wait for ecore_main_loop_begin() call to start
                            efl::ecore::main_loop_thread_safe_call_async
                              ([&]
                               {
                                  int* i = static_cast<int*>(malloc(sizeof(int)));
                                  *i = 5;
                                  efl_promise_value_set(p, i, & ::free);
                               });
                         
                            int i = future.get();
                            ck_assert_int_eq(i, 5);
                            efl::ecore::main_loop_thread_safe_call_sync([] { ecore_main_loop_quit(); });
                         });

      ecore_main_loop_begin();
   
      thread.join();
   }
   ecore_shutdown();
}
END_TEST

START_TEST(eo_cxx_future_get_error)
{
   Efl_Promise *p;
   Efl_Future *f;

   ecore_init();

   p = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!p);
  
   f = efl_promise_future_get(p);
   fail_if(!f);

   {
      efl::shared_future<int> future(efl_ref(f));

      std::thread thread([&]
                         {
                            efl::ecore::main_loop_thread_safe_call_sync([]{}); // wait for ecore_main_loop_begin() call to start
                            efl::ecore::main_loop_thread_safe_call_async
                              ([&]
                               {
                                  efl_promise_failed_set(p, EINA_ERROR_OUT_OF_MEMORY);
                               });

                            try {
                               future.get();
                               ck_abort_msg("Execution should not have continued, exception expected");
                            }
                            catch(std::system_error const& e)
                              {
                              }
                            efl::ecore::main_loop_thread_safe_call_sync([] { ecore_main_loop_quit(); });
                         });

      ecore_main_loop_begin();
   
      thread.join();
   }
   ecore_shutdown();
}
END_TEST

START_TEST(eo_cxx_future_then_value)
{
   Efl_Promise *promise;
   Efl_Future *f;

   ecore_init();

   promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   fail_if(!promise);
  
   f = efl_promise_future_get(promise);
   fail_if(!f);

   {
      efl::shared_future<int> future(efl_ref(f)), rfuture;

      
      std::thread thread
        ([&]
         {
            efl::ecore::main_loop_thread_safe_call_sync
              ([&]
               {
                  rfuture = then
                    (future, [] (int i) -> int
                     {
                        ck_assert_int_eq(i, 5);
                        return 42;
                     }, [] (std::error_code)
                     {
                        throw std::bad_alloc();
                     });
               });
            efl::ecore::main_loop_thread_safe_call_async
              ([&]
               {
                  int* i = static_cast<int*>(malloc(sizeof(int)));
                  *i = 5;
                  efl_promise_value_set(promise, i, &::free);
               });

            int i = rfuture.get();
            ck_assert_int_eq(i, 42);
            efl::ecore::main_loop_thread_safe_call_sync([] { ecore_main_loop_quit(); });
         });

      ecore_main_loop_begin();
      thread.join();
   }
   ecore_shutdown();
}
END_TEST

START_TEST(eo_cxx_future_composite_construct_and_destroy)
{
   ecore_init();

   {
      Efl_Promise *p1 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
      fail_if(!p1);
  
      Efl_Future *f1 = efl_promise_future_get(p1);
      fail_if(!f1);

      Efl_Promise *p2 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
      fail_if(!p1);
  
      Efl_Future *f2 = efl_promise_future_get(p2);
      fail_if(!f2);

      Efl_Future *f3 = efl_future_all(f1, f2);
      fail_if(!f3);
      
      efl::shared_future<int> future1(efl_ref(f1))
        , future2(efl_ref(f2));
      efl::shared_future<int, int> future3(efl_ref(f3));
   }
   ecore_shutdown();
}
END_TEST

START_TEST(eo_cxx_future_composite_wait)
{
   ecore_init();

   {
      Efl_Promise *p1 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
      fail_if(!p1);
  
      Efl_Future* f1 = efl_promise_future_get(p1);
      fail_if(!f1);

      Efl_Promise *p2 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
      fail_if(!p2);
  
      Efl_Future* f2 = efl_promise_future_get(p2);
      fail_if(!f2);

      Efl_Future *f3 = efl_future_all(f1, f2);
      fail_if(!f3);
      
      efl::shared_future<int> future1(efl_ref(f1))
        , future2(efl_ref(f2));
      efl::shared_future<int, int> future3(efl_ref(f3));

      std::thread thread([&]
                         {
                            efl::ecore::main_loop_thread_safe_call_sync([]{}); // wait for ecore_main_loop_begin() call to start
                            efl::ecore::main_loop_thread_safe_call_async
                              ([&]
                               {
                                  int* i1 = static_cast<int*>(malloc(sizeof(int)));
                                  *i1 = 5;
                                  efl_promise_value_set(p1, i1, & ::free);
                                  int* i2 = static_cast<int*>(malloc(sizeof(int)));
                                  *i2 = 42;
                                  efl_promise_value_set(p2, i2, & ::free);
                               });

                            future3.wait();
                            efl::ecore::main_loop_thread_safe_call_sync([] { ecore_main_loop_quit(); });
                         });

      ecore_main_loop_begin();
   
      thread.join();
   }
   ecore_shutdown();
}
END_TEST

START_TEST(eo_cxx_future_composite_get)
{
   ecore_init();

   {
      Efl_Promise *p1 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
      fail_if(!p1);
  
      Efl_Future *f1 = efl_promise_future_get(p1);
      fail_if(!f1);

      Efl_Promise *p2 = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
      fail_if(!p2);
  
      Efl_Future *f2 = efl_promise_future_get(p2);
      fail_if(!f2);
      
      Efl_Future *f3 = efl_future_all(f1, f2);
      fail_if(!f3);
      
      efl::shared_future<int> future1(efl_ref(f1))
        , future2(efl_ref(f2));
      efl::shared_future<int, int> future3(efl_ref(f3));

      std::thread thread([&]
                         {
                            efl::ecore::main_loop_thread_safe_call_sync([]{}); // wait for ecore_main_loop_begin() call to start
                            efl::ecore::main_loop_thread_safe_call_async
                              ([&]
                               {
                                  int* i1 = static_cast<int*>(malloc(sizeof(int)));
                                  *i1 = 5;
                                  efl_promise_value_set(p1, i1, & ::free);
                                  int* i2 = static_cast<int*>(malloc(sizeof(int)));
                                  *i2 = 42;
                                  efl_promise_value_set(p2, i2, & ::free);
                               });
                         
                            std::tuple<int, int> tuple = future3.get();
                            ck_assert_int_eq(std::get<0>(tuple), 5);
                            ck_assert_int_eq(std::get<1>(tuple), 42);
                            efl::ecore::main_loop_thread_safe_call_sync([] { ecore_main_loop_quit(); });
                         });

      ecore_main_loop_begin();
   
      thread.join();
   }
   ecore_shutdown();
}
END_TEST

void
eo_cxx_test_promise(TCase* tc)
{
  tcase_add_test(tc, eo_cxx_future_construct_and_destroy);
  tcase_add_test(tc, eo_cxx_future_wait);
  tcase_add_test(tc, eo_cxx_future_get);
  tcase_add_test(tc, eo_cxx_future_get_error);
  tcase_add_test(tc, eo_cxx_future_then_value);
  tcase_add_test(tc, eo_cxx_future_composite_construct_and_destroy);
  tcase_add_test(tc, eo_cxx_future_composite_wait);
  tcase_add_test(tc, eo_cxx_future_composite_get);
}
