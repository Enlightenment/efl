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

template <typename OutputIterator, typename Context>
static bool generate_equals_method(OutputIterator sink, Context const &context)
{
   return as_generator(
       scope_tab << "/// <summary>Verifies if the given object is equal to this one.</summary>\n"
       << scope_tab << "/// <param name=\"instance\">The object to compare to.</param>\n"
       << scope_tab << "/// <returns>True if both objects point to the same native object.</returns>\n"
       << scope_tab << "public override bool Equals(object instance)\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "var other = instance as Efl.Object;\n"
       << scope_tab << scope_tab << "if (other == null)\n"
       << scope_tab << scope_tab << "{\n"
       << scope_tab << scope_tab << scope_tab << "return false;\n"
       << scope_tab << scope_tab << "}\n"
       << scope_tab << scope_tab << "return this.NativeHandle == other.NativeHandle;\n"
       << scope_tab << "}\n\n"
       << scope_tab << "/// <summary>Gets the hash code for this object based on the native pointer it points to.</summary>\n"
       << scope_tab << "/// <returns>The value of the pointer, to be used as the hash code of this object.</returns>\n"
       << scope_tab << "public override int GetHashCode()\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "return this.NativeHandle.ToInt32();\n"
       << scope_tab << "}\n\n"
       << scope_tab << "/// <summary>Turns the native pointer into a string representation.</summary>\n"
       << scope_tab << "/// <returns>A string with the type and the native pointer for this object.</returns>\n"
       << scope_tab << "public override String ToString()\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "return $\"{this.GetType().Name}@[{this.NativeHandle.ToInt32():x}]\";\n"
       << scope_tab << "}\n\n"
      ).generate(sink, nullptr, context);
}

/* Get the actual number of functions of a class, checking for blacklisted ones */
template<typename Context>
static std::size_t
get_implementable_function_count(grammar::attributes::klass_def const& cls, Context context)
{
   auto methods = helpers::get_all_implementable_methods(cls);
   return std::count_if(methods.cbegin(), methods.cend(), [&context](grammar::attributes::function_def const& func)
     {
        return !blacklist::is_function_blacklisted(func, context) && !func.is_static;
     });
}

template<typename Context>
static bool
is_inherit_context(Context const& context)
{
   return context_find_tag<class_context>(context).current_wrapper_kind == class_context::inherit;
}

