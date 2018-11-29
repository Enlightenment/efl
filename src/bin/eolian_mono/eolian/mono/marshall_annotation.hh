#ifndef EOLIAN_MONO_MARSHALL_ANNOTATION_IMPL_HH
#define EOLIAN_MONO_MARSHALL_ANNOTATION_IMPL_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "name_helpers.hh"
#include "type_impl.hh"

namespace eolian_mono {

namespace eina = efl::eina;

namespace detail {

template <typename Array, typename F, int N, typename A>
eina::optional<bool> call_annotation_match(Array const (&array)[N], F f, A a)
{
   typedef Array const* iterator_type;
   iterator_type match_iterator = &array[0], match_last = match_iterator + N;
   match_iterator = std::find_if(match_iterator, match_last, f);
   if(match_iterator != match_last)
     {
        return a(match_iterator->function());
     }
   return {nullptr};
}
  
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
      match const parameter_match_table[] =
        {
           // signed primitives
          {"bool", nullptr, [&] { return " [MarshalAs(UnmanagedType.U1)]"; }},
          {"string", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))]";
          }},
          {"string", false, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))]";
          }},
          {"mstring", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))]";
          }},
          {"mstring", false, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))]";
          }},
          {"stringshare", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringsharePassOwnershipMarshaler))]";
          }},
          {"stringshare", false, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringshareKeepOwnershipMarshaler))]";
          }},
          {"any_value_ptr", true, [&] {
                    return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueMarshalerOwn))]";
          }},
          {"any_value_ptr", false, [&] {
                    return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueMarshaler))]";
          }},
          {"strbuf", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufPassOwnershipMarshaler))]";
          }},
          {"strbuf", false, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufKeepOwnershipMarshaler))]";
          }},
        };
      match const return_match_table[] =
        {
           // signed primitives
          {"bool", nullptr, [&] { return " [return: MarshalAs(UnmanagedType.U1)]"; }},
          {"string", true, [&] {
                return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))]";
          }},
          {"string", false, [&] {
                return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))]";
          }},
          {"mstring", true, [&] {
                return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))]";
          }},
          {"mstring", false, [&] {
                return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))]";
          }},
          {"stringshare", true, [&] {
                return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringsharePassOwnershipMarshaler))]";
          }},
          {"stringshare", false, [&] {
                return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringshareKeepOwnershipMarshaler))]";
          }},
          {"any_value_ptr", true, [&] {
                    return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueMarshalerOwn))]";
          }},
          {"any_value_ptr", false, [&] {
                    return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Eina.ValueMarshaler))]";
          }},
          {"strbuf", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufPassOwnershipMarshaler))]";
          }},
          {"strbuf", false, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufKeepOwnershipMarshaler))]";
          }},
        };

        if(eina::optional<bool> b = call_annotation_match
           ((is_return ? return_match_table : parameter_match_table)
          , [&] (match const& m)
          {
            return (!m.name || *m.name == regular.base_type)
            && (!m.has_own || *m.has_own == (bool)(regular.base_qualifier & qualifier_info::is_own))
            ;
          }
          , [&] (std::string const& string)
          {
            std::copy(string.begin(), string.end(), sink);
            return true;
          }))
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
     const char no_return_prefix[] = "[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Efl.Eo.MarshalTest<";
     const char return_prefix[] = "[return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Efl.Eo.MarshalTest<";
     return as_generator
       ((is_return ? return_prefix : no_return_prefix)
        << string << ", Efl.Eo." << (klass_name.base_qualifier & qualifier_info::is_own ? "OwnTag" : "NonOwnTag") << ">))]"
        ).generate(sink, name_helpers::klass_full_concrete_name(klass_name), *context);
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

template <typename OutputIterator, typename Context>
struct marshall_native_annotation_visitor_generate
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
      match const parameter_match_table[] =
        {
           // signed primitives
          {"bool", nullptr, [&] { return " [MarshalAs(UnmanagedType.U1)]"; }},
          {"string", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))]";
          }},
          {"string", false, [&] {
                if (is_out)
                   return "";
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringKeepOwnershipMarshaler))]";
          }},
          {"stringshare", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringsharePassOwnershipMarshaler))]";
          }},
          {"stringshare", false, [&] {
                if (is_out)
                    return "";
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringshareKeepOwnershipMarshaler))]";
          }},
          {"strbuf", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufPassOwnershipMarshaler))]";
          }},
          {"strbuf", false, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufKeepOwnershipMarshaler))]";
          }},
        };
      match const return_match_table[] =
        {
           // signed primitives
          {"bool", nullptr, [&] { return " [return: MarshalAs(UnmanagedType.U1)]"; }},
          {"string", true, [&] {
                return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringPassOwnershipMarshaler))]";
          }},
          {"string", false, [&] { return ""; }},
          {"stringshare", true, [&] {
                return " [return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StringsharePassOwnershipMarshaler))]";
          }},
          {"stringshare", false, [&] { return ""; }},
          {"strbuf", true, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufPassOwnershipMarshaler))]";
          }},
          {"strbuf", false, [&] {
                return " [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef=typeof(Efl.Eo.StrbufKeepOwnershipMarshaler))]";
          }},
        };

        if(eina::optional<bool> b = call_annotation_match
           ((is_return ? return_match_table : parameter_match_table)
          , [&] (match const& m)
          {
            return (!m.name || *m.name == regular.base_type)
            && (!m.has_own || *m.has_own == (bool)(regular.base_qualifier & qualifier_info::is_own))
            ;
          }
          , [&] (std::string const& string)
          {
            std::copy(string.begin(), string.end(), sink);
            return true;
          }))
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
     const char no_return_prefix[] = "[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Efl.Eo.MarshalTest<";
     const char return_prefix[] = "[return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Efl.Eo.MarshalTest<";
     return as_generator
       ((is_return ? return_prefix : no_return_prefix)
        << string << ", Efl.Eo." << (klass_name.base_qualifier & qualifier_info::is_own ? "OwnTag" : "NonOwnTag") << ">))]"
        ).generate(sink, name_helpers::klass_full_concrete_name(klass_name), *context);
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
