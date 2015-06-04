#ifndef ECORE_JS_ANIMATOR_HH
#define ECORE_JS_ANIMATOR_HH

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

void register_pos_map_linear(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_pos_map_accelerate(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name);

void register_pos_map_decelerate(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name);

void register_pos_map_sinusoidal(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name);

void register_pos_map_accelerate_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name);

void register_pos_map_decelerate_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name);

void register_pos_map_sinusoidal_factor(v8::Isolate *isolate,
                                        v8::Handle<v8::Object> global,
                                        v8::Handle<v8::String> name);

void register_pos_map_divisor_interp(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name);

void register_pos_map_bounce(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_pos_map_spring(v8::Isolate *isolate,
                             v8::Handle<v8::Object> global,
                             v8::Handle<v8::String> name);

void register_pos_map_cubic_bezier(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name);

void register_animator_source_timer(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name);

void register_animator_source_custom(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name);

void register_animator_frametime_set(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name);

void register_animator_frametime_get(v8::Isolate *isolate,
                                     v8::Handle<v8::Object> global,
                                     v8::Handle<v8::String> name);

void register_animator_pos_map(v8::Isolate *isolate,
                               v8::Handle<v8::Object> global,
                               v8::Handle<v8::String> name);

void register_animator_pos_map_n(v8::Isolate *isolate,
                                 v8::Handle<v8::Object> global,
                                 v8::Handle<v8::String> name);

void register_animator_source_set(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name);

void register_animator_source_get(v8::Isolate *isolate,
                                  v8::Handle<v8::Object> global,
                                  v8::Handle<v8::String> name);

void
register_animator_custom_source_tick_begin_callback_set(v8::Isolate *isolate,
                                                        v8::Handle<v8::Object>
                                                        global,
                                                        v8::Handle<v8::String>
                                                        name);

void
register_animator_custom_source_tick_end_callback_set(v8::Isolate *isolate,
                                                      v8::Handle<v8::Object>
                                                      global,
                                                      v8::Handle<v8::String>
                                                      name);

void register_animator_custom_tick(v8::Isolate *isolate,
                                   v8::Handle<v8::Object> global,
                                   v8::Handle<v8::String> name);

void register_animator_add(v8::Isolate *isolate, v8::Handle<v8::Object> global,
                           v8::Handle<v8::String> name);

void register_animator_timeline_add(v8::Isolate *isolate,
                                    v8::Handle<v8::Object> global,
                                    v8::Handle<v8::String> name);

} } } // namespace efl { namespace ecore { namespace js {

#endif /* ECORE_JS_ANIMATOR_HH */
