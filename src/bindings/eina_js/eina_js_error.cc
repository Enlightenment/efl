#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina_Js.hh>

namespace efl { namespace eina {namespace js {

EAPI
js::compatibility_return_type convert_error_to_javascript_exception(v8::Isolate *isolate)
{
     Eina_Error err = eina_error_get();
     if (!err)
       return compatibility_return();

  v8::Local<v8::Object> je = eina::js::compatibility_new<v8::Object>(isolate);
     je->Set(compatibility_new<v8::String>(isolate, "code"),
             compatibility_new<v8::String>(isolate, "Eina_Error"));
     je->Set(compatibility_new<v8::String>(isolate, "value"),
             compatibility_new<v8::String>(isolate, eina_error_msg_get(err)));
     return compatibility_throw(isolate, je);
}

} } } // namespace efl { namespace js {
