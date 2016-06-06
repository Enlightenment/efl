#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include <generic.eo.hh>
#include <name1_name2_type_generation.eo.hh>

#include "eolian_cxx_suite.h"

START_TEST(eolian_cxx_test_binding_constructor_only_required)
{
  efl::eo::eo_init i;

  nonamespace::Generic g
    (
     [&]
     {
       g.required_ctor_a(1);
       g.required_ctor_b(2);
     }
    );

  fail_if(1 != g.req_ctor_a_value_get());
  fail_if(2 != g.req_ctor_b_value_get());
}
END_TEST

START_TEST(eolian_cxx_test_binding_constructor_all_optionals)
{
  efl::eo::eo_init i;

  nonamespace::Generic g
    (
    [&]
    {
      g.required_ctor_a(2);
      g.required_ctor_b(4);
      g.optional_ctor_a(3);
      g.optional_ctor_b(5);
    }
    );

  fail_if(2 != g.req_ctor_a_value_get());
  fail_if(3 != g.opt_ctor_a_value_get());
  fail_if(4 != g.req_ctor_b_value_get());
  fail_if(5 != g.opt_ctor_b_value_get());
}
END_TEST

START_TEST(eolian_cxx_test_type_generation)
{
  efl::eo::eo_init eo_init;

  name1::name2::Type_Generation g;

  g.invoidptr(nullptr);
  g.inint(42);
  std::unique_ptr<int> i (new int(42));
  g.inintptr(i.get());
  {
    int* p = (int*)malloc(sizeof(int));
    *p = 42;
    std::unique_ptr<int, void(*)(const void*)> inintptrown(p, (void(*)(const void*))&free);
    g.inintptrown(std::move(inintptrown));
  }
  {
    int** p = (int**)malloc(sizeof(int*));
    *p = (int*)malloc(sizeof(int));
    **p = 42;
    std::unique_ptr<int*, void(*)(const void*)> inintptrownptr(p, (void(*)(const void*))&free);
    g.inintptrownptr(std::move(inintptrownptr));
  }
  {
    int*** p = (int***)malloc(sizeof(int**));
    *p = (int**)malloc(sizeof(int*));
    **p = (int*)malloc(sizeof(int));
    ***p = 42;
    std::unique_ptr<int**, void(*)(const void*)> inintptrownptrptr(p, (void(*)(const void*))&free);
    g.inintptrownptrptr(std::move(inintptrownptrptr));
  }
  {
    int*** p = (int***)malloc(sizeof(int**));
    *p = (int**)malloc(sizeof(int*));
    **p = (int*)malloc(sizeof(int));
    ***p = 42;
    std::unique_ptr<int**, void(*)(const void*)> inintptrptrownptr(p, (void(*)(const void*))&free);
    g.inintptrptrownptr(std::move(inintptrptrownptr));
  }
  {
    int* p = (int*)malloc(sizeof(int));
    *p = 42;
    std::unique_ptr<int, void(*)(const void*)> inintptrownfree(p, (void(*)(const void*))&free);
    g.inintptrownfree(std::move(inintptrownfree));
  }
  g.instring("foobar");
  // {
  //   efl::eina::string_view v("foobar");
  //   g.instringptr(&v);
  // }
  g.instringown("foobar");
  // {
  //   std::string v("foobar");
  //   g.instringptrown(&v);
  // }
  // {
  //   std::unique_ptr<efl::eina::string_view, void(*)(const void*)> v
  //     ((efl::eina::string_view*)malloc(sizeof(string_view)), (void(*)(const void*))&free);
  //   g.instringptrown(v);
  // }
  // {
  //   std::string v("foobar");
  //   g.instringptrown(&v);
  // }
}
END_TEST

START_TEST(eolian_cxx_test_type_generation_in)
{
  efl::eo::eo_init i;

  name1::name2::Type_Generation g;
}
END_TEST

START_TEST(eolian_cxx_test_type_callback)
{
  efl::eo::eo_init i;

  bool event1 = false, event2 = false, event3 = false, event4 = false
    , event5 = false;
  
  nonamespace::Generic g;
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
  efl::eolian::event_add(g.prefix_event4_event, g, [&] (nonamespace::Generic, efl::eina::range_list<int> e)
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
END_TEST

void
eolian_cxx_test_binding(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_binding_constructor_only_required);
   tcase_add_test(tc, eolian_cxx_test_binding_constructor_all_optionals);
   tcase_add_test(tc, eolian_cxx_test_type_generation);
   tcase_add_test(tc, eolian_cxx_test_type_generation_in);
   tcase_add_test(tc, eolian_cxx_test_type_callback);
}
