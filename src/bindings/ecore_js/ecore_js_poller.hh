#ifndef ECORE_JS_POLLER_HH
#define ECORE_JS_POLLER_HH

#include <Eina.hh>
#include EINA_STRINGIZE(V8_INCLUDE_HEADER)

#include <eina_js_compatibility.hh>

namespace efl { namespace ecore { namespace js {

using ::efl::eina::js::compatibility_new;
using ::efl::eina::js::compatibility_return_type;
using ::efl::eina::js::compatibility_callback_info_type;
using ::efl::eina::js::compatibility_return;
using ::efl::eina::js::compatibility_get_pointer_internal_field;
using ::efl::eina::js::compatibility_set_pointer_internal_field;
using ::efl::eina::js::compatibility_persistent;

void register_poller_core(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                          v8::Handle<v8::String> name);

void register_poller_poll_interval_set(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name);

void register_poller_poll_interval_get(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name);

void register_poller_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name);

} } } // namespace efl { namespace ecore { namespace js {

#endif /* ECORE_JS_POLLER_HH */
