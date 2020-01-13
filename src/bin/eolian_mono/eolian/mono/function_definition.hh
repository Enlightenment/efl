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
#ifndef EOLIAN_MONO_FUNCTION_DEFINITION_HH
#define EOLIAN_MONO_FUNCTION_DEFINITION_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/kleene.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/attribute_reorder.hpp"
#include "grammar/eps.hpp"
#include "grammar/counter.hpp"
#include "logging.hh"
#include "type.hh"
#include "name_helpers.hh"
#include "helpers.hh"
#include "function_helpers.hh"
#include "marshall_type.hh"
#include "parameter.hh"
#include "documentation.hh"
#include "using_decl.hh"
#include "generation_contexts.hh"
#include "blacklist.hh"

namespace eolian_mono {

struct native_function_definition_generator
{
  attributes::klass_def const* klass;

  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "native_function_definition_generator: " << f.c_name << std::endl;
    if(blacklist::is_function_blacklisted(f, context))
      return true;

    auto const& indent = current_indentation(context);

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
       .generate(sink, std::make_tuple(f.return_type, f.return_type, f.c_name, f.parameters), context))
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
    if (blacklist::is_non_public_interface_member(f, *klass))
      return true;

    // Do not generate static method in interface
    if (((klass->type == attributes::class_type::interface_) ||
        (klass->type == attributes::class_type::mixin)) && f.is_static)
      return true;

    // Actual method implementation to be called from C.
    std::string return_type;
    if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
      return false;

    std::string klass_cast_name;
    if ((klass->type == attributes::class_type::interface_) ||
        ((klass->type == attributes::class_type::mixin) && !f.is_static))
      klass_cast_name = name_helpers::klass_interface_name(*klass);
    else
      klass_cast_name = name_helpers::klass_inherit_name(*klass);

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
        << indent << scope_tab << scope_tab << scope_tab << (return_type != "void" ? "_ret_var = " : "")
        << (f.is_static ? "" : "((") << klass_cast_name << (f.is_static ? "." : ")ws.Target).") << string
        << "(" << (native_argument_invocation % ", ") << ");\n"
        << indent << scope_tab << scope_tab << "}\n"
        << indent << scope_tab << scope_tab << "catch (Exception e)\n"
        << indent << scope_tab << scope_tab << "{\n"
        << indent << scope_tab << scope_tab << scope_tab << "Eina.Log.Warning($\"Callback error: {e.ToString()}\");\n"
        << indent << scope_tab << scope_tab << scope_tab << "Eina.Error.Set(Eina.Error.UNHANDLED_EXCEPTION);\n"
        << indent << scope_tab << scope_tab << "}\n\n"
        << indent << eolian_mono::native_function_definition_epilogue(*klass) << "\n"
        << indent << scope_tab << "}\n"
        << indent << scope_tab << "else\n"
        << indent << scope_tab << "{\n"
        << indent << scope_tab << scope_tab << (return_type != "void" ? "return " : "") << string
        << "_ptr.Value.Delegate(" << self << ((!f.is_static && f.parameters.size() > 0) ? ", " : "") << (argument % ", ") << ");\n"
        << indent << scope_tab << "}\n"
        << indent << "}\n\n"
       )
       .generate(sink, std::make_tuple(f.return_type, escape_keyword(f.name), f.parameters
                                       , /***/f.c_name/***/
                                       , f
                                       , name_helpers::managed_method_name(f)
                                       , f.parameters
                                       , f
                                       , f.c_name
                                       , f.parameters
                                      )
                 , context))
      return false;

    // Static functions do not need to be called from C
    if (f.is_static)
      return true;

    // This is the delegate that will be passed to Eo to be called from C.
    if(!as_generator(
            indent << "private static " << f.c_name << "_delegate " << f.c_name << "_static_delegate;\n\n"
        ).generate(sink, attributes::unused, context))
      return false;

    return true;
  }
};

