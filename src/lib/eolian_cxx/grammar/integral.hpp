#ifndef EOLIAN_CXX_INTEGRAL_HH
#define EOLIAN_CXX_INTEGRAL_HH

#include <cstdlib>
#include <cstring>

#include "grammar/generator.hpp"
#include "grammar/attributes.hpp"
#include "grammar/case.hpp"

namespace efl { namespace eolian { namespace grammar {

namespace detail {

template <typename OutputIterator, typename T>
void generate_integral(OutputIterator sink, T integer)
{
  std::stringstream stm;
  stm << integer;
  std::string string = stm.str();
  std::copy(string.begin(), string.end(), sink);
}

}

// literal
template <typename T>
struct literal_integral_generator
{
   T integral;

   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const&, Context const&) const
   {
      detail::generate_integral(sink, integral);
      return true;
   }
};

template <typename T, typename Enable = typename std::enable_if<std::is_integral<T>::value>::type>
literal_integral_generator<T> as_generator(T&& literal) { return {std::forward<T>(literal)}; }
literal_integral_generator<std::size_t> as_generator(std::size_t literal) { return {literal}; }

struct integral_terminal {
  template <typename T>
  literal_integral_generator<T> operator()(T literal) const
  {
    return {literal};
  }
} const int_ = {};

struct integral_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const&) const
   {
      detail::generate_integral(sink, attribute);
      return true;
   }
};

integral_generator as_generator(integral_terminal) { return {}; }
      
template <typename T>
struct is_eager_generator<literal_integral_generator<T>> : std::true_type {};
template <>
struct is_eager_generator<integral_generator> : std::true_type {};
template <typename T>
struct is_generator<literal_integral_generator<T>> : std::true_type {};
template <>
struct is_generator<integral_generator> : std::true_type {};
template <typename T>
struct is_generator<T, typename std::enable_if<std::is_integral<T>::value>::type> : std::true_type {};
template <>
struct is_generator<integral_terminal> : std::true_type {};

namespace type_traits {
template <typename T>
struct attributes_needed<literal_integral_generator<T>> : std::integral_constant<int, 0> {};  
template <>
struct attributes_needed<integral_generator> : std::integral_constant<int, 1> {};  
template <>
struct attributes_needed<integral_terminal> : std::integral_constant<int, 1> {};  
}      
      
} } }

#endif
