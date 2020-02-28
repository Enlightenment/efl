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
#ifndef EOLIAN_MONO_PROPERTY_DEFINITION_HH
#define EOLIAN_MONO_PROPERTY_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "parameter.hh"
#include "name_helpers.hh"
#include "using_decl.hh"
#include "blacklist.hh"

#include <eina_variant.hh>

namespace eolian_mono {

struct compare_get_and_set_value_type
{
  inline bool operator () (attributes::parameter_def const& get, attributes::parameter_def const& set) const;
  inline bool operator () (attributes::type_def const& get, attributes::type_def const& set) const;
};

struct compare_get_and_set_value_type_overload
{
  template <typename T, typename U>
  bool operator()(T const& /*left*/, U const& /*right*/) const
  {
    return false;
  }
  bool operator()(attributes::regular_type_def const& left, attributes::regular_type_def const& right) const
  {
    return left.base_type == right.base_type
      && left.namespaces == right.namespaces;
  }
  bool operator()(attributes::complex_type_def const& left, attributes::complex_type_def const& right) const
  {
    return (*this)(left.outer, right.outer)
      && std::equal (left.subtypes.begin(), left.subtypes.end(), right.subtypes.begin()
                     , compare_get_and_set_value_type{});
  }
  bool operator()(attributes::klass_name const& left, attributes::klass_name const& right) const
  {
    return left.namespaces == right.namespaces
      && left.eolian_name == right.eolian_name;
  }

