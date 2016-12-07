#ifndef EFL_EINA_JS_COMPATIBILITY_HH
#define EFL_EINA_JS_COMPATIBILITY_HH

#include <type_traits>
#include <utility>
#include <map>
#include <string>
#include <cstdlib>


#ifdef HAVE_NODE_V8_H
#include <node/v8.h>
#elif defined(HAVE_NODEJS_DEPS_V8_V8_H)
#include <nodejs/deps/v8/v8.h>
#elif defined(HAVE_NODEJS_DEPS_V8_INCLUDE_V8_H)
#include <nodejs/deps/v8/include/v8.h>
#elif defined(HAVE_NODEJS_SRC_V8_H)
#include <nodejs/src/v8.h>
#elif defined(HAVE_V8_H)
#include <v8.h>
#else
#error We must have at least one v8 header to include
#endif

namespace v8 {

template <typename T>
struct FunctionCallbackInfo;
template <typename T>
struct PropertyCallbackInfo;
template <typename T>
#ifdef HAVE_V8_GLOBAL
struct Global;
#else
struct UniquePersistent;
#endif

class AccessorInfo;

class Arguments;

}

namespace efl { namespace eina { namespace js {

template <typename T>
struct value_tag
{
  typedef T type;
};

template <typename T, typename... U>
struct complex_tag
{
  T value;
  typedef std::tuple<U...> inner_types;
};

template <typename T>
struct make_tag_traits
{
  typedef typename std::remove_reference<T>::type a1;
  typedef typename std::conditional
  <std::is_pointer<T>::value
    , typename std::remove_cv<T>::type
    , T>::type type;
};
      
template <typename T, typename... U>
using make_complex_tag = complex_tag<typename make_tag_traits<T>::type
                                     , typename make_tag_traits<U>::type...>;

template <typename T>
struct struct_tag
{
  T value;
};

template <typename T>
using make_struct_tag = struct_tag<typename make_tag_traits<T>::type>;
      
template <typename T>
struct struct_ptr_tag
{
  T value;
};

template <typename T>
using make_struct_ptr_tag = struct_ptr_tag<typename make_tag_traits<T>::type>;
      
template <typename T, typename... Ts>
struct remove_tag
{
  typedef T type;
};

template <typename T, typename... U>
struct remove_tag<complex_tag<T, U...>>
{
  typedef typename eina::js::remove_tag<T, U...>::type type;
};

template <typename T>
struct remove_tag<struct_tag<T>>
{
  typedef typename eina::js::remove_tag<T>::type type;
};

template <typename T>
struct remove_tag<struct_ptr_tag<T>>
{
  typedef typename eina::js::remove_tag<T>::type type;
};

template <typename T>
struct is_handable_by_value
{
  static constexpr bool value = !std::is_class<T>::value || std::is_same<T, ::efl::eo::concrete>::value;
};

template <typename... T>
struct container_wrapper
{
  typedef typename eina::js::remove_tag<T...>::type _notag_type;
  typedef typename std::conditional<
    std::is_convertible<_notag_type, Eo const* const>::value
    , typename std::conditional<
        std::is_const<typename std::remove_pointer<_notag_type>::type>::value
        , ::efl::eo::concrete const
        , ::efl::eo::concrete
    >::type
    , _notag_type
  >::type type;
};

template <typename T>
inline T get_c_container_data(void* ptr, typename std::enable_if<
  std::is_pointer<T>::value
>::type* = 0)
{
  return static_cast<T>(ptr);
}

template <typename T>
inline T get_c_container_data(void* ptr, typename std::enable_if<
  !std::is_pointer<T>::value
>::type* = 0)
{
  return *static_cast<T*>(ptr);
}

template <typename T>
T container_wrap(T&& v)
{
  return std::forward<T>(v);
}

inline ::efl::eo::concrete container_wrap(Eo* v)
{
  if(v)
    efl_ref(v);
  return ::efl::eo::concrete{v};
}

inline ::efl::eo::concrete container_wrap(Eo const* v)
{
  if (v)
    efl_ref(v);
  return ::efl::eo::concrete{const_cast<Eo*>(v)};
}

template <typename T>
T& container_unwrap(T& v)
{
  return v;
}

inline Eo* container_unwrap(::efl::eo::concrete& v)
{
  return v._eo_ptr();
}

inline Eo* container_unwrap(::efl::eo::concrete const& v)
{
  return v._eo_ptr();
}

template <typename T>
struct is_complex_tag : std::false_type {};

template <typename... T>
struct is_complex_tag<complex_tag<T...>> : std::true_type {};

template <typename T>
struct is_struct_tag : std::false_type {};

template <typename T>
struct is_struct_tag<struct_tag<T>> : std::true_type {};

template <typename T>
struct is_struct_ptr_tag : std::false_type {};

template <typename T>
struct is_struct_ptr_tag<struct_ptr_tag<T>> : std::true_type {};

template <typename T>
struct is_type_tag
{
  static constexpr bool value =
    is_complex_tag<T>::value
    || is_struct_tag<T>::value
    || is_struct_ptr_tag<T>::value;
};


// Class name getters
struct cls_name_getter_base {};
struct cls_name_getter_generated_base : cls_name_getter_base {};
/// Name getter for types that are not classes
struct nonclass_cls_name_getter : cls_name_getter_base
{
  static char const* class_name() { return ""; }
};

// JS container base
enum container_type
{
  list_container_type
  , array_container_type
  , container_type_size
};

struct eina_container_base
{
  virtual ~eina_container_base() {}

