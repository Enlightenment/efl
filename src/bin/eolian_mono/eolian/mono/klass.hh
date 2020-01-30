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
#ifndef EOLIAN_MONO_CLASS_DEFINITION_HPP
#define EOLIAN_MONO_CLASS_DEFINITION_HPP

#include "grammar/integral.hpp"
#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/attribute_conditional.hpp"
#include "blacklist.hh"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "name_helpers.hh"
#include "async_function_definition.hh"
#include "function_definition.hh"
#include "property_definition.hh"
#include "function_registration.hh"
#include "function_declaration.hh"
#include "documentation.hh"
#include "part_definition.hh"
#include "events.hh"
#include "grammar/string.hpp"
#include "grammar/attribute_replace.hpp"
#include "grammar/integral.hpp"
#include "grammar/case.hpp"
#include "using_decl.hh"
#include "utils.hh"

#include <string>
#include <algorithm>

namespace eolian_mono {

template<typename Context>
static bool
is_inherit_context(Context const& context)
{
   return context_find_tag<class_context>(context).current_wrapper_kind == class_context::inherit;
}

enum partial_class
{
 class_partial = 1
};

struct klass
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "klass_generator: " << cls.eolian_name << std::endl;

     auto const& indent = current_indentation(context);

     if (blacklist::is_class_blacklisted(cls, context))
       {
          EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "class " << cls.eolian_name << " is blacklisted. Skipping." << std::endl;
          return true;
       }

     std::string suffix, class_type;
     switch(cls.type)
       {
       case attributes::class_type::regular:
         class_type = "class";
         suffix = "CLASS";
         break;
       case attributes::class_type::abstract_:
         class_type = "abstract class";
         suffix = "CLASS";
         break;
       case attributes::class_type::mixin:
         class_type = "interface";
         suffix = "MIXIN";
         break;
       case attributes::class_type::interface_:
         class_type = "interface";
         suffix = "INTERFACE";
         break;
       }

     if (!name_helpers::open_namespaces(sink, cls.namespaces, context))
       return false;

     auto methods = cls.get_all_methods();

