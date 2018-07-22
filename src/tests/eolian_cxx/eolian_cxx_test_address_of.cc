#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.hh>

#include <a.eo.hh>
#include <b.eo.hh>
#include <c.eo.hh>
#include <d.eo.hh>

#include "eolian_cxx_suite.h"

EFL_START_TEST(eolian_cxx_test_addess_of_conversions)
{
   efl::eo::eo_init init;

   nonamespace::D d_obj;

   nonamespace::A* a_ptr = &d_obj;
   nonamespace::B* b_ptr = &d_obj;
   nonamespace::C* c_ptr = &d_obj;
   nonamespace::D* d_ptr = &d_obj;

   fail_unless(a_ptr == (void*) b_ptr);
   fail_unless(a_ptr == (void*) c_ptr);
   fail_unless(a_ptr == (void*) d_ptr);
}
EFL_END_TEST

void
eolian_cxx_test_address_of(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_addess_of_conversions);
}
