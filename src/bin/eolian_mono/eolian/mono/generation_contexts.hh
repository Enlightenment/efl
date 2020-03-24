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
#ifndef EOLIAN_MONO_GENERATION_CONTEXTS_HH
#define EOLIAN_MONO_GENERATION_CONTEXTS_HH

#include <map>

#include "grammar/context.hpp"
#include "grammar/indentation.hpp"

namespace eolian_mono {

struct class_context
{
    enum wrapper_kind {
        interface,
        concrete,
        inherit,
        inherit_native,
        structs,
        enums,
        function_ptr,
        alias,
        variables,
    };
    wrapper_kind current_wrapper_kind;
    std::string name;

    class_context(wrapper_kind current_wrapper_kind)
      :  current_wrapper_kind(current_wrapper_kind)
      , name()
    {}

    class_context(wrapper_kind current_wrapper_kind, std::string const& name)
      :  current_wrapper_kind(current_wrapper_kind)
      , name(name)
    {}
};

struct direction_context
{
    enum direction {
        native_to_managed,
        managed_to_native,
    };
    direction current_direction;

    direction_context(direction current_direction)
      :  current_direction(current_direction)
    {}
};

struct indentation_context
{
  constexpr indentation_context(indentation_context const& other) = default;
  constexpr indentation_context(efl::eolian::grammar::scope_tab_generator indent)
    : indent(indent)
  {}
  constexpr indentation_context(int n)
    : indent(n)
  {}
  constexpr indentation_context(int n, int m)
    : indent(n, m)
  {}
  efl::eolian::grammar::scope_tab_generator indent;
};

template <typename Context>
inline constexpr efl::eolian::grammar::scope_tab_generator const& current_indentation(Context const& context)
{
  return efl::eolian::grammar::context_find_tag<indentation_context>(context).indent;
}

template <typename Context>
inline constexpr Context change_indentation(efl::eolian::grammar::scope_tab_generator const& indent, Context const& context)
{
  return efl::eolian::grammar::context_replace_tag(indentation_context(indent), context);
}

struct library_context
{
  std::string library_name;
  int v_major;
  int v_minor;
  std::map<const std::string, std::string> references;

  const std::string actual_library_name(const std::string& filename) const;
};

const std::string
library_context::actual_library_name(const std::string& filename) const
{
    // Libraries mapped follow the efl.Libs.NAME scheme.
    // TODO What about references from outside efl (not present in the efl.Libs class?)
    auto ref = references.find(filename);
    if (ref != references.end())
      return "efl.Libs." + ref->second;

    // Fallback to original behaviour with explicit library name
    return '"' + library_name + '"';
}

struct eolian_state_context {
    const Eolian_State *state;
};

struct options_context {
    bool want_beta;
    std::string examples_dir;
};

}

#endif
