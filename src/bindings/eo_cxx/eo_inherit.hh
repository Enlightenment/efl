
///
/// @file eo_inherit.hh
///

#ifndef EFL_CXX_EO_INHERIT_HH
#define EFL_CXX_EO_INHERIT_HH

#include <tuple>
#include <cassert>

#include <eina_integer_sequence.hh>

#include "eo_ops.hh"
#include "eo_private.hh"

namespace efl { namespace eo {

namespace detail {

template <typename D, typename Args, typename... E, std::size_t... S>
Eo_Class const* create_class(eina::index_sequence<S...>);

template <typename Args, typename ... E>
void inherit_constructor(void* this_, Args args);

}

/// @addtogroup Efl_Cxx_API
/// @{

/// @brief Template-class that allows client code to inherit from
/// <em>EO C++ Classes</em> without the need to make explicit calls to
/// <em>EO</em> methods --- that would naturally be necessary to
/// register itself in the <em>EO Subsystem</em>.
///
/// @param D The derived class
/// @param O The parent class
/// @param E Class extensions (either mixins or interfaces)
///
/// The derived class @p D will inherit all EO operations and event
/// callbacks from the parent class @p P, as well as from the <c>Base
/// Class</c> (@ref efl::eo::base) since every EO C++ Class must
/// inherit from it.
///
/// efl::eo::inherit makes use of meta-template elements to build (in
/// compile-time) code capable of registering @p D as an <em>EO
/// Class</em> within <em>EO Subsystem</em>. Each class is registered
/// only once upon instantiation of an object of its type.
///
/// @note Function overriding is currently not supported.
///
template <typename D, typename... E>
struct inherit;

/// @}

/// @addtogroup Efl_Cxx_API
/// @{

template <typename D, typename... E>
struct inherit
  : detail::operations<E>::template type<inherit<D, E...> > ...
  , detail::conversion_operator<inherit<D, E...>, E>...
{
   /// @typedef inherit_base
   ///
   typedef inherit<D, E...> inherit_base;

   /// @brief Class constructor.
   ///
   /// @ref inherit has a "variadic" constructor implementation that
   /// allows from zero to EFL_MAX_ARGS heterogeneous parameters.
   ///
   template<typename... Args>
   inherit(Args&& ... args)
   {
      typedef std::tuple<typename std::remove_reference<Args>::type...> tuple_type;
      _eo_cls = detail::create_class<D, tuple_type, E...> (eina::make_index_sequence<sizeof...(E)>());
      _eo_raw = eo_add_custom
        (_eo_cls, NULL,
         detail::inherit_constructor
         <tuple_type, E...>
         (static_cast<void*>(this), tuple_type(std::move(args)...)));
  }

   /// @brief Class destructor.
   ///
   ~inherit()
   {
      detail::unref(_eo_raw);
   }

   /// @brief Gets the <em>EO Object</em> corresponding to this <em>EO
   /// C++ Object</em>.
   ///
   /// @return A pointer to the <em>EO Object</em>.
   ///
   Eo* _eo_ptr() const { return _eo_raw; }

   /// @brief Gets the <em>EO Class</em> corresponding to this <em>EO
   /// C++ Class</em>.
   ///
   /// @return A pointer to the <em>EO Class</em>.
   ///
   Eo_Class const* _eo_class() const { return _eo_cls; }

protected:
   /// @brief Copy constructor.
   ///
   inherit(inherit const& other)
     : _eo_cls(other._eo_cls)
     , _eo_raw(other._eo_raw)
     { detail::ref(_eo_raw); }

   /// @brief Assignment Operator
   ///
   inherit& operator=(inherit const& other)
   {
      _eo_cls = other._eo_cls;
      _eo_raw = other._eo_raw;
      detail::ref(_eo_raw);
      return *this;
   }

private:
   Eo_Class const* _eo_cls;   ///< The <em>EO Class</em>.
   Eo* _eo_raw;               ///< The <em>EO Object</em>.
};

/// @}

} } // namespace efl { namespace eo {

#include "eo_inherit_bindings.hh"

#endif // EFL_CXX_INHERIT_HH
