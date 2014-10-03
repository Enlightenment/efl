#ifndef EINA_JS_VALUE_HH
#define EINA_JS_VALUE_HH

#include <v8.h>
#include <Eina.hh>
#include <type_traits>

namespace efl { namespace js {

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
    return v8::Integer::New(isolate, v);
}

template<class T>
typename std::enable_if<is_representable_as_v8_integer<T>::value
                        && std::is_unsigned<T>::value,
                        v8::Local<v8::Value>>::type
to_v8_number(const T &v, v8::Isolate *isolate)
{
    return v8::Integer::NewFromUnsigned(isolate, v);
}

template<class T>
typename std::enable_if<(std::is_integral<T>::value
                         && !is_representable_as_v8_integer<T>::value)
                        || std::is_floating_point<T>::value,
                        v8::Local<v8::Value>>::type
to_v8_number(const T &v, v8::Isolate *isolate)
{
    return v8::Number::New(isolate, v);
}

template<class T>
typename std::enable_if<std::is_same<T, ::efl::eina::stringshare>::value
                        || std::is_same<T, std::string>::value,
    v8::Local<v8::Value>>::type
to_v8_string(const T &v, v8::Isolate *isolate)
{
    return v8::String::NewFromUtf8(isolate, v.c_str(),
                                   v8::String::kNormalString, v.size());
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
value_cast(const v8::Local<v8::Value> &v)
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
inline
void register_make_value(v8::Isolate *isolate,
                         v8::Handle<v8::ObjectTemplate> global,
                         v8::Local<v8::String> name)
{
    //v8::Local<v8::FunctionTemplate>
    using v8::Local;
    using v8::Value;
    using v8::String;
    using v8::Object;
    using v8::FunctionTemplate;
    using v8::FunctionCallbackInfo;

    typedef ::efl::eina::value value_type;
    typedef value_type *ptr_type;

    auto ctor = [](const FunctionCallbackInfo<Value> &info) {
        auto set = [](const FunctionCallbackInfo<Value> &info) {
            if (info.Length() != 1)
                return;

            void *ptr = info.Holder()->GetAlignedPointerFromInternalField(0);
            try {
                *static_cast<ptr_type>(ptr) = value_cast<value_type>(info[0]);
            } catch(const std::bad_cast &e) {
            } catch(const ::efl::eina::system_error &e) {
            }
        };
        auto get = [](const FunctionCallbackInfo<Value> &info) {
            void *ptr = info.Holder()->GetAlignedPointerFromInternalField(0);
            auto &value = *static_cast<ptr_type>(ptr);
            info.GetReturnValue().Set(value_cast<Local<Value>>
                                      (value, info.GetIsolate()));
        };

        if (info.Length() != 1)
            return;

        auto obj_tpl = v8::ObjectTemplate::New(info.GetIsolate());
        obj_tpl->SetInternalFieldCount(1);

        auto ret = obj_tpl->NewInstance();
        info.GetReturnValue().Set(ret);

        ret->Set(String::NewFromUtf8(info.GetIsolate(), "set"),
                 FunctionTemplate::New(info.GetIsolate(), set)->GetFunction());
        ret->Set(String::NewFromUtf8(info.GetIsolate(), "get"),
                 FunctionTemplate::New(info.GetIsolate(), get)->GetFunction());

        try {
            std::unique_ptr<value_type>
                ptr(new value_type(value_cast<value_type>(info[0])));
            ret->SetAlignedPointerInInternalField(0, ptr.get());
            ptr.release();
        } catch(const std::bad_cast &e) {
        } catch(const ::efl::eina::system_error &e) {
        }
    };

    global->Set(name, FunctionTemplate::New(isolate, ctor));
}

inline
void register_destroy_value(v8::Isolate *isolate,
                            v8::Handle<v8::ObjectTemplate> global,
                            v8::Local<v8::String> name)
{
    //v8::Local<v8::FunctionTemplate>
    using v8::Handle;
    using v8::Local;
    using v8::Value;
    using v8::FunctionTemplate;
    using v8::FunctionCallbackInfo;

    typedef ::efl::eina::value value_type;
    typedef value_type *ptr_type;

    auto dtor = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 1)
            return;

        auto o = info[0]->ToObject();
        delete static_cast<ptr_type>(o->GetAlignedPointerFromInternalField(0));
        o->SetAlignedPointerInInternalField(0, nullptr);
        assert(o->GetAlignedPointerFromInternalField(0) == nullptr);
    };

    global->Set(name, FunctionTemplate::New(isolate, dtor));
}

} } // namespace efl::js

#endif /* EINA_JS_VALUE_HH */
