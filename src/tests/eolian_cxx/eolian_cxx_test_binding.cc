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
#include <vector>
#include <memory>

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

// Arrays of strings across the boundary, run rather than only compiled.
//
// An Eina_Array of strings holds each string where an array of objects would
// hold a pointer to one, so reading an element means building it from the
// char* rather than dereferencing through it. Every direction the generator
// emits is here: handed in borrowed and handed in owned, handed back borrowed
// and handed back owned, and returned. The C side records what arrived, so
// each check is that the contents crossed, not merely that the call did.
EFL_START_TEST(eolian_cxx_test_string_arrays)
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

  // In, borrowed: the caller keeps the array and the strings.
  {
    efl::eina::range_array< ::efl::eina::string_view> sent{"sent-one", "sent-two"};
    g.string_array_in(sent);

    ck_assert_str_eq("sent-one,sent-two", g.string_array_seen_get().data());

    // and the caller's copy is untouched by the call
    ck_assert(sent.size() == 2u);
    ck_assert(sent[0] == "sent-one");
  }

  // In, owned: @move, so the array and the strings become the callee's. The
  // caller's object must not free them again on the way out of this scope.
  {
    efl::eina::array< ::efl::eina::string_view> sent{"moved-one", "moved-two"};
    g.string_array_in_own(sent);

    ck_assert_str_eq("moved-one,moved-two", g.string_array_seen_get().data());
  }

  // Out, borrowed: the object keeps what it handed back.
  {
    efl::eina::range_array< ::efl::eina::string_view> got;
    g.string_array_out(got);

    ck_assert(got.size() == 2u);
    ck_assert(got[0] == "kept-one");
    ck_assert(got[1] == "kept-two");

    char const* expected[] = {"kept-one", "kept-two"};
    ck_assert(std::equal(got.begin(), got.end(), expected));
  }

  // Out, owned: the strings and the array are the caller's, and freeing them
  // is what its destructor does at the end of this scope.
  {
    efl::eina::array< ::efl::eina::string_view> got;
    g.string_array_out_own(got);

    ck_assert(got.size() == 2u);
    ck_assert(got[0] == "given-one");
    ck_assert(got[1] == "given-two");
  }

  // Returned, owned: same, by return value.
  {
    efl::eina::array< ::efl::eina::string_view> got = g.string_array_return_own();

    ck_assert(got.size() == 2u);
    ck_assert(got.front() == "given-one");
    ck_assert(got.back() == "given-two");

    char const* reversed[] = {"given-two", "given-one"};
    ck_assert(std::equal(got.rbegin(), got.rend(), reversed));
  }
}
EFL_END_TEST

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

/* An event whose payload is a bool. The payload is an Eina_Bool, so bool has
   a convert_to_event overload of its own -- and the generic one beside it
   did not exclude bool, so both were viable, neither won, and no C++ handler
   could be attached to any such event at all. Compiling is most of the test. */
EFL_START_TEST(eolian_cxx_test_bool_event_payload)
{
  Eina_Bool payload = EINA_TRUE;

  ck_assert(efl::eolian::convert_to_event<bool>(&payload));
  payload = EINA_FALSE;
  ck_assert(!efl::eolian::convert_to_event<bool>(&payload));
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

namespace {
/* A closure member whose destruction is counted, once per object. */
struct counted
{
  int* destroyed;
  explicit counted(int* d) : destroyed(d) {}
  counted(counted const&) = delete;
  ~counted() { ++*destroyed; }
};
}

/* The three-argument form frees its closure when the emitter goes. */
EFL_START_TEST(eolian_cxx_test_event_closure_freed_with_emitter)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0, calls = 0;
  std::unique_ptr<nonamespace::Generic> g
    (new nonamespace::Generic(efl::eo::instantiate));
  auto* c = &calls;
  {
    auto k = std::make_shared<counted>(&destroyed);
    efl::eolian::event_add(g->prefix_event1_event, *g,
                           [c, k] (nonamespace::Generic) { ++*c; });
  }
  g->call_event1();
  ck_assert_int_eq(calls, 1);
  ck_assert_int_eq(destroyed, 0);
  g.reset();
  ck_assert_int_eq(destroyed, 1);
  ecore_main_loop_iterate();
  ck_assert_int_eq(destroyed, 1);
  ecore_shutdown();
}
EFL_END_TEST

