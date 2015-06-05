#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eina.hh>
#include EINA_STRINGIZE(V8_INCLUDE_HEADER)

#include <cassert>
#include <cstdlib>
#include <fstream>

#include <eina_js_compatibility.hh>
#include <Eina.h>
#include <Eo.hh>

#include <ecore_js_init.hh>
#include <ecore_js_mainloop.hh>
#include <ecore_js_timer.hh>
#include <ecore_js_event.hh>
#include <ecore_js_job.hh>
#include <ecore_js_idle.hh>
#include <ecore_js_animator.hh>
#include <ecore_js_poller.hh>
#include <ecore_js_throttle.hh>

const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate,
                   v8::Handle<v8::String> source,
                   v8::Handle<v8::Value> name)
{
  efl::eina::js::compatibility_handle_scope handle_scope(isolate);
  v8::TryCatch try_catch;
  v8::ScriptOrigin origin(name);
  v8::Handle<v8::Script> script = v8::Script::Compile(source, &origin);
  if (script.IsEmpty()) {
    std::cerr << "Compilation failed" << std::endl;
    std::abort();
    // Print errors that happened during compilation.
    // if (report_exceptions)
    //   ReportException(isolate, &try_catch);
    return false;
  }
  else
  {
    std::cerr << "Compilation succesful" << std::endl;
    v8::Handle<v8::Value> result = script->Run();
    if (result.IsEmpty()) {
      std::cout << "Failed with exception thrown" << std::endl;
      //assert(try_catch.HasCaught());
      //std::abort();
      // Print errors that happened during execution.
      // if (report_exceptions)
      //   ReportException(isolate, &try_catch);
      if(try_catch.HasCaught())
        std::cerr << "Exception " << ToCString(v8::String::Utf8Value(try_catch.Message()->Get()))
                  << std::endl;
      std::abort();
      return false;
    } else {
      assert(!try_catch.HasCaught());
      // if (print_result && !result->IsUndefined()) {
      //   // If all went well and the result wasn't undefined then print
      //   // the returned value.
      //   v8::String::Utf8Value str(result);
      //   const char* cstr = ToCString(str);
      //   printf("%s\n", cstr);
      // }
      return true;
    }
  }
}

