#ifndef _EFL_ECORE_CXX_ECORE_HH
#define _EFL_ECORE_CXX_ECORE_HH

#include <Ecore.h>

#include <utility>
#include <type_traits>
#include <memory>
#include <cstring>

namespace efl { namespace ecore {

template <typename T, typename Enable = void>
struct _ecore_result_type_marshaller;

template <typename T>
struct _ecore_result_type_marshaller
  <T, typename std::enable_if<std::is_pointer<T>::value>::type>
{
  static void* to_void(T o)
  {
    return static_cast<void*>(o);
  }
  static T from_void(void* o)
  {
    return static_cast<T>(o);
  }
};

template <typename T>
struct _ecore_result_type_marshaller
 <T, typename std::enable_if<!std::is_pointer<T>::value && std::is_pod<T>::value
                              && sizeof(T) <= sizeof(void*)>::type>
{
  static void* to_void(T&& o)
  {
    unsigned char buf[sizeof(void*)];
    T* p = static_cast<T*>(static_cast<void*>(&buf[0]));
    new (p) T(std::move(o));
    void* store;
    std::memcpy(&store, buf, sizeof(void*));
    return store;
  }
  static T from_void(void* store)
  {
    T* p = static_cast<T*>(static_cast<void*>(&store));
    struct destroy_T
    {
      destroy_T(T& p)
        : p(p) {}
      ~destroy_T()
      {
        p.~T();
      }
      T& p;
    } destroy(*p);
    return T(std::move(*p));
  }
};

template <typename T>
struct _ecore_result_type_marshaller
<T, typename std::enable_if<(sizeof(T) > sizeof(void*)) || !std::is_pod<T>::value>::type>
{
  static void* to_void(T&& o)
  {
    return new T(o);
  }
  static T from_void(void* store)
  {
    std::unique_ptr<T> p(static_cast<T*>(store));
    return T(std::move(*p.get()));
  }
};

template <typename F>
void _ecore_main_loop_thread_safe_call_async_callback(void* data)
{
  F* f = static_cast<F*>(data);
  (*f)();
  delete f;
}

template <typename F>
void* _ecore_main_loop_thread_safe_call_sync_callback(void* data)
{
  F* f = static_cast<F*>(data);
  typedef typename std::result_of<F()>::type result_type;
  return _ecore_result_type_marshaller<result_type>::to_void((*f)());
}

template <typename F>
void main_loop_thread_safe_call_async(F&& f)
{
  ::ecore_main_loop_thread_safe_call_async( &ecore::_ecore_main_loop_thread_safe_call_async_callback<F>
                                            , new F(std::move(f)) );
}

template <typename F>
typename std::result_of<F()>::type
main_loop_thread_safe_call_sync(F&& f)
{
  void* d = ::ecore_main_loop_thread_safe_call_sync
    (&ecore::_ecore_main_loop_thread_safe_call_sync_callback<F>, &f);
  typedef typename std::result_of<F()>::type result_type;
  return _ecore_result_type_marshaller<result_type>::from_void(d);
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
