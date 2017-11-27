#ifndef EOLIAN_CXX_PART_DECLARATION_HH
#define EOLIAN_CXX_PART_DECLARATION_HH

#include "grammar/generator.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/keyword.hpp"

namespace efl { namespace eolian { namespace grammar {

struct part_declaration_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::part_def const& part, Context const& ctx) const
   {
      if(!as_generator("::efl::eolian::return_traits<::" << *(string << "::"))
            .generate(sink, part.klass.namespaces, add_lower_case_context(ctx)))
        return false;
      if(!as_generator(string << ">::type " << string << "() const")
            .generate(sink, std::make_tuple(part.klass.eolian_name, part.name), ctx))
        return false;

      return true;
   }
};

template <>
struct is_eager_generator<part_declaration_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<part_declaration_generator> : std::integral_constant<int, 1> {};
}

part_declaration_generator const part_declaration = {};

} } }

#endif
