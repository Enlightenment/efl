#ifndef EOLIAN_CXX_KLEENE_HH
#define EOLIAN_CXX_KLEENE_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename Generator>
struct kleene_generator
{
   kleene_generator(Generator g)
     : generator(g) {}
  
  template <typename OutputIterator, typename Attribute, typename Context>
  bool generate(OutputIterator sink, Attribute const& attribute, Context const& context) const
   {
      bool b;
      for(auto&& c : attribute)
        {
           b = as_generator(generator).generate(sink, c, context);
           if(!b)
             return false;
        }
      return true;
   }

   Generator generator;
};

template <typename Generator>
struct is_eager_generator<kleene_generator<Generator> > : std::true_type {};

namespace type_traits {
template  <typename G>
struct attributes_needed<kleene_generator<G> > : std::integral_constant<int, 1> {};
}
      
template <typename Generator>
typename std::enable_if<grammar::is_generator<Generator>::value, kleene_generator<Generator>>::type
operator*(Generator g)
{
   return kleene_generator<Generator>{g};
}

} } }

#endif
