#ifndef ECORE_JS_TIMER_HH
#define ECORE_JS_TIMER_HH

#include <Eina.hh>

#include <Eina_Js.hh>

namespace efl { namespace ecore { namespace js {

using ::efl::eina::js::compatibility_new;
using ::efl::eina::js::compatibility_return_type;
using ::efl::eina::js::compatibility_callback_info_type;
using ::efl::eina::js::compatibility_return;
using ::efl::eina::js::compatibility_get_pointer_internal_field;
using ::efl::eina::js::compatibility_set_pointer_internal_field;
using ::efl::eina::js::compatibility_persistent;

void register_timer_precision_get(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name);

void register_timer_precision_set(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name);

void register_timer_dump(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name);

void register_timer_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name);

void register_timer_loop_add(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

} } } // namespace efl::js

#endif /* ECORE_JS_TIMER_HH */