struct function_definition_generator
{
  function_definition_generator(bool do_super = false)
    : do_super(do_super)
  {}

  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "function_definition_generator: " << f.c_name << std::endl;

    bool is_concrete = context_find_tag<class_context>(context).current_wrapper_kind == class_context::concrete;
    if(blacklist::is_function_blacklisted(f, context))
      return true;

    // Do not generate static function for concrete class
    if (is_concrete && f.is_static)
      return true;

    std::string return_type;
    if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
      return false;

    if(!as_generator
       (documentation(2)).generate(sink, f, context))
      return false;

    std::string self = "this.NativeHandle";

    // IsGeneratedBindingClass is set in the constructor, true if this
    // instance is from a pure C# class (not generated).
    if (do_super && !f.is_static)
      self = "(IsGeneratedBindingClass ? " + self + " : Efl.Eo.Globals.Super(" + self + ", this.NativeClass))";
    else if (f.is_static)
      self = "";

    if(!as_generator
       (scope_tab(2) << eolian_mono::function_scope_get(f) << ((do_super && !f.is_static) ? "virtual " : "") << (f.is_static ? "static " : "") << return_type << " " << string << "(" << (parameter % ", ")
        << ") {\n"
        << scope_tab(3) << eolian_mono::function_definition_preamble()
        << klass_full_native_inherit_name(f.klass) << "." << string << "_ptr.Value.Delegate("
        << self
        << ((!f.is_static && (f.parameters.size() > 0)) ? ", " : "")
        << (argument_invocation % ", ") << ");\n"
        << scope_tab(3) << eolian_mono::function_definition_epilogue()
        << scope_tab(2) << "}\n\n")
       .generate(sink, std::make_tuple(name_helpers::managed_method_name(f), f.parameters, f, f.c_name, f.parameters, f), context))
      return false;

    return true;
  }

  bool do_super;
};

struct function_definition_parameterized
{
  function_definition_generator operator()(bool do_super) const
  {
    return {do_super};
  }
} const function_definition;
function_definition_generator as_generator(function_definition_parameterized)
{
  return {};
}
struct native_function_definition_parameterized
{
  native_function_definition_generator operator()(attributes::klass_def const& klass) const
  {
    return {&klass};
  }
} const native_function_definition;

struct property_extension_method_definition_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::property_def const& property, Context context) const
   {
      if (blacklist::is_property_blacklisted(property, context))
        return true;

      auto options = efl::eolian::grammar::context_find_tag<options_context>(context);

      if (!options.want_beta)
        return true; // Bindable is a beta feature for now.

      auto get_params = property.getter.is_engaged() ? property.getter->parameters.size() : 0;
      auto set_params = property.setter.is_engaged() ? property.setter->parameters.size() : 0;

      std::string managed_name = name_helpers::property_managed_name(property);

      if (get_params > 0 || set_params > 1)
        return true;

      std::string dir_mod;
      if (property.setter.is_engaged())
        dir_mod = direction_modifier(property.setter->parameters[0]);

      if (property.setter.is_engaged())
        {
          attributes::type_def prop_type = property.setter->parameters[0].type;
          if (!as_generator(scope_tab(2) << "public static Efl.BindableProperty<" << type(true) << "> " << managed_name << "<T>(this Efl.Ui.ItemFactory<T> fac, Efl.Csharp.ExtensionTag<"
                            << name_helpers::klass_full_concrete_or_interface_name(cls)
                            << ", T>magic = null) where T : " << name_helpers::klass_full_concrete_or_interface_name(cls) <<  " {\n"
                            << scope_tab(2) << scope_tab << "return new Efl.BindableProperty<" << type(true) << ">(\"" << property.name << "\", fac);\n"
                            << scope_tab(2) << "}\n\n"
                            ).generate(sink, std::make_tuple(prop_type, prop_type), context))
            return false;
        }

      // Do we need BindablePart extensions for this class?
      // IContent parts are handled directly through BindableFactoryParts
      if (!helpers::inherits_from(cls, "Efl.Ui.LayoutPart") || helpers::inherits_from(cls, "Efl.IContent"))
        return true;

      if (property.setter.is_engaged())
        {
          attributes::type_def prop_type = property.setter->parameters[0].type;
          if (!as_generator(scope_tab(2) << "public static Efl.BindableProperty<" << type(true) << "> " << managed_name << "<T>(this Efl.BindablePart<T> part, Efl.Csharp.ExtensionTag<"
                            << name_helpers::klass_full_concrete_or_interface_name(cls)
                            << ", T>magic = null) where T : " << name_helpers::klass_full_concrete_or_interface_name(cls) <<  " {\n"
                            << scope_tab(2) << scope_tab << "Contract.Requires(part != null, nameof(part));\n"
                            << scope_tab(2) << scope_tab << "return new Efl.BindableProperty<" << type(true) << ">(part.PartName, \"" << property.name << "\", part.Binder);\n"
                            << scope_tab(2) << "}\n\n"
                            ).generate(sink, std::make_tuple(prop_type, prop_type), context))
            return false;
        }

      return true;
   }

   grammar::attributes::klass_def const& cls;
};

