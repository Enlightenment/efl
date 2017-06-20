#ifndef EOLIAN_MONO_CLASS_DEFINITION_HPP
#define EOLIAN_MONO_CLASS_DEFINITION_HPP

#include "grammar/integral.hpp"
#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "function_blacklist.hh"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "namespace.hh"
#include "function_definition.hh"
#include "function_registration.hh"
#include "function_declaration.hh"
#include "grammar/string.hpp"
#include "grammar/attribute_replace.hpp"
#include "grammar/integral.hpp"
#include "grammar/case.hpp"
#include "using_decl.hh"
#include "utils.hh"

#include <string>
#include <algorithm>

namespace eolian_mono {

struct get_csharp_type_visitor
{
    typedef get_csharp_type_visitor visitor_type;
    typedef std::string result_type;
    std::string operator()(grammar::attributes::regular_type_def const& type) const
    {
        std::stringstream csharp_name;
        for (auto&& i  : escape_namespace(type.namespaces))
           csharp_name << utils::to_lowercase(i) << ".";
        csharp_name << type.base_type;

        return csharp_name.str();
    }
    std::string operator()(grammar::attributes::klass_name const& name) const
    {
        std::stringstream csharp_name;
        for (auto&& i  : escape_namespace(name.namespaces))
           csharp_name << utils::to_lowercase(i) << ".";
        csharp_name << name.eolian_name;

        return csharp_name.str();
    }
    std::string operator()(attributes::complex_type_def const&) const
    {
        return "UNSUPPORTED";
    }
};

struct get_event_args_visitor
{

    std::string arg_type;

    typedef get_event_args_visitor visitor_type;
    typedef std::string result_type;
    std::string operator()(grammar::attributes::regular_type_def const&) const
    {
        return "(" + arg_type + ")Marshal.PtrToStructure(evt.info, typeof(" + arg_type + "))";
    }
    std::string operator()(grammar::attributes::klass_name const&) const
    {
        return "new " + arg_type + "Concrete(evt.info)";
    }
    std::string operator()(attributes::complex_type_def const&) const
    {
        return "UNSUPPORTED";
    }
};

struct klass
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::string suffix, class_type;
     switch(cls.type)
       {
       case attributes::class_type::regular:
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

     std::vector<std::string> namespaces = escape_namespace(cls.namespaces);
     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate(sink, namespaces, add_lower_case_context(context))) return false;

     // FIXME Generate local event argument wrappers
     for (auto&& e : cls.events)
       {
          efl::eina::optional<grammar::attributes::type_def> etype = e.type;
          if (!etype.is_engaged())
            continue;

          std::string evt_name = utils::to_uppercase(e.name);
          std::replace(evt_name.begin(), evt_name.end(), ',', '_');
          std::string arg_type = (*etype).original_type.visit(get_csharp_type_visitor{});


          if (!as_generator("public class " << evt_name << "_Args : EventArgs {\n"
                      << scope_tab << "public " << arg_type << " arg { get; set; }\n"
                      << "}\n").generate(sink, NULL, context))
              return false;
       }

     // Interface class
     if(!as_generator
        (
         "public " /*<< class_type*/ "interface" /*<<*/ " " << string << " : "
         )
        .generate(sink, cls.cxx_name, context))
       return false;
     for(auto first = std::begin(cls.immediate_inherits)
           , last = std::end(cls.immediate_inherits); first != last; ++first)
       {
         if(!as_generator("\n" << scope_tab << *(lower_case[string] << ".") << string << " ,")
            .generate(sink, std::make_tuple(escape_namespace(first->namespaces), first->eolian_name), context))
           return false;
         // if(std::next(first) != last)
         //   *sink++ = ',';
       }
     // if(cls.immediate_inherits.empty())
       if(!as_generator("\n" << scope_tab << "efl.eo.IWrapper").generate(sink, attributes::unused, context)) return false;
     if(!as_generator("\n{\n").generate(sink, attributes::unused, context)) return false;
     
     if(!as_generator(*(scope_tab << function_declaration))
        .generate(sink, cls.functions, context)) return false;

