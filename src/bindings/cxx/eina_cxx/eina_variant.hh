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
#ifndef EINA_VARIANT_HH_
#define EINA_VARIANT_HH_

#include <cstddef>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <tuple>
#include <iosfwd>

#include <eina_aligned_union.hh>
#include <eina_tuple.hh>

namespace efl { namespace eina {

template <typename... Args>
struct variant;

template <typename...Args>
struct variant_size : std::tuple_size<std::tuple<Args...>>::type
{
};

template <typename V>
struct variant_as_tuple;
    
namespace _impl {

template <typename T, typename U, typename...Others>
struct is_one_of : std::conditional<std::is_same<T, U>::value
                                    , std::is_same<T, U>
                                    , is_one_of<T, Others...> >::type::type
{};

template <typename T, typename U>
struct is_one_of<T, U> : std::is_same<T, U>
{};


template <std::size_t size, typename T, typename U, typename...Args>
struct find_impl : find_impl<size+1, T, Args...>
{};

template <std::size_t size, typename T, typename...Args>
struct find_impl<size, T, T, Args...> : std::integral_constant<std::size_t, size> {};
  
template <typename T, typename U, typename...Args>
struct find : find_impl<0u, T, U, Args...>
{};

template <std::size_t NT, std::size_t NV, typename TupleVariant, typename TypesFound>
struct visit_impl_meta_args
{
  typedef std::integral_constant<std::size_t, NT> current_type_index;
  typedef std::integral_constant<std::size_t, NV> current_variant_index;
  typedef TupleVariant variants;
  // typedef typename std::tuple_element<NV, variants>::type current_variant;
  // typedef typename variant_as_tuple<current_variant>::type current_variant_types;
  // typedef typename std::tuple_element<NT, current_variant_types>::type current_type;
  typedef TypesFound types_found;
};

template <typename T> struct current_variant_types
{
  typedef typename std::tuple_element<T::current_variant_index::value, typename T::variants>::type current_variant;
  typedef typename variant_as_tuple<current_variant>::type type;
};
}

// template <typename FoundTypes, std::size_t N, typename...Tuples>
// struct call_n_visitor;

// template <typename FoundTypes, std::size_t N, typename Tuple, typename...Tuples>
// struct call_n_visitor<FoundTypes, N, Tuple, Tuples...>
// {

      /*
   template <typename F>
   static typename F::result_type call(int type, void* buffer, F f)
   {
      if(type == N)
        {
          using std::tuple_element;
          typedef typename tuple_element<N, Tuple>::type type;
          type* o = static_cast<type*>(buffer);
          return f(*o);
        }
      else
        return call_visitor<N+1, L, Tuple>::call(type, buffer, f);
   }
      */
  
// template <typename FoundTypes, std::size_t L, typename Tuple>
// struct call_n_visitor<FoundTypes, L, L, Tuple>
// {
//   template <typename F, typename...Variants>
//   static typename F::result_type call(int, void const*, F, Variants&&... variants)
//     {
//        std::abort();
//     }
// };
    
template <std::size_t N, std::size_t L, typename Tuple>
struct call_visitor
{
   template <typename F>
   static typename F::result_type call(int type, void const* buffer, F f)
   {
      if(type == N)
        {
          using std::tuple_element;
          typedef typename tuple_element<N, Tuple>::type type;
          type const* o = static_cast<type const*>(buffer);
          return f(*o);
        }
      else
        return call_visitor<N+1, L, Tuple>::call(type, buffer, f);
   }
   template <typename F>
   static typename F::result_type call(int type, void* buffer, F f)
   {
      if(type == N)
        {
          using std::tuple_element;
          typedef typename tuple_element<N, Tuple>::type type;
          type* o = static_cast<type*>(buffer);
          return f(*o);
        }
      else
        return call_visitor<N+1, L, Tuple>::call(type, buffer, f);
   }
};

template <std::size_t L, typename Tuple>
struct call_visitor<L, L, Tuple>
{
    template <typename F>
    static typename F::result_type call(int, void const*, F)
    {
       std::abort();
    }
};

struct compare_equal_visitor
{
   void const* buffer;
   typedef bool result_type;
   template <typename T>
   bool operator()(T const& other) const
   {
      return *static_cast<T const*>(buffer) == other;
   }
};
        
struct copy_visitor
{
   typedef void result_type;
   void* buffer;
   template <typename T>
   void operator()(T const& other) const
   {
      new (buffer) T(other);
   }
};
    
struct move_visitor
{
   typedef void result_type;
   void* buffer;
   template <typename T>
   void operator()(T& other) const
   {
      typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
      new (buffer) type(std::move(other));
   }
};

struct assign_visitor
{
   typedef void result_type;
   void* buffer;
   template <typename T>
   void operator()(T const& other) const
   {
      typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
      type* assigned = static_cast<type*>(buffer);
      *assigned = other;
   }
};

struct move_assign_visitor
{
   typedef void result_type;
   void* buffer;
   template <typename T>
   void operator()(T& other) const
   {
      typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
      type* assigned = static_cast<type*>(buffer);
      *assigned = std::move(other);
   }
};
    
struct destroy_visitor
{
   typedef void result_type;
   template <typename T>
   void operator()(T&& other) const noexcept
   {
      typedef typename std::remove_cv<typename std::remove_reference<T>::type>::type type;
      other.~type();
   }
};

struct ostream_visitor
{
   std::ostream* s;
   typedef std::ostream& result_type;
   template <typename T>
   std::ostream& operator()(T const& other) const
   {
     return *s << other;
   }
};

template <typename T>
struct get_visitor
{
   typedef T* result_type;
   T* operator()(T& object) const
   {
     return &object;
   }
   template <typename U>
   T* operator()(U&) const { return nullptr; }
};
        
template <typename... Args>
struct variant
{
   typedef variant<Args...> _self_type; /**< Type for the optional class itself. */

