#ifndef EOLIAN_CXX_IF_HH
#define EOLIAN_CXX_IF_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename F, typename TrueGenerator, typename FalseGenerator>
struct if_true_false_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& ctx) const
   {
     if(f(attribute))
       {
         return true_generator.generate(sink, attribute, ctx);
       }
     else
       {
         return false_generator.generate(sink, attribute, ctx);
       }
   }

   F f;
   TrueGenerator true_generator;
   FalseGenerator false_generator;
};

template <typename F, typename TrueGenerator>
struct if_true_generator
{
  template <typename FalseGenerator>
  if_true_false_generator<F, TrueGenerator, FalseGenerator> operator[](FalseGenerator false_g) const
  {
    return {f, true_generator, false_g};
  }

  template <typename OutputIterator, typename Attribute, typename Context>
  bool generate(OutputIterator sink, Attribute const& attribute, Context const& context) const
  {
    if(f(attribute))
      {
         return true_generator.generate(sink, attribute, context);
      }
    else
      {
        return false;
      }
  }
  
  F f;
  TrueGenerator true_generator;
};

template <typename F>
struct if_generator
{
  template <typename TrueGenerator>
  if_true_generator<F, TrueGenerator> operator[](TrueGenerator true_g) const
  {
    return {f, true_g};
  }

  template <typename OutputIterator, typename Attribute, typename Context>
  bool generate(OutputIterator, Attribute const& attribute, Context const&) const
  {
    return f(attribute);
  }
  
  F f;
};
      
struct if_terminal
{
  template <typename F>
  if_generator<F> operator()(F f) const
  {
    return {f};
  }
} const if_ = {};

template <typename F>
struct is_eager_generator<if_generator<F>> : std::true_type {};
template <typename F, typename TrueGenerator>
struct is_eager_generator<if_true_generator<F, TrueGenerator>> : std::true_type {};
template <typename F, typename TrueGenerator, typename FalseGenerator>
struct is_eager_generator<if_true_false_generator<F, TrueGenerator, FalseGenerator>> : std::true_type {};
      
template <typename F>
struct is_generator<if_generator<F>> : std::true_type {};
template <typename F, typename TrueGenerator>
struct is_generator<if_true_generator<F, TrueGenerator>> : std::true_type {};
template <typename F, typename TrueGenerator, typename FalseGenerator>
struct is_generator<if_true_false_generator<F, TrueGenerator, FalseGenerator>> : std::true_type {};
      
namespace type_traits {

template <typename F>
struct attributes_needed<if_generator<F>>
  : std::integral_constant<int, 1> {};
  
template <typename F, typename TrueGenerator>
struct attributes_needed<if_true_generator<F, TrueGenerator>> : attributes_needed<TrueGenerator> {};
template <typename F, typename TrueGenerator, typename FalseGenerator>
struct attributes_needed<if_true_false_generator<F, TrueGenerator, FalseGenerator>>
  : std::conditional<(attributes_needed<TrueGenerator>::value >= attributes_needed<FalseGenerator>::value)
    , attributes_needed<TrueGenerator>, attributes_needed<FalseGenerator>>::type
{};

template <typename F, typename TrueGenerator, typename T>
struct accepts_specific_tuple<if_true_generator<F, TrueGenerator>, T>
  : accepts_specific_tuple<TrueGenerator, T> {};
template <typename F, typename TrueGenerator, typename FalseGenerator, typename T>
struct accepts_specific_tuple<if_true_false_generator<F, TrueGenerator, FalseGenerator>, T>
  : std::integral_constant<bool, accepts_specific_tuple<TrueGenerator, T>::value
                           ||accepts_specific_tuple<FalseGenerator, T>::value>
{};

template <typename F, typename TrueGenerator>
struct accepts_tuple<if_true_generator<F, TrueGenerator>>
  : accepts_tuple<TrueGenerator> {};
template <typename F, typename TrueGenerator, typename FalseGenerator>
struct accepts_tuple<if_true_false_generator<F, TrueGenerator, FalseGenerator>>
  : std::integral_constant<bool, accepts_tuple<TrueGenerator>::value || accepts_tuple<FalseGenerator>::value>
{};

}
      
} } }

#endif
