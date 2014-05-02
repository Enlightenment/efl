
#ifndef EFL_CXX_DETAIL_INHERIT_BINDINGS_HH
#define EFL_CXX_DETAIL_INHERIT_BINDINGS_HH

#include <typeinfo>
#include <eina_integer_sequence.hh>

namespace efl { namespace eo { namespace detail {

/// @addtogroup Efl_Cxx_Detail
/// @{

/// @internal
///
/// @brief Invokes the <em>EO C Constructor</em> that corresponds to the
/// binded <em>EO C++ Class</em>.
///
/// @param T The corresponding <em>EO C++ Class</em>
/// @param Args An heterogeneous list of constructor arguments
///
/// @param tag Used to instruct the compiler during compile-time which
/// of the overloads should be invoked.
/// @param eo A pointer to the <em>EO C Object</em> to be constructed.
/// @param cls Unused.
/// @param args An heterogenous vector containing the constructor
/// arguments, in the correct order.
///
/// To ensure full reciprocity of the C++ binding there must exist one
/// (and only one) implementation of @ref efl::eo::detail::call_constructor
/// for each available <em>EO C++ Class</em> --- the implementations
/// are differentiated by this unique specialization of
/// @ref efl::eo::detail::tag for the first argument of
/// @ref efl::eo::detail::call_constructor.
///
/// For example this is how the overload for @ref eo_simple is
/// written as follows:
///
/// @dontinclude eo_simple.hh
/// @skip call_constructor
/// @until }
///
/// As you can see @c ::simple_constructor is called with a single
/// argument in this case. Each EO Class has its own constructor
/// prototype -- which can have different argument types as well as
/// distinct number of arguments, etc. -- hence the need to specify a
/// choice for every known <em>EO C++ Class</em>.
///
/// @see efl::eo::detail::tag
///
template <typename T, typename Args>
void call_constructor(efl::eo::detail::tag<T> tag, Eo* eo, Eo_Class const* cls, Args args);

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

template <typename T>
struct is_args_class : std::false_type
{
};

template <typename T, typename Tuple>
struct is_args_class<args_class<T, Tuple> >
  : std::true_type
{
};

template <typename Tuple>
struct are_args_class;

template <>
struct are_args_class<std::tuple<> >
  : std::true_type
{
};

template <typename T0, typename... T>
struct are_args_class<std::tuple<T0, T...> >
  : std::integral_constant
  <bool
   , is_args_class<T0>::value
   && are_args_class<std::tuple<T...> >::value
  >
{
};

template <typename T, typename Tuple>
struct has_args_class : std::false_type
{
   typedef std::integral_constant<std::size_t, 0u> index;
};

template <typename T, typename Tuple, typename... Args>
struct has_args_class<T, std::tuple<detail::args_class<T, Tuple>, Args...> >
  : std::true_type
{
   typedef detail::args_class<T, Tuple> type;
   typedef std::integral_constant<std::size_t, 0u> index;
};

template <typename T, typename T0, typename... Args>
struct has_args_class<T, std::tuple<T0, Args...> >
  : has_args_class<T, std::tuple<Args...> >
{
   typedef has_args_class<T, std::tuple<Args...> > base_type;
   typedef std::integral_constant
   <std::size_t, 1u + base_type::index::value> index;
};

/// @internal
///
/// @brief An auxiliary template-class used to select the correct
/// implementation of @ref efl::eo::call_constructor for @p T with
/// proper parameters and variadic size.
///
/// @param T An <em>EO C++ Class</em>.
///
template <typename T, std::size_t N>
struct call_constructor_aux
{
   template <typename Args, typename P>
   static void do_(Args const&, Eo* eo, Eo_Class const* cls
                  , P, typename std::enable_if<!P::value>::type* = 0)
   {
      call_constructor(tag<T>(), eo, cls, args_class<T, std::tuple<> >(std::tuple<>()));
   }

   template <typename Args, typename P>
   static void do_(Args const& args, Eo* eo, Eo_Class const* cls
                  , P, typename std::enable_if<P::value>::type* = 0)
   {
      call_constructor(tag<T>(), eo, cls, std::get<P::index::value>(args));
   }

