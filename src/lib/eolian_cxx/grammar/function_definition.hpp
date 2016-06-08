#ifndef EOLIAN_CXX_FUNCTION_DEFINITION_HH
#define EOLIAN_CXX_FUNCTION_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"
#include "grammar/converting_argument.hpp"
#include "grammar/case.hpp"
#include "grammar/keyword.hpp"
#include "grammar/attribute_conditional.hpp"
#include "grammar/attribute_reorder.hpp"
#include "grammar/type_impl.hpp"

namespace efl { namespace eolian { namespace grammar {

struct function_definition_generator
{
   function_definition_generator(attributes::klass_name const& name)
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
      if(f.is_protected &&
         !as_generator("#ifdef " << *(string << "_") << string << "_" << string << "_PROTECTED\n")
         .generate(sink, std::make_tuple(_klass_name.namespaces, _klass_name.eolian_name, suffix), add_upper_case_context(ctx)))
        return false;
     
      if(!as_generator
         ("inline " << grammar::type(true) << " " << string << "::" << string << "(" << (parameter % ", ") << ") const\n{\n")
         .generate(sink, std::make_tuple(f.return_type, _klass_name.eolian_name, escape_keyword(f.name), f.parameters), ctx))
        return false;

      auto out_declaration =
        attribute_conditional([] (attributes::parameter_def const& p) -> bool
                              { return p.direction == attributes::parameter_direction::out; })
        [
          attribute_reorder<1, 2>
          (scope_tab << c_type << " __out_param_" << string << " = {};\n")
        ]
        | attribute_conditional([] (attributes::parameter_def const& p) -> bool
                                { return p.direction == attributes::parameter_direction::inout; })
        [
          attribute_reorder<1, 2, 1, 1, 2>
          (scope_tab << c_type << " __out_param_" << string << " = ::efl::eolian::convert_inout<" << c_type
           << ", " << type << ">(" << string << ");\n")
        ]
        | eps
        ;
      if(!as_generator(*(out_declaration))
         .generate(sink, f.parameters, ctx)) return false;
      
      if(!as_generator(scope_tab).generate(sink, attributes::unused, ctx)) return false;
      
      if(f.return_type != attributes::void_
         && !as_generator(attributes::c_type({attributes::parameter_direction::in, f.return_type, "", f.return_type.c_type})
                          << " __return_value = "
                          ).generate(sink, attributes::unused, ctx)) return false;
      
      if(!as_generator
         (" ::" << string << "(this->_eo_ptr()"
          <<
          *(
            "\n" << scope_tab << scope_tab << ", "
            <<
            (
             attribute_conditional([] (attributes::parameter_def const& p)
             { return p.direction == attributes::parameter_direction::in; })
             [converting_argument]
             | ("& __out_param_" << attribute_reorder<2>(string))
            )
          )
          << ");\n"
         ).generate(sink, std::make_tuple(f.c_name, f.parameters), ctx))
        return false;

      auto out_assignments =
        attribute_conditional([] (attributes::parameter_def const& p) -> bool
                              { return p.direction != attributes::parameter_direction::in; })
        [
          attribute_reorder<-1, 1, 1, 2, 2>
          (
            scope_tab << "::efl::eolian::assign_out<" << parameter_type << ", " << c_type
            <<
            (
             attribute_conditional([] (attributes::type_def const& type)
             { return type.original_type.visit(attributes::get_qualifier_visitor{}) & qualifier_info::is_own; })
             [
               ", true"
             ] | eps
            )
            << ">(" << string << ", __out_param_" << string << ");\n"
           )
        ]
        | eps
        ;
      if(!as_generator(*(out_assignments))
         .generate(sink, f.parameters, ctx)) return false;
      
      if(f.return_type != attributes::void_
         && !as_generator(scope_tab << "return ::efl::eolian::convert_to_return<"
                          << type(true) << ">(__return_value);\n"
                          ).generate(sink, f.return_type, ctx)) return false;

      if(!as_generator("}\n").generate(sink, attributes::unused, ctx))
        return false;

      if(f.is_beta &&
         !as_generator("#endif\n").generate(sink, attributes::unused, ctx))
        return false;
      if(f.is_protected &&
         !as_generator("#endif\n").generate(sink, attributes::unused, ctx))
        return false;
      return true;
   }

   attributes::klass_name _klass_name;
};

template <>
struct is_eager_generator<function_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<function_definition_generator> : std::integral_constant<int, 1> {};
}
      
struct function_definition_terminal
{
  function_definition_generator operator()(attributes::klass_name name) const
  {
    return function_definition_generator{name};
  }
} const function_definition = {};
      
} } }

#endif
