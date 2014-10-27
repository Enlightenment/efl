#ifndef EFL_EO_JS_CALL_FUNCTION_HH
#define EFL_EO_JS_CALL_FUNCTION_HH

#include <v8.h>

#include <Eo.h>

#include <cstdlib>

#include <functional>

namespace efl { namespace eo { namespace js {

inline void call_function(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  if(args.IsConstructCall())
    {
      
    }
  else
    std::abort();
}

template <typename T>
v8::Handle<v8::Value> call_function_data(v8::Isolate* isolate, T /*f*/)
{
  return v8::External::New(isolate, new std::function<void()>());
}

} } }

#endif
