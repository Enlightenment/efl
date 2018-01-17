#ifndef EOLIAN_JS_OUT_PARAMETER_DEFINITION_HPP
#define EOLIAN_JS_OUT_PARAMETER_DEFINITION_HPP

#include <grammar/klass_def.hpp>
#include <grammar/indentation.hpp>
#include <grammar/string.hpp>
#include <grammar/attribute_reorder.hpp>
#include <grammar/attributed.hpp>
#include <grammar/attribute_replace.hpp>
#include <eolian/grammar/stub_function_definition.hpp>

namespace eolian { namespace js { namespace grammar {

namespace attributes = efl::eolian::grammar::attributes;

template <typename ParameterContainer, typename Generator>
struct out_parameter_definition_generator
{
  out_parameter_definition_generator(attributes::klass_def const& cls, ParameterContainer const& container
                                     , Generator prefix_generator)
    : klass(cls), container(container), prefix_generator(prefix_generator) {}
  
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
  {
    using namespace efl::eolian::grammar;
    using efl::eolian::grammar::attributes::unused;
    namespace attributes = efl::eolian::grammar::attributes;

    std::size_t index;
    {
      auto iterator = std::find(container.begin(), container.end(), param);
      assert(iterator != container.end());
      index = std::distance(container.begin(), iterator);
    }

    switch(param.direction)
      {
      case attributes::parameter_direction::in:
        break;
      case attributes::parameter_direction::inout:
      case attributes::parameter_direction::out:
        attributes::generate(prefix_generator, sink, param, context);
        
        as_generator(string
                     << " __out_"
                     << string
                     ).generate(sink, std::make_tuple(param.type.c_type, param.param_name), context);

        if(param.direction == attributes::parameter_direction::inout)
          as_generator(" = ::efl::eina::js::get_value_from_javascript(args["
                       << string
                       << "], isolate, \"\", ::efl::eina::js::value_tag< "
                       << string
                       << " >{})"
                       ).generate(sink, std::make_tuple(std::to_string(index), param.type.c_type), context);

        *sink++ = ';';
        *sink++ = '\n';
        break;
      };
    
    
    return true;
  }

  attributes::klass_def klass;
  ParameterContainer const& container;
  Generator prefix_generator;
};

template <typename ParameterContainer>
struct out_parameter_definition_directive
{
  template <typename Generator>
  out_parameter_definition_generator<ParameterContainer, Generator>
  operator[](Generator const& gen) const
  {
    return {klass, container, gen};
  }

  attributes::klass_def klass;
  ParameterContainer const& container;
};

template <typename ParameterContainer>
out_parameter_definition_directive<ParameterContainer>
out_parameter_definition(attributes::klass_def const&  cls, ParameterContainer const& container)
{
  return {cls, container};
}
      
} } }

namespace efl { namespace eolian { namespace grammar {

template <typename ParameterContainer, typename Generator>
struct is_generator< ::eolian::js::grammar::out_parameter_definition_generator<ParameterContainer, Generator> > : std::true_type {};
template <typename ParameterContainer, typename Generator>
struct is_eager_generator< ::eolian::js::grammar::out_parameter_definition_generator<ParameterContainer, Generator>> : std::true_type {};

namespace type_traits {
      
template <typename ParameterContainer, typename Generator>
struct attributes_needed< ::eolian::js::grammar::out_parameter_definition_generator<ParameterContainer, Generator>> : std::integral_constant<int, 1> {};
template <typename ParameterContainer, typename Generator>
struct accepts_specific_tuple< ::eolian::js::grammar::out_parameter_definition_generator<ParameterContainer, Generator>
                               , ::efl::eolian::grammar::attributes::parameter_def > : std::true_type {};
 
} } } }

#endif