  virtual std::size_t size() const = 0;
  virtual eina_container_base* concat(eina_container_base const& rhs) const = 0;
  virtual eina_container_base* slice(std::int64_t i, std::int64_t j) const = 0;
  virtual int index_of(v8::Isolate* isolate, v8::Local<v8::Value> v) const = 0;
  virtual int last_index_of(v8::Isolate* isolate, v8::Local<v8::Value> v) const = 0;
  virtual v8::Local<v8::Value> get(v8::Isolate*, std::size_t) const = 0;
  virtual v8::Local<v8::Value> set(v8::Isolate* isolate, std::size_t index, v8::Local<v8::Value> v) = 0;
  virtual int push(v8::Isolate* isolate, v8::Local<v8::Value> v) = 0;
  virtual v8::Local<v8::Value> pop(v8::Isolate* isolate) = 0;
  virtual v8::Local<v8::String> to_string(v8::Isolate*) const = 0;
  virtual v8::Local<v8::String> join(v8::Isolate*, v8::Local<v8::Value> separator) const = 0;
  virtual container_type get_container_type() const = 0;
  virtual void* get_container_native_handle() = 0;
  virtual void const* get_container_native_handle() const = 0;
};

// Containers forward declarations

// Array
template <typename T, typename K = nonclass_cls_name_getter, typename W = typename container_wrapper<T>::type>
struct eina_array;
template <typename T, typename K = nonclass_cls_name_getter, typename W = typename container_wrapper<T>::type>
struct range_eina_array;

EAPI v8::Handle<v8::Function> get_array_instance_template();

// List
template <typename T, typename K = nonclass_cls_name_getter, typename W = typename container_wrapper<T>::type>
struct eina_list;
template <typename T, typename K = nonclass_cls_name_getter, typename W = typename container_wrapper<T>::type>
struct range_eina_list;

EAPI v8::Handle<v8::Function> get_list_instance_template();

// Accessor
template <typename T, typename W>
v8::Local<v8::Object> export_accessor(::efl::eina::accessor<W>&, v8::Isolate*, const char*);

template <typename T>
::efl::eina::accessor<T>& import_accessor(v8::Handle<v8::Object>);

// Iterator
template <typename T>
inline v8::Local<v8::Object> export_iterator(Eina_Iterator*, v8::Isolate*, const char*);

inline Eina_Iterator* import_iterator(v8::Handle<v8::Object>);

// Wrap value functions
template <typename R, typename T>
typename std::remove_cv<typename std::remove_reference<R>::type>::type
wrap_value(T v, value_tag<R>
           , typename std::enable_if<!is_type_tag<typename std::remove_cv<R>::type>::value>::type* = 0)
{
   return v;
}

template <typename R, typename T>
R wrap_value(T const& v, value_tag<eina::js::struct_tag<T>>)
{
   return R {v};
}

template <typename R, typename T>
R wrap_value(T v, value_tag<eina::js::struct_ptr_tag<T>>)
{
   return R {v};
}

template <typename R, typename T, typename... U>
R wrap_value(T v, value_tag<eina::js::complex_tag<T, U...>>)
{
   return R {v};
}

template <typename R, typename T, typename... U>
R wrap_value(T const& v, value_tag<eina::js::complex_tag<T*, U...>>)
{
   return R {const_cast<T*>(&v)};
}

template <typename T = v8::External>
struct _libv8_isolate_test
{
   using new_signature = v8::Local<T>(*)(v8::Isolate*, void*);
   static const bool value = std::is_same<decltype(static_cast<new_signature>(&T::New)), new_signature>::value;
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

// NOTE: ifndef needed because a bug in doxygen makes it fail with this class
#ifndef EFL_DOXYGEN
template <typename T = v8::ObjectTemplate, typename Enable = void>
struct _libv8_property_callback_info_test
  : std::true_type {};
#endif

typedef v8::Handle<v8::Value>(*_libv8_getter_callback)(v8::Local<v8::String>, v8::AccessorInfo const&);
typedef void(*_libv8_setter_callback)(v8::Local<v8::String>, v8::Local<v8::Value>, v8::AccessorInfo const&);

#ifndef EFL_DOXYGEN
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
#endif

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
using compatibility_accessor_getter_callback_info_type
 = std::conditional<v8_uses_property_callback_info
                    , v8::PropertyCallbackInfo<v8::Value> const&, v8::AccessorInfo const&>
  ::type;

using compatibility_accessor_setter_return_type
  = void;
using compatibility_accessor_setter_callback_info_type
 = std::conditional<v8_uses_property_callback_info
                    , v8::PropertyCallbackInfo<void> const&, v8::AccessorInfo const&>
  ::type;

using compatibility_indexed_property_getset_return_type
 = std::conditional<v8_uses_property_callback_info, void, v8::Handle<v8::Value> >::type;
using compatibility_indexed_property_callback_info_type
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
auto compatibility_new(std::nullptr_t, Args...args) ->
  decltype(js::compatibility_new_impl<>(nullptr, std::integral_constant<bool, v8_uses_isolate>()
                                        , compatibility_type_tag<T>()
                                        , args...))
{
  return js::compatibility_new_impl<>(nullptr, std::integral_constant<bool, v8_uses_isolate>()
                                      , compatibility_type_tag<T>()
                                      , args...);
}

#ifdef HAVE_V8_CREATE_PARAMS
namespace detail {
class array_buffer_allocator : public v8::ArrayBuffer::Allocator
{
  public:
  virtual void* Allocate(std::size_t length)
  {
    void* data = AllocateUninitialized(length);
    return data ? std::memset(data, 0, length) : data;
  }
  virtual void* AllocateUninitialized(std::size_t length) { return std::malloc(length); }
  virtual void Free(void* data, std::size_t) { std::free(data); }
};
}

inline v8::Isolate* compatibility_isolate_new()
{
  static detail::array_buffer_allocator allocator;
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator = &allocator;
  return v8::Isolate::New(create_params);
}

#else

inline v8::Isolate* compatibility_isolate_new()
{
  return v8::Isolate::New();
}

#endif

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
compatibility_return_impl(T object, compatibility_accessor_getter_callback_info_type, std::false_type)
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
compatibility_return(T object, compatibility_accessor_getter_callback_info_type args)
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

template <typename T, typename U>
v8::Local<T> compatibility_cast(v8::Local<U> v);
template <typename T, typename U>
v8::Local<T> compatibility_cast(U* v);

template <typename Tag>
struct hack_private_member {
  /* export it ... */
  typedef typename Tag::type type;
  static type ptr;
};

template <typename Tag>
typename hack_private_member<Tag>::type hack_private_member<Tag>::ptr;

template <typename Tag, typename Tag::type p>
struct rob_private_member : hack_private_member<Tag> {
  /* fill it ... */
  struct filler {
    filler() { hack_private_member<Tag>::ptr = p; }
  };
  static filler filler_obj;
};

template<typename Tag, typename Tag::type p>
typename rob_private_member<Tag, p>::filler rob_private_member<Tag, p>::filler_obj;

template <typename T>
struct persistent_base_new { typedef T*(*type)(v8::Isolate*, T*); };

template class rob_private_member<persistent_base_new<v8::Value>, &v8::PersistentBase<v8::Value>::New>;
      
template <typename T>
v8::Local<T> make_persistent(v8::Isolate* isolate, v8::Handle<T> v)
{
  v8::Value* p = hack_private_member<persistent_base_new<v8::Value>>::ptr
    (isolate, *compatibility_cast<v8::Value>(v));
  return compatibility_cast<T>(compatibility_cast<v8::Value>(p));
}

template <typename T, typename F>
v8::Local<T> make_weak(v8::Isolate* isolate, v8::Handle<T> v, F&& f)
{
  v8::Value* p = hack_private_member<persistent_base_new<v8::Value>>::ptr
    (isolate, *compatibility_cast<v8::Value>(v));
  v8::PersistentBase<v8::Value>* persistent = static_cast<v8::PersistentBase<v8::Value>*>
    (static_cast<void*>(&p));

  auto callback = [](const v8::WeakCallbackInfo<typename std::remove_reference<F>::type>& data) -> void
    {
      typename std::remove_reference<F>::type* f = data.GetParameter();
      (*f)();
      delete f;
    };

  persistent->SetWeak(new typename std::remove_reference<F>::type(std::forward<F>(f)),
                      callback, v8::WeakCallbackType::kParameter);
  return compatibility_cast<T>(compatibility_cast<v8::Value>(p));
}

template <typename T>
struct global_ref
{
  global_ref() {}
  global_ref(v8::Local<T> v)
    : _value(make_persistent(nullptr, v))
  {
  }
  global_ref(v8::Isolate* isolate, v8::Local<T> v)
    : _value(make_persistent(isolate, v))
  {
  }

