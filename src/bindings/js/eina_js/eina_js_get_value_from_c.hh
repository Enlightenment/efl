#ifndef EFL_EINA_JS_GET_VALUE_FROM_C_HH
#define EFL_EINA_JS_GET_VALUE_FROM_C_HH

#include <eina_js_compatibility.hh>

#include <type_traits>
#include <cstdlib>
#include <typeinfo>
#include <memory>

namespace efl { namespace eina { namespace js {

template <typename T> struct print_tag {};

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate, const char*
                 , typename std::enable_if<std::is_integral<typename std::remove_reference<T>::type>::value && !std::is_same<T, Eina_Bool>::value>::type* = 0)
{
  return eina::js::compatibility_new<v8::Integer>(isolate, v);
}

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate, const char*
                 , typename std::enable_if<std::is_enum<typename std::remove_reference<T>::type>::value>::type* = 0)
{
  return eina::js::compatibility_new<v8::Integer>(isolate, v);
}

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate, const char*
                 , typename std::enable_if<std::is_same<typename std::remove_reference<T>::type, Eina_Bool>::value>::type* = 0)
{
  return eina::js::compatibility_new<v8::Boolean>(isolate, v);
}

template <typename T>
inline v8::Local<T>
get_value_from_c(v8::Local<T> v, v8::Isolate*, const char*)
{
  return v;
}

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T v, v8::Isolate* isolate, const char*
                 , typename std::enable_if<std::is_floating_point<typename std::remove_reference<T>::type>::value>::type* = 0)
{
  return eina::js::compatibility_new<v8::Number>(isolate, v);
}

inline v8::Local<v8::Value>
get_value_from_c(const char* v, v8::Isolate* isolate, const char*)
{
  if (!v)
    return v8::Null(isolate);

  return eina::js::compatibility_new<v8::String>(isolate, v);
}

inline v8::Local<v8::Value>
get_value_from_c(char* v, v8::Isolate* isolate, const char* class_name)
{
  return js::get_value_from_c(const_cast<const char*>(v), isolate, class_name);
}

inline v8::Local<v8::Value>
get_value_from_c(void*, v8::Isolate*, const char*)
{
  // TODO: create Extern?
  std::cerr << "aborting because we don't know the type void*" << std::endl;
  std::abort();
}

inline v8::Local<v8::Value>
get_value_from_c(const void*, v8::Isolate*, const char*)
{
  // TODO: create Extern?
  std::cerr << "aborting because we don't know the type void*" << std::endl;
  std::abort();
}

// For function pointer types
template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T, v8::Isolate*, const char*
                 , typename std::enable_if
                 <std::is_pointer<typename std::remove_reference<T>::type>::value
                 && std::is_function<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value
                 >::type* = 0)
{
  // TODO: create Extern?
  std::cerr << "aborting because we don't know the type " << typeid(print_tag<T>).name() << std::endl;
  std::abort();
}

// For all non-pointer types that are not handled
template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(T, v8::Isolate*, const char*
                 , typename std::enable_if
                 <!std::is_pointer<typename std::remove_reference<T>::type>::value
                 && !std::is_integral<typename std::remove_reference<T>::type>::value
                 && !std::is_floating_point<typename std::remove_reference<T>::type>::value
                 && !std::is_enum<typename std::remove_reference<T>::type>::value
                 && !std::is_same<typename std::remove_reference<T>::type, Eina_Bool>::value
                 && !js::is_struct_tag<typename std::remove_reference<T>::type>::value
                 && !js::is_struct_ptr_tag<typename std::remove_reference<T>::type>::value
                 && !js::is_complex_tag<typename std::remove_reference<T>::type>::value
                 >::type* = 0)
{
  std::cerr << "aborting because we don't know the type " << typeid(print_tag<T>).name() << std::endl;
  std::abort();
}

