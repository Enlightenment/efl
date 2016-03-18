#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

#include <Eo.h>
#include <Ecore.h>

#include <callback.eo.hh>

#include "eolian_cxx_suite.h"

void foo(void*) {}

START_TEST(eolian_cxx_test_callback_method)
{
  efl::eo::eo_init i;

  nonamespace::Callback c;

  bool called1 = false, called2 = false;

  c.onecallback(std::bind([&called1] { called1 = true; }));
  c.twocallback(std::bind([&called2] { called2 = true; }), &foo);

  fail_if(!called1);
  fail_if(!called2);
}
END_TEST

START_TEST(eolian_cxx_test_callback_event_add)
{
  efl::eo::eo_init i;

  nonamespace::Callback c;

  bool called1 = false, called2 = false;


  c.callback_call_on_add_add(std::bind([&called1] { called1 = true; }));
  c.callback_call_on_add_add(std::bind([&called2] { called2 = true; }));

  fail_if(!called1);
  fail_if(!called2);
}
END_TEST

START_TEST(eolian_cxx_test_callback_event_del)
{
  efl::eo::eo_init i;

  nonamespace::Callback c;

  int called1 = 0, called2 = 0, called3 = 0, called4 = 0;

  efl::eo::signal_connection s1 = c.callback_call_on_add_add
    (std::bind([&]
     {
       std::cerr << "called1 " << called1 << " called2 " << called2 << " called3 " << called3
                 << " called4 " << called4 << " 1" << std::endl;
       fail_if(!(    (called1 == 0 && called2 == 0 && called3 == 0 && called4 == 0)
                  || (called1 == 1 && called2 == 1 && called3 == 0 && called4 == 0)
               ));
       ++called1;
     }));
  efl::eo::signal_connection s2 = c.callback_call_on_add_add
    (std::bind([&]
     {
       std::cerr << "called1 " << called1 << " called2 " << called2 << " called3 " << called3
                 << " called4 " << called4 << " 2" << std::endl;
       fail_if(!(    (called1 == 1 && called2 == 0 && called3 == 0 && called4 == 0)
                  || (called1 == 1 && called2 == 1 && called3 == 0 && called4 == 0)
                  || (called1 == 2 && called2 == 1 && called3 == 1 && called4 == 0)
              ));
       ++called2;
     }));

  s1.disconnect();

  c.callback_call_on_add_add
    (
     std::bind([&]
     {
       std::cerr << "called1 " << called1 << " called2 " << called2 << " called3 " << called3
                 << " called4 " << called4 << " 3" << std::endl;
       fail_if(!(    (called1 == 2 && called2 == 1 && called3 == 0 && called4 == 0)
                  || (called1 == 2 && called2 == 2 && called3 == 1 && called4 == 1)
              ));
       ++called3;
     }));

  s2.disconnect();

  c.callback_call_on_add_add
    (
     std::bind([&]
     {
       std::cerr << "called1 " << called1 << " called2 " << called2 << " called3 " << called3
                 << " called4 " << called4 << " 4" << std::endl;
       fail_if(!(    (called1 == 2 && called2 == 2 && called3 == 1 && called4 == 0)
              ));
       ++called4;
     }));

  fail_if(called1 != 2);
  fail_if(called2 != 2);
  fail_if(called3 != 2);
  fail_if(called4 != 1);
}
END_TEST

START_TEST(eolian_cxx_test_global_callback)
{
  efl::eo::eo_init i;

  bool called = false;

  nonamespace::Callback::test_global_callbacks(std::bind([&called] { called = true; }));

  fail_if(!called);
}
END_TEST

START_TEST(eolian_cxx_test_disconnect_inside_callback)
{
  efl::eo::eo_init i;
  nonamespace::Callback c;

  std::vector<long> capture_me;
  int times_called = 0;

  ::efl::eo::signal_connection sig(nullptr);
  sig = c.callback_callback_add_add(
           std::bind([&sig, &capture_me, &times_called](void *info)
             {
               ++times_called;
               std::cout << "times_called: " << times_called << std::endl;
               std::cout << "&sig: " << &sig << std::endl;
               if (times_called <= 1)
                 return;

               sig.disconnect();

               long* info_l = static_cast<long*>(info);
               std::cout << "info: " << info << std::endl;
               std::cout << "*info_l: " << *info_l << std::endl;

               fail_if(*info_l != 42);

               capture_me = {9, 0, 8, 1, 7, 2, 6, 3, 5, 4};
               std::sort(capture_me.begin(), capture_me.end());

               capture_me[0] = capture_me[1] + +capture_me[2] + capture_me[9];

               std::cout << "&capture_me: " << &capture_me << std::endl;
               std::cout << "capture_me [0] [9]: [" << capture_me[0] << "] ["<< capture_me[9] << "]" << std::endl;

               fail_if(capture_me.size() != 10);
               fail_if(capture_me[0] != 12);
               fail_if(times_called != 2);
             }, std::placeholders::_3));

  long n = 42;
  c.callback_callback_add_call(&n);

  fail_if(capture_me.size() != 10);
  fail_if(capture_me[0] != 12);
  fail_if(times_called != 2);
}
END_TEST

void
eolian_cxx_test_callback(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_callback_method);
   tcase_add_test(tc, eolian_cxx_test_callback_event_add);
   tcase_add_test(tc, eolian_cxx_test_callback_event_del);
   tcase_add_test(tc, eolian_cxx_test_global_callback);
   tcase_add_test(tc, eolian_cxx_test_disconnect_inside_callback);
}
