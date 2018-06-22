#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.hh>

extern "C" {
typedef Eo Cyclic1;
typedef Eo Cyclic2;
}

#include <cyclic1.eo.h>
#include <cyclic2.eo.h>
#include <cyclic1.eo.hh>
#include <cyclic2.eo.hh>

#include "eolian_cxx_suite.h"

EFL_START_TEST(eolian_cxx_test_cyclic_call)
{
}
EFL_END_TEST

void
eolian_cxx_test_cyclic(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_cyclic_call);
}
