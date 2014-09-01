#ifndef EINA_JS_VALUE_HH
#define EINA_JS_VALUE_HH

#include <type_traits>

#include <eina_js_compatibility.hh>

namespace efl { namespace eina { namespace js {

namespace detail {

template<class T, class = void>
struct is_representable_as_v8_integer: std::false_type {};

template<class T>
struct is_representable_as_v8_integer
<T,
 typename std::enable_if<std::is_integral<T>::value
                         /* v8::Integer only stores 32-bit signed and unsigned
                            numbers. */
                         && (sizeof(T) <= sizeof(int32_t))>::type>
: std::true_type {};

template<class T>
typename std::enable_if<is_representable_as_v8_integer<T>::value
                        && std::is_signed<T>::value,
                        v8::Local<v8::Value>>::type
to_v8_number(const T &v, v8::Isolate *isolate)
{
  return compatibility_new<v8::Integer>(isolate, v);
}

template<class T>
typename std::enable_if<is_representable_as_v8_integer<T>::value
                        && std::is_unsigned<T>::value,
                        v8::Local<v8::Value>>::type
to_v8_number(const T &v, v8::Isolate *isolate)
{
  return compatibility_new<v8::Integer>(isolate, v);
}

template<class T>
typename std::enable_if<(std::is_integral<T>::value
                         && !is_representable_as_v8_integer<T>::value)
                        || std::is_floating_point<T>::value,
                        v8::Local<v8::Value>>::type
to_v8_number(const T &v, v8::Isolate *isolate)
{
  return compatibility_new<v8::Number>(isolate, v);
}

template<class T>
typename std::enable_if<std::is_same<T, ::efl::eina::stringshare>::value
                        || std::is_same<T, std::string>::value,
    v8::Local<v8::Value>>::type
to_v8_string(const T &v, v8::Isolate *isolate)
{
  return compatibility_new<v8::String>(isolate, v.c_str());
}

} // namespace detail

template<class T>
typename std::enable_if<std::is_same<T, v8::Local<v8::Value>>::value, T>::type
value_cast(const ::efl::eina::value &v, v8::Isolate *isolate)
{
    using detail::to_v8_number;
    using detail::to_v8_string;
    using ::efl::eina::get;

    const auto &t = v.type_info();
    if (t == EINA_VALUE_TYPE_UINT64) {
        return to_v8_number(get<uint64_t>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_UCHAR) {
        return to_v8_number(get<unsigned char>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_USHORT) {
        return to_v8_number(get<unsigned short>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_UINT) {
        return to_v8_number(get<unsigned int>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_ULONG) {
        return to_v8_number(get<unsigned long>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_CHAR) {
        return to_v8_number(get<char>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_SHORT) {
        return to_v8_number(get<short>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_INT) {
        return to_v8_number(get<int>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_LONG) {
        return to_v8_number(get<long>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_FLOAT) {
        return to_v8_number(get<float>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_DOUBLE) {
        return to_v8_number(get<double>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_STRINGSHARE) {
        return to_v8_string(get<::efl::eina::stringshare>(v), isolate);
    } else if (t == EINA_VALUE_TYPE_STRING) {
        return to_v8_string(get<std::string>(v), isolate);
    }

    throw std::bad_cast{};
}

template<class T>
typename std::enable_if<std::is_same<T, ::efl::eina::value>::value, T>::type
value_cast(const v8::Handle<v8::Value> &v)
{
    using ::efl::eina::value;

    if (v->IsBoolean()) {
        return value(int{v->BooleanValue()});
    } else if (v->IsInt32()) {
        return value(v->Int32Value());
    } else if (v->IsUint32()) {
        return value(v->Uint32Value());
    } else if (v->IsNumber()) {
        return value(v->NumberValue());
    } else if (v->IsString()) {
        v8::String::Utf8Value data(v);
        return value(std::string(*data, data.length()));
    }

    throw std::bad_cast{};
}

/*
  # JS binding

  - There is the `value()` constructor, which accepts a primitive value as input
    argument and might throw.
    - The returned object has a `get()` method, which can be used to get the
      wrapped value as a JavaScript value.
    - The returned object has a `set()` method, which can be used to change the
      wrapped value.
 */
void register_value(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                    v8::Handle<v8::String> name);

} } } // namespace efl::js

EAPI void eina_value_register(v8::Handle<v8::Object> global, v8::Isolate* isolate);

#endif /* EINA_JS_VALUE_HH */
