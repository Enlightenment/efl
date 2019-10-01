#ifndef EOLIAN_MONO_FUNCTION_REGISTRATION_HH
#define EOLIAN_MONO_FUNCTION_REGISTRATION_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/attribute_reorder.hpp"
#include "logging.hh"
#include "type.hh"
#include "marshall_type.hh"
#include "parameter.hh"
#include "using_decl.hh"
#include "generation_contexts.hh"
#include "blacklist.hh"

namespace eolian_mono {

// template <typename I>
struct function_registration_generator
{
  // I index_generator;
  attributes::klass_def const* klass;
  
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "function_registration_generator: " << f.name << std::endl;
    auto const& indent = current_indentation(context);

    if(blacklist::is_function_blacklisted(f, context) || f.is_static) // Static methods aren't overrideable
      return true;

    // We do not generate registration wrappers for non public interface/mixin members in their concrete classes.
    // They go in the first concrete/abstract implementation.
    if(blacklist::is_non_public_interface_member(f, *klass))
      return true;

    if(!as_generator(
               indent << "if (" << f.c_name << "_static_delegate == null)\n"
               << indent << "{\n"
               << indent << scope_tab << f.c_name << "_static_delegate = new " << f.c_name << "_delegate(" << escape_keyword(f.name) << ");\n"
               << indent << "}\n\n"
        ).generate(sink, attributes::unused, context))
      return false;

    if(!as_generator(
        indent << "if (methods.FirstOrDefault(m => m.Name == \"" << string << "\") != null)\n"
        << indent << "{\n"
        << indent << scope_tab << "descs.Add(new Efl_Op_Description() {"
#ifdef _WIN32
        << "api_func = Marshal.StringToHGlobalAnsi(\"" << string << "\")"
#else
        << "api_func = Efl.Eo.FunctionInterop.LoadFunctionPointer(Module.Module, \"" << string << "\")"
#endif
        << ", func = Marshal.GetFunctionPointerForDelegate(" << string << "_static_delegate) });\n"
        << indent << "}\n\n"
       )
       .generate(sink, std::make_tuple(name_helpers::managed_method_name(f), f.c_name, f.c_name), context))
      return false;
    return true;
  }
};
  
struct function_registration_parameterized
{
  function_registration_generator operator()(attributes::klass_def const& klass) const
  {
    return {&klass};
  }
} const function_registration;

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::function_registration_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_registration_generator> : std::true_type {};

namespace type_traits {

template <>
struct attributes_needed< ::eolian_mono::function_registration_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
