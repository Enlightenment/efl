///
/// @file eo_promise_meta.hh
///

#ifndef EFL_CXX_EO_PROMISE_META_HH
#define EFL_CXX_EO_PROMISE_META_HH

namespace efl {

template <typename...Args>
struct shared_future;

template <typename...Args>
struct shared_race_future;

template <typename T>
struct progress;
  
namespace _impl {

template <typename T>
struct is_progress : std::false_type {};

template <typename T>
struct is_progress<progress<T>> : std::true_type {};

template <typename L, typename R>
struct is_progress_param_compatible : std::false_type {};

template <typename T>
struct is_progress_param_compatible<T, T> : std::true_type {};
  
template <>
struct is_progress_param_compatible<void, progress<void>> : std::true_type {};

template <>
struct is_progress_param_compatible<progress<void>, void> : std::true_type {};

template <typename...Args>
struct progress_param : std::conditional
 <is_progress<typename std::tuple_element<sizeof...(Args) - 1, std::tuple<Args...>>::type>::value
  , typename std::tuple_element<sizeof...(Args) - 1, std::tuple<Args...>>::type
  , void>
{
};

template <typename T>
struct progress_type;

template <typename T>
struct progress_type<progress<T>>
{
  typedef T type;
};

template <>
struct progress_type<void>
{
   typedef void type;
};
  
template <typename...Futures>
struct all_result_type;

template <typename...Args>
struct all_result_type<shared_future<Args...>>
{
  typedef shared_future<Args...> type;
};

template <typename...Args1, typename...Args2>
struct all_result_type<shared_future<Args1...>, shared_future<Args2...>>
{
  typedef shared_future<Args1..., Args2...> type;
};

template <typename...Args1, typename...Args2, typename...OtherFutures>
struct all_result_type<shared_future<Args1...>, shared_future<Args2...>, OtherFutures...>
{
  typedef typename all_result_type<shared_future<Args1..., Args2...>, OtherFutures...>::type type;
};

template <typename...Futures>
typename all_result_type<Futures...>::type
all_impl(Futures const& ... futures)
{
  Efl_Future* future = ::efl_future_all_internal(futures.native_handle()..., NULL);
  return typename all_result_type<Futures...>::type{ ::efl_ref(future)};
}

template <typename...Futures>
struct race_result_type;

template <typename...Args>
struct race_result_type<shared_future<Args...>>
{
  typedef shared_race_future<Args...> type;
};

template <typename T, typename...Args>
struct race_compose_impl;

template <typename T, typename A0, typename...Args>
struct race_compose_impl<T, A0, Args...>
{
  typedef typename std::conditional<eina::_mpl::tuple_contains<A0, T>::value
                                    , typename race_compose_impl<T, Args...>::type
                                    , typename race_compose_impl<typename eina::_mpl::push_back<T, A0>::type, Args...>::type
                                    >::type type;
};

template <typename T>
struct race_compose_impl<T>
{
   typedef T type;
};

template <typename T>
struct variant_from_tuple;

template <typename...Args>
struct variant_from_tuple<std::tuple<Args...>>
{
  typedef eina::variant<Args...> type;
};
  
template <typename...Args>
struct race_variant
{
  typedef typename variant_from_tuple<typename race_compose_impl<std::tuple<>, Args...>::type>::type type;
};
  
template <typename A0>
struct race_result_type<shared_future<A0>>
{
  typedef shared_race_future<A0> type;
};

template <typename A0>
struct race_result_type<shared_future<eina::variant<A0>>>
{
  typedef shared_race_future<A0> type;
};
  
template <typename...Args1, typename...Args2>
struct race_result_type<shared_future<Args1...>, shared_future<Args2...>>
{
  typedef typename race_result_type<shared_future<typename race_variant<Args1..., Args2...>::type>>::type type;
};

template <typename...Args1, typename...Args2, typename...OtherFutures>
struct race_result_type<shared_future<Args1...>, shared_future<Args2...>, OtherFutures...>
{
  typedef typename race_result_type<shared_future<typename race_variant<Args1..., Args2...>::type>
                                    , OtherFutures...>::type type;
};

template <typename...Futures>
typename race_result_type<Futures...>::type
race_impl(Futures const& ... futures)
{
  Efl_Future* future = ::efl_future_race_internal(futures.native_handle()..., NULL);
  return typename race_result_type<Futures...>::type{ ::efl_ref(future)};
}

template <typename T, typename Enabler = void>
struct future_copy_traits
{
   static void copy(T* storage, Efl_Future_Event_Success const* info)
   {
      eina::copy_from_c_traits<T>::copy_to_unitialized
        (storage, info->value);
   }
   static void copy_race(T* storage, Efl_Future_Event_Success const* info)
   {
      Efl_Future_Race_Success const* race = static_cast<Efl_Future_Race_Success const*>(info->value);
      eina::copy_from_c_traits<T>::copy_to_unitialized
        (storage, race->value);
   }
};

template <typename...Args>
struct future_copy_traits<eina::variant<Args...>>
{
   template <std::size_t I>
   static void copy_impl(eina::variant<Args...>*, void const*, int, std::integral_constant<std::size_t, I>
                         , std::integral_constant<std::size_t, I>)
   {
     std::abort();
   }

