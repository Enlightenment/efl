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
#include "grammar/integral.hpp"
#include "grammar/case.hpp"
#include "using_decl.hh"

namespace eolian_mono {

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

     if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;

     auto class_get_name = *(lower_case[string] << "_") << lower_case[string] << "_class_get";
     // Concrete class
     // if(class_type == "class")
     //   {
         if(!as_generator
            (
             "public class " << string << "Concrete : " << string << "\n{\n"
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
             << scope_tab << "}\n"
             << (class_type == "class" ? "" : "*/")
             << scope_tab << "public " << string << "Concrete(System.IntPtr raw)\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "handle = raw;\n"
             << scope_tab << "}\n"
            )
            .generate(sink, std::make_tuple(cls.cxx_name, cls.cxx_name, cls.namespaces, cls.eolian_name, cls.cxx_name, cls.namespaces, cls.eolian_name, cls.cxx_name), context))
           return false;
     
         if(!as_generator(*(function_definition))
            .generate(sink, cls.functions, context)) return false;

         for(auto first = std::begin(cls.inherits)
               , last = std::end(cls.inherits); first != last; ++first)
           {
             attributes::klass_def klass(get_klass(*first));
             
             if(!as_generator(*(function_definition))
                .generate(sink, klass.functions, context)) return false;
           }

         
         if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;
       // }

     // Inherit class
     if(class_type == "class")
       {
         if(!as_generator
            (
             "public " << class_type << " " << string << "Inherit : " << string << "\n{\n"
             << scope_tab << "System.IntPtr handle;\n"
             << scope_tab << "public static System.IntPtr klass;\n"
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
             << scope_tab << scope_tab << "klass = efl.eo.Globals.register_class(new efl.eo.Globals.class_initializer(" << string << "NativeInherit.class_initializer), " << class_get_name << "());\n"
             //<< scope_tab << scope_tab << "klass = efl.eo.Globals.register_class(null/*new efl.eo.Globals.class_initializer(" << string << "NativeInherit.class_initializer)*/, " << class_get_name << "());\n"             
             << scope_tab << scope_tab << "handle = efl.eo.Globals.instantiate(klass, parent);\n"
             << scope_tab << scope_tab << "efl.eo.Globals.data_set(this);\n"
             << scope_tab << "}\n"
            )
            .generate(sink, std::make_tuple(cls.cxx_name, cls.cxx_name, cls.namespaces, cls.eolian_name, cls.cxx_name, cls.cxx_name, cls.namespaces, cls.eolian_name, cls.cxx_name), context))
           return false;
     
         if(!as_generator(*(function_definition(true)))
            .generate(sink, cls.functions, context)) return false;

         for(auto first = std::begin(cls.inherits)
               , last = std::end(cls.inherits); first != last; ++first)
           {
             attributes::klass_def klass(get_klass(*first));
             
             if(!as_generator(*(function_definition(true)))
                .generate(sink, klass.functions, context)) return false;
           }

         
         if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;
       }

     std::size_t function_count = cls.functions.size();
     for(auto first = std::begin(cls.inherits)
           , last = std::end(cls.inherits); first != last; ++first)
       {
         attributes::klass_def klass(get_klass(*first));
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
             attributes::klass_def klass(get_klass(*first));
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
             attributes::klass_def klass(get_klass(*first));
             
             if(!as_generator(*(native_function_definition(cls)))
                .generate(sink, klass.functions, context)) return false;
           }
         
         if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;
       }
     
     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, namespaces, context)) return false;
     
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
