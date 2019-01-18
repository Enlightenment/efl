#ifndef EOLIAN_MONO_TYPE_IMPL_HH
#define EOLIAN_MONO_TYPE_IMPL_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "name_helpers.hh"

namespace eolian_mono {

namespace eina = efl::eina;

template <typename T>
T const* as_const_pointer(T* p) { return p; }

inline
attributes::regular_type_def replace_base_type(attributes::regular_type_def v, std::string name)
{
  v.base_type = name;
  return v;
}

template <typename T>
attributes::regular_type_def replace_base_integer(attributes::regular_type_def v)
{
  bool s = std::is_signed<T>::value;
  switch (sizeof(T))
  {
  case 1: return s ? replace_base_type(v, " sbyte") : replace_base_type(v, " byte");
  case 2: return s ? replace_base_type(v, " short") : replace_base_type(v, " ushort");
  case 4: return s ? replace_base_type(v, " int") : replace_base_type(v, " uint");
  case 8: return s ? replace_base_type(v, " long") : replace_base_type(v, " ulong");
  default: return v;
  }
}

inline
attributes::complex_type_def replace_outer(attributes::complex_type_def v, attributes::regular_type_def const& regular)
{
  v.outer = regular;
  return v;
}
      
template <typename Array, typename F, int N, typename A>
eina::optional<bool> call_match(Array const (&array)[N], F f, A a)
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
struct visitor_generate
{
   mutable OutputIterator sink;
   Context const* context;
   std::string c_type;
   bool is_out;
   bool is_return;
   bool is_ptr;

   typedef visitor_generate<OutputIterator, Context> visitor_type;
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
             {"byte", nullptr, [&] { return replace_base_type(regular, " sbyte"); }}
           , {"short", nullptr, [&] { return replace_base_integer<short>(regular); }}
           , {"int", nullptr, [&] { return replace_base_integer<int>(regular); }}
           , {"long", nullptr, [&] { return replace_base_integer<long>(regular); }}
           , {"llong", nullptr, [&] { return replace_base_integer<long long>(regular); }}
           , {"int8", nullptr, [&] { return replace_base_type(regular, " sbyte"); }}
           , {"int16", nullptr, [&] { return replace_base_type(regular, " short"); }}
           , {"int32", nullptr, [&] { return replace_base_type(regular, " int"); }}
           , {"int64", nullptr, [&] { return replace_base_type(regular, " long"); }}
           , {"ssize", nullptr, [&] { return replace_base_integer<ssize_t>(regular); }}
           // unsigned primitives
           , {"ubyte", nullptr, [&] { return replace_base_type(regular, " byte"); }}
           , {"ushort", nullptr, [&] { return replace_base_integer<unsigned short>(regular); }}
           , {"uint", nullptr, [&] { return replace_base_integer<unsigned int>(regular); }}
           , {"ulong", nullptr, [&] { return replace_base_integer<unsigned long>(regular); }}
           , {"ullong", nullptr, [&] { return replace_base_integer<unsigned long long>(regular); }}
           , {"uint8", nullptr, [&] { return replace_base_type(regular, " byte"); }}
           , {"uint16", nullptr, [&] { return replace_base_type(regular, " ushort"); }}
           , {"uint32", nullptr, [&] { return replace_base_type(regular, " uint"); }}
           , {"uint64", nullptr, [&] { return replace_base_type(regular, " ulong"); }}
           , {"size", nullptr, [&] { return replace_base_integer<size_t>(regular); }}
           
