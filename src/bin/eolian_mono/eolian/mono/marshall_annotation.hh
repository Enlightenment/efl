#ifndef EOLIAN_MONO_MARSHALL_ANNOTATION_IMPL_HH
#define EOLIAN_MONO_MARSHALL_ANNOTATION_IMPL_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "namespace.hh"
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
          {"bool", nullptr, [&] { return " [MarshalAs(UnmanagedType.I1)]"; }}
        };
      match const return_match_table[] =
        {
           // signed primitives
          {"bool", nullptr, [&] { return " [return: MarshalAs(UnmanagedType.I1)]"; }}
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
     const char no_return_prefix[] = "[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(efl.eo.MarshalTest<";
     const char return_prefix[] = "[return: MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(efl.eo.MarshalTest<";
     return as_generator
       ((is_return ? return_prefix : no_return_prefix)
        << *(lower_case[string] << ".") << string
        << "Concrete>))]"
        ).generate(sink, std::make_tuple(klass_name.namespaces, klass_name.eolian_name), *context);
   }
   bool operator()(attributes::complex_type_def const&) const
   {
     return true;
   }
};
      
} }

#endif
