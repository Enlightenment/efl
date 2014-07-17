
///
/// @file eo_wref.hh
///

#ifndef EFL_CXX_WREF_HH
#define EFL_CXX_WREF_HH

#include <eina_optional.hh>

#include "eo_ops.hh"

namespace efl { namespace eo {

/// @addtogroup Efl_Cxx_API
/// @{

/// @brief Weak references to an <em>EO Object</em>.
///
template<typename T>
struct wref
{
   /// @brief Default constructor.
   ///
   /// Create a empty weak reference.
   ///
   wref() : _eo_wref(nullptr)
   {
   }
  
   /// @brief Class constructor.
   ///
   /// @param obj The <em>EO Object</em> to be referenced.
   ///
   /// Create a weak reference to @p obj.
   ///
   explicit wref(Eo* obj) : _eo_wref(obj)
   {
      _add();
   }

   /// @brief Class constructor.
   ///
   /// @param obj The <em>EO C++ Object</em> to be referenced.
   ///
   /// Create a weak reference to @p obj.
   ///
   wref(T obj) : _eo_wref(obj._eo_ptr())
   {
      _add();
   }

   /// @brief Class destructor.
   ///
   ~wref()
   {
      if(_eo_wref)
        if(eina::optional<T> p = lock())
          _del();
   }

   /// @brief Try to acquire a strong reference to the underlying
   /// <em>EO Object</em>.
   ///
   /// This function checks whether the weak reference still points to
   /// a valid <em>EO Object</em>. If the reference is still valid it
   /// increments the reference counter of the object and returns a
   /// pointer to it.
   ///
   /// @return If the lock was successfully acquired it returns a
   /// strong reference to the <em>EO Object</em>. Otherwise it returns
   /// an empty eina::optional.
   ///
   eina::optional<T> lock()
   {
      if(_eo_wref) // XXX eo_ref() should work on multi-threaded environments
        {
           detail::ref(_eo_wref);
        }
      else
        {
           return nullptr;
        }
      return T(_eo_wref);
   }

   /// @brief Copy constructor.
   ///
   wref(wref const& other)
     : _eo_wref(other._eo_wref)
   {
      if(eina::optional<T> p = lock())
        {
           _add();
        }
      else
        {
           _eo_wref = 0;
        }
   }

   /// @brief Assignment operator.
   ///
   wref& operator=(wref const& other)
   {
      _eo_wref = other._eo_wref;
      if(eina::optional<T> p = lock())
        {
           _add();
        }
      else
        {
           _eo_wref = 0;
        }
   }

private:
   void _add()
   {
      detail::wref_add(_eo_wref, &_eo_wref);
   }

   void _del()
   {
      detail::wref_del(_eo_wref, &_eo_wref);
   }

   Eo* _eo_wref; ///< The weak reference.
};

/// @}

} } // namespace efl { namespace eo {

#endif // EFL_CXX_WREF_HH
