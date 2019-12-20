/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
               case '\\': if (pos < input.size() - 1) pos++;
                    // fall through
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