property_extension_method_definition_generator property_extension_method_definition (grammar::attributes::klass_def const& cls)
{
  return {cls};
}

struct property_wrapper_definition_generator
{
   template <typename OutputIterator, typename Context>
   bool generate_get_indexer(OutputIterator sink, attributes::property_def const& property, Context const& context
                             , std::string get_scope
                             , bool is_interface) const
   {
     if (is_interface)
     {
        if (!as_generator(scope_tab(3) << get_scope <<  "get;\n"
                          ).generate(sink, attributes::unused, context))
          return false;
     }
     else
     {
        if (!as_generator(scope_tab(2) << scope_tab << get_scope << "get\n"
                          << scope_tab(2) << scope_tab << "{\n"
                          << scope_tab(2) << scope_tab(2) << "var i = new "
                          << name_helpers::property_concrete_indexer_name(property) << "();\n"
                          << scope_tab(2) << scope_tab(2) << "i.Self = this;\n"
                          << scope_tab(2) << scope_tab(2) << "return i;\n"
                          << scope_tab(2) << scope_tab << "}\n"
                          ).generate(sink, attributes::unused, context))
          return false;
     }

     return true;
   }

   template <typename OutputIterator, typename Context, typename C1, typename C2>
   bool generate_indexer(OutputIterator sink
                         , attributes::property_def const& property
                         , Context const& context
                         , std::string scope, std::string get_scope, std::string set_scope
                         , std::string class_name
                         , C1 keys, C2 values
                         , bool is_interface
                         , bool is_concrete_for_interface
                         , bool has_setter) const
   {
     if (is_interface)
       return true;

     auto size_not_one = [] (std::vector<attributes::parameter_def> k) { return k.size() != 1; };
     auto type_or_tuple
       =
       (
        (
         attribute_conditional(size_not_one)["("]
         << (type(false) % ", ")
         << ")"
        )
        | *type(false)
       )
       ;

     std::string parentship = "\n";

     bool is_self_property = *implementing_klass == *klass_from_property;

     if (!(is_self_property && !is_concrete_for_interface))
       parentship = " : " + name_helpers::property_interface_indexer_name(property, *klass_from_property) + "\n";

     if (!as_generator
         (
          scope_tab(2) << scope << "class " << name_helpers::property_concrete_indexer_name(property) << parentship
          << scope_tab(2) << "{\n"
          << scope_tab(3) << "public " << class_name << " Self {get; set;}\n"
          << scope_tab(3) << "public "
          << type_or_tuple << " this[" << type_or_tuple <<" i]\n"
          << scope_tab(3) << "{\n"
         ).generate(sink, make_tuple(values, values, keys, keys), context))
       return false;

     assert (!keys.empty());
     std::vector<std::string> get_keys;
     if(keys.size() != 1)
     {
       unsigned int i = 0;
       for (auto&& key : keys)
       {
         static_cast<void>(key);
         ++i;
         get_keys.push_back("i.Item" + std::to_string(i));
       }
     }
     else
     {
       get_keys.push_back ("i");
     }
     assert (!get_keys.empty());

     generate_get(sink, property, context, get_scope, get_keys, values, is_interface, "Self.");
     if (has_setter)
       generate_set(sink, property, context, set_scope, get_keys, values, is_interface, "Self.");

     if (!as_generator
         (
          scope_tab(3) << "}\n"
          << scope_tab(2) << "};\n"
          ).generate(sink, attributes::unused, context))
       return false;
     return true;
   }
   template <typename OutputIterator, typename Context, typename CK, typename CV>
   bool generate_set(OutputIterator sink, attributes::property_def const& property, Context const& context
                     , std::string set_scope
                     , CK keys, CV values
                     , bool is_interface
                     , std::string name_prefix = "") const
   {
     using efl::eolian::grammar::counter;
     if (is_interface)
     {
       if (!as_generator(scope_tab(2) << scope_tab << set_scope <<  "set;\n"
                         ).generate(sink, attributes::unused, context))
         return false;
     }
     else if (values.size() == 1)
     {
       if (!as_generator(scope_tab(2) << scope_tab << set_scope <<  "set " << "{ " << name_prefix << name_helpers::managed_method_name(*property.setter) + "(" << *(string << ",") << "value); }\n"
            ).generate(sink, keys, context))
         return false;
     }
     else if (values.size() > 1)
     {
       if (!as_generator(
            scope_tab(2) << scope_tab << set_scope <<  "set "
            << ("{ " << name_prefix << name_helpers::managed_method_name(*property.setter) + "(")
            << *(string << ",") << ((" value.Item" << counter(1)) % ", ")
            << "); }\n"
          ).generate(sink, std::make_tuple(keys, values), context))
         return false;
     }
     return true;
   }
   template <typename OutputIterator, typename Context, typename CK, typename CV>
   bool generate_get(OutputIterator sink, attributes::property_def const& property, Context const& context
                     , std::string get_scope
                     , CK keys, CV values
                     , bool is_interface
                     , std::string name_prefix = "") const
   {
      using efl::eolian::grammar::attribute_reorder;
      using efl::eolian::grammar::attributes::parameter_direction;
      using efl::eolian::grammar::attributes::parameter_def;

      if (is_interface) // only declaration
      {
        if (!as_generator(scope_tab(2) << scope_tab << get_scope <<  "get;\n"
                          ).generate(sink, attributes::unused, context))
          return false;
      }
      else
      if (/*has_getter && */values.size() == 1)
      {
        if (!as_generator
            (scope_tab(2) << scope_tab << get_scope
             << "get " << "{ return " << name_prefix << name_helpers::managed_method_name(*property.getter)
             << "(" << (string % ",") << "); }\n"
            ).generate(sink, keys, context))
          return false;
      }
      else if (/*has_getter && */values.size() > 1)
      {
        if (!as_generator
                 (scope_tab(2) << scope_tab << get_scope << "get "
                  << "{\n"
                  << *attribute_reorder<1, -1, 1>
                    (scope_tab(4) << type(true) << " _out_"
                     << argument(false) << " = default(" << type(true) << ");\n"
                    )
                  << scope_tab(4) << name_prefix << name_helpers::managed_method_name(*property.getter)
                  << "(" << *(string << ",") << (("out _out_" << argument(false)) % ", ") << ");\n"
                  << scope_tab(4) << "return (" << (("_out_"<< argument(false)) % ", ") << ");\n"
                  << scope_tab(3) << "}" << "\n"
                 ).generate(sink, std::make_tuple(values, keys, values, values), context))
          return false;
      }
      // else if (values.size() == 1)
      // {
      //   if (!as_generator
      //            (scope_tab << scope_tab << get_scope << "get "
      //             << "{\n"
      //             << *attribute_reorder<1, -1, 1>(scope_tab(3) << type(true) << " _out_" << argument(false) << " = default(" << type(true) << ");\n")
      //             << scope_tab(3) << name_prefix << name_helpers::managed_method_name(*property.getter)
      //             << "(" << *(string << ",") << (("out _out_" << argument(false)) % ",") << ");\n"
      //             << scope_tab(3) << "return " << (("_out_"<< argument(false)) % ",") << ";\n"
      //             << scope_tab(2) << "}" << "\n"
      //            ).generate(sink, std::make_tuple(values, keys, values, values), context))
      //     return false;
      // }
      return true;
   }

   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::property_def const& property, Context const& context) const
   {
      using efl::eolian::grammar::attribute_reorder;
      using efl::eolian::grammar::counter;
      using efl::eolian::grammar::attributes::parameter_direction;
      using efl::eolian::grammar::attributes::parameter_def;

      /// C(k) = keys count, C(v) = values count
      ///                                             /------------\           /------\.
      ///                                             |blacklisted?|---yes-----| skip |--------------\.
      ///                                             \------------/           \------/              |
      ///                                                 |                       |                  |
      ///                                                 no                     yes                 |
      ///                                                 |                       |                  |
      ///                                             /---------\            /------------\          |
      ///                                             |is-static|----yes-----|is-interface|          |
      ///                                             \---------/            \------------/          |
      ///                                                 |                       |                  |
      ///                                                 no                      no                 |
      ///                                                 |                       |                  |
      ///                                             /--------\             /-----------\           |
      ///                                             |has-get?|---no-conc---|is-concrete|-----yes---/
      ///                                             \--------/             \-----------/
      ///                                              /     \.
      ///                                            no      yes
      ///                                            /         \.
      ///                                         /----\     /--------------------------------------\.
      ///                                         |skip|-yes-|explicit return != Eina.Error or void |
      ///                                         \----/     \--------------------------------------/
      ///                                                        |
      ///                                                        no
      ///                                                        |
      ///                                                    /--------\.
      ///                                                    |has-set?|
      ///                                                    \--------/
      ///                                                     /     \.
      ///                                                   no      yes
      ///                                                   /         \.
      ///                                                /------\    /--------------------------------------\.
      ///                             /------------------|no-set|    |explicit return != Eina.Error or void |---- yes --\.
      ///                             |                  \------/    \--------------------------------------/           |
      ///                             |                     \------------|----------------------------------------------/
      ///                             |                                  no
      ///                             |                                  |
      ///                             |                              /--------\.
      ///                             |                              |has-both|
      ///                             |                              \--------/
      ///                             |                                  |
      ///                             |                      /-------------------\.
      ///                             |                      |set-keys = get-keys|
      ///                             |                      \-------------------/
      ///                             |                       /              |
      ///                             |                     no               |
      ///                             |                     /                |
      ///                             |                /----\       /-----------------------\.
      ///                             |                |skip|--no---|set-values = get-values|
      ///                             |                \----/       \-----------------------/
      ///                             |                                /
      ///                             |                              yes
      ///                             |                              /
      ///                             |                         /--------\.
      ///                             \-------------------------|  keys  |
      ///                                                       \--------/
      ///                                                       /        \.
      ///                                                      0         >0
      ///                                                     /           \.
      ///                                              /----------\    /----------\.
      ///                                              |no-indexer|    | keys > 1 |
      ///                                              \----------/    \----------/
      ///                                                |              /      |
      ///                                                |            no      yes
      ///                                                |            /        |
      ///                                                |           /         |
      ///                                                |   /---------\ /-------------------\.
      ///                                                |   | indexer | | indexer tuple key |
      ///                                                |   \---------/ \-------------------/
      ///                                                |     /           |
      ///                                              /--------\          |
      ///                                              | values |----------/
      ///                                              \--------/
      ///                                               /       \.
      ///                                              1        >1
      ///                                             /           \.
      ///                                     /----------------\  /-------------\.
      ///                                     | no tuple value |  | tuple value |
      ///                                     \----------------/  \-------------/
      ///

      auto has_wrapper = helpers::has_property_wrapper (property, implementing_klass, context);
      bool has_getter = has_wrapper & helpers::has_property_wrapper_bit::has_getter;
      if (!has_getter) return true;
      bool has_setter = has_wrapper & helpers::has_property_wrapper_bit::has_setter;
      bool has_indexer = has_wrapper & helpers::has_property_wrapper_bit::has_indexer;

      bool is_interface = context_find_tag<class_context>(context).current_wrapper_kind == class_context::interface;
      bool is_static = (property.getter.is_engaged() && property.getter->is_static)
                       || (property.setter.is_engaged() && property.setter->is_static);
      bool is_concrete = context_find_tag<class_context>(context).current_wrapper_kind == class_context::concrete;
      bool is_concrete_for_interface = is_concrete
        && (implementing_klass->type == attributes::class_type::interface_
            || implementing_klass->type == attributes::class_type::mixin);

      //if (name_helpers::klass_concrete_or_interface_name (*implementing_klass) == "IMapping")
      if (false)
      {
        if (!as_generator(grammar::lit("/// is interface ") << (int)is_interface
                          << " is static " << (int)is_static
                          << " is concrete " << (int)is_concrete
                          << " is concrete_for_interface " << (int)is_concrete_for_interface
                          << " klass_from_property->type " << (int)klass_from_property->type
                          << " has_setter " << (int)has_setter
                          << " property.setter->explicit_return_type != attributes::void_ " << (property.setter && property.setter->explicit_return_type != attributes::void_)
                          << " property.setter->keys != property.getter->keys " << (property.setter && property.setter->keys != property.getter->keys)
                          << " property.setter->values != property.getter->values " << (property.setter && property.setter->values != property.getter->values)
                          << " has_setter && property.setter->scope != attributes::member_scope::scope_public " << (property.setter && property.setter->scope != attributes::member_scope::scope_public)
                          << "\n")
            .generate (sink, attributes::unused, context))
          return false;
      }

      if (blacklist::is_property_blacklisted(property, context))
        return true;

      std::string managed_name = name_helpers::property_managed_name(property);

      std::string scope = "public ";
      std::string get_scope = eolian_mono::function_scope_get(*property.getter);
      std::string set_scope = has_setter ? eolian_mono::function_scope_get(*property.setter) : "";

      // C# interface members are declared automatically as public
      if (is_interface)
        {
           scope = "";
           get_scope = "";
           set_scope = "";
        }
      else if ((get_scope != "") && (get_scope == set_scope))
        {
           scope = get_scope;
           get_scope = "";
           set_scope = "";
        }
      else if (!has_setter || (get_scope == scope))
        {
           scope = get_scope;
           get_scope = "";
        }

      std::string virtual_mod = (is_static || is_interface || is_concrete) ? "" : "virtual ";

      auto keys = property.getter->keys;
      auto values = property.getter->values;
      auto generated_values = values;
      auto klass_name = name_helpers::klass_concrete_or_interface_name (*implementing_klass);

      if (has_indexer)
      {
        assert (!!implementing_klass);
        generate_indexer (sink, property, context, scope, get_scope, set_scope
                          , klass_name, keys, values
                          , is_interface, is_concrete_for_interface, has_setter);

        generated_values.clear();
        if (!is_interface && *implementing_klass == *klass_from_property
            && !is_concrete_for_interface)
        {
          generated_values.push_back
            (attributes::parameter_def
             {parameter_direction::in
                , attributes::type_def
                {
                  attributes::regular_type_def{name_helpers::property_concrete_indexer_name(property), {attributes::qualifier_info::is_none, ""}, {}}
                  , name_helpers::property_concrete_indexer_name(property)
                  , false, false, false, ""
                }
              , "indexer", {}, nullptr
            });
        }
        else
        {
          generated_values.push_back
            (attributes::parameter_def
             {parameter_direction::in
                , attributes::type_def
                {
                  attributes::regular_type_def{name_helpers::klass_full_concrete_or_interface_name (*klass_from_property) + managed_name + "Indexer", {attributes::qualifier_info::is_none, ""}, {}}
                  , name_helpers::property_interface_indexer_name(property, *klass_from_property)
                  , false, false, false, ""
                }
              , "indexer", {}, nullptr
            });
        }
      }

      if (generated_values.size() == 1)
      {
        if (!as_generator(
                    documentation(2)
                    << scope_tab(2) << scope << (is_static ? "static " : virtual_mod) << type(true) << " " << managed_name << " {\n"
              ).generate(sink, std::make_tuple(property, generated_values[0].type), context))
          return false;
      }
      else
      {
        if (!as_generator
            (
             documentation(2)
             << scope_tab(2) << scope << (is_static ? "static (" : "(")
             << (attribute_reorder<1, -1>(type(true) /*<< " " << argument*/) % ", ") << ") "
             << managed_name << " {\n"
            ).generate(sink, std::make_tuple(property, generated_values), context))
          return false;
      }

      if (has_indexer)
      {
        generate_get_indexer (sink, property, context, get_scope, is_interface);
      }
      else
      {
        std::vector<std::string> empty_keys;
        generate_get(sink, property, context, get_scope, empty_keys, values, is_interface);

        if (has_setter)
          generate_set (sink, property, context, set_scope, empty_keys, values, is_interface);
      }

      if (!as_generator(scope_tab(2) << "}\n\n").generate(sink, attributes::unused, context))
        return false;

      return true;
   }
   attributes::klass_def const* implementing_klass, *klass_from_property;
};
struct property_wrapper_definition_parameterized
{
  property_wrapper_definition_generator operator()(attributes::klass_def const& klass
                                                   , attributes::klass_def const& prop_from_klass) const
  {
    return {&klass, &prop_from_klass};
  }
} const property_wrapper_definition;
property_wrapper_definition_generator as_generator(property_wrapper_definition_parameterized)
{
   return {};
}

