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
#ifndef EOLIAN_CXX_IGNORE_WARNING_HH
#define EOLIAN_CXX_IGNORE_WARNING_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

struct ignore_warning_begin_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::unused_type, Context const& context) const
   {
      return as_generator(
        "\n"
        "#pragma GCC diagnostic push\n"
        "#pragma GCC diagnostic ignored \"-Wignored-qualifiers\"\n"
        "\n"
      ).generate(sink, nullptr, context);
   }
};

struct ignore_warning_end_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::unused_type, Context const& context) const
   {
      return as_generator(
        "\n#pragma GCC diagnostic pop\n\n"
      ).generate(sink, nullptr, context);
   }
};

template <>
struct is_eager_generator<ignore_warning_begin_generator> : std::true_type {};
template <>
struct is_generator<ignore_warning_begin_generator> : std::true_type {};

template <>
struct is_eager_generator<ignore_warning_end_generator> : std::true_type {};
template <>
struct is_generator<ignore_warning_end_generator> : std::true_type {};

ignore_warning_begin_generator constexpr ignore_warning_begin = {};
ignore_warning_end_generator constexpr ignore_warning_end = {};

} } }

#endif
