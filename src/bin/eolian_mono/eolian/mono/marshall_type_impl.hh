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
                if (is_return || is_out)
                    return replace_base_type(r, " System.IntPtr");
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
                if (is_return || is_out)
                   return replace_base_type(r, " System.IntPtr");
                else
                   return replace_base_type(r, " System.String");
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
        ,{"inarray", nullptr, nullptr, [&]
           {
              return regular_type_def{" System.IntPtr", complex.outer.base_qualifier, {}};
           }
        }
        ,{"list", nullptr, nullptr, [&]
           {
              return regular_type_def{" System.IntPtr", complex.outer.base_qualifier, {}};
           }
        }
        ,{"inlist", nullptr, nullptr, [&]
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
      };

      auto default_match = [&] (attributes::complex_type_def const& complex)
        {
          regular_type_def no_pointer_regular = complex.outer;
          // std::vector<attributes::pointer_indirection> pointers;
          // pointers.swap(no_pointer_regular.pointers);
          // if(is_out)
          //   pointers.push_back({{attributes::qualifier_info::is_none, {}}, true});
          return visitor_type{sink, context, c_type, false}(no_pointer_regular)
            && as_generator("<" << (type % ", ") << ">").generate(sink, complex.subtypes, *context)
          ;
            // && detail::generate_pointers(sink, pointers, *context, false);
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

      //return default_match(complex);
     return visitor_generate<OutputIterator, Context>{sink, context, c_type, is_out, is_return}(complex);
     // return as_generator(" System.IntPtr").generate(sink, attributes::unused, *context);
   }
};
      
} }

#endif
