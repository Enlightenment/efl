#ifndef EOLIAN_CXX_IMPLEMENTATION_INCLUDE_DIRECTIVE_HH
#define EOLIAN_CXX_IMPLEMENTATION_INCLUDE_DIRECTIVE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"

namespace efl { namespace eolian { namespace grammar {

struct implementation_include_directive_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& ctx) const
   {
     return as_generator("#include \"" << string << ".impl.hh\"\n")
       .generate(sink, std::string(eolian_class_file_get(get_klass(get_klass_name(cls)))), add_lower_case_context(ctx));
   }
};

template <>
struct is_eager_generator<implementation_include_directive_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<implementation_include_directive_generator> : std::integral_constant<int, 1> {};
}
      
implementation_include_directive_generator const implementation_include_directive;
      
} } }

#endif
