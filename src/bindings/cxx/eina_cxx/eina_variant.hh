#ifndef EINA_VARIANT_HH_
#define EINA_VARIANT_HH_

#include <cstddef>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <tuple>
#include <iosfwd>

#include <eina_aligned_union.hh>

namespace efl { namespace eina {

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

}
    
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
        
} }

#endif
