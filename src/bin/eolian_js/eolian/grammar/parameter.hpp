#ifndef EOLIAN_JS_PARAMETER_HPP
#define EOLIAN_JS_PARAMETER_HPP

#include <grammar/klass_def.hpp>
#include <grammar/indentation.hpp>
#include <grammar/string.hpp>
#include <grammar/attribute_reorder.hpp>
#include <grammar/attributed.hpp>
#include <grammar/attribute_replace.hpp>
#include <eolian/grammar/stub_function_definition.hpp>

namespace eolian { namespace js { namespace grammar {

namespace attributes = efl::eolian::grammar::attributes;

template <typename ParameterContainer>
struct parameter_generator
{
  parameter_generator(attributes::klass_def const& cls, ParameterContainer const& container)
    : klass(cls), container(container) {}
  
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

    bool is_function_ptr = false;
    if(attributes::regular_type_def const* def
       = ::efl::eina::get<attributes::regular_type_def>(&param.type.original_type))
      {
        is_function_ptr = def->is_function_ptr();
      }
    
    if(is_function_ptr)
      {
        as_generator(" NULL, NULL, NULL").generate(sink, unused, context);
      }
    else
      {
    switch(param.direction)
      {
      case attributes::parameter_direction::in:
        as_generator(" ::efl::eina::js::get_value_from_javascript(args["
                     << string
                     << "], isolate, \"\", ::efl::eina::js::value_tag< "
                     << string
                     << " >{})"
                     ).generate(sink, std::make_tuple(std::to_string(index), param.type.c_type), context);
        break;
      case attributes::parameter_direction::inout:
      case attributes::parameter_direction::out:
        as_generator("&__out_"
                     << string
                     ).generate(sink, param.param_name, context);
        break;
      };
      }    
    
    return true;
  }

  attributes::klass_def klass;
  ParameterContainer const& container;  
};


template <typename ParameterContainer>
parameter_generator<ParameterContainer>
parameter(attributes::klass_def const&  cls, ParameterContainer const& container)
{
  return {cls, container};
}
      
} } }

namespace efl { namespace eolian { namespace grammar {

template <typename ParameterContainer>
struct is_generator< ::eolian::js::grammar::parameter_generator<ParameterContainer> > : std::true_type {};
template <typename ParameterContainer>
struct is_eager_generator< ::eolian::js::grammar::parameter_generator<ParameterContainer>> : std::true_type {};

namespace type_traits {
      
template <typename ParameterContainer>
struct attributes_needed< ::eolian::js::grammar::parameter_generator<ParameterContainer>> : std::integral_constant<int, 1> {};
template <typename ParameterContainer>
struct accepts_specific_tuple< ::eolian::js::grammar::parameter_generator<ParameterContainer>
                               , ::efl::eolian::grammar::attributes::parameter_def > : std::true_type {};
 
} } } }

#endif
