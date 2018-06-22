#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <a.eo.hh>

#include "eolian_cxx_suite.h"

EFL_START_TEST(eolian_cxx_test_wrapper_size)
{
  efl::eo::eo_init init;

  ::efl::eo::concrete b(nullptr);
  ::nonamespace::A c;

  fail_if(sizeof(b) != sizeof(Eo*));
  fail_if(sizeof(b) != sizeof(c));
}
EFL_END_TEST

void
eolian_cxx_test_wrapper(TCase* tc)
{
  tcase_add_test(tc, eolian_cxx_test_wrapper_size);
}
