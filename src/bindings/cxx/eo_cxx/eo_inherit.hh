
// ///
// /// @file eo_inherit.hh
// ///

// #ifndef EFL_CXX_EO_INHERIT_HH
// #define EFL_CXX_EO_INHERIT_HH

// #include <tuple>
// #include <cassert>

// #include <eina_integer_sequence.hh>

// #include "eo_ops.hh"
// #include "eo_private.hh"
// #include "eo_cxx_interop.hh"

// namespace efl { namespace eo {

// namespace detail {

// template <typename D, typename... E, std::size_t... S>
// Eo_Class const* create_class(eina::index_sequence<S...>);

// /// @internal
// ///
// /// @brief Find the correct function for the <em>"constructor"</em>
// /// operation and invoke it.
// ///
// /// @param this_ The <em>user data</em> to be passed to the resolved function.
// /// @param args An heterogeneous sequence of arguments.
// ///
// inline EO_VOID_FUNC_BODYV(inherit_constructor, EO_FUNC_CALL(this_), void* this_);

// }

// /// @addtogroup Efl_Cxx_API
// /// @{

// /// @brief Template-class that allows client code to inherit from
// /// <em>EO C++ Classes</em> without the need to make explicit calls to
// /// <em>EO</em> methods --- that would naturally be necessary to
// /// register itself in the <em>EO Subsystem</em>.
// ///
// /// @param D The derived class
// /// @param O The parent class
// /// @param E Class extensions (either mixins or interfaces)
// ///
// /// The derived class @p D will inherit all EO operations and event
// /// callbacks from the parent class @p P, as well as from the <c>Base
// /// Class</c> (@ref efl::eo::concrete) since every EO C++ Class must
// /// inherit from it.
// ///
// /// efl::eo::inherit makes use of meta-template elements to build (in
// /// compile-time) code capable of registering @p D as an <em>EO
// /// Class</em> within <em>EO Subsystem</em>. Each class is registered
// /// only once upon instantiation of an object of its type.
// ///
// /// @note Function overriding is currently not supported.
// ///
// template <typename D, typename... E>
// struct inherit;

// /// @}

// /// @addtogroup Efl_Cxx_API
// /// @{

// template <typename D, typename... E>
// struct inherit
//   : detail::operations<E>::template type<inherit<D, E...> > ...
//   , detail::conversion_operator<inherit<D, E...>, E>...
// {
//    /// @typedef inherit_base
//    ///
//    typedef inherit<D, E...> inherit_base;

//    //@{
//    /// @brief Class constructor.
//    ///
//    /// @ref inherit has a "variadic" constructor implementation that
//    /// allows from zero to EFL_MAX_ARGS heterogeneous parameters.
//    ///
//    template<typename... Args>
//    inherit(efl::eo::parent_type _p, Args&& ... args)
//    {
//       _eo_cls = detail::create_class<D, E...> (eina::make_index_sequence<sizeof...(E)>());
//       _eo_raw = eo_add_ref(_eo_cls, _p._eo_raw, detail::inherit_constructor(eo_self, this), ::efl::eolian::call_ctors(eo_self, args...));
//       ::efl::eolian::register_ev_del_free_callback(_eo_raw, args...);
//   }

//   template<typename... Args>
//    inherit(Args&& ... args)
//      : inherit(::efl::eo::parent = nullptr, std::forward<Args>(args)...)
//    {}
//    //@}

//    /// @brief Class destructor.
//    ///
//    ~inherit()
//    {
//       detail::unref(_eo_raw);
//    }

//    /// @brief Gets the <em>EO Object</em> corresponding to this <em>EO
//    /// C++ Object</em>.
//    ///
//    /// @return A pointer to the <em>EO Object</em>.
//    ///
//    Eo* _eo_ptr() const { return _eo_raw; }

//    /// @brief Gets the <em>EO Class</em> corresponding to this <em>EO
//    /// C++ Class</em>.
//    ///
//    /// @return A pointer to the <em>EO Class</em>.
//    ///
//    Eo_Class const* _eo_class() const { return _eo_cls; }

//    Eo* _release()
//    {
//       Eo* tmp = _eo_raw;
//       _eo_raw = nullptr;
//       return tmp;
//    }

// protected:
//    /// @brief Copy constructor.
//    ///
//    inherit(inherit const& other)
//      : _eo_cls(other._eo_cls)
//      , _eo_raw(other._eo_raw)
//      { detail::ref(_eo_raw); }

//    /// @brief Assignment Operator
//    ///
//    inherit& operator=(inherit const& other)
//    {
//       _eo_cls = other._eo_cls;
//       _eo_raw = other._eo_raw;
//       detail::ref(_eo_raw);
//       return *this;
//    }

// private:
//    Eo_Class const* _eo_cls;   ///< The <em>EO Class</em>.
//    Eo* _eo_raw;               ///< The <em>EO Object</em>.
// };

// /// @}

// } } // namespace efl { namespace eo {

// #include "eo_inherit_bindings.hh"

// #endif // EFL_CXX_INHERIT_HH
