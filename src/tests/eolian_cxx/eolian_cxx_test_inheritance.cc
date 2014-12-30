
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <simple.eo.hh>

#include <check.h>

struct bar
: efl::eo::inherit<bar, simple>
{
  bar()
    : inherit_base(efl::eo::parent = nullptr)
  {}

  bool simple_get()
  {
     printf("calling bar::%s\n", __FUNCTION__);
     return false;
  }
};

void foo(simple is)
{
   fail_if(is.simple_get());
}

START_TEST(eolian_cxx_test_inheritance_simple)
{
  efl::eo::eo_init i;
  bar b;
  foo(b);
}
END_TEST

void
eolian_cxx_test_inheritance(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_inheritance_simple);
}
