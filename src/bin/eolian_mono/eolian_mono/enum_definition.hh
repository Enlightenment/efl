#ifndef EOLIAN_MONO_ENUM_DEFINITION_HH
#define EOLIAN_MONO_ENUM_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "keyword.hh"
#include "using_decl.hh"

namespace eolian_mono {

struct enum_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::enum_def const& enum_, Context const& context) const
  {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(enum_.namespaces);

     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate(sink, cpp_namespaces, add_lower_case_context(context))) return false;

     if(!as_generator
        (
         "public enum " << string << "\n{\n"
         )
        .generate(sink, enum_.cxx_name, context))
       return false;

     // iterate enum fiels
     for(auto first = std::begin(enum_.fields)
             , last = std::end(enum_.fields); first != last; ++first)
       {
          auto name = (*first).name;
          name[0] = std::toupper(name[0]); // Hack to allow 'static' as a field name
          if (!as_generator
              (
               string << ",\n"
              )
              .generate(sink, name, context))
            return false;
       }

     if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;

     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, cpp_namespaces, context)) return false;
     
     return true;
  }
};

enum_definition_generator const enum_definition = {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::enum_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::enum_definition_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
