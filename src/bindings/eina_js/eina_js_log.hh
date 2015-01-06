#ifndef EINA_JS_LOG_HH
#define EINA_JS_LOG_HH

#include V8_INCLUDE_HEADER
#include <Eina.hh>
#include <type_traits>

namespace efl { namespace js {

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

} } // namespace efl::js

#endif /* EINA_JS_LOG_HH */
