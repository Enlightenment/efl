
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <callback.eo.hh>

#include <check.h>

void foo(void*) {}

START_TEST(eolian_cxx_test_callback_constructor)
{
  efl::eo::eo_init i;

  bool called1 = false, called2 = false;
  ::callback c1 (std::bind([&called1] { called1 = true; }));
  ::callback c2 (std::bind([&called2] { called2 = true; }), &foo);
  fail_if(!called1);
  fail_if(!called2);
}
END_TEST

START_TEST(eolian_cxx_test_callback_method)
{
  efl::eo::eo_init i;

  callback c;

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

  callback c;

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

  callback c;

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

void
eolian_cxx_test_callback(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_callback_constructor);
   tcase_add_test(tc, eolian_cxx_test_callback_method);
   tcase_add_test(tc, eolian_cxx_test_callback_event_add);
   tcase_add_test(tc, eolian_cxx_test_callback_event_del);
}