   constexpr variant()
     : type(-1)
   {}

   template <typename T>
   variant(T object,
           typename std::enable_if<_impl::is_one_of
           <typename std::remove_cv<typename std::remove_reference<T>::type>::type, Args...>::value>::type* = 0)
     : type(_impl::find<typename std::remove_cv<typename std::remove_reference<T>::type>::type, Args...>::value)
   {
      construct(object);
   }

   variant(variant const& other)
     : type(other.type)
   {
     if(other.type != -1)
       other.visit(copy_visitor{static_cast<void*>(&buffer)});
   }
   variant& operator=(variant const& other)
   {
     if(type == other.type && type != -1)
       {
         other.visit(assign_visitor{static_cast<void*>(&buffer)});
       }
     else if(type != other.type)
       {
         if(type != -1)
           destroy_unsafe();
         type = other.type;
         other.visit(copy_visitor{static_cast<void*>(&buffer)});
       }
     return *this;
   }
   variant(variant&& other)
     : type(other.type)
   {
     if(other.type != -1)
       other.visit(move_visitor{static_cast<void*>(&buffer)});
   }
   variant& operator=(variant&& other)
   {
     if(type == other.type && type != -1)
       {
         other.visit(move_assign_visitor{static_cast<void*>(&buffer)});
       }
     else if(type != other.type)
       {
         if(type != -1)
           destroy_unsafe();
         type = other.type;
         other.visit(move_visitor{static_cast<void*>(&buffer)});
       }
     return *this;
   }
   ~variant()
   {
     if(type != -1)
       destroy_unsafe();
   }

   void destroy()
   {
     if(type != -1)
       {
         destroy_unsafe();
         type = -1;
       }
   }

   void destroy_unsafe()
   {
     visit_unsafe(destroy_visitor());
   }

   bool empty() const
   {
     return type == -1;
   }
  
   template <typename F>
   typename F::result_type visit(F f) const
   {
      if(type == -1)
        {
           throw std::runtime_error("variant is empty");
        }
      else
        return call_visitor<0u, sizeof...(Args), std::tuple<Args...>>::call(type, static_cast<const void*>(&buffer), f);
   }

   template <typename F>
   typename F::result_type visit(F f)
   {
      if(type == -1)
        {
           throw std::runtime_error("variant is empty");
        }
      else
        return call_visitor<0u, sizeof...(Args), std::tuple<Args...>>::call(type, static_cast<void*>(&buffer), f);
   }

   template <typename F>
   typename F::result_type visit_unsafe(F f) const
   {
     return call_visitor<0u, sizeof...(Args), std::tuple<Args...>>::call(type, static_cast<const void*>(&buffer), f);
   }

   template <typename F>
   typename F::result_type visit_unsafe(F f)
   {
     return call_visitor<0u, sizeof...(Args), std::tuple<Args...>>::call(type, static_cast<void*>(&buffer), f);
   }

   constexpr std::size_t index() const
   {
     return type;
   }
  
private:
   template <typename T>
   void construct(T object)
   {
     new (&buffer) T(std::move(object));
   }
  
   typedef typename eina::aligned_union<1, Args...>::type buffer_type;

   friend bool operator==(variant<Args...> const& lhs, variant<Args...> const& rhs)
   {
     return rhs.type == lhs.type
       && (rhs.type == -1
           || rhs.visit(compare_equal_visitor{&lhs.buffer}));
   }
   friend std::ostream& operator<<(std::ostream& s, variant<Args...> const& rhs)
   {
     return rhs.visit(ostream_visitor{&s});
   }
  
