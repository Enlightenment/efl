#ifndef EOLIAN_CXX_TYPE_IMPL_HH
#define EOLIAN_CXX_TYPE_IMPL_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "grammar/container.hpp"
#include "grammar/type.hpp"

namespace efl { namespace eolian { namespace grammar {

namespace detail {

bool has_own(attributes::regular_type_def const& def)
{
  for(auto&& c : def.pointers)
    if(is_own(c.qualifier))
      return true;
  return false;
}
  
}
      
namespace detail {

struct swap_pointers_visitor
{
  std::vector<attributes::pointer_indirection>* pointers;
  typedef void result_type;
  template <typename T>
  void operator()(T& object) const
  {
     std::swap(*pointers, object.pointers);
  }
  void operator()(attributes::complex_type_def& complex) const
  {
    (*this)(complex.outer);
  }
};

template <typename OutputIterator, typename Context>
bool generate_pointers(OutputIterator sink, std::vector<attributes::pointer_indirection> const& pointers, Context const&
                       , bool no_reference)
{
   for(auto first = pointers.rbegin()
         , last = pointers.rend(); first != last; ++first)
     {
       if(std::next(first) == last && first->reference && !no_reference)
         *sink++ = '&';
       else
         *sink++ = '*';
     }
   return true;
}
  
}
      
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
           "void_ptr", nullptr, [&]
           {
             std::vector<attributes::pointer_indirection> pointers = regular.pointers;
             pointers.insert(pointers.begin(), {{attributes::qualifier_info::is_none, {}}, false});
             return attributes::regular_type_def{"void", regular.base_qualifier, pointers, {}};
           }
           // signed primitives
           , {"byte", nullptr, [&] { return replace_base_type(regular, " char"); }}
           , {"llong", nullptr, [&] { return replace_base_type(regular, " long long"); }}
           , {"int8", nullptr, [&] { return replace_base_type(regular, " int8_t"); }}
           , {"int16", nullptr, [&] { return replace_base_type(regular, " int16_t"); }}
           , {"int32", nullptr, [&] { return replace_base_type(regular, " int32_t"); }}
           , {"int64", nullptr, [&] { return replace_base_type(regular, " int64_t"); }}
           , {"ssize", nullptr, [&] { return replace_base_type(regular, " ssize_t"); }}
           // unsigned primitives
           , {"ubyte", nullptr, [&] { return replace_base_type(regular, " unsigned char"); }}
           , {"ushort", nullptr, [&] { return replace_base_type(regular, " unsigned short"); }}
           , {"uint", nullptr, [&] { return replace_base_type(regular, " unsigned int"); }}
           , {"ulong", nullptr, [&] { return replace_base_type(regular, " unsigned long"); }}
           , {"ullong", nullptr, [&] { return replace_base_type(regular, " unsigned long long"); }}
           , {"uint8", nullptr, [&] { return replace_base_type(regular, " uint8_t"); }}
           , {"uint16", nullptr, [&] { return replace_base_type(regular, " uint16_t"); }}
           , {"uint32", nullptr, [&] { return replace_base_type(regular, " uint32_t"); }}
           , {"uint64", nullptr, [&] { return replace_base_type(regular, " uint64_t"); }}
           , {"size", nullptr, [&] { return replace_base_type(regular, " size_t"); }}
           
           , {"ptrdiff", nullptr, [&] { return replace_base_type(regular, " ptrdiff_t"); }}
           , {"intptr", nullptr, [&] { return replace_base_type(regular, " intptr_t"); }}
           , {"string", true, [&] { return replace_base_type(regular, " ::std::string"); }}
           , {"string", false, [&] { return replace_base_type(regular, " ::efl::eina::string_view"); }}
           , {"generic_value", nullptr, [&]
              { return regular_type_def{" ::efl::eina::value", regular.base_qualifier
                                        , {regular.pointers.empty()
                                           || (regular.pointers.size() == 1 && regular.pointers[0].reference)
                                           ? regular.pointers
                                           : std::vector<attributes::pointer_indirection>
                                           {regular.pointers.begin(), std::prev(regular.pointers.end())}}
                                        , {}};
              }}
        };

      if(eina::optional<bool> b = call_match
         (match_table
          , [&] (match const& m)
          {
            return (!m.name || *m.name == regular.base_type)
            && (!m.has_own || *m.has_own == is_own(regular.base_qualifier))
            ;
          }
          , [&] (attributes::type_def::variant_type const& v)
          {
            return v.visit(*this); // we want to keep is_out info
          }))
        {
           return *b;
        }
      else if(attributes::is_optional(regular.base_qualifier))
       {
         if(regular.pointers.empty() || (regular.pointers.size() == 1 && regular.pointers[0].reference == true))
           {
             attributes::complex_type_def def
             {attributes::regular_type_def{" ::efl::eina::optional", attributes::qualifier_info::is_none, {}}};
             attributes::regular_type_def no_optional_regular = regular;
             attributes::remove_optional(no_optional_regular.base_qualifier);

             def.subtypes.push_back({no_optional_regular, c_type});
             return (*this)(def);
           }
         else
           {
             attributes::regular_type_def no_optional_regular = regular;
             attributes::remove_optional(no_optional_regular.base_qualifier);
             no_optional_regular.pointers[0].reference = 0;
             return (*this)(no_optional_regular);
           }
       }
     // else if(detail::has_own(regular) && !regular.pointers.empty())
     //   {
     //     attributes::complex_type_def def
     //     {attributes::regular_type_def{" ::efl::eolian::own_ptr", attributes::qualifier_info::is_none, {}}};

