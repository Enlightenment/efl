#ifndef EFL_EO_JS_COMPATIBILITY_HH
#define EFL_EO_JS_COMPATIBILITY_HH

#include <type_traits>

#if 0
#include <v8.h>
#else
#include <node/v8.h>
#endif

namespace v8 {

template <typename T>
struct FunctionCallbackInfo;
  
}

namespace efl { namespace eo { namespace js {

template <typename T = v8::External, typename Enable = void>
struct _libv8_isolate_test;

template <typename T>
struct _libv8_isolate_test
<T, typename std::enable_if
 <std::is_same<decltype( & T::New)
               , v8::Local<T> (*)(v8::Isolate*, void*)>::value>::type>
  : std::true_type
{
};

template <typename T>
struct _libv8_isolate_test
<T, typename std::enable_if
 <std::is_same<decltype( & T::New)
               , v8::Local<T> (*)(void*)>::value>::type>
  : std::false_type
{
};

template <typename T = v8::FunctionTemplate, typename Enable = void>
struct _libv8_callback_info_test;

typedef v8::Handle<v8::Value>(*_libv8_invocation_callback)(v8::Arguments const&);
      
template <typename T>
struct _libv8_callback_info_test
<T, typename std::enable_if
 <!std::is_same<decltype( & T::SetCallHandler)
   , void (T::*)(_libv8_invocation_callback, v8::Handle<v8::Value>)>::value>::type>
  : std::true_type
{
};

template <typename T>
struct _libv8_callback_info_test
<T, typename std::enable_if
 <std::is_same<decltype( & T::SetCallHandler)
   , void (T::*)(_libv8_invocation_callback, v8::Handle<v8::Value>)>::value>::type>
  : std::false_type
{
};

static constexpr bool const v8_uses_isolate = _libv8_isolate_test<>::value;
static constexpr bool const v8_uses_callback_info = _libv8_callback_info_test<>::value;

using compatibility_return_type = std::conditional<v8_uses_callback_info, void, v8::Handle<v8::Value> >::type;
using compatibility_callback_info_type
 = std::conditional<v8_uses_callback_info, v8::FunctionCallbackInfo<v8::Value> const&, v8::Arguments const&>
  ::type;

static_assert(!v8_uses_callback_info, "");
static_assert(!v8_uses_isolate, "");

template <typename T>
struct compatibility_type_tag {};

template <typename T = std::integral_constant<bool, v8_uses_isolate> >
struct compatibility_string;

template <>
struct compatibility_string<std::true_type> : v8::String {};

template <>
struct compatibility_string<std::false_type> : v8::String
{
  static v8::Local<v8::String> NewFromUtf8(v8::Isolate*, const char* data)
  {
    return v8::String::New(data);
  }
};
      
template <typename...Args>
auto compatibility_new_impl(v8::Isolate* isolate, std::true_type, compatibility_type_tag<v8::String>
                            , Args...args) ->
  decltype(compatibility_string<>::NewFromUtf8(isolate, args...)) 
{
  return compatibility_string<>::NewFromUtf8(isolate, args...);
}

template <typename...Args>
auto compatibility_new_impl(nullptr_t, std::true_type, compatibility_type_tag<v8::String>
                            , Args...args) ->
  decltype(compatibility_string<>::NewFromUtf8(v8::Isolate::GetCurrent(), args...)) 
{
  return compatibility_string<>::NewFromUtf8(v8::Isolate::GetCurrent(), args...);
}
      
template <typename T, typename...Args>
auto compatibility_new_impl(v8::Isolate* isolate, std::true_type, compatibility_type_tag<T>
                            , Args...args) ->
  decltype(T::New(isolate, args...)) 
{
  return T::New(isolate, args...);
}

template <typename T, typename...Args>
auto compatibility_new_impl(v8::Isolate*, std::false_type, compatibility_type_tag<T>
                            , Args...args) ->
  decltype(T::New(args...)) 
{
  return T::New(args...);
}

template <typename T, typename...Args>
auto compatibility_new_impl(std::nullptr_t, std::true_type, compatibility_type_tag<T>
                            , Args...args) ->
  decltype(T::New(v8::Isolate::GetCurrent(), args...))
{
  return T::New(v8::Isolate::GetCurrent(), args...);
}

template <typename T, typename...Args>
auto compatibility_new_impl(std::nullptr_t, std::false_type, compatibility_type_tag<T>
                            , Args...args) ->
  decltype(T::New(args...)) 
{
  return T::New(args...);
}

template <typename T, typename...Args>
auto compatibility_new(v8::Isolate* isolate, Args...args) ->
  decltype(js::compatibility_new_impl<T>
           (isolate, std::integral_constant<bool, v8_uses_isolate>()
            , compatibility_type_tag<T>()
            , args...))
{
  return js::compatibility_new_impl(isolate, std::integral_constant<bool, v8_uses_isolate>()
                                    , compatibility_type_tag<T>()
                                    , args...);
}

template <typename T, typename...Args>
auto compatibility_new(nullptr_t, Args...args) ->
  decltype(js::compatibility_new_impl<T>(nullptr, std::integral_constant<bool, v8_uses_isolate>()
                                         , compatibility_type_tag<T>()
                                         , args...))
{
  return js::compatibility_new_impl<T>(nullptr, std::integral_constant<bool, v8_uses_isolate>()
                                       , compatibility_type_tag<T>()
                                       , args...);
}

template <typename T>
inline void compatibility_return_impl(T object, compatibility_callback_info_type, std::true_type)
{
  // should set to info.ReturnValue(object);
}

template <typename T>
inline v8::Handle<v8::Value>
compatibility_return_impl(T object, compatibility_callback_info_type, std::false_type)
{
  return object;
}
      
template <typename T>
compatibility_return_type
compatibility_return(T object, compatibility_callback_info_type args)
{
  return compatibility_return_impl(object, args, std::integral_constant<bool, v8_uses_callback_info>());
}

inline void compatibility_return_nil_impl(std::true_type) {}

inline v8::Handle<v8::Value>
compatibility_return_nil_impl(std::false_type)
{
  return v8::Handle<v8::Value>();
}

inline
compatibility_return_type
compatibility_return()
{
  return compatibility_return_nil_impl(std::integral_constant<bool, v8_uses_callback_info>());
}

template <typename T = std::integral_constant<bool, v8_uses_isolate> >
struct _v8_isolate_throw_exception;

template <>
struct _v8_isolate_throw_exception<std::true_type> : v8::Isolate
{
};

template <>
struct _v8_isolate_throw_exception<std::false_type> : v8::Isolate
{
  static v8::Handle<v8::Value> ThrowException(v8::Handle<v8::Value> v)
  {
    return v8::ThrowException(v);
  }
};

inline void
compatibility_throw_impl(v8::Isolate* isolate, v8::Local<v8::Value> exception, std::true_type)
{
  static_cast<_v8_isolate_throw_exception<>*>(isolate)->ThrowException(exception);
}

inline v8::Handle<v8::Value>
compatibility_throw_impl(v8::Isolate*, v8::Local<v8::Value> exception, std::false_type)
{
  return v8::ThrowException(exception);
}
      
inline std::conditional<v8_uses_isolate, void, v8::Handle<v8::Value> >::type
compatibility_throw(v8::Isolate* isolate, v8::Local<v8::Value> exception)
{
  return compatibility_throw_impl(isolate, exception, std::integral_constant<bool, v8_uses_isolate>());
}

inline void
compatibility_throw_impl(v8::Local<v8::Value> exception, std::true_type)
{
  static_cast<_v8_isolate_throw_exception<>*>(v8::Isolate::GetCurrent())->ThrowException(exception);
}

inline v8::Handle<v8::Value>
compatibility_throw_impl(v8::Local<v8::Value> exception, std::false_type)
{
  return v8::ThrowException(exception);
}
      
inline std::conditional<v8_uses_isolate, void, v8::Handle<v8::Value> >::type
compatibility_throw(v8::Local<v8::Value> exception)
{
  return compatibility_throw_impl(exception, std::integral_constant<bool, v8_uses_isolate>());
}
      
} } }

#endif
