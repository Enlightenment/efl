#ifndef EINA_OPTIONAL_HH_
#define EINA_OPTIONAL_HH_

#include <cstddef>
#include <algorithm>
#include <utility>

namespace efl_eina_swap_adl {

template <typename T>
void swap_impl(T& lhs, T& rhs)
{
  using namespace std;
  swap(lhs, rhs);
}

}

namespace efl { namespace eina {

template <typename T>
void adl_swap(T& lhs, T& rhs)
{
  ::efl_eina_swap_adl::swap_impl<T>(lhs, rhs);
}

template <typename T>
struct optional
{
   typedef optional<T> _self_type;

   optional(std::nullptr_t) : engaged(false)
   {}
   optional() : engaged(false)
   {}
   optional(T&& other) : engaged(false)
   {
      _construct(std::move(other));
   }
   optional(T const& other) : engaged(false)
   {
     _construct(std::move(other));
   }
   optional(optional<T> const& other)
     : engaged(false)
   {
      if(other.engaged) _construct(*other);
   }
   optional(optional<T>&& other)
     : engaged(false)
   {
      _construct(std::move(*other));
      other._destroy();
   }

   _self_type& operator=(optional<T>&& other)
   {
      _destroy();
      engaged = other.engaged;
      if(engaged)
        _construct(std::move(*other));
      other._destroy();
      return *this;
   }
   _self_type& operator=(optional<T>const& other)
   {
      optional<T> tmp(other);
      tmp.swap(*this);
      return *this;
   }

   ~optional()
   {
      _destroy();
   }

   explicit operator bool() const
   {
      return is_engaged();
   }
   bool operator!() const
   {
      bool b ( *this );
      return !b;
   }

   T* operator->()
   {
      assert(is_engaged());
      return static_cast<T*>(static_cast<void*>(&buffer));
   }
   T const* operator->() const
   {
      return const_cast<_self_type&>(*this).operator->();
   }

   T& operator*() { return get(); }
   T const& operator*() const { return get(); }

   T& get() { return *this->operator->(); }
   T const& get() const { return *this->operator->(); }

   void swap(optional<T>& other)
   {
      if(is_engaged() && other.is_engaged())
        {
           eina::adl_swap(**this, *other);
        }
      else if(is_engaged())
        {
          other._construct(std::move(**this));
          _destroy();
        }
      else if(other.is_engaged())
        {
           _construct(std::move(*other));
           other._destroy();
        }
   }

   bool is_engaged() const
   {
      return engaged;
   }
private:
   template <typename U>
   void _construct(U&& object)
   {
      assert(!is_engaged());
      new (&buffer) T(std::move(object));
      engaged = true;
   }
   void _destroy()
   {
     if(is_engaged())
       {
         static_cast<T*>(static_cast<void*>(&buffer))->~T();
         engaged = false;
       }
   }

   typedef typename std::aligned_storage
     <sizeof(T),std::alignment_of<T>::value>::type buffer_type;
   buffer_type buffer;
   bool engaged;
};

template <typename T>
void swap(optional<T>& lhs, optional<T>& rhs)
{
   lhs.swap(rhs);
}

template <typename T>
bool operator==(optional<T> const& lhs, optional<T> const& rhs)
{
   if(!lhs && !rhs)
     return true;
   else if(!lhs || !rhs)
     return false;
   else
     return *lhs == *rhs;
}
template <typename T>
bool operator!=(optional<T> const& lhs, optional<T> const& rhs)
{
   return !(lhs == rhs);
}
template <typename T>
bool operator<(optional<T> const& lhs, optional<T> const& rhs)
{
   if(!lhs && !rhs)
     return false;
   else if(!lhs)
     return true;
   else if(!rhs)
     return false;
   else
     return *lhs < *rhs;
}
template <typename T>
bool operator<=(optional<T> const& lhs, optional<T> const& rhs)
{
   return lhs < rhs || lhs == rhs;
}
template <typename T>
bool operator>(optional<T> const& lhs, optional<T> const& rhs)
{
   return !(lhs <= rhs);
}
template <typename T>
bool operator>=(optional<T> const& lhs, optional<T> const& rhs)
{
   return !(lhs < rhs);
}

} } // efl::eina

#endif // EINA_OPTIONAL_HH_
