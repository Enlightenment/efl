#ifndef EINA_JS_ITERATOR_HH
#define EINA_JS_ITERATOR_HH

#include <v8.h>
#include <Eina.hh>
#include <type_traits>

#include <eina_js_value.hh>

namespace efl { namespace js {

/* Exports the \p iterator to be manipulated by the JS code. The iterator should
   remain alive as long as there is JS code referencing it. The JS code is able
   to destroy the iterator by itself if you register the appropriate function
   through `register_destroy_iterator`.

   The exported JS object models part the [iterator concept from ECMAScript
   6](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/The_Iterator_protocol).

   The iterator will have the `next` function, but the returned object won't
   have a `done` attribute, because the eina_iterator itself doesn't expose this
   information.*/
template<class T>
v8::Local<v8::Object> export_iterator(::efl::eina::iterator<T> *i,
                                      v8::Isolate *isolate)
{
    using v8::Local;
    using v8::Value;
    using v8::Object;
    using v8::String;
    using v8::FunctionCallbackInfo;
    using v8::FunctionTemplate;
    using v8::ObjectTemplate;

    typedef ::efl::eina::iterator<T> value_type;
    typedef value_type *ptr_type;
    typedef void (*deleter_t)(void*);

    auto obj_tpl = ObjectTemplate::New(isolate);
    obj_tpl->SetInternalFieldCount(2);

    auto ret = obj_tpl->NewInstance();

    auto next = [](const FunctionCallbackInfo<Value> &info) {
        if (info.Length() != 0)
            return;

        void *ptr = info.This()->GetAlignedPointerFromInternalField(0);
        auto &value = *static_cast<ptr_type>(ptr);
        Local<Object> o = v8::Object::New(info.GetIsolate());
        o->Set(String::NewFromUtf8(info.GetIsolate(), "value"),
               value_cast<Local<Value>>(*value, info.GetIsolate()));
        info.GetReturnValue().Set(o);
        ++value;
    };

    ret->Set(String::NewFromUtf8(isolate, "next"),
             FunctionTemplate::New(isolate, next)->GetFunction());

    {
        deleter_t deleter = [](void *i) {
            delete static_cast<ptr_type>(i);
        };
        ret->SetAlignedPointerInInternalField(0, i);
        ret->SetAlignedPointerInInternalField(1,
                                              reinterpret_cast<void*>(deleter));
    }

    return ret;
}

/* Extracts and returns a copy from the internal iterator object from the JS
   object. */
template<class T>
::efl::eina::iterator<T> *import_iterator(v8::Handle<v8::Object> o)
{
    typedef ::efl::eina::iterator<T> value_type;
    typedef value_type *ptr_type;

    return reinterpret_cast<ptr_type>(o->GetAlignedPointerFromInternalField(0));
}

void register_destroy_iterator(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

} } // namespace efl::js

#endif /* EINA_JS_ITERATOR_HH */
