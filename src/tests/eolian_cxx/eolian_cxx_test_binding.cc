#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include <generic.eo.hh>
#include <generic.eo.impl.hh>
#include <name1_name2_type_generation.eo.hh>
#include <name1_name2_type_generation.eo.impl.hh>

#include "eolian_cxx_suite.h"

EFL_START_TEST(eolian_cxx_test_binding_constructor_only_required)
{
  efl::eo::eo_init init;

  nonamespace::Generic g
    (efl::eo::instantiate,
     [&]
     {
       g.required_ctor_a(1);
       g.required_ctor_b(2);
     }
    );

  ck_assert_int_eq(1, g.req_ctor_a_value_get());
  ck_assert_int_eq(2, g.req_ctor_b_value_get());
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_binding_constructor_all_optionals)
{
  efl::eo::eo_init i;

  nonamespace::Generic g
    (efl::eo::instantiate,
    [&]
    {
      g.required_ctor_a(2);
      g.required_ctor_b(4);
      g.optional_ctor_a(3);
      g.optional_ctor_b(5);
    }
    );

  nonamespace::Generic g2
    (efl::eo::instantiate, g,
    [&]
    {
      g.required_ctor_a(2);
      g.required_ctor_b(4);
      g.optional_ctor_a(3);
      g.optional_ctor_b(5);
    }
    );
  
  ck_assert_int_eq(2, g.req_ctor_a_value_get());
  ck_assert_int_eq(3, g.opt_ctor_a_value_get());
  ck_assert_int_eq(4, g.req_ctor_b_value_get());
  ck_assert_int_eq(5, g.opt_ctor_b_value_get());
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_type_generation)
{
  efl::eo::eo_init eo_init;

  name1::name2::Type_Generation g1(efl::eo::instantiate);
  name1::name2::Type_Generation g2(efl::eo::instantiate
                                   , [] {});
  name1::name2::Type_Generation g3(efl::eo::instantiate
                                   , [] (name1::name2::Type_Generation) {});
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_type_generation_in)
{
  efl::eo::eo_init i;

  name1::name2::Type_Generation g(efl::eo::instantiate);

  int v = 42;
  g.inrefint(v);
  g.inrefintown(42);
  g.inrefintownfree(42);
  g.invoidptr(nullptr);
  g.inint(42);
  g.inintptr(42);
  g.inintptrown(42);
  g.inintptrownfree(42);
  g.instring("foobar");
  g.instringown("foobar");
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_type_generation_return)
{
  efl::eo::eo_init i;

  name1::name2::Type_Generation g(efl::eo::instantiate);

  {
    int&i = g.returnrefint();
    ck_assert(i == 42);
  }
  {
    int i = g.returnint();
    ck_assert(i == 42);
  }
  {
    void* p = g.returnvoidptr();
    ck_assert(*(int*)p == 42);
  }
  {
    int& p = g.returnintptr();
    ck_assert(p == 42);
  }
  {
    efl::eina::unique_malloc_ptr<int> p = g.returnintptrown();
    ck_assert(*p == 42);
  }
  {
    efl::eina::string_view string = g.returnstring();
    ck_assert_str_eq(string.c_str(), "foobar");
  }
  {
    std::string string = g.returnstring();
    ck_assert_str_eq(string.c_str(), "foobar");
  }
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_type_generation_optional)
{
  efl::eo::eo_init init;

  using efl::eina::optional;

  name1::name2::Type_Generation g(efl::eo::instantiate);

  g.optionalinvoidptr(NULL);
  g.optionalinvoidptr(&g);
  g.optionalinvoidptr(nullptr);

  int i = 42;
  g.optionalinint(nullptr);
  g.optionalinint(i);

  g.optionalinintptr(i);
  g.optionalinintptr(nullptr);

  g.optionalinintptrown(i);
  g.optionalinintptrown(nullptr);

  g.optionalinintptrownfree(i);
  g.optionalinintptrownfree(nullptr);
  
  i = 0;
  g.optionaloutint(&i);
  ck_assert(i == 42);
  g.optionaloutint(nullptr);

  i = 0;
  int* j = nullptr;
  g.optionaloutintptr(&j);
  ck_assert(j != nullptr);
  ck_assert(*j == 42);
  g.optionaloutintptr(nullptr);

  i = 0;
  efl::eina::unique_malloc_ptr<int> k = nullptr;
  g.optionaloutintptrown(k);
  ck_assert(!!k);
  ck_assert(*k == 42);
  g.optionaloutintptrown(nullptr);
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_type_callback)
{
  efl::eo::eo_init i;

  bool event1 = false, event2 = false, event3 = false, event4 = false
    , event5 = false;
  
  nonamespace::Generic g(efl::eo::instantiate);
  efl::eolian::event_add(g.prefix_event1_event, g, [&] (nonamespace::Generic)
                         {
                           event1 = true;
                         });
  efl::eolian::event_add(g.prefix_event2_event, g, [&] (nonamespace::Generic, nonamespace::Generic)
                         {
                           event2 = true;
                         });
  efl::eolian::event_add(g.prefix_event3_event, g, [&] (nonamespace::Generic, int v)
                         {
                           event3 = true;
                           ck_assert(v == 42);
                         });
  efl::eolian::event_add(g.prefix_event4_event, g, [&] (nonamespace::Generic, efl::eina::range_list<const int &> e)
                         {
                           event4 = true;
                           ck_assert(e.size() == 1);
                           ck_assert(*e.begin() == 42);
                         });
  efl::eolian::event_add(g.prefix_event5_event, g, [&] (nonamespace::Generic, Generic_Event)
                         {
                           event5 = true;
                         });

  g.call_event1();
  g.call_event2();
  g.call_event3();
  g.call_event4();
  g.call_event5();

  ck_assert(event1);
  ck_assert(event2);
  ck_assert(event3);
  ck_assert(event4);
  ck_assert(event5);
}
EFL_END_TEST

void
eolian_cxx_test_binding(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_binding_constructor_only_required);
   tcase_add_test(tc, eolian_cxx_test_binding_constructor_all_optionals);
   tcase_add_test(tc, eolian_cxx_test_type_generation);
   tcase_add_test(tc, eolian_cxx_test_type_generation_in);
   tcase_add_test(tc, eolian_cxx_test_type_generation_return);
   tcase_add_test(tc, eolian_cxx_test_type_generation_optional);
   tcase_add_test(tc, eolian_cxx_test_type_callback);
}
