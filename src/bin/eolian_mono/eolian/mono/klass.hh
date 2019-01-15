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
static bool generate_static_cast_method(OutputIterator sink, grammar::attributes::klass_def const& cls, Context const &context)
{
   return as_generator(
       scope_tab << "///<summary>Casts obj into an instance of this type.</summary>\n"
       << scope_tab << "public " << (helpers::has_regular_ancestor(cls) ? "new " : "") <<"static " << name_helpers::klass_concrete_name(cls) << " static_cast(Efl.Object obj)\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "if (obj == null)\n"
       << scope_tab << scope_tab << scope_tab << "throw new System.ArgumentNullException(\"obj\");\n"
       << scope_tab << scope_tab << "return new " << name_helpers::klass_concrete_name(cls) << "(obj.NativeHandle);\n"
       << scope_tab << "}\n"
       ).generate(sink, nullptr, context);
}

template <typename OutputIterator, typename Context>
static bool generate_equals_method(OutputIterator sink, Context const &context)
{
   return as_generator(
       scope_tab << "///<summary>Verifies if the given object is equal to this one.</summary>\n" 
       << scope_tab << "public override bool Equals(object obj)\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "var other = obj as Efl.Object;\n"
       << scope_tab << scope_tab << "if (other == null)\n"
       << scope_tab << scope_tab << scope_tab << "return false;\n"
       << scope_tab << scope_tab << "return this.NativeHandle == other.NativeHandle;\n"
       << scope_tab << "}\n"
       << scope_tab << "///<summary>Gets the hash code for this object based on the native pointer it points to.</summary>\n"
       << scope_tab << "public override int GetHashCode()\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "return this.NativeHandle.ToInt32();\n"
       << scope_tab << "}\n"
       << scope_tab << "///<summary>Turns the native pointer into a string representation.</summary>\n"
       << scope_tab << "public override String ToString()\n"
       << scope_tab << "{\n"
       << scope_tab << scope_tab << "return $\"{this.GetType().Name}@[{this.NativeHandle.ToInt32():x}]\";\n"
       << scope_tab << "}\n"
      ).generate(sink, nullptr, context);
}

