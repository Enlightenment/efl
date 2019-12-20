/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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

  explicit operator std::string() const
  {
     return std::string(n * m, ' ');
  }
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

  explicit operator std::string() const
  {
      return static_cast<std::string>(scope_tab_generator{1});
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
