#ifndef EOLIAN_MONO_STRUCT_FIELDS_HH
#define EOLIAN_MONO_STRUCT_FIELDS_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "name_helpers.hh"
#include "type.hh"
#include "using_decl.hh"
#include "documentation.hh"

namespace eolian_mono {

struct field_argument_default_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
       if (!as_generator(type << " " << string << " = default(" << type << ")")
               .generate(sink, std::make_tuple(field.type, name_helpers::to_field_name(field.name), field.type), context))
           return false;
       return true;
   }
} const field_argument_default {};

struct field_argument_assignment_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
       if (!as_generator("this." << string << " = " << string)
               .generate(sink, std::make_tuple(name_helpers::to_field_name(field.name), name_helpers::to_field_name(field.name)), context))
           return false;
       return true;
   }
} const field_argument_assignment {};

}

namespace efl { namespace eolian { namespace grammar {

template<>
struct is_eager_generator< ::eolian_mono::field_argument_default_generator> : std::true_type {};
template<>
struct is_generator< ::eolian_mono::field_argument_default_generator> : std::true_type {};

template<>
struct is_eager_generator< ::eolian_mono::field_argument_assignment_generator> : std::true_type {};
template<>
struct is_generator< ::eolian_mono::field_argument_assignment_generator> : std::true_type {};

namespace type_traits {

template <>
struct attributes_needed< ::eolian_mono::field_argument_default_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::field_argument_assignment_generator> : std::integral_constant<int, 1> {};

}

} } }

#endif