  typedef bool result_type;
};

inline bool compare_get_and_set_value_type::operator () (attributes::parameter_def const& get, attributes::parameter_def const& set) const
{
  return efl::eina::visit(compare_get_and_set_value_type_overload{}, get.type.original_type, set.type.original_type);
}
inline bool compare_get_and_set_value_type::operator () (attributes::type_def const& get, attributes::type_def const& set) const
{
  return efl::eina::visit(compare_get_and_set_value_type_overload{}, get.original_type, set.original_type);
}

template <typename Context>
bool property_generate_wrapper_both_check(attributes::property_def const& property, Context const& context)
{
  if (blacklist::is_property_blacklisted(property, context))
    return false;

  bool is_interface = context_find_tag<class_context>(context).current_wrapper_kind == class_context::interface;
  bool is_static = (property.getter.is_engaged() && property.getter->is_static)
    || (property.setter.is_engaged() && property.setter->is_static);
  bool is_concrete = context_find_tag<class_context>(context).current_wrapper_kind == class_context::concrete;

  if ((is_concrete || is_interface) && is_static)
    return false;

  if (!property.getter)
    return false;

  if (property.setter)
  {
    if (property.getter->values.size() == property.setter->values.size())
    {
      if (!std::equal (property.getter->values.begin(), property.getter->values.end(), property.setter->values.begin()
                       , compare_get_and_set_value_type{}))
        return false;
    }
    else
      return false;
  }

  return true;
}

template <typename Context>
bool property_generate_wrapper_getter(attributes::property_def const& property, Context const& context)
{
  if (!property_generate_wrapper_both_check (property, context))
    return false;

  if (!property.getter->keys.empty())
    return false;

  if (property.getter->explicit_return_type != attributes::void_
      && property.getter->explicit_return_type.c_type != "Eina_Success_Flag")
  {
    return false;
  }

  assert (!!property.getter.is_engaged());

  bool is_interface = context_find_tag<class_context>(context).current_wrapper_kind == class_context::interface;
  if (is_interface)
  {
    std::string get_scope = property.getter.is_engaged() ? eolian_mono::function_scope_get(*property.getter) : "";
    bool is_get_public = get_scope == "public ";
    if (!is_get_public)
      return false;
  }
  return true;
}

template <typename Context>
bool property_generate_wrapper_setter (attributes::property_def const& property, Context const& context)
{
  if (!property_generate_wrapper_both_check (property, context))
    return false;
  if (!property.setter)
    return false;

  if (property.setter->explicit_return_type != attributes::void_)
    return false;

  if (!property.setter->keys.empty())
    return false;

  bool is_interface = context_find_tag<class_context>(context).current_wrapper_kind == class_context::interface;
  if (property.setter.is_engaged() && is_interface)
  {
    std::string set_scope = property.setter.is_engaged() ? eolian_mono::function_scope_get(*property.setter) : "";
    bool is_set_public = set_scope == "public ";
    if (!is_set_public)
      return false;
  }

  return true;
}

struct native_property_function_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::property_def const& property, Context const& context) const
  {
    EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "native_property_function_definition_generator: " << property.name << std::endl;

    if(blacklist::is_property_blacklisted(property, context))
      return true;

    auto const& indent = current_indentation(context);

    bool has_wrapper_getter = property_generate_wrapper_getter (property, context);
    bool has_wrapper_setter = property_generate_wrapper_setter (property, context);

    auto gen = [&] (attributes::function_def const& f, bool is_set)
    {
      // Delegate for the C# method we will export to EO as a method implementation.
      if(!as_generator
       (
        indent << eolian_mono::marshall_annotation(true) << "\n"
        << indent << "private delegate "
        << eolian_mono::marshall_type(true)
        << " "
        << string
        << "_delegate(" << (f.is_static ? "" : "System.IntPtr obj, System.IntPtr pd")
        << ((!f.is_static && f.parameters.size() > 0) ? ", " : "")
        << (grammar::attribute_reorder<-1, -1>
        (
         (marshall_annotation << " " << marshall_parameter)
        ) % ", ")
        << ");\n\n")
         .generate(sink,
                   std::make_tuple(f.return_type, f.return_type, f.c_name, f.parameters),
                   context_add_tag(direction_context{direction_context::native_to_managed}, context)))
        return false;

      // API delegate is the wrapper for the Eo methods exported from C that we will use from C#.
      if(!as_generator
       (
        indent << eolian_mono::marshall_annotation(true) << "\n"
        << indent << "internal delegate "
        << eolian_mono::marshall_type(true)
        << " "
        << string << "_api_delegate(" << (f.is_static ? "" : "System.IntPtr obj")
        << ((!f.is_static && f.parameters.size() > 0) ? ", " : "")
        << (grammar::attribute_reorder<-1, -1>
        (
         (marshall_annotation << " " << marshall_parameter)
        ) % ", ")
        << ");\n\n")
         .generate(sink, std::make_tuple(f.return_type, f.return_type, f.c_name, f.parameters), context))
        return false;

      // Delegate holder (so it can't be collected).
      if(!as_generator
         (indent << "internal static readonly Efl.Eo.FunctionWrapper<" << string << "_api_delegate> " << string << "_ptr = new Efl.Eo.FunctionWrapper<"
          << string << "_api_delegate>(Module, \"" << string << "\");\n\n")
         .generate(sink, std::make_tuple(f.c_name, f.c_name, f.c_name, f.c_name), context))
        return false;

      // We do not generate the wrapper to be called from C for non public interface member directly.
      if (blacklist::is_non_public_interface_member(f, *implementing_klass))
        return true;

      // Do not generate static method in interface
      if (((implementing_klass->type == attributes::class_type::interface_) ||
           (implementing_klass->type == attributes::class_type::mixin)) && f.is_static)
        return true;

      // Actual method implementation to be called from C.
      std::string return_type;
      if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
        return false;

      std::string klass_cast_name;
      if ((implementing_klass->type == attributes::class_type::interface_) ||
          ((implementing_klass->type == attributes::class_type::mixin) && !f.is_static))
        klass_cast_name = name_helpers::klass_interface_name(*implementing_klass);
      else
        klass_cast_name = name_helpers::klass_inherit_name(*implementing_klass);

      std::string self = "Efl.Eo.Globals.Super(obj, Efl.Eo.Globals.GetClass(obj))";

      if (f.is_static)
        self = "";

      if(!as_generator
       (indent << "[SuppressMessage(\"Microsoft.Reliability\", \"CA2000:DisposeObjectsBeforeLosingScope\", Justification = \"The instantiated objects can be stored in the called Managed API method.\")]\n"
        << indent << "private static "
        << eolian_mono::marshall_type(true) << " "
        << string
        << "(System.IntPtr obj, System.IntPtr pd"
        << *(", " << marshall_parameter)
        << ")\n"
        << indent << "{\n"
        << indent << scope_tab << "Eina.Log.Debug(\"function " << string << " was called\");\n"
        << indent << scope_tab << "var ws = Efl.Eo.Globals.GetWrapperSupervisor(obj);\n"
        << indent << scope_tab << "if (ws != null)\n"
        << indent << scope_tab << "{\n"
        << indent << scope_tab << scope_tab << eolian_mono::native_function_definition_preamble() << "\n"
        << indent << scope_tab << scope_tab << "try\n"
        << indent << scope_tab << scope_tab << "{\n"
        )
         .generate(sink, std::make_tuple(f.return_type, escape_keyword(f.name), f.parameters
                                         , /***/f.c_name/***/
                                         , f
                                         ), context))
        return false;
      if (is_set/* && has_wrapper_setter*/)
      {
        if(!as_generator
         (
           indent << scope_tab << scope_tab << scope_tab << (return_type != "void" ? "_ret_var = " : "")
        << (f.is_static ? "" : "((") << klass_cast_name << (f.is_static ? "." : ")ws.Target).") << string
          )
         .generate(sink, std::make_tuple(name_helpers::property_managed_name(property), f.parameters), context))
          return false;

        if(!f.keys.empty() && !as_generator(lit("[(") << (native_argument_invocation % ", ") << ")]").generate (sink, f.keys, context))
          return false;

        if(!as_generator
           (" = ("
            << (native_tuple_argument_invocation % ", ") << ");\n"
           )
           .generate(sink, f.values, context))
          return false;
      }
      else if (!is_set/* && has_wrapper_getter*/)
      {
        if(!as_generator
         (
           indent << scope_tab << scope_tab << scope_tab << "var ret = "
           << (f.is_static ? "" : "((") << klass_cast_name << (f.is_static ? "." : ")ws.Target).")
           << string

          )
           .generate(sink, std::make_tuple(name_helpers::property_managed_name(property)), context))
          return false;

        if(!f.keys.empty() && !as_generator(lit("[(") << (native_argument_invocation % ", ") << ")]").generate (sink, f.keys, context))
          return false;

        if (!as_generator(";\n").generate (sink, attributes::unused, context))
          return false;

      }
      // else if (!as_generator
      //          (indent << scope_tab << scope_tab << scope_tab << (return_type != "void" ? "_ret_var = " : "")
      //           << (f.is_static ? "" : "((") << klass_cast_name << (f.is_static ? "." : ")ws.Target).") << string
      //           << "(" << (native_argument_invocation % ", ") << ");\n"
      //           ).generate(sink, std::make_tuple(name_helpers::managed_method_name(f), f.parameters), context))
      //   return false;

      if(!as_generator
         (
           indent << scope_tab << scope_tab << "}\n"
        << indent << scope_tab << scope_tab << "catch (Exception e)\n"
        << indent << scope_tab << scope_tab << "{\n"
        << indent << scope_tab << scope_tab << scope_tab << "Eina.Log.Warning($\"Callback error: {e.ToString()}\");\n"
        << indent << scope_tab << scope_tab << scope_tab << "Eina.Error.Set(Eina.Error.UNHANDLED_EXCEPTION);\n"
        << indent << scope_tab << scope_tab << "}\n\n"
        << indent << eolian_mono::native_function_definition_epilogue(*implementing_klass) << "\n"
        << indent << scope_tab << "}\n"
        << indent << scope_tab << "else\n"
        << indent << scope_tab << "{\n"
        << indent << scope_tab << scope_tab << (return_type != "void" ? "return " : "") << string
        << "_ptr.Value.Delegate(" << self << ((!f.is_static && f.parameters.size() > 0) ? ", " : "") << (argument % ", ") << ");\n"
        << indent << scope_tab << "}\n"
        << indent << "}\n\n"
       )
         .generate(sink, std::make_tuple(f, f.c_name, f.parameters), context))
        return false;

      // Static functions do not need to be called from C
      if (f.is_static)
        return true;

      // This is the delegate that will be passed to Eo to be called from C.
      if(!as_generator(
            indent << "private static " << f.c_name << "_delegate " << f.c_name << "_static_delegate;\n\n"
        ).generate(sink,
                   attributes::unused,
                   context_add_tag(direction_context{direction_context::native_to_managed}, context)))
        return false;
    return true;
    };

    bool r = true;
    if(r && property.getter && has_wrapper_getter
       && helpers::is_function_registerable (*property.getter, *implementing_klass))
      r &= gen (*property.getter, false);
    if(r && property.setter && has_wrapper_setter
       && helpers::is_function_registerable (*property.setter, *implementing_klass))
      r &= gen (*property.setter, true);
    return r;
  }

   attributes::klass_def const* implementing_klass, *klass_from_property;
};

struct native_property_function_definition_parameterized
{
  native_property_function_definition_generator operator()(attributes::klass_def const& klass
                                                           , attributes::klass_def const& prop_from_klass) const
  {
    return {&klass, &prop_from_klass};
  }
} const native_property_function_definition;

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::native_property_function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_property_function_definition_generator> : std::true_type {};

namespace type_traits {

template <>
struct attributes_needed< ::eolian_mono::native_property_function_definition_generator> : std::integral_constant<int, 1> {};

} } } }

#endif
