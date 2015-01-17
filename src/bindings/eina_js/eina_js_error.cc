#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <eina_js_error.hh>
#include <eina_js_compatibility.hh>

namespace efl { namespace eina {namespace js {

EAPI
js::compatibility_return_type convert_error_to_javascript_exception(v8::Isolate *isolate)
{
  std::cerr << __func__ << ':' << __LINE__ << std::endl;
     Eina_Error err = eina_error_get();
     if (!err)
       return compatibility_return();
  std::cerr << __func__ << ':' << __LINE__ << std::endl;

  v8::Local<v8::Object> je = eina::js::compatibility_new<v8::Object>(isolate);
  std::cerr << __func__ << ':' << __LINE__ << std::endl;
     je->Set(compatibility_new<v8::String>(isolate, "code"),
             compatibility_new<v8::String>(isolate, "Eina_Error"));
  std::cerr << __func__ << ':' << __LINE__ << std::endl;
     je->Set(compatibility_new<v8::String>(isolate, "value"),
             compatibility_new<v8::String>(isolate, eina_error_msg_get(err)));
  std::cerr << __func__ << ':' << __LINE__ << std::endl;
     return compatibility_throw(isolate, je);
}

} } } // namespace efl { namespace js {
