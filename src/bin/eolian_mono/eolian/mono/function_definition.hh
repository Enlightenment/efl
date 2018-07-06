#ifndef EOLIAN_MONO_FUNCTION_DEFINITION_HH
#define EOLIAN_MONO_FUNCTION_DEFINITION_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/attribute_reorder.hpp"
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
    if(blacklist::is_function_blacklisted(f.c_name) || f.is_static) // Only Concrete classes implement static methods.
      return true;
    else
      {
    if(!as_generator
       ("\n\n" << scope_tab
        << eolian_mono::marshall_native_annotation(true)
        << " private delegate "
        << eolian_mono::marshall_type(true)
        << " "
        << string
        << "_delegate(System.IntPtr obj, System.IntPtr pd"
        << *grammar::attribute_reorder<-1, -1>
        (
         (", " << marshall_native_annotation << " " << marshall_parameter)
        )
        << ");\n")
       .generate(sink, std::make_tuple(f.return_type, f.return_type, f.c_name, f.parameters), context))
      return false;

    if(!as_generator
       (scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(context).actual_library_name(f.filename) << ")] "
        << eolian_mono::marshall_native_annotation(true)
        << " private static extern "
        << eolian_mono::marshall_type(true)
        << " " << string
        << "(System.IntPtr obj"
        << *grammar::attribute_reorder<-1, -1>
        (
         (", " << marshall_native_annotation << " " << marshall_parameter)
        )
        << ");\n")
       .generate(sink, std::make_tuple(f.return_type, f.return_type, f.c_name, f.parameters), context))
      return false;

    std::string return_type;
    if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
      return false;

    std::string klass_inherit_name = name_helpers::klass_inherit_name(*klass);

    if(!as_generator
       (scope_tab
        << " private static "
        << eolian_mono::marshall_type(true) << " "
        << string
        << "(System.IntPtr obj, System.IntPtr pd"
        << *(", " << marshall_parameter)
        << ")\n"
        << scope_tab << "{\n"
        /****/
        << scope_tab << scope_tab << "eina.Log.Debug(\"function " << string << " was called\");\n"
        /****/
        << scope_tab << scope_tab << "efl.eo.IWrapper wrapper = efl.eo.Globals.data_get(pd);\n"
        << scope_tab << scope_tab << "if(wrapper != null) {\n"
        << scope_tab << scope_tab << scope_tab << eolian_mono::native_function_definition_preamble()
        << scope_tab << scope_tab << scope_tab << "try {\n"
        << scope_tab << scope_tab << scope_tab << scope_tab << (return_type != " void" ? "_ret_var = " : "") << "((" << klass_inherit_name << ")wrapper)." << string
        << "(" << (native_argument_invocation % ", ") << ");\n"
        << scope_tab << scope_tab << scope_tab << "} catch (Exception e) {\n"
        << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Log.Warning($\"Callback error: {e.ToString()}\");\n"
        << scope_tab << scope_tab << scope_tab << scope_tab << "eina.Error.Set(eina.Error.EFL_ERROR);\n"
        << scope_tab << scope_tab << scope_tab << "}\n"
        << eolian_mono::native_function_definition_epilogue(*klass)
        << scope_tab << scope_tab << "} else {\n"
        << scope_tab << scope_tab << scope_tab << (return_type != " void" ? "return " : "") << string
        << "(efl.eo.Globals.efl_super(obj, " << klass_inherit_name << ".klass)" << *(", " << argument) << ");\n"
        << scope_tab << scope_tab << "}\n"
        << scope_tab << "}\n"
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

    if(!as_generator
       (scope_tab << "private static  "
        << string
        << "_delegate "
        << string << "_static_delegate = new " << string << "_delegate(" << name_helpers::klass_native_inherit_name(*klass) << "." << string << ");\n"
       )
       .generate(sink, std::make_tuple(f.c_name, f.c_name, f.c_name, escape_keyword(f.name)), context))
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
    EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "function_definition_generator: " << f.c_name << std::endl;
    if(do_super && f.is_static) // Static methods goes only on Concrete classes.
      return true;
    if(blacklist::is_function_blacklisted(f.c_name))
      return true;

    if(!as_generator
       ("\n\n" << scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(context).actual_library_name(f.filename) << ")]\n"
        << scope_tab << eolian_mono::marshall_annotation(true)
        << " private static extern "
        << eolian_mono::marshall_type(true)
        << " " << string
        << "(System.IntPtr obj"
        << *grammar::attribute_reorder<-1, -1>
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
       (documentation(1)).generate(sink, f, context))
      return false;

    if(!as_generator
       (scope_tab << (do_super ? "virtual " : "") << "public " << (f.is_static ? "static " : "") << return_type << " " << string << "(" << (parameter % ", ")
        << ") {\n "
        << eolian_mono::function_definition_preamble() << string << "("
        << (do_super ? "efl.eo.Globals.efl_super(" : "")
        << (f.is_static ? name_helpers::klass_get_full_name(f.klass) + "()": "this.raw_handle")
        << (do_super ? ", this.raw_klass)" : "")
        << *(", " << argument_invocation ) << ");\n"
        << eolian_mono::function_definition_epilogue()
        << " }\n")
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