   /// @internal
   ///
   /// @brief Invoke @def efl::eo::detail::call_constructor
   /// implementation for the parent and each available extension.
   ///
   /// @param args An heterogenous sequence of arguments.
   /// @param eo The opaque <em>EO Object</em>.
   /// @param cls The opaque <em>EO Class</em>.
   ///
   template <typename Args>
   static int do_(Args const& args, Eo* eo, Eo_Class const* cls)
   {
      static_assert(std::tuple_size<Args>::value <= N, "");
      static_assert(are_args_class<Args>::value, "");
      do_(args, eo, cls, has_args_class<T, Args>());
      return 0;
   }
};

template <typename T>
struct call_constructor_aux<T, 1u>
{
   template <typename Args>
   static void do_(Args const& args, Eo* eo, Eo_Class const* cls
                  , std::true_type)
   {
      static_assert(std::tuple_size<Args>::value == 1, "");
      static_assert(std::is_same
                    <typename std::tuple_element<0u, Args>::type::class_type
                    , T>::value, "");
      call_constructor(tag<T>(), eo, cls, std::get<0u>(args));
   }

   template <typename Args>
   static void do_(Args const& args, Eo* eo, Eo_Class const* cls
                  , std::false_type)
   {
      call_constructor(tag<T>(), eo, cls, args_class<T, Args>(args));
   }

   template <typename Args>
   static int do_(Args const& args, Eo* eo, Eo_Class const* cls)
   {
      do_(args, eo, cls, has_args_class<T, Args>());
      return 0;
   }
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
/// @param obj The opaque <em>EO Object</em>.
/// @param self A pointer to @p obj's private data.
/// @param this_ A void pointer to the opaque <em>EO Class</em> ---
/// passed as <em>user data</em>.
/// @param args The arguments for the underlying constructor.
///
template <typename D, typename Args, typename... E>
void inherit_constructor_impl(Eo* obj, Inherit_Private_Data* self, void* this_, Args args)
{
   self->this_ = this_;
   Eo_Class const* cls = static_cast<inherit<D, E...>*>(this_)->_eo_class();
   detail::call_varargs(detail::call_constructor_aux<E, sizeof...(E)>::do_(args, obj, cls) ...);
}

/// @internal
///
/// @brief Find the correct function for the <em>"constructor"</em>
/// operation and invoke it.
///
/// @param this_ The <em>user data</em> to be passed to the resolved function.
/// @param args An heterogeneous sequence of arguments.
///
template <typename Args, typename... E>
EAPI void inherit_constructor(void* this_, Args args)
{
   typedef void (*func_t)(Eo *, void *, void*, Args);
   Eo_Op_Call_Data call;
   static Eo_Op op = EO_NOOP;
   if ( op == EO_NOOP )
     op = _eo_api_op_id_get
       (reinterpret_cast<void*>
        (static_cast<void(*)(void*, Args)>(&detail::inherit_constructor<Args, E...>)),
        __FILE__, __LINE__);
   if (!_eo_call_resolve("detail::inherit_constructor", op, &call, __FILE__, __LINE__))
     {
        assert(_eo_call_resolve("detail::inherit_constructor", op, &call, __FILE__, __LINE__));
        return;
     }
   func_t func = (func_t) call.func;
   EO_HOOK_CALL_PREPARE(eo_hook_call_pre);
   func(call.obj, call.data, this_, args);
   EO_HOOK_CALL_PREPARE(eo_hook_call_post);
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
template <typename D, typename TupleArgs, typename... E, std::size_t ... S>
Eo_Class const* create_class(eina::index_sequence<S...>)
{
   static const Eo_Class* my_class = NULL;
   static Eo_Op_Description op_descs
     [ detail::operation_description_size<E...>::value + 2 ];

   op_descs[detail::operation_description_size<E...>::value].func =
     reinterpret_cast<void*>
     (
      static_cast<void(*)(Eo*, Inherit_Private_Data*, void*, TupleArgs)>
      (&detail::inherit_constructor_impl<D, TupleArgs, E...>)
     );
   op_descs[detail::operation_description_size<E...>::value].api_func =
     reinterpret_cast<void*>
     (
      static_cast<void(*)(void*, TupleArgs)>
      (&detail::inherit_constructor<TupleArgs, E...>)
     );
   op_descs[detail::operation_description_size<E...>::value].op = EO_NOOP;
   op_descs[detail::operation_description_size<E...>::value].op_type = EO_OP_TYPE_REGULAR;
   op_descs[detail::operation_description_size<E...>::value].doc = NULL;

   op_descs[detail::operation_description_size<E...>::value+1].func = 0;
   op_descs[detail::operation_description_size<E...>::value+1].api_func = 0;
   op_descs[detail::operation_description_size<E...>::value+1].op = 0;
   op_descs[detail::operation_description_size<E...>::value+1].op_type = EO_OP_TYPE_INVALID;
   op_descs[detail::operation_description_size<E...>::value+1].doc = NULL;

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