     // FIXME Move the event generator into another generator like function?
     for (auto &&e : cls.events)
       {
          std::string wrapper_args_type;
          std::string evt_name = utils::to_uppercase(e.name);
          std::replace(evt_name.begin(), evt_name.end(), ',', '_');

          efl::eina::optional<grammar::attributes::type_def> etype = e.type;
          if (etype.is_engaged())
              wrapper_args_type = "<" + evt_name + "_Args>";

         //FIXME Add a way to generate camelcase names
         if (!as_generator(
                     scope_tab << "event EventHandler" << wrapper_args_type << " " 
                     << evt_name << ";\n"
                     ).generate(sink, NULL, context))
             return false;
       }

     if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;

     auto class_get_name = *(lower_case[string] << "_") << lower_case[string] << "_class_get";
     // Concrete class
     // if(class_type == "class")
     //   {
         if(!as_generator
            (
             "public class " << string << "Concrete : " << string << ", IDisposable\n{\n"
             << scope_tab << "System.IntPtr handle;\n"
             << scope_tab << "public System.IntPtr raw_handle {\n"
             << scope_tab << scope_tab << "get { return handle; }\n"
             << scope_tab << "}\n"
             << scope_tab << "public System.IntPtr raw_klass {\n"
             << scope_tab << scope_tab << "get { return efl.eo.Globals.efl_class_get(handle); }\n"
             << scope_tab << "}\n"
             << scope_tab << "[System.Runtime.InteropServices.DllImport(\"" << context_find_tag<library_context>(context).library_name
             << "\")] static extern System.IntPtr\n"
             << scope_tab << scope_tab << class_get_name << "();\n"
             << (class_type == "class" ? "" : "/*")
             << scope_tab << "public " << string << "Concrete(efl.Object parent = null)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "System.IntPtr klass = " << class_get_name << "();\n"
             << scope_tab << scope_tab << "System.IntPtr parent_ptr = System.IntPtr.Zero;\n"
             << scope_tab << scope_tab << "if(parent != null)\n"
             << scope_tab << scope_tab << scope_tab << "parent_ptr = parent.raw_handle;\n"
             << scope_tab << scope_tab << "System.IntPtr eo = efl.eo.Globals._efl_add_internal_start(\"file\", 0, klass, parent_ptr, 0, 0);\n"
             << scope_tab << scope_tab << "handle = efl.eo.Globals._efl_add_end(eo, 0, 0);\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"
             << (class_type == "class" ? "" : "*/")
             << scope_tab << "public " << string << "Concrete(System.IntPtr raw)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "handle = raw;\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"
             << scope_tab << "~" << string << "Concrete()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(false);\n"
             << scope_tab << "}\n"
             << scope_tab << "protected virtual void Dispose(bool disposing)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (handle != System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << "efl.eo.Globals.efl_unref(handle);\n"
             << scope_tab << scope_tab << scope_tab << "handle = System.IntPtr.Zero;\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << "}\n"
             << scope_tab << "public void Dispose()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(true);\n"
             << scope_tab << scope_tab << "GC.SuppressFinalize(this);\n"
             << scope_tab << "}\n"
             /* << scope_tab << "public delegate void EflEventHandler(object sender, EventArgs e);\n" */
            )
            .generate(sink
              , std::make_tuple(
                cls.cxx_name, cls.cxx_name, cls.namespaces, cls.eolian_name
                , cls.cxx_name, cls.namespaces, cls.eolian_name, cls.cxx_name
                , cls.cxx_name)
              , context))
           return false;

         if (!generate_events(sink, cls, context))
             return false;

         if (!generate_events_registration(sink, cls, context))
             return false;
     
         if(!as_generator(*(function_definition))
            .generate(sink, cls.functions, context)) return false;

         for(auto first = std::begin(cls.inherits)
               , last = std::end(cls.inherits); first != last; ++first)
           {
             attributes::klass_def klass(get_klass(*first, NULL), NULL);
             
             if(!as_generator(*(function_definition))
                .generate(sink, klass.functions, context)) return false;
           }

         
         if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;
       // }

     // Inherit class
     if(class_type == "class")
       {
        bool cls_has_string_return = has_string_return(cls);
        bool cls_has_stringshare_return = has_stringshare_return(cls);

         if(!as_generator
            (
             "public " << class_type << " " << string << "Inherit : " << string << ", IDisposable\n{\n"
             << scope_tab << "System.IntPtr handle;\n"
             << scope_tab << "public static System.IntPtr klass = System.IntPtr.Zero;\n"
             << scope_tab << "private static readonly object klassAllocLock = new object();\n"
             << scope_tab << (cls_has_string_return ? ("public Dictionary<String, IntPtr> cached_strings = new Dictionary<String, IntPtr>();") : "") << "\n"
             << scope_tab << (cls_has_stringshare_return ? ("public Dictionary<String, IntPtr> cached_stringshares = new Dictionary<String, IntPtr>();") : "") << "\n"
             << scope_tab << "public System.IntPtr raw_handle {\n"
             << scope_tab << scope_tab << "get { return handle; }\n"
             << scope_tab << "}\n"
             << scope_tab << "public System.IntPtr raw_klass {\n"
             << scope_tab << scope_tab << "get { return klass; }\n"
             << scope_tab << "}\n"
             << scope_tab << "[System.Runtime.InteropServices.DllImport(\"" << context_find_tag<library_context>(context).library_name
             << "\")] static extern System.IntPtr\n"
             << scope_tab << scope_tab << class_get_name << "();\n"
             << scope_tab << "public " << string << "Inherit(efl.Object parent = null, System.Type interface1 = null)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (klass == System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << "lock (klassAllocLock) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "if (klass == System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "klass = efl.eo.Globals.register_class(new efl.eo.Globals.class_initializer(" << string << "NativeInherit.class_initializer), " << class_get_name << "());\n"
             //<< scope_tab << scope_tab << "klass = efl.eo.Globals.register_class(null/*new efl.eo.Globals.class_initializer(" << string << "NativeInherit.class_initializer)*/, " << class_get_name << "());\n"
             << scope_tab << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "handle = efl.eo.Globals.instantiate(klass, parent);\n"
             << scope_tab << scope_tab << "efl.eo.Globals.data_set(this);\n"
             << scope_tab << scope_tab << "register_event_proxies();\n"
             << scope_tab << "}\n"
             << scope_tab << "~" << string << "Inherit()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Dispose(false);\n"
             << scope_tab << "}\n"
             << scope_tab << "protected virtual void Dispose(bool disposing)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "if (handle != System.IntPtr.Zero) {\n"
             << scope_tab << scope_tab << scope_tab << "efl.eo.Globals.efl_unref(handle);\n"
             << scope_tab << scope_tab << scope_tab << "handle = System.IntPtr.Zero;\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << "}\n"
             << scope_tab << "public void Dispose()\n"
             << scope_tab << "{\n"
             << scope_tab << (cls_has_string_return ? "efl.eo.Globals.free_dict_values(cached_strings);" : "") << "\n"
             << scope_tab << (cls_has_stringshare_return ? "efl.eo.Globals.free_stringshare_values(cached_stringshares);" : "") << "\n"
             << scope_tab << scope_tab << "Dispose(true);\n"
             << scope_tab << scope_tab << "GC.SuppressFinalize(this);\n"
             << scope_tab << "}\n"
            )
            .generate(sink
              , std::make_tuple(
                cls.cxx_name, cls.cxx_name, cls.namespaces, cls.eolian_name
                , cls.cxx_name, cls.cxx_name, cls.namespaces, cls.eolian_name, cls.cxx_name
                , cls.cxx_name)
              , context))
           return false;

         if (!generate_events(sink, cls, context))
             return false;

         if (!generate_events_registration(sink, cls, context))
             return false;
     
         if(!as_generator(*(function_definition(true)))
            .generate(sink, cls.functions, context)) return false;

         for(auto first = std::begin(cls.inherits)
               , last = std::end(cls.inherits); first != last; ++first)
           {
             attributes::klass_def klass(get_klass(*first, NULL), NULL);
             
             if(!as_generator(*(function_definition(true)))
                .generate(sink, klass.functions, context)) return false;
           }

         
         if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;
       }

     std::size_t function_count = cls.functions.size();
     for(auto first = std::begin(cls.inherits)
           , last = std::end(cls.inherits); first != last; ++first)
       {
         attributes::klass_def klass(get_klass(*first, NULL), NULL);
         function_count += klass.functions.size();
       }
     // function_count--;

     int function_registration_index = 0;
     auto index_generator = [&function_registration_index]
       {
         return function_registration_index++;
       };
     
     // Native Inherit class
     if(class_type == "class")
       {
         if(!as_generator
            (
             "public " << class_type << " " << string << "NativeInherit {\n"
             << scope_tab << "public static byte class_initializer(IntPtr klass)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "Efl_Op_Description[] descs = new Efl_Op_Description[" << grammar::int_ << "];\n"
             << *(function_registration(index_generator, cls))
            )
            .generate(sink, std::make_tuple(cls.cxx_name, function_count, cls.functions), context))
           return false;
         for(auto first = std::begin(cls.inherits)
               , last = std::end(cls.inherits); first != last; ++first)
           {
             attributes::klass_def klass(get_klass(*first, NULL), NULL);
             if(!as_generator(*(function_registration(index_generator, cls)))
                .generate(sink, klass.functions, context)) return false;
           }

         if(!as_generator
            (   scope_tab << scope_tab << "IntPtr descs_ptr = Marshal.AllocHGlobal(Marshal.SizeOf(descs[0])*" << function_count << ");\n"
             << scope_tab << scope_tab << "IntPtr ptr = descs_ptr;\n"
             << scope_tab << scope_tab << "for(int i = 0; i != " << function_count << "; ++i)\n"
             << scope_tab << scope_tab << "{\n"
             << scope_tab << scope_tab << scope_tab << "Marshal.StructureToPtr(descs[i], ptr, false);\n"
             << scope_tab << scope_tab << scope_tab << "ptr = IntPtr.Add(ptr, Marshal.SizeOf(descs[0]));\n"
             << scope_tab << scope_tab << "}\n"
             << scope_tab << scope_tab << "Efl_Object_Ops ops;\n"
             << scope_tab << scope_tab << "ops.descs = descs_ptr;\n"
             << scope_tab << scope_tab << "ops.count = (UIntPtr)" << function_count << ";\n"
             << scope_tab << scope_tab << "IntPtr ops_ptr = Marshal.AllocHGlobal(Marshal.SizeOf(ops));\n"
             << scope_tab << scope_tab << "Marshal.StructureToPtr(ops, ops_ptr, false);\n"
             << scope_tab << scope_tab << "efl.eo.Globals.efl_class_functions_set(klass, ops_ptr, IntPtr.Zero);\n"
            ).generate(sink, attributes::unused, context)) return false;
         
         
         if(!as_generator(scope_tab << scope_tab << "return 1;\n"
                          << scope_tab << "}\n")
            .generate(sink, attributes::unused, context)) return false;
     
         if(!as_generator(*(native_function_definition(cls)))
            .generate(sink, cls.functions, context)) return false;

         for(auto first = std::begin(cls.inherits)
               , last = std::end(cls.inherits); first != last; ++first)
           {
             attributes::klass_def klass(get_klass(*first, NULL), NULL);
             
             if(!as_generator(*(native_function_definition(cls)))
                .generate(sink, klass.functions, context)) return false;
           }
         
         if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;
       }
     
     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, namespaces, context)) return false;
     
     return true;
   }

   template <typename OutputIterator, typename Context>
   bool generate_events_registration(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     // Event proxy registration
     if (!as_generator(
            scope_tab << "private void register_event_proxies()\n"
            << scope_tab << "{\n"
         )
         .generate(sink, NULL, context))
         return false;

     // Generate event registrations here

     // Assigning the delegates
     for (auto&& e : cls.events)
       {
           if (!as_generator(scope_tab << scope_tab << "evt_" << grammar::string_replace(',', '_') << "_delegate = "
                       << "new efl.Event_Cb(on_" << grammar::string_replace(',', '_') << "_NativeCallback);\n")
                   .generate(sink, std::make_tuple(e.name, e.name), context))
                return false;
       }

     for (auto&& c : cls.inherits)
       {
          attributes::klass_def klass(get_klass(c, NULL), NULL);

          for (auto&& e : klass.events)
            {
               std::string wrapper_event_name = translate_inherited_event_name(e, klass);

               if (!as_generator(scope_tab << scope_tab << "evt_" << wrapper_event_name << "_delegate = "
                           << "new efl.Event_Cb(on_" << wrapper_event_name << "_NativeCallback);\n")
                       .generate(sink, NULL, context))
                   return false;
            }
       }
     

     if (!as_generator(
            scope_tab << "}\n"
                 ).generate(sink, NULL, context))
         return false;

     return true;
   }

   static std::string translate_inherited_event_name(const attributes::event_def &evt, const attributes::klass_def &klass)
   {
       std::stringstream s;

       for (auto&& n : klass.namespaces)
         {
            s << n;
            s << '_';
         }
       std::string evt_name = utils::to_uppercase(evt.name);
       std::replace(evt_name.begin(), evt_name.end(), ',', '_');
       s << klass.cxx_name << '_' << evt_name;
       return s.str();
   }

   template <typename OutputIterator, typename Context>
   bool generate_events(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {

     if (!has_events(cls))
         return true;

     if (!as_generator(scope_tab << "private readonly object eventLock = new object();\n"
             << scope_tab << "private Dictionary<string, int> event_cb_count = new Dictionary<string, int>();\n")
             .generate(sink, NULL, context))
         return false;

     // Callback registration functions
     if (!as_generator(
            scope_tab << "private bool add_cpp_event_handler(string key, efl.Event_Cb evt_delegate) {\n"
            << scope_tab << scope_tab << "int event_count = 0;\n"
            << scope_tab << scope_tab << "if (!event_cb_count.TryGetValue(key, out event_count))\n"
            << scope_tab << scope_tab << scope_tab << "event_cb_count[key] = event_count;\n"
            << scope_tab << scope_tab << "if (event_count == 0) {\n"
            << scope_tab << scope_tab << scope_tab << "efl.kw_event.Description desc = new efl.kw_event.Description(key);\n"
            << scope_tab << scope_tab << scope_tab << "bool result = efl.eo.Globals.efl_event_callback_priority_add(handle, desc, 0, evt_delegate, System.IntPtr.Zero);\n"
            << scope_tab << scope_tab << scope_tab << "if (!result) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error(\"Failed to add event proxy for event ${key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << scope_tab << "eina.Error.RaiseIfOcurred();\n"
            << scope_tab << scope_tab << "} \n"
            << scope_tab << scope_tab << "event_cb_count[key]++;\n"
            << scope_tab << scope_tab << "return true;\n"
            << scope_tab << "}\n"
            << scope_tab << "private bool remove_cpp_event_handler(string key, efl.Event_Cb evt_delegate) {\n"
            << scope_tab << scope_tab << "int event_count = 0;\n"
            << scope_tab << scope_tab << "if (!event_cb_count.TryGetValue(key, out event_count))\n"
            << scope_tab << scope_tab << scope_tab << "event_cb_count[key] = event_count;\n"
            << scope_tab << scope_tab << "if (event_count == 1) {\n"
            << scope_tab << scope_tab << scope_tab << "efl.kw_event.Description desc = new efl.kw_event.Description(key);\n"
            << scope_tab << scope_tab << scope_tab << "bool result = efl.eo.Globals.efl_event_callback_del(handle, desc, evt_delegate, System.IntPtr.Zero);\n"
            << scope_tab << scope_tab << scope_tab << "if (!result) {\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error(\"Failed to remove event proxy for event ${key}\");\n"
            << scope_tab << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << scope_tab << "}\n"
            << scope_tab << scope_tab << scope_tab << "eina.Error.RaiseIfOcurred();\n"
            << scope_tab << scope_tab << "} else if (event_count == 0) {\n"
            << scope_tab << scope_tab << scope_tab << "eina.Log.Error(\"Trying to remove proxy for event ${key} when there is nothing registered.\");\n"
            << scope_tab << scope_tab << scope_tab << "return false;\n"
            << scope_tab << scope_tab << "} \n"
            << scope_tab << scope_tab << "event_cb_count[key]--;\n"
            << scope_tab << scope_tab << "return true;\n"
            << scope_tab << "}\n"
            )
             .generate(sink, NULL, context))
         return false;

     // Self events
     for (auto&& e : cls.events)
       {
           std::string upper_name = utils::to_uppercase(e.name);
           std::replace(upper_name.begin(), upper_name.end(), ',', '_');
           std::string upper_c_name = utils::to_uppercase(e.c_name);
           std::string event_name = e.name;
           std::replace(event_name.begin(), event_name.end(), ',', '_');

           std::string wrapper_args_type = "EventArgs";
           std::string wrapper_args_template = "";
           std::string event_args = "EventArgs args = EventArgs.Empty;\n";

           efl::eina::optional<grammar::attributes::type_def> etype = e.type;
           if (etype.is_engaged())
             {
                wrapper_args_type = upper_name + "_Args";
                wrapper_args_template = "<" + wrapper_args_type + ">";
                std::string arg_type = wrapper_args_type + " args = new " + wrapper_args_type + "();\n"; // = (*etype).original_type.visit(get_csharp_type_visitor{});
                std::string actual_arg_type = (*etype).original_type.visit(get_csharp_type_visitor{});
                arg_type += "args.arg = " + (*etype).original_type.visit(get_event_args_visitor{actual_arg_type}) + ";\n";

                event_args = arg_type;
             }
           // Marshal.PtrToStructure for value types

           // Wrapper event declaration
          if(!as_generator(
                scope_tab << "protected event EventHandler" << wrapper_args_template << " " << upper_name << ";\n"
                << scope_tab << "protected void On_" << event_name << "(" << wrapper_args_type << " e)\n"
                << scope_tab << "{\n"
                << scope_tab << scope_tab << "EventHandler" << wrapper_args_template << " evt;\n"
                << scope_tab << scope_tab << "lock (eventLock) {\n"
                << scope_tab << scope_tab << scope_tab << "evt = " << upper_name << ";\n"
                << scope_tab << scope_tab << "}\n"
                << scope_tab << scope_tab << "if (evt != null) { evt(this, e); }\n"
                << scope_tab << "}\n"
                << scope_tab << "public void on_" << event_name << "_NativeCallback(System.IntPtr data, ref efl.Event evt)\n"
                << scope_tab << "{\n"
                << scope_tab << event_args
                << scope_tab << scope_tab << "try {\n"
                << scope_tab << scope_tab << scope_tab << "On_" << event_name << "(args);\n"
                << scope_tab << scope_tab <<  "} catch (Exception e) {\n"
                << scope_tab << scope_tab << scope_tab << "eina.Log.Error(e.Message);\n"
                << scope_tab << scope_tab << scope_tab << "eina.Error.Set(eina.Error.EFL_ERROR);\n"
                << scope_tab << scope_tab << "}\n"
                << scope_tab << "}\n"
                << scope_tab << "efl.Event_Cb evt_" << event_name << "_delegate;\n"
                << scope_tab << "event EventHandler" << wrapper_args_template << " " << cls.cxx_name << "." << upper_name << "{\n")
                  .generate(sink, NULL, context))
              return false;

          if (!as_generator(
                      scope_tab << scope_tab << "add {\n"
                      << scope_tab << scope_tab << scope_tab << "lock (eventLock) {\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << "if (add_cpp_event_handler(key, this.evt_" << event_name << "_delegate))\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << upper_name << " += value;\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << "else\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error(\"Error adding proxy for event ${key}\");\n"
                      << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
                      << scope_tab << scope_tab << "}\n"
                      << scope_tab << scope_tab << "remove {\n"
                      << scope_tab << scope_tab << scope_tab << "lock (eventLock) {\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << "if (remove_cpp_event_handler(key, this.evt_" << event_name << "_delegate))\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << upper_name << " -= value;\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << "else\n"
                      << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error(\"Error removing proxy for event ${key}\");\n"
                      << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
                      << scope_tab << scope_tab << "}\n"
                      << scope_tab << "}\n")
                  .generate(sink, NULL, context))
              return false;
       }

     // Inherited events
     for (auto&& c : cls.inherits)
       {
          attributes::klass_def klass(get_klass(c, NULL), NULL);

          // FIXME Enable inherited events registration. Beware of conflicting events
          for (auto&& e : klass.events)
            {

               std::string wrapper_evt_name = translate_inherited_event_name(e, klass);
               std::string upper_name = utils::to_uppercase(e.name);
               std::replace(upper_name.begin(), upper_name.end(), ',', '_');
               std::string upper_c_name = utils::to_uppercase(e.c_name);

               std::stringstream wrapper_args_type;
               std::string wrapper_args_template;
               std::string event_args = "EventArgs args = EventArgs.Empty;\n";

               efl::eina::optional<grammar::attributes::type_def> etype = e.type;
               if (etype.is_engaged())
                 {
                   for (auto&& i : klass.namespaces) 
                     {
                         wrapper_args_type << utils::to_lowercase(i) << ".";
                     }
                   wrapper_args_type << upper_name << "_Args";
                   wrapper_args_template = "<" + wrapper_args_type.str() + ">";
                   std::string arg_type = wrapper_args_type.str() + " args = new " + wrapper_args_type.str() + "();\n"; // = (*etype).original_type.visit(get_csharp_type_visitor{});
                   std::string actual_arg_type = (*etype).original_type.visit(get_csharp_type_visitor{});
                arg_type += "args.arg = " + (*etype).original_type.visit(get_event_args_visitor{actual_arg_type}) + ";\n";
                   event_args = arg_type;
                 }
               else
                 {
                   wrapper_args_type << "EventArgs";
                 }

               if (!as_generator(
                     scope_tab << "protected event EventHandler" << wrapper_args_template << " " << wrapper_evt_name << ";\n"
                     << scope_tab << "protected void On_" << wrapper_evt_name << "(" << wrapper_args_type.str() << " e)\n"
                     << scope_tab << "{\n"
                     << scope_tab << scope_tab << "EventHandler" << wrapper_args_template << " evt;\n"
                     << scope_tab << scope_tab << "lock (eventLock) {\n"
                     << scope_tab << scope_tab << scope_tab << "evt = " << wrapper_evt_name << ";\n"
                     << scope_tab << scope_tab << "}\n"
                     << scope_tab << scope_tab << "if (evt != null) { evt(this, e); }\n"
                     << scope_tab << "}\n"
                     << scope_tab << "efl.Event_Cb evt_" << wrapper_evt_name << "_delegate;\n"
                     << scope_tab << "protected void on_" << wrapper_evt_name << "_NativeCallback(System.IntPtr data, ref efl.Event evt)"
                     << scope_tab << "{\n"
                     << scope_tab << event_args
                    << scope_tab << scope_tab << "try {\n"
                    << scope_tab << scope_tab << scope_tab << "On_" << wrapper_evt_name << "(args);\n"
                    << scope_tab << scope_tab <<  "} catch (Exception e) {\n"
                    << scope_tab << scope_tab << scope_tab << "eina.Log.Error(e.Message);\n"
                    << scope_tab << scope_tab << scope_tab << "eina.Error.Set(eina.Error.EFL_ERROR);\n"
                    << scope_tab << scope_tab << "}\n"
                     << scope_tab << "}\n"
                     << scope_tab << "event EventHandler" << wrapper_args_template << " " << *(lower_case[string] << ".") << klass.cxx_name << ".")
                       .generate(sink, escape_namespace(klass.namespaces), context))
                   return false;
               if (!as_generator(upper_name << " {\n"
                          << scope_tab << scope_tab << "add {\n"
                          << scope_tab << scope_tab << scope_tab << "lock (eventLock) {\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << "if (add_cpp_event_handler(key, this.evt_" << wrapper_evt_name << "_delegate))\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << wrapper_evt_name << " += value;\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << "else\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error(\"Error adding proxy for event ${key}\");\n"
                          << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
                          << scope_tab << scope_tab << "}\n"
                          << scope_tab << scope_tab << "remove {\n"
                          << scope_tab << scope_tab << scope_tab << "lock (eventLock) {\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << "string key = \"_" << upper_c_name << "\";\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << "if (remove_cpp_event_handler(key, this.evt_" << wrapper_evt_name << "_delegate))\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << wrapper_evt_name << " -= value;\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << "else\n"
                          << scope_tab << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Error(\"Error removing proxy for event ${key}\");\n"
                          << scope_tab << scope_tab << scope_tab << "}\n" // End of lock block
                          << scope_tab << scope_tab << "}\n"
                     << scope_tab << "}\n")
                 .generate(sink, NULL, context))
                   return false;
            }
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
