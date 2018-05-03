#ifndef EOLIAN_CXX_PART_IMPLEMENTATION_HH
#define EOLIAN_CXX_PART_IMPLEMENTATION_HH

#include "grammar/generator.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/keyword.hpp"

namespace efl { namespace eolian { namespace grammar {

struct part_implementation_generator
{
   part_implementation_generator(std::string const& _klass_name)
      : klass_name(_klass_name) {}

   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::part_def const& part, Context const& ctx) const
   {
      if(!as_generator("#ifdef EFL_CXXPERIMENTAL\n").generate(sink, attributes::unused, ctx))
        return false;

      if(!as_generator("::efl::eolian::return_traits<::" << *(string << "::"))
            .generate(sink, part.klass.namespaces, add_lower_case_context(ctx)))
        return false;
      // FIXME: part_def can't depend on klass_def so C type is not known :(
      if(!as_generator(string << ">::type "<< string << "::" << string << "() const\n{\n"
                       << scope_tab << "::Eo *__return_value = ::efl_part_get"
                       << "(this->_eo_ptr(), \"" << string << "\");\n")
            .generate(sink, std::make_tuple(part.klass.eolian_name, klass_name, part.name, part.name), ctx))
        return false;
      if(!as_generator(scope_tab << "return ::" << *(string << "::"))
            .generate(sink, part.klass.namespaces, add_lower_case_context(ctx)))
        return false;
      if(!as_generator(string << "{__return_value};\n}\n")
            .generate(sink, part.klass.eolian_name, ctx))
        return false;

      if(!as_generator("#endif\n").generate(sink, attributes::unused, ctx))
        return false;

      return true;
   }

private:
   std::string klass_name;
};

template <>
struct is_eager_generator<part_implementation_generator> : std::true_type {};
template <>
struct is_generator<part_implementation_generator> : std::true_type {};
namespace type_traits {
template <>
struct attributes_needed<part_implementation_generator> : std::integral_constant<int, 1> {};
}

struct part_implementation_terminal
{
  part_implementation_generator operator()(std::string klass_name) const
  {
    return part_implementation_generator{klass_name};
  }
} const part_implementation = {};

} } }

#endif
