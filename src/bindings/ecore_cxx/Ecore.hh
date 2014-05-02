#ifndef _EFL_ECORE_CXX_ECORE_HH
#define _EFL_ECORE_CXX_ECORE_HH

#include <Ecore.h>

#include <Eina.hh>

#include <utility>
#include <type_traits>
#include <memory>
#include <cstring>

#ifdef EFL_BETA_API_SUPPORT
#include <Ecore.eo.hh>
#endif

namespace efl { namespace ecore {

template <typename T>
struct _identity
{
  typedef T type;
};

template <typename F>
void _ecore_main_loop_thread_safe_call_async_callback(void* data)
{
  std::unique_ptr<F> f (static_cast<F*>(data));
  try
    {
      (*f)();
    }
  catch(std::bad_alloc const& e)
    {
      eina_error_set( ::EINA_ERROR_OUT_OF_MEMORY);
    }
  catch(std::system_error const& e)
    {
      efl::eina::set_error_code(e.code());
    }
  catch(...)
    {
      eina_error_set( efl::eina::unknown_error() );
    }
}

template <typename T>
struct _return_buffer
{
  typename std::aligned_storage<sizeof(T),std::alignment_of<T>::value>::type buffer;
};

template <>
struct _return_buffer<void>
{
};

template <typename F>
struct _data
{
  F& f;
  std::exception_ptr exception;
  typedef typename std::result_of<F()>::type result_type;
  _return_buffer<result_type> return_buffer;
};

template <typename F>
void* _ecore_main_loop_thread_safe_call_sync_callback_aux(_data<F>* d, _identity<void>)
{
  d->f();
  if(eina_error_get())
    d->exception = make_exception_ptr(std::system_error(efl::eina::get_error_code()));
  return 0;
}

template <typename F, typename R>
void* _ecore_main_loop_thread_safe_call_sync_callback_aux(_data<F>* d, _identity<R>)
{
  typedef R result_type;
  new (&d->return_buffer.buffer) result_type ( std::move(d->f()) );
  if(eina_error_get())
    {
      d->exception = make_exception_ptr(std::system_error(efl::eina::get_error_code()));
      eina_error_set(0);
      result_type* p = static_cast<result_type*>(static_cast<void*>(&d->return_buffer.buffer));
      p->~result_type();
    }
  return 0;
}

template <typename F>
void* _ecore_main_loop_thread_safe_call_sync_callback(void* data)
{
  _data<F>* d = static_cast<_data<F>*>(data);
  try
    {
      return _ecore_main_loop_thread_safe_call_sync_callback_aux
        (d, _identity<typename std::result_of<F()>::type>());
    }
  catch(std::bad_alloc const& e)
    {
      d->exception = std::current_exception();
    }
  catch(std::system_error const& e)
    {
      d->exception = std::current_exception();
    }
  catch(...)
    {
      d->exception = std::current_exception();
    }
  return 0;
}

template <typename F>
void main_loop_thread_safe_call_async(F&& f)
{
  ::ecore_main_loop_thread_safe_call_async( &ecore::_ecore_main_loop_thread_safe_call_async_callback<F>
                                            , new F(std::move(f)) );
}

template <typename F>
void _get_return_value(_data<F>& data, _identity<void>)
{
  if(data.exception)
    {
      std::rethrow_exception(data.exception);
    }
}

template <typename F, typename R>
R _get_return_value(_data<F>& data, _identity<R>)
{
  if(!data.exception)
    {
      R* b = static_cast<R*>(static_cast<void*>(&data.return_buffer.buffer));
      struct destroy
      {
        destroy(R* p) : p(p)
        {}
        ~destroy()
        {
          p->~R();
        }
        R* p;
      } destroy_temp(b);
      return std::move(*b);
    }
  else
    {
      std::rethrow_exception(data.exception);
    }
}

template <typename F>
typename std::result_of<F()>::type
main_loop_thread_safe_call_sync(F&& f)
{
  typedef typename std::result_of<F()>::type result_type;
  _data<F> data {f, nullptr, {}};
  ::ecore_main_loop_thread_safe_call_sync
      (&ecore::_ecore_main_loop_thread_safe_call_sync_callback<F>, &data);
  return _get_return_value(data, _identity<result_type>());
}

struct ecore_init
{
  ecore_init()
  {
    ::ecore_init();
  }
  ~ecore_init()
  {
    ::ecore_shutdown();
  }
};

} }

#endif
