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
#ifndef EOLIAN_MONO_FUNCTION_DECLARATION_HH
#define EOLIAN_MONO_FUNCTION_DECLARATION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "parameter.hh"
#include "name_helpers.hh"
#include "using_decl.hh"
#include "blacklist.hh"

namespace eolian_mono {

struct function_declaration_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    if(blacklist::is_function_blacklisted(f, context) || f.is_static)
      return true;

    // C# interfaces can't have non-public members
    if(f.scope != attributes::member_scope::scope_public)
      return true;

    if(!as_generator(documentation(1)).generate(sink, f, context))
      return false;

    return as_generator
      (scope_tab << eolian_mono::type(true) << " " << string << "(" << (parameter % ", ") << ");\n\n")
      .generate(sink, std::make_tuple(f.return_type, name_helpers::managed_method_name(f), f.parameters), context);
  }
};

function_declaration_generator const function_declaration = {};

struct property_declaration_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::property_def const& property, Context const& context) const
  {
    if(blacklist::is_property_blacklisted(property, *implementing_klass, context))
      return true;

    auto has_wrapper = helpers::has_property_wrapper (property, implementing_klass, context);
    bool has_getter_wrapper = has_wrapper & helpers::has_property_wrapper_bit::has_getter;
    bool has_setter_wrapper = has_wrapper & helpers::has_property_wrapper_bit::has_setter;

    auto gen = [&] (attributes::function_def const& f)
    {
      // C# interfaces can't have non-public members
      if(f.scope != attributes::member_scope::scope_public)
        return true;

      if (f.is_static)
        return true;

      if(!as_generator(documentation(1)).generate(sink, f, context))
        return false;

      return as_generator
        (scope_tab << eolian_mono::type(true) << " " << string << "(" << (parameter % ", ") << ");\n\n")
        .generate(sink, std::make_tuple(f.return_type, name_helpers::managed_method_name(f), f.parameters), context);
    };
    bool r = true;
    if (property.getter)
    {
      if (!has_getter_wrapper)
        r &= gen (*property.getter);
      // else
      //   r &= function_declaration.generate(sink, *property.getter, context);
    }
    if (r && property.setter)
    {
      if (!has_setter_wrapper)
        r &= gen (*property.setter);
      // else
      //   r &= function_declaration.generate(sink, *property.setter, context);
    }
    return r;
  }
  attributes::klass_def const* implementing_klass, *klass_from_property;
};

struct property_declaration_parameterized
{
  property_declaration_generator operator()(attributes::klass_def const& klass
                                            , attributes::klass_def const& prop_from_klass) const
  {
    return {&klass, &prop_from_klass};
  }
} const property_declaration;
  
}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::function_declaration_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::property_declaration_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::property_declaration_parameterized> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_declaration_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::property_declaration_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::property_declaration_parameterized> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::function_declaration_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::property_declaration_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::property_declaration_parameterized> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
