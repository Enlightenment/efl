#ifndef EOLIAN_MONO_ENUM_DEFINITION_HH
#define EOLIAN_MONO_ENUM_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "name_helpers.hh"
#include "using_decl.hh"

namespace eolian_mono {

struct enum_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::enum_def const& enum_, Context const& context) const
  {
     if(!name_helpers::open_namespaces(sink, enum_.namespaces, context))
       return false;

     if(!as_generator(documentation).generate(sink, enum_, context))
       return false;

     if(!as_generator
        (
         "public enum " << string << "\n{\n"
         )
        .generate(sink, name_helpers::typedecl_managed_name(enum_), context))
       return false;

     // iterate enum fiels
     for(auto first = std::begin(enum_.fields)
             , last = std::end(enum_.fields); first != last; ++first)
       {
          auto name = name_helpers::enum_field_managed_name((*first).name);
          auto literal = (*first).value.literal;
          if (!as_generator
              (
               documentation << string << " = " << string << ",\n"
              )
              .generate(sink, std::make_tuple(*first, name, literal), context))
            return false;
       }

     if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;

     if(!name_helpers::close_namespaces(sink, enum_.namespaces, context))
       return false;

     return true;
  }
};

enum_definition_generator const enum_definition = {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::enum_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::enum_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::enum_definition_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
