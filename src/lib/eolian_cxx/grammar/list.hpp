#ifndef EOLIAN_CXX_LIST_HH
#define EOLIAN_CXX_LIST_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename G, typename S>
struct list_generator
{
   list_generator(G g, S s)
     : g(g), s(s) {}
  
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& context) const
   {
      auto generator = as_generator(g);
      auto separator = as_generator(s);
      bool first = true;
      for(auto&& c : attribute)
        {
           if(!first)
             if(!separator.generate(sink, attributes::unused, context)) return false;
           if(!generator.generate(sink, c, context))
             return false;
           first = false;
        }
      return true;
   }

   G g;
   S s;
};

template <typename G, typename S>
struct is_eager_generator<list_generator<G, S> > : std::true_type {};

namespace type_traits {
template  <typename G, typename S>
struct attributes_needed<list_generator<G, S> > : std::integral_constant<int, 1> {};
}
      
template <typename G, typename S>
typename std::enable_if<grammar::is_generator<G>::value && grammar::is_generator<S>::value, list_generator<G, S>>::type
operator%(G g, S s)
{
   return list_generator<G, S>{g, s};
}

} } }

#endif
