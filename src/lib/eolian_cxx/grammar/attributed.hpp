#ifndef EOLIAN_CXX_ATTRIBUTED_HH
#define EOLIAN_CXX_ATTRIBUTED_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename Attr, typename G>
struct attributed_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const&, Context const& ctx) const
   {
     return as_generator(g).generate(sink, attr, ctx);
   }

   Attr attr;
   G g;
};

template <typename Attr>
struct attributed_directive
{
  template <typename G>
  attributed_generator<Attr, G> operator[](G g) const
  {
    return {attr, g};
  }

  template <typename OutputIterator, typename Attribute, typename Context>
  bool generate(OutputIterator, Attribute const& attribute, Context const&) const
  {
    return f(attribute);
  }
  
  Attr attr;
};

template <typename Attr, typename G>
struct is_eager_generator<attributed_generator<Attr, G>> : std::true_type {};
template <typename Attr>
struct is_eager_generator<attributed_directive<Attr>> : std::true_type {};
template <typename Attr, typename G>
struct is_generator<attributed_generator<Attr, G>> : std::true_type {};
template <typename Attr>
struct is_generator<attributed_directive<Attr>> : std::true_type {};
      
struct attributed_terminal
{
  template <typename Attr>
  attributed_directive<Attr> operator()(Attr attr) const
  {
    return {attr};
  }
} const attributed = {};

namespace type_traits {
template <typename Attr, typename G>
struct attributes_needed<attributed_generator<Attr, G>> : attributes_needed<G> {};  
template <typename Attr>
struct attributes_needed<attributed_directive<Attr>> : std::integral_constant<int, 0> {};  
}
      
} } }

#endif
