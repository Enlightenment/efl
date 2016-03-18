#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <generic.eo.hh>

#include "eolian_cxx_suite.h"

START_TEST(eolian_cxx_test_binding_constructor_only_required)
{
  efl::eo::eo_init i;

  bool called1 = false;

  nonamespace::Generic g(
    g.required_ctor_a(1),
    g.required_ctor_b(std::bind([&called1] { called1 = true; }))
  );

  g.call_req_ctor_b_callback();
  g.call_opt_ctor_b_callback();

  fail_if(!called1);
  fail_if(1 != g.req_ctor_a_value_get());
}
END_TEST

START_TEST(eolian_cxx_test_binding_constructor_all_optionals)
{
  efl::eo::eo_init i;

  bool called1 = false;
  bool called2 = false;

  nonamespace::Generic g(
    g.required_ctor_a(2),
    g.required_ctor_b(std::bind([&called1] { called1 = true; })),
    g.optional_ctor_a(3),
    g.optional_ctor_b(std::bind([&called2] { called2 = true; }))
  );

  g.call_req_ctor_b_callback();
  g.call_opt_ctor_b_callback();

  fail_if(!called1);
  fail_if(!called2);
  fail_if(2 != g.req_ctor_a_value_get());
  fail_if(3 != g.opt_ctor_a_value_get());
}
END_TEST

void
eolian_cxx_test_binding(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_binding_constructor_only_required);
   tcase_add_test(tc, eolian_cxx_test_binding_constructor_all_optionals);
}
