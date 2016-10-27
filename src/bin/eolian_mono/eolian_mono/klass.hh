#ifndef EOLIAN_MONO_CLASS_DEFINITION_HPP
#define EOLIAN_MONO_CLASS_DEFINITION_HPP

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
//#include "grammar/parameter.hpp"
#include "function_definition.hh"
#include "function_declaration.hh"
#include "grammar/string.hpp"
#include "grammar/case.hpp"
// #include "grammar/address_of.hpp"
// #include "grammar/attribute_reorder.hpp"
// #include "grammar/attribute_conditional.hpp"
// #include "grammar/attribute_replace.hpp"
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

     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);
     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate(sink, cpp_namespaces, add_lower_case_context(context))) return false;

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
         if(!as_generator("\n" << scope_tab << *(lower_case[string] << ".") << string)
            .generate(sink, std::make_tuple(attributes::cpp_namespaces(first->namespaces), first->eolian_name), context))
           return false;
         if(std::next(first) != last)
           *sink++ = ',';
       }
     if(cls.immediate_inherits.empty())
       if(!as_generator("\n" << scope_tab << "efl.eo.IWrapper").generate(sink, attributes::unused, context)) return false;
     if(!as_generator("\n{\n").generate(sink, attributes::unused, context)) return false;
     
     if(!as_generator(*(scope_tab << function_declaration))
        .generate(sink, cls.functions, context)) return false;

     if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;

     // Concrete class
     if(class_type == "class")
       {
         auto class_get_name = *(lower_case[string] << "_") << lower_case[string] << "_class_get";
         if(!as_generator
            (
             "public " << class_type << " " << string << "Concrete : " << string << "\n{\n"
             << scope_tab << "System.IntPtr handle;\n"
             << scope_tab << "[System.Runtime.InteropServices.DllImport(\"eo\")] static extern System.IntPtr\n"
             << scope_tab << "_efl_add_internal_start([System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.LPStr)] System.String file, int line,\n"
             << scope_tab << scope_tab << "System.IntPtr klass, System.IntPtr parent, byte is_ref, byte is_fallback);\n"
             << scope_tab << "[System.Runtime.InteropServices.DllImport(\"eo\")] static extern System.IntPtr\n"
             << scope_tab << "_efl_add_end(System.IntPtr eo, byte is_ref, byte is_fallback);\n"
             << scope_tab << "[System.Runtime.InteropServices.DllImport(\"" << context_find_tag<library_context>(context).library_name
             << "\")] static extern System.IntPtr\n"
             << scope_tab << scope_tab << class_get_name << "();\n"
             << scope_tab << "public " << string << "Concrete()\n"
             << scope_tab << "{\n"
             << scope_tab << scope_tab << "System.IntPtr klass = " << class_get_name << "();\n"
             << scope_tab << scope_tab << "System.IntPtr parent = System.IntPtr.Zero;\n"
             << scope_tab << scope_tab << "System.IntPtr eo = _efl_add_internal_start(\"file\", 0, klass, parent, 0, 0);\n"
             << scope_tab << scope_tab << "handle = _efl_add_end(eo, 0, 0);\n"
             << scope_tab << "}\n"
            )
            .generate(sink, std::make_tuple(cls.cxx_name, cls.cxx_name, cls.namespaces, cls.eolian_name, cls.cxx_name, cls.namespaces, cls.eolian_name), context))
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
       }
     
     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, cpp_namespaces, context)) return false;
     
     return true;
   }
};

struct klass const klass = {};

}

namespace efl { namespace eolian { namespace grammar {
  
template <>
struct is_eager_generator<struct ::eolian_mono::klass> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<struct ::eolian_mono::klass> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
