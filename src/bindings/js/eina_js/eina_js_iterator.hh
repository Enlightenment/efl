#ifndef EINA_JS_ITERATOR_HH
#define EINA_JS_ITERATOR_HH

#include <type_traits>

#include <eina_js_value.hh>

namespace efl { namespace eina { namespace js {

/* Exports the \p iterator to be manipulated by the JS code. The iterator should
   remain alive as long as there is JS code referencing it. The JS code is able
   to destroy the iterator by itself if you register the appropriate function
   through `register_destroy_iterator`.

   The exported JS object models part the [iterator concept from ECMAScript
   6](https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/The_Iterator_protocol).

   The iterator will have the `next` function, but the returned object won't
   have a `done` attribute, because the eina_iterator itself doesn't expose this
   information.*/
template <typename T>
inline v8::Local<v8::Object> export_iterator(Eina_Iterator *i,
                                             v8::Isolate *isolate,
                                             const char *class_name)
{
    using no_tag_type = typename remove_tag<T>::type;
    typedef void (*deleter_t)(void*);

    auto obj_tpl = compatibility_new<v8::ObjectTemplate>(isolate);
    obj_tpl->SetInternalFieldCount(2);

    auto ret = obj_tpl->NewInstance();

    auto next = [](js::compatibility_callback_info_type info) -> compatibility_return_type
      {
        if (info.Length() != 0)
          return compatibility_return();

        void *ptr = compatibility_get_pointer_internal_field(info.This(), 0);
        auto it = static_cast<Eina_Iterator*>(ptr);
        void *value = nullptr;
        auto done = !::eina_iterator_next(it, &value);
        v8::Local<v8::Object> o = compatibility_new<v8::Object>(info.GetIsolate());
        o->Set(compatibility_new<v8::String>(info.GetIsolate(), "done"),
               compatibility_new<v8::Boolean>(info.GetIsolate(), done));
        if (!done)
          {
             std::string obj_class_name;
             if (info.Data()->IsString())
               {
                  v8::String::Utf8Value str(info.Data());
                  obj_class_name = *str;
               }
             o->Set(compatibility_new<v8::String>(info.GetIsolate(), "value"),
                    get_value_from_c(js::wrap_value<T>(get_c_container_data<no_tag_type>(value), js::value_tag<T>{}),
                                     info.GetIsolate(), obj_class_name.c_str()));
          }
        return compatibility_return(o, info);
      };

    ret->Set(compatibility_new<v8::String>(isolate, "next"),
             compatibility_new<v8::FunctionTemplate>(isolate, next, js::compatibility_new<v8::String>(isolate, class_name))->GetFunction());

    {
        deleter_t deleter = [](void *i) {
            ::eina_iterator_free(static_cast<Eina_Iterator*>(i));
        };
        compatibility_set_pointer_internal_field(ret, 0, i);
        // compatibility_set_pointer_internal_field
        //   (ret, 1, reinterpret_cast<void*>(deleter));
    }

    return ret;
}

/* Extracts and returns a copy from the internal iterator object from the JS
   object. */
inline
Eina_Iterator* import_iterator(v8::Handle<v8::Object> o)
{
    void* ptr = compatibility_get_pointer_internal_field(o, 0);
    return static_cast<Eina_Iterator*>(ptr);
}

void register_destroy_iterator(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

} } } // namespace efl::js

#endif /* EINA_JS_ITERATOR_HH */
