#ifndef EOLIAN_CXX_COUNTER_HH_HH
#define EOLIAN_CXX_COUNTER_HH_HH

#include <cstdlib>
#include <cstring>

#include "grammar/generator.hpp"
#include "grammar/attributes.hpp"
#include "grammar/case.hpp"
#include "grammar/integral.hpp"

namespace efl { namespace eolian { namespace grammar {

namespace detail {

}

struct counter_generator
{
   std::shared_ptr<std::size_t> count;

   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const&, Context const&) const
   {
      detail::generate_integral(sink, *count);
      ++*count;
      return true;
   }
};

struct counter_terminal
{
  counter_generator operator()(std::size_t initial) const
  {
    return {std::shared_ptr<std::size_t>{new std::size_t{initial}}};
  }
} const counter = {};

counter_generator as_generator(counter_terminal) { return {std::shared_ptr<std::size_t>{new std::size_t{0u}}}; }

template <>
struct is_eager_generator<counter_generator> : std::true_type {};
template <>
struct is_eager_generator<counter_terminal> : std::true_type {};
template <>
struct is_generator<counter_terminal> : std::true_type {};
template <>
struct is_generator<counter_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<counter_generator> : std::integral_constant<int, 0> {};
template <>
struct attributes_needed<counter_terminal> : std::integral_constant<int, 0> {};
}

} } }

#endif
