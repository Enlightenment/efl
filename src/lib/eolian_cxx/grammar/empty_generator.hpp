#ifndef EOLIAN_CXX_EMPTY_GENERATOR_HH
#define EOLIAN_CXX_EMPTY_GENERATOR_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

struct empty_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator const&, Attribute const&, Context const&) const
   {
     return true;
   }
};

struct empty_generator const empty_generator = {};

template <>
struct is_eager_generator<struct ::efl::eolian::grammar::empty_generator> : std::true_type {};
template <>
struct is_generator<struct ::efl::eolian::grammar::empty_generator> : std::true_type {};
      
namespace type_traits {

template <>
struct attributes_needed<struct ::efl::eolian::grammar::empty_generator>
  : std::integral_constant<int, 0> {};
  
}
      
} } }

#endif
