#ifndef EOLIAN_CXX_HTML_ESCAPED_STRING_HH
#define EOLIAN_CXX_HTML_ESCAPED_STRING_HH

#include <cstdlib>
#include <cstring>

#include "grammar/generator.hpp"
#include "grammar/attributes.hpp"
#include "grammar/string.hpp"

namespace efl { namespace eolian { namespace grammar {

struct html_escaped_string_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, std::string const& input, Context const& context) const
   {
      std::string out;
      out.reserve(input.size());
      for (size_t pos = 0; pos != input.size(); ++pos)
        {
           switch(input[pos])
            {
               case '&':  out.append("&amp;"); break;
               case '\"': out.append("&quot;"); break;
               case '\'': out.append("&apos;"); break;
               case '<':  out.append("&lt;"); break;
               case '>':  out.append("&gt;"); break;
               default:   out.append(&input[pos], 1); break;
            }
        }

      return as_generator(string).generate(sink, out, context);
   }
};

struct html_escaped_string_generator const html_escaped_string = {};

template <>
struct is_eager_generator<html_escaped_string_generator> : std::true_type {};
template <>
struct is_generator<html_escaped_string_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<html_escaped_string_generator> : std::integral_constant<int, 1> {};
}
} } }

#endif

