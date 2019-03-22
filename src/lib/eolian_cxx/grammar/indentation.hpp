#ifndef EOLIAN_CXX_GRAMMAR_INDENTATION_HPP
#define EOLIAN_CXX_GRAMMAR_INDENTATION_HPP

#include "grammar/generator.hpp"
#include "grammar/attributes.hpp"

namespace efl { namespace eolian { namespace grammar {

struct scope_tab_generator
{
  constexpr scope_tab_generator(int n, int m = 4)
    : n(n)
    , m(m)
  {}
  constexpr scope_tab_generator(scope_tab_generator const&) = default;
  scope_tab_generator& operator=(scope_tab_generator const&) = default;

  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::unused_type, Context const&) const
  {
     for(int i = 0; i != n; ++i)
       {
          for(int j = 0; j != m; ++j)
            *sink++ = ' ';
       }
     return true;
  }

  constexpr scope_tab_generator inc(int nplus = 1) const
  {
     return {n+nplus, m};
  }

  constexpr scope_tab_generator dec(int nminus = 1) const
  {
     return {n-nminus, m};
  }

  int n;
  int m;
};

template <>
struct is_eager_generator<scope_tab_generator> : std::true_type {};
template <>
struct is_generator<scope_tab_generator> : std::true_type {};
      
struct scope_tab_terminal
{
  scope_tab_generator operator()(int n, int m = 4) const
  {
     return {n, m};
  }

  operator scope_tab_generator() const
  {
     return {1};
  }
} const scope_tab = {};

template <>
struct is_generator<scope_tab_terminal> : std::true_type {};

scope_tab_generator as_generator(scope_tab_terminal)
{
  return scope_tab_generator(1);
}
      
} } }

#endif