// For all non-handled pointer types (which are not function pointers):
// - we try to dereference it in the SFINAE
// - if it matches we call get_value_from_c for the dereferenced type
// - if it fails (probably because it is opaque) the void* or const void*
//   overload will take place (implicit conversion)
template <typename T>
inline auto
get_value_from_c(T object, v8::Isolate* isolate, const char* class_name
                 , typename std::enable_if<
                   (std::is_pointer<typename std::remove_reference<T>::type>::value
                    && !std::is_function<typename std::remove_pointer<typename std::remove_reference<T>::type>::type>::value
                   )
                   && !(std::is_same<typename std::remove_reference<T>::type, char*>::value ||
                        std::is_same<typename std::remove_reference<T>::type, const char*>::value ||
                        std::is_same<typename std::remove_reference<T>::type, void*>::value ||
                        std::is_same<typename std::remove_reference<T>::type, const void*>::value ||
                        std::is_same<typename std::remove_reference<T>::type, Eo*>::value ||
                        std::is_same<typename std::remove_reference<T>::type, const Eo*>::value
                   )>::type* = 0) -> decltype(get_value_from_c(*object, isolate, class_name))
{
  std::cerr << "dereferencing " << typeid(print_tag<T>).name() << std::endl;
  return get_value_from_c(*object, isolate, class_name);
}

inline v8::Local<v8::Value>
get_value_from_c(Eo* v, v8::Isolate* isolate, const char* class_name)
{
  try
    {
       auto ctor = ::efl::eina::js::get_class_constructor(class_name);
       auto obj = new_v8_external_instance(ctor, ::efl_ref(v), isolate);
       efl::eina::js::make_weak(isolate, obj, [v]{ ::efl_unref(v); });
       return obj;
    }
  catch (std::runtime_error const& error)
    {
       eina::js::compatibility_throw
            (isolate, v8::Exception::TypeError
             (eina::js::compatibility_new<v8::String>(isolate, error.what())));
      return v8::Null(isolate);
    }
}

inline v8::Local<v8::Value>
get_value_from_c(const Eo* v, v8::Isolate* isolate, const char* class_name)
{
  // TODO: implement const objects?
  try
    {
       auto ctor = ::efl::eina::js::get_class_constructor(class_name);
       auto obj = new_v8_external_instance(ctor, ::efl_ref(v), isolate);
       efl::eina::js::make_weak(isolate, obj, [v]{ ::efl_unref(v); });
       return obj;
    }
  catch (std::runtime_error const& error)
    {
       eina::js::compatibility_throw
            (isolate, v8::Exception::TypeError
             (eina::js::compatibility_new<v8::String>(isolate, error.what())));
      return v8::Null(isolate);
    }
}

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(struct_ptr_tag<T*> v, v8::Isolate* isolate, const char* class_name)
{
  try
    {
       using nonconst_type = typename std::remove_const<T>::type;
       bool own = false; // TODO: handle ownership
       auto ptr = const_cast<nonconst_type*>(v.value);
       auto ctor = ::efl::eina::js::get_class_constructor(class_name);
       auto obj = new_v8_external_instance(ctor, ptr, isolate);
       if (own)
          efl::eina::js::make_weak(isolate, obj, [ptr]{ free(ptr); });
       return obj;
    }
  catch (std::runtime_error const& error)
    {
       eina::js::compatibility_throw
            (isolate, v8::Exception::TypeError
             (eina::js::compatibility_new<v8::String>(isolate, error.what())));
      return v8::Null(isolate);
    }

}