   int type;
   /**
    * Member variable for holding the contained value.
    */
   buffer_type buffer;

  template <typename V>
  friend struct variant_as_tuple;

  // template <std::size_t NT, std::size_t NV, typename F, typename Tuple, typename Types, typename TupleVariant, std::size_t...I>
  // friend typename F::result_type visit_impl2
  //   (std::false_type, std::false_type
  //    , _impl::visit_impl_meta_args<NT, NV, Tuple, Types>, F&& f, TupleVariant&& variants, index_sequence<I...>);

  // template <std::size_t NT, std::size_t NV, typename F, typename Tuple, typename Types, typename TupleVariant, std::size_t...I>
  // friend typename F::result_type visit_impl2
  // (std::true_type, std::false_type
  //  , _impl::visit_impl_meta_args<NT, NV, Tuple, Types>, F&& f, TupleVariant&& variants, index_sequence<I...>);
  // template <std::size_t NT, std::size_t NV, typename F, typename Tuple, typename Types, typename TupleVariant, std::size_t...I>
  // friend typename F::result_type visit_impl2
  // (std::false_type, std::true_type
  //  , _impl::visit_impl_meta_args<NT, NV, Tuple, Types>, F&& f, TupleVariant&& variants, index_sequence<I...>);
  
  // // template <std::size_t NT, std::size_t NV, typename F, typename Tuple, typename Types, typename TupleVariant>
  // // friend typename F::result_type visit_impl2
  // // (std::true_type, std::false_type
  // //    , _impl::visit_impl_meta_args<NT, NV, Tuple, Types>, F&& f, TupleVariant&& variants)
  // // {
  // // }
  
  // // template <typename F, typename...AllVariants, typename...Variants>
  // // friend typename F::result_type call
  // //   (std::true_type, int, F &&, Variants&&... variants)
  // // {
  // //   std::abort();
  // // }

  // // template <typename F, typename Variant, typename...Variants>
  // // friend typename F::result_type call (F&&f, Variant&& variant, Variants&&... variants)
  // // {
  // //   return call (std::integral_constant<bool, (N == variant_size<Variant>::value)>{}
  // //                , variant.type, std::forward<F>(f), std::forward<Variant>(variant), std::forward<Variants>(variants)...);
  // // }

  // // template <typename F, typename Variant, typename...Variants>
  // // friend typename F::result_type call (F&&f, Variant&& variant, Variants&&... variants)
  // // {
  // //   return call (std::integral_constant<bool, (N == variant_size<Variant>::value)>{}
  // //                , variant.type, std::forward<F>(f), std::forward<Variant>(variant), std::forward<Variants>(variants)...);
  // // }

  // template <typename F, typename...Variants>
  // friend typename F::result_type visit_impl (F&& f, Variants&&... variants);
};


template <typename...VArgs>
struct variant_as_tuple<variant<VArgs...>>
{
  typedef std::tuple<VArgs...> type;
};

template <typename...VArgs>
struct variant_as_tuple<const variant<VArgs...>>
{
  typedef std::tuple<VArgs...> type;
};

template <typename...VArgs>
struct variant_as_tuple<volatile variant<VArgs...>>
{
  typedef std::tuple<VArgs...> type;
};

template <typename...VArgs>
struct variant_as_tuple<const volatile variant<VArgs...>>
{
  typedef std::tuple<VArgs...> type;
};

template <typename...VArgs>
struct variant_as_tuple<variant<VArgs...>&>
{
  typedef std::tuple<VArgs...> type;
};

template <typename...VArgs>
struct variant_as_tuple<const variant<VArgs...>&>
{
  typedef std::tuple<VArgs...> type;
};

template <typename...VArgs>
struct variant_as_tuple<volatile variant<VArgs...>&>
{
  typedef std::tuple<VArgs...> type;
};

template <typename...VArgs>
struct variant_as_tuple<const volatile variant<VArgs...>&>
{
  typedef std::tuple<VArgs...> type;
};

template <typename...Args>
inline bool operator!=(variant<Args...>const& lhs, variant<Args...> const& rhs)
{
   return !(lhs == rhs);
}

template <typename T, typename...Args>
T* get(variant<Args...>* variant, typename std::enable_if<_impl::is_one_of
       <typename std::remove_cv<typename std::remove_reference<T>::type>::type, Args...>::value>::type* = 0)
{
   return variant->visit(get_visitor<T>{});
}
template <typename T, typename...Args>
T const* get(variant<Args...>const* variant, typename std::enable_if<_impl::is_one_of
       <typename std::remove_cv<typename std::remove_reference<T>::type>::type, Args...>::value>::type* = 0)
{
   return variant->visit(get_visitor<T const>{});
}
template <typename T, typename...Args>
T& get(variant<Args...>& variant, typename std::enable_if<_impl::is_one_of
       <typename std::remove_cv<typename std::remove_reference<T>::type>::type, Args...>::value>::type* = 0)
{
   T* p = variant.visit(get_visitor<T>{});
   if(p)
     return *p;
   else
     throw std::logic_error("");
}
template <typename T, typename...Args>
T const& get(variant<Args...>const& variant, typename std::enable_if<_impl::is_one_of
       <typename std::remove_cv<typename std::remove_reference<T>::type>::type, Args...>::value>::type* = 0)
{
   T const* p = variant.visit(get_visitor<T const>{});
   if(p)
     return *p;
   else
     throw std::logic_error("");
}

template <std::size_t NT, std::size_t NV, typename F, typename Tuple, typename Types, typename TupleVariant, std::size_t...I>
  typename F::result_type visit_impl2
  (std::false_type, std::true_type
     , _impl::visit_impl_meta_args<NT, NV, Tuple, Types>, F&& f, TupleVariant&& variants
   , eina::index_sequence<I...>)
  {
    return f (eina::get<typename std::tuple_element<I, Types>::type>(std::get<I>(variants))...);
  }

