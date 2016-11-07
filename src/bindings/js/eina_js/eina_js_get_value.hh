#ifndef EFL_EINA_JS_GET_VALUE_HH
#define EFL_EINA_JS_GET_VALUE_HH

#include <eina_js_compatibility.hh>

#include <type_traits>
#include <cstdlib>
#include <iostream>
#include <typeinfo>

namespace efl { namespace eina { namespace js {

template <typename T>
inline int get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char*
   , value_tag<T>
   , bool throw_js_exception = true
   , typename std::enable_if<(std::is_integral<T>::value && !std::is_same<T, Eina_Bool>::value)>::type* = 0)
{
  if(v->IsInt32())
    return v->Int32Value();
  else if(v->IsUint32())
    return v->Uint32Value();
  else
    {
      if (throw_js_exception)
        eina::js::compatibility_throw
          (isolate, v8::Exception::TypeError
           (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected Integral type")));

      throw std::logic_error("");
    }
  return 0;
}

inline char* get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char*
   , value_tag<char*>
   , bool throw_js_exception = true)
{
  if(v->IsNull())
    return nullptr;
  else if(v->IsString())
    {
      v8::String::Utf8Value str(v->ToString());
      char* string = strdup(*str); // TODO: leaks
      return string;
    }
  else
    {
      if (throw_js_exception)
        eina::js::compatibility_throw
          (isolate, v8::Exception::TypeError
           (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected String type")));

      throw std::logic_error("");
    }
  return 0;
}

inline const char* get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char* class_name
   , value_tag<const char*>
   , bool throw_js_exception = true)
{
  return get_value_from_javascript(v, isolate, class_name, value_tag<char*>(), throw_js_exception);
}

inline Eo* get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char*
   , value_tag<Eo*>
   , bool throw_js_exception = true)
{
  if(v->IsNull())
    return nullptr;
  else if(v->IsObject())
    {
      v8::Local<v8::Object> object = v->ToObject();
      if(object->InternalFieldCount() == 1)
        {
          v8::Local<v8::Value> r = object->GetInternalField(0);
          if(v8::External* external = v8::External::Cast(*r))
            {
              return static_cast<Eo*>(external->Value());
            }
        }
    }
  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected Eolian object type")));
  throw std::logic_error("");
  return nullptr;
}

inline Eo* get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char* class_name
   , value_tag<Eo* const>
   , bool throw_js_exception = true)
{
  return get_value_from_javascript(v, isolate, class_name, value_tag<Eo*>(), throw_js_exception);
}

// Futures
template <typename ClassGetter>
Eo* get_value_from_javascript
  (v8::Local<v8::Value>
   , v8::Isolate*
   , const char*
   , value_tag<complex_tag<Eo**, void*, ClassGetter>>
   , bool = true)
{
  throw std::logic_error("");
  return nullptr;
}
      
template <typename T>
inline T get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char*
   , value_tag<struct_ptr_tag<T>>
   , bool throw_js_exception = true)
{
  if(v->IsNull())
    return nullptr;
  else if(v->IsObject())
    {
      v8::Local<v8::Object> object = v->ToObject();
      if(object->InternalFieldCount() == 1)
        {
           return compatibility_get_pointer_internal_field<T>(object, 0);
        }
    }
  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected Eolian struct type")));
  throw std::logic_error("");
  return nullptr;
}

template <typename T>
inline T get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char* class_name
   , value_tag<struct_tag<T>>
   , bool throw_js_exception = true)
{
  T* ptr = get_value_from_javascript(v, isolate, class_name, value_tag<struct_ptr_tag<T*>>(), throw_js_exception);
  if (ptr)
    return *ptr;

  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Conversion of null pointer to by-value struct.")));
  throw std::logic_error("");
  return T{};
}

