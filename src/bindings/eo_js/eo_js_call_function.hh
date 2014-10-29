#ifndef EFL_EO_JS_CALL_FUNCTION_HH
#define EFL_EO_JS_CALL_FUNCTION_HH

#include <v8.h>

#include <eina_tuple.hh>
#include <eina_function.hh>
#include <Eo.h>

#include <eo_js_get_value.hh>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <cassert>
#include <vector>

namespace efl { namespace eo { namespace js {

inline void call_function(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  void* data = v8::External::Cast(*args.Data())->Value();
  std::function<void(v8::FunctionCallbackInfo<v8::Value>const&)>*
    f = static_cast<std::function<void(v8::FunctionCallbackInfo<v8::Value>const&)>*>(data);
  (*f)(args);
}

template <typename F>
struct method_caller
{
  template <typename U, std::size_t I>
  static
  typename std::tuple_element<I, typename eina::_mpl::function_params<U>::type>::type
  get_value(v8::Local<v8::Value> v, v8::Isolate* isolate)
  {
    return js::get_value_from_javascript
      (v, isolate
       , js::value_tag<typename std::tuple_element
       <I, typename eina::_mpl::function_params<U>::type>::type>());
  }

  template <std::size_t... I>
  void aux(v8::FunctionCallbackInfo<v8::Value> const& args, eina::index_sequence<I...>) const
  {
    function(get_value<F, I>(args[I], args.GetIsolate())...);
  }
  
  void operator()(v8::FunctionCallbackInfo<v8::Value> const& args)
  {
    std::size_t const parameters
      = std::tuple_size<typename eina::_mpl::function_params<F>::type>::value;
    if(parameters <= args.Length())
      {
        v8::Local<v8::Object> self = args.This();
        v8::Local<v8::Value> external = self->GetInternalField(0);
        Eo* eo = static_cast<Eo*>(v8::External::Cast(*external)->Value());
        try
          {
            eo_do(eo, aux(args, eina::make_index_sequence<parameters>()));
          }
        catch(std::logic_error const&) {}
      }
    else
      {
        args.GetIsolate()->ThrowException
          (v8::Exception::TypeError
           (v8::String::NewFromUtf8(args.GetIsolate(), "Expected more arguments for this call")));
      }      
  }

  F function;
};

template <typename F>
v8::Handle<v8::Value> call_function_data(v8::Isolate* isolate, F f)
{
  return v8::External::New
    (isolate, new std::function<void(v8::FunctionCallbackInfo<v8::Value> const&)>
     (method_caller<F>{f}));
}

} } }

#endif
