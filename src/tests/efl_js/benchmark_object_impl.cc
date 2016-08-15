#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>

#include <stdlib.h>

extern "C" {
#include "benchmark_object.eo.h"

void _benchmark_object_emptyarg(Eo*, void*)
{
}
void _benchmark_object_onearg(Eo*, void*, int)
{
}
void _benchmark_object_twoarg(Eo*, void*, int, int)
{
}
void _benchmark_object_tenarg(Eo*, void*, int, int, int, int, int, int, int, int, int, int)
{
}
void _benchmark_object_onecomplexarg(Eo*, void*, Eina_List*)
{
}
void _benchmark_object_tencomplexarg(Eo*, void*, Eina_List*, Eina_List*, Eina_List*, Eina_List*, Eina_List*, Eina_List*, Eina_List*, Eina_List*, Eina_List*, Eina_List*)
{
}
  
#include "benchmark_object.eo.c"
}

#include "benchmark_object.eo.js.cc"

#ifdef HAVE_NODEJS
namespace {
using efl::eina::js::compatibility_return_type;
using efl::eina::js::compatibility_callback_info_type;
using efl::eina::js::compatibility_return;
using efl::eina::js::compatibility_new;

#define JS_BENCHMARK_ARGS0(v)
#define JS_BENCHMARK_ARGS1(v) ,v
#define JS_BENCHMARK_ARGS2(v) ,v,v
#define JS_BENCHMARK_ARGS10(v) ,v,v,v,v,v,v,v,v,v,v
#define JS_BENCHMARK_FUNC(name, number, v)                              \
  compatibility_return_type js_benchmark_object_##name##arg(compatibility_callback_info_type) \
  {                                                                     \
    Eina_Counter* counter = eina_counter_new("counter");                \
    Eo* object = efl_add(BENCHMARK_OBJECT_CLASS, NULL);                  \
    Eina_List* l = NULL;                                                \
    (void)l;                                                            \
    /* Warm */                                                          \
    for(int i = 0; i != 10; i++)                                        \
      {                                                                 \
        benchmark_object_##name##arg(object JS_BENCHMARK_ARGS##number(v)); \
      }                                                                 \
    /* Real loop */                                                     \
    eina_counter_start(counter);                                        \
    for(int i = 0; i != 20000; i++)                                     \
      {                                                                 \
        benchmark_object_##name##arg(object JS_BENCHMARK_ARGS##number(v)); \
      }                                                                 \
    eina_counter_stop(counter, 20000);                                  \
    fprintf(stderr, "%s", eina_counter_dump(counter));                  \
    efl_unref(object);                                                   \
    return compatibility_return();                                      \
  }

JS_BENCHMARK_FUNC(empty, 0, 1)
JS_BENCHMARK_FUNC(one, 1, 1)
JS_BENCHMARK_FUNC(two, 2, 1)
JS_BENCHMARK_FUNC(ten, 10, 1)
JS_BENCHMARK_FUNC(onecomplex, 1, l)
JS_BENCHMARK_FUNC(tencomplex, 10, l)

  
void benchmark_object_module_init(v8::Handle<v8::Object> exports)
{
  fprintf(stderr, "test suite eolian_js_module_init\n"); fflush(stderr);
  try
    {
       eina_init();
       efl_object_init();
       register_benchmark_object(exports, v8::Isolate::GetCurrent());
#define JS_BENCHMARK_EXPORT(name)                                       \
       exports->Set(compatibility_new<v8::String>(nullptr, "benchmark_" #name "arg") \
                    , compatibility_new<v8::FunctionTemplate>(nullptr, &js_benchmark_object_##name##arg)->GetFunction());
       JS_BENCHMARK_EXPORT(empty)
       JS_BENCHMARK_EXPORT(one)
       JS_BENCHMARK_EXPORT(two)
       JS_BENCHMARK_EXPORT(ten)
       JS_BENCHMARK_EXPORT(onecomplex)
       JS_BENCHMARK_EXPORT(tencomplex)
    }
  catch(...)
    {
       std::cerr << "Error" << std::endl;
       std::abort();
    }
}

}

#include <eina_js_node.hh>

NODE_MODULE(benchmark_object, ::benchmark_object_module_init)
#endif