  void dispose() const
  {
    v8::PersistentBase<T>* p = static_cast<v8::PersistentBase<T>*>(static_cast<void*>(&_value));
    p->Reset();
  }

  v8::Handle<T> handle() const { return _value; }
private:
  mutable v8::Local<T> _value;
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
  compatibility_handle_scope_impl()
    : HandleScope(v8::Isolate::GetCurrent())
  {}
  compatibility_handle_scope_impl(v8::Isolate* isolate)
    : HandleScope((assert(isolate != nullptr), isolate))
  {}
};

template <typename T>
struct compatibility_handle_scope_impl<T, false> : v8::HandleScope
{
  compatibility_handle_scope_impl()
  {}
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
#ifdef HAVE_V8_CREATE_PARAMS
  constexpr const char* argv[] = {""};
  static_cast<_v8_initialize_icu<>*>(nullptr)->InitializeICU();
  v8::V8::InitializeExternalStartupData(argv[0]);
  v8::V8::Initialize();
#else
  v8::V8::Initialize();
  static_cast<_v8_initialize_icu<>*>(nullptr)->InitializeICU();
#endif
}

template <typename T, typename U>
v8::Local<T> compatibility_cast(v8::Local<U> v)
{
  static_assert(sizeof(v8::Local<T>) == sizeof(v8::Local<U>), "");
  v8::Local<T> l;
  std::memcpy(&l, &v, sizeof(v8::Local<T>));
  return l;
}

template <typename T, typename U>
v8::Local<T> compatibility_cast(U* v)
{
  static_assert(sizeof(v8::Local<T>) == sizeof(U*), "");
  v8::Local<T> l;
  std::memcpy(&l, &v, sizeof(v8::Local<T>));
  return l;
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

inline v8::Local<v8::Value>
new_v8_external_instance(v8::Handle<v8::Function>& ctor, void const* v, v8::Isolate* isolate)
{
  // TODO: ensure v8::External ownership ??? (memory leak in case NewInstance throws)
  v8::Handle<v8::Value> a[] = {efl::eina::js::compatibility_new<v8::External>(isolate, const_cast<void*>(v))};
  return ctor->NewInstance(1, a);
}

inline
compatibility_return_type cast_function(compatibility_callback_info_type args);

inline v8::Local<v8::Value>
new_v8_external_instance(v8::Handle<v8::Function>& ctor, Eo const* v, v8::Isolate* isolate)
{
  // TODO: ensure v8::External ownership ??? (memory leak in case NewInstance throws)
  v8::Handle<v8::Value> a[] = {efl::eina::js::compatibility_new<v8::External>(isolate, const_cast<Eo*>(v))};
  auto obj = ctor->NewInstance(1, a);
  obj->Set(compatibility_new<v8::String>(isolate, "cast"),
           compatibility_new<v8::FunctionTemplate>(isolate, &cast_function)->GetFunction());
  return obj;
}

inline v8::Local<v8::Object> compatibility_global()
{
  return _v8_get_current_context<>::GetCurrent()->Global();
}

EAPI extern std::map<std::string, v8::Local<v8::Function>> constructors_map_;

inline v8::Handle<v8::Function> get_class_constructor(std::string const& class_name)
{
  auto it = constructors_map_.find(class_name);
  if (it == constructors_map_.end())
    throw std::runtime_error("Class not found: " + class_name);
  return it->second;
}

inline void register_class_constructor(std::string const& class_name,
                                       v8::Handle<v8::Function> constructor_ptr)
{
  // TODO: check if already exist?
  constructors_map_[class_name] = constructor_ptr;
}

template<class T = v8::StackTrace>
typename std::enable_if<!v8_uses_isolate, v8::Local<T>>::type
compatibility_current_stack_trace(v8::Isolate*, int frame_limit,
				  v8::StackTrace::StackTraceOptions options)
{
  return T::CurrentStackTrace(frame_limit, options);
}

template<class T = v8::StackTrace>
typename std::enable_if<v8_uses_isolate, v8::Local<T>>::type
compatibility_current_stack_trace(v8::Isolate *isolate, int frame_limit,
				  v8::StackTrace::StackTraceOptions options)
{
  return T::CurrentStackTrace(isolate, frame_limit, options);
}

inline
compatibility_return_type cast_function(compatibility_callback_info_type args)
{
  auto isolate = args.GetIsolate();
  compatibility_handle_scope scope(isolate);
  v8::Local<v8::Value> type;
  try
    {
      if(args.Length() == 1 && (type = args[0])->IsString())
        {
          v8::Local<v8::Object> self = args.This();
          v8::Local<v8::Value> external = self->GetInternalField(0);
          Eo* eo = static_cast<Eo*>(v8::External::Cast(*external)->Value());

          v8::String::Utf8Value str(type->ToString());
          char* class_name = *str;

          auto ctor = ::efl::eina::js::get_class_constructor(class_name);
          auto obj = new_v8_external_instance(ctor, ::efl_ref(eo), isolate);
          efl::eina::js::make_weak(isolate, obj, [eo]{ ::efl_unref(eo); });
          return compatibility_return(obj, args);
        }
      else
        {
          throw std::runtime_error("Type expected is different. Expected String type");
        }
    }
  catch (std::runtime_error const& error)
    {
      eina::js::compatibility_throw
        (isolate, v8::Exception::TypeError
         (eina::js::compatibility_new<v8::String>(isolate, error.what())));
      return compatibility_return();
    }
}

} } }

#endif
