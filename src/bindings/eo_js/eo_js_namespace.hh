#ifndef EFL_EO_JS_NAMESPACE_HH
#define EFL_EO_JS_NAMESPACE_HH

namespace efl { namespace eo { namespace js {

inline
v8::Local<v8::Object> get_namespace(std::vector<const char*> const& nss, v8::Isolate* isolate, v8::Local<v8::Object> to_export)
{
   for (auto ns : nss)
     {
        v8::Local<v8::Value> ns_obj = to_export->Get(::efl::eina::js::compatibility_new<v8::String>(isolate, ns));
        if (ns_obj->IsUndefined() || ns_obj->IsNull())
          {
            ns_obj = ::efl::eina::js::compatibility_new<v8::Object>(isolate);
            to_export->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, ns), ns_obj);
          }
        to_export = ns_obj->ToObject();
     }
   return to_export;
}

} } }

#endif