/* A forgotten (kept) connection does not keep the emitter alive. */
EFL_START_TEST(eolian_cxx_test_event_connection_is_weak)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0;
  std::unique_ptr<nonamespace::Generic> g
    (new nonamespace::Generic(efl::eo::instantiate));
  Eo* raw = g->_eo_ptr();
  std::unique_ptr<efl::eolian::signal_connection> cp;
  {
    auto k = std::make_shared<counted>(&destroyed);
    cp.reset(new efl::eolian::signal_connection(efl::eolian::event_add
      (g->prefix_event1_event, *g, [k] (nonamespace::Generic) {})));
  }
  efl::eolian::signal_connection& c = *cp;
  ck_assert_int_eq(efl_ref_count(raw), 1);   /* the wrapper's, nobody else's */
  g.reset();                                  /* last reference: it dies now */
  ck_assert_int_eq(destroyed, 1);
  c.disconnect();                             /* after the emitter: nothing */
  ck_assert_int_eq(destroyed, 1);
  ecore_shutdown();
}
EFL_END_TEST

/* Disconnecting after the emitter is gone is a no-op (valgrind). */
EFL_START_TEST(eolian_cxx_test_event_disconnect_after_emitter)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0;
  std::unique_ptr<efl::eolian::scoped_signal_connection> scoped;
  std::unique_ptr<efl::eolian::signal_connection> plain;
  {
    nonamespace::Generic g(efl::eo::instantiate);
    auto k = std::make_shared<counted>(&destroyed);
    efl::eolian::signal_connection c = efl::eolian::event_add
      (g.prefix_event1_event, g, [k] (nonamespace::Generic) {});
    k.reset();
    plain.reset(new efl::eolian::signal_connection(c));
    scoped.reset(new efl::eolian::scoped_signal_connection(c));
  }
  ck_assert_int_eq(destroyed, 1);
  plain->disconnect();
  scoped.reset();
  plain->disconnect();
  ck_assert_int_eq(destroyed, 1);
  ecore_shutdown();
}
EFL_END_TEST

/* Any number of copies and scoped handles: the first disconnect wins, the
 * callback is removed once and the closure freed once (valgrind: no double
 * free). */
EFL_START_TEST(eolian_cxx_test_event_copies_disconnect_once)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0, calls = 0;
  nonamespace::Generic g(efl::eo::instantiate);
  auto* c = &calls;
  std::unique_ptr<efl::eolian::signal_connection> c1p;
  {
    auto k = std::make_shared<counted>(&destroyed);
    c1p.reset(new efl::eolian::signal_connection(efl::eolian::event_add
      (g.prefix_event1_event, g, [c, k] (nonamespace::Generic) { ++*c; })));
  }
  efl::eolian::signal_connection& c1 = *c1p;
  efl::eolian::signal_connection c2 = c1;
  {
    efl::eolian::scoped_signal_connection s(c1);
    efl::eolian::scoped_signal_connection s2(std::move(s));
    g.call_event1();
    ck_assert_int_eq(calls, 1);
    c1.disconnect();
    ck_assert_int_eq(destroyed, 1);
    c2.disconnect();
    s2.disconnect();
    c1.disconnect();
    ck_assert_int_eq(destroyed, 1);
  }                       /* scoped handles die: still nothing */
  g.call_event1();
  ck_assert_int_eq(calls, 1);
  ck_assert_int_eq(destroyed, 1);
  ecore_shutdown();
}
EFL_END_TEST

/* A handler of the end-of-life events, attached with event_add, still runs
 * once while the emitter is torn down, and its closure goes afterwards. */
EFL_START_TEST(eolian_cxx_test_event_teardown_handlers_still_run)
{
  efl::eo::eo_init i;
  ecore_init();
  int del_calls = 0, inv_calls = 0, del_alive = 0, inv_alive = 0;
  int del_destroyed = 0, inv_destroyed = 0;
  {
    nonamespace::Generic g(efl::eo::instantiate);
    auto* dc = &del_calls;
    auto* ic = &inv_calls;
    auto* da = &del_alive;
    auto* ia = &inv_alive;
    auto* dd = &del_destroyed;
    auto* id = &inv_destroyed;
    auto dk = std::make_shared<counted>(&del_destroyed);
    auto ik = std::make_shared<counted>(&inv_destroyed);
    efl::eolian::event_add(efl::Object::del_event, g,
                           [dc, da, dd, dk] (nonamespace::Generic)
                           {
                              ++*dc;
                              *da += (*dd == 0);
                           });
    efl::eolian::event_add(efl::Object::invalidate_event, g,
                           [ic, ia, id, ik] (nonamespace::Generic)
                           {
                              ++*ic;
                              *ia += (*id == 0);
                           });
    dk.reset();
    ik.reset();
  }
  ck_assert_int_eq(inv_calls, 1);
  ck_assert_int_eq(del_calls, 1);
  ck_assert_int_eq(inv_alive, 1);   /* the closure was still there for it */
  ck_assert_int_eq(del_alive, 1);
  ck_assert_int_eq(inv_destroyed, 1);
  ck_assert_int_eq(del_destroyed, 1);
  ecore_shutdown();
}
EFL_END_TEST

