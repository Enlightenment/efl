#ifndef EOLIAN_MONO_MARSHALL_TYPE_IMPL_HH
#define EOLIAN_MONO_MARSHALL_TYPE_IMPL_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "helpers.hh"
#include "name_helpers.hh"
#include "type_impl.hh"
#include "generation_contexts.hh"
#include "blacklist.hh"

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
        std::function<attributes::type_def::variant_type()> function;
      }
      const match_table[] =
        {
           // signed primitives
           {"string", true, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           , {"string", false, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           , {"mstring", true, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           , {"mstring", false, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           , {"stringshare", true, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           , {"stringshare", false, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           , {"strbuf", nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " Eina.Strbuf");
              }}
           , {"Binbuf", true, [&]
              {
                regular_type_def r = regular;
                r.base_type = " System.IntPtr";
                r.namespaces.clear();
                return r;
              }}
           , {"Binbuf", false, [&]
              {
                regular_type_def r = regular;
                r.base_type = " System.IntPtr";
                r.namespaces.clear();
                return r;
              }}
           , {"any_value", true, [&]
               {
                regular_type_def r = regular;
                r.namespaces.clear();
                if (is_ptr)
                    r.base_type = " Eina.Value";
                else
                    r.base_type = " Eina.ValueNative";
                return r;
               }}
           , {"any_value", false, [&]
               {
                regular_type_def r = regular;
                r.namespaces.clear();
                if (is_ptr)
                    r.base_type = " Eina.Value";
                else
                    r.base_type = " Eina.ValueNative";
                return r;
               }}
           , {"any_value_ptr", true, [&]
               {
                regular_type_def r = regular;
                r.namespaces.clear();
                r.base_type = " Eina.Value";
                return r;
               }}
           , {"any_value_ptr", false, [&]
               {
                regular_type_def r = regular;
                r.namespaces.clear();
                r.base_type = " Eina.Value";
                return r;
               }}
           , {"void", nullptr, [&]
               {
                regular_type_def r = regular;
                r.namespaces.clear();
                if (is_out) // @inout too
                    r.base_type = " System.IntPtr";
                else
                    r.base_type = " void";
                return r;
               }}
        };

        if (regular.is_struct() && !blacklist::is_struct_blacklisted(regular) && !(bool)(regular.base_qualifier & qualifier_info::is_own))
          {
             if ((is_out || is_return) && is_ptr)
                 return as_generator(" System.IntPtr").generate(sink, attributes::unused, *context);
             return as_generator(string << "_StructInternal")
                    .generate(sink, name_helpers::type_full_managed_name(regular), *context);
          }
        else if (eina::optional<bool> b = call_match
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
      else if (is_ptr && helpers::need_pointer_conversion(&regular))
        {
           regular_type_def r = regular;
           r.base_type = " System.IntPtr";
           r.namespaces.clear();
           return visitor_generate<OutputIterator, Context>{sink, context, c_type, is_out, is_return, is_ptr}(r);
        }
      else
        {
          return visitor_generate<OutputIterator, Context>{sink, context, c_type, is_out, is_return, is_ptr}(regular);
        }
   }
   bool operator()(attributes::klass_name klass_name) const
   {
     return visitor_generate<OutputIterator, Context>{sink, context, c_type, is_out, is_return, is_ptr}(klass_name);
   }
   bool operator()(attributes::complex_type_def const& complex) const
   {
      using attributes::regular_type_def;
      using attributes::qualifier_info;
      struct match
      {
        eina::optional<std::string> name;
        eina::optional<bool> has_own;
        eina::optional<bool> is_const;
        std::function<attributes::type_def::variant_type()> function;
      } const matches[] =
      {
        {"array", nullptr, nullptr, [&]
           {
              return regular_type_def{" System.IntPtr", complex.outer.base_qualifier, {}};
           }
        }
        ,{"list", nullptr, nullptr, [&]
           {
              return regular_type_def{" System.IntPtr", complex.outer.base_qualifier, {}};
           }
        }
        ,{"hash", nullptr, nullptr, [&]
           {
              return regular_type_def{" System.IntPtr", complex.outer.base_qualifier, {}};
           }
        }
        ,{"iterator", nullptr, nullptr, [&]
           {
              return regular_type_def{" System.IntPtr", complex.outer.base_qualifier, {}};
           }
        }
        ,{"accessor", nullptr, nullptr, [&]
           {
              return regular_type_def{" System.IntPtr", complex.outer.base_qualifier, {}};
           }
        }
      };

      auto default_match = [&] (attributes::complex_type_def const& complex)
        {
          regular_type_def no_pointer_regular = complex.outer;
          return visitor_type{sink, context, c_type, false}(no_pointer_regular)
            && as_generator("<" << (type % ", ") << ">").generate(sink, complex.subtypes, *context);
        };

      if(eina::optional<bool> b = call_match
         (matches
          , [&] (match const& m)
          {
            return (!m.name || *m.name == complex.outer.base_type)
            && (!m.has_own || *m.has_own == bool(complex.outer.base_qualifier & qualifier_info::is_own))
            && (!m.is_const || *m.is_const == bool(complex.outer.base_qualifier & qualifier_info::is_const));
          }
          , [&] (attributes::type_def::variant_type const& v)
          {
            if(v.empty())
              return true;
            else if(attributes::complex_type_def const* complex
               = eina::get<attributes::complex_type_def>(&v))
              return default_match(*complex);
            else
              return v.visit(*this);
          }))
        {
           return *b;
        }

     return visitor_generate<OutputIterator, Context>{sink, context, c_type, is_out, is_return, is_ptr}(complex);
   }
};
} }

#endif
