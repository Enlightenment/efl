#ifndef EOLIAN_MONO_MARSHALL_TYPE_HH
#define EOLIAN_MONO_MARSHALL_TYPE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"

namespace eolian_mono {

namespace attributes = efl::eolian::grammar::attributes;
namespace detail {
template <typename OutputIterator, typename Context>
struct marshall_type_visitor_generate;
template <typename OutputIterator, typename Context>
struct marshall_annotation_visitor_generate;
template <typename OutputIterator, typename Context>
struct marshall_native_annotation_visitor_generate;
}

/*
 * Converts a given type/parameter to the type used in the DllImport signatures.
 *
 * For example, Eina.Value can be marshaled either as an eina.Value instance through
 * CustomMarshallers if we have a ptr(Eina.Value) or through the intermediate
 * eina.ValueNative blittable struct if it is passed by value.
 *
 * For details, check marshall_type_impl.h with the actual conversion rules.
 */
struct marshall_type_generator
{
   marshall_type_generator(bool is_return = false)
     : is_return(is_return) {}
  
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& type, Context const& context) const
   {
      return type.original_type.visit(detail::marshall_type_visitor_generate<OutputIterator, Context>{sink, &context, type.c_type, false, is_return, type.is_ptr });
   }
   /* Some types may require a different conversion when they are in @out parameters. */
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return param.type.original_type.visit(detail::marshall_type_visitor_generate<OutputIterator, Context>{sink, &context, param.type.c_type
            , param.direction != attributes::parameter_direction::in, false, param.type.is_ptr});
   }

   bool is_return;
};

/*
 * Generates the "[MarshalAs(...)]" rules for the given type.
 *
 * For example, the CustomMarshallers definitions for String and eina.Values and the
 * boolean size defintion (Eina_Value is 1 byte, while C# bool has 4 bytes).
 */
struct marshall_annotation_generator
{
   marshall_annotation_generator(bool is_return = false)
     : is_return(is_return) {}
  
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& type, Context const& context) const
   {
      return type.original_type.visit(detail::marshall_annotation_visitor_generate<OutputIterator, Context>{sink, &context, type.c_type, false, is_return, type.is_ptr});
   }
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return param.type.original_type.visit(detail::marshall_annotation_visitor_generate<OutputIterator, Context>{sink, &context, param.type.c_type
            , param.direction != attributes::parameter_direction::in, false, param.type.is_ptr});
   }

   bool is_return;
};

// struct marshall_native_annotation_generator
// {
//    marshall_native_annotation_generator(bool is_return = false)
//      : is_return(is_return) {}

//    template <typename OutputIterator, typename Context>
//    bool generate(OutputIterator sink, attributes::type_def const& type, Context const& context) const
//    {
//       return type.original_type.visit(detail::marshall_native_annotation_visitor_generate<OutputIterator, Context>{sink, &context, type.c_type, false, is_return, type.is_ptr});
//    }
//    template <typename OutputIterator, typename Context>
//    bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
//    {
//       return param.type.original_type.visit(detail::marshall_native_annotation_visitor_generate<OutputIterator, Context>{sink, &context, param.type.c_type
//             , param.direction != attributes::parameter_direction::in, false, param.type.is_ptr});
//    }

//    bool is_return;
// };
  
struct marshall_type_terminal
{
  marshall_type_generator const operator()(bool is_return) const
  {
    return marshall_type_generator(is_return);
  }
} const marshall_type = {};

marshall_type_generator const as_generator(marshall_type_terminal)
{
  return marshall_type_generator{};
}

struct marshall_annotation_terminal
{
  marshall_annotation_generator const operator()(bool is_return) const
  {
    return marshall_annotation_generator(is_return);
  }
} const marshall_annotation = {};

marshall_annotation_generator const as_generator(marshall_annotation_terminal)
{
  return marshall_annotation_generator{};
}


// struct marshall_native_annotation_terminal
// {
//   marshall_native_annotation_generator const operator()(bool is_return) const
//   {
//     return marshall_native_annotation_generator(is_return);
//   }
// } const marshall_native_annotation = {};

// marshall_native_annotation_generator const as_generator(marshall_native_annotation_terminal)
// {
//   return marshall_native_annotation_generator{};
// }
  
}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::marshall_type_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::marshall_type_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::marshall_type_terminal> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::marshall_type_generator> : std::integral_constant<int, 1> {};  
template <>
struct attributes_needed< ::eolian_mono::marshall_type_terminal> : std::integral_constant<int, 1> {};  
}

template <>
struct is_eager_generator< ::eolian_mono::marshall_annotation_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::marshall_annotation_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::marshall_annotation_terminal> : std::true_type {};

// template <>
// struct is_eager_generator< ::eolian_mono::marshall_native_annotation_generator> : std::true_type {};
// template <>
// struct is_generator< ::eolian_mono::marshall_native_annotation_generator> : std::true_type {};
// template <>
// struct is_generator< ::eolian_mono::marshall_native_annotation_terminal> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::marshall_annotation_generator> : std::integral_constant<int, 1> {};  
template <>
struct attributes_needed< ::eolian_mono::marshall_annotation_terminal> : std::integral_constant<int, 1> {};  

// template <>
// struct attributes_needed< ::eolian_mono::marshall_native_annotation_generator> : std::integral_constant<int, 1> {};
// template <>
// struct attributes_needed< ::eolian_mono::marshall_native_annotation_terminal> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
