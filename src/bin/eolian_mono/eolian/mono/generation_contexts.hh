#ifndef EOLIAN_MONO_GENERATION_CONTEXTS_HH
#define EOLIAN_MONO_GENERATION_CONTEXTS_HH

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
};

}

#endif
