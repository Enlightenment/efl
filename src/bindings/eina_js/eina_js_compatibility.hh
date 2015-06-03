#ifndef EFL_EINA_JS_COMPATIBILITY_HH
#define EFL_EINA_JS_COMPATIBILITY_HH

#include <type_traits>

#include EINA_STRINGIZE(V8_INCLUDE_HEADER)

namespace v8 {

template <typename T>
struct FunctionCallbackInfo;
template <typename T>
struct PropertyCallbackInfo;
template <typename T>
struct UniquePersistent;

class AccessorInfo;

class Arguments;
  
}

namespace efl { namespace eina { namespace js {

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

template <typename T = v8::ObjectTemplate, typename Enable = void>
struct _libv8_property_callback_info_test
  : std::true_type {};

typedef v8::Handle<v8::Value>(*_libv8_getter_callback)(v8::Local<v8::String>, v8::AccessorInfo const&);
typedef void(*_libv8_setter_callback)(v8::Local<v8::String>, v8::Local<v8::Value>, v8::AccessorInfo const&);

template <typename T>
struct _libv8_property_callback_info_test
<T, typename std::enable_if
 <std::is_same<decltype( & T::SetAccessor)
   , void (T::*)
     (v8::Handle<v8::String>
      , _libv8_getter_callback
      , _libv8_setter_callback
      , v8::Handle<v8::Value>
      , v8::AccessControl
      , v8::PropertyAttribute
      , v8::Handle<v8::AccessorSignature>
      )>::value>::type>
  : std::false_type
{
};

static constexpr bool const v8_uses_isolate = _libv8_isolate_test<>::value;
static constexpr bool const v8_uses_callback_info = _libv8_callback_info_test<>::value;
static constexpr bool const v8_uses_property_callback_info = _libv8_property_callback_info_test<>::value;

using compatibility_return_type = std::conditional<v8_uses_callback_info, void, v8::Handle<v8::Value> >::type;
using compatibility_callback_info_type
 = std::conditional<v8_uses_callback_info, v8::FunctionCallbackInfo<v8::Value> const&, v8::Arguments const&>
  ::type;
using compatibility_callback_info_pointer
 = std::conditional<v8_uses_callback_info, v8::FunctionCallbackInfo<v8::Value> const*, v8::Arguments const*>
  ::type;

typedef compatibility_return_type(*compatibility_function_callback)(compatibility_callback_info_type);
      
using compatibility_accessor_getter_return_type
  = std::conditional<v8_uses_property_callback_info, void, v8::Handle<v8::Value> >::type;
using compatibility_accessor_callback_info_type
 = std::conditional<v8_uses_property_callback_info
                    , v8::PropertyCallbackInfo<v8::Value> const&, v8::AccessorInfo const&>
  ::type;
      
static_assert(v8_uses_property_callback_info == v8_uses_callback_info
              && v8_uses_callback_info == v8_uses_isolate, "");

template <typename T>
struct compatibility_type_tag {};

template <bool = v8_uses_isolate>
struct compatibility_string;

template <>
struct compatibility_string<true> : v8::String
{
  template <typename... Args>
  static v8::Local<v8::String> New(Args...args)
  {
    return NewFromUtf8(v8::Isolate::GetCurrent(), args...);
  }
};

template <>
struct compatibility_string<false> : v8::String
{
};

template <typename...Args>
auto compatibility_new_impl(v8::Isolate*, std::true_type, compatibility_type_tag<v8::String>
                            , Args...args) ->
  decltype(compatibility_string<>::New(args...)) 
{
  return compatibility_string<>::New(args...);
}

template <typename...Args>
auto compatibility_new_impl(v8::Isolate*, std::false_type, compatibility_type_tag<v8::String>
                            , Args...args) ->
  decltype(compatibility_string<>::New(args...)) 
{
  return compatibility_string<>::New(args...);
}

template <typename...Args>
auto compatibility_new_impl(std::nullptr_t, std::true_type, compatibility_type_tag<v8::String>
                            , Args...args) ->
  decltype(compatibility_string<>::New(args...))
{
  return compatibility_string<>::New(args...);
}

template <typename...Args>
auto compatibility_new_impl(std::nullptr_t, std::false_type, compatibility_type_tag<v8::String>
                            , Args...args) ->
  decltype(compatibility_string<>::New(args...)) 
{
  return compatibility_string<>::New(args...);
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
  decltype(js::compatibility_new_impl<>
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
  decltype(js::compatibility_new_impl<>(nullptr, std::integral_constant<bool, v8_uses_isolate>()
                                        , compatibility_type_tag<T>()
                                        , args...))
{
  return js::compatibility_new_impl<>(nullptr, std::integral_constant<bool, v8_uses_isolate>()
                                      , compatibility_type_tag<T>()
                                      , args...);
}

template <typename T, typename U>
inline void compatibility_return_impl(T object, U const& info, std::true_type)
{
  info.GetReturnValue().Set(object);
}

template <typename T>
inline v8::Handle<v8::Value>
compatibility_return_impl(T object, compatibility_callback_info_type, std::false_type)
{
  return object;
}

template <typename T>
inline v8::Handle<v8::Value>
compatibility_return_impl(T object, compatibility_accessor_callback_info_type, std::false_type)
{
  return object;
}
      
template <typename T>
compatibility_return_type
compatibility_return(T object, compatibility_callback_info_type args)
{
  return compatibility_return_impl(object, args, std::integral_constant<bool, v8_uses_callback_info>());
}

template <typename T>
compatibility_return_type
compatibility_return(T object, compatibility_accessor_callback_info_type args)
{
  return compatibility_return_impl(object, args, std::integral_constant<bool, v8_uses_property_callback_info>());
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

struct _v8_isolate_throw_exception : v8::Isolate
{
  v8::Handle<v8::Value> ThrowException_impl(v8::Handle<v8::Value> v)
  {
    using namespace v8;
    return ThrowException(v);
  }
};

inline void
compatibility_throw_impl(v8::Isolate* isolate, v8::Local<v8::Value> exception, std::true_type)
{
  static_cast<_v8_isolate_throw_exception*>(isolate)->ThrowException_impl(exception);
}

inline v8::Handle<v8::Value>
compatibility_throw_impl(v8::Isolate* isolate, v8::Local<v8::Value> exception, std::false_type)
{
  return static_cast<_v8_isolate_throw_exception*>(isolate)->ThrowException_impl(exception);
}

inline std::conditional<v8_uses_isolate, void, v8::Handle<v8::Value> >::type
compatibility_throw(v8::Isolate* isolate, v8::Local<v8::Value> exception)
{
  return compatibility_throw_impl(isolate, exception, std::integral_constant<bool, v8_uses_isolate>());
}

inline void
compatibility_throw_impl(v8::Local<v8::Value> exception, std::true_type)
{
  static_cast<_v8_isolate_throw_exception*>(v8::Isolate::GetCurrent())->ThrowException_impl(exception);
}

inline v8::Handle<v8::Value>
compatibility_throw_impl(v8::Local<v8::Value> exception, std::false_type)
{
  return static_cast<_v8_isolate_throw_exception*>(v8::Isolate::GetCurrent())->ThrowException_impl(exception);
}
      
inline std::conditional<v8_uses_isolate, void, v8::Handle<v8::Value> >::type
compatibility_throw(v8::Local<v8::Value> exception)
{
  return compatibility_throw_impl(exception, std::integral_constant<bool, v8_uses_isolate>());
}

template <typename T, bool = v8_uses_isolate>
struct compatibility_persistent;

template <typename T>
struct compatibility_persistent<T, true> : v8::UniquePersistent<T>
{
  typedef v8::UniquePersistent<T> _base;

  compatibility_persistent(compatibility_persistent&& other)
    : _base(other.Pass())
  {
  }
  compatibility_persistent& operator=(compatibility_persistent&& other)
  {
    this->_base::operator=(other.Pass());
    return *this;
  }
  
  compatibility_persistent() {}
  compatibility_persistent(v8::Isolate* isolate, v8::Handle<T> v)
    : _base(isolate, v)
    , isolate(isolate)
  {
  }
  
  T* operator->() const
  {
    return *handle();
  }

  v8::Handle<T> handle() const { return v8::Local<T>::New(isolate, *this); }

  v8::Isolate* GetIsolate() { return isolate; }

private:
  v8::Isolate *isolate;
};

template <typename T>
struct compatibility_persistent<T, false> : v8::Persistent<T>
{
  typedef v8::Persistent<T> _base;

  compatibility_persistent() {}
  compatibility_persistent(v8::Isolate *isolate, v8::Handle<T> v)
    : _base(v)
    , isolate(isolate)
  {
  }

  v8::Handle<T>& handle() { return *this; }
  v8::Handle<T> const& handle() const { return *this; }

  v8::Isolate* GetIsolate() { return isolate; }

private:
  v8::Isolate *isolate;
};
      
template <typename T = std::integral_constant<bool, v8_uses_isolate> >
struct _v8_object_internal_field;

template <>
struct _v8_object_internal_field<std::true_type> : v8::Object
{
};

inline void* GetPointerFromInternalField(int) { return nullptr; }
inline void SetPointerInInternalField(int, void*) {}
      
template <>
struct _v8_object_internal_field<std::false_type> : v8::Object
{
  void* GetAlignedPointerFromInternalField(int index)
  {
    return GetPointerFromInternalField(index);
  }
  void SetAlignedPointerInInternalField(int index, void* p)
  {
    SetPointerInInternalField(index, p);
  }
};
      
template <typename T = void*>
inline T compatibility_get_pointer_internal_field(v8::Handle<v8::Object> object, std::size_t index)
{
  return reinterpret_cast<T>
    (static_cast<_v8_object_internal_field<>*>(*object)->GetAlignedPointerFromInternalField(index));
}

template <typename T>
inline void compatibility_set_pointer_internal_field(v8::Handle<v8::Object> object, std::size_t index
                                                     , T* pointer)
{
  static_cast<_v8_object_internal_field<>*>(*object)->SetAlignedPointerInInternalField(index, pointer);
}
      
template <typename T = void, bool = v8_uses_isolate>
struct compatibility_handle_scope_impl;

template <typename T>
struct compatibility_handle_scope_impl<T, true> : v8::HandleScope
{
  compatibility_handle_scope_impl(v8::Isolate* isolate)
    : HandleScope(isolate)
  {}
};

template <typename T>
struct compatibility_handle_scope_impl<T, false> : v8::HandleScope
{
  compatibility_handle_scope_impl(v8::Isolate*)
  {}
};

using compatibility_handle_scope = compatibility_handle_scope_impl<>;

template <bool = v8_uses_isolate>
struct _v8_initialize_icu;

template <>
struct _v8_initialize_icu<true> : v8::V8
{
};

template <>
struct _v8_initialize_icu<false> : v8::V8
{
  static bool InitializeICU(const char* = NULL)
  {
    return true;
  }
};
      
inline void compatibility_initialize()
{
  v8::V8::Initialize();
  static_cast<_v8_initialize_icu<>*>(nullptr)->InitializeICU();
}

template <typename T, typename U>
v8::Local<T> compatibility_cast(v8::Local<U> v)
{
  return *static_cast<v8::Local<T>*>(static_cast<void*>(&v));
}

template <typename T, typename U>
v8::Local<T> compatibility_cast(U* v)
{
  return *static_cast<v8::Local<T>*>(static_cast<void*>(v));
}

template <typename T = v8::Isolate, bool = v8_uses_isolate>
struct _v8_get_current_context;

template <typename T>
struct _v8_get_current_context<T, false> : v8::Context
{
};

template <typename T>
struct _v8_get_current_context<T, true> : T
{
  static v8::Local<v8::Context> GetCurrent()
  {
    return T::GetCurrent()->GetCurrentContext();
  }
};
      
inline v8::Local<v8::Object> compatibility_global()
{
  return _v8_get_current_context<>::GetCurrent()->Global();
}
      
} } }

#endif
