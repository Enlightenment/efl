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

    std::string klass_cast_name;
    if (klass->type != attributes::class_type::interface_)
      klass_cast_name = name_helpers::klass_inherit_name(*klass);
    else
      klass_cast_name = name_helpers::klass_interface_name(*klass);

    if(!as_generator
       (scope_tab
        << " private "
        << eolian_mono::marshall_type(true) << " "
        << string
        << "(System.IntPtr obj, System.IntPtr pd"
        << *(", " << marshall_parameter)
        << ")\n"
        << scope_tab << "{\n"
        /****/
        << scope_tab << scope_tab << "Eina.Log.Debug(\"function " << string << " was called\");\n"
        /****/
        << scope_tab << scope_tab << "Efl.Eo.IWrapper wrapper = Efl.Eo.Globals.data_get(pd);\n"
        << scope_tab << scope_tab << "if(wrapper != null) {\n"
        << scope_tab << scope_tab << scope_tab << eolian_mono::native_function_definition_preamble()
        << scope_tab << scope_tab << scope_tab << "try {\n"
        << scope_tab << scope_tab << scope_tab << scope_tab << (return_type != " void" ? "_ret_var = " : "") << "((" << klass_cast_name << ")wrapper)." << string
        << "(" << (native_argument_invocation % ", ") << ");\n"
        << scope_tab << scope_tab << scope_tab << "} catch (Exception e) {\n"
        << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Log.Warning($\"Callback error: {e.ToString()}\");\n"
        << scope_tab << scope_tab << scope_tab << scope_tab << "Eina.Error.Set(Eina.Error.UNHANDLED_EXCEPTION);\n"
        << scope_tab << scope_tab << scope_tab << "}\n"
        << eolian_mono::native_function_definition_epilogue(*klass)
        << scope_tab << scope_tab << "} else {\n"
        << scope_tab << scope_tab << scope_tab << (return_type != " void" ? "return " : "") << string
        << "(Efl.Eo.Globals.efl_super(obj, " << "GetEflClass())" << *(", " << argument) << ");\n"
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

    // This is the delegate that will be passed to Eo to be called from C.
    if(!as_generator(
            scope_tab << "private " << f.c_name << "_delegate " << f.c_name << "_static_delegate;\n"
        ).generate(sink, attributes::unused, context))
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
    if(blacklist::is_function_blacklisted(f.c_name))
      return true;

    if(!as_generator
       ("\n\n" << scope_tab << "[System.Runtime.InteropServices.DllImport(" << context_find_tag<library_context>(context).actual_library_name(f.filename) << ")]\n"
        << scope_tab << eolian_mono::marshall_annotation(true)
        << (do_super ? " protected " : " private ") << "static extern "
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

    std::string self = "this.NativeHandle";

    // inherited is set in the constructor, true if this instance is from a pure C# class (not generated).
    if (do_super && !f.is_static)
      self = "(inherited ? Efl.Eo.Globals.efl_super(" + self + ", this.NativeClass) : " + self + ")";
    else
      self = name_helpers::klass_get_full_name(f.klass) + "()";

    if(!as_generator
       (scope_tab << ((do_super && !f.is_static) ? "virtual " : "") << "public " << (f.is_static ? "static " : "") << return_type << " " << string << "(" << (parameter % ", ")
        << ") {\n "
        << eolian_mono::function_definition_preamble() << string << "("
        << self
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

struct property_wrapper_definition_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::property_def const& property, Context context) const
   {
      if (blacklist::is_property_blacklisted(property))
        return true;

      bool interface = context_find_tag<class_context>(context).current_wrapper_kind == class_context::interface;
      bool is_static = (property.getter.is_engaged() && property.getter->is_static)
                       || (property.setter.is_engaged() && property.setter->is_static);


      if (interface && is_static)
        return true;

      auto get_params = property.getter.is_engaged() ? property.getter->parameters.size() : 0;
      auto set_params = property.setter.is_engaged() ? property.setter->parameters.size() : 0;

      // C# properties must have a single value.
      //
      // Single values in getters are automatically converted to return_type,
      // meaning they should have 0 parameters.
      //
      // For setters, we ignore the return type - usually boolean.
      if (get_params > 0 || set_params > 1)
        return true;

      attributes::type_def prop_type;

      if (property.getter.is_engaged())
        prop_type = property.getter->return_type;
      else if (property.setter.is_engaged())
        prop_type = property.setter->parameters[0].type;
      else
        {
           EINA_CXX_DOM_LOG_ERR(eolian_mono::domain) << "Property must have either a getter or a setter." << std::endl;
           return false;
        }

      std::string dir_mod;
      if (property.setter.is_engaged())
        dir_mod = direction_modifier(property.setter->parameters[0]);

      std::string managed_name = name_helpers::property_managed_name(property);

      if (!as_generator(
                  scope_tab << documentation
                  << scope_tab << (interface ? "" : "public ") << (is_static ? "static " : "") << type(true) << " " << managed_name << " {\n"
            ).generate(sink, std::make_tuple(property, prop_type), context))
        return false;

      if (property.getter.is_engaged())
        if (!as_generator(scope_tab << scope_tab << "get " << (interface ? ";" : "{ return Get" + managed_name + "(); }") << "\n"
            ).generate(sink, attributes::unused, context))
          return false;

      if (property.setter.is_engaged())
        if (!as_generator(scope_tab << scope_tab << "set " << (interface ? ";" : "{ Set" + managed_name + "(" + dir_mod + "value); }") << "\n"
            ).generate(sink, attributes::unused, context))
          return false;

      if (!as_generator(scope_tab << "}\n").generate(sink, attributes::unused, context))
        return false;

      return true;
   }
} const property_wrapper_definition;

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::function_definition_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::native_function_definition_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian_mono::property_wrapper_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_definition_parameterized> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::property_wrapper_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::function_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::function_definition_parameterized> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::native_function_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::property_wrapper_definition_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
