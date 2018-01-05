#ifndef EOLIAN_JS_CLASS_DEFINITION_HPP
#define EOLIAN_JS_CLASS_DEFINITION_HPP

#include <grammar/klass_def.hpp>

namespace eolian { namespace js { namespace grammar {

namespace attributes = efl::eolian::grammar::attributes;

struct class_registration_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
  {
    
    
    return true;
  }
};


class_registration_generator const class_registration = {};
      
} } }

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_generator< ::eolian::js::grammar::class_registration_generator> : std::true_type {};
template <>
struct is_eager_generator< ::eolian::js::grammar::class_registration_generator> : std::true_type {};
      
} } }

#endif