           , {"ptrdiff", nullptr, [&] { return replace_base_integer<ptrdiff_t>(regular); }}
           , {"intptr", nullptr, [&] { return replace_base_type(regular, " System.IntPtr"); }}
           , {"uintptr", nullptr, [&] { return replace_base_type(regular, " System.IntPtr"); }}
           , {"void_ptr", nullptr, [&] { return replace_base_type(regular, " System.IntPtr"); }}
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
           , {"Eina.Error", nullptr, [&] // Eina.Error
              {
                return regular_type_def{" Eina.Error", regular.base_qualifier, {}};
              }} // TODO
           , {"string", nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " System.String");
              }}
           , {"mstring", nullptr, [&]
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
           , {"strbuf", nullptr, [&]
              {
                return regular_type_def{" Eina.Strbuf", regular.base_qualifier, {}};
              }}
           , {"any_value", true, [&]
              { return regular_type_def{" Eina.Value", regular.base_qualifier, {}};
              }}
           , {"any_value", false, [&]
              { return regular_type_def{" Eina.Value", regular.base_qualifier, {}};
              }}
           , {"any_value_ptr", nullptr, [&] 
              { return regular_type_def{" Eina.Value", regular.base_qualifier, {}};
              }} // FIXME add proper support for any_value_ptr
        };
        std::string full_type_name = name_helpers::type_full_eolian_name(regular);
        if(eina::optional<bool> b = call_match
         (match_table
          , [&] (match const& m)
          {
            return (!m.name || *m.name == regular.base_type || *m.name == full_type_name)
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
      // in A @optional -> optional<A>
      // in A& @optional -> optional<A&>
      // in A& @optional -> optional<A&>
      // in own(A&) @optional -> A*
      //
      // out A @optional -> optional<A&>
      // out A& @optional -> optional<A&>
      // out own(A&) @optional -> optional<A*&>
      // else if(regular.base_qualifier & qualifier_info::is_optional)
      //  {
      //    attributes::regular_type_def no_optional_regular = regular;
      //    no_optional_regular.base_qualifier.qualifier ^= qualifier_info::is_optional;
      //    if(is_out)
      //      {
      //        if(no_optional_regular.base_qualifier & qualifier_info::is_own)
      //          {
      //            return as_generator(" ::efl::eina::optional<").generate(sink, attributes::unused, *context)
      //              && (*this)(no_optional_regular)
      //              && as_generator("&>").generate(sink, attributes::unused, *context);
      //         }
      //        else if(no_optional_regular.base_qualifier & qualifier_info::is_ref)
      //          {
      //             no_optional_regular.base_qualifier.qualifier ^= qualifier_info::is_ref;
      //             return (*this)(no_optional_regular)
      //               && as_generator("**").generate(sink, attributes::unused, *context);
      //          }
      //        else
      //          return (*this)(no_optional_regular)
      //            && as_generator("*").generate(sink, attributes::unused, *context);
      //      }
      //    else
      //      {
      //        // regular.base_qualifier & qualifier_info::is_ref
      //        return as_generator(" ::efl::eina::optional<").generate(sink, attributes::unused, *context)
      //          && (*this)(no_optional_regular)
      //          && as_generator(">").generate(sink, attributes::unused, *context);
      //      }
      //  }
      // else if((is_return || is_out) && regular.base_qualifier & qualifier_info::is_ref
      //         && regular.base_qualifier & qualifier_info::is_own)
      //   {
      //     if(as_generator
      //        (
      //         " ::std::unique_ptr<"
      //         << *(string << "_")
      //         << string
      //         << (regular.base_qualifier & qualifier_info::is_const ? " const" : "")
      //         << ", ::efl::eina::malloc_deleter>"
      //        )
      //        .generate(sink, std::make_tuple(regular.namespaces, regular.base_type), *context))
      //       return true;
      //     else
      //       return false;
      //   }
      // else if(Eolian_Typedecl const* typedecl = eolian_state_struct_by_name_get(c_type.c_str()))
      //   {
      //   return as_generator
      //        (
      //         *(string << ".")
      //         << string
      //        )
      //     .generate(sink, std::make_tuple(regular.namespaces, regular.base_type), *context);
      //   }
      else
        {
          return as_generator(string).generate(sink, name_helpers::type_full_managed_name(regular), *context);
        }
   }
   bool operator()(attributes::klass_name klass) const
   {
     if(klass.type == attributes::class_type::regular || klass.type == attributes::class_type::abstract_)
       return as_generator(string).generate(sink, name_helpers::klass_full_concrete_name(klass), *context);
     else
       return as_generator(string).generate(sink, name_helpers::klass_full_interface_name(klass), *context);
   }
   bool operator()(attributes::complex_type_def const& complex) const
   {
      using attributes::regular_type_def;
      using attributes::complex_type_def;
      using attributes::qualifier_info;
       struct match
      {
        eina::optional<std::string> name;
        eina::optional<bool> has_own;
        eina::optional<bool> is_const;
        std::function<attributes::type_def::variant_type()> function;
      } const matches[] =
      {
        {"list", nullptr, nullptr, [&]
         {
           complex_type_def c = complex;
           c.outer.base_type = "Eina.List";
           return c;
         }}
        , {"inlist", nullptr, nullptr, [&]
           {
           complex_type_def c = complex;
           c.outer.base_type = "Eina.Inlist";
           return c;
         }}
        , {"array", nullptr, nullptr, [&]
           {
           complex_type_def c = complex;
           c.outer.base_type = "Eina.Array";
           return c;
         }}
        , {"inarray", nullptr, nullptr, [&]
           {
           complex_type_def c = complex;
           c.outer.base_type = "Eina.Inarray";
           return c;
         }}
        , {"hash", nullptr, nullptr
           , [&]
           {
             complex_type_def c = complex;
             c.outer.base_type = "Eina.Hash";
             return c;
         }}
        , {"future", nullptr, nullptr, [&]
           {
             (*this)(regular_type_def{" Eina.Future", complex.outer.base_qualifier, {}});
             return attributes::type_def::variant_type();
           }           
          }
        , {"iterator", nullptr, nullptr, [&]
           {
             complex_type_def c = complex;
             c.outer.base_type = "Eina.Iterator";
             return c;
           }           
          }
        , {"accessor", nullptr, nullptr, [&]
           {
             complex_type_def c = complex;
             c.outer.base_type = "Eina.Accessor";
             return c;
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
        return *b;
      else
        {
          return default_match(complex);
        }
   }
};
      
}

#endif
