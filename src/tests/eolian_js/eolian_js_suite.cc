#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>
#include <Eina_Js.hh>

#include <check.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>

#ifndef HAVE_NODEJS
#include "suite_runner.hh"
void eolian_js_test_register_eolian_js_binding(v8::Handle<v8::Object> exports);

int main(int, char*[])
{
  return run_script(TESTS_SRC_DIR "/eolian_js_suite.js", &eolian_js_test_register_eolian_js_binding);
}
#else
#ifdef HAVE_NODE_NODE_H
#include <node/node.h>
#elif defined(HAVE_NODEJS_DEPS_NODE_NODE_H)
#include <nodejs/deps/node/node.h>
#elif defined(HAVE_NODEJS_DEPS_NODE_INCLUDE_NODE_H)
#include <nodejs/deps/node/include/node.h>
#elif defined(HAVE_NODEJS_SRC_NODE_H)
#include <nodejs/src/node.h>
#elif defined(HAVE_NODE_H)
#include <node.h>
#else
#error We must have at least one node header to include
#endif
void eolian_js_test_register_eolian_js_binding(v8::Handle<v8::Object> exports);

namespace {

void eolian_js_module_init(v8::Handle<v8::Object> exports)
{
  fprintf(stderr, "test suite eolian_js_module_init\n"); fflush(stderr);
  try
    {
       eina_init();
       efl_object_init();
       eolian_js_test_register_eolian_js_binding(exports);
       std::cerr << "registered" << std::endl;
    }
  catch(...)
    {
       std::cerr << "Error" << std::endl;
       std::abort();
    }
}

}

NODE_MODULE(eolian_js_suite_mod, ::eolian_js_module_init)

#endif