     //     attributes::complex_type_def tagged_def
     //     {attributes::regular_type_def{" ::efl::eolian::own", attributes::qualifier_info::is_none, {}}};
         
     //     auto pointer_iterator = regular.pointers.begin()
     //       , pointer_last = regular.pointers.end();

     //     for(;pointer_iterator != pointer_last && !attributes::is_own(pointer_iterator->qualifier)
     //           ;++pointer_iterator)
     //       {
     //         tagged_def.outer.pointers.push_back(*pointer_iterator);
     //         tagged_def.outer.pointers.front().reference = false;
     //       }

     //     assert(attributes::is_own(pointer_iterator->qualifier));

     //     attributes::regular_type_def base_type (regular);
     //     base_type.pointers.clear();

     //     for(;pointer_iterator != pointer_last; ++pointer_iterator)
     //       {
     //         base_type.pointers.insert(base_type.pointers.begin(), *pointer_iterator);
     //         attributes::remove_own(base_type.pointers.back().qualifier);
     //       }

     //     tagged_def.subtypes.push_back({base_type, c_type});
     //     def.subtypes.push_back({tagged_def, c_type});
     //     return (*this)(def);
     //   }
     else if(detail::has_own(regular) && !regular.pointers.empty())
       {
          attributes::regular_type_def pointee = regular;
          std::vector<attributes::pointer_indirection> pointers;
          std::swap(pointers, pointee.pointers);
          pointers.erase(pointers.begin());

          attributes::pointer_indirection reference {{attributes::qualifier_info::is_none,{}}, true};
          
          return as_generator(" ::std::unique_ptr<" << type).generate
            (sink, attributes::type_def{pointee, c_type}, *context)
            && detail::generate_pointers(sink, pointers, *context, true)
            && as_generator(", void(*)(const void*)>").generate(sink, attributes::unused, *context)
            && (!is_out || detail::generate_pointers(sink, {reference}, *context, false));
       }            
     else
       {
         auto pointers = regular.pointers;
         if(is_out)
           pointers.push_back({{attributes::qualifier_info::is_none,{}}, true});
         if(as_generator(*(string << "_") << string << (is_const(regular.base_qualifier)? " const" : ""))
            .generate(sink, std::make_tuple(regular.namespaces, regular.base_type), *context))
           return detail::generate_pointers(sink, pointers, *context
                                            , regular.base_type == "void");
         else
           return false;
       }
   }
   bool operator()(attributes::klass_name klass) const
   {
     if(is_out)
       klass.pointers.push_back({{attributes::qualifier_info::is_none, {}}, true});
     if(as_generator(" " << *("::" << lower_case[string]) << "::" << string)
        .generate(sink, std::make_tuple(attributes::cpp_namespaces(klass.namespaces), klass.eolian_name), *context))
       return detail::generate_pointers(sink, klass.pointers, *context, false);
     else
       return false;
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
           { regular_type_def r{"Eina_Hash", complex.outer.base_qualifier, complex.outer.pointers, {}};
             r.pointers.push_back({{qualifier_info::is_none, {}}, false});
             return r;
           }}
        , {"promise", nullptr, nullptr, [&]
           {
             return replace_outer
             (complex, regular_type_def{" ::efl::eina::future", complex.outer.base_qualifier, {}, {}});
           }           
          }
        , {"iterator", nullptr, nullptr, [&]
           {
             return replace_outer
             (complex, regular_type_def{" ::efl::eina::iterator", complex.outer.base_qualifier, {}, {}});
           }           
          }
        , {"accessor", nullptr, nullptr, [&]
           {
             return replace_outer
             (complex, regular_type_def{" ::efl::eina::accessor", complex.outer.base_qualifier, {}, {}});
           }           
          }
      };

      auto default_match = [&] (attributes::complex_type_def const& complex)
        {
          regular_type_def no_pointer_regular = complex.outer;
          std::vector<attributes::pointer_indirection> pointers;
          pointers.swap(no_pointer_regular.pointers);
          if(is_out)
            pointers.push_back({{attributes::qualifier_info::is_none, {}}, true});
          return visitor_type{sink, context, c_type, false}(no_pointer_regular)
            && as_generator("<" << (type % ", ") << ">").generate(sink, complex.subtypes, *context)
            && detail::generate_pointers(sink, pointers, *context, false);
        };
       
      if(eina::optional<bool> b = call_match
         (matches
          , [&] (match const& m)
          {
            return (!m.name || *m.name == complex.outer.base_type)
            && (!m.has_own || *m.has_own == is_own(complex.outer.base_qualifier))
            && (!m.is_const || *m.is_const == is_const(complex.outer.base_qualifier));
          }
          , [&] (attributes::type_def::variant_type const& v)
          {
            if(v.empty())
              return true;
            else if(attributes::complex_type_def const* complex
               = attributes::get<attributes::complex_type_def>(&v))
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
