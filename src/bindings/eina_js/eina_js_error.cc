#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <eina_js_error.hh>
#include <eina_js_compatibility.hh>

namespace efl { namespace eina {namespace js {

EAPI
void convert_error_to_javascript_exception(v8::Isolate *isolate)
{
    using v8::Local;
    using v8::Object;
    using v8::String;

     Eina_Error err = eina_error_get();
     if (!err)
         return;

     Local<Object> je = eina::js::compatibility_new<v8::Object>(isolate);
     je->Set(compatibility_new<v8::String>(isolate, "code"),
             compatibility_new<v8::String>(isolate, "Eina_Error"));
     je->Set(compatibility_new<v8::String>(isolate, "value"),
             compatibility_new<v8::String>(isolate, eina_error_msg_get(err)));
     compatibility_throw(isolate, je);
}

} } } // namespace efl { namespace js {