template <typename T>
inline v8::Local<v8::Value>
get_value_from_c(struct_tag<T> v, v8::Isolate* isolate, const char* class_name)
{
  T* s = static_cast<T*>(malloc(sizeof(T)));
  *s = v.value;
  return get_value_from_c(struct_ptr_tag<T*>{s}, isolate, class_name);
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<Eina_Accessor *, T, K> v, v8::Isolate* isolate, const char*)
{
  using wrapped_type = typename container_wrapper<T>::type;
  bool own = false; // TODO: handle ownership
  auto a = new ::efl::eina::accessor<wrapped_type>{v.value};
  auto obj = export_accessor<T>(*a , isolate, K::class_name());
  efl::eina::js::make_weak(isolate, obj, [a, own]
    {
       if (!own)
         a->release_native_handle();
       delete a;
    });
  return obj;
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<Eo**, T, K>, v8::Isolate*, const char*)
{
  throw std::logic_error("");
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<const Eina_Accessor *, T, K> v, v8::Isolate* isolate, const char* class_name)
{
  // TODO implement const accessor?
  return get_value_from_c(efl::eina::js::complex_tag<Eina_Accessor*, T, K>{const_cast<Eina_Accessor*>(v.value)}, isolate, class_name);
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<Eina_Array *, T, K> v, v8::Isolate* isolate, const char*)
{
  bool own = false; // TODO: handle ownership
  auto o = new ::efl::eina::js::eina_array<T, K>(v.value);
  auto ctor = get_array_instance_template();
  auto obj = new_v8_external_instance(ctor, o, isolate);
  efl::eina::js::make_weak(isolate, obj, [o, own]
    {
       if (!own)
         o->release_native_handle();
       delete o;
    });
  return obj;
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<const Eina_Array *, T, K> v, v8::Isolate* isolate, const char*)
{
  bool own = false; // TODO: handle ownership
  auto o = new ::efl::eina::js::range_eina_array<T, K>(const_cast<Eina_Array*>(v.value));
  auto ctor = get_array_instance_template();
  auto obj = new_v8_external_instance(ctor, o, isolate);
  efl::eina::js::make_weak(isolate, obj, [o, own]
    {
       if (!own)
         o->release_native_handle();
       delete o;
    });
  return obj;
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<Eina_Iterator *, T, K> v, v8::Isolate* isolate, const char*)
{
  bool own = false; // TODO: handle ownership
  auto ptr = v.value;
  auto obj = export_iterator<T>(ptr , isolate, K::class_name());
  if (own && ptr)
    efl::eina::js::make_weak(isolate, obj, [ptr]{ ::eina_iterator_free(ptr); });
  return obj;
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<const Eina_Iterator *, T, K> v, v8::Isolate* isolate, const char* class_name)
{
  // TODO: implement const iterators?
  return get_value_from_c(efl::eina::js::complex_tag<Eina_Iterator *, T, K>{const_cast<Eina_Iterator*>(v.value)}, isolate, class_name);
}

template <typename T, typename KT, typename U, typename KU>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<Eina_Hash *, T, KT, U, KU>, v8::Isolate*, const char*)
{
  std::cerr << "get_value_from_c for Eina_Hash not implemented. Aborting..." << std::endl;
  std::abort();
}

template <typename T, typename KT, typename U, typename KU>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<const Eina_Hash *, T, KT, U, KU>, v8::Isolate*, const char*)
{
  std::cerr << "get_value_from_c for Eina_Hash not implemented. Aborting..." << std::endl;
  std::abort();
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<Eina_List *, T, K> v, v8::Isolate* isolate, const char*)
{
  bool own = false; // TODO: handle ownership
  auto o = new ::efl::eina::js::eina_list<T, K>(v.value);
  auto ctor = get_list_instance_template();
  auto obj = new_v8_external_instance(ctor, o, isolate);
  efl::eina::js::make_weak(isolate, obj, [o, own]
    {
       if (!own)
         o->release_native_handle();
       delete o;
    });
  return obj;
}

template <typename T, typename K>
inline v8::Local<v8::Value>
get_value_from_c(efl::eina::js::complex_tag<const Eina_List *, T, K> v, v8::Isolate* isolate, const char*)
{
  bool own = false; // TODO: handle ownership
  auto o = new ::efl::eina::js::range_eina_list<T, K>(const_cast<Eina_List*>(v.value));
  auto ctor = get_list_instance_template();
  auto obj = new_v8_external_instance(ctor, o, isolate);
  efl::eina::js::make_weak(isolate, obj, [o, own]
    {
       if (!own)
         o->release_native_handle();
       delete o;
    });
  return obj;
}


} } }

#endif
