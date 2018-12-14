#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include <map>

#include <generic.eo.hh>
#include <generic.eo.impl.hh>
#include <name1_name2_type_generation.eo.hh>
#include <name1_name2_type_generation.eo.impl.hh>

#include "eolian_cxx_suite.h"

#include <Eolian_Cxx.hh>
#include "grammar/klass_def.hpp"

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

using efl::eolian::grammar::attributes::klass_def;
using efl::eolian::grammar::attributes::function_def;
using efl::eolian::grammar::attributes::property_def;
using efl::eolian::grammar::attributes::type_def;

// FIXME Unify this definition some so we can share it with documentation tests.
static
klass_def init_test_data(std::string const target_file, std::string const target_klass, efl::eolian::eolian_state const& state)
{
   ck_assert(::eolian_state_directory_add(state.value, TESTS_SRC_DIR));
   ck_assert(::eolian_state_directory_add(state.value, EO_SRC_DIR));
   ck_assert(::eolian_state_all_eot_files_parse(state.value));
   std::string filename = TESTS_SRC_DIR;
   filename += "/" + target_file;
   ck_assert(::eolian_state_file_parse(state.value, filename.c_str()));

   const Eolian_Class *c_klass = ::eolian_state_class_by_name_get(state.value, target_klass.c_str());
   ck_assert_ptr_ne(c_klass, NULL);

   klass_def klass(c_klass, state.as_unit());
   return klass;
}

EFL_START_TEST(eolian_cxx_test_properties)
{
  efl::eolian::eolian_init eolian_init;
  efl::eolian::eolian_state eolian_state;

  klass_def cls = init_test_data("property_holder.eo", "Property_Holder", eolian_state);

  // FIXME Currently parsing only properties with both get/set values.
  auto props = cls.properties;
  ck_assert_int_eq(4, cls.properties.size());

  ck_assert("prop_simple" == props[0].name);
  ck_assert("getter_only" == props[1].name);
  ck_assert("setter_only" == props[2].name);
  ck_assert("prop_with_key" == props[3].name);

  auto property = props[0];
  ck_assert(property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());
  ck_assert(property.getter->name == "prop_simple_get");
  ck_assert(property.setter->name == "prop_simple_set");
  auto function = std::find_if(cls.functions.cbegin(), cls.functions.cend(), [](const function_def &f) {
     return f.name == "prop_simple_get";
  });
  ck_assert(*property.getter == *function);

  property = props[1];
  ck_assert(property.getter.is_engaged());
  ck_assert(!property.setter.is_engaged());

  property = props[2];
  ck_assert(!property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());

  property = props[3];
  ck_assert(property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());

}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_parent_extensions)
{
  efl::eolian::eolian_init eolian_init;
  efl::eolian::eolian_state eolian_state;

  klass_def cls = init_test_data("generic.eo", "Generic", eolian_state);

  auto parent = cls.parent;
  ck_assert(parent.is_engaged());
  ck_assert_str_eq("Object", parent->eolian_name.c_str());

  ck_assert_int_eq(1, cls.extensions.size());
  auto extension = *cls.extensions.cbegin();
  ck_assert_str_eq("Generic_Interface", extension.eolian_name.c_str());
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
   tcase_add_test(tc, eolian_cxx_test_properties);
   tcase_add_test(tc, eolian_cxx_test_parent_extensions);
}
