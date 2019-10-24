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
#ifndef EOLIAN_CXX_NAMESPACES_HH
#define EOLIAN_CXX_NAMESPACES_HH

#include "grammar/generator.hpp"
#include "grammar/attributes.hpp"
#include "grammar/type_traits.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename Generator>
struct namespaces_generator
{
  template <typename OutputIterator, typename Attributes, typename Context>
  bool generate(OutputIterator sink, Attributes const& attributes, Context const& context) const
  {
     using std::get;
     typedef typename attributes::tuple_element<0ul, Attributes>::type namespaces_t;
     namespaces_t const& namespaces = attributes::cpp_namespaces(get<0>(attributes));
     std::size_t size = namespaces.size();

     for(auto&& n : namespaces)
       {
         const char keyword[] = "namespace ";
         const char open[] = " { ";
         std::copy(keyword, &keyword[0] + sizeof(keyword)-1, sink);
         std::transform(std::begin(n), std::end(n), sink, &::tolower);
         std::copy(open, &open[0] + sizeof(open)-1, sink);
       }
     *sink++ = '\n';
     
     if(!attributes::generate(as_generator(generator), sink, attributes::pop_front(attributes), context))
       return false;

     for(std::size_t i = 0; i != size; ++i)
       {
         const char close[] = "} ";
         std::copy(close, &close[0] + sizeof(close)-1, sink);
       }
     
     return true;
  }

  Generator generator;
};

struct namespaces_directive
{
  template <typename G>
  namespaces_generator<G> operator[](G g) const
  {
    return namespaces_generator<G>{g};
  }
} const namespaces = {};

template <typename G>
struct is_eager_generator<namespaces_generator<G>> : std::true_type {};
template <typename G>
struct is_generator<namespaces_generator<G>> : std::true_type {};

namespace type_traits {
template <typename G>
struct attributes_needed<namespaces_generator<G>> : std::integral_constant<int, attributes_needed<G>::value+1> {};
template <typename G>
struct accepts_tuple<namespaces_generator<G> > : std::true_type {};
}
      
} } }

#endif
