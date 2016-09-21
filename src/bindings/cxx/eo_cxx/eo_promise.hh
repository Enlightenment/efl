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

template <typename P>
struct promise_progress : promise_common
{
   void set_progress(P const& progress)
   {
      efl_promise_progress_set(this->_promise, &progress);
   }
};

template <>
struct promise_progress<void> : promise_common
{
   void set_progress()
   {
      efl_promise_progress_set(this->_promise, nullptr);
   }
};
  
template <typename T, typename Progress>
struct promise_1_type : promise_progress<Progress>
{
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

template <typename Progress>
struct promise_1_type<void, Progress> : promise_progress<Progress>
{
   void set_value()
   {
      efl_promise_value_set(this->_promise, nullptr, nullptr);
   }
};
  
}
  
template <typename T, typename Progress = void>
struct promise : private _impl::promise_1_type<T, Progress>
{
   typedef _impl::promise_1_type<T, Progress> _base_type;
   using _base_type::_base_type;
   using _base_type::set_value;
   using _base_type::set_progress;
   using _base_type::set_exception;

   shared_future<T, progress<Progress>> get_future()
   {
      return shared_future<T, progress<Progress>>{ ::efl_ref( ::efl_promise_future_get(this->_promise)) };
   }

   void swap(promise<T, progress<Progress>>& other)
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
