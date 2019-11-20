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
#ifndef CULTURE_INFO_GENERATOR_HH
#define CULTURE_INFO_GENERATOR_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

namespace eolian_mono {

namespace attributes = efl::eolian::grammar::attributes;

// Some methods on string require a culture info parameter due to warning CA1307
struct culture_info_generator
{
    template<typename OutputIterator, typename Context>
    bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
    {
        std::string managed_name;

        if (!as_generator(type).generate(std::back_inserter(managed_name), field.type, context))
          return false;

        if (managed_name == "System.String")
          if (!as_generator(lit("StringComparison.Ordinal")).generate(sink, attributes::unused, context))
            return false;

        return true;
    }
} const culture_info {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::culture_info_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::culture_info_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::culture_info_generator> : std::integral_constant<int, 1> {};
}

} } }

#endif // CULTURE_INFO_GENERATOR_HH