     // Interface class
     if(class_type == "interface")
     {
       auto iface_cxt = context_add_tag(class_context{class_context::interface,
                                        name_helpers::klass_full_concrete_or_interface_name(cls)},
                                        context);

       if(!as_generator(documentation(1)).generate(sink, cls, iface_cxt))
         return false;

       // Mark the interface with the proper native Efl_Class* getter
       if(!as_generator(scope_tab << lit("[") << name_helpers::klass_full_native_inherit_name(cls) << "]\n")
          .generate(sink, attributes::unused, iface_cxt))
         return false;

       if(!as_generator(scope_tab << "[Efl.Eo.BindingEntity]\n").generate(sink, attributes::unused, iface_cxt))
         return false;

       using efl::eolian::grammar::lit;
       if(!as_generator
        (
         scope_tab << lit("public ") << (is_partial ? "partial ":"")
         /*<< class_type*/ << "interface" /*<<*/ " " << string << " : "
         )
        .generate(sink, name_helpers::klass_interface_name(cls), iface_cxt))
         return false;
       for(auto first = std::begin(cls.immediate_inherits)
             , last = std::end(cls.immediate_inherits); first != last; ++first)
         {
            if (!context_find_tag<options_context>(iface_cxt).want_beta && first->is_beta)
              continue;

            if(first->type != attributes::class_type::regular && first->type != attributes::class_type::abstract_)
              if(!as_generator("\n" << scope_tab << string << ",").generate(sink, name_helpers::klass_full_interface_name(*first), iface_cxt))
                return false;
         }

       if(!as_generator("\n" << scope_tab(2) << "Efl.Eo.IWrapper, IDisposable").generate(sink, attributes::unused, iface_cxt))
         return false;

       if(!as_generator("\n" << scope_tab << "{\n").generate(sink, attributes::unused, iface_cxt))
         return false;

       auto properties = cls.properties;
       auto functions = cls.functions;
       functions.erase (std::remove_if (functions.begin(), functions.end()
                                        , [&] (attributes::function_def const& f)
                                        {
                                          auto it = std::find_if (properties.begin(), properties.end()
                                                                  , [&f] (attributes::property_def const& prop)
                                                                    {
                                                                      return (prop.getter && *prop.getter == f)
                                                                        || (prop.setter && *prop.setter == f);
                                                                    });
                                          if (it != properties.end())
                                          {
                                            if (it->getter && *it->getter == f)
                                              return property_generate_wrapper_getter (*it, iface_cxt);
                                            else if (it->setter && *it->setter == f)
                                              return property_generate_wrapper_setter (*it, iface_cxt);
                                          }
                                          return false;
                                        }), functions.end());
       if(!as_generator(*(function_declaration)).generate(sink, functions, iface_cxt))
         return false;

       if(!as_generator(*(async_function_declaration)).generate(sink, cls.functions, iface_cxt))
         return false;

       if(!as_generator(*(event_declaration)).generate(sink, cls.events, iface_cxt))
         return false;

       for (auto &&p : cls.parts)
         if (!as_generator(
              documentation(2)
              << name_helpers::klass_full_concrete_or_interface_name(p.klass) << " " << utils::capitalize(p.name) << "{ get;}\n"
            ).generate(sink, p, iface_cxt))
           return false;

       properties.erase(std::remove_if (properties.begin(), properties.end()
                                        , [&] (attributes::property_def const& prop)
                                          { return !property_generate_wrapper_getter (prop, iface_cxt); }), properties.end());
       if (!as_generator(*(property_wrapper_definition(cls))).generate(sink, properties, iface_cxt))
         return false;

       // End of interface declaration
       if(!as_generator(scope_tab << "}\n\n").generate(sink, attributes::unused, iface_cxt)) return false;
     }

     // Events arguments go in the top namespace to avoid the Concrete suffix clutter in interface events.
     // Regular/abstract class events go here too for consistency.
     if(!as_generator(*(event_argument_wrapper)).generate(sink, cls.events, context))
       return false;

     bool root = !helpers::has_regular_ancestor(cls);
     std::set<attributes::klass_name, attributes::compare_klass_name_by_name> inherit_interfaces = helpers::non_implemented_interfaces(cls, context);
     std::vector<attributes::klass_name> inherit_classes;
     std::copy_if(cls.immediate_inherits.begin(), cls.immediate_inherits.end()
                  , std::back_inserter(inherit_classes)
                  , [] (attributes::klass_name const& klass)
                    {
                       switch (klass.type)
                         {
                          case attributes::class_type::regular:
                          case attributes::class_type::abstract_:
                            return true;
                          default:
                            return false;
                         };
                    });

     // Concrete class for interfaces, mixins, etc.
     if(class_type != "class" && class_type != "abstract class")
       {
         auto concrete_cxt = context_add_tag(class_context{class_context::concrete,
                                             name_helpers::klass_full_concrete_or_interface_name(cls)},
                                             context);

         if(!generate_native_inherit_class(sink, cls, change_indentation(indent, concrete_cxt)))
           return true;
       }

