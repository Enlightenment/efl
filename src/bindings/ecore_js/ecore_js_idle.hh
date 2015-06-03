#ifndef ECORE_JS_IDLE_HH
#define ECORE_JS_IDLE_HH

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

void register_idler_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name);

void register_idle_enterer_add(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_idle_enterer_before_add(v8::Isolate *isolate,
                                      v8::Handle<v8::Object> global,
                                      v8::Handle<v8::String> name);

void register_idle_exiter_add(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name);

} } } // namespace efl { namespace ecore { namespace js {

#endif /* ECORE_JS_IDLE_HH */
