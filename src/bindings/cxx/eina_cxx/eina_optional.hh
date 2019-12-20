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
#ifndef EINA_OPTIONAL_HH_
#define EINA_OPTIONAL_HH_

#include <cstddef>
#include <algorithm>
#include <utility>
#include <type_traits>

/**
 * @addtogroup Eina_Cxx_Data_Types_Group
 *
 * @{
 */

namespace efl_eina_swap_adl {

/**
 * @internal
 */
template <typename T>
void swap_impl(T& lhs, T& rhs)
{
  using namespace std;
  swap(lhs, rhs);
}

}

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Optional_Group Optional Value
 * @ingroup Eina_Cxx_Data_Types_Group
 *
 * @{
 */

/**
 * @internal
 */
template <typename T>
void adl_swap(T& lhs, T& rhs)
{
  ::efl_eina_swap_adl::swap_impl<T>(lhs, rhs);
}

/**
 * This class manages an optional contained value, i.e. a value that
 * semantically may not be present.
 *
 * A common use case for optional is the return value of a function that
 * may fail. As opposed to other approaches, such as
 * <tt>std::pair<T,bool></tt>, optional handles expensive to construct
 * objects well and is more readable, as the intent is expressed
 * explicitly.
 *
 * An optional object holding a semantically present value is considered
 * to be @em engaged, otherwise it is considered to be @em disengaged.
 */
template <typename T>
struct optional
{
   typedef optional<T> _self_type; /**< Type for the optional class itself. */

   /**
    * @brief Create a disengaged object.
    *
    * This constructor creates a disengaged <tt>eina::optional</tt>
    * object.
    */
   constexpr optional(std::nullptr_t) : engaged(false)
   {}

   /**
    * @brief Default constructor. Create a disengaged object.
    */
   constexpr optional() : engaged(false)
   {}

   /**
    * @brief Create an engaged object by moving @p other content.
    * @param other R-value reference to the desired type.
    *
    * This constructor creates an <tt>eina::optional</tt> object in an
    * engaged state. The contained value is initialized by moving
    * @p other.
    */
   optional(T&& other) : engaged(false)
   {
      _construct(std::forward<T>(other));
   }

   /**
    * @brief Create an engaged object by copying @p other content.
    * @param other Constant reference to the desired type.
    *
    * This constructor creates an <tt>eina::optional</tt> object in an
    * engaged state. The contained value is initialized by copying
    * @p other.
    */
   optional(T const& other) : engaged(false)
   {
     _construct(other);
   }

   /**
    * @brief Create an engaged object by moving @p other content.
    * @param other R-value reference to the desired type.
    *
    * This constructor creates an <tt>eina::optional</tt> object in an
    * engaged state. The contained value is initialized by moving
    * @p other.
    */
   template <typename U>
   optional(U&& other, typename std::enable_if<std::is_convertible<U, T>::value>::type* = 0) : engaged(false)
   {
     _construct(std::forward<U>(other));
   }

   /**
    * @brief Create an engaged object by copying @p other content.
    * @param other Constant reference to the desired type.
    *
    * This constructor creates an <tt>eina::optional</tt> object in an
    * engaged state. The contained value is initialized by copying
    * @p other.
    */
   template <typename U>
   optional(U const& other, typename std::enable_if<std::is_convertible<U, T>::value>::type* = 0) : engaged(false)
   {
     _construct(other);
   }

   /**
    * @brief Copy constructor. Create an object containing the same value as @p other and in the same state.
    * @param other Constant reference to another <tt>eina::optional</tt> object that holds the same value type.
    *
    * This constructor creates an <tt>eina::optional</tt> object with
    * the same engagement state of @p other. If @p other is engaged then
    * the contained value of the newly created object is initialized by
    * copying the contained value of @p other.
    */
   optional(optional<T> const& other)
     : engaged(false)
   {
      if(other.engaged) _construct(*other);
   }

   /**
    * @brief Move constructor. Create an object containing the same value as @p other and in the same state.
    * @param other R-value reference to another <tt>eina::optional</tt> object that holds the same value type.
    *
    * This constructor creates an <tt>eina::optional</tt> object with
    * the same engagement state of @p other. If @p other is engaged then
    * the contained value of the newly created object is initialized by
    * moving the contained value of @p other.
    */
   optional(optional<T>&& other)
     : engaged(false)
   {
      if(other.engaged) _construct(std::move(*other));
      other._destroy();
   }

   /**
    * @brief Move constructor. Create an object containing the same value as @p other and in the same state.
    * @param other R-value reference to another <tt>eina::optional</tt> object that holds a different, but convertible, value type.
    *
    * This constructor creates an <tt>eina::optional</tt> object with
    * the same engagement state of @p other. If @p other is engaged then
    * the contained value of the newly created object is initialized by
    * moving the contained value of @p other.
    */
   template <typename U>
   optional(optional<U>&& other, typename std::enable_if<std::is_convertible<U, T>::value>::type* = 0)
     : engaged(false)
   {
      if (other.is_engaged()) _construct(std::move(*other));
      other.disengage();
   }

   /**
    * @brief Copy constructor. Create an object containing the same value as @p other and in the same state.
    * @param other Constant reference to another <tt>eina::optional</tt> object that holds a different, but convertible, value type.
    *
    * This constructor creates an <tt>eina::optional</tt> object with
    * the same engagement state of @p other. If @p other is engaged then
    * the contained value of the newly created object is initialized by
    * converting and copying the contained value of @p other.
    */
   template <typename U>
   optional(optional<U> const& other, typename std::enable_if<std::is_convertible<U, T>::value>::type* = 0)
     : engaged(false)
   {
      if (other.is_engaged()) _construct(*other);
   }

   /**
    * @brief Assign new content to the object.
    * @param other R-value reference to another <tt>eina::optional</tt> object that holds the same value type.
    *
    * This operator replaces the current content of the object. If
    * @p other is engaged its contained value is moved to this object,
    * making <tt>*this</tt> be considered engaged too. If @p other is
    * disengaged <tt>*this</tt> is also made disengaged and its
    * contained value, if any, is simple destroyed.
    */
   _self_type& operator=(optional<T>&& other)
   {
      _destroy();
      if (other.engaged)
        _construct(std::move(*other));
      other._destroy();
      return *this;
   }

   /**
    * @brief Assign new content to the object.
    * @param other Constant reference to another <tt>eina::optional</tt> object that holds the same value type.
    *
    * This operator replaces the current content of the object. If
    * @p other is engaged its contained value is copied to this object,
    * making <tt>*this</tt> be considered engaged too. If @p other is
    * disengaged <tt>*this</tt> is also made disengaged and its
    * contained value, if any, is simple destroyed.
    */
   _self_type& operator=(optional<T>const& other)
   {
      optional<T> tmp(other);
      tmp.swap(*this);
      return *this;
   }

   /**
    * @brief Assign new content to the object.
    * @param other R-value reference to another <tt>eina::optional</tt> object that holds a different, but convertible, value type.
    *
    * This operator replaces the current content of the object. If
    * @p other is engaged its contained value is moved to this object,
    * making <tt>*this</tt> be considered engaged too. If @p other is
    * disengaged <tt>*this</tt> is also made disengaged and its
    * contained value, if any, is simple destroyed.
    */
   template <typename U>
   typename std::enable_if<std::is_convertible<U, T>::value, _self_type>::type& operator=(optional<U>&& other)
   {
      _destroy();
      if (other.is_engaged())
        _construct(std::move(*other));
      other.disengage();
      return *this;
   }

   /**
    * @brief Assign new content to the object.
    * @param other Constant reference to another <tt>eina::optional</tt> object that holds a different, but convertible, value type.
    *
    * This operator replaces the current content of the object. If
    * @p other is engaged its contained value is converted and copied to this
    * object, making <tt>*this</tt> be considered engaged too. If @p other is
    * disengaged <tt>*this</tt> is also made disengaged and its
    * contained value, if any, is simple destroyed.
    */
   template <typename U>
   typename std::enable_if<std::is_convertible<U, T>::value, _self_type>::type& operator=(optional<U>const& other)
   {
      _destroy();
      if (other.is_engaged())
        _construct(*other);
      return *this;
   }

   /**
    * @brief Disengage the object, destroying the current contained value, if any.
    */
   void disengage()
   {
      _destroy();
   }

   /**
    * @brief Releases the contained value if the object is engaged.
    */
   ~optional()
   {
      _destroy();
   }

   /**
    * @brief Convert to @c bool based on whether the object is engaged or not.
    * @return @c true if the object is engaged, @c false otherwise.
    */
   explicit operator bool() const
   {
      return is_engaged();
   }

   /**
    * @brief Convert to @c bool based on whether the object is engaged or not.
    * @return @c true if the object is disengaged, @c false otherwise.
    */
   bool operator!() const
   {
      bool b ( *this );
      return !b;
   }

   /**
    * @brief Access member of the contained value.
    * @return Pointer to the contained value, whose member will be accessed.
    */
   T* operator->()
   {
      assert(is_engaged());
      return static_cast<T*>(static_cast<void*>(&buffer));
   }

   /**
    * @brief Access constant member of the contained value.
    * @return Constant pointer to the contained value, whose member will be accessed.
    */
   T const* operator->() const
   {
      return const_cast<_self_type&>(*this).operator->();
   }

   /**
    * @brief Get the contained value.
    * @return Reference to the contained value.
    */
   T& operator*() { return get(); }

   /**
    * @brief Get the contained value.
    * @return Constant reference to the contained value.
    */
   T const& operator*() const { return get(); }

   /**
    * @brief Get the contained value.
    * @return Reference to the contained value.
    */
   T& get() { return *this->operator->(); }

   /**
    * @brief Get the contained value.
    * @return Constant reference to the contained value.
    */
   T const& get() const { return *this->operator->(); }

   /**
    * @brief Swap content with another <tt>eina::optional</tt> object.
    * @param other Another <tt>eina::optional</tt> object.
    */
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

   /**
    * @brief Check if the object is engaged.
    * @return @c true if the object is currently engaged, @c false otherwise.
    */
   bool is_engaged() const
   {
      return engaged;
   }
private:

   /**
    * @internal
    */
   template <typename U>
   void _construct(U&& object)
   {
      assert(!is_engaged());
      // NOTE: the buffer memory is intended to be in an
      // uninitialized state here.
      // So this warning can be disabled.
#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
      new (&buffer) T(std::forward<U>(object));
#pragma GCC diagnostic pop
      engaged = true;
   }

   /**
    * @internal
    */
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

   /**
    * Member variable for holding the contained value.
    */
   buffer_type buffer;

   /**
    * Flag to tell whether the object is engaged or not.
    */
   bool engaged;
};

template <typename T>
struct optional<T&>
{
   typedef optional<T&> _self_type; /**< Type for the optional class itself. */

   /**
    * @brief Create a disengaged object.
    *
    * This constructor creates a disengaged <tt>eina::optional</tt>
    * object.
    */
   constexpr optional(std::nullptr_t) : pointer(nullptr)
   {}

   /**
    * @brief Default constructor. Create a disengaged object.
    */
   constexpr optional() : pointer(nullptr)
   {}

   /**
    * @brief Create an engaged object by moving @p other content.
    * @param other R-value reference to the desired type.
    *
    * This constructor creates an <tt>eina::optional</tt> object in an
    * engaged state. The contained value is initialized by moving
    * @p other.
    */
   optional(T& other) : pointer(&other)
   {
   }

   /**
    * @brief Copy constructor. Create an object containing the same value as @p other and in the same state.
    * @param other Constant reference to another <tt>eina::optional</tt> object that holds the same value type.
    *
    * This constructor creates an <tt>eina::optional</tt> object with
    * the same engagement state of @p other. If @p other is engaged then
    * the contained value of the newly created object is initialized by
    * copying the contained value of @p other.
    */
   optional(_self_type const& other) = default;

   /**
    * @brief Assign new content to the object.
    * @param other Constant reference to another <tt>eina::optional</tt> object that holds the same value type.
    *
    * This operator replaces the current content of the object. If
    * @p other is engaged its contained value is copied to this object,
    * making <tt>*this</tt> be considered engaged too. If @p other is
    * disengaged <tt>*this</tt> is also made disengaged and its
    * contained value, if any, is simple destroyed.
    */
   _self_type& operator=(_self_type const& other) = default;

   /**
    * @brief Disengage the object, destroying the current contained value, if any.
    */
   void disengage()
   {
      pointer = NULL;
   }

   /**
    * @brief Convert to @c bool based on whether the object is engaged or not.
    * @return @c true if the object is engaged, @c false otherwise.
    */
   explicit operator bool() const
   {
      return pointer;
   }

   /**
    * @brief Convert to @c bool based on whether the object is engaged or not.
    * @return @c true if the object is disengaged, @c false otherwise.
    */
   bool operator!() const
   {
      bool b ( *this );
      return !b;
   }

   /**
    * @brief Access member of the contained value.
    * @return Pointer to the contained value, whose member will be accessed.
    */
   T* operator->()
   {
      assert(is_engaged());
      return pointer;
   }

   /**
    * @brief Access constant member of the contained value.
    * @return Constant pointer to the contained value, whose member will be accessed.
    */
   T const* operator->() const
   {
      return pointer;
   }

   /**
    * @brief Get the contained value.
    * @return Reference to the contained value.
    */
   T& operator*() { return get(); }

   /**
    * @brief Get the contained value.
    * @return Constant reference to the contained value.
    */
   T const& operator*() const { return get(); }

   /**
    * @brief Get the contained value.
    * @return Reference to the contained value.
    */
   T& get() { return *this->operator->(); }

   /**
    * @brief Get the contained value.
    * @return Constant reference to the contained value.
    */
   T const& get() const { return *this->operator->(); }

   /**
    * @brief Swap content with another <tt>eina::optional</tt> object.
    * @param other Another <tt>eina::optional</tt> object.
    */
   void swap(optional<T>& other)
   {
      std::swap(pointer, other.pointer);
   }

   /**
    * @brief Check if the object is engaged.
    * @return @c true if the object is currently engaged, @c false otherwise.
    */
   bool is_engaged() const
   {
      return pointer;
   }
private:

   /**
    * Member variable for holding the contained value.
    */
   T* pointer;
};
    
template <typename T>
constexpr optional<typename std::decay<T>::type>
make_optional(T&& value)
{
   return optional<typename std::decay<T>::type>(std::forward<T>(value));
}

/**
 * @brief Swap content with another <tt>eina::optional</tt> object.
 *
 */
template <typename T>
void swap(optional<T>& lhs, optional<T>& rhs)
{
   lhs.swap(rhs);
}

/**
 * @brief Check if both <tt>eina::optional</tt> object are equal.
 * @param lhs <tt>eina::optional</tt> object at the left side of the expression.
 * @param rhs <tt>eina::optional</tt> object at the right side of the expression.
 * @return @c true if both are objects are disengaged of if both objects
 *            are engaged and contain the same value, @c false in all
 *            other cases.
 */
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

/**
 * @brief Check if the <tt>eina::optional</tt> objects are different.
 * @param lhs <tt>eina::optional</tt> object at the left side of the expression.
 * @param rhs <tt>eina::optional</tt> object at the right side of the expression.
 * @return The opposite of @ref operator==(optional<T> const& lhs, optional<T> const& rhs).
 */
template <typename T>
bool operator!=(optional<T> const& lhs, optional<T> const& rhs)
{
   return !(lhs == rhs);
}

/**
 * @brief Less than comparison between <tt>eina::optional</tt> objects.
 * @param lhs <tt>eina::optional</tt> object at the left side of the expression.
 * @param rhs <tt>eina::optional</tt> object at the right side of the expression.
 * @return @c true if both objects are engaged and the contained value
 *         of @p lhs is less than the contained value of @p rhs, or if
 *         only @p lhs is disengaged. In all other cases returns
 *         @c false.
 */
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

/**
 * @brief Less than or equal comparison between <tt>eina::optional</tt> objects.
 * @param lhs <tt>eina::optional</tt> object at the left side of the expression.
 * @param rhs <tt>eina::optional</tt> object at the right side of the expression.
 * @return @c true if @p lhs is disengaged or if both objects are
 *         engaged and the contained value of @p lhs is less than or
 *         equal to the contained value of @p rhs. In all other cases
 *         returns @c false.
 */
template <typename T>
bool operator<=(optional<T> const& lhs, optional<T> const& rhs)
{
   return lhs < rhs || lhs == rhs;
}

/**
 * @brief More than comparison between <tt>eina::optional</tt> objects.
 * @param lhs <tt>eina::optional</tt> object at the left side of the expression.
 * @param rhs <tt>eina::optional</tt> object at the right side of the expression.
 * @return @c true if both objects are engaged and the contained value
 *         of @p lhs is more than the contained value of @p rhs, or if
 *         only @p rhs is disengaged. In all other cases returns
 *         @c false.
 */
template <typename T>
bool operator>(optional<T> const& lhs, optional<T> const& rhs)
{
   return !(lhs <= rhs);
}

/**
 * @brief More than or equal comparison between <tt>eina::optional</tt> objects.
 * @param lhs <tt>eina::optional</tt> object at the left side of the expression.
 * @param rhs <tt>eina::optional</tt> object at the right side of the expression.
 * @return @c true if @p rhs is disengaged or if both objects are
 *         engaged and the contained value of @p lhs is more than or
 *         equal to the contained value of @p rhs. In all other
 *         cases returns @c false.
 */
template <typename T>
bool operator>=(optional<T> const& lhs, optional<T> const& rhs)
{
   return !(lhs < rhs);
}

/**
 * @}
 */

} } // efl::eina

/**
 * @}
 */

#endif // EINA_OPTIONAL_HH_