     // Inheritable class
     if(class_type == "class" || class_type == "abstract class")
       {
        auto inherit_cxt = context_add_tag(class_context{class_context::inherit,
                                           name_helpers::klass_full_concrete_or_interface_name(cls)},
                                           context);

        // Class header
        if(!as_generator
            (
             documentation(1)
             << scope_tab << "[" << name_helpers::klass_full_native_inherit_name(cls) << "]\n"
             << scope_tab << "[Efl.Eo.BindingEntity]\n"
             << scope_tab << "[SuppressMessage(\"Microsoft.Naming\", \"CA1724:TypeNamesShouldNotMatchNamespaces\")]\n"
             << scope_tab<< "public "
             << (is_partial
                 ? class_type == "class"
                 ? "partial class"
                 : "abstract partial class"
                 : class_type
                )
             << " " << name_helpers::klass_concrete_name(cls) << " : "
             << (klass_full_concrete_or_interface_name % ",") // classes
             << (root ? "Efl.Eo.EoWrapper" : "") // ... or root
             << (inherit_interfaces.empty() ? "" : ", ")
             << (klass_full_concrete_or_interface_name % ", ") // interfaces
             << "\n" << scope_tab << "{\n"
             )
           .generate(sink, std::make_tuple(cls, inherit_classes, inherit_interfaces), inherit_cxt))
           return false;


         // Class body
         if(!generate_fields(sink, cls, inherit_cxt))
           return false;

         if (!generate_constructors(sink, cls, inherit_cxt))
           return false;

         if (!generate_events(sink, cls, inherit_cxt))
             return false;

         // Parts
         if(!as_generator(*(part_definition))
            .generate(sink, cls.parts, inherit_cxt)) return false;

         // Inherit function definitions
         auto implemented_methods = helpers::get_all_implementable_methods(cls, inherit_cxt);
         if(!as_generator(*(function_definition(true, helpers::get_all_implementable_properties(cls, inherit_cxt))))
            .generate(sink, implemented_methods, inherit_cxt)) return false;

         // Async wrappers
         if(!as_generator(*(async_function_definition(true))).generate(sink, implemented_methods, inherit_cxt))
           return false;

         // Property wrappers
         if (!as_generator(*(property_wrapper_definition(cls))).generate(sink, cls.properties, inherit_cxt))
           return false;

         for (auto&& klass : helpers::non_implemented_interfaces(cls, inherit_cxt))
           {
              attributes::klass_def c(get_klass(klass, cls.unit), cls.unit);
              if (!as_generator(*(property_wrapper_definition(cls))).generate(sink, c.properties, inherit_cxt))
                return false;
           }

         // Copied from nativeinherit class, used when setting up providers.
         if(!as_generator(
              scope_tab(2) << "private static IntPtr GetEflClassStatic()\n"
              << scope_tab(2) << "{\n"
              << scope_tab(2) << scope_tab << "return " << name_helpers::klass_get_full_name(cls) << "();\n"
              << scope_tab(2) << "}\n\n"
           ).generate(sink, attributes::unused, inherit_cxt))
           return false;

         if(!generate_native_inherit_class(sink, cls, change_indentation(indent.inc(), inherit_cxt)))
           return true;

         if(!as_generator(scope_tab << "}\n").generate(sink, attributes::unused, inherit_cxt)) return false;
       }


     if(!name_helpers::close_namespaces(sink, cls.namespaces, context))
       return false;

     std::vector<attributes::property_def> implementable_properties;
     std::copy(cls.properties.begin(), cls.properties.end(), std::back_inserter(implementable_properties));

     for (auto&& klass : helpers::non_implemented_interfaces(cls, context))
     {
         attributes::klass_def c(get_klass(klass, cls.unit), cls.unit);
         std::copy(c.properties.begin(), c.properties.end(), std::back_inserter(implementable_properties));
     }

     std::stringstream extension_method_stream;
     std::ostream_iterator<char> extension_method_iterator(extension_method_stream);

     if (!as_generator
         (*property_extension_method_definition(cls)
          << *part_extension_method_definition(cls))
         .generate(extension_method_iterator, std::make_tuple(implementable_properties, cls.parts), context))
       return false;

     if (extension_method_stream.tellp() <= 0)
       return true;

     if (!name_helpers::open_namespaces(sink, cls.namespaces, context))
       return false;

     if(!as_generator
        (lit("#if EFL_BETA\n")
         << "#pragma warning disable CS1591\n" // Disabling warnings as DocFx will hide these classes
         << scope_tab << "public static class " << name_helpers::klass_concrete_name(cls)
         << "Extensions {\n"
         << extension_method_stream.str()
         << scope_tab << "}\n"
         << "#pragma warning restore CS1591\n"
         << "#endif\n")
        .generate(sink, cls.namespaces, context))
     return false;

