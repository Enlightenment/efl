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
#ifndef EOLIAN_CXX_TYPE_HH
#define EOLIAN_CXX_TYPE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename OutputIterator, typename Context>
struct visitor_generate;
      
struct type_generator
{
   type_generator(bool is_return = false)
     : is_return(is_return) {}
  
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& type, Context const& context) const
   {
      return type.original_type.visit(visitor_generate<OutputIterator, Context>{sink, &context, type.c_type, false, is_return});
   }
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return param.type.original_type.visit(visitor_generate<OutputIterator, Context>{sink, &context, param.type.c_type
            , param.direction != attributes::parameter_direction::in, false});
   }

   bool is_return;
};

struct type_terminal
{
  type_generator const operator()(bool is_return) const
  {
    return type_generator(is_return);
  }
} const type = {};

type_generator const as_generator(type_terminal)
{
  return type_generator{};
}

template <>
struct is_eager_generator<type_generator> : std::true_type {};
template <>
struct is_generator<type_generator> : std::true_type {};
template <>
struct is_generator<type_terminal> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<type_generator> : std::integral_constant<int, 1> {};  
template <>
struct attributes_needed<type_terminal> : std::integral_constant<int, 1> {};  
}

      
} } }

#endif