/* Get the actual number of functions of a class, checking for blacklisted ones */
static std::size_t
get_implementable_function_count(grammar::attributes::klass_def const& cls)
{
   auto methods = helpers::get_all_implementable_methods(cls);
   return std::count_if(methods.cbegin(), methods.cend(), [](grammar::attributes::function_def const& func)
     {
        return !blacklist::is_function_blacklisted(func.c_name) && !func.is_static;
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
     std::string suffix, class_type;
     switch(cls.type)
       {
       case attributes::class_type::regular:
         class_type = "class";
         suffix = "CLASS";
         break;
       case attributes::class_type::abstract_:
         class_type = "class";
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
     auto iface_cxt = context_add_tag(class_context{class_context::interface}, context);

     if(!as_generator(documentation).generate(sink, cls, iface_cxt))
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

     if (!as_generator(*(property_wrapper_definition)).generate(sink, cls.properties, iface_cxt))
       return false;

     // End of interface declaration
     if(!as_generator("}\n").generate(sink, attributes::unused, iface_cxt)) return false;

     }

     // Events arguments go in the top namespace to avoid the Concrete suffix clutter in interface events.
     // Regular/abstract class events go here too for consistency.
     if(!as_generator(*(event_argument_wrapper)).generate(sink, cls.events, context))
       return false;

     bool root = !helpers::has_regular_ancestor(cls);
     std::set<attributes::klass_name, attributes::compare_klass_name_by_name> inherit_interfaces = helpers::non_implemented_interfaces(cls);
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
     if(class_type != "class")
       {
         auto concrete_cxt = context_add_tag(class_context{class_context::concrete}, context);
         auto concrete_name = name_helpers::klass_concrete_name(cls);
         auto interface_name = name_helpers::klass_interface_name(cls);

         if(!as_generator
            (
             documentation
             << "sealed public class " << concrete_name << " : " << "\n"
             << (klass_full_concrete_or_interface_name % ",") << "\n"
             << (inherit_classes.size() > 0 ? ", " : "" ) << interface_name << "\n"
             << scope_tab << *(", " << name_helpers::klass_full_concrete_or_interface_name) << "\n"
             << "{\n"
            ).generate(sink, std::make_tuple(cls, inherit_classes, inherit_interfaces), concrete_cxt))
              return false;

         if (!generate_fields(sink, cls, concrete_cxt))
           return false;

         if (!as_generator
            (
             scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(concrete_cxt).actual_library_name(cls.filename)
             << ")] internal static extern System.IntPtr\n"
             << scope_tab << scope_tab << name_helpers::klass_get_name(cls) << "();\n"
             << scope_tab << "///<summary>Constructs an instance from a native pointer.</summary>\n"
             << scope_tab << "public " << concrete_name << "(System.IntPtr raw)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "handle = raw;\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"
            )
            .generate(sink, attributes::unused, concrete_cxt))
           return false;

         if (!generate_dispose_methods(sink, cls, concrete_cxt))
           return false;

         if (!generate_static_cast_method(sink, cls, concrete_cxt))
           return false;

         if (!generate_equals_method(sink, concrete_cxt))
           return false;

         if (!generate_events(sink, cls, concrete_cxt))
             return false;

         if (!generate_events_registration(sink, cls, concrete_cxt))
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
         if (!as_generator(*(property_wrapper_definition)).generate(sink, cls.properties, concrete_cxt))
           return false;

         for (auto&& klass : helpers::non_implemented_interfaces(cls))
           {
              attributes::klass_def c(get_klass(klass, cls.unit), cls.unit);
              if (!as_generator(*(property_wrapper_definition)).generate(sink, c.properties, concrete_cxt))
                return false;
           }


         if(!as_generator("}\n").generate(sink, attributes::unused, concrete_cxt)) return false;

       }

     // Inheritable class
     if(class_type == "class")
       {
        auto inherit_cxt = context_add_tag(class_context{class_context::inherit}, context);

        // Class header
        if(!as_generator
            (
             documentation
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

         if (!generate_static_cast_method(sink, cls, inherit_cxt))
           return false;

         if (!generate_equals_method(sink, inherit_cxt))
           return false;

         if (!generate_events(sink, cls, inherit_cxt))
             return false;

         if (!generate_events_registration(sink, cls, inherit_cxt))
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
         if (!as_generator(*(property_wrapper_definition)).generate(sink, cls.properties, inherit_cxt))
           return false;

         for (auto&& klass : helpers::non_implemented_interfaces(cls))
           {
              attributes::klass_def c(get_klass(klass, cls.unit), cls.unit);
              if (!as_generator(*(property_wrapper_definition)).generate(sink, c.properties, inherit_cxt))
                return false;
           }

         if(!as_generator("}\n").generate(sink, attributes::unused, inherit_cxt)) return false;
       }

     std::size_t function_count = get_implementable_function_count(cls);

     int function_registration_index = 0;
     auto index_generator = [&function_registration_index]
       {
         return function_registration_index++;
       };
     
     // Native Inherit class
     if(class_type == "class")
       {
         auto inative_cxt = context_add_tag(class_context{class_context::inherit_native}, context);
         auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);
         auto inherit_name = name_helpers::klass_inherit_name(cls);
         std::string base_name;
         if(!root)
           {
              attributes::klass_def parent_klass(get_klass(*cls.parent, cls.unit), cls.unit);
              base_name = name_helpers::klass_full_native_inherit_name(parent_klass);
           }

         if(!as_generator
            (
             "public " << class_type << " " << native_inherit_name << " " << (root ? "" : (": " + base_name)) <<"{\n"
             << scope_tab << (root ? "protected IntPtr EoKlass { get; set; }\n" : "\n")
             << scope_tab << "public " << (root ? "" : "new ") << "Efl_Op_Description[] GetEoOps()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Efl_Op_Description[] descs = new Efl_Op_Description[" << grammar::int_ << "];\n"
            )
            .generate(sink, function_count, inative_cxt))
           return false;

         // Native wrapper registration
         if(!as_generator(*(function_registration(index_generator, cls)))
            .generate(sink, helpers::get_all_implementable_methods(cls), inative_cxt)) return false;

         if(!root)
           if(!as_generator(scope_tab << scope_tab << "descs = descs.Concat(base.GetEoOps()).ToArray();\n").generate(sink, attributes::unused, inative_cxt))
             return false;

         if(!as_generator(
                scope_tab << scope_tab << "return descs;\n"
                << scope_tab << "}\n"
            ).generate(sink, attributes::unused, inative_cxt))
           return false;

         if(!as_generator
            (scope_tab << "public " << (root ? "" : "new " ) << "byte class_initializer(IntPtr klass)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "var descs = GetEoOps();\n"
             << scope_tab << scope_tab << "var count = descs.Length;\n"
             << scope_tab << scope_tab << "IntPtr descs_ptr = Marshal.AllocHGlobal(Marshal.SizeOf(descs[0])*count);\n"
             << scope_tab << scope_tab << "IntPtr ptr = descs_ptr;\n"
             << scope_tab << scope_tab << "for(int i = 0; i != count; ++i)\n"
             << scope_tab << scope_tab << "{\n"
             << scope_tab << scope_tab << scope_tab << "Marshal.StructureToPtr(descs[i], ptr, false);\n"
             << scope_tab << scope_tab << scope_tab << "ptr = IntPtr.Add(ptr, Marshal.SizeOf(descs[0]));\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "Efl_Object_Ops ops;\n"
             << scope_tab << scope_tab << "ops.descs = descs_ptr;\n"
             << scope_tab << scope_tab << "ops.count = (UIntPtr)count;\n"
             << scope_tab << scope_tab << "IntPtr ops_ptr = Marshal.AllocHGlobal(Marshal.SizeOf(ops));\n"
             << scope_tab << scope_tab << "Marshal.StructureToPtr(ops, ops_ptr, false);\n"
             << scope_tab << scope_tab << "Efl.Eo.Globals.efl_class_functions_set(klass, ops_ptr, IntPtr.Zero);\n"
             << scope_tab << scope_tab << "EoKlass = klass;\n"
            ).generate(sink, attributes::unused, inative_cxt)) return false;


         if(!as_generator(scope_tab << scope_tab << "return 1;\n"
                          << scope_tab << "}\n")
            .generate(sink, attributes::unused, inative_cxt)) return false;
         //
         // Native method definitions
         if(!as_generator(*(native_function_definition(cls)))
            .generate(sink, helpers::get_all_implementable_methods(cls), inative_cxt)) return false;

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

     std::string class_getter = "return Efl.Eo.Globals.efl_class_get(handle);";
     std::string native_inherit_full_name = name_helpers::klass_full_native_inherit_name(cls);

     // The klass field is static but there is no problem if multiple C# classes inherit from this generated one
     // as it is just a simple wrapper, forwarding the Eo calls either to the user API (where C#'s virtual method
     // resolution kicks in) or to the base implementation (efl_super).
     if (is_inherit)
       {
          if(!as_generator(
             scope_tab << "public " << (root ? "" : "new ") <<  "static System.IntPtr klass = System.IntPtr.Zero;\n"
             << scope_tab << "public " << (root ? "" : "new ") << "static " << native_inherit_full_name << " nativeInherit = new " << native_inherit_full_name << "();\n"
             ).generate(sink, attributes::unused, context))
            return false;
          class_getter = "return klass;";
       }

     std::string raw_klass_modifier;
     if (!root)
       raw_klass_modifier = "override ";
     else if (is_inherit)
       raw_klass_modifier = "virtual ";

     if(!as_generator(
                scope_tab << "///<summary>Pointer to the native class description.</summary>\n"
                << scope_tab << "public " << raw_klass_modifier << "System.IntPtr NativeClass {\n"
                << scope_tab << scope_tab << "get {\n"
                << scope_tab << scope_tab << scope_tab << class_getter << "\n" //return klass; }\n"
                << scope_tab << scope_tab << "}\n"
                << scope_tab << "}\n"
            ).generate(sink, attributes::unused, context))
         return false;

     // The remaining fields aren't needed in children classes.
     if (!root)
       return true;

     if (cls.get_all_events().size() > 0)
        if (!as_generator(scope_tab << (is_inherit ? "protected " : "private ") << "EventHandlerList eventHandlers = new EventHandlerList();\n").generate(sink, attributes::unused, context))
          return false;

     if (is_inherit)
      {
         if (!as_generator(
                scope_tab << "private static readonly object klassAllocLock = new object();\n"
                << scope_tab << "protected bool inherited;\n"
                ).generate(sink, attributes::unused, context))
           return false;
      }

     return as_generator(
                scope_tab << visibility << " System.IntPtr handle;\n"
                << scope_tab << "public Dictionary<String, IntPtr> cached_strings = new Dictionary<String, IntPtr>();" << "\n"
                << scope_tab << "public Dictionary<String, IntPtr> cached_stringshares = new Dictionary<String, IntPtr>();" << "\n"
                << scope_tab << "///<summary>Pointer to the native instance.</summary>\n"
                << scope_tab << "public System.IntPtr NativeHandle {\n"
                << scope_tab << scope_tab << "get { return handle; }\n"
                << scope_tab << "}\n"
             ).generate(sink, attributes::unused, context);
   }

   template <typename OutputIterator, typename Context>
   bool generate_constructors(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     bool root = !helpers::has_regular_ancestor(cls);
     auto inherit_name = name_helpers::klass_concrete_name(cls);
     auto native_inherit_name = name_helpers::klass_native_inherit_name(cls);

     if(!as_generator(
             scope_tab << "///<summary>Delegate for function to be called from inside the native constructor.</summary>\n"
             << scope_tab << "public" << (root ? "" : " new") << " delegate void ConstructingMethod(" << inherit_name << " obj);\n"
             << scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(context).actual_library_name(cls.filename)
             << ")] internal static extern System.IntPtr\n"
             << scope_tab << scope_tab << name_helpers::klass_get_name(cls) << "();\n"
            ).generate(sink, attributes::unused, context))
       return false;


     if (!root)
       {
          return as_generator(
                     scope_tab << "///<summary>Creates a new instance.</summary>\n"
                     << scope_tab << "///<param name=\"parent\">Parent instance.</param>\n"
                     << scope_tab << "///<param name=\"init_cb\">Delegate to call constructing methods that should be run inside the constructor.</param>\n"
                     << scope_tab << "public " << inherit_name << "(Efl.Object parent = null, ConstructingMethod init_cb=null) : base(nativeInherit.class_initializer, \"" << inherit_name << "\", " << name_helpers::klass_get_name(cls) <<  "(), typeof(" << inherit_name << "), parent, ref klass)\n"
                     << scope_tab << "{\n"
                     << scope_tab << scope_tab << "if (init_cb != null) {\n"
                     << scope_tab << scope_tab << scope_tab << "init_cb(this);\n"
                     << scope_tab << scope_tab << "}\n"
                     << scope_tab << scope_tab << "FinishInstantiation();\n"
                     << scope_tab << "}\n"

                     << scope_tab << "///<summary>Internal constructor to forward the wrapper initialization to the root class.</summary>\n"
                     << scope_tab << "protected " << inherit_name << "(Efl.Eo.Globals.class_initializer class_initializer, String klass_name, IntPtr base_klass, System.Type managed_type, Efl.Object parent, ref IntPtr target_klass) : base(class_initializer, klass_name, base_klass, managed_type, parent, ref target_klass) {}\n"

                     << scope_tab << "///<summary>Constructs an instance from a native pointer.</summary>\n"
                     << scope_tab << "public " << inherit_name << "(System.IntPtr raw)" << (root ? "" : " : base(raw)") << "\n"
                     << scope_tab << "{\n"
                     << scope_tab << scope_tab << (root ? "handle = raw;\n" : "")
                     << scope_tab << scope_tab << "register_event_proxies();\n"
                     << scope_tab << "}\n"
                  ).generate(sink, attributes::unused, context);
       }

     // Detailed constructors go only in root classes.
     return as_generator(
             scope_tab << "///<summary>Creates a new instance.</summary>\n"
             << scope_tab << "///<param name=\"parent\">Parent instance.</param>\n"
             << scope_tab << "///<param name=\"init_cb\">Delegate to call constructing methods that should be run inside the constructor.</param>\n"
             << scope_tab << "public " << inherit_name << "(Efl.Object parent = null, ConstructingMethod init_cb=null) : this(nativeInherit.class_initializer, \"" << inherit_name << "\", " << name_helpers::klass_get_name(cls) << "(), typeof(" << inherit_name << "), parent, ref klass)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (init_cb != null) {\n"
             << scope_tab << scope_tab << scope_tab << "init_cb(this);\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "FinishInstantiation();\n"
             << scope_tab << "}\n"

             << scope_tab << "protected " << inherit_name << "(Efl.Eo.Globals.class_initializer class_initializer, String klass_name, IntPtr base_klass, System.Type managed_type, Efl.Object parent, ref IntPtr target_klass)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "inherited = this.GetType() != managed_type;\n"
             << scope_tab << scope_tab << "IntPtr actual_klass = base_klass;\n"
             << scope_tab << scope_tab << "if (inherited) {\n"
             << scope_tab << scope_tab << scope_tab << "if (target_klass == System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "lock (klassAllocLock) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "if (target_klass == System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "target_klass = Efl.Eo.Globals.register_class(class_initializer, klass_name, base_klass);\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "if (target_klass == System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "throw new System.InvalidOperationException(\"Failed to initialize class '" << inherit_name << "'\");\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << "actual_klass = target_klass;\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "handle = Efl.Eo.Globals.instantiate_start(actual_klass, parent);\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"

             << scope_tab << "protected void FinishInstantiation()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (inherited) {\n"
             << scope_tab << scope_tab << scope_tab << "Efl.Eo.Globals.data_set(this);\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "handle = Efl.Eo.Globals.instantiate_end(handle);\n"
             << scope_tab << scope_tab << "Eina.Error.RaiseIfUnhandledException();\n"
             << scope_tab << "}\n"

             << scope_tab << "///<summary>Constructs an instance from a native pointer.</summary>\n"
             << scope_tab << "public " << inherit_name << "(System.IntPtr raw)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "handle = raw;\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"
             ).generate(sink, attributes::unused, context);
   }


   template <typename OutputIterator, typename Context>
   bool generate_dispose_methods(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::string name = join_namespaces(cls.namespaces, '.') + cls.eolian_name;
     if (helpers::has_regular_ancestor(cls))
       return true;

     std::string visibility = is_inherit_context(context) ? "protected virtual " : "";

     auto inherit_name = name_helpers::klass_concrete_name(cls);

     return as_generator(

             scope_tab << "///<summary>Destructor.</summary>\n"
             << scope_tab << "~" << inherit_name << "()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(false);\n"
             << scope_tab << "}\n"

             << scope_tab << "///<summary>Releases the underlying native instance.</summary>\n"
             << scope_tab << visibility << "void Dispose(bool disposing)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (handle != System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << "Efl.Eo.Globals.efl_unref(handle);\n"
             << scope_tab << scope_tab << scope_tab << "handle = System.IntPtr.Zero;\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << "}\n"

             << scope_tab << "///<summary>Releases the underlying native instance.</summary>\n"
             << scope_tab << "public void Dispose()\n"
             << scope_tab << "{\n"
             << scope_tab << "Efl.Eo.Globals.free_dict_values(cached_strings);" << "\n"
             << scope_tab << "Efl.Eo.Globals.free_stringshare_values(cached_stringshares);" << "\n"
             << scope_tab << scope_tab << "Dispose(true);\n"
             << scope_tab << scope_tab << "GC.SuppressFinalize(this);\n"
             << scope_tab << "}\n"
             ).generate(sink, attributes::unused, context);
   }

   template <typename OutputIterator, typename Context>
   bool generate_events_registration(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     bool root = !helpers::has_regular_ancestor(cls);
     std::string virtual_modifier = " ";

     if (!root)
       virtual_modifier = "override ";
     else
       {
          if (is_inherit_context(context))
             virtual_modifier = "virtual ";
       }

     // Event proxy registration
     if (!as_generator(
            scope_tab << (is_inherit_context(context) || !root ? "protected " : "") << virtual_modifier << "void register_event_proxies()\n"
            << scope_tab << "{\n"
         )
         .generate(sink, NULL, context))
         return false;

     // Generate event registrations here

     if (!root)
       if (!as_generator(scope_tab << scope_tab << "base.register_event_proxies();\n").generate(sink, NULL, context))
         return false;

     // Assigning the delegates
     if (!as_generator(*(event_registration(cls, cls))).generate(sink, cls.events, context))
       return false;

     for (auto&& c : helpers::non_implemented_interfaces(cls))
       {
          // Only non-regular types (which declare events through interfaces) need to register them.
          if (c.type == attributes::class_type::regular)
            continue;

          attributes::klass_def klass(get_klass(c, cls.unit), cls.unit);

          if (!as_generator(*(event_registration(klass, cls))).generate(sink, klass.events, context))
             return false;
       }

     if (!as_generator(
            scope_tab << "}\n"
                 ).generate(sink, NULL, context))
         return false;

     return true;
   }

   template <typename OutputIterator, typename Context>
   bool generate_events(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {

     if (!has_events(cls))
         return true;

     std::string visibility = is_inherit_context(context) ? "protected " : "private ";

     if (!helpers::has_regular_ancestor(cls))
       {
         if (!as_generator(scope_tab << visibility << "readonly object eventLock = new object();\n"
                 << scope_tab << visibility << "Dictionary<string, int> event_cb_count = new Dictionary<string, int>();\n")
                 .generate(sink, NULL, context))
             return false;

     // Callback registration functions
     if (!as_generator(
            scope_tab << visibility << "bool add_cpp_event_handler(string key, Efl.EventCb evt_delegate) {\n"
            << scope_tab << scope_tab << "int event_count = 0;\n"
            << scope_tab << scope_tab << "if (!event_cb_count.TryGetValue(key, out event_count))\n"
            << scope_tab << scope_tab << scope_tab << "event_cb_count[key] = event_count;\n"
            << scope_tab << scope_tab << "if (event_count == 0) {\n"

            << scope_tab << scope_tab << scope_tab << "IntPtr desc = Efl.EventDescription.GetNative(key);\n"
            << scope_tab << scope_tab << scope_tab << "if (desc == IntPtr.Zero) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to get native event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"

            << scope_tab << scope_tab << scope_tab << " bool result = Efl.Eo.Globals.efl_event_callback_priority_add(handle, desc, 0, evt_delegate, System.IntPtr.Zero);\n"
            << scope_tab << scope_tab << scope_tab << "if (!result) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to add event proxy for event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Error.RaiseIfUnhandledException();\n"
            << scope_tab << scope_tab << "} \n"
            << scope_tab << scope_tab << "event_cb_count[key]++;\n"
            << scope_tab << scope_tab << "return true;\n"
            << scope_tab << "}\n"
            << scope_tab << visibility << "bool remove_cpp_event_handler(string key, Efl.EventCb evt_delegate) {\n"
            << scope_tab << scope_tab << "int event_count = 0;\n"
            << scope_tab << scope_tab << "if (!event_cb_count.TryGetValue(key, out event_count))\n"
            << scope_tab << scope_tab << scope_tab << "event_cb_count[key] = event_count;\n"
            << scope_tab << scope_tab << "if (event_count == 1) {\n"

            << scope_tab << scope_tab << scope_tab << "IntPtr desc = Efl.EventDescription.GetNative(key);\n"
            << scope_tab << scope_tab << scope_tab << "if (desc == IntPtr.Zero) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to get native event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"

            << scope_tab << scope_tab << scope_tab << "bool result = Efl.Eo.Globals.efl_event_callback_del(handle, desc, evt_delegate, System.IntPtr.Zero);\n"
            << scope_tab << scope_tab << scope_tab << "if (!result) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Failed to remove event proxy for event {key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Error.RaiseIfUnhandledException();\n"
            << scope_tab << scope_tab << "} else if (event_count == 0) {\n"
            << scope_tab << scope_tab << scope_tab << "Eina.Log.Error($\"Trying to remove proxy for event {key} when there is nothing registered.\");\n"
            << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << "} \n"
            << scope_tab << scope_tab << "event_cb_count[key]--;\n"
            << scope_tab << scope_tab << "return true;\n"
            << scope_tab << "}\n"
            )
             .generate(sink, NULL, context))
         return false;
       }

     // Self events
     if (!as_generator(*(event_definition(cls, cls))).generate(sink, cls.events, context))
       return false;

     // Inherited events

     // For now, as mixins can inherit from regular classes, we can't filter out inherited events.
     auto inherits = helpers::non_implemented_interfaces(cls);
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