     if (!name_helpers::close_namespaces(sink, cls.namespaces, context))
       return false;

     return true;
   }

   // NativeInherit class. Contains function pointers for the native Eo methods and delegates that are registered in Eo as virtual method implementations
   // These delegates are called from C to C#, checking whether the C# subclass reimplemented it.
   template <typename OutputIterator, typename Context>
   bool generate_native_inherit_class(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
       {
         auto inative_cxt = context_add_tag(class_context{class_context::inherit_native,
                                            name_helpers::klass_full_concrete_or_interface_name(cls)},
                                            context);
         auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);
         auto inherit_name = name_helpers::klass_inherit_name(cls);
         auto implementable_methods = helpers::get_all_registerable_methods(cls, context);
         auto implementable_properties = helpers::get_all_implementable_properties(cls, context);
         bool root = !helpers::has_regular_ancestor(cls);
         bool is_concrete = context_find_tag<class_context>(context).current_wrapper_kind == class_context::concrete;
         auto const& indent = current_indentation(inative_cxt).inc();
         std::string klass_since;

         if (!documentation_helpers::generate_since_tag_line(std::back_inserter(klass_since), cls.documentation, indent, context))
           return false;

         std::string base_name;
         if(!root)
           {
              attributes::klass_def parent_klass(get_klass(*cls.parent, cls.unit), cls.unit);
              base_name = name_helpers::klass_full_native_inherit_name(parent_klass);
           }

         if(!as_generator
            (
             indent << lit("/// <summary>Wrapper for native methods and virtual method delegates.\n")
             << indent << "/// For internal use by generated code only.\n"
             << klass_since
             << indent << "/// </summary>\n"
             << indent << "[EditorBrowsable(EditorBrowsableState.Never)]\n"
             << indent << "internal " << (is_concrete ? "" : "new ") << "class " << native_inherit_name << " : " << (root ? "Efl.Eo.EoWrapper.NativeMethods" : base_name) << "\n"
             << indent << "{\n"
            ).generate(sink, attributes::unused, inative_cxt))
           return false;

         if(is_concrete)
           {
              if (!as_generator
                 (
                  scope_tab(2) << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(context).actual_library_name(cls.filename)
                  << ")] internal static extern System.IntPtr\n"
                  << scope_tab(2) << scope_tab << name_helpers::klass_get_name(cls) << "();\n"
                 )
                 .generate(sink, attributes::unused, inative_cxt))
                return false;
           }

         if(!implementable_methods.empty())
           {
              if(!as_generator(
                    indent << scope_tab << "private static Efl.Eo.NativeModule Module = new Efl.Eo.NativeModule("
                    <<  context_find_tag<library_context>(context).actual_library_name(cls.filename) << "); // " << implementable_methods.size()
                    << " " << implementable_properties.size() << "\n\n"
                 ).generate(sink, attributes::unused, inative_cxt))
                return false;
           }

         if(!as_generator(
             indent << scope_tab << "/// <summary>Gets the list of Eo operations to override.\n"
             << klass_since
             << indent << "/// </summary>\n"
             << indent << scope_tab << "/// <returns>The list of Eo operations to be overload.</returns>\n"
             << indent << scope_tab << "internal override System.Collections.Generic.List<EflOpDescription> GetEoOps(System.Type type, bool includeInherited)\n"
             << indent << scope_tab << "{\n"
             << indent << scope_tab << scope_tab << "var descs = new System.Collections.Generic.List<EflOpDescription>();\n"
            )
            .generate(sink, attributes::unused, inative_cxt))
           return false;

         // Native wrapper registration
         // We write them first to a temporary function as the implementable function list may contain
         // only non-registrable methods like class functions, leading to unused `methods` variable.
         std::string tmp_registration;
         if(!as_generator(*(function_registration(cls)))
            .generate(std::back_inserter(tmp_registration), implementable_methods, change_indentation(indent.inc(2), inative_cxt)))
           return false;

         if (tmp_registration.find("methods") != std::string::npos)
           if (!as_generator(
                    indent << scope_tab << scope_tab << "var methods = Efl.Eo.Globals.GetUserMethods(type);\n\n"
                    << tmp_registration
                ).generate(sink,  attributes::unused, inative_cxt))
             return false;

         if(!as_generator(
             indent << scope_tab << scope_tab << "if (includeInherited)\n"
             << indent << scope_tab(2) << "{\n"
             << indent << scope_tab(3) << "var all_interfaces = type.GetInterfaces();\n"
             << indent << scope_tab(3) << "foreach (var iface in all_interfaces)\n"
             << indent << scope_tab(3) << "{\n"
             << indent << scope_tab(4) <<  "var moredescs = ((Efl.Eo.NativeClass)iface.GetCustomAttributes(false)?.FirstOrDefault(attr => attr is Efl.Eo.NativeClass))?.GetEoOps(type, false);\n"
             << indent << scope_tab(4) <<  "if (moredescs != null)\n"
             << indent << scope_tab(5) <<  "descs.AddRange(moredescs);\n"
             << indent << scope_tab(3) << "}\n"
             << indent << scope_tab(2) << "}\n"
           ).generate(sink, attributes::unused, inative_cxt))
             return false;

         if (!root || !is_concrete)
           if(!as_generator(indent << scope_tab << scope_tab << "descs.AddRange(base.GetEoOps(type, false));\n").generate(sink, attributes::unused, inative_cxt))
             return false;

         if(!as_generator(
                indent << scope_tab << scope_tab << "return descs;\n"
                << indent << scope_tab << "}\n\n"
            ).generate(sink, attributes::unused, inative_cxt))
           return false;

         if (!klass_since.empty())
             klass_since = static_cast<std::string>(scope_tab(2)) + klass_since;

         // Attribute getter of the native 'Efl_Class *' handle (for proper inheritance from additional explicit interfaces)
         if(!as_generator(
              indent << scope_tab << "/// <summary>Returns the Eo class for the native methods of this class.\n"
              << klass_since
              << indent << scope_tab << "/// </summary>\n"
              << indent << scope_tab << "/// <returns>The native class pointer.</returns>\n"
              << indent << scope_tab << "internal override IntPtr GetEflClass()\n"
              << indent << scope_tab << "{\n"
              << indent << scope_tab << scope_tab << "return " << name_helpers::klass_get_full_name(cls) << "();\n"
              << indent << scope_tab << "}\n\n"
           ).generate(sink, attributes::unused, inative_cxt))
           return false;

         // Native method definitions
         if(!as_generator(
                indent << scope_tab << "#pragma warning disable CA1707, CS1591, SA1300, SA1600\n\n"
                <<  *(native_function_definition(cls, implementable_properties))
                << indent << scope_tab << "#pragma warning restore CA1707, CS1591, SA1300, SA1600\n\n")
            .generate(sink, implementable_methods, change_indentation(indent.inc(), inative_cxt))) return false;

         if(!as_generator(
                indent << scope_tab << "#pragma warning disable CA1707, CS1591, SA1300, SA1600\n\n"
                <<  *(native_property_function_definition(cls, cls))
                << indent << scope_tab << "#pragma warning restore CA1707, CS1591, SA1300, SA1600\n\n")
            .generate(sink, implementable_properties
                      , change_indentation(indent.inc(), inative_cxt))) return false;

         if(!as_generator(indent << "}\n").generate(sink, attributes::unused, inative_cxt)) return false;
       }
     return true;
   }

   template <typename OutputIterator, typename Context>
   bool generate_fields(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::string visibility = is_inherit_context(context) ? "protected " : "private ";

     std::string class_getter = name_helpers::klass_get_name(cls) + "()";
     std::string native_inherit_full_name = name_helpers::klass_full_native_inherit_name(cls);
     auto inherit_name = name_helpers::klass_concrete_name(cls);

     if(!as_generator(
                scope_tab(2) << "/// <summary>Pointer to the native class description.</summary>\n"
                << scope_tab(2) << "public override System.IntPtr NativeClass\n"
                << scope_tab(2) << "{\n"
                << scope_tab(2) << scope_tab << "get\n"
                << scope_tab(2) << scope_tab << "{\n"
                << scope_tab(2) << scope_tab << scope_tab << "if (((object)this).GetType() == typeof(" << inherit_name << "))\n"
                << scope_tab(2) << scope_tab << scope_tab << "{\n"
                << scope_tab(2) << scope_tab << scope_tab << scope_tab << "return GetEflClassStatic();\n"
                << scope_tab(2) << scope_tab << scope_tab << "}\n"
                << scope_tab(2) << scope_tab << scope_tab << "else\n"
                << scope_tab(2) << scope_tab << scope_tab << "{\n"
                << scope_tab(2) << scope_tab << scope_tab << scope_tab << "return Efl.Eo.ClassRegister.klassFromType[((object)this).GetType()];\n"
                << scope_tab(2) << scope_tab << scope_tab << "}\n"
                << scope_tab(2) << scope_tab << "}\n"
                << scope_tab(2) << "}\n\n"
            ).generate(sink, attributes::unused, context))
         return false;

     return true;
   }

   template <typename OutputIterator, typename Context>
   bool generate_constructors(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     auto inherit_name = name_helpers::klass_concrete_name(cls);

     if(!as_generator(
             scope_tab(2) << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(context).actual_library_name(cls.filename)
             << ")] internal static extern System.IntPtr\n"
             << scope_tab(2) << scope_tab << name_helpers::klass_get_name(cls) << "();\n\n"
            ).generate(sink, attributes::unused, context))
       return false;

     auto all_constructors = helpers::reorder_constructors(cls.get_all_constructors());
     decltype (all_constructors) constructors;

     std::copy_if(all_constructors.cbegin(), all_constructors.cend(), std::back_inserter(constructors), [&context](attributes::constructor_def const& ctor) {
        return !blacklist::is_function_blacklisted(ctor.function, context);
     });

     std::string klass_since;
     if (!documentation_helpers::generate_since_tag_line(std::back_inserter(klass_since), cls.documentation, scope_tab(2), context))
       return false;

     // Public (API) constructors
     if (!as_generator(
                     scope_tab(2) << "/// <summary>Initializes a new instance of the <see cref=\"" << inherit_name << "\"/> class.\n"
                     << klass_since
                     << scope_tab(2) << "/// </summary>\n"
                     << scope_tab(2) << "/// <param name=\"parent\">Parent instance.</param>\n"
                     << *(documentation(1))
                     // For constructors with arguments, the parent is also required, as optional parameters can't come before non-optional paramenters.
                     << scope_tab(2) << "public " << inherit_name << "(Efl.Object parent" << ((constructors.size() > 0) ? "" : "= null")
                     << *(", " << constructor_param ) << ") : "
                     << "base(" << name_helpers::klass_get_name(cls) <<  "(), parent)\n"
                     << scope_tab(2) << "{\n"
                     << (*(scope_tab << scope_tab << constructor_invocation << "\n"))
                     << scope_tab(2) << scope_tab << "FinishInstantiation();\n"
                     << scope_tab(2) << "}\n\n"
                     << scope_tab(2) << "/// <summary>Subclasses should override this constructor if they are expected to be instantiated from native code.\n"
                     << scope_tab(2) << "/// Do not call this constructor directly.\n"
                     << klass_since
                     << scope_tab(2) << "/// </summary>\n"
                     << scope_tab(2) << "/// <param name=\"ch\">Tag struct storing the native handle of the object being constructed.</param>\n"
                     << scope_tab(2) << "protected " << inherit_name << "(ConstructingHandle ch) : base(ch)\n"
                     << scope_tab(2) << "{\n"
                     << scope_tab(2) << "}\n\n"
                     << scope_tab(2) << "/// <summary>Initializes a new instance of the <see cref=\"" << inherit_name << "\"/> class.\n"
                     << scope_tab(2) << "/// Internal usage: Constructs an instance from a native pointer. This is used when interacting with C code and should not be used directly.\n"
                     << klass_since
                     << scope_tab(2) << "/// </summary>\n"
                     << scope_tab(2) << "/// <param name=\"wh\">The native pointer to be wrapped.</param>\n"
                     << scope_tab(2) << "internal " << inherit_name << "(Efl.Eo.WrappingHandle wh) : base(wh)\n"
                     << scope_tab(2) << "{\n"
                     << scope_tab(2) << "}\n\n"
                 ).generate(sink, std::make_tuple(constructors, constructors, constructors), context))
         return false;

     // Some abstract classes (like Efl.App) have a simple regular class that is used to instantiate them
     // in a controlled manner. These fake-private classes can be returned from C and we use a similarly-named
     // private class to be able to instantiate them when they get to the C# world.
     if (cls.type == attributes::class_type::abstract_)
     {
         if (!as_generator(
                scope_tab(2) << "[Efl.Eo.PrivateNativeClass]\n"
                << scope_tab(2) << "private class " << inherit_name << "Realized : " << inherit_name << "\n"
                << scope_tab(2) << "{\n"
                << scope_tab(2) << scope_tab << "private " << inherit_name << "Realized(Efl.Eo.WrappingHandle wh) : base(wh)\n"
                << scope_tab(2) << scope_tab << "{\n"
                << scope_tab(2) << scope_tab << "}\n"
                << scope_tab(2) << "}\n"
            ).generate(sink, attributes::unused, context))
           return false;
     }

     return as_generator(
                 scope_tab(2) << "/// <summary>Initializes a new instance of the <see cref=\"" << inherit_name << "\"/> class.\n"
                 << scope_tab(2) << "/// Internal usage: Constructor to forward the wrapper initialization to the root class that interfaces with native code. Should not be used directly.\n"
                 << klass_since
                 << scope_tab(2) << "/// </summary>\n"
                 << scope_tab(2) << "/// <param name=\"baseKlass\">The pointer to the base native Eo class.</param>\n"
                 << scope_tab(2) << "/// <param name=\"parent\">The Efl.Object parent of this instance.</param>\n"
                 << scope_tab(2) << "protected " << inherit_name << "(IntPtr baseKlass, Efl.Object parent) : base(baseKlass, parent)\n"
                 << scope_tab(2) << "{\n"
                 << scope_tab(2) << "}\n\n"
              ).generate(sink, attributes::unused, context);
   }

   template <typename OutputIterator, typename Context>
   bool generate_events(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {

     if (!has_events(cls))
         return true;

     // Self events
     if (!as_generator(*(event_definition(cls, cls)) << "\n").generate(sink, cls.events, context))
       return false;

     // Inherited events

     // For now, as mixins can inherit from regular classes, we can't filter out inherited events.
     auto inherits = helpers::non_implemented_interfaces(cls, context);
     for (auto&& c : inherits)
       {
          attributes::klass_def klass(get_klass(c, cls.unit), cls.unit);
          if (!as_generator(*(event_definition(klass, cls))).generate(sink, klass.events, context))
            return false;
       }
     return true;
   }

  bool is_partial;

  klass const operator()(partial_class) const
  {
    return klass{true};
  }
};

struct klass const klass = {false};

}

namespace efl { namespace eolian { namespace grammar {
  
template <>
struct is_eager_generator<struct ::eolian_mono::klass> : std::true_type {};
template <>
struct is_generator<struct ::eolian_mono::klass> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<struct ::eolian_mono::klass> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