/* A handler that disconnects itself keeps its closure for the call and frees
 * it after (valgrind). */
EFL_START_TEST(eolian_cxx_test_event_handler_self_disconnect_frees)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0, seen = 0;
  nonamespace::Generic g(efl::eo::instantiate);
  std::unique_ptr<efl::eolian::signal_connection> h;
  auto* hp = &h;
  auto* out = &seen;
  auto* dp = &destroyed;
  std::vector<int> payload(16, 7);
  {
    auto k = std::make_shared<counted>(&destroyed);
    h.reset(new efl::eolian::signal_connection(efl::eolian::event_add
      (g.prefix_event1_event, g,
       [hp, out, dp, payload, k] (nonamespace::Generic)
       {
         (*hp)->disconnect();
         *out += payload[5] + (*dp == 0);
       })));
  }
  g.call_event1();
  ck_assert_int_eq(seen, 8);
  ck_assert_int_eq(destroyed, 1);
  g.call_event1();
  ck_assert_int_eq(seen, 8);
  ecore_shutdown();
}
EFL_END_TEST

/* A handler that deletes its emitter while it runs (valgrind). */
EFL_START_TEST(eolian_cxx_test_event_handler_deletes_emitter)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0, seen = 0;
  std::unique_ptr<nonamespace::Generic> g
    (new nonamespace::Generic(efl::eo::instantiate));
  auto* gp = &g;
  auto* out = &seen;
  auto* dp = &destroyed;
  std::vector<int> payload(16, 7);
  {
    auto k = std::make_shared<counted>(&destroyed);
    efl::eolian::event_add
      (g->prefix_event1_event, *g,
       [gp, out, dp, payload, k] (nonamespace::Generic)
       {
         gp->reset();                       /* the emitter goes while we run */
         *out += payload[5] + (*dp == 0);
       });
  }
  g->call_event1();
  ck_assert_int_eq(seen, 8);
  ck_assert_int_eq(destroyed, 1);
  ecore_main_loop_iterate();
  ecore_shutdown();
}
EFL_END_TEST

static int _binding_log_errors = 0;

static void
_binding_count_errors(const Eina_Log_Domain *d EINA_UNUSED, Eina_Log_Level level,
                      const char *file EINA_UNUSED, const char *fnc EINA_UNUSED,
                      int line EINA_UNUSED, const char *fmt EINA_UNUSED,
                      void *data EINA_UNUSED, va_list args EINA_UNUSED)
{
  if (level <= EINA_LOG_LEVEL_ERR) _binding_log_errors++;
}

/* A handler that captured its own emitter, which has a parent: deleting the
 * object invalidates it, its connections are cut then, the closure goes and
 * gives back the reference it held, so the object is destroyed (no leak) and
 * nothing is logged as an error (no "too many unrefs"). */
EFL_START_TEST(eolian_cxx_test_event_self_capture_parented)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0, calls = 0;
  nonamespace::Generic parent(efl::eo::instantiate);
  eina_log_print_cb_set(_binding_count_errors, nullptr);
  _binding_log_errors = 0;
  {
    nonamespace::Generic g(efl::eo::instantiate, parent);
    auto k = std::make_shared<counted>(&destroyed);
    auto* c = &calls;
    efl::eolian::event_add(g.prefix_event1_event, g,
                           [g, c, k] (nonamespace::Generic) { ++*c; });
    k.reset();
    g.call_event1();
    ck_assert_int_eq(calls, 1);
    ck_assert_int_eq(destroyed, 0);
    ::efl_del(g._eo_ptr());   /* parent gone: invalidate cuts the closure */
  }
  ck_assert_int_eq(destroyed, 1);
  ecore_main_loop_iterate();
  eina_log_print_cb_set(eina_log_print_cb_stderr, nullptr);
  ck_assert_int_eq(_binding_log_errors, 0);
  ecore_shutdown();
}
EFL_END_TEST

