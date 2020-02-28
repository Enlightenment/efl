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
#ifndef EOLIAN_MONO_MARSHALL_ANNOTATION_IMPL_HH
#define EOLIAN_MONO_MARSHALL_ANNOTATION_IMPL_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "name_helpers.hh"
#include "type_impl.hh"
#include "type_match.hh"

namespace eolian_mono {

namespace eina = efl::eina;

namespace detail {

template <typename OutputIterator, typename Context>

struct marshall_annotation_visitor_generate
{
   mutable OutputIterator sink;
   Context const* context;
   std::string c_type;
   bool is_out;
   bool is_return;
   bool is_ptr;

   typedef marshall_type_visitor_generate<OutputIterator, Context> visitor_type;
   typedef bool result_type;

   bool operator()(attributes::regular_type_def const& regular) const
   {
      using attributes::regular_type_def;
      struct match
      {
        eina::optional<std::string> name;
        eina::optional<bool> has_own;
        std::function<std::string()> function;
      };
      // These two tables are currently the same but will hold different marshallers
      // for @in and @out/return semantics in a future commit.
      match const parameter_match_table[] =
        {
           // signed primitives
          {"bool", nullptr, [] { return "MarshalAs(UnmanagedType.U1)"; }},
          {"string", true, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))";
          }},
          {"string", false, [this] {
                auto is_native_to_managed = false;
                if constexpr (efl::eolian::grammar::tag_check<direction_context, Context>::value)
                    is_native_to_managed = context_find_tag<direction_context>(*context).current_direction == direction_context::native_to_managed;

                if((is_out || is_return) && is_native_to_managed)
                    return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringOutMarshaler))";
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))";
          }},
          {"mstring", true, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))";
          }},
          {"mstring", false, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))";
          }},
          {"stringshare", true, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringsharePassOwnershipMarshaler))";
          }},
          {"stringshare", false, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringshareKeepOwnershipMarshaler))";
          }},
          {"any_value_ref", true, [] {
                    return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueMarshalerOwn))";
          }},
          {"any_value_ref", false, [] {
                    return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueMarshaler))";
          }},
          {"strbuf", true, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufPassOwnershipMarshaler))";
          }},
          {"strbuf", false, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufKeepOwnershipMarshaler))";
          }},
          {"Value_Type", false, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueTypeMarshaler))";
          }},
        };
      match const return_match_table[] =
        {
           // signed primitives
          {"bool", nullptr, [] { return "MarshalAs(UnmanagedType.U1)"; }},
          {"string", true, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))";
          }},
          {"string", false, [this] {
                auto is_native_to_managed = false;
                if constexpr (efl::eolian::grammar::tag_check<direction_context, Context>::value)
                    is_native_to_managed = context_find_tag<direction_context>(*context).current_direction == direction_context::native_to_managed;

                if((is_out || is_return) && is_native_to_managed)
                    return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringOutMarshaler))";
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))";
          }},
          {"mstring", true, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))";
          }},
          {"mstring", false, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))";
          }},
          {"stringshare", true, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringsharePassOwnershipMarshaler))";
          }},
          {"stringshare", false, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringshareKeepOwnershipMarshaler))";
          }},
          {"any_value_ref", true, [] {
                    return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueMarshalerOwn))";
          }},
          {"any_value_ref", false, [] {
                    return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueMarshaler))";
          }},
          {"strbuf", true, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufPassOwnershipMarshaler))";
          }},
          {"strbuf", false, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufKeepOwnershipMarshaler))";
          }},
          {"Value_Type", false, [] {
                return "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueTypeMarshaler))";
          }},
        };

        auto predicate = [&regular] (match const& m)
          {
            return (!m.name || *m.name == regular.base_type)
            && (!m.has_own || *m.has_own == (bool)(regular.base_qualifier & qualifier_info::is_own))
            ;
          };

        auto acceptCb = [this] (std::string const& marshalTag)
          {
            std::string prefix = is_return ? "return: " : "";
            return as_generator("[" << prefix << marshalTag << "]").generate(sink, nullptr, *context);
          };

        const auto& match_table = is_return ? return_match_table : parameter_match_table;

        if(eina::optional<bool> b = type_match::get_match(match_table, predicate, acceptCb))
          {
             return *b;
          }
        else
          {
             return true;
          }
   }
   bool operator()(attributes::klass_name const& klass_name) const
   {
     const char *return_prefix = is_return ? "return:" : "";
     const char *marshal_prefix = "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(";

     std::string name = name_helpers::klass_full_concrete_name(klass_name);

     if (name == "Efl.Class")
       name = "Efl.Eo.MarshalEflClass";
     else
       {
          std::string own = klass_name.base_qualifier & qualifier_info::is_own ? "Move" : "NoMove";
          name = "Efl.Eo.MarshalEo" + own;
       }

     return as_generator(
             lit("[") << return_prefix << marshal_prefix << name << "))]"
        ).generate(sink, name, *context);
   }
   bool operator()(attributes::complex_type_def const& c) const
   {
     if (c.outer.base_type == "future")
       {
          std::string prefix = is_return ? "return: " : "";
          return as_generator("[" << prefix << "MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.FutureMarshaler))]").generate(sink, nullptr, *context);
       }
     return true;
   }
};
} }

#endif
