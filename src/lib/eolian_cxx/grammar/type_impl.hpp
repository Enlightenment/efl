#ifndef EOLIAN_CXX_TYPE_IMPL_HH
#define EOLIAN_CXX_TYPE_IMPL_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "grammar/container.hpp"
#include "grammar/type.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename T>
T const* as_const_pointer(T* p) { return p; }

attributes::regular_type_def replace_base_type(attributes::regular_type_def v, std::string name)
{
  v.base_type = name;
  return v;
}

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

   typedef visitor_generate<OutputIterator, Context> visitor_type;
   typedef bool result_type;
  
   bool operator()(attributes::regular_type_def const& regular) const
   {
      using attributes::regular_type_def;
      struct match
      {
        eina::optional<std::string> name;
        eina::optional<bool> has_own;
        eina::optional<bool> is_ref;
        eina::optional<std::vector<std::string>> namespaces;
        std::function<attributes::type_def::variant_type()> function;
      }
      const match_table[] =
        {
           // signed primitives
             {"byte", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " char"); }}
           , {"llong", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " long long"); }}
           , {"int8", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " int8_t"); }}
           , {"int16", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " int16_t"); }}
           , {"int32", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " int32_t"); }}
           , {"int64", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " int64_t"); }}
           , {"ssize", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " ssize_t"); }}
           // unsigned primitives
           , {"ubyte", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " unsigned char"); }}
           , {"ushort", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " unsigned short"); }}
           , {"uint", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " unsigned int"); }}
           , {"ulong", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " unsigned long"); }}
           , {"ullong", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " unsigned long long"); }}
           , {"uint8", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " uint8_t"); }}
           , {"uint16", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " uint16_t"); }}
           , {"uint32", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " uint32_t"); }}
           , {"uint64", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " uint64_t"); }}
           , {"size", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " size_t"); }}
           , {"size", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " size_t"); }}
           , {"File", nullptr, nullptr, {{"Eina"}}, [&]
              {
                const char const_[] = "const ";
                if(regular.base_qualifier.qualifier & qualifier_info::is_const)
                  std::copy(&const_[0], &const_[0] + sizeof(const_) - 1, sink);
                const char name[] = "Eina_File*";
                std::copy(&name[0], &name[0] + sizeof(name) - 1, sink);
                if(is_out)
                  *sink++ = '*';
                return attributes::type_def::variant_type{};
              }}
           
           , {"ptrdiff", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " ptrdiff_t"); }}
           , {"intptr", nullptr, nullptr, nullptr, [&] { return replace_base_type(regular, " intptr_t"); }}
           , {"string", true, nullptr, nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                if(is_out || is_return)
                  return replace_base_type(r, " ::std::string");
                else return replace_base_type(r, " ::efl::eina::string_view");
              }}
           , {"string", false, nullptr, nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " ::efl::eina::string_view");
              }}
           , {"mstring", false, nullptr, nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier |= qualifier_info::is_ref;
                // r.base_qualifier.qualifier ^= qualifier_info::is_const;
                if(is_out || is_return)
                  return replace_base_type(r, " ::std::string");
                return replace_base_type(r, " ::efl::eina::string_view");
              }}
           , {"mstring", true, nullptr, nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                // r.base_qualifier.qualifier ^= qualifier_info::is_const;
                if(is_out || is_return)
                  return replace_base_type(r, " ::std::string");
                return replace_base_type(r, " ::efl::eina::string_view");
              }}
           , {"stringshare", nullptr, nullptr, nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " ::efl::eina::stringshare");
              }}
           , {"strbuf", nullptr, nullptr, nullptr, [&]
              {
                regular_type_def r = regular;
                r.base_qualifier.qualifier ^= qualifier_info::is_ref;
                return replace_base_type(r, " ::efl::eina::strbuf");
              }}
           /* FIXME: handle any_value_ptr */
           , {"any_value", true, nullptr, nullptr, [&]
              {
                return regular_type_def{" ::efl::eina::value", regular.base_qualifier ^ qualifier_info::is_ref, {}};
              }}
           , {"any_value", false, nullptr, nullptr, [&]
              { return regular_type_def{" ::efl::eina::value_view", regular.base_qualifier, {}};
              }}
           , {"any_value_ptr", true, nullptr, nullptr, [&]
              {
                return regular_type_def{" ::efl::eina::value", regular.base_qualifier ^ qualifier_info::is_ref, {}};
              }}
           , {"any_value_ptr", false, nullptr, nullptr, [&]
              { return regular_type_def{" ::efl::eina::value_view", regular.base_qualifier ^ qualifier_info::is_ref, {}};
              }}
        };
      if(regular.base_type == "void_ptr")
        {
          if(regular.base_qualifier & qualifier_info::is_ref)
            throw std::runtime_error("ref of void_ptr is invalid");
          return as_generator
             (
              lit("void") << (regular.base_qualifier & qualifier_info::is_const ? " const" : "")
              << "*"
              << (is_out ? "*" : "")
             )
             .generate(sink, attributes::unused, *context);
        }
      else if(eina::optional<bool> b = call_match
         (match_table
          , [&] (match const& m)
          {
            return (!m.name || *m.name == regular.base_type)
            && (!m.has_own || *m.has_own == (bool)(regular.base_qualifier & qualifier_info::is_own))
            && (!m.namespaces || *m.namespaces == regular.namespaces)
            && (!m.is_ref || *m.is_ref == (bool)(regular.base_qualifier & qualifier_info::is_ref))
            ;
          }
          , [&] (attributes::type_def::variant_type const& v)
          {
            if(!v.empty())
              return v.visit(*this); // we want to keep is_out info
            else
              return true;
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
      else if(regular.base_qualifier & qualifier_info::is_optional)
       {
         attributes::regular_type_def no_optional_regular = regular;
         no_optional_regular.base_qualifier.qualifier ^= qualifier_info::is_optional;
         if(is_out)
           {
             if(no_optional_regular.base_qualifier & qualifier_info::is_own)
               {
                 return as_generator(" ::efl::eina::optional<").generate(sink, attributes::unused, *context)
                   && (*this)(no_optional_regular)
                   && as_generator("&>").generate(sink, attributes::unused, *context);
              }
             else if(no_optional_regular.base_qualifier & qualifier_info::is_ref)
               {
                  no_optional_regular.base_qualifier.qualifier ^= qualifier_info::is_ref;
                  return (*this)(no_optional_regular)
                    && as_generator("**").generate(sink, attributes::unused, *context);
               }
             else
               return (*this)(no_optional_regular)
                 && as_generator("*").generate(sink, attributes::unused, *context);
           }
         else
           {
             // regular.base_qualifier & qualifier_info::is_ref
             return as_generator(" ::efl::eina::optional<").generate(sink, attributes::unused, *context)
               && (*this)(no_optional_regular)
               && as_generator(">").generate(sink, attributes::unused, *context);
           }
       }
      else if((is_return || is_out) && regular.base_qualifier & qualifier_info::is_ref
              && regular.base_qualifier & qualifier_info::is_own)
        {
          if(as_generator
             (
              " ::std::unique_ptr<"
              << *(string << "_")
              << string
              << (regular.base_qualifier & qualifier_info::is_const ? " const" : "")
              << ", ::efl::eina::malloc_deleter>"
             )
             .generate(sink, std::make_tuple(regular.namespaces, regular.base_type), *context))
            return true;
          else
            return false;
        }
      else
        {
          if(as_generator
             (
              *(string << "_")
              << string
              << (regular.base_qualifier & qualifier_info::is_const
                  || (regular.base_qualifier & qualifier_info::is_ref
                      && !is_return && !is_out)
                  ? " const" : "")
              << (regular.base_qualifier & qualifier_info::is_ref ? (regular.is_undefined ? "*" : "&") : "")
             )
             .generate(sink, std::make_tuple(regular.namespaces, regular.base_type), *context))
            return true;
          else
            return false;
        }
   }
   bool operator()(attributes::klass_name klass) const
   {
     return
       as_generator(" " << *("::" << lower_case[string]) << "::" << string)
       .generate(sink, std::make_tuple(attributes::cpp_namespaces(klass.namespaces), klass.eolian_name), *context)
       && (!(klass.base_qualifier & qualifier_info::is_ref)
           || as_generator("&").generate(sink, attributes::unused, *context));
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
        {"list", true, nullptr, [&]
         {
           generate_container(sink, complex, *context, " ::efl::eina::list");
           return attributes::type_def::variant_type();
         }}
        , {"list", false, nullptr, [&]
           {
           generate_container(sink, complex, *context, " ::efl::eina::range_list");
           return attributes::type_def::variant_type();
         }}
        , {"array", true, nullptr, [&]
           {
           generate_container(sink, complex, *context, " ::efl::eina::array");
           return attributes::type_def::variant_type();
         }}
        , {"array", false, nullptr, [&]
           {
           generate_container(sink, complex, *context, " ::efl::eina::range_array");
           return attributes::type_def::variant_type();
         }}
        , {"hash", nullptr, nullptr
           , [&]
           { regular_type_def r{"Eina_Hash*", complex.outer.base_qualifier, {}};
             return r;
           }}
        , {"promise", nullptr, nullptr, [&]
           {
             return replace_outer
             (complex, regular_type_def{" ::efl::promise", complex.outer.base_qualifier, {}});
           }           
          }
        , {"future", nullptr, nullptr, [&]
           {
             return replace_outer
             (complex, regular_type_def{" ::efl::eina::future", complex.outer.base_qualifier, {}});
           }
          }
        , {"iterator", nullptr, nullptr, [&]
           {
             return replace_outer
             (complex, regular_type_def{" ::efl::eina::iterator", complex.outer.base_qualifier, {}});
           }           
          }
        , {"accessor", nullptr, nullptr, [&]
           {
             return replace_outer
             (complex, regular_type_def{" ::efl::eina::accessor", complex.outer.base_qualifier, {}});
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
      
} } }

#endif
