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
    if(blacklist::is_function_blacklisted(f, context) || f.is_static) // Static methods aren't overrideable
      return true;
    else
      {
    // auto index = index_generator();

    if(!as_generator(
            scope_tab << scope_tab << f.c_name << "_static_delegate = new " << f.c_name << "_delegate(" <<
                escape_keyword(f.name) << ");\n"
        ).generate(sink, attributes::unused, context))
      return false;

    if(!as_generator
       (scope_tab << scope_tab << "descs.Add(new Efl_Op_Description() {"
#ifdef _WIN32
        << "api_func = Marshal.StringToHGlobalAnsi(\"" << string << "\")"
#else
        << "api_func = Efl.Eo.Globals.dlsym(Efl.Eo.Globals.RTLD_DEFAULT, \"" << string << "\")"
#endif
        ", func = Marshal.GetFunctionPointerForDelegate(" << string << "_static_delegate)});\n"
       )
       .generate(sink, std::make_tuple(f.c_name, f.c_name), context))
      return false;
    return true;
      }
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
