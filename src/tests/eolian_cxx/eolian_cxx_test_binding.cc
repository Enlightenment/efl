/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include <map>

#define GENERIC_BETA
#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

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

  g.invoidptr(nullptr);
  g.inint(42);
  g.instring("foobar");
  g.instringown("foobar");
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_type_generation_return)
{
  efl::eo::eo_init i;

  name1::name2::Type_Generation g(efl::eo::instantiate);

  {
    int i = g.returnint();
    ck_assert(i == 42);
  }
  {
    void* p = g.returnvoidptr();
    ck_assert(*(int*)p == 42);
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

  i = 0;
  g.optionaloutint(&i);
  ck_assert(i == 42);
  g.optionaloutint(nullptr);
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_type_callback)
{
  efl::eo::eo_init i;

  bool event1 = false, event2 = false, event3 = false, event4 = false
    , event5 = false, event6 = false;

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
  efl::eolian::event_add(g.prefix_event4_event, g, [&] (nonamespace::Generic, efl::eina::range_array<::efl::eina::string_view> e)
                         {
                           event4 = true;
                           ck_assert(e.size() == 1);
                           // FIXME eina::range_array is incompatible with eina::string_view
                           //ck_assert(*e.begin() == efl::eina::string_view{"42"});
                         });
  efl::eolian::event_add(g.prefix_event5_event, g, [&] (nonamespace::Generic, Generic_Beta_Event)
                         {
                           event5 = true;
                         });
  efl::eolian::event_add(g.prefix_event6_event, g, [&] (nonamespace::Generic, Generic_Event)
                         {
                           event6 = true;
                         });

  g.call_event1();
  g.call_event2();
  g.call_event3();
  g.call_event4();
  g.call_event5();
  g.call_event6();

  ck_assert(event1);
  ck_assert(event2);
  ck_assert(event3);
  ck_assert(event4);
  ck_assert(event5);
  ck_assert(event6);
}
EFL_END_TEST

using efl::eolian::grammar::attributes::klass_def;
using efl::eolian::grammar::attributes::function_def;
using efl::eolian::grammar::attributes::property_def;
using efl::eolian::grammar::attributes::type_def;
using efl::eolian::grammar::attributes::event_def;

// FIXME Unify this definition some so we can share it with documentation tests.
static
klass_def init_test_data(std::string const target_file, std::string const target_klass, efl::eolian::eolian_state const& state)
{
   ck_assert(::eolian_state_directory_add(state.value, TESTS_SRC_DIR));
   ck_assert(::eolian_state_directory_add(state.value, EO_SRC_DIR));
   ck_assert(::eolian_state_all_eot_files_parse(state.value));
   ck_assert(::eolian_state_file_parse(state.value, target_file.c_str()));

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

  auto props = cls.properties;
  ck_assert_int_eq(8, cls.properties.size());

  ck_assert("prop_simple" == props[0].name);
  ck_assert("getter_only" == props[1].name);
  ck_assert("setter_only" == props[2].name);
  ck_assert("prop_with_key" == props[3].name);
  ck_assert("multi_value_prop" == props[4].name);
  ck_assert("setter_with_return" == props[5].name);
  ck_assert("getter_with_return" == props[6].name);
  ck_assert("value_override" == props[7].name);

  auto property = props[0];
  ck_assert(property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());
  ck_assert(property.getter->name == "prop_simple_get");
  ck_assert(property.setter->name == "prop_simple_set");
  auto function = std::find_if(cls.functions.cbegin(), cls.functions.cend(), [](const function_def &f) {
     return f.name == "prop_simple_get";
  });
  ck_assert(*property.getter == *function);

  ck_assert_int_eq(0, property.getter->keys.size());
  ck_assert_int_eq(1, property.getter->values.size());
  ck_assert_int_eq(0, property.setter->keys.size());
  ck_assert_int_eq(1, property.setter->values.size());

  property = props[1];
  ck_assert(property.getter.is_engaged());
  ck_assert(!property.setter.is_engaged());
  ck_assert_int_eq(0, property.getter->keys.size());
  ck_assert_int_eq(1, property.getter->values.size());

  property = props[2];
  ck_assert(!property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());
  ck_assert_int_eq(0, property.setter->keys.size());
  ck_assert_int_eq(1, property.setter->values.size());

  property = props[3];
  ck_assert(property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());
  ck_assert_int_eq(1, property.getter->keys.size());
  ck_assert_int_eq(1, property.getter->values.size());

  property = props[4];
  ck_assert(property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());
  ck_assert_int_eq(0, property.getter->keys.size());
  ck_assert_int_eq(2, property.getter->values.size());
  ck_assert_int_eq(0, property.setter->keys.size());
  ck_assert_int_eq(2, property.setter->values.size());

  property = props[5];
  ck_assert(property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());
  ck_assert_int_eq(0, property.getter->keys.size());
  ck_assert_int_eq(1, property.getter->values.size());
  ck_assert_int_eq(0, property.setter->keys.size());
  ck_assert_int_eq(1, property.setter->values.size());

  property = props[6];
  ck_assert(property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());
  ck_assert_int_eq(0, property.getter->keys.size());
  ck_assert_int_eq(1, property.getter->values.size());
  ck_assert_int_eq(0, property.setter->keys.size());
  ck_assert_int_eq(1, property.setter->values.size());

  property = props[7];
  ck_assert(property.getter.is_engaged());
  ck_assert(property.setter.is_engaged());
  ck_assert_int_eq(0, property.getter->keys.size());
  ck_assert_int_eq(1, property.getter->values.size());
  ck_assert_int_eq(0, property.setter->keys.size());
  ck_assert_int_eq(1, property.setter->values.size());

}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_property_accessor_info)
{
  efl::eolian::eolian_init eolian_init;
  efl::eolian::eolian_state eolian_state;

  klass_def cls = init_test_data("property_holder.eo", "Property_Holder", eolian_state);

  auto props = cls.properties;
  auto property = props[0];
  auto getter = *property.getter;

  // Single-valued getter
  ck_assert(getter.return_type.c_type == "int");
  ck_assert_int_eq(0, getter.parameters.size());
  ck_assert(getter.explicit_return_type == efl::eolian::grammar::attributes::void_);
  ck_assert_int_eq(1, getter.values.size());
  ck_assert_int_eq(0, getter.keys.size());

  // Single-valued setter
  property = props[2];
  auto setter = *property.setter;
  ck_assert(setter.return_type.c_type == "void");
  ck_assert_int_eq(1, setter.parameters.size());
  ck_assert(setter.explicit_return_type == efl::eolian::grammar::attributes::void_);
  ck_assert_int_eq(1, setter.values.size());
  ck_assert_int_eq(0, setter.keys.size());

  // Multi valued getter
  property = props[4];
  getter = *property.getter;
  ck_assert(getter.return_type.c_type == "void");
  ck_assert_int_eq(2, getter.parameters.size());
  ck_assert(getter.explicit_return_type == efl::eolian::grammar::attributes::void_);
  ck_assert_int_eq(2, getter.values.size());
  ck_assert_int_eq(0, getter.keys.size());

  // Setter with return value
  property = props[5];
  setter = *property.setter;
  ck_assert_str_eq("Eina_Bool", setter.return_type.c_type.c_str());
  ck_assert_int_eq(1, setter.parameters.size());
  ck_assert_str_eq("Eina_Bool", setter.explicit_return_type.c_type.c_str());
  ck_assert_int_eq(1, setter.values.size());
  ck_assert_int_eq(0, setter.keys.size());

  // Getter with return value
  property = props[6];
  getter = *property.getter;
  ck_assert_str_eq("Eina_Bool", getter.return_type.c_type.c_str());
  ck_assert_int_eq(1, getter.parameters.size());
  ck_assert_str_eq("Eina_Bool", getter.explicit_return_type.c_type.c_str());
  ck_assert_int_eq(1, getter.values.size());
  ck_assert_int_eq(0, getter.keys.size());

  // Value override. This mimics Efl.Ui.Win.icon_object behavior.
  property = props[7];
  getter = *property.getter;
  ck_assert_str_eq("const Property_Holder *", getter.return_type.c_type.c_str());
  ck_assert_int_eq(0, getter.parameters.size());
  ck_assert_str_eq("void", getter.explicit_return_type.c_type.c_str());
  ck_assert_int_eq(1, getter.values.size());
  ck_assert_int_eq(0, getter.keys.size());

  setter = *property.setter;
  ck_assert_str_eq("void", setter.return_type.c_type.c_str());
  ck_assert_int_eq(1, setter.parameters.size());
  ck_assert_str_eq("void", setter.explicit_return_type.c_type.c_str());
  ck_assert_str_eq("Property_Holder *", setter.parameters[0].type.c_type.c_str());
  ck_assert_int_eq(1, setter.values.size());
  ck_assert_int_eq(0, setter.keys.size());



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

EFL_START_TEST(eolian_cxx_test_cls_get)
{
  efl::eolian::eolian_init eolian_init;
  efl::eolian::eolian_state eolian_state;

  klass_def cls = init_test_data("generic.eo", "Generic", eolian_state);
  ck_assert_str_eq("generic_class_get", cls.klass_get_name.c_str());

  klass_def iface = init_test_data("generic_interface.eo", "Generic_Interface", eolian_state);
  ck_assert_str_eq("generic_interface_interface_get", iface.klass_get_name.c_str());
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_constructors)
{
    efl::eolian::eolian_init eolian_init;
    efl::eolian::eolian_state eolian_state;

    klass_def cls = init_test_data("generic.eo", "Generic", eolian_state);

    auto constructors = cls.constructors;

    ck_assert_int_eq(constructors.size(), 4);

    auto ctor = constructors[0];
    ck_assert_str_eq("Generic.required_ctor_a", ctor.name.c_str());
    ck_assert(!ctor.is_optional);

    auto function = ctor.function;
    ck_assert_str_eq("required_ctor_a", function.name.c_str());

    ctor = constructors[2];
    ck_assert_str_eq("Generic.optional_ctor_a", ctor.name.c_str());
    ck_assert(ctor.is_optional);

    function = ctor.function;
    ck_assert_str_eq("optional_ctor_a", function.name.c_str());
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_beta)
{
    efl::eolian::eolian_init eolian_init;
    efl::eolian::eolian_state eolian_state;

    klass_def cls = init_test_data("generic.eo", "Generic", eolian_state);
    klass_def beta_cls = init_test_data("beta_class.eo", "Beta_Class", eolian_state);

    ck_assert(!cls.is_beta);
    ck_assert(beta_cls.is_beta);
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_beta_cascading)
{
    efl::eolian::eolian_init eolian_init;
    efl::eolian::eolian_state eolian_state;

    klass_def cls = init_test_data("beta_class.eo", "Beta_Class", eolian_state);

    ck_assert(cls.is_beta);

    auto func = std::find_if(cls.functions.begin(), cls.functions.end(), [](function_def const& f) {
        return f.name == "method_should_be_beta";
    });

    ck_assert(func != cls.functions.end());
    ck_assert(func->is_beta);

    auto evt = std::find_if(cls.events.begin(), cls.events.end(), [](event_def const& e) {
        return e.name == "event_should_be_beta";
    });

    ck_assert(evt != cls.events.end());
    ck_assert(evt->is_beta);
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
   tcase_add_test(tc, eolian_cxx_test_property_accessor_info);
   tcase_add_test(tc, eolian_cxx_test_parent_extensions);
   tcase_add_test(tc, eolian_cxx_test_cls_get);
   tcase_add_test(tc, eolian_cxx_test_constructors);
   tcase_add_test(tc, eolian_cxx_test_beta);
   tcase_add_test(tc, eolian_cxx_test_beta_cascading);
}
