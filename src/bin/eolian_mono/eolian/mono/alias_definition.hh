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
#ifndef EOLIAN_MONO_ALIAS_DEFINITION_HH
#define EOLIAN_MONO_ALIAS_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"

#include "using_decl.hh"
#include "name_helpers.hh"
#include "blacklist.hh"
#include "documentation.hh"
#include "generation_contexts.hh"

namespace eolian_mono {

struct alias_definition_generator
{
  template<typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::alias_def const& alias, Context const& context) const
  {
     if (blacklist::is_alias_blacklisted(alias, context))
       {
          EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "Alias " <<  name_helpers::alias_full_eolian_name(alias) << "is blacklisted. Skipping.";
          return true;
       }

     if (alias.is_undefined)
       {
          EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "Alias " <<  name_helpers::alias_full_eolian_name(alias) << "is undefined. Skipping.";
          return true;
       }

     if (!name_helpers::open_namespaces(sink, alias.namespaces, context))
       return false;

     std::string alias_type;
     if (!as_generator(eolian_mono::type).generate(std::back_inserter(alias_type), alias.base_type, context))
       return false;

     std::string alias_type_doc;
     alias_type_doc = utils::replace_all(alias_type, "<", "&lt;");
     alias_type_doc = utils::replace_all(alias_type_doc, ">", "&gt;");

     std::string alias_since;
     if (!documentation_helpers::generate_since_tag_line(std::back_inserter(alias_since), alias.documentation, scope_tab, context))
       return false;

     std::string const alias_name = utils::remove_all(alias.eolian_name, '_');
     if (!as_generator(
                 documentation
                 << "public struct " << alias_name << " : IEquatable<" << alias_name << ">\n"
                 << "{\n"
                 << scope_tab << "private " << alias_type << " payload;\n\n"

                 << scope_tab << "/// <summary>Converts an instance of " << alias_type_doc << " to this struct.\n"
                 << alias_since
                 << scope_tab << "/// </summary>\n"
                 << scope_tab << "/// <param name=\"value\">The value to be converted.</param>\n"
                 << scope_tab << "/// <returns>A struct with the given value.</returns>\n"
                 << scope_tab << "public static implicit operator " << alias_name << "(" << alias_type << " value)\n"
                 << scope_tab << "{\n"
                 << scope_tab << scope_tab << "return new " << alias_name << "{payload=value};\n"
                 << scope_tab << "}\n\n"

                 << scope_tab << "/// <summary>Converts an instance of this struct to " << alias_type_doc << ".\n"
                 << alias_since
                 << scope_tab << "/// </summary>\n"
                 << scope_tab << "/// <param name=\"value\">The value to be converted packed in this struct.</param>\n"
                 << scope_tab << "/// <returns>The actual value the alias is wrapping.</returns>\n"
                 << scope_tab << "public static implicit operator " << alias_type << "(" << alias_name << " value)\n"
                 << scope_tab << "{\n"
                 << scope_tab << scope_tab << "return value.payload;\n"
                 << scope_tab << "}\n"

                 << scope_tab << "/// <summary>Converts an instance of " << alias_type_doc << " to this struct.</summary>\n"
                 << scope_tab << "/// <param name=\"value\">The value to be converted.</param>\n"
                 << scope_tab << "/// <returns>A struct with the given value.</returns>\n"
                 << scope_tab << "public static " << alias_name << " From" << name_helpers::translate_value_type(alias_type) << "(" << alias_type << " value)\n"
                 << scope_tab << "{\n"
                 << scope_tab << scope_tab << "return value;\n"
                 << scope_tab << "}\n\n"

                 << scope_tab << "/// <summary>Converts an instance of this struct to " << alias_type_doc << ".</summary>\n"
                 << scope_tab << "/// <returns>The actual value the alias is wrapping.</returns>\n"
                 << scope_tab << "public " << alias_type << " To" << name_helpers::translate_value_type(alias_type) << "()\n"
                 << scope_tab << "{\n"
                 << scope_tab << scope_tab << "return this;\n"
                 << scope_tab << "}\n"
                 ).generate(sink, alias, context))
       return false;

     std::string since_line;
     if (!alias.documentation.since.empty())
         if (!as_generator(scope_tab << "/// <para>Since EFL " + alias.documentation.since + ".</para>\n"
                 ).generate(std::back_inserter(since_line), attributes::unused, context))
           return false;

     // GetHashCode (needed by the equality comparisons)
     if (!as_generator(
             scope_tab << "/// <summary>Get a hash code for this item.\n"
             << since_line
             << scope_tab << "/// </summary>\n"
             << scope_tab << "public override int GetHashCode() => payload.GetHashCode();\n"
          ).generate(sink, attributes::unused, context))
       return false;

     // IEquatble<T> Equals
     if (!as_generator(
                 scope_tab << "/// <summary>Equality comparison.\n"
                 << since_line
                 << scope_tab << "/// </summary>\n"
                 << scope_tab << "public bool Equals(" << alias_name << " other) => payload == other.payload;\n"
          ).generate(sink, attributes::unused, context))
       return false;

     // ValueType.Equals
     if (!as_generator(
                 scope_tab << "/// <summary>Equality comparison.\n"
                 << since_line
                 << scope_tab << "/// </summary>\n"
                 << scope_tab << "public override bool Equals(object other)\n"
                 << scope_tab << scope_tab << "=> ((other is " << alias_name << ") ? Equals((" << alias_name << ")other) : false);\n"
        ).generate(sink, attributes::unused, context))
       return false;

     // Equality operators
     if (!as_generator(
                 scope_tab << "/// <summary>Equality comparison.\n"
                 << since_line
                 << scope_tab << "/// </summary>\n"
                 << scope_tab << "public static bool operator ==(" << alias_name << " lhs, " << alias_name << " rhs)\n"
                 << scope_tab << scope_tab << "=> lhs.payload == rhs.payload;\n"
        ).generate(sink, attributes::unused, context))
       return false;

     if (!as_generator(
                 scope_tab << "/// <summary>Equality comparison.\n"
                 << since_line
                 << scope_tab << "/// </summary>\n"
                 << scope_tab << "public static bool operator !=(" << alias_name << " lhs, " << alias_name << " rhs)\n"
                 << scope_tab << scope_tab << "=> lhs.payload != rhs.payload;\n"
        ).generate(sink, attributes::unused, context))
       return false;

     if (!as_generator(
                 "}\n"
                 ).generate(sink, alias, context))
       return false;

     if (!name_helpers::close_namespaces(sink, alias.namespaces, context))
       return false;

     return true;
  }
} const alias_definition {};

}

namespace efl { namespace eolian { namespace grammar {

template<>
struct is_eager_generator< ::eolian_mono::alias_definition_generator> : std::true_type {};
template<>
struct is_generator< ::eolian_mono::alias_definition_generator> : std::true_type {};

namespace type_traits {

template<>
struct attributes_needed< ::eolian_mono::alias_definition_generator> : std::integral_constant<int, 1> {};

}

} } }

#endif