template <typename T>
inline T get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char*
   , value_tag<T>
   , bool throw_js_exception = true
   , typename std::enable_if<std::is_enum<T>::value>::type* = 0)
{
  if(v->IsInt32())
    return static_cast<T>(v->Int32Value());
  else if(v->IsUint32())
    return static_cast<T>(v->Uint32Value());
  else
    {
      if (throw_js_exception)
        eina::js::compatibility_throw
          (isolate, v8::Exception::TypeError
           (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected enumeration type")));

      throw std::logic_error("");
    }
  return T();
}

inline Eina_Bool get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char*
   , value_tag<Eina_Bool>
   , bool throw_js_exception = true)
{
  if(v->IsBoolean() || v->IsBooleanObject())
    {
      return v->BooleanValue() ? EINA_TRUE : EINA_FALSE;
    }
  else
    {
      if (throw_js_exception)
        eina::js::compatibility_throw
          (isolate, v8::Exception::TypeError
           (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected Boolean type")));

      throw std::logic_error("");
    }
  return 0;
}

template <typename T>
inline double get_value_from_javascript
  (v8::Local<v8::Value> v
   , v8::Isolate* isolate
   , const char*
   , value_tag<T>
   , bool throw_js_exception = true
   , typename std::enable_if<std::is_floating_point<T>::value>::type* = 0)
{
  if(v->IsNumber())
    {
      return v->NumberValue();
    }
  else
    {
      if (throw_js_exception)
        eina::js::compatibility_throw
          (isolate, v8::Exception::TypeError
           (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected floating point type")));
      throw std::logic_error("");
    }
  return 0.0;
}

template <typename T>
inline T get_value_from_javascript
  (v8::Local<v8::Value>, v8::Isolate* isolate, const char*, value_tag<T>
   , bool throw_js_exception = true
   , typename std::enable_if<
     !std::is_floating_point<T>::value &&
     !std::is_integral<T>::value &&
     !std::is_enum<T>::value &&
     !js::is_struct_tag<T>::value &&
     !js::is_struct_ptr_tag<T>::value &&
     !js::is_complex_tag<T>::value &&
     !std::is_same<T, Eina_Accessor*>::value &&
     !std::is_same<T, Eina_Array*>::value &&
     !std::is_same<T, Eina_Iterator*>::value &&
     !std::is_same<T, Eina_Hash*>::value &&
     !std::is_same<T, Eina_List*>::value &&
     !std::is_same<T, const Eina_Accessor*>::value &&
     !std::is_same<T, const Eina_Array*>::value &&
     !std::is_same<T, const Eina_Iterator*>::value &&
     !std::is_same<T, const Eina_Hash*>::value &&
     !std::is_same<T, const Eina_List*>::value
   >::type* = 0)
{
  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Not implemented yet")));
  throw std::logic_error("");
}

// TODO: Fix for const types
template <typename T, typename K>
inline Eina_Accessor* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char*,
  value_tag<complex_tag<Eina_Accessor *, T, K>>,
  bool throw_js_exception = true)
{
  if(v->IsNull())
    return nullptr;
  else if(v->IsObject())
    {
      using wrapped_type = typename container_wrapper<T>::type;
      v8::Local<v8::Object> object = v->ToObject();
      auto& acc = import_accessor<wrapped_type>(object);
      return acc.native_handle();
    }
  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected Eolian accessor type")));
  throw std::logic_error("");
}

template <typename...I>
inline const Eina_Accessor* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char* class_name,
  value_tag<complex_tag<const Eina_Accessor *, I...>>,
  bool throw_js_exception = true)
{
  return get_value_from_javascript(v, isolate, class_name, value_tag<complex_tag<Eina_Accessor *, I...>>{}, throw_js_exception);
}

template <typename...I>
inline Eina_Array* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char*,
  value_tag<complex_tag<Eina_Array *, I...>>,
  bool throw_js_exception = true)
{
  if(v->IsNull())
    return nullptr;
  else if(v->IsObject())
    {
      v8::Local<v8::Object> object = v->ToObject();
      if(object->InternalFieldCount() == 1)
        {
           eina_container_base* cbase = compatibility_get_pointer_internal_field<eina_container_base*>(object, 0);
           return static_cast<Eina_Array*>(cbase->get_container_native_handle());
        }
    }
  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected Eolian list type")));
  throw std::logic_error("");
  return nullptr;
}

template <typename...I>
inline const Eina_Array* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char* class_name,
  value_tag<complex_tag<const Eina_Array *, I...>>,
  bool throw_js_exception = true)
{
  return get_value_from_javascript(v, isolate, class_name, value_tag<complex_tag<Eina_Array *, I...>>{}, throw_js_exception);
}

template <typename T, typename K>
inline Eina_Iterator* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char*,
  value_tag<complex_tag<Eina_Iterator *, T, K>> /*tag*/,
  bool throw_js_exception = true)
{
  if(v->IsNull())
    return nullptr;
  else if(v->IsObject())
    {
       return import_iterator(v->ToObject());
    }
  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected Eolian accessor type")));
  throw std::logic_error("");
}

