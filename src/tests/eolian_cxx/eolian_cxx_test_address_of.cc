
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <a.eo.hh>
#include <b.eo.hh>
#include <c.eo.hh>
#include <d.eo.hh>

#include <check.h>

START_TEST(eolian_cxx_test_addess_of_conversions)
{
   efl::eo::eo_init init;

   d d_obj;

   a* a_ptr = &d_obj;
   b* b_ptr = &d_obj;
   c* c_ptr = &d_obj;
   d* d_ptr = &d_obj;

   fail_unless(a_ptr == (void*) b_ptr);
   fail_unless(a_ptr == (void*) c_ptr);
   fail_unless(a_ptr == (void*) d_ptr);
}
END_TEST

void
eolian_cxx_test_address_of(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_addess_of_conversions);
}
