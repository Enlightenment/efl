#ifndef EOLIAN_MONO_STRUCT_DEFINITION_HH
#define EOLIAN_MONO_STRUCT_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "keyword.hh"
#include "using_decl.hh"

namespace eolian_mono {

// Blacklist structs that require some kind of manual binding.
static bool is_struct_blacklisted(attributes::struct_def const& struct_)
{
  std::string full_name;

  for (auto it=struct_.namespaces.begin(); it != struct_.namespaces.end(); it++)
    {
       full_name += *it + ".";
    }

  full_name += struct_.cxx_name;
  return full_name == "Efl.Event.Description"
      || full_name == "Eina.File"
      || full_name == "Eina.Binbuf"
      || full_name == "Eina.Slice"
      || full_name == "Eina.Rw_Slice";
}

struct struct_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     if (is_struct_blacklisted(struct_))
         return true;

     std::vector<std::string> cpp_namespaces = escape_namespace(attributes::cpp_namespaces(struct_.namespaces));

     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate(sink, cpp_namespaces, add_lower_case_context(context))) return false;

     if(!as_generator
        (
         "public struct " << string << "\n{\n"
         )
        .generate(sink, struct_.cxx_name, context))
       return false;

     // iterate enum fiels
     for(auto first = std::begin(struct_.fields)
             , last = std::end(struct_.fields); first != last; ++first)
       {
          auto field_name = (*first).name;
          auto field_type = (*first).type;
          field_name[0] = std::toupper(field_name[0]); // Hack to allow 'static' as a field name
          if (!as_generator
              (
               "public " << type << " " << string << ";\n"
              )
              .generate(sink, std::make_tuple(field_type, field_name), context))
            return false;
       }

     if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;

     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, cpp_namespaces, context)) return false;

     return true;
  }
};

struct_definition_generator const struct_definition = {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::struct_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::struct_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::struct_definition_generator> : std::integral_constant<int, 1> {};
}

} } }

#endif
