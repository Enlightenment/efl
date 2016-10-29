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
        std::function<attributes::type_def::variant_type()> function;
      };
      match const parameter_match_table[] =
        {
           // signed primitives
             {"bool", nullptr, [&] { return replace_base_type(regular, " [System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.I1)]"); }}
        };
      match const return_match_table[] =
        {
           // signed primitives
             {"bool", nullptr, [&] { return replace_base_type(regular, " [return: System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.I1)]"); }}
        };

        if(eina::optional<bool> b = call_match
           ((is_return ? return_match_table : parameter_match_table)
          , [&] (match const& m)
          {
            return (!m.name || *m.name == regular.base_type)
            && (!m.has_own || *m.has_own == (bool)(regular.base_qualifier & qualifier_info::is_own))
            ;
          }
          , [&] (attributes::type_def::variant_type const& v)
          {
            return v.visit(*this); // we want to keep is_out info
          }))
        {
           return *b;
        }
      else
        {
          return true;
        }
   }
   bool operator()(attributes::klass_name) const
   {
     if(!is_return)
       {
         const char marshal[] = "[System.Runtime.InteropServices.MarshalAs(System.Runtime.InteropServices.UnmanagedType.CustomMarshaler, /*System.Runtime.InteropServices.*/MarshalTypeRef = typeof(efl.eo.MarshalTest))]";
         std::copy(&marshal[0], &marshal[0] + sizeof(marshal) - 1, sink);
       }
     return true;
   }
   bool operator()(attributes::complex_type_def const&) const
   {
     return true;
   }
};
      
} }

#endif
