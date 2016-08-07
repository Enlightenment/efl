///
/// @file eo_concrete.hh
///

#ifndef EFL_CXX_EO_PROMISE_HH
#define EFL_CXX_EO_PROMISE_HH

#include <Efl.h>

#include <Eina.hh>
#include <Ecore_Manual.hh>

#include <mutex>
#include <condition_variable>

namespace efl {

namespace _impl {

template <typename F, typename...Args>
struct future_invoke_result_of;

template <typename F, typename A0>
struct future_invoke_result_of<F, A0>
{
   typedef typename std::result_of<F(A0)>::type type;
};

template <typename F>
struct future_invoke_result_of<F, void>
{
   typedef typename std::result_of<F()>::type type;
};
  
template <typename A0, typename F>
typename std::enable_if
<
  !std::is_same<A0, void>::value
  && !std::is_same<typename std::result_of<F(A0)>::type, void>::value
>::type
future_invoke(F f, Eo_Event const* event)
{
   Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);
   try
     {
        typename std::aligned_storage<sizeof(A0), alignof(A0)>::type storage;
        eina::copy_from_c_traits<A0>::copy_to_unitialized
          (static_cast<A0*>(static_cast<void*>(&storage)), info->value);
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

template <typename A0, typename F>
typename std::enable_if<std::is_same<A0, void>::value>::type
future_invoke(F f, Eo_Event const* event)
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
  
template <typename A0, typename A1, typename...OtherArgs, typename F>
// typename future_invoke_result_of<F, A0, A1, OtherArgs...>::type
void
future_invoke(F f, Eo_Event const* event)
{
   Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);
   
}
  
}
  
template <typename T>
struct progress;

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

static void get_error_cb(void* data, Eo_Event const* event)
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
        eo_unref(_future);
   }
   shared_future_common(shared_future_common const& future)
     : _future(eo_ref(future._future))
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
   static void wait_success(void* data, Eo_Event const*)
   {
      struct wait_state<>* wait_state = static_cast<struct wait_state<>*>(data);
      std::unique_lock<std::mutex> l(wait_state->mutex);
      wait_state->available = true;
      wait_state->cv.notify_one();
   }

   typedef Efl_Future* native_handle_type;
   typedef Efl_Future const* const_native_handle_type;
   native_handle_type native_handle() noexcept { return _future; }
   const_native_handle_type native_handle() const noexcept { return _future; }

   typedef shared_future_common _self_type;
   Efl_Future* _future;
};
  
template <typename T>
struct shared_future_1_type : private shared_future_common
{
   typedef shared_future_common _base_type;

   using _base_type::_base_type;
   using _base_type::swap;
   using _base_type::valid;
   using _base_type::native_handle;
   using _base_type::wait;
   typedef _base_type::native_handle_type native_handle_type;

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

   static void get_success(void* data, Eo_Event const* event)
   {
      struct wait_state<T>* wait_state = static_cast<struct wait_state<T>*>(data);
      Efl_Future_Event_Success* info = static_cast<Efl_Future_Event_Success*>(event->info);

      std::unique_lock<std::mutex> l(wait_state->mutex);
      eina::copy_from_c_traits<T>::copy_to_unitialized
        (static_cast<T*>(static_cast<void*>(&wait_state->storage)), info->value);
      wait_state->available = true;
      wait_state->cv.notify_one();
   }

   typedef shared_future_1_type<T> _self_type;
};

template <typename...Args>
struct shared_future_varargs_type : private shared_future_common
{
   typedef shared_future_common _base_type;

   using _base_type::_base_type;
   using _base_type::swap;
   using _base_type::valid;
   using _base_type::native_handle;
   using _base_type::wait;
   typedef _base_type::native_handle_type native_handle_type;

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

          std::cout << "copied value " << *static_cast<type*>(static_cast<void*>(&std::get<N>(storage_tuple)))
                    << std::endl;
     
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
  
   static void get_success(void* data, Eo_Event const* event)
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
struct shared_future : private std::conditional<sizeof...(Args) == 1, _impl::shared_future_1_type<typename std::tuple_element<0u, std::tuple<Args...>>::type>, _impl::shared_future_varargs_type<Args...>>::type
{
   typedef typename std::conditional<sizeof...(Args) == 1, _impl::shared_future_1_type<typename std::tuple_element<0u, std::tuple<Args...>>::type>, _impl::shared_future_varargs_type<Args...>>::type _base_type;

   using _base_type::_base_type;
   using _base_type::swap;
   using _base_type::valid;
   using _base_type::get;
   using _base_type::wait;
   using _base_type::native_handle;
   typedef typename _base_type::native_handle_type native_handle_type;
};

template <typename...Args, typename Success, typename Error>
shared_future
<
  typename std::enable_if
  <
    !std::is_same<void, typename std::tuple_element<0, std::tuple<Args...>>::type>::value
    && !std::is_same<void, typename std::result_of<Success(Args...)>::type>::value
    , typename std::result_of<Success(Args...)>::type
  >::type
> then(shared_future<Args...> future, Success success_cb, Error error_cb)
{
  struct private_data
  {
    Success success_cb;
    Error error_cb;
    shared_future<Args...> future;
  };
  private_data* pdata = new private_data
    {std::move(success_cb), std::move(error_cb), std::move(future)};
     
  Eo_Event_Cb raw_success_cb =
    [] (void* data, Eo_Event const* event)
    {
       private_data* pdata = static_cast<private_data*>(data);
       try
         {
           _impl::future_invoke<Args...>(pdata->success_cb, event);
           // should value_set the next promise
         }
       catch(...)
         {
           // should fail the next promise
         }
       delete pdata;
    };
  Eo_Event_Cb raw_error_cb =
    [] (void* data, Eo_Event const* event)
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
  return shared_future<typename std::result_of<Success(Args...)>::type>{eo_ref(new_future)};
}

template <typename...Args, typename F>
void then(shared_future<Args...> future, F function)
{
  
}

template <typename...Args>
struct promise
{
  
};
    
}

#endif
