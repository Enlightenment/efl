
///
/// @file eo_base.hh
///

#ifndef EFL_CXX_EO_BASE_HH
#define EFL_CXX_EO_BASE_HH

#include <cassert>
#include <stdexcept>
#include <cstddef>
#include <eina_optional.hh>

#include "eo_ops.hh"


namespace efl { namespace eo {

/// @addtogroup Efl_Cxx_API
/// @{

/// @brief A binding to the <em>EO Base Class</em>.
///
/// This class implements C++ wrappers to all the <em>EO Base</em>
/// operations.
///
struct base
{
   /// @brief Class constructor.
   ///
   /// @param eo The <em>EO Object</em>.
   ///
   /// efl::eo::base constructors semantics are that of stealing the
   /// <em>EO Object</em> lifecycle management. Its constructors do not
   /// increment the <em>EO</em> reference counter but the destructors
   /// do decrement.
   ///
   explicit base(Eo* eo) : _eo_raw(eo)
   {
      assert(eo != 0);
   }

   /// @brief Class destructor.
   ///
   ~base()
   {
      detail::unref(_eo_raw);
   }

   /// @brief Assignment operator.
   ///
   base& operator=(base const& other)
   {
      _eo_raw = detail::ref(other._eo_ptr());
      return *this;
   }

   /// @brief Return a pointer to the <em>EO Object</em> stored in this
   /// instance.
   ///
   /// @return A pointer to the opaque <em>EO Object</em>.
   ///
   Eo* _eo_ptr() const { return _eo_raw; }

   /// @brief Get the reference count of this object.
   ///
   /// @return The referencer count of this object.
   ///
   int ref_get() const { return detail::ref_get(_eo_raw); }

   /// @brief Set the parent of this object.
   ///
   /// @param parent The new parent.
   ///
   void parent_set(base parent)
   {
      detail::parent_set(_eo_raw, parent._eo_ptr());
   }

   /// @brief Get the parent of this object.
   ///
   /// @return An @ref efl::eo::base instance that binds the parent
   /// object. Returns NULL if there is no parent.
   ///
   eina::optional<base> parent_get()
   {
      Eo *r = detail::parent_get(_eo_raw);
      if(!r) return nullptr;
      else
        {
           detail::ref(r); // XXX eo_parent_get does not call eo_ref so we may.
           return base(r);
        }
   }

   /// @brief Set generic data to object.
   ///
   /// @param key The key associated with the data.
   /// @param data The data to set.
   /// @param free_func A pointer to the function that frees the
   /// data. @c (::eo_key_data_free_func*)0 is valid.
   ///
   void base_data_set(const char *key, const void *data, ::eo_key_data_free_func func)
   {
      detail::base_data_set(_eo_raw, key, data, func);
   }

   /// @brief Get generic data from object.
   ///
   /// @param key The key associated with desired data.
   /// @return A void pointer to the data.
   ///
   void* base_data_get(const char *key)
   {
      return detail::base_data_get(_eo_raw, key);
   }

   /// @brief Delete generic data from object.
   ///
   /// @param key The key associated with the data.
   ///
   void base_data_del(const char *key)
   {
      detail::base_data_del(_eo_raw, key);
   }

   /// @brief Freeze any event directed to this object.
   ///
   /// Prevents event callbacks from being called for this object.
   ///
   void event_freeze()
   {
      detail::event_freeze(_eo_raw);
   }

   /// @brief Thaw the events of this object.
   ///
   /// Let event callbacks be called for this object.
   ///
   void event_thaw()
   {
      detail::event_thaw(_eo_raw);
   }

   /// @brief Get the event freeze count for this object.
   ///
   /// @return The event freeze count for this object.
   ///
   int event_freeze_get()
   {
      return detail::event_freeze_get(_eo_raw);
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

 protected:
   Eo* _eo_raw; ///< The opaque <em>EO Object</em>.
};

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
        throw std::runtime_error("Invalid cast");
     }
}

///
/// @brief Type used to hold the parent passed to base Eo C++
/// constructors.
///
struct parent_type
{
   Eo* _eo_raw;
};

///
/// @brief The expression type declaring the assignment operator used
/// in the parent argument of the base Eo C++ class.
///
struct parent_expr
{
   parent_type operator=(efl::eo::base const& parent)
   {
      return { parent._eo_ptr() };
   }

   parent_type operator=(std::nullptr_t)
   {
      return { NULL };
   }
};

///
/// @brief Placeholder for the parent argument.
///
parent_expr parent = {};

/// @}

} } // namespace efl { namespace eo {

#endif // EFL_CXX_EO_BASE_HH
