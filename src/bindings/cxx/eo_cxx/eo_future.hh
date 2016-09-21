///
/// @file eo_future.hh
///

#ifndef EFL_CXX_EO_FUTURE_HH
#define EFL_CXX_EO_FUTURE_HH

#include <Efl.h>

#include <Eina.hh>
#include <Ecore_Manual.hh>

#include <mutex>
#include <condition_variable>

#include <eina_tuple.hh>
#include <eo_promise_meta.hh>

namespace efl {

template <typename...Args>
struct shared_future;

namespace _impl {

template <typename V = char>
struct wait_state
{
   bool available = false;
   bool has_failed = false;
   std::mutex mutex;
   std::condition_variable cv;
   typename std::aligned_storage<sizeof(V), alignof(V)>::type storage;
   Eina_Error error;
};

static void get_error_cb(void* data, Efl_Event const* event)
{ 
   struct wait_state<>* wait_state = static_cast<struct wait_state<>*>(data);
   Efl_Future_Event_Failure* info = static_cast<Efl_Future_Event_Failure*>(event->info);
   std::unique_lock<std::mutex> l(wait_state->mutex);
   wait_state->error = info->error;
   wait_state->has_failed = true;
   wait_state->available = true;
   wait_state->cv.notify_one();
}

struct shared_future_common
{
   explicit shared_future_common(Efl_Future* future)
     : _future(future) {}
   shared_future_common()
     : _future(nullptr) {}
   ~shared_future_common()
   {
      if(_future)
        efl_unref(_future);
   }
   shared_future_common(shared_future_common const& future)
     : _future(efl_ref(future._future))
   {
   }
   shared_future_common& operator=(shared_future_common const& other)
   {
      _self_type tmp(other);
      tmp.swap(*this);
      return *this;
   }
   shared_future_common(shared_future_common&& future)
     : _future(future._future)
   {
      future._future = nullptr;
   }
   shared_future_common& operator=(shared_future_common&& other)
   {
      other.swap(*this);
      return *this;
   }
   void swap(shared_future_common& other)
   {
      std::swap(_future, other._future);
   }
   bool valid() const noexcept
   {
      return _future != nullptr;
   }
   void wait() const
   {
      if(eina_main_loop_is())
        throw std::runtime_error("Deadlock");

      struct wait_state<> wait_state;
      
      efl::ecore::main_loop_thread_safe_call_async
        ([&]
         {
            efl_future_then(this->_future, &wait_success, &wait_success, nullptr, &wait_state);
         });

      std::unique_lock<std::mutex> lock(wait_state.mutex);
      while(!wait_state.available)
        wait_state.cv.wait(lock);
   }
   static void wait_success(void* data, Efl_Event const*)
   {
      struct wait_state<>* wait_state = static_cast<struct wait_state<>*>(data);
      std::unique_lock<std::mutex> l(wait_state->mutex);
      wait_state->available = true;
      wait_state->cv.notify_one();
   }

   typedef Efl_Future* native_handle_type;
   native_handle_type native_handle() const noexcept { return _future; }

   typedef shared_future_common _self_type;
   Efl_Future* _future;
};
  
template <typename T, typename Progress = void>
struct shared_future_1_type : shared_future_common
{
   typedef shared_future_common _base_type;

   using _base_type::_base_type;
   shared_future_1_type() = default;
   shared_future_1_type(shared_future_common const& other)
     : _base_type(other) {}

   T get() const
   {
      if(eina_main_loop_is())
        throw std::runtime_error("Deadlock");

      struct wait_state<T> wait_state;

      efl::ecore::main_loop_thread_safe_call_async
        ([&]
         {
            efl_future_then(this->_future, &get_success, &_impl::get_error_cb, nullptr, &wait_state);
         });

      {
        std::unique_lock<std::mutex> lock(wait_state.mutex);
        while(!wait_state.available)
          wait_state.cv.wait(lock);
      }
      if(wait_state.has_failed)
        EFL_CXX_THROW(eina::system_error(eina::error_code(wait_state.error, eina::eina_error_category()), "EFL Eina Error"));
      return *static_cast<T*>(static_cast<void*>(&wait_state.storage));
   }

