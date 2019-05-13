#ifndef EOLIAN_MONO_ASYNC_FUNCTION_DEFINITION_HH
#define EOLIAN_MONO_ASYNC_FUNCTION_DEFINITION_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/attribute_reorder.hpp"
#include "logging.hh"
#include "type.hh"
#include "name_helpers.hh"
#include "helpers.hh"
#include "function_helpers.hh"
#include "marshall_type.hh"
#include "parameter.hh"
#include "documentation.hh"
#include "using_decl.hh"
#include "generation_contexts.hh"
#include "blacklist.hh"

namespace eolian_mono {

struct is_future
{
  typedef is_future visitor_type;
  typedef bool result_type;

  bool operator()(grammar::attributes::complex_type_def const& c) const
  {
     return c.outer.base_type == "future";
  }

  template<typename T>
  bool operator()(T const&) const
  {
     return false;
  }
};

struct async_function_declaration_generator
{
  template<typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    if (f.is_static)
      return true;
    if (blacklist::is_function_blacklisted(f, context))
      return true;
    if (!f.return_type.original_type.visit(is_future{}))
      return true;

    if (!as_generator(
            scope_tab << "/// <summary>Async wrapper for <see cref=\"" << name_helpers::managed_method_name(f) << "\" />.</summary>\n"
        ).generate(sink, attributes::unused, context))
      return false;

    // generate_parameter is not a proper as_generator-compatible generator, so we had to do an old fashioned loop
    for (auto&& param : f.parameters)
      if (!documentation(1).generate_parameter(sink, param, context))
        return false;

    if (!as_generator(
            scope_tab << "/// <param name=\"token\">Token to notify the async operation of external request to cancel.</param>\n"
            << scope_tab << "/// <returns>An async task wrapping the result of the operation.</returns>\n"
            << scope_tab << "System.Threading.Tasks.Task<Eina.Value> " << name_helpers::managed_async_method_name(f) << "(" << *(parameter << ",") <<
                                    " System.Threading.CancellationToken token = default(System.Threading.CancellationToken));\n\n"
        ).generate(sink, f.parameters, context))
      return false;

    return true;
  }
} const async_function_declaration {};

struct async_function_definition_generator
{
  async_function_definition_generator(bool do_super = false)
      : do_super(do_super)
  {}

  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "async_function_definition_generator: " << f.c_name;

    if(do_super && f.is_static) // Static methods goes only on Concrete classes.
      return true;
    if(blacklist::is_function_blacklisted(f, context))
      return true;
    if(!f.return_type.original_type.visit(is_future{}))
      return true;

    auto parameter_forwarding = [](attributes::parameter_def const& param) {
        return direction_modifier(param) + " " + name_helpers::escape_keyword(param.param_name);
    };
    std::vector<std::string> param_forwarding;

    std::transform(f.parameters.begin(), f.parameters.end(), std::back_inserter(param_forwarding), parameter_forwarding);

    if (!as_generator(
            scope_tab << "/// <summary>Async wrapper for <see cref=\"" << name_helpers::managed_method_name(f) << "\" />.</summary>\n"
        ).generate(sink, attributes::unused, context))
      return false;

    // generate_parameter is not a proper as_generator-compatible generator, so we had to do an old fashioned loop
    for (auto&& param : f.parameters)
      if (!documentation(1).generate_parameter(sink, param, context))
        return false;

    if(!as_generator(
            scope_tab << "/// <param name=\"token\">Token to notify the async operation of external request to cancel.</param>\n"
            << scope_tab << "/// <returns>An async task wrapping the result of the operation.</returns>\n"
            << scope_tab << "public System.Threading.Tasks.Task<Eina.Value> " << name_helpers::managed_async_method_name(f) << "(" << *(parameter << ",") << " System.Threading.CancellationToken token = default(System.Threading.CancellationToken))\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << "Eina.Future future = " << name_helpers::managed_method_name(f) << "(" << (string % ",") << ");\n"
            << scope_tab << scope_tab << "return Efl.Eo.Globals.WrapAsync(future, token);\n"
            << scope_tab << "}\n\n"
        ).generate(sink, std::make_tuple(f.parameters, param_forwarding), context))
      return false;
    return true;
  }

  bool do_super;
};

struct async_function_definition_parameterized
{
  async_function_definition_generator operator()(bool do_super=false) const
  {
    return {do_super};
  }
} const async_function_definition;
async_function_definition_generator as_generator(async_function_definition_parameterized)
{
  return {};
}

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::async_function_declaration_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::async_function_declaration_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::async_function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::async_function_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::async_function_definition_parameterized> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::async_function_declaration_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::async_function_definition_generator> : std::integral_constant<int, 1> {};
template <>
struct attributes_needed< ::eolian_mono::async_function_definition_parameterized> : std::integral_constant<int, 1> {};
}

} } }

#endif