template <typename...I>
inline const Eina_Iterator* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char* class_name,
  value_tag<complex_tag<const Eina_Iterator *, I...>>,
  bool throw_js_exception = true)
{
  return get_value_from_javascript(v, isolate, class_name, value_tag<complex_tag<Eina_Iterator *, I...>>{}, throw_js_exception);
}

template <typename T, typename...U>
inline Eina_Hash* get_value_from_javascript(
  v8::Local<v8::Value>,
  v8::Isolate* isolate,
  const char*,
  value_tag<complex_tag<Eina_Hash *, T, U...>> tag,
  bool throw_js_exception = true)
{
  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Not implemented yet")));
  throw std::logic_error("");
}

template <typename T, typename...U>
inline const Eina_Hash* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char* class_name,
  value_tag<complex_tag<const Eina_Hash *, T, U...>>,
  bool throw_js_exception = true)
{
  return get_value_from_javascript(v, isolate, class_name, value_tag<complex_tag<Eina_Hash *, T, U...>>{}, throw_js_exception);
}

template <typename...I>
inline Eina_List* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char*,
  value_tag<complex_tag<Eina_List *, I...>> /*tag*/,
  bool throw_js_exception = true)
{
  if(v->IsNull())
    return nullptr;
  else if(v->IsObject())
    {
      v8::Local<v8::Object> object = v->ToObject();
      if(object->InternalFieldCount() == 1)
        {
           eina_container_base* cbase = compatibility_get_pointer_internal_field<eina_container_base*>(object, 0);
           return static_cast<Eina_List*>(cbase->get_container_native_handle());
        }
    }
  if (throw_js_exception)
    eina::js::compatibility_throw
      (isolate, v8::Exception::TypeError
       (eina::js::compatibility_new<v8::String>(isolate, "Type expected is different. Expected Eolian list type")));
  throw std::logic_error("");
  return nullptr;
}

template <typename...I>
inline const Eina_List* get_value_from_javascript(
  v8::Local<v8::Value> v,
  v8::Isolate* isolate,
  const char* class_name,
  value_tag<complex_tag<const Eina_List *, I...>>,
  bool throw_js_exception = true)
{
  return get_value_from_javascript(v, isolate, class_name, value_tag<complex_tag<Eina_List *, I...>>{}, throw_js_exception);
}

inline const void* get_value_from_javascript
  (v8::Local<v8::Value>,
   v8::Isolate*,
   const char*,
   value_tag<const void *>)
{
  return nullptr;
}


typedef void (*Evas_Smart_Cb)(void*, _Eo_Opaque*, void*);

inline Evas_Smart_Cb get_value_from_javascript (
  v8::Local<v8::Value>,
  v8::Isolate*,
  const char*,
  value_tag<Evas_Smart_Cb>)
{
  return nullptr;
}

} } }

#endif
