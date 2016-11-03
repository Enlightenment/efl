#ifndef EOLIAN_MONO_FUNCTION_DEFINITION_HH
#define EOLIAN_MONO_FUNCTION_DEFINITION_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/attribute_reorder.hpp"
#include "type.hh"
#include "marshall_type.hh"
#include "parameter.hh"
#include "keyword.hh"
#include "using_decl.hh"
#include "library_context.hh"

namespace eolian_mono {

struct native_function_definition_generator
{
  attributes::klass_def const* klass;
  
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    if(is_function_blacklisted(f.c_name))
      return true;
    else
      {
    if(!as_generator
       (scope_tab
        << eolian_mono::marshall_annotation(true)
        << " public delegate "
        << eolian_mono::marshall_type(true)
        << " "
        << string
        << "_delegate(System.IntPtr obj, System.IntPtr pd"
        << *grammar::attribute_reorder<1, -1>
        (
         (", " << marshall_annotation << " " << marshall_parameter)
        )
        << ");\n")
       .generate(sink, std::make_tuple(f.return_type, f.return_type, escape_keyword(f.name), f.parameters), context))
      return false;

    std::string return_type;
    if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
      return false;
    
    if(!as_generator
       (scope_tab
        << eolian_mono::marshall_annotation(true)
        << " public static "
        << eolian_mono::marshall_type(true) << " "
        << string
        << "(System.IntPtr obj, System.IntPtr pd"
        << *grammar::attribute_reorder<1, -1>
        (
         (", " << marshall_annotation << " " << marshall_parameter)
        )
        << ")\n"
        << scope_tab << "{\n"
        /****/
        << scope_tab << scope_tab << "System.Console.WriteLine(\"function " << string << " was called\");\n"
        /****/
        << scope_tab << scope_tab << "efl.eo.IWrapper wrapper = efl.eo.Globals.data_get(pd);\n"
        << scope_tab << scope_tab << "if(wrapper != null)\n"
        << scope_tab << scope_tab << scope_tab << (return_type != "void" ? "return " : "") << "((" << string << "Inherit)wrapper)." << string
        << "(" << (argument % ", ") << ");\n"
        << scope_tab << scope_tab << "else\n"
        << scope_tab << scope_tab << scope_tab << (return_type != "void" ? "return " : "") << string << "Inherit." << string
        << "(efl.eo.Globals.efl_super(obj, " << string << "Inherit.klass)" << *(", " << argument) << ");\n"
        << scope_tab << "}\n"
       )
       .generate(sink, std::make_tuple(f.return_type, f.return_type, escape_keyword(f.name), f.parameters
                                       , /***/f.c_name/***/
                                       , klass->cxx_name, escape_keyword(f.name)
                                       , f.parameters
                                       , klass->cxx_name, f.c_name, klass->cxx_name, f.parameters), context))
      return false;

    if(!as_generator
       (scope_tab << "public static  "
        << string
        << "_delegate "
        << string << "_static_delegate = new " << string << "_delegate(" << string << "NativeInherit." << string << ");\n"
       )
       .generate(sink, std::make_tuple(escape_keyword(f.name), escape_keyword(f.name), escape_keyword(f.name), escape_keyword(klass->cxx_name)
                                       , escape_keyword(f.name)), context))
      return false;
    return true;
      }
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
    if(is_function_blacklisted(f.c_name))
      return true;
    else
      {
    if(!as_generator
       (scope_tab << "[System.Runtime.InteropServices.DllImport(\"" << context_find_tag<library_context>(context).library_name << "\")]\n"
        << scope_tab << eolian_mono::marshall_annotation(true)
        << " public static extern "
        << eolian_mono::marshall_type(true)
        << " " << string
        << "(System.IntPtr obj"
        << *grammar::attribute_reorder<1, -1>
        (
         (", " << marshall_annotation << " " << marshall_parameter)
        )
        << ");\n")
       .generate(sink, std::make_tuple(f.return_type, f.return_type, f.c_name, f.parameters), context))
      return false;

    std::string return_type;
    if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
      return false;
    
    if(!as_generator
       (scope_tab << (do_super ? "virtual " : "") << "public " << return_type << " " << string << "(" << (parameter % ", ")
        << ") { "
        << (return_type == "void" ? "":"return ") << string << "("
        << (do_super ? "efl.eo.Globals.efl_super(" : "")
        << "this.raw_handle"
        << (do_super ? ", this.raw_klass)" : "")
        << *(", " << argument) << ");"
        << " }\n")
       .generate(sink, std::make_tuple(escape_keyword(f.name), f.parameters, f.c_name, f.parameters), context))
      return false;

    return true;
      }
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

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::function_definition_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::native_function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_definition_parameterized> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::function_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::function_definition_parameterized> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::native_function_definition_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