   static void get_success(void* data, Efl_Event const* event)
   {
      struct wait_state<T>* wait_state = static_cast<struct wait_state<T>*>(data);
      Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);

      std::unique_lock<std::mutex> l(wait_state->mutex);
      _impl::future_copy_traits<T>::copy(static_cast<T*>(static_cast<void*>(&wait_state->storage)), info);
      wait_state->available = true;
      wait_state->cv.notify_one();
   }

   typedef shared_future_1_type<T, Progress> _self_type;
};

template <typename T>
struct shared_race_future_1_type : shared_future_common
{
   typedef shared_future_common _base_type;

   using _base_type::_base_type;
   shared_race_future_1_type(_base_type const& other)
     : _base_type(other) {}

   T get() const
   {
      if(eina_main_loop_is())
        throw std::runtime_error("Deadlock");

      struct wait_state<T> wait_state;

      efl::ecore::main_loop_thread_safe_call_async
        ([&]
         {
            efl_future_then(this->_future, &get_success, &_impl::get_error_cb, nullptr, &wait_state);
         });

      {
        std::unique_lock<std::mutex> lock(wait_state.mutex);
        while(!wait_state.available)
          wait_state.cv.wait(lock);
      }
      if(wait_state.has_failed)
        EFL_CXX_THROW(eina::system_error(eina::error_code(wait_state.error, eina::eina_error_category()), "EFL Eina Error"));
      return *static_cast<T*>(static_cast<void*>(&wait_state.storage));
   }

   static void get_success(void* data, Efl_Event const* event)
   {
      struct wait_state<T>* wait_state = static_cast<struct wait_state<T>*>(data);
      Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);

      std::unique_lock<std::mutex> l(wait_state->mutex);
      _impl::future_copy_traits<T>::copy_race(static_cast<T*>(static_cast<void*>(&wait_state->storage)), info);
      wait_state->available = true;
      wait_state->cv.notify_one();
   }

   typedef shared_race_future_1_type<T> _self_type;
};

template <typename...Args>
struct shared_future_varargs_type : shared_future_common
{
   typedef shared_future_common _base_type;

   using _base_type::_base_type;
   shared_future_varargs_type() = default;
   shared_future_varargs_type(_base_type const& other)
     : _base_type(other) {}

   typedef std::tuple<Args...> tuple_type;

   std::tuple<Args...> get() const
   {
      if(eina_main_loop_is())
        throw std::runtime_error("Deadlock");

      struct wait_state<tuple_type> wait_state;

      efl::ecore::main_loop_thread_safe_call_async
        ([&]
         {
            efl_future_then(this->_future, &get_success, &_impl::get_error_cb, nullptr, &wait_state);
         });

      {
        std::unique_lock<std::mutex> lock(wait_state.mutex);
        while(!wait_state.available)
          wait_state.cv.wait(lock);
      }
      if(wait_state.has_failed)
        EFL_CXX_THROW(eina::system_error(eina::error_code(wait_state.error, eina::eina_error_category()), "EFL Eina Error"));
      return *static_cast<tuple_type*>(static_cast<void*>(&wait_state.storage));
   }

   template <std::size_t N>
   static void read_accessor(Eina_Accessor* accessor
                             , std::tuple<typename std::aligned_storage<sizeof(Args), alignof(Args)>::type...>& storage_tuple
                             , wait_state<tuple_type>* wait_state
                             , std::false_type)
   {
      typedef typename std::tuple_element<N, tuple_type>::type type;
      void* value;
      if(eina_accessor_data_get(accessor, N, &value))
        {
          eina::copy_from_c_traits<type>::copy_to_unitialized
            (static_cast<type*>(static_cast<void*>(&std::get<N>(storage_tuple))), value);

          _self_type::read_accessor<N+1>(accessor, storage_tuple, wait_state
                                         , std::integral_constant<bool, (N+1 == sizeof...(Args))>());
        }
      else
        {
          std::abort();
          // some error
        }
   }