efl::eina::js::compatibility_return_type Print(efl::eina::js::compatibility_callback_info_type args)
{
  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    efl::eina::js::compatibility_handle_scope handle_scope(args.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
  return efl::eina::js::compatibility_return();
}

void test_setup(v8::Handle<v8::Object> exports)
{
  using namespace efl::ecore::js;
  using efl::eina::js::compatibility_new;
  using v8::String;

  v8::Isolate *isolate = v8::Isolate::GetCurrent();

  // init
  efl::ecore::js::register_init(isolate, exports,
                                efl::eina::js::compatibility_new<v8::String>
                                (isolate, "ecore_init"));
  efl::ecore::js::register_shutdown(isolate, exports,
                                    efl::eina::js::compatibility_new<v8::String>
                                    (isolate, "ecore_shutdown"));

  // mainloop
  register_callback_cancel(isolate, exports,
                           compatibility_new<String>
                           (isolate, "ECORE_CALLBACK_CANCEL"));
  register_callback_renew(isolate, exports,
                          compatibility_new<String>
                          (isolate, "ECORE_CALLBACK_RENEW"));
  register_callback_pass_on(isolate, exports,
                            compatibility_new<String>
                            (isolate, "ECORE_CALLBACK_PASS_ON"));
  register_callback_done(isolate, exports,
                         compatibility_new<String>
                         (isolate, "ECORE_CALLBACK_DONE"));
  register_mainloop_iterate(isolate, exports,
                            compatibility_new<String>
                            (isolate, "ecore_mainloop_iterate"));
  register_mainloop_iterate_may_block(isolate, exports,
                                      compatibility_new<String>
                                      (isolate,
                                       "ecore_mainloop_iterate_may_block"));
  register_mainloop_begin(isolate, exports,
                          compatibility_new<String>
                          (isolate, "ecore_mainloop_begin"));
  register_mainloop_quit(isolate, exports,
                         compatibility_new<String>
                         (isolate, "ecore_mainloop_quit"));
  register_mainloop_animator_ticked_get(isolate, exports,
                                        compatibility_new<String>
                                        (isolate,
                                         "ecore_mainlop_animator_ticked_get"));
  register_mainloop_nested_get(isolate, exports,
                               compatibility_new<String>
                               (isolate, "ecore_mainloop_nested_get"));
  register_mainloop_thread_safe_call_async(isolate, exports,
                                           compatibility_new<String>
                                           (isolate,
                                            "ecore_mainloop_thread_safe_call"
                                            "_async"));
  register_mainloop_thread_safe_call_sync(isolate, exports,
                                          compatibility_new<String>
                                          (isolate,
                                           "ecore_mainloop_thread_safe_call"
                                           "_sync"));

  // timer
  register_timer_precision_get(isolate, exports,
                               compatibility_new<String>(isolate,
                                                         "ecore_timer_precision"
                                                         "_get"));
  register_timer_precision_set(isolate, exports,
                               compatibility_new<String>(isolate,
                                                         "ecore_timer_precision"
                                                         "_set"));
  register_timer_dump(isolate, exports,
                      compatibility_new<String>(isolate, "ecore_timer_dump"));
  register_timer_add(isolate, exports,
                     compatibility_new<String>(isolate, "ecore_timer_add"));
  register_timer_loop_add(isolate, exports,
                          compatibility_new<String>(isolate,
                                                    "ecore_timer_loop_add"));

  // event
  register_event_none(isolate, exports,
                      compatibility_new<String>(isolate,
                                                "ECORE_EVENT_NONE"));
  register_event_signal_user(isolate, exports,
                             compatibility_new<String>
                             (isolate, "ECORE_EVENT_SIGNAL_USER"));
  register_event_signal_hup(isolate, exports,
                            compatibility_new<String>
                            (isolate, "ECORE_EVENT_SIGNAL_HUP"));
  register_event_signal_exit(isolate, exports,
                             compatibility_new<String>
                             (isolate, "ECORE_EVENT_SIGNAL_EXIT"));
  register_event_signal_power(isolate, exports,
                              compatibility_new<String>
                              (isolate, "ECORE_EVENT_SIGNAL_POWER"));
  register_event_signal_realtime(isolate, exports,
                                 compatibility_new<String>
                                 (isolate, "ECORE_EVENT_SIGNAL_REALTIME"));
  register_event_memory_state(isolate, exports,
                              compatibility_new<String>
                              (isolate, "ECORE_EVENT_MEMORY_STATE"));
  register_event_power_state(isolate, exports,
                             compatibility_new<String>
                             (isolate, "ECORE_EVENT_POWER_STATE"));
  register_event_locale_changed(isolate, exports,
                                compatibility_new<String>
                                (isolate, "ECORE_EVENT_LOCALE_CHANGED"));
  register_event_hostname_changed(isolate, exports,
                                  compatibility_new<String>
                                  (isolate, "ECORE_EVENT_HOSTNAME_CHANGED"));
  register_event_system_timedate_changed(isolate, exports,
                                         compatibility_new<String>
                                         (isolate,
                                          "ECORE_EVENT_SYSTEM_TIMEDATE"
                                          "_CHANGED"));
  register_event_type_new(isolate, exports,
                          compatibility_new<String>(isolate,
                                                    "ecore_event_type_new"));
  register_event_add(isolate, exports,
                     compatibility_new<String>(isolate, "ecore_event_add"));
  register_event_handler_add(isolate, exports,
                             compatibility_new<String>
                             (isolate, "ecore_event_handler_add"));
  register_event_filter_add(isolate, exports,
                            compatibility_new<String>
                            (isolate, "ecore_event_filter_add"));
  register_event_current_type_get(isolate, exports,
                                  compatibility_new<String>
                                  (isolate, "ecore_event_current_type_get"));
  register_memory_state_normal(isolate, exports,
                               compatibility_new<String>
                               (isolate, "ECORE_MEMORY_STATE_NORMAL"));
  register_memory_state_low(isolate, exports,
                            compatibility_new<String>
                            (isolate, "ECORE_MEMORY_STATE_LOW"));
  register_power_state_mains(isolate, exports,
                             compatibility_new<String>
                             (isolate, "ECORE_POWER_STATE_MAINS"));
  register_power_state_battery(isolate, exports,
                               compatibility_new<String>
                               (isolate, "ECORE_POWER_STATE_BATTERY"));
  register_power_state_low(isolate, exports,
                           compatibility_new<String>(isolate,
                                                     "ECORE_POWER_STATE_LOW"));
  register_event_signal_user_handler_add(isolate, exports,
                                         compatibility_new<String>
                                         (isolate,
                                          "ecore_event_signal_user_handler"
                                          "_add"));
  register_event_signal_exit_handler_add(isolate, exports,
                                         compatibility_new<String>
                                         (isolate,
                                          "ecore_event_signal_exit_handler"
                                          "_add"));
  register_event_signal_realtime_handler_add(isolate, exports,
                                             compatibility_new<String>
                                             (isolate,
                                              "ecore_event_signal_realtime"
                                              "_handler_add"));

  // job
  register_job_add(isolate, exports,
                   compatibility_new<String>(isolate, "ecore_job_add"));

  // idle
  register_idler_add(isolate, exports,
                     compatibility_new<String>(isolate, "ecore_idler_add"));
  register_idle_enterer_add(isolate, exports,
                            compatibility_new<String>(isolate,
                                                      "ecore_idle_enterer"
                                                      "_add"));
  register_idle_enterer_before_add(isolate, exports,
                                   compatibility_new<String>(isolate,
                                                             "ecore_idle"
                                                             "_enterer_before"
                                                             "_add"));
  register_idle_exiter_add(isolate, exports,
                           compatibility_new<String>(isolate,
                                                     "ecore_idle_exiter_add"));

  // animator
  register_pos_map_linear(isolate, exports,
                          compatibility_new<String>(isolate,
                                                    "ECORE_POS_MAP_LINEAR"));
  register_pos_map_accelerate(isolate, exports,
                              compatibility_new<String>(isolate,
                                                        "ECORE_POS_MAP"
                                                        "_ACCELERATE"));
  register_pos_map_decelerate(isolate, exports,
                              compatibility_new<String>(isolate,
                                                        "ECORE_POS_MAP"
                                                        "_DECELERATE"));
  register_pos_map_sinusoidal(isolate, exports,
                              compatibility_new<String>(isolate,
                                                        "ECORE_POS_MAP"
                                                        "_SINUSOIDAL"));
  register_pos_map_accelerate_factor(isolate, exports,
                                     compatibility_new<String>(isolate,
                                                               "ECORE_POS_MAP"
                                                               "_ACCELERATE"
                                                               "_FACTOR"));
  register_pos_map_decelerate_factor(isolate, exports,
                                     compatibility_new<String>(isolate,
                                                               "ECORE_POS_MAP"
                                                               "_DECELERATE"
                                                               "_FACTOR"));
  register_pos_map_sinusoidal_factor(isolate, exports,
                                     compatibility_new<String>(isolate,
                                                               "ECORE_POS_MAP"
                                                               "_SINUSOIDAL"
                                                               "_FACTOR"));
  register_pos_map_divisor_interp(isolate, exports,
                                  compatibility_new<String>(isolate,
                                                            "ECORE_POS_MAP"
                                                            "_DIVISOR_INTERP"));
  register_pos_map_bounce(isolate, exports,
                          compatibility_new<String>(isolate,
                                                    "ECORE_POS_MAP_BOUNCE"));
  register_pos_map_spring(isolate, exports,
                          compatibility_new<String>(isolate,
                                                    "ECORE_POS_MAP_SPRING"));
  register_pos_map_cubic_bezier(isolate, exports,
                                compatibility_new<String>(isolate,
                                                          "ECORE_POS_MAP_CUBIC"
                                                          "_BEZIER"));
  register_animator_source_timer(isolate, exports,
                                 compatibility_new<String>(isolate,
                                                           "ECORE_ANIMATOR"
                                                           "_SOURCE_TIMER"));
  register_animator_source_custom(isolate, exports,
                                  compatibility_new<String>(isolate,
                                                            "ECORE_ANIMATOR"
                                                            "_SOURCE_CUSTOM"));
  register_animator_frametime_set(isolate, exports,
                                  compatibility_new<String>(isolate,
                                                            "ecore_animator"
                                                            "_frametime_set"));
  register_animator_frametime_get(isolate, exports,
                                  compatibility_new<String>(isolate,
                                                            "ecore_animator"
                                                            "_frametime_get"));
  register_animator_pos_map(isolate, exports,
                            compatibility_new<String>(isolate,
                                                      "ecore_animator_pos"
                                                      "_map"));
  register_animator_pos_map_n(isolate, exports,
                              compatibility_new<String>(isolate,
                                                        "ecore_animator_pos_map"
                                                        "_n"));
  register_animator_source_set(isolate, exports,
                               compatibility_new<String>(isolate,
                                                         "ecore_animator_source"
                                                         "_set"));
  register_animator_source_get(isolate, exports,
                               compatibility_new<String>(isolate,
                                                         "ecore_animator_source"
                                                         "_get"));
  register_animator_custom_source_tick_begin_callback_set
      (isolate, exports,
       compatibility_new<String>(isolate,
                                 "ecore_animator_custom_source_tick_begin"
                                 "_callback_set"));
  register_animator_custom_source_tick_end_callback_set
      (isolate, exports,
       compatibility_new<String>(isolate,
                                 "ecore_animator_custom_source_tick_end"
                                 "_callback_set"));
  register_animator_custom_tick(isolate, exports,
                                compatibility_new<String>(isolate,
                                                          "ecore_animator"
                                                          "_custom_tick"));
  register_animator_add(isolate, exports,
                        compatibility_new<String>(isolate,
                                                  "ecore_animator_add"));
  register_animator_timeline_add(isolate, exports,
                                 compatibility_new<String>(isolate,
                                                           "ecore_animator"
                                                           "_timeline_add"));

  // poller
  register_poller_core(isolate, exports,
                       compatibility_new<String>(isolate, "ECORE_POLLER_CORE"));
  register_poller_poll_interval_set(isolate, exports,
                                    compatibility_new<String>(isolate,
                                                              "ecore_poller"
                                                              "_poll_interval"
                                                              "_set"));
  register_poller_poll_interval_get(isolate, exports,
                                    compatibility_new<String>(isolate,
                                                              "ecore_poller"
                                                              "_poll_interval"
                                                              "_get"));
  register_poller_add(isolate, exports,
                      compatibility_new<String>(isolate, "ecore_poller_add"));

  // throttle
  register_throttle_adjust(isolate, exports,
                           compatibility_new<String>(isolate,
                                                     "ecore_throttle_adjust"));
  register_throttle_get(isolate, exports,
                        compatibility_new<String>(isolate,
                                                  "ecore_throttle_get"));

  std::cerr << __LINE__ << std::endl;
}

#ifndef HAVE_NODEJS
int main(int, char*[])
{
  efl::eina::eina_init eina_init;
  efl::eo::eo_init eo_init;

  efl::eina::js::compatibility_initialize();
  v8::Isolate* isolate = v8::Isolate::New();
  assert(isolate != 0);

  v8::Isolate::Scope isolate_scope(isolate);

  efl::eina::js::compatibility_handle_scope handle_scope(isolate);
  v8::Handle<v8::Context> context
    = efl::eina::js::compatibility_new<v8::Context>
    (isolate, nullptr
     , efl::eina::js::compatibility_new<v8::ObjectTemplate>(isolate));
  if (context.IsEmpty()) {
    fprintf(stderr, "Error creating context\n");
    return 1;
  }
  context->Enter();
  {
    std::vector<char> script;
    {
      std::ifstream script_file(TESTS_SRC_DIR "/ecore_js_suite.js");
      script_file.seekg(0, std::ios::end);
      std::size_t script_size = script_file.tellg();
      script_file.seekg(0, std::ios::beg);
      script.resize(script_size+1);
      script_file.rdbuf()->sgetn(&script[0], script_size);
      auto line_break = std::find(script.begin(), script.end(), '\n');
      assert(line_break != script.end());
      ++line_break;
      std::fill(script.begin(), line_break, ' ');

      std::cerr << "program:" << std::endl;
      std::copy(script.begin(), script.end(), std::ostream_iterator<char>(std::cerr));
      std::cerr << "end of program" << std::endl;
    }


    // Enter the execution environment before evaluating any code.
    v8::Context::Scope context_scope(context);
    v8::Local<v8::String> name(efl::eina::js::compatibility_new<v8::String>
                               (nullptr, "(shell)"));
    v8::Local<v8::Object> global = context->Global();
    v8::Handle<v8::Object> console = efl::eina::js::compatibility_new<v8::Object>(isolate);
    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "console"), console);
    console->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "log")
                 , efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, & ::Print)
                 ->GetFunction());

    std::cerr << __LINE__ << std::endl;
    v8::Handle<v8::Object> exports = efl::eina::js::compatibility_new<v8::Object>(isolate);
    global->Set(efl::eina::js::compatibility_new<v8::String>(isolate, "suite"), exports);

    test_setup(exports);
    std::cerr << __LINE__ << std::endl;


    efl::eina::js::compatibility_handle_scope handle_scope(v8::Isolate::GetCurrent());
    std::cerr << __LINE__ << std::endl;
    ExecuteString(v8::Isolate::GetCurrent(),
                  efl::eina::js::compatibility_new<v8::String>(v8::Isolate::GetCurrent(), &script[0]),
                  name);
  std::cerr << __LINE__ << std::endl;
  }
  context->Exit();
}

#else
#include EINA_STRINGIZE(NODE_INCLUDE_HEADER)

namespace {

void eina_js_module_init(v8::Handle<v8::Object> exports)
{
  fprintf(stderr, "teste\n"); fflush(stderr);
  try
    {

      eina_init();
      eo_init();

      test_setup(exports);

      std::cerr << "registered" << std::endl;
    }
  catch(...)
    {
      std::cerr << "Error" << std::endl;
      std::abort();
    }
}

}

NODE_MODULE(ecore_js_suite_mod, ::eina_js_module_init)

#endif
