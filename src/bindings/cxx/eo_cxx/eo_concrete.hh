///
/// @file eo_concrete.hh
///

#ifndef EFL_CXX_EO_CONCRETE_HH
#define EFL_CXX_EO_CONCRETE_HH

#include <cassert>
#include <stdexcept>
#include <cstddef>
#include <eina_optional.hh>

#include <eina_eo_concrete_fwd.hh>

#include "eo_ops.hh"

#ifndef EFL_CXX_THROW
#if defined ( EFL_CXX_NO_EXCEPTIONS )
# define EFL_CXX_THROW(x)    std::abort()
#else
# define EFL_CXX_THROW(x)    throw (x)
#endif
#endif

#if !defined(EFL_DOXYGEN) && !defined(EO_CXX_INHERIT)
# define EO_CXX_INHERIT(name)   ::eo_cxx name
#elif !defined(EO_CXX_INHERIT)
# define EO_CXX_INHERIT(name)   name
#endif

namespace efl { namespace eo {

/// @addtogroup Efl_Cxx_API
/// @{

struct instantiate_t {
#ifdef EFL_CXXPERIMENTAL
   /// @brief A helper to create objects with a different syntax
   ///
   /// @param obj The object to instantiate
   /// @return obj The newly created object
   ///
   /// Consider an object declared by its type T on the stack, like T obj.
   /// Initially it will be empty (_eo_ptr() is nullptr). It can be created
   /// in two ways:
   ///   obj = T(instantiate, obj);
   /// or:
   ///   instantiate(obj);
   ///
   /// Note that if @c obj is already a valid object, it will be unreferenced.
   template<typename T> T& operator()(T& obj) const {
      obj = T(*this);
      return obj;
   }
#endif
};

/// @brief The handle to use to create real EFL objects
///
/// Use @c instantiate as first argument of any object constructor in order
/// to trigger a real EFL object creation. The following syntax is preferred:
///   T obj(instantiate, ...);
instantiate_t const instantiate = {};

#ifdef EFL_CXXPERIMENTAL
instantiate_t const add = {};
#endif
    
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
     : _eo_raw(nullptr)
   {
     if(other._eo_raw)
       _eo_raw = detail::ref(other._eo_raw);
   }

   concrete(concrete&& other)
   {
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
      else
        _eo_raw = nullptr;
      return *this;
   }

   concrete& operator=(concrete&& other)
   {
      concrete tmp(std::move(other));
      swap(tmp);
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

   void _delete()
   {
      Eo* tmp = _eo_raw;
      _eo_raw = nullptr;
      detail::del(tmp);
   }

   explicit operator bool() const
   {
      return _eo_raw;
   }

   friend void swap(concrete& lhs, concrete& rhs)
   {
      lhs.swap(rhs);
   }
   void swap(concrete& other)
   {
      std::swap(_eo_raw, other._eo_raw);
   }
protected:
   Eo* _eo_raw; ///< The opaque <em>EO Object</em>.
};

template<class CharT, class Traits>
inline std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, concrete const& eo)
{
   return os << eo._eo_ptr();
}

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

/// @}

} } // namespace efl { namespace eo {

#endif // EFL_CXX_EO_CONCRETE_HH
