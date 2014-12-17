
// Test Eolian-Cxx wrappers

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <callback.eo.hh>

#include <check.h>

START_TEST(eolian_cxx_test_wrapper_size)
{
  efl::eo::eo_init init;

  ::efl::eo::concrete b(nullptr);
  ::callback c;

  fail_if(sizeof(b) != sizeof(Eo*));
  fail_if(sizeof(b) != sizeof(c));
}
END_TEST

void
eolian_cxx_test_wrapper(TCase* tc)
{
  tcase_add_test(tc, eolian_cxx_test_wrapper_size);
}
