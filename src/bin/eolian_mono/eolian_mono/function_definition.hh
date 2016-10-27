#ifndef EOLIAN_MONO_FUNCTION_DEFINITION_HH
#define EOLIAN_MONO_FUNCTION_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
//#include "grammar/parameter.hpp"
#include "keyword.hh"
#include "using_decl.hh"
#include "library_context.hh"

namespace eolian_mono {

struct function_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    //using namespace System;
    //using namespace System::Runtime::InteropServices;
    std::string return_type;
    if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
      return false;
    
    if(!as_generator
       (scope_tab << "[System.Runtime.InteropServices.DllImport(\"" << context_find_tag<library_context>(context).library_name << "\")] static extern "
        << return_type << " "
        << string
        << "(" /*<< (parameter % ", ")*/ << ");\n")
       .generate(sink, std::make_tuple(f.c_name/*, f.parameters*/), context))
      return false;

    if(!as_generator
       (scope_tab << "public " << return_type << " " << string << "(" /*<< (parameter % ", ")*/
        << ") { "
        << (return_type == "void" ? "":"return ") << string << "();"
        << " }\n")
       .generate(sink, std::make_tuple(escape_keyword(f.name), f.c_name/*, f.parameters*/), context))
      return false;

    return true;
  }
};

function_definition_generator const function_definition = {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::function_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::function_definition_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