   template <std::size_t I, std::size_t N>
   static void copy_impl(eina::variant<Args...>* storage, void const* value, int index, std::integral_constant<std::size_t, I>
                         , std::integral_constant<std::size_t, N> max
                         , typename std::enable_if<I != N>::type* = 0)
   {
     if(I == index)
       {
         eina::copy_from_c_traits<eina::variant<Args...>>::copy_to_unitialized
           (storage, static_cast<typename std::tuple_element<I, std::tuple<Args...>>::type const*>
            (static_cast<void const*>(value)));
       }
     else
       copy_impl(storage, value, index, std::integral_constant<std::size_t, I+1>{}, max);
   }
   static void copy(eina::variant<Args...>*, Efl_Future_Event_Success const*)
   {
     std::abort();
   }
   static void copy_race(eina::variant<Args...>* storage, Efl_Future_Event_Success const* other_info)
   {
      Efl_Future_Race_Success const* info = static_cast<Efl_Future_Race_Success const*>
        (static_cast<void const*>(other_info->value));
      copy_impl(storage, info->value, info->index, std::integral_constant<std::size_t, 0ul>{}
                , std::integral_constant<std::size_t, sizeof...(Args)>{});
   }
};
  
template <typename A0, typename F, bool IsRace>
typename std::enable_if
<
  !std::is_same<A0, void>::value
  && !std::is_same<typename std::result_of<F(A0)>::type, void>::value
>::type
future_invoke(F f, Efl_Event const* event, std::integral_constant<bool, IsRace> /* is_race */)
{
   Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);
   try
     {
        typename std::aligned_storage<sizeof(A0), alignof(A0)>::type storage;
        if(IsRace)
          future_copy_traits<A0>::copy_race(static_cast<A0*>(static_cast<void*>(&storage)), info);
        else
          future_copy_traits<A0>::copy(static_cast<A0*>(static_cast<void*>(&storage)), info);
        auto r = f(*static_cast<A0*>(static_cast<void*>(&storage)));
        typedef decltype(r) result_type;
        typedef typename eina::alloc_to_c_traits<result_type>::c_type c_type;
        c_type* c_value = eina::alloc_to_c_traits<result_type>::copy_alloc(r);
        efl_promise_value_set(info->next, c_value, & eina::alloc_to_c_traits<result_type>::free_alloc);
     }
   catch(...)
     {
     }
}

template <typename A0, typename F, bool IsRace>
typename std::enable_if<std::is_same<A0, void>::value>::type
future_invoke(F f, Efl_Event const* event, std::integral_constant<bool, IsRace>)
{
   Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);
   try
     {
        f();
     }
   catch(...)
     {
     }
}

template <std::size_t N, typename...Args, typename...StorageArgs>
static void future_invoke_impl_read_accessor
  (Eina_Accessor*, std::tuple<StorageArgs...>&, std::tuple<Args...>*, std::true_type)
{
}

template <std::size_t N, typename...Args, typename...StorageArgs>
static void future_invoke_impl_read_accessor
  (Eina_Accessor* accessor
   , std::tuple<StorageArgs...>& storage_tuple
   , std::tuple<Args...>* args
   , std::false_type)
{
   typedef std::tuple<Args...> tuple_type;
   typedef typename std::tuple_element<N, tuple_type>::type type;
   void* value;
   if(eina_accessor_data_get(accessor, N, &value))
     {
        eina::copy_from_c_traits<type>::copy_to_unitialized
          (static_cast<type*>(static_cast<void*>(&std::get<N>(storage_tuple))), value);

        _impl::future_invoke_impl_read_accessor<N+1>
          (accessor, storage_tuple, args
           , std::integral_constant<bool, (N+1 == sizeof...(Args))>());
     }
   else
     {
        std::abort();
        // some error
     }
}
  
template <typename F, typename...Args, std::size_t...I, bool IsRace>
void future_invoke_impl(F f, Efl_Event const* event, std::tuple<Args...>* arguments_dummy, std::integral_constant<bool, IsRace>, eina::index_sequence<I...>)
{
   Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);
   try
     {
        typedef std::tuple<Args...> arguments;
        typedef std::tuple<typename std::aligned_storage<sizeof(Args), alignof(Args)>::type...>
          storage_tuple_type;
        storage_tuple_type storage_tuple;

        future_invoke_impl_read_accessor<0ul>
          (static_cast<Eina_Accessor*>(info->value)
           , storage_tuple
           , arguments_dummy, std::false_type{});

        auto r = f(*static_cast<typename std::tuple_element<I, arguments>::type*>
                   (static_cast<void*>(&std::get<I>(storage_tuple)))...);

        typedef decltype(r) result_type;
        typedef typename eina::alloc_to_c_traits<result_type>::c_type c_type;
        c_type* c_value = eina::alloc_to_c_traits<result_type>::copy_alloc(r);
        efl_promise_value_set(info->next, c_value, & eina::alloc_to_c_traits<result_type>::free_alloc);
     }
   catch(...)
     {
     }
}
  
template <typename A0, typename A1, typename...OtherArgs, typename F, bool IsRace>
void
future_invoke(F f, Efl_Event const* event, std::integral_constant<bool, IsRace> race)
{
  std::tuple<A0, A1, OtherArgs...>* p = nullptr;
  _impl::future_invoke_impl(f, event, p, race, eina::make_index_sequence<sizeof...(OtherArgs) + 2>{});
}
  
} }

#endif
