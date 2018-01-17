#ifndef EOLIAN_JS_STUB_FUNCTION_DEFINITION_HPP
#define EOLIAN_JS_STUB_FUNCTION_DEFINITION_HPP

#include <grammar/klass_def.hpp>
#include <grammar/indentation.hpp>
#include <grammar/string.hpp>
#include <grammar/attribute_reorder.hpp>
#include <eolian/grammar/parameter.hpp>
#include <eolian/grammar/out_parameter_definition.hpp>

namespace eolian { namespace js { namespace grammar {

namespace attributes = efl::eolian::grammar::attributes;

struct stub_function_definition_generator
{
  stub_function_definition_generator(attributes::klass_def klass)
    : klass(klass) {}
  
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    using namespace efl::eolian::grammar;
    using efl::eolian::grammar::attributes::unused;

    if(f.is_beta || f.is_protected)
      return true;

     // std::string suffix;
     // switch(cls.type)
     //   {
     //   case attributes::class_type::regular:
     //   case attributes::class_type::abstract_:
     //     suffix = "CLASS";
     //     break;
     //   case attributes::class_type::mixin:
     //     suffix = "MIXIN";
     //     break;
     //   case attributes::class_type::interface_:
     //     suffix = "INTERFACE";
     //     break;
     //   }

    std::size_t ins = 0;
    for(auto&& parameters : f.parameters)
      {
        if(parameters.direction == attributes::parameter_direction::in)
          ++ins;
      }
    
    as_generator
      (
       attribute_reorder<0, 1, 2, 3, 4, 3>
       (
        "eina::js::compatibility_return_type stub_function_"
       << lower_case[*(string << "_")]
       << lower_case[string]
       << "_"
       << lower_case[string]
       << "(eina::js::compatibility_callback_info_type args)\n"
       << "{\n"
        << scope_tab << "if(/*input_parameters*/" << std::to_string(ins) << " == args.Length())\n"
       << scope_tab << "{\n"
       << scope_tab(2) << "v8::Isolate* isolate = args.GetIsolate(); static_cast<void>(isolate);\n"
       << scope_tab(2) << "v8::Local<v8::Object> self = args.This();\n"
       << scope_tab(2) << "v8::Local<v8::Value> external = self->GetInternalField(0);\n"
       << scope_tab(2) << "Eo* eo = static_cast<Eo*>(v8::External::Cast(*external)->Value());\n"
       << scope_tab(2) << "try\n"
       << scope_tab(2) << "{\n"
       << *(out_parameter_definition(klass, f.parameters)[scope_tab(3)])
       << scope_tab(3) << "::" << lower_case[string]
       << "("
       << "eo"
       << *(", " << parameter(klass, f.parameters))
       << ")"
       << ";\n"
       << scope_tab(2) << "}\n"
       << scope_tab(2) << "catch(std::logic_error const&)\n"
       << scope_tab(2) << "{\n"
       << scope_tab(3) << "return eina::js::compatibility_return();\n"
       << scope_tab(2) << "}\n"
       << scope_tab << "}\n"
       << scope_tab << "else\n"
       << scope_tab << "{\n"
       << scope_tab(2) << "return eina::js::compatibility_throw\n"
       << scope_tab(3) << "(v8::Exception::TypeError\n"
       << scope_tab(4) << "(eina::js::compatibility_new<v8::String>(nullptr, \"Expected more arguments for this call\")));\n"
       << scope_tab << "}\n"
       << "}\n"
        )).generate(sink, std::make_tuple(klass.namespaces, klass.cxx_name, f.name, f.parameters
                                          , f.c_name), context);
    
    // methods

    // events
    
    return true;
  }

  attributes::klass_def klass;
};


struct stub_function_definition_generator_
{
  stub_function_definition_generator operator()(attributes::klass_def const& klass) const
  {
    return {klass};
  }
} const stub_function_definition = {};
      
} } }

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_generator< ::eolian::js::grammar::stub_function_definition_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian::js::grammar::stub_function_definition_generator> : std::true_type {};

namespace type_traits {

template <>
struct attributes_needed< ::eolian::js::grammar::stub_function_definition_generator> : std::integral_constant<int, 1> {};
template <>
struct accepts_specific_tuple< ::eolian::js::grammar::stub_function_definition_generator
                               , ::efl::eolian::grammar::attributes::function_def> : std::true_type {};

      
} } } }

#endif
