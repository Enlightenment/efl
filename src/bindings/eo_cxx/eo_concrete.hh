///
/// @file eo_concrete.hh
///

#ifndef EFL_CXX_EO_CONCRETE_HH
#define EFL_CXX_EO_CONCRETE_HH

#include <cassert>
#include <stdexcept>
#include <cstddef>
#include <eina_optional.hh>

#include "eo_ops.hh"
#include "eo_event.hh"

#ifndef EFL_CXX_THROW
#if defined ( EFL_CXX_NO_EXCEPTIONS )
# define EFL_CXX_THROW(x)    std::abort()
#else
# define EFL_CXX_THROW(x)    throw (x)
#endif
#endif

namespace efl { namespace eo {

/// @addtogroup Efl_Cxx_API
/// @{

/// @brief Creates concrete versions for <em>Eo</em> wrappers.
///
/// This class creates the concrete version of all C++ <em>Eo</em> wrappers.
/// It holds the Eo pointer that is used on all operations and provides some
/// functions for manipulating it.
///
struct concrete
{
   /// @brief Class constructor.
   ///
   /// @param eo The <em>EO Object</em>.
   ///
   /// efl::eo::concrete constructors semantics are that of stealing the
   /// <em>EO Object</em> lifecycle management. Its constructors do not
   /// increment the <em>EO</em> reference counter but the destructors
   /// do decrement.
   ///
   explicit concrete(Eo* eo) : _eo_raw(eo)
   {
   }


   /// @brief Default constructor.
   ///
   /// Constructs a NULL initialized efl::eo::concrete object.
   ///
   concrete() : _eo_raw(nullptr)
   {
   }

   /// @brief Class destructor.
   ///
   ~concrete()
   {
      if(_eo_raw)
        detail::unref(_eo_raw);
   }

   concrete(concrete const& other)
   {
     if(other._eo_raw)
       _eo_raw = detail::ref(other._eo_raw);
   }

   concrete(concrete&& other)
   {
     if(_eo_raw) detail::unref(_eo_raw);
     _eo_raw = other._eo_raw;
     other._eo_raw = nullptr;
   }

   /// @brief Assignment operator.
   ///
   concrete& operator=(concrete const& other)
   {
      if(_eo_raw)
        {
           detail::unref(_eo_raw);
           _eo_raw = nullptr;
        }
      if(other._eo_raw)
        _eo_raw = detail::ref(other._eo_raw);
      return *this;
   }

   concrete& operator=(concrete&& other)
   {
      if(_eo_raw)
        {
           detail::unref(_eo_raw);
           _eo_raw = nullptr;
        }
      std::swap(_eo_raw, other._eo_raw);
      return *this;
   }
  
   /// @brief Return a pointer to the <em>EO Object</em> stored in this
   /// instance.
   ///
   /// @return A pointer to the opaque <em>EO Object</em>.
   ///
   Eo* _eo_ptr() const { return _eo_raw; }

   /// @brief Releases the reference from this concrete object and
   /// return the pointer to the <em>EO Object</em> stored in this
   /// instance.
   ///
   /// @return A pointer to the opaque <em>EO Object</em>.
   ///
   Eo* _release()
   {
     Eo* tmp = _eo_raw;
     _eo_raw = nullptr;
     return tmp;
   }

   /// @brief Reset the current pointer to reference a new Eo object.
   ///
   void _reset(Eo* _ptr = nullptr)
   {
      if(_eo_raw)
        detail::unref(_eo_raw);
     _eo_raw = _ptr;
   }

   /// @brief Get the reference count of this object.
   ///
   /// @return The referencer count of this object.
   ///
   int ref_get() const { return detail::ref_get(_eo_raw); }

   /// @brief Set the parent of this object.
   ///
   /// @param parent The new parent.
   ///
   void parent_set(concrete parent)
   {
      detail::parent_set(_eo_raw, parent._eo_ptr());
   }

   /// @brief Get the parent of this object.
   ///
   /// @return An @ref efl::eo::concrete instance that binds the parent
   /// object. Returns NULL if there is no parent.
   ///
   eina::optional<concrete> parent_get()
   {
      Eo *r = detail::parent_get(_eo_raw);
      if(!r) return nullptr;
      else
        {
           detail::ref(r); // XXX eo_parent_get does not call eo_ref so we may.
           return concrete(r);
        }
   }

   /// @brief Get debug information of this object.
   ///
   /// @return The root node of the debug information tree.
   ///
   Eo_Dbg_Info dbg_info_get()
   {
      Eo_Dbg_Info info;
      detail::dbg_info_get(_eo_raw, &info);
      return info;
   }

   explicit operator bool() const
   {
      return _eo_raw;
   }
 protected:
   Eo* _eo_raw; ///< The opaque <em>EO Object</em>.
};

inline bool operator==(concrete const& lhs, concrete const& rhs)
{
  return lhs._eo_ptr() == rhs._eo_ptr();
}

inline bool operator!=(concrete const& lhs, concrete const& rhs)
{
  return !(lhs == rhs);
}

namespace detail {

template <typename T>
struct extension_inheritance;
  
template<>
struct extension_inheritance<concrete>
{
   template <typename T>
   struct type
   {
      operator concrete() const
      {
         return concrete(eo_ref(static_cast<T const*>(this)->_eo_ptr()));
      }

   };
};

struct concrete_address_of
{
   explicit concrete_address_of(void* p) : p(p) {}
   operator void*() { return p; }
   void* p;
};

struct concrete_address_const_of
{
   explicit concrete_address_const_of(void const* p) : p(p) {}
   operator void const*() { return p; }
   void const* p;
};

}

/// @brief Downcast @p U to @p T.
///
/// @param T An <em>EO C++ Class</em>.
/// @param U An <em>EO C++ Class</em>.
///
/// @param object The target object.
/// @return This function returns a new instance of @p T if the
/// downcast is successful --- otherwise it raises a @c
/// std::runtime_error.
///
template <typename T, typename U>
T downcast(U object)
{
   Eo *eo = object._eo_ptr();

   if(detail::isa(eo, T::_eo_class()))
     {
        return T(detail::ref(eo));
     }
   else
     {
        EFL_CXX_THROW(std::runtime_error("Invalid cast"));
     }
}

///
/// @brief Type used to hold the parent passed to concrete Eo C++
/// constructors.
///
struct parent_type
{
   Eo* _eo_raw;
};

///
/// @brief The expression type declaring the assignment operator used
/// in the parent argument of the concrete Eo C++ class.
///
struct parent_expr
{
   parent_type operator=(efl::eo::concrete const& parent) const
   {
      return { parent._eo_ptr() };
   }

   template <typename T>
   parent_type operator=(T const& parent) const
   {
      return { parent._eo_ptr() };
   }
   parent_type operator=(std::nullptr_t) const
   {
      return { nullptr };
   }
};

///
/// @brief Placeholder for the parent argument.
///
parent_expr const parent = {};

/// @}

} } // namespace efl { namespace eo {

#endif // EFL_CXX_EO_CONCRETE_HH
