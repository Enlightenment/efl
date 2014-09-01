#ifndef EINA_JS_LOG_HH
#define EINA_JS_LOG_HH

#include <type_traits>

namespace efl { namespace eina { namespace js {

using ::efl::eina::js::compatibility_new;
using ::efl::eina::js::compatibility_return_type;
using ::efl::eina::js::compatibility_callback_info_type;
using ::efl::eina::js::compatibility_return;
using ::efl::eina::js::compatibility_get_pointer_internal_field;
using ::efl::eina::js::compatibility_set_pointer_internal_field;

void register_log_level_critical(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name);

void register_log_level_err(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name);

void register_log_level_warn(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_log_level_info(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_log_level_dbg(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name);

void register_log_domain_global(v8::Isolate *isolate,
                                v8::Handle<v8::Object> global,
                                v8::Handle<v8::String> name);

void register_log_print(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                        v8::Handle<v8::String> name);

void register_log_domain_register(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name);

void register_log_domain_unregister(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name);

void register_log_domain_registered_level_get(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name);

void register_log_domain_registered_level_set(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name);

void register_log_print_cb_set(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_log_level_set(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name);

void register_log_level_get(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                            v8::Handle<v8::String> name);

void register_log_level_check(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name);

void register_log_color_disable_set(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name);

void register_log_color_disable_get(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name);

void register_log_file_disable_set(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name);

void register_log_file_disable_get(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name);

void register_log_function_disable_set(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name);

void register_log_function_disable_get(v8::Isolate *isolate,
                                       v8::Handle<v8::Object> global,
                                       v8::Handle<v8::String> name);

void register_log_abort_on_critical_set(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name);

void register_log_abort_on_critical_get(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name);

void register_log_abort_on_critical_level_set(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name);

void register_log_abort_on_critical_level_get(v8::Isolate *isolate,
                                              v8::Handle<v8::Object> global,
                                              v8::Handle<v8::String> name);

void register_log_domain_level_set(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name);

void register_log_domain_level_get(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name);

void register_log_state_start(v8::Isolate *isolate,
                              v8::Handle<v8::Object> global,
                              v8::Handle<v8::String> name);

void register_log_state_stop(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_log_timing(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                         v8::Handle<v8::String> name);

} } } // namespace efl::js

EAPI void eina_log_register(v8::Handle<v8::Object> exports, v8::Isolate* isolate);

#endif /* EINA_JS_LOG_HH */
