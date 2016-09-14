///
/// @file eo_promise.hh
///

#ifndef EFL_CXX_EO_PROMISE_HH
#define EFL_CXX_EO_PROMISE_HH

#include <Efl.h>

#include <Eina.hh>
#include <Ecore_Manual.hh>

#include <mutex>
#include <condition_variable>

#include <eina_tuple.hh>
#include <eo_promise_meta.hh>
#include <eo_future.hh>

namespace efl {

template <typename...Args>
struct shared_future;
  
template <typename T>
struct progress;

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

namespace _impl {

struct promise_common
{
   explicit promise_common(Efl_Promise* _promise) : _promise(_promise) {}
   explicit promise_common(std::nullptr_t) : _promise(nullptr) {}
   promise_common()
   {
      _promise = efl_add(EFL_PROMISE_CLASS, ecore_main_loop_get());
   }
   ~promise_common()
   {
      if(_promise)
        ::efl_unref(_promise);
   }
   promise_common(promise_common const& other)
     : _promise( ::efl_ref(other._promise))
   {
   }
   promise_common(promise_common&& other)
     : _promise(nullptr)
   {
     std::swap(*this, other);
   }
   promise_common& operator=(promise_common const& other)
   {
      _promise =  ::efl_ref(other._promise);
      return *this;
   }
   promise_common& operator=(promise_common&& other)
   {
      std::swap(*this, other);
      return *this;
   }
   bool valid() const
   {
     return _promise != nullptr;
   }
   void swap(promise_common& other)
   {
      std::swap(*this, other);
   }
   void set_exception(std::exception_ptr /*p*/)
   {
   }
  
   Efl_Promise* _promise;
};
  
template <typename T>
struct promise_1_type : promise_common
{
   typedef promise_common _base_type;
   using _base_type::_base_type;
   using _base_type::swap;
   using _base_type::set_exception;
  
   void set_value(T const& v)
   {
      typedef typename eina::alloc_to_c_traits<T>::c_type c_type;
      c_type* c_value = eina::alloc_to_c_traits<T>::copy_alloc(v);
      efl_promise_value_set(this->_promise, c_value, & eina::alloc_to_c_traits<T>::free_alloc);
   }
   void set_value(T&& v)
   {
      typedef typename eina::alloc_to_c_traits<T>::c_type c_type;
      c_type* c_value = eina::alloc_to_c_traits<T>::copy_alloc(std::move(v));
      efl_promise_value_set(this->_promise, c_value, & eina::alloc_to_c_traits<T>::free_alloc);
   }
};

template <>
struct promise_1_type<void> : promise_common
{
   typedef promise_common _base_type;
   using _base_type::_base_type;
   using _base_type::swap;
   using _base_type::set_exception;
  
   void set_value()
   {
      efl_promise_value_set(this->_promise, nullptr, nullptr);
   }
};
  
}
  
template <typename T, typename Progress = void>
struct promise : private _impl::promise_1_type<T>
{
   typedef _impl::promise_1_type<T> _base_type;
   using _base_type::_base_type;
   using _base_type::set_value;
   using _base_type::set_exception;

   shared_future<T> get_future()
   {
      return shared_future<T>{ ::efl_ref( ::efl_promise_future_get(this->_promise)) };
   }

   void swap(promise<T>& other)
   {
      _base_type::swap(other);
   }
};

template <typename...Args>
void swap(promise<Args...>& lhs, promise<Args...>& rhs)
{
   lhs.swap(rhs);
}

}

#endif