/* A parentless object whose handler captures it strongly is a cycle, and that
 * is documented: it lives until the connection is cut, and goes with it. */
EFL_START_TEST(eolian_cxx_test_event_self_capture_parentless_cycle)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0;
  eina_log_print_cb_set(_binding_count_errors, nullptr);
  _binding_log_errors = 0;
  std::unique_ptr<efl::eolian::signal_connection> handle;
  {
    nonamespace::Generic g(efl::eo::instantiate);
    auto k = std::make_shared<counted>(&destroyed);
    handle.reset(new efl::eolian::signal_connection(efl::eolian::event_add
      (g.prefix_event1_event, g, [g, k] (nonamespace::Generic) {})));
  }                                /* the last wrapper but the closure's */
  ck_assert_int_eq(destroyed, 0);  /* still alive: that is the cycle */
  handle->disconnect();
  ck_assert_int_eq(destroyed, 1);  /* cut: closure freed, object released */
  eina_log_print_cb_set(eina_log_print_cb_stderr, nullptr);
  ck_assert_int_eq(_binding_log_errors, 0);
  ecore_shutdown();
}
EFL_END_TEST

/* A del handler that uses its argument is called once, and no error is
 * logged while the emitter is torn down. */
EFL_START_TEST(eolian_cxx_test_event_del_handler_uses_argument)
{
  efl::eo::eo_init i;
  ecore_init();
  int calls = 0, destroyed = 0, same = 0;
  eina_log_print_cb_set(_binding_count_errors, nullptr);
  _binding_log_errors = 0;
  {
    nonamespace::Generic g(efl::eo::instantiate);
    Eo* raw = g._eo_ptr();
    auto k = std::make_shared<counted>(&destroyed);
    auto* c = &calls;
    auto* sm = &same;
    efl::eolian::event_add(efl::Object::del_event, g,
                           [c, sm, raw, k] (nonamespace::Generic obj)
                           {
                              ++*c;
                              *sm += (obj._eo_ptr() == raw);
                           });
  }
  ck_assert_int_eq(calls, 1);
  ck_assert_int_eq(same, 1);
  ck_assert_int_eq(destroyed, 1);
  eina_log_print_cb_set(eina_log_print_cb_stderr, nullptr);
  ck_assert_int_eq(_binding_log_errors, 0);
  ecore_shutdown();
}
EFL_END_TEST

/* A connection made while the emitter is invalidating (from its own
 * invalidate handler) or already invalidated but still alive is not made: the
 * object calls no such handler after its invalidation, so there is nothing to
 * keep, and the closure is freed at once. Disconnecting later, after the
 * emitter is gone, logs nothing. */
EFL_START_TEST(eolian_cxx_test_event_connect_while_invalidating)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0, calls = 0;
  eina_log_print_cb_set(_binding_count_errors, nullptr);
  _binding_log_errors = 0;
  {
    std::unique_ptr<nonamespace::Generic> g
      (new nonamespace::Generic(efl::eo::instantiate));
    auto* c = &calls;
    auto* d = &destroyed;
    efl::eolian::event_add(efl::Object::invalidate_event, *g,
      [c, d] (nonamespace::Generic obj)
      {
        auto k = std::make_shared<counted>(d);
        efl::eolian::event_add(obj.prefix_event1_event, obj,
                               [c, k] (nonamespace::Generic) { ++*c; });
        ck_assert_int_eq(*d, 0);   /* k still held here */
      });
    g.reset();
  }
  ck_assert_int_eq(destroyed, 1);
  ck_assert_int_eq(calls, 0);
  eina_log_print_cb_set(eina_log_print_cb_stderr, nullptr);
  ck_assert_int_eq(_binding_log_errors, 0);
  ecore_shutdown();
}
EFL_END_TEST

