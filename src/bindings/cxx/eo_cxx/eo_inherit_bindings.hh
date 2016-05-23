
#ifndef EFL_CXX_DETAIL_INHERIT_BINDINGS_HH
#define EFL_CXX_DETAIL_INHERIT_BINDINGS_HH

#include <typeinfo>
#include <eina_integer_sequence.hh>

namespace efl { namespace eo { namespace detail {

/// @addtogroup Efl_Cxx_Detail
/// @{

/// @internal
///
/// @brief Sums up the number of <em>EO Operations</em> of each class
/// passed as argument to the template.
///
/// @see efl::eo::detail::operation_description_class_size
///
template <typename... E>
struct operation_description_size;

template <typename E0, typename... E>
struct operation_description_size<E0, E...>
{
   static const int value = operation_description_class_size<E0>::value +
     operation_description_size<E...>::value;
};

template <>
struct operation_description_size<>
{
   static const int value = 0;
};

template <typename... Args>
void call_varargs(Args...)
{
}

/// @internal
///
/// @brief The procedure that actually is invoked when the constructor
/// of @c D is sought from the <em>EO Subsystem</em>.
///
/// @param self A pointer to @p obj's private data.
/// @param this_ A void pointer to the opaque <em>EO Class</em> ---
/// passed as <em>user data</em>.
///
inline
void inherit_constructor_impl(Eo*, Inherit_Private_Data* self, void* this_)
{
   self->this_ = this_;
}

template <typename T>
int initialize_operation_description(detail::tag<void>, void*);

template <std::size_t I, typename... E>
struct
operation_description_index
{
   typedef std::tuple<E...> tuple_type;
   static const std::size_t value =
     detail::operation_description_size
     < typename std::tuple_element
       <I-1, tuple_type>::type >::value +
     operation_description_index<I-1, E...>::value;
};
template <typename... E>
struct
operation_description_index<0u, E...>
{
   static const std::size_t value = 0u;
};

/// @internal
///
/// @brief This function is responsible for declaring a new <em>EO C
/// Class</em> representing @p D within <em>EO Subsystem</em>.
///
/// @param D The derived class
/// @param P The parent class
/// @param En Class extensions (either mixins or interfaces)
/// @param Args An heterogeneous list of arguments to be passed to the
/// constructor of this class.
///
/// @see efl::eo::inherit::inherit
///
template <typename D, typename... E, std::size_t ... S>
Eo_Class const* create_class(eina::index_sequence<S...>)
{
   static const Eo_Class* my_class = NULL;
   static Eo_Op_Description op_descs
     [ detail::operation_description_size<E...>::value + 2 ];

   op_descs[detail::operation_description_size<E...>::value].func =
     reinterpret_cast<void*>
     (
      &detail::inherit_constructor_impl
     );
   op_descs[detail::operation_description_size<E...>::value].api_func =
     reinterpret_cast<void*>
     (
      &detail::inherit_constructor
     );
   op_descs[detail::operation_description_size<E...>::value].op_type = EO_OP_TYPE_REGULAR;

   op_descs[detail::operation_description_size<E...>::value+1].func = 0;
   op_descs[detail::operation_description_size<E...>::value+1].api_func = 0;
   op_descs[detail::operation_description_size<E...>::value+1].op_type = EO_OP_TYPE_INVALID;

   typedef inherit<D, E...> inherit_type;
   using namespace detail;
   call_varargs(
                initialize_operation_description<inherit_type>
                (detail::tag<E>(),
                 &op_descs[operation_description_index<S, E...>::value]) ...
                );

   //locks
   if(!my_class)
     {
        static Eo_Class_Description class_desc = {
          EO_VERSION,
          "Eo C++ Class",
          EO_CLASS_TYPE_REGULAR,
          EO_CLASS_DESCRIPTION_OPS(op_descs),
          NULL,
          sizeof(detail::Inherit_Private_Data),
          NULL,
          NULL
        };
        my_class = detail::do_eo_class_new<E...>(class_desc);
     }
   return my_class;
}

} } }  // namespace efl { namespace eo { namespace detail {

#endif // EFL_CXX_DETAIL_INHERIT_BINDINGS_HH
