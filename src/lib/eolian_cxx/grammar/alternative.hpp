#ifndef EOLIAN_CXX_ALTERNATIVE_HH
#define EOLIAN_CXX_ALTERNATIVE_HH

#include "grammar/generator.hpp"
#include "grammar/meta.hpp"
#include "grammar/variant.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename L, typename R>
struct alternative_generator
{
   template <typename OutputIterator, typename...Args, typename Context>
   bool generate(OutputIterator /*sink*/, attributes::variant<Args...> const& /*attribute*/, Context const& /*context*/) const
   {
      // return grammar::alternative_sequence(left, right, sink, attribute);
      return false;
   }
   template <typename OutputIterator, typename Attribute, typename...Args, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& context) const
   {
      if(!attributes::generate(as_generator(left), sink, attribute, context))
        return attributes::generate(as_generator(right), sink, attribute, context);
      else
        return true;
   }

   L left;
   R right;
};

template <typename L, typename R>
struct is_eager_generator<alternative_generator<L, R> > : std::true_type {};

namespace type_traits {
template  <typename  L, typename R>
struct attributes_needed<alternative_generator<L, R> > : std::integral_constant
  <int, meta::max<attributes_needed<L>::value, attributes_needed<R>::value>::value> {};
template <typename L, typename R>
struct accepts_tuple<alternative_generator<L, R> > : std::true_type {};
}
      
template <typename L, typename R>
typename std::enable_if<grammar::is_generator<L>::value && grammar::is_generator<R>::value, alternative_generator<L, R>>::type
operator|(L l, R r)
{
   return alternative_generator<L, R>{l, r};
}

} } }

#endif
