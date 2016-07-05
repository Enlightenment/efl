#ifndef EOLIAN_CXX_FUNCTION_DECLARATION_HH
#define EOLIAN_CXX_FUNCTION_DECLARATION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/keyword.hpp"

namespace efl { namespace eolian { namespace grammar {

struct function_declaration_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    return as_generator
      ("::efl::eolian::return_traits<" << grammar::type(true) << ">::type " << string << "(" << (parameter % ", ") << ") const;\n")
      .generate(sink, std::make_tuple(f.return_type, escape_keyword(f.name), f.parameters), context);
  }
};

template <>
struct is_eager_generator<function_declaration_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<function_declaration_generator> : std::integral_constant<int, 1> {};
}
      
function_declaration_generator const function_declaration = {};
      
} } }

#endif
