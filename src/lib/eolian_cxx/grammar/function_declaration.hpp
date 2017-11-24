#ifndef EOLIAN_CXX_FUNCTION_DECLARATION_HH
#define EOLIAN_CXX_FUNCTION_DECLARATION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/keyword.hpp"

namespace efl { namespace eolian { namespace grammar {

struct function_declaration_generator
{
   function_declaration_generator(attributes::klass_name const& name)
      : _klass_name(name)
   {}

   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::function_def const& f, Context const& ctx) const
   {
      std::string suffix;
      switch(_klass_name.type)
        {
        case attributes::class_type::regular:
        case attributes::class_type::abstract_:
          suffix = "CLASS";
          break;
        case attributes::class_type::mixin:
         suffix = "MIXIN";
         break;
        case attributes::class_type::interface_:
          suffix = "INTERFACE";
          break;
        }

      if(f.is_beta &&
            !as_generator("#ifdef " << *(string << "_") << string << "_" << string << "_BETA\n")
            .generate(sink, std::make_tuple(_klass_name.namespaces, _klass_name.eolian_name, suffix), add_upper_case_context(ctx)))
        return false;
      if(!as_generator
            ("::efl::eolian::return_traits<" << grammar::type(true) << ">::type " << string << "(" << (parameter % ", ") << ") const;\n")
            .generate(sink, std::make_tuple(f.return_type, escape_keyword(f.name), f.parameters), ctx))
        return false;
      if(f.is_beta &&
            !as_generator("#endif\n").generate(sink, attributes::unused, ctx))
        return false;
      return true;
   }

   attributes::klass_name _klass_name;
};

template <>
struct is_eager_generator<function_declaration_generator> : std::true_type {};
template <>
struct is_generator<function_declaration_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<function_declaration_generator> : std::integral_constant<int, 1> {};
}

struct function_declaration_terminal
{
  function_declaration_generator operator()(attributes::klass_name name) const
  {
    return function_declaration_generator{name};
  }
} const function_declaration = {};
      
} } }

#endif
