#ifndef EFL_EO_JS_CONSTRUCTOR_HH
#define EFL_EO_JS_CONSTRUCTOR_HH

#include <v8.h>

#include <Eo.h>

#include <cstdlib>

#include <functional>

#include <iostream>

namespace efl { namespace eo { namespace js {

inline void constructor(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  std::cout << "Constructor ! " << std::endl;
  if(args.IsConstructCall())
    {
      
    }
  else
    std::abort();
}

template <typename T = void>
v8::Handle<v8::Value> constructor_data(v8::Isolate* isolate, Eo_Class const* /*cls*/)
{
  return v8::External::New(isolate, new std::function<void()>());
}

} } }

#endif