  template <std::size_t NT, std::size_t NV, typename F, typename Tuple, typename Types, typename TupleVariant, std::size_t...I>
  typename F::result_type visit_impl2
  (std::true_type, std::false_type
     , _impl::visit_impl_meta_args<NT, NV, Tuple, Types>, F&&, TupleVariant&&, eina::index_sequence<I...>)
  {
    std::abort();
  }


  template <std::size_t NT, std::size_t NV, typename F, typename Tuple, typename Types, typename TupleVariant
            , std::size_t...I>
  typename F::result_type visit_impl2 (std::false_type, std::false_type
                                       , _impl::visit_impl_meta_args<NT, NV, Tuple, Types>, F&& f, TupleVariant&& variants
                                       , index_sequence<I...>)
  {
    using std::tuple_element;
    typedef _impl::visit_impl_meta_args<NT, NV, Tuple, Types> meta_args;
    if(std::get<NV>(variants).index() == NT)
    {
      typedef typename _impl::current_variant_types<meta_args>::type variant_types;
      typedef typename tuple_element<NT, variant_types>::type type;
      std::integral_constant<bool, (std::tuple_size<Tuple>::value == NV+1)> is_true {};
      return visit_impl2( std::false_type{}
                          ,  is_true
                          , _impl::visit_impl_meta_args<0u, NV+1, Tuple, typename _mpl::push_back<Types, type>::type>{}
                          , std::forward<F>(f), std::forward<TupleVariant>(variants)
                          , make_index_sequence<std::tuple_size<TupleVariant>::value>{});
    }
    else
    {
      typedef typename _impl::current_variant_types<meta_args>::type variant_types;
      return visit_impl2 (std::integral_constant<bool, (std::tuple_size<variant_types>::value == NT+1)>{}
                         , std::false_type{}
                         , _impl::visit_impl_meta_args<NT+1, NV, Tuple, Types>{}, std::forward<F>(f), std::forward<TupleVariant>(variants)
                         , make_index_sequence<std::tuple_size<TupleVariant>::value>{});
    }
  }

  template <std::size_t NT, std::size_t NV, typename F, typename Tuple, typename Types, typename TupleVariant>
  typename F::result_type visit_impl_aux (std::false_type fals, std::false_type
                                       , _impl::visit_impl_meta_args<NT, NV, Tuple, Types> args, F&& f, TupleVariant&& variants)
  {
    return visit_impl2 (fals, fals, args, std::forward<F>(f), std::forward<TupleVariant>(variants)
                        , make_index_sequence<std::tuple_size<TupleVariant>::value>{});
  }

  template <typename F, typename...Variants>
  typename F::result_type visit_impl (F&& f, Variants&&... variants)
  {
    return visit_impl_aux
      (std::false_type{}
      , std::false_type{}
       , _impl::visit_impl_meta_args
       <0u, 0u
       , std::tuple<typename std::remove_cv<Variants>::type...>, std::tuple<>>{}, std::forward<F>(f), std::forward_as_tuple(std::forward<Variants>(variants)...));
  }

template <typename F, typename...Variants>
auto visit (F&& function, Variants&& ... variants) -> typename F::result_type
{
  return visit_impl (std::forward<F>(function), std::forward<Variants>(variants)...);
}
    
} }

#endif

