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
#ifndef EOLIAN_MONO_TYPE_HH
#define EOLIAN_MONO_TYPE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"

namespace eolian_mono {

namespace attributes = efl::eolian::grammar::attributes;

template <typename OutputIterator, typename Context>
struct visitor_generate;
      
struct type_generator
{
   type_generator(bool is_return = false, bool is_optional = false, bool is_special_subtype = false)
     : is_return(is_return)
     , is_optional(is_optional)
     , is_special_subtype(is_special_subtype)
   {}

   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& type, Context const& context) const
   {
      return type.original_type.visit(visitor_generate<OutputIterator, Context>{
        sink
        , &context
        , type.c_type
        , false
        , is_return
        , type.is_ptr
        , is_optional
        , is_special_subtype
      });
   }
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return param.type.original_type.visit(visitor_generate<OutputIterator, Context>{
        sink
        , &context
        , param.type.c_type
        , param.direction != attributes::parameter_direction::in
        , false
        , param.type.is_ptr
        , is_optional
        , is_special_subtype
      });
   }

  bool is_return, is_optional, is_special_subtype;
};

struct type_terminal
{
  type_generator const operator()(bool is_return, bool is_optional = false, bool is_special_subtype = false) const
  {
    return type_generator(is_return, is_optional, is_special_subtype);
  }
} const type = {};

type_generator const as_generator(type_terminal)
{
  return type_generator{};
}

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::type_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::type_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::type_terminal> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::type_generator> : std::integral_constant<int, 1> {};  
template <>
struct attributes_needed< ::eolian_mono::type_terminal> : std::integral_constant<int, 1> {};  
}

} } }

#endif
