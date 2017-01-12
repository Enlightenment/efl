#ifndef EOLIAN_MONO_MARSHALL_TYPE_IMPL_HH
#define EOLIAN_MONO_MARSHALL_TYPE_IMPL_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "namespace.hh"
#include "type_impl.hh"

namespace eolian_mono {

namespace eina = efl::eina;

namespace detail {
  
template <typename OutputIterator, typename Context>
struct marshall_type_visitor_generate
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
      }
      const match_table[] =
        {
           // signed primitives
           {"string", true, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                // if(is_out || is_return)
                  return replace_base_type(r, " System.String");
                // else return replace_base_type(r, " ::efl::eina::string_view");
              }}
           , {"string", false, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           , {"stringshare", nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           // , {"generic_value", true, [&]
           //    { return regular_type_def{" int", regular.base_qualifier, {}};
           //    }}
           // , {"generic_value", false, [&]
           //    { return regular_type_def{" int", regular.base_qualifier, {}};
           //    }}
        };

        if(eina::optional<bool> b = call_match
         (match_table
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
          return visitor_generate<OutputIterator, Context>{sink, context, c_type, is_out, is_return}(regular);
        }
   }
   bool operator()(attributes::klass_name klass_name) const
   {
     return visitor_generate<OutputIterator, Context>{sink, context, c_type, is_out, is_return}(klass_name);
     // return as_generator(" System.IntPtr").generate(sink, attributes::unused, *context);
   }
   bool operator()(attributes::complex_type_def const& complex) const
   {
     return visitor_generate<OutputIterator, Context>{sink, context, c_type, is_out, is_return}(complex);
     // return as_generator(" System.IntPtr").generate(sink, attributes::unused, *context);
   }
};
      
} }

#endif
