
///
/// @file eo_private.hh
///

#ifndef EFL_CXX_EO_PRIVATE_HH
#define EFL_CXX_EO_PRIVATE_HH

#include "eo_ops.hh"

namespace efl { namespace eo { namespace detail {

/// @addtogroup Efl_Cxx_Detail
/// @{

/// @internal
///
/// @brief Provides a getter for an heterogeous sequence of arguments.
///
/// @param T An <em>EO C++ Class</em>
/// @param Seq An heterogenous sequence of arguments.
///
template <typename T, typename Tuple>
struct args_class
{
   typedef T class_type;

   /// @internal
   ///
   /// @brief Class constructor.
   ///
   /// @param tuple An heterogenous sequence of arguments.
   ///
   args_class(Tuple tuple) : _tuple(tuple) {}

   /// @internal
   ///
   /// @brief Get the Nth element of the sequence.
   ///
   /// @param N The index of the argument to be retrieved.
   ///
   template <int N>
   typename std::remove_reference
   <
     typename std::tuple_element<N, Tuple>::type
   >::type get() const
   {
      return std::get<N>(_tuple);
   }

   ///
   /// TODO document.
   ///
   constexpr std::size_t size() const
   {
      return std::tuple_size<Tuple>::value;
   }

   Tuple _tuple; ///< The sequence of arguments.
};

/// @internal
///
/// @brief A simple generic tag to help keeping track of a type.
///
/// @details
/// Used to mimic what would be a "template specialization" of a
/// function through the overload of an argument of type @ref
/// efl::eo::detail::tag --- because @c C++0x does not implement
/// template specialization of functions.
///
template <typename T> struct tag {};

/// @internal
///
/// @brief Invokes the different implementations of @ref
/// efl::eo::detail::eo_class_new for the parent and all extensions.
///
/// @see efl::eo::detail::eo_class_new
///
template <typename ... E>
Eo_Class const* do_eo_class_new(Eo_Class_Description& class_desc)
{
   return eo_class_new(&class_desc, get_eo_class(tag<E>())..., (void*)NULL);
}

template <typename T> struct operation_description_class_size;

/// @internal
///
/// @brief Provides the operator to convert @p T to @p D.
///
/// @param D The target (derived) class
/// @param T An <em>EO C++ Class</em>
///
template <typename D, typename T>
struct conversion_operator
{
   operator T() const
   {
      detail::ref(static_cast<D const*>(this)->_eo_ptr());
      return T(static_cast<D const*>(this)->_eo_ptr());
   }
};

/// @}

} // namespace detail {

template <typename T, typename... A>
detail::args_class<T, std::tuple< A... > >
args(A... arg)
{
   return detail::args_class<T, std::tuple<A...> >(std::tuple<A...>(arg...));
}

namespace detail {

/// @addtogroup Efl_Cxx_Detail
/// @{

/// @internal
///
/// @brief Provides all operations of type @p T.
///
/// @param T An <em>EO C++ Class</em>
///
/// There must be an unique specialization of this class for each
/// declared <em>EO C++ Class</em>.
///
template <typename T> struct operations;

/// @internal
///
/// @brief Provides the operations of an extension as well as its
/// conversion operator.
///
/// @param T The <em>EO C++ Extension</em>
///
/// There must be an unique specialization of @ref
/// efl::eo::detail::extension_inheritance for each known <em>EO C++
/// Extension</em> -- i.e., @em Interfaces and @em Mixins.
///
template <typename T>
struct extension_inheritance {};

/// @internal
///
/// @brief efl::eo::inherit's private data.
///
struct Inherit_Private_Data
{
   void* this_;
};

/// @}

} } } // namespace efl { namespace eo { namespace detail {

#endif
