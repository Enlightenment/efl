#ifndef EOLIAN_CXX_ATTRIBUTE_CONDITIONAL_HH
#define EOLIAN_CXX_ATTRIBUTE_CONDITIONAL_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename F, typename G, typename SpecificAttribute>
struct functional_attribute_conditional_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& ctx) const
   {
     if(f(attribute))
       return as_generator(g).generate(sink, attribute, ctx);
     else
       return false;
   }

   F f;
   G g;
};

template <typename F, typename SpecificAttribute>
struct functional_attribute_conditional_directive
{
  template <typename G>
  functional_attribute_conditional_generator<F, G, SpecificAttribute> operator[](G g) const
  {
    return {f, g};
  }

  template <typename OutputIterator, typename Attribute, typename Context>
  bool generate(OutputIterator, Attribute const& attribute, Context const&) const
  {
    return f(attribute);
  }
  
  F f;
};

// struct attribute_conditional_terminal
// {
//   template <typename F>
//   functional_attribute_conditional_directive<F> operator()(F f) const
//   {
//     return {f};
//   }
// } const attribute_conditional = {};

template <typename SpecificAttribute = void, typename F = void>
functional_attribute_conditional_directive<F, SpecificAttribute>
attribute_conditional(F f)
{
  return {f};
}

template <typename F, typename G, typename SpecificAttribute>
struct is_eager_generator<functional_attribute_conditional_generator<F, G, SpecificAttribute>> : std::true_type {};
template <typename F, typename SpecificAttribute>
struct is_eager_generator<functional_attribute_conditional_directive<F, SpecificAttribute>> : std::true_type {};
template <typename F, typename G, typename SpecificAttribute>
struct is_generator<functional_attribute_conditional_generator<F, G, SpecificAttribute>> : std::true_type {};
template <typename F, typename SpecificAttribute>
struct is_generator<functional_attribute_conditional_directive<F, SpecificAttribute>> : std::true_type {};
      
namespace type_traits {
template <typename F, typename G, typename SpecificAttribute>
struct attributes_needed<functional_attribute_conditional_generator<F, G, SpecificAttribute>>
  : std::conditional<(attributes_needed<G>::value >= 1)
                     , attributes_needed<G>
                     , std::integral_constant<int, 1>>::type {};  
template <typename F, typename SpecificAttribute>
struct attributes_needed<functional_attribute_conditional_directive<F, SpecificAttribute>> : std::integral_constant<int, 1> {};  
template <typename F, typename G, typename SpecificAttribute>
struct accepts_specific_tuple<functional_attribute_conditional_generator<F, G, SpecificAttribute>, SpecificAttribute> : std::true_type {};
template <typename F, typename G, typename SpecificAttribute, typename Attribute>
struct accepts_specific_tuple<functional_attribute_conditional_generator<F, G, SpecificAttribute>, Attribute>
  : accepts_specific_tuple<G, Attribute> {};
template <typename F, typename G, typename SpecificAttribute>
struct accepts_tuple<functional_attribute_conditional_generator<F, G, SpecificAttribute>>
  : accepts_tuple<G> {};

}
      
} } }

#endif
