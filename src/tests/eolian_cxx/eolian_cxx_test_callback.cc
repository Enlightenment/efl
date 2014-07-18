
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
  assert(called1 == true);
  assert(called2 == true);
}
END_TEST

START_TEST(eolian_cxx_test_callback_method)
{
  efl::eo::eo_init i;

  callback c;

  bool called1 = false, called2 = false;

  c.onecallback(std::bind([&called1] { called1 = true; }));
  c.twocallback(std::bind([&called2] { called2 = true; }), &foo);

  assert(called1 == true);
  assert(called2 == true);
}
END_TEST

void
eolian_cxx_test_callback(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_callback_constructor);
   tcase_add_test(tc, eolian_cxx_test_callback_method);
}
