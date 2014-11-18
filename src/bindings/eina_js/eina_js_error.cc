#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <eina_js_error.hh>

namespace efl { namespace js {

EAPI
void convert_error_to_javascript_exception(v8::Isolate *isolate)
{
    using v8::Local;
    using v8::Object;
    using v8::String;

     Eina_Error err = eina_error_get();
     if (!err)
         return;

     Local<Object> je = Object::New(isolate);
     je->Set(String::NewFromUtf8(isolate, "code"),
             String::NewFromUtf8(isolate, "Eina_Error"));
     je->Set(String::NewFromUtf8(isolate, "value"),
             String::NewFromUtf8(isolate, eina_error_msg_get(err)));
     isolate->ThrowException(je);
}

} } // namespace efl { namespace js {