EFL_START_TEST(eolian_cxx_test_event_connect_after_invalidate)
{
  efl::eo::eo_init i;
  ecore_init();
  int destroyed = 0, calls = 0;
  eina_log_print_cb_set(_binding_count_errors, nullptr);
  _binding_log_errors = 0;
  nonamespace::Generic parent(efl::eo::instantiate);
  std::unique_ptr<efl::eolian::signal_connection> handle;
  {
    nonamespace::Generic g(efl::eo::instantiate, parent);
    Eo* raw = g._eo_ptr();
    ::efl_ref(raw);
    ::efl_del(raw);               /* invalidated, kept alive by the others */
    auto k = std::make_shared<counted>(&destroyed);
    auto* c = &calls;
    handle.reset(new efl::eolian::signal_connection(efl::eolian::event_add
      (g.prefix_event1_event, g, [c, k] (nonamespace::Generic) { ++*c; })));
    k.reset();
    ck_assert_int_eq(destroyed, 1);   /* not made: closure already freed */
    g.call_event1();
    ck_assert_int_eq(calls, 0);
    ::efl_unref(raw);
  }                                   /* g's own reference: it dies now */
  ecore_main_loop_iterate();
  handle->disconnect();               /* after the emitter: nothing, no error */
  handle.reset();
  eina_log_print_cb_set(eina_log_print_cb_stderr, nullptr);
  ck_assert_int_eq(_binding_log_errors, 0);
  ecore_shutdown();
}
EFL_END_TEST

/* A "string" event carries a char const* (Efl.Ui.Textbox's activated carries
 * the text that way), not a string_view. */
EFL_START_TEST(eolian_cxx_test_string_event_payload)
{
  char const text[] = "a payload that is longer than a string_view is";
  efl::eina::string_view v =
    efl::eolian::convert_to_event<efl::eina::string_view>(const_cast<char*>(text));
  std::string got(v);
  ck_assert_str_eq(got.c_str(), text);

  /* Efl.Config's config,changed carries NULL when every value changed. */
  efl::eina::string_view none = efl::eolian::convert_to_event<efl::eina::string_view>(nullptr);
  ck_assert(none.empty());
}
EFL_END_TEST

/* A handler may disconnect its own handle and then go on using what it
 * captured: the closure outlives the call (valgrind: no invalid read). */
EFL_START_TEST(eolian_cxx_test_event_handler_disconnects_own_handle)
{
  efl::eo::eo_init i;
  ecore_init();
  int seen = 0;
  nonamespace::Generic g(efl::eo::instantiate);
  std::unique_ptr<efl::eolian::scoped_signal_connection> handle;
  std::vector<int> payload(16, 7);
  auto* h = &handle;
  auto* out = &seen;
  handle.reset(new efl::eolian::scoped_signal_connection(
    efl::eolian::event_add(g.prefix_event1_event, g,
                           [h, out, payload] (nonamespace::Generic)
                           {
                             h->reset();
                             *out += payload[5];
                           })));
  g.call_event1();
  ck_assert_int_eq(seen, 7);
  g.call_event1();
  ck_assert_int_eq(seen, 7);
  ecore_main_loop_iterate();
  ecore_shutdown();
}
EFL_END_TEST

void
eolian_cxx_test_binding(TCase* tc)
{
   tcase_add_test(tc, eolian_cxx_test_event_handler_disconnects_own_handle);
   tcase_add_test(tc, eolian_cxx_test_event_closure_freed_with_emitter);
   tcase_add_test(tc, eolian_cxx_test_event_connection_is_weak);
   tcase_add_test(tc, eolian_cxx_test_event_disconnect_after_emitter);
   tcase_add_test(tc, eolian_cxx_test_event_copies_disconnect_once);
   tcase_add_test(tc, eolian_cxx_test_event_teardown_handlers_still_run);
   tcase_add_test(tc, eolian_cxx_test_event_handler_self_disconnect_frees);
   tcase_add_test(tc, eolian_cxx_test_event_handler_deletes_emitter);
   tcase_add_test(tc, eolian_cxx_test_event_self_capture_parented);
   tcase_add_test(tc, eolian_cxx_test_event_self_capture_parentless_cycle);
   tcase_add_test(tc, eolian_cxx_test_event_del_handler_uses_argument);
   tcase_add_test(tc, eolian_cxx_test_event_connect_while_invalidating);
   tcase_add_test(tc, eolian_cxx_test_event_connect_after_invalidate);
   tcase_add_test(tc, eolian_cxx_test_string_event_payload);
   tcase_add_test(tc, eolian_cxx_test_string_arrays);
   tcase_add_test(tc, eolian_cxx_test_binding_constructor_only_required);
   tcase_add_test(tc, eolian_cxx_test_binding_constructor_all_optionals);
   tcase_add_test(tc, eolian_cxx_test_type_generation);
   tcase_add_test(tc, eolian_cxx_test_bool_event_payload);
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
