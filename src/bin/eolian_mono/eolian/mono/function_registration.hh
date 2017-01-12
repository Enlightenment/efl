#ifndef EOLIAN_MONO_FUNCTION_REGISTRATION_HH
#define EOLIAN_MONO_FUNCTION_REGISTRATION_HH

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

template <typename I>
struct function_registration_generator
{
  I index_generator;
  attributes::klass_def const* klass;
  
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    if(is_function_blacklisted(f.c_name))
      return true;
    else
      {
    auto index = index_generator();
    
    if(!as_generator
       (scope_tab << scope_tab << "descs[" << index << "].api_func = efl.eo.Globals.dlsym(IntPtr.Zero, \"" << string << "\");\n"
        << scope_tab << scope_tab << "descs[" << index << "].func = Marshal.GetFunctionPointerForDelegate(" << string << "NativeInherit." << string << "_static_delegate);\n"
       )
       .generate(sink, std::make_tuple(f.c_name, klass->cxx_name, escape_keyword(f.name)), context))
      return false;
    return true;
      }
  }
};
  
struct function_registration_parameterized
{
  template <typename I>
  function_registration_generator<I> operator()(I i, attributes::klass_def const& klass) const
  {
    return {i, &klass};
  }
} const function_registration;

}

namespace efl { namespace eolian { namespace grammar {

template <typename I>
struct is_eager_generator< ::eolian_mono::function_registration_generator<I>> : std::true_type {};
template <typename I>
struct is_generator< ::eolian_mono::function_registration_generator<I>> : std::true_type {};

namespace type_traits {

template <typename I>
struct attributes_needed< ::eolian_mono::function_registration_generator<I>> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