   template <std::size_t N, std::size_t...I>
   static void read_accessor_end(std::tuple<typename std::aligned_storage<sizeof(Args), alignof(Args)>::type...>& storage_tuple
                                 , wait_state<tuple_type>* wait_state
                                 , eina::index_sequence<I...>)
   {
      std::unique_lock<std::mutex> l(wait_state->mutex);

      new (&wait_state->storage) tuple_type{(*static_cast<typename std::tuple_element<I, tuple_type>::type*>
                                             (static_cast<void*>(&std::get<I>(storage_tuple))))...};

      wait_state->available = true;
      wait_state->cv.notify_one();
   }
  
   template <std::size_t N>
   static void read_accessor(Eina_Accessor*
                             , std::tuple<typename std::aligned_storage<sizeof(Args), alignof(Args)>::type...>& storage_tuple
                             , wait_state<tuple_type>* wait_state
                             , std::true_type)
   {
      _self_type::read_accessor_end<N>(storage_tuple, wait_state, eina::make_index_sequence<sizeof...(Args)>{});
   }
  
   static void get_success(void* data, Efl_Event const* event)
   {
      struct wait_state<tuple_type>* wait_state = static_cast<struct wait_state<tuple_type>*>(data);
      Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);

      Eina_Accessor* accessor = static_cast<Eina_Accessor*>(info->value);
      std::tuple<typename std::aligned_storage<sizeof(Args), alignof(Args)>::type...> storage_tuple;

      _self_type::read_accessor<0u>(accessor, storage_tuple, wait_state, std::false_type());
   }
  
   typedef shared_future_varargs_type<Args...> _self_type;
};
  
}

template <typename...Args>
struct shared_future : private
  std::conditional
  <
    sizeof...(Args) == 1
    , _impl::shared_future_1_type<typename std::tuple_element<0u, std::tuple<Args...>>::type>
    , typename std::conditional
      <_impl::is_progress<typename std::tuple_element<sizeof...(Args) - 1, std::tuple<Args...>>::type>::value
      , typename std::conditional
        <sizeof...(Args) == 2
        , _impl::shared_future_1_type<Args...>
         , _impl::shared_future_varargs_type<Args...>
         >::type
       , _impl::shared_future_varargs_type<Args...>
       >::type
  >::type
{
   typedef typename
  std::conditional
  <
    sizeof...(Args) == 1
    , _impl::shared_future_1_type<Args...>
    , typename std::conditional
      <_impl::is_progress<typename std::tuple_element<sizeof...(Args) - 1, std::tuple<Args...>>::type>::value
      , typename std::conditional
        <sizeof...(Args) == 2
        , _impl::shared_future_1_type<Args...>
         , _impl::shared_future_varargs_type<Args...>
         >::type
       , _impl::shared_future_varargs_type<Args...>
       >::type
  >::type
     _base_type;
   typedef typename _impl::progress_param<Args...>::type progress_param_type;
   typedef typename _impl::progress_type<progress_param_type>::type progress_type;
   typedef typename _base_type::native_handle_type native_handle_type;
   using _base_type::_base_type;
   using _base_type::swap;
   using _base_type::valid;
   using _base_type::get;
   using _base_type::wait;
   using _base_type::native_handle;

   shared_future() = default;
   template <typename...OtherArgs>
   shared_future(shared_future<OtherArgs...> const& other
                 , typename std::enable_if<_impl::is_progress_param_compatible
                 <progress_param_type, typename _impl::progress_param<OtherArgs...>::type>::value>::type* = nullptr)
     : _base_type(static_cast< _impl::shared_future_common const&>(other))
   {
   }

   template <typename...OtherArgs>
   friend struct shared_future;
};

template <typename...Args>
struct shared_race_future : private std::conditional<sizeof...(Args) == 1, _impl::shared_race_future_1_type<typename std::tuple_element<0u, std::tuple<Args...>>::type>, void>::type
{
   typedef typename std::conditional<sizeof...(Args) == 1, _impl::shared_race_future_1_type<typename std::tuple_element<0u, std::tuple<Args...>>::type>, void>::type _base_type;

   using _base_type::_base_type;
   using _base_type::swap;
   using _base_type::valid;
   using _base_type::get;
   using _base_type::wait;
   using _base_type::native_handle;
   typedef typename _base_type::native_handle_type native_handle_type;
};

