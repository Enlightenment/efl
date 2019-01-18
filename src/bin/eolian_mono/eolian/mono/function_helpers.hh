#ifndef EOLIAN_MONO_FUNCTION_DEFINITION_HELPERS_HH
#define EOLIAN_MONO_FUNCTION_DEFINITION_HELPERS_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/attribute_reorder.hpp"
#include "parameter.hh"

namespace eolian_mono {

/*
 * Generators for things that must happen inside the function definition *before* and
 * *after* the actual invocation of the underlying C function.
 *
 * For example, declaration and assignment of intermediate variables for out/ return types
 * that require some kind of manual work (e.g. string and Stringshare).
 */

struct native_function_definition_preamble_generator
{
    // FIXME Suport scoping tabs
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  { 
      std::string return_type;

      if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
          return false;

      if (!as_generator(
                *(scope_tab << scope_tab << native_convert_in_variable)
                << *(scope_tab << scope_tab << native_convert_out_variable)
                << *(scope_tab << scope_tab << native_convert_function_pointer)
                << scope_tab << scope_tab << scope_tab << native_convert_return_variable
                ).generate(sink, std::make_tuple(f.parameters, f.parameters, f.parameters, f.return_type), context))
          return false;

      return as_generator("\n").generate(sink, attributes::unused, context);
  }
};

struct function_definition_preamble_generator
{
    // FIXME Suport scoping tabs
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  { 
      std::string return_type;

      if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
          return false;

      if (!as_generator(
                  *(scope_tab << scope_tab << convert_in_variable)
                  << *(scope_tab << scope_tab << convert_out_variable)
                  << *(scope_tab << scope_tab << convert_function_pointer)
                  << scope_tab << scope_tab << convert_return_variable
                  ).generate(sink, std::make_tuple(f.parameters, f.parameters, f.parameters, f.return_type), context))
          return false;

      return true;
  }
};

struct native_function_definition_epilogue_generator
{
    attributes::klass_def const* klass;

    // FIXME Suport scoping tabs
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  { 
      std::string return_type;

      if(!as_generator(eolian_mono::type(true)).generate(std::back_inserter(return_type), f.return_type, context))
          return false;

      if (!as_generator(
                  *(scope_tab << scope_tab << native_convert_out_assign(*klass))
                  << *(scope_tab << scope_tab << native_convert_in_ptr_assign)
                  << scope_tab << scope_tab << native_convert_return(*klass)
                  ).generate(sink, std::make_tuple(f.parameters, f.parameters, f.return_type), context))
          return false;

      return true;
  }
};

struct function_definition_epilogue_generator
{
    // FIXME Suport scoping tabs
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  { 
      if (!as_generator(
                  scope_tab << scope_tab << "Eina.Error.RaiseIfUnhandledException();\n"
                  << *(scope_tab << scope_tab << convert_out_assign)
                  << *(scope_tab << scope_tab << convert_in_ptr_assign)
                  << scope_tab << scope_tab << convert_return
                  ).generate(sink, std::make_tuple(f.parameters, f.parameters, f.return_type), context))
          return false;

      return true;
  }
};

// Preamble tokens
struct native_function_definition_preamble_terminal
{
  native_function_definition_preamble_generator const operator()() const
  {
      return native_function_definition_preamble_generator();
  }
} const native_function_definition_preamble = {};

native_function_definition_preamble_generator const as_generator(native_function_definition_preamble_terminal)
{
    return native_function_definition_preamble_generator{};
}

struct function_definition_preamble_terminal
{
  function_definition_preamble_generator const operator()() const
  {
      return function_definition_preamble_generator();
  }
} const function_definition_preamble = {};

function_definition_preamble_generator const as_generator(function_definition_preamble_terminal)
{
    return function_definition_preamble_generator{};
}

// Epilogue tokens
struct native_function_definition_epilogue_parameterized
{
  native_function_definition_epilogue_generator const operator()(attributes::klass_def const& klass) const
  {
    return {&klass};
  }
  native_function_definition_epilogue_generator const operator()(attributes::klass_def const* klass=nullptr) const
  {
    return {klass};
  }
} const native_function_definition_epilogue;

struct function_definition_epilogue_terminal
{
  function_definition_epilogue_generator const operator()() const
  {
      return function_definition_epilogue_generator();
  }
} const function_definition_epilogue = {};

function_definition_epilogue_generator const as_generator(function_definition_epilogue_terminal)
{
    return function_definition_epilogue_generator{};
}

} // namespace eolian_mono

namespace efl { namespace eolian { namespace grammar {

// Preamble
template <>
struct is_eager_generator< ::eolian_mono::native_function_definition_preamble_generator> : std::true_type {};

template <>
struct is_generator< ::eolian_mono::native_function_definition_preamble_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_function_definition_preamble_terminal> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::function_definition_preamble_generator> : std::true_type {};

template <>
struct is_generator< ::eolian_mono::function_definition_preamble_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_definition_preamble_terminal> : std::true_type {};

// Epilogue
template <>
struct is_eager_generator< ::eolian_mono::native_function_definition_epilogue_generator> : std::true_type {};

template <>
struct is_generator< ::eolian_mono::native_function_definition_epilogue_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_function_definition_epilogue_parameterized> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::function_definition_epilogue_generator> : std::true_type {};

template <>
struct is_generator< ::eolian_mono::function_definition_epilogue_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_definition_epilogue_terminal> : std::true_type {};

namespace type_traits {
// Preamble
template <>
struct attributes_needed< ::eolian_mono::native_function_definition_preamble_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::native_function_definition_preamble_terminal> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::function_definition_preamble_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::function_definition_preamble_terminal> : std::integral_constant<int, 1> {};

// Epilogue
template <>
struct attributes_needed< ::eolian_mono::native_function_definition_epilogue_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::native_function_definition_epilogue_parameterized> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::function_definition_epilogue_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::function_definition_epilogue_terminal> : std::integral_constant<int, 1> {};

}

} } }

#endif
