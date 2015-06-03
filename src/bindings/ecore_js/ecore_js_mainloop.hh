#ifndef ECORE_JS_MAINLOOP_HH
#define ECORE_JS_MAINLOOP_HH

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

void register_callback_cancel(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name);

void register_callback_renew(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_callback_pass_on(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_callback_done(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name);

void register_mainloop_iterate(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_mainloop_iterate_may_block(v8::Isolate *isolate,
                                         v8::Handle<v8::Object> global,
                                         v8::Handle<v8::String> name);

void register_mainloop_begin(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_mainloop_quit(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name);

void register_mainloop_animator_ticked_get(v8::Isolate *isolate,
                                           v8::Handle<v8::Object> global,
                                           v8::Handle<v8::String> name);

void register_mainloop_nested_get(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name);

void register_mainloop_thread_safe_call_async(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name);

void register_mainloop_thread_safe_call_sync(v8::Isolate *isolate,
                                             v8::Handle<v8::Object> global,
                                             v8::Handle<v8::String> name);

} } } // namespace efl::js

#endif /* ECORE_JS_MAINLOOP_HH */
