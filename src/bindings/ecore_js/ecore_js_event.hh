#ifndef ECORE_JS_EVENT_HH
#define ECORE_JS_EVENT_HH

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

void register_event_none(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name);

void register_event_signal_user(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name);

void register_event_signal_hup(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_event_signal_exit(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name);

void register_event_signal_power(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name);

void register_event_signal_realtime(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name);

void register_event_memory_state(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name);

void register_event_power_state(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name);

void register_event_locale_changed(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name);

void register_event_hostname_changed(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name);

void register_event_system_timedate_changed(v8::Isolate *isolate,
                                            v8::Handle<v8::Object> global,
                                            v8::Handle<v8::String> name);

void register_event_type_new(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_event_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name);

void register_event_handler_add(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name);

void register_event_filter_add(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_event_current_type_get(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name);

void register_memory_state_normal(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name);

void register_memory_state_low(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_power_state_mains(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name);

void register_power_state_battery(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name);

void register_power_state_low(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name);

void register_memory_state_get(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_power_state_get(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name);

void register_event_signal_user_handler_add(v8::Isolate *isolate,
                                            v8::Handle<v8::Object> global,
                                            v8::Handle<v8::String> name);

void register_event_signal_exit_handler_add(v8::Isolate *isolate,
                                            v8::Handle<v8::Object> global,
                                            v8::Handle<v8::String> name);

void register_event_signal_realtime_handler_add(v8::Isolate *isolate,
                                                v8::Handle<v8::Object> global,
                                                v8::Handle<v8::String> name);

} } } // namespace efl { namespace ecore { namespace js {

#endif /* ECORE_JS_EVENT_HH */
