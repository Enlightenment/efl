#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cassert>

#include <Eolian_Cxx.hh>

#include "eolian_cxx_suite.h"

START_TEST(eolian_cxx_test_generate_complex_types)
{
   // TODO implement
}
END_TEST

void
eolian_cxx_test_generate(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_generate_complex_types);
}
