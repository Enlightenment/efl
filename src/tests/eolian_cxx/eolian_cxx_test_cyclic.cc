#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.hh>

#include <cyclic1.eo.hh>
#include <cyclic2.eo.hh>

#include "eolian_cxx_suite.h"

START_TEST(eolian_cxx_test_cyclic_call)
{
}
END_TEST

void
eolian_cxx_test_cyclic(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_cyclic_call);
}
