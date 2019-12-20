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
#ifndef EOLIAN_CXX_TYPE_FUNCTION_DECLARATION_HH
#define EOLIAN_CXX_TYPE_FUNCTION_DECLARATION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/keyword.hpp"
#include "grammar/converting_argument.hpp"
#include "grammar/eps.hpp"

namespace efl { namespace eolian { namespace grammar {

/** This generates the caller struct for function pointers. */
struct type_function_declaration_generator {
   type_function_declaration_generator() {}

   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::function_def const& f, Context const& ctx) const
   {
      std::string guard = f.c_name + "_defined";

      if (!as_generator("#ifndef " << string << "\n" <<
                        "#define " << string << "\n")
          .generate(sink, std::make_tuple(guard, guard), add_upper_case_context(ctx)))
        return false;

      // efl::eolian::function_wrapper<T, F>
      if (!as_generator("namespace efl { namespace eolian {\n")
          .generate(sink, attributes::unused, add_lower_case_context(ctx)))
      return false;

      if (!as_generator(
             "template <typename F>\n"
             "struct function_wrapper<" << string << ", F, struct " << string << "__function_tag> {\n"
             << scope_tab << "function_wrapper(F cxx_func) : _cxx_func(cxx_func) {}\n"
             ).generate(sink, std::make_tuple(f.c_name, f.c_name), ctx))
        return false;

      if (!as_generator(
             scope_tab << "void *data_to_c() { return static_cast<void *>(this); }\n"
             << scope_tab << string << " func_to_c() const { return &caller; }\n"
             << scope_tab << "Eina_Free_Cb free_to_c() const { return &deleter; }\n"
             << "private:\n"
             << scope_tab << "F _cxx_func;\n"
             << scope_tab << "static void deleter(void *data) {\n"
             << scope_tab << scope_tab << "delete static_cast<function_wrapper<" << string << ", F, ::efl::eolian::" << string << "__function_tag>*>(data);\n"
             << scope_tab << "}\n"
             ).generate(sink, std::make_tuple(f.c_name, f.c_name, f.c_name), ctx))
        return false;

      std::vector<std::string> c_args;
      for (auto itr : f.parameters)
        {
           std::string arg;
           if (!as_generator(grammar::c_type << " " << string).generate(std::back_inserter(arg), std::make_tuple(itr, itr.param_name), ctx))
             return false;
           c_args.push_back(arg);
        }
      if (!as_generator(
             scope_tab << "static " << string << " caller(void *cxx_call_data"
             << *(", " << string) << ") {\n"
             << scope_tab << scope_tab << "auto fw = static_cast<function_wrapper<"
             << string << ", F, ::efl::eolian::" << string << "__function_tag>*>(cxx_call_data);\n"
             ).generate(sink, std::make_tuple(f.return_type.c_type, c_args, f.c_name, f.c_name), ctx))
        return false;

      if (f.return_type != attributes::void_
          && !as_generator(scope_tab << scope_tab << "auto __return_value =\n")
          .generate(sink, attributes::unused, ctx))
        return false;

      if (!f.parameters.empty())
        {
           std::vector<attributes::parameter_def> params;
           for (auto itr = f.parameters.begin() + 1; itr != f.parameters.end(); itr++)
             params.push_back(*itr);
           if (!as_generator(
                  scope_tab << scope_tab << "fw->_cxx_func(" << parameter_as_argument << *(", " << parameter_as_argument) << ");\n"
                  ).generate(sink, std::make_tuple(*f.parameters.begin(), params), ctx))
               return false;
        }

      if (f.return_type != attributes::void_
          && !as_generator(scope_tab << scope_tab << "return ::efl::eolian::convert_to_c<"
                           << type << ">(__return_value);\n")
          .generate(sink, f.return_type, ctx))
        return false;

      if (!as_generator(scope_tab << "}\n").generate(sink, attributes::unused, ctx))
        return false;

      if (!as_generator("};\n"
                        "} }\n"
                        "#endif\n\n")
          .generate(sink, attributes::unused, ctx))
        return false;

      return true;
   }
};

template <>
struct is_eager_generator<type_function_declaration_generator> : std::true_type {};
template <>
struct is_generator<type_function_declaration_generator> : std::true_type {};
namespace type_traits {
template <>
struct attributes_needed<type_function_declaration_generator> : std::integral_constant<int, 1> {};
}

struct type_function_declaration_terminal
{
  type_function_declaration_generator operator()() const
  {
    return type_function_declaration_generator{};
  }
} const type_function_declaration = {};

} } }

#endif
