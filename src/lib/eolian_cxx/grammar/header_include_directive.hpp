#ifndef EOLIAN_CXX_HEADER_INCLUDE_DIRECTIVE_HH
#define EOLIAN_CXX_HEADER_INCLUDE_DIRECTIVE_HH

#include "generator.hpp"

namespace efl { namespace eolian { namespace grammar {

struct header_include_directive_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const&) const
   {
      const char include_directive[] = "#include \"";
      std::copy(include_directive, include_directive + sizeof(include_directive)-1, sink);
      std::copy(std::begin(attribute), std::end(attribute), sink);
      *sink++ = '\"';
      *sink++ = '\n';
      return true;
   }
};

template <>
struct is_eager_generator<header_include_directive_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<header_include_directive_generator> : std::integral_constant<int, 1> {};
}

header_include_directive_generator const header_include_directive = {};

} } }

#endif
