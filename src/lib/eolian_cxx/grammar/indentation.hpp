#ifndef EOLIAN_CXX_GRAMMAR_INDENTATION_HPP
#define EOLIAN_CXX_GRAMMAR_INDENTATION_HPP

#include "grammar/generator.hpp"
#include "grammar/attributes.hpp"

namespace efl { namespace eolian { namespace grammar {

struct scope_tab_generator
{
  scope_tab_generator(int n)
    : n(n) {}
  
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::unused_type, Context const&) const
  {
     for(int i = 0; i != n; ++i)
       {
          *sink++ = ' ';
          *sink++ = ' ';
          *sink++ = ' ';
       }
     return true;
  }

  int n;
};

template <>
struct is_eager_generator<scope_tab_generator> : std::true_type {};
      
struct scope_tab_terminal
{
  scope_tab_generator operator()(int n) const
  {
     return scope_tab_generator(n);
  }
} const scope_tab;

template <>
struct is_generator<scope_tab_terminal> : std::true_type {};

scope_tab_generator as_generator(scope_tab_terminal)
{
  return scope_tab_generator(1);
}
      
} } }

#endif