namespace _impl {

template <typename T>
struct is_race_future : std::false_type {};

template <typename...Args>
struct is_race_future<shared_race_future<Args...>> : std::true_type {};

}

template <template <typename...> class Future, typename...Args, typename F>
typename std::enable_if
<
   !std::is_same<typename Future<Args...>::progress_type, void>::value
>::type on_progress(Future<Args...> future, F function)
{
  struct private_data
  {
    F progress_cb;
    Future<Args...> future;
  };
  private_data* pdata = new private_data
    {std::move(function), std::move(future)};

  typedef typename Future<Args...>::progress_type progress_type;
  
  Efl_Event_Cb raw_progress_cb =
    [] (void* data, Efl_Event const* event)
    {
       private_data* pdata = static_cast<private_data*>(data);
       try
         {
           Efl_Future_Event_Progress const* info = static_cast<Efl_Future_Event_Progress const*>(event->info);
           pdata->progress_cb(*static_cast<progress_type const*>(info->progress));
         }
       catch(...)
         {
           // what should happen if progress_cb fails?
         }
    };
  Efl_Event_Cb raw_delete_cb =
    [] (void* data, Efl_Event const*)
    {
       private_data* pdata = static_cast<private_data*>(data);
       delete pdata;
    };

  assert(pdata->future.valid());
  efl_future_then(pdata->future.native_handle(), raw_delete_cb, raw_delete_cb, raw_progress_cb, pdata);
}   

template <template <typename...> class Future, typename...Args, typename Success, typename Error>
shared_future
<
  typename std::enable_if
  <
    !std::is_same<void, typename std::tuple_element<0, std::tuple<Args...>>::type>::value
    && !std::is_same<void, typename std::result_of<Success(Args...)>::type>::value
    , typename std::result_of<Success(Args...)>::type
  >::type
> then(Future<Args...> future, Success success_cb, Error error_cb)
{
  struct private_data
  {
    Success success_cb;
    Error error_cb;
    Future<Args...> future;
  };
  private_data* pdata = new private_data
    {std::move(success_cb), std::move(error_cb), std::move(future)};
     
  Efl_Event_Cb raw_success_cb =
    [] (void* data, Efl_Event const* event)
    {
       private_data* pdata = static_cast<private_data*>(data);
       try
         {
           _impl::future_invoke<Args...>(pdata->success_cb, event, _impl::is_race_future<Future<Args...>>{});
           // should value_set the next promise
         }
       catch(...)
         {
           // should fail the next promise
         }
       delete pdata;
    };
  Efl_Event_Cb raw_error_cb =
    [] (void* data, Efl_Event const* event)
    {
       private_data* pdata = static_cast<private_data*>(data);
       Efl_Future_Event_Failure* info = static_cast<Efl_Future_Event_Failure*>(event->info);
       pdata->error_cb(eina::error_code(info->error, eina::eina_error_category()));
       // should error the next promise (or should the promise do that for me automatically?)
       delete pdata;
    };
  
      assert(pdata->future.valid());
  Efl_Future* new_future
    = efl_future_then(pdata->future.native_handle(), raw_success_cb, raw_error_cb, nullptr, pdata);
  return shared_future<typename std::result_of<Success(Args...)>::type>{efl_ref(new_future)};
}
  
template <typename...Args, typename F>
void then(shared_future<Args...> future, F function)
{
  
}

template <typename...Args1, typename...Args2, typename...Futures>
typename _impl::all_result_type<shared_future<Args1...>, shared_future<Args2...>, Futures...>::type
all(shared_future<Args1...> future1, shared_future<Args2...> future2, Futures...futures)
{
  return _impl::all_impl(future1, future2, futures...);
}

template <typename...Args1, typename...Args2, typename...Futures>
typename _impl::race_result_type<shared_future<Args1...>, shared_future<Args2...>, Futures...>::type
race(shared_future<Args1...> future1, shared_future<Args2...> future2, Futures...futures)
{
  return _impl::race_impl(future1, future2, futures...);
}

}

#endif
