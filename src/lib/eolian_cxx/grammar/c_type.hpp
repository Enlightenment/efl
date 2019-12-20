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
#ifndef EOLIAN_CXX_C_TYPE_HH
#define EOLIAN_CXX_C_TYPE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/string.hpp"

namespace efl { namespace eolian { namespace grammar { namespace attributes {

struct c_type_visitor
{
  std::string const* c_type;
  typedef std::string result_type;
  std::string operator()(attributes::klass_name const& name) const
  {
    std::string n;
    as_generator(" ::" << *(string << "_") << string << string << "*")
      .generate(std::back_insert_iterator<std::string>(n)
                , std::make_tuple(name.namespaces, name.eolian_name
                                  , std::string{name.base_qualifier & qualifier_info::is_const ? " const" : ""})
                , context_null {});
    if(name.base_qualifier & qualifier_info::is_ref)
      n += '*';
    return n;
  }
  template <typename T>
  std::string operator()(T const&) const
  {
    return *c_type;
  }
};
        
inline std::string c_type(parameter_def const& param)
{
   switch(param.direction)
     {
     case parameter_direction::in:
       return param.type.original_type.visit(c_type_visitor{&param.type.c_type});
     case parameter_direction::out:
     case parameter_direction::inout:
       return param.type.original_type.visit(c_type_visitor{&param.type.c_type}) + "*";
     default:
       throw std::runtime_error("Unknown parameter direction");
     };
}

}
      
struct c_type_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::parameter_def const& attribute, Context const& context) const
  {
    return as_generator(attributes::c_type(attribute)).generate(sink, attributes::unused, context);
  }
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::type_def const& attribute, Context const& context) const
  {
    return as_generator(attribute.original_type.visit(attributes::c_type_visitor{&attribute.c_type}))
      .generate(sink, attributes::unused, context);
  }
};

template <>
struct is_eager_generator<c_type_generator> : std::true_type {};
template <>
struct is_generator<c_type_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<c_type_generator> : std::integral_constant<int, 1> {};
}
      
c_type_generator const c_type = {};
      
} } }

#endif