struct klass
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "klass_generator: " << cls.eolian_name << std::endl;

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

       if(!as_generator(documentation).generate(sink, cls, iface_cxt))
         return false;

       // Mark the interface with the proper native Efl_Class* getter
       if(!as_generator(lit("[") << name_helpers::klass_native_inherit_name(cls) << "]\n")
          .generate(sink, attributes::unused, iface_cxt))
         return false;

       if(!as_generator
        (
         "public " /*<< class_type*/ "interface" /*<<*/ " " << string << " : "
         )
        .generate(sink, name_helpers::klass_interface_name(cls), iface_cxt))
         return false;
       for(auto first = std::begin(cls.immediate_inherits)
             , last = std::end(cls.immediate_inherits); first != last; ++first)
         {
            if(first->type != attributes::class_type::regular && first->type != attributes::class_type::abstract_)
              if(!as_generator("\n" << scope_tab << string << " ,").generate(sink, name_helpers::klass_full_interface_name(*first), iface_cxt))
                return false;
         }

       if(!as_generator("\n" << scope_tab << "Efl.Eo.IWrapper, IDisposable").generate(sink, attributes::unused, iface_cxt))
         return false;

       if(!as_generator("\n{\n").generate(sink, attributes::unused, iface_cxt))
         return false;

       if(!as_generator(*(scope_tab << function_declaration)).generate(sink, cls.functions, iface_cxt))
         return false;

       if(!as_generator(*(scope_tab << async_function_declaration)).generate(sink, cls.functions, iface_cxt))
         return false;

       if(!as_generator(*(event_declaration)).generate(sink, cls.events, iface_cxt))
         return false;

       for (auto &&p : cls.parts)
         if (!as_generator(
              documentation(1)
              << name_helpers::klass_full_concrete_or_interface_name(p.klass) << " " << utils::capitalize(p.name) << "{ get;}\n"
            ).generate(sink, p, iface_cxt))
           return false;

       if (!as_generator(*(property_wrapper_definition(cls))).generate(sink, cls.properties, iface_cxt))
         return false;

       // End of interface declaration
       if(!as_generator("}\n").generate(sink, attributes::unused, iface_cxt)) return false;
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
         auto concrete_name = name_helpers::klass_concrete_name(cls);
         auto interface_name = name_helpers::klass_interface_name(cls);

         if(!as_generator
            (
             documentation
             << "sealed internal class " << concrete_name << " : " << "\n"
             << (klass_full_concrete_or_interface_name % ",") << "\n"
             << (inherit_classes.size() > 0 ? ", " : "" ) << interface_name << "\n"
             << scope_tab << *(", " << name_helpers::klass_full_concrete_or_interface_name) << "\n"
             << "{\n"
            ).generate(sink, std::make_tuple(cls, inherit_classes, inherit_interfaces), concrete_cxt))
              return false;

         if (!generate_fields(sink, cls, concrete_cxt))
           return false;

         bool root = !helpers::has_regular_ancestor(cls);
         if (!as_generator
            (
             scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(concrete_cxt).actual_library_name(cls.filename)
             << ")] internal static extern System.IntPtr\n"
             << scope_tab << scope_tab << name_helpers::klass_get_name(cls) << "();\n"
             << scope_tab << "/// <summary>Initializes a new instance of the <see cref=\"" << interface_name << "\"/> class.\n"
             << scope_tab << "/// Internal usage: This is used when interacting with C code and should not be used directly.</summary>\n"
             << scope_tab << "private " << concrete_name << "(System.IntPtr raw)" << (root ? "" : " : base(raw)") << "\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << (root ? "handle = raw;\n" : "")
             << scope_tab << "}\n"
            )
            .generate(sink, attributes::unused, concrete_cxt))
           return false;

         if (!generate_dispose_methods(sink, cls, concrete_cxt))
           return false;

         if (!generate_equals_method(sink, concrete_cxt))
           return false;

         if (!generate_events(sink, cls, concrete_cxt))
             return false;

         // Parts
         if(!as_generator(*(part_definition))
            .generate(sink, cls.parts, concrete_cxt)) return false;

         // Concrete function definitions
         auto implemented_methods = helpers::get_all_implementable_methods(cls);
         if(!as_generator(*(function_definition))
            .generate(sink, implemented_methods, concrete_cxt)) return false;

         // Async wrappers
         if(!as_generator(*(async_function_definition)).generate(sink, implemented_methods, concrete_cxt))
           return false;

         // Property wrappers
         if (!as_generator(*(property_wrapper_definition(cls))).generate(sink, cls.properties, concrete_cxt))
           return false;

         for (auto&& klass : helpers::non_implemented_interfaces(cls, concrete_cxt))
           {
              attributes::klass_def c(get_klass(klass, cls.unit), cls.unit);
              if (!as_generator(*(property_wrapper_definition(cls))).generate(sink, c.properties, concrete_cxt))
                return false;
           }

         // Copied from nativeinherit class, used when setting up providers.
         if(!as_generator(
              scope_tab << "private static IntPtr GetEflClassStatic()\n"
              << scope_tab << "{\n"
              << scope_tab << scope_tab << "return " << name_helpers::klass_get_full_name(cls) << "();\n"
              << scope_tab << "}\n"
           ).generate(sink, attributes::unused, concrete_cxt))
           return false;


         if(!as_generator("}\n").generate(sink, attributes::unused, concrete_cxt)) return false;
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
             documentation
             << "[" << name_helpers::klass_native_inherit_name(cls) << "]\n"
             << "public " << class_type << " " << name_helpers::klass_concrete_name(cls) << " : "
             << (klass_full_concrete_or_interface_name % ",") // classes
             << (inherit_classes.empty() ? "" : ",")
             << " Efl.Eo.IWrapper" << (root ? ", IDisposable" : "")
             << (inherit_interfaces.empty() ? "" : ",")
             << (klass_full_concrete_or_interface_name % ",") // interfaces
             << "\n{\n"
             )
           .generate(sink, std::make_tuple(cls, inherit_classes, inherit_interfaces), inherit_cxt))
           return false;


         // Class body
         if(!generate_fields(sink, cls, inherit_cxt))
           return false;

         if (!generate_constructors(sink, cls, inherit_cxt))
           return false;

         if (!generate_dispose_methods(sink, cls, inherit_cxt))
           return false;

         if (!generate_equals_method(sink, inherit_cxt))
           return false;

         if (!generate_events(sink, cls, inherit_cxt))
             return false;

         // Parts
         if(!as_generator(*(part_definition))
            .generate(sink, cls.parts, inherit_cxt)) return false;

         // Inherit function definitions
         auto implemented_methods = helpers::get_all_implementable_methods(cls);
         if(!as_generator(*(function_definition(true)))
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
              scope_tab << "private static IntPtr GetEflClassStatic()\n"
              << scope_tab << "{\n"
              << scope_tab << scope_tab << "return " << name_helpers::klass_get_full_name(cls) << "();\n"
              << scope_tab << "}\n"
           ).generate(sink, attributes::unused, inherit_cxt))
           return false;

         if(!as_generator("}\n").generate(sink, attributes::unused, inherit_cxt)) return false;
       }

     // Native Inherit class
     //if(class_type == "class")
       {
         auto inative_cxt = context_add_tag(class_context{class_context::inherit_native,
                                            name_helpers::klass_full_concrete_or_interface_name(cls)},
                                            context);
         auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);
         auto inherit_name = name_helpers::klass_inherit_name(cls);
         auto implementable_methods = helpers::get_all_implementable_methods(cls);
         std::string base_name;
         if(!root)
           {
              attributes::klass_def parent_klass(get_klass(*cls.parent, cls.unit), cls.unit);
              base_name = name_helpers::klass_full_native_inherit_name(parent_klass);
           }

         if(!as_generator
            (
             "public class " << native_inherit_name << " " << (root ? " : Efl.Eo.NativeClass" : (": " + base_name)) <<"{\n"
             << scope_tab << "public " << (root ? "" : "new ") << " static Efl.Eo.NativeModule _Module = new Efl.Eo.NativeModule("
             << context_find_tag<library_context>(context).actual_library_name(cls.filename) << ");\n"
             << scope_tab << "public override System.Collections.Generic.List<Efl_Op_Description> GetEoOps(System.Type type)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "var descs = new System.Collections.Generic.List<Efl_Op_Description>();\n"
            )
            .generate(sink, attributes::unused, inative_cxt))
           return false;

         // Native wrapper registration
         // We write them first to a temporary function as the implementable function list may contain
         // only non-registrable methods like class functions, leading to unused `methods` variable.
         std::string tmp_registration;
         if(!as_generator(*(function_registration(cls)))
            .generate(std::back_inserter(tmp_registration), implementable_methods, inative_cxt))
           return false;

         if (tmp_registration.find("methods") != std::string::npos)
           if (!as_generator(
                    scope_tab << scope_tab << "var methods = Efl.Eo.Globals.GetUserMethods(type);\n"
                    << tmp_registration
                ).generate(sink,  attributes::unused, inative_cxt))
             return false;

         if(!root)
           if(!as_generator(scope_tab << scope_tab << "descs.AddRange(base.GetEoOps(type));\n").generate(sink, attributes::unused, inative_cxt))
             return false;

         if(!as_generator(
                scope_tab << scope_tab << "return descs;\n"
                << scope_tab << "}\n"
            ).generate(sink, attributes::unused, inative_cxt))
           return false;

         // Attribute getter of the native 'Efl_Class *' handle (for proper inheritance from additional explicit interfaces)
         if(!as_generator(
                 scope_tab << "public override IntPtr GetEflClass()\n"
              << scope_tab << "{\n"
              << scope_tab << scope_tab << "return " << name_helpers::klass_get_full_name(cls) << "();\n"
              << scope_tab << "}\n\n"
           ).generate(sink, attributes::unused, inative_cxt))
           return false;

         if(!as_generator(
              scope_tab << "public static " << (root ? "" : "new ") << " IntPtr GetEflClassStatic()\n"
              << scope_tab << "{\n"
              << scope_tab << scope_tab << "return " << name_helpers::klass_get_full_name(cls) << "();\n"
              << scope_tab << "}\n\n"
           ).generate(sink, attributes::unused, inative_cxt))
           return false;

         // Native method definitions
         if(!as_generator(*(native_function_definition(cls)))
            .generate(sink, implementable_methods, inative_cxt)) return false;

         if(!as_generator("}\n").generate(sink, attributes::unused, inative_cxt)) return false;
       }

     if(!name_helpers::close_namespaces(sink, cls.namespaces, context))
       return false;

     return true;
   }

   template <typename OutputIterator, typename Context>
   bool generate_fields(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::string visibility = is_inherit_context(context) ? "protected " : "private ";
     bool root = !helpers::has_regular_ancestor(cls);
     bool is_inherit = is_inherit_context(context);

     std::string class_getter = name_helpers::klass_get_name(cls) + "()";
     std::string native_inherit_full_name = name_helpers::klass_full_native_inherit_name(cls);
     auto inherit_name = name_helpers::klass_concrete_name(cls);

     std::string raw_klass_modifier;
     if (!root)
       raw_klass_modifier = "override ";
     else if (is_inherit)
       raw_klass_modifier = "virtual ";

     if(!as_generator(
                scope_tab << "///<summary>Pointer to the native class description.</summary>\n"
                << scope_tab << "public " << raw_klass_modifier << "System.IntPtr NativeClass\n"
                << scope_tab << "{\n"
                << scope_tab << scope_tab << "get\n"
                << scope_tab << scope_tab << "{\n"
                << scope_tab << scope_tab << scope_tab << "if (((object)this).GetType() == typeof(" << inherit_name << "))\n"
                << scope_tab << scope_tab << scope_tab << "{\n"
                << scope_tab << scope_tab << scope_tab << scope_tab << "return " << native_inherit_full_name << ".GetEflClassStatic();\n"
                << scope_tab << scope_tab << scope_tab << "}\n"
                << scope_tab << scope_tab << scope_tab << "else\n"
                << scope_tab << scope_tab << scope_tab << "{\n"
                << scope_tab << scope_tab << scope_tab << scope_tab << "return Efl.Eo.ClassRegister.klassFromType[((object)this).GetType()];\n"
                << scope_tab << scope_tab << scope_tab << "}\n"
                << scope_tab << scope_tab << "}\n"
                << scope_tab << "}\n\n"
            ).generate(sink, attributes::unused, context))
         return false;

     // The remaining fields aren't needed in children classes.
     if (!root)
       return true;

     if (cls.get_all_events().size() > 0)
        if (!as_generator(scope_tab << visibility << "Dictionary<(IntPtr desc, object evtDelegate), (IntPtr evtCallerPtr, Efl.EventCb evtCaller)> eoEvents = new Dictionary<(IntPtr desc, object evtDelegate), (IntPtr evtCallerPtr, Efl.EventCb evtCaller)>();\n"
                          << scope_tab << visibility << "readonly object eventLock = new object();\n")
              .generate(sink, attributes::unused, context))
          return false;

     if (is_inherit)
      {
         if (!as_generator(
                scope_tab << "protected bool inherited;\n"
                ).generate(sink, attributes::unused, context))
           return false;
      }

     return as_generator(
                scope_tab << visibility << " System.IntPtr handle;\n"
                << scope_tab << "///<summary>Pointer to the native instance.</summary>\n"
                << scope_tab << "public System.IntPtr NativeHandle\n"
                << scope_tab << "{\n"
                << scope_tab << scope_tab << "get { return handle; }\n"
                << scope_tab << "}\n\n"
             ).generate(sink, attributes::unused, context);
   }

   template <typename OutputIterator, typename Context>
   bool generate_constructors(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     bool root = !helpers::has_regular_ancestor(cls);
     auto inherit_name = name_helpers::klass_concrete_name(cls);
     auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);

     if(!as_generator(
             scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(context).actual_library_name(cls.filename)
             << ")] internal static extern System.IntPtr\n"
             << scope_tab << scope_tab << name_helpers::klass_get_name(cls) << "();\n"
            ).generate(sink, attributes::unused, context))
       return false;

     auto all_constructors = helpers::reorder_constructors(cls.get_all_constructors());
     decltype (all_constructors) constructors;

     std::copy_if(all_constructors.cbegin(), all_constructors.cend(), std::back_inserter(constructors), [&context](attributes::constructor_def const& ctor) {
        return !blacklist::is_function_blacklisted(ctor.function, context);
     });

     // Public (API) constructors
     if (!as_generator(
                     scope_tab << "/// <summary>Initializes a new instance of the <see cref=\"" << inherit_name << "\"/> class.</summary>\n"
                     << scope_tab << "/// <param name=\"parent\">Parent instance.</param>\n"
                     << *(documentation)
                     // For constructors with arguments, the parent is also required, as optional parameters can't come before non-optional paramenters.
                     << scope_tab << "public " << inherit_name << "(Efl.Object parent" << ((constructors.size() > 0) ? "" : "= null") << "\n"
                     << scope_tab << scope_tab << scope_tab << *(", " << constructor_param ) << ") : "
                             << (root ? "this" : "base")  << "(" << name_helpers::klass_get_name(cls) <<  "(), typeof(" << inherit_name << "), parent)\n"
                     << scope_tab << "{\n"
                     << (*(scope_tab << scope_tab << constructor_invocation << "\n"))
                     << scope_tab << scope_tab << "FinishInstantiation();\n"
                     << scope_tab << "}\n\n"
                     << scope_tab << "/// <summary>Initializes a new instance of the <see cref=\"" << inherit_name << "\"/> class.\n"
                     << scope_tab << "/// Internal usage: Constructs an instance from a native pointer. This is used when interacting with C code and should not be used directly.</summary>\n"
                     << scope_tab << "/// <param name=\"raw\">The native pointer to be wrapped.</param>\n"
                     << scope_tab << "protected " << inherit_name << "(System.IntPtr raw)" << (root ? "" : " : base(raw)") << "\n"
                     << scope_tab << "{\n"
                     << scope_tab << scope_tab << (root ? "handle = raw;\n" : "")
                     << scope_tab << "}\n\n"
                 ).generate(sink, std::make_tuple(constructors, constructors, constructors), context))
         return false;

     // Some abstract classes (like Efl.App) have a simple regular class that is used to instantiate them
     // in a controlled manner. These fake-private classes can be returned from C and we use a similarly-named
     // private class to be able to instantiate them when they get to the C# world.
     if (cls.type == attributes::class_type::abstract_)
     {
         if (!as_generator(
                scope_tab << "[Efl.Eo.PrivateNativeClass]\n"
                << scope_tab << "private class " << inherit_name << "Realized : " << inherit_name << "\n"
                << scope_tab << "{\n"
                << scope_tab << scope_tab << "private " << inherit_name << "Realized(IntPtr ptr) : base(ptr)\n"
                << scope_tab << scope_tab << "{\n"
                << scope_tab << scope_tab << "}\n"
                << scope_tab << "}\n"
            ).generate(sink, attributes::unused, context))
           return false;
     }

     // Internal constructors
     if (!root)
     {
         return as_generator(
                     scope_tab << "/// <summary>Initializes a new instance of the <see cref=\"" << inherit_name << "\"/> class.\n"
                     << scope_tab << "/// Internal usage: Constructor to forward the wrapper initialization to the root class that interfaces with native code. Should not be used directly.</summary>\n"
                     << scope_tab << "/// <param name=\"baseKlass\">The pointer to the base native Eo class.</param>\n"
                     << scope_tab << "/// <param name=\"managedType\">The managed type of the public constructor that originated this call.</param>\n"
                     << scope_tab << "/// <param name=\"parent\">The Efl.Object parent of this instance.</param>\n"
                     << scope_tab << "protected " << inherit_name << "(IntPtr baseKlass, System.Type managedType, Efl.Object parent) : base(baseKlass, managedType, parent)\n"
                     << scope_tab << "{\n"
                     << scope_tab << "}\n\n"
                  ).generate(sink, attributes::unused, context);

     }

     // Detailed constructors go only in root classes.
     return as_generator(
             /// Actual root costructor that creates class and instantiates 
             scope_tab << "/// <summary>Initializes a new instance of the <see cref=\"" << inherit_name << "\"/> class.\n"
             << scope_tab << "/// Internal usage: Constructor to actually call the native library constructors. C# subclasses\n"
             << scope_tab << "/// must use the public constructor only.</summary>\n"
             << scope_tab << "/// <param name=\"baseKlass\">The pointer to the base native Eo class.</param>\n"
             << scope_tab << "/// <param name=\"managedType\">The managed type of the public constructor that originated this call.</param>\n"
             << scope_tab << "/// <param name=\"parent\">The Efl.Object parent of this instance.</param>\n"
             << scope_tab << "protected " << inherit_name << "(IntPtr baseKlass, System.Type managedType, Efl.Object parent)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "inherited = ((object)this).GetType() != managedType;\n"
             << scope_tab << scope_tab << "IntPtr actual_klass = baseKlass;\n"
             << scope_tab << scope_tab << "if (inherited)\n"
             << scope_tab << scope_tab << "{\n"
             << scope_tab << scope_tab << scope_tab << "actual_klass = Efl.Eo.ClassRegister.GetInheritKlassOrRegister(baseKlass, ((object)this).GetType());\n"
             << scope_tab << scope_tab << "}\n\n"
             << scope_tab << scope_tab << "handle = Efl.Eo.Globals.instantiate_start(actual_klass, parent);\n"
             << scope_tab << scope_tab << "if (inherited)\n"
             << scope_tab << scope_tab << "{\n"
             << scope_tab << scope_tab << scope_tab << "Efl.Eo.Globals.PrivateDataSet(this);\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << "}\n\n"

             << scope_tab << "/// <summary>Finishes instantiating this object.\n"
             << scope_tab << "/// Internal usage by generated code.</summary>\n"
             << scope_tab << "protected void FinishInstantiation()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "handle = Efl.Eo.Globals.instantiate_end(handle);\n"
             << scope_tab << scope_tab << "Eina.Error.RaiseIfUnhandledException();\n"
             << scope_tab << "}\n\n"

             ).generate(sink, attributes::unused, context);
   }


   template <typename OutputIterator, typename Context>
   bool generate_dispose_methods(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     if (helpers::has_regular_ancestor(cls))
       return true;

     std::string visibility = is_inherit_context(context) ? "protected virtual " : "private ";

     auto inherit_name = name_helpers::klass_concrete_name(cls);

     std::string events_gchandle;
     if (cls.get_all_events().size() > 0)
       {
           auto events_gchandle_sink = std::back_inserter(events_gchandle);
           if (!as_generator(scope_tab << scope_tab << scope_tab << "if (eoEvents.Count != 0)\n"
                             << scope_tab << scope_tab << scope_tab << "{\n"
                             << scope_tab << scope_tab << scope_tab << scope_tab << "GCHandle gcHandle = GCHandle.Alloc(eoEvents);\n"
                             << scope_tab << scope_tab << scope_tab << scope_tab << "gcHandlePtr = GCHandle.ToIntPtr(gcHandle);\n"
                             << scope_tab << scope_tab << scope_tab << "}\n\n")
                 .generate(events_gchandle_sink, attributes::unused, context))
             return false;
       }

     return as_generator(

             scope_tab << "///<summary>Destructor.</summary>\n"
             << scope_tab << "~" << inherit_name << "()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(false);\n"
             << scope_tab << "}\n\n"

             << scope_tab << "///<summary>Releases the underlying native instance.</summary>\n"
             << scope_tab << visibility << "void Dispose(bool disposing)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (handle != System.IntPtr.Zero)\n"
             << scope_tab << scope_tab << "{\n"
             << scope_tab << scope_tab << scope_tab << "IntPtr h = handle;\n"
             << scope_tab << scope_tab << scope_tab << "handle = IntPtr.Zero;\n\n"

             << scope_tab << scope_tab << scope_tab << "IntPtr gcHandlePtr = IntPtr.Zero;\n"
             << events_gchandle

             << scope_tab << scope_tab << scope_tab << "if (disposing)\n"
             << scope_tab << scope_tab << scope_tab << "{\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "Efl.Eo.Globals.efl_mono_native_dispose(h, gcHandlePtr);\n"
             << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << "else\n"
             << scope_tab << scope_tab << scope_tab << "{\n"

             << scope_tab << scope_tab << scope_tab << scope_tab << "Monitor.Enter(Efl.All.InitLock);\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "if (Efl.All.MainLoopInitialized)\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "{\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "Efl.Eo.Globals.efl_mono_thread_safe_native_dispose(h, gcHandlePtr);\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "}\n\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "Monitor.Exit(Efl.All.InitLock);\n"
             << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "}\n\n"
             << scope_tab << "}\n\n"

             << scope_tab << "///<summary>Releases the underlying native instance.</summary>\n"
             << scope_tab << "public void Dispose()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(true);\n"
             << scope_tab << scope_tab << "GC.SuppressFinalize(this);\n"
             << scope_tab << "}\n\n"
             ).generate(sink, attributes::unused, context);
   }

   template <typename OutputIterator, typename Context>
   bool generate_events(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {

     if (!has_events(cls))
         return true;

     std::string visibility = is_inherit_context(context) ? "protected " : "private ";

     if (!helpers::has_regular_ancestor(cls))
       {
     // Callback registration functions
     if (!as_generator(
            scope_tab << "///<summary>Adds a new event handler, registering it to the native event. For internal use only.</summary>\n"
            << scope_tab << "///<param name=\"lib\">The name of the native library definining the event.</param>\n"
            << scope_tab << "///<param name=\"key\">The name of the native event.</param>\n"
            << scope_tab << "///<param name=\"evtCaller\">Delegate to be called by native code on event raising.</param>\n"
            << scope_tab << "///<param name=\"evtDelegate\">Managed delegate that will be called by evtCaller on event raising.</param>\n"
            << scope_tab << visibility << "void AddNativeEventHandler(string lib, string key, Efl.EventCb evtCaller, object evtDelegate)\n"
            << scope_tab << "{\n"

            << scope_tab << scope_tab << "IntPtr desc = Efl.EventDescription.GetNative(lib, key);\n"
            << scope_tab << scope_tab << "if (desc == IntPtr.Zero)\n"
            << scope_tab << scope_tab << "{\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to get native event {key}\");\n"
            << scope_tab << scope_tab << "}\n\n"

            << scope_tab << scope_tab << "if (eoEvents.ContainsKey((desc, evtDelegate)))\n"
            << scope_tab << scope_tab << "{\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Log.Warning($\"Event proxy for event {key} already registered!\");\n"
            << scope_tab << scope_tab << scope_tab << "return;\n"
            << scope_tab << scope_tab << "}\n\n"

            << scope_tab << scope_tab << "IntPtr evtCallerPtr = Marshal.GetFunctionPointerForDelegate(evtCaller);\n"
            << scope_tab << scope_tab << "if (!Efl.Eo.Globals.efl_event_callback_priority_add(handle, desc, 0, evtCallerPtr, IntPtr.Zero))\n"
            << scope_tab << scope_tab << "{\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to add event proxy for event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << "return;\n"
            << scope_tab << scope_tab << "}\n\n"

            << scope_tab << scope_tab << "eoEvents[(desc, evtDelegate)] = (evtCallerPtr, evtCaller);\n"
            << scope_tab << scope_tab << "Eina.Error.RaiseIfUnhandledException();\n"
            << scope_tab << "}\n\n"

            << scope_tab << "///<summary>Removes the given event handler for the given event. For internal use only.</summary>\n"
            << scope_tab << "///<param name=\"lib\">The name of the native library definining the event.</param>\n"
            << scope_tab << "///<param name=\"key\">The name of the native event.</param>\n"
            << scope_tab << "///<param name=\"evtDelegate\">The delegate to be removed.</param>\n"
            << scope_tab << visibility << "void RemoveNativeEventHandler(string lib, string key, object evtDelegate)\n"
            << scope_tab << "{\n"

            << scope_tab << scope_tab << "IntPtr desc = Efl.EventDescription.GetNative(lib, key);\n"
            << scope_tab << scope_tab << "if (desc == IntPtr.Zero)\n"
            << scope_tab << scope_tab << "{\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to get native event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << "return;\n"
            << scope_tab << scope_tab << "}\n\n"

            << scope_tab << scope_tab << "var evtPair = (desc, evtDelegate);\n"
            << scope_tab << scope_tab << "if (eoEvents.TryGetValue(evtPair, out var caller))\n"
            << scope_tab << scope_tab << "{\n"

            << scope_tab << scope_tab << scope_tab << "if (!Efl.Eo.Globals.efl_event_callback_del(handle, desc, caller.evtCallerPtr, IntPtr.Zero))\n"
            << scope_tab << scope_tab << scope_tab << "{\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to remove event proxy for event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return;\n"
            << scope_tab << scope_tab << scope_tab << "}\n\n"

            << scope_tab << scope_tab << scope_tab << "eoEvents.Remove(evtPair);\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Error.RaiseIfUnhandledException();\n"
            << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << "else\n"
            << scope_tab << scope_tab << "{\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Trying to remove proxy for event {key} when it is nothing registered.\");\n"
            << scope_tab << scope_tab << "}\n"
            << scope_tab << "}\n\n"
            )
             .generate(sink, NULL, context))
         return false;
       }

     // Self events
     if (!as_generator(*(event_definition(cls, cls))).generate(sink, cls.events, context))
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
};

struct klass const klass = {};

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
