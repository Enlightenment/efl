#ifndef EOLIAN_MONO_PART_DEFINITION_HH
#define EOLIAN_MONO_PART_DEFINITION_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"

#include "utils.hh"
#include "name_helpers.hh"
#include "documentation.hh"

namespace eolian_mono {

struct part_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::part_def const& part, Context const& context) const
  {
     auto part_interface_name = name_helpers::klass_full_interface_name(part.klass);
     auto part_klass_name = name_helpers::klass_full_concrete_name(part.klass);
     return as_generator(scope_tab << documentation
                       << scope_tab << "public " << part_interface_name << " " << utils::capitalize(part.name) << "\n"
                       << scope_tab << "{\n"
                       << scope_tab << scope_tab << "get\n"
                       << scope_tab << scope_tab << "{\n"
                       << scope_tab << scope_tab << scope_tab << "efl.IObject obj = efl_part_get(raw_handle, \"" << part.name << "\");\n"
                       << scope_tab << scope_tab << scope_tab << "return " << part_klass_name << ".static_cast(obj);\n"
                       << scope_tab << scope_tab << "}\n"
                       << scope_tab << "}\n"
            ).generate(sink, part.documentation, context);
  }

} const part_definition {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::part_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::part_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::part_definition_generator> : std::integral_constant<int, 1> {};

}
} } }

#endif