struct interface_property_indexer_definition_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::property_def const& property, Context const& context) const
   {
      using efl::eolian::grammar::attribute_reorder;
      using efl::eolian::grammar::counter;
      using efl::eolian::grammar::attributes::parameter_direction;
      using efl::eolian::grammar::attributes::parameter_def;

      bool is_interface = context_find_tag<class_context>(context).current_wrapper_kind == class_context::interface;

      assert (is_interface);
      auto klass_name = name_helpers::klass_concrete_or_interface_name (*implementing_klass);
      std::string managed_name = name_helpers::property_managed_name(property);

      if (!as_generator
           (scope_tab << "public interface " << name_helpers::property_interface_indexer_short_name(property, *implementing_klass) << "\n"
           << scope_tab << "{\n"
           << scope_tab << "}\n"
           ).generate (sink, attributes::unused, context))
        return false;

      return true;
   }
   attributes::klass_def const* implementing_klass;
};
struct interface_property_indexer_definition_parameterized
{
  interface_property_indexer_definition_generator operator()(attributes::klass_def const& klass) const
  {
     return {&klass};
  }
} const interface_property_indexer_definition;
interface_property_indexer_definition_generator as_generator(interface_property_indexer_definition_parameterized)
{
   return {};
}

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::function_definition_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::native_function_definition_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::property_extension_method_definition_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::property_wrapper_definition_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::property_wrapper_definition_parameterized> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::interface_property_indexer_definition_parameterized> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::interface_property_indexer_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_definition_parameterized> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::property_extension_method_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::property_wrapper_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::property_wrapper_definition_parameterized> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::interface_property_indexer_definition_parameterized> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::interface_property_indexer_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::function_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::function_definition_parameterized> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::native_function_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::property_extension_method_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::property_wrapper_definition_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::property_wrapper_definition_parameterized> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::interface_property_indexer_definition_parameterized> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::interface_property_indexer_definition_generator> : std::integral_constant<int, 1> {};
}

} } }

#endif
