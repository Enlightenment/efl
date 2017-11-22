#ifndef EOLIAN_CXX_TYPES_DEFINITION_HH
#define EOLIAN_CXX_TYPES_DEFINITION_HH

#include "header_guards.hpp"
#include "eps.hpp"
#include "string.hpp"
#include "sequence.hpp"
#include "kleene.hpp"
#include "header_include_directive.hpp"
#include "type_function_declaration.hpp"

namespace efl { namespace eolian { namespace grammar {

struct types_definition_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, std::vector<attributes::function_def> const& functions, Context const& ctx) const
   {
      if(!as_generator(*(type_function_declaration()))
            .generate(sink, functions, ctx))
        return false;
      return true;
   }
};

template <>
struct is_eager_generator<types_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<types_definition_generator> : std::integral_constant<int, 1> {};
}

types_definition_generator const types_definition = {};

} } }

#endif
