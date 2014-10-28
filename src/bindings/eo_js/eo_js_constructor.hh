#ifndef EFL_EO_JS_CONSTRUCTOR_HH
#define EFL_EO_JS_CONSTRUCTOR_HH

#include <v8.h>

#include <eina_tuple.hh>
#include <Eo.h>

#include <cstdlib>

#include <functional>

#include <iostream>

#include <cassert>

namespace efl { namespace eo { namespace js {

inline void constructor(v8::FunctionCallbackInfo<v8::Value> const& args)
{
  if(args.IsConstructCall())
    {
      void* data = v8::External::Cast(*args.Data())->Value();
      std::function<void(v8::FunctionCallbackInfo<v8::Value>const&)>*
        f = static_cast<std::function<void(v8::FunctionCallbackInfo<v8::Value>const&)>*>(data);
      (*f)(args);
    }
  else
    std::abort();
}

template <typename...F>
struct constructor_caller
{
  struct call
  {
    template <typename T>
    void operator()(T function) const
    {
      std::cout << " should call " << typeid(function).name() << std::endl;
    }

    v8::FunctionCallbackInfo<v8::Value> const* args;
  };

  void operator()(v8::FunctionCallbackInfo<v8::Value> const& args)
  {
    std::cout << "Constructing" << std::endl;
    Eo* eo = eo_add
      (klass
       , NULL
       , eina::_mpl::for_each(constructors, call())
      );
    assert(eo != 0);
    v8::Local<v8::Object> self = args.This();
    self->SetInternalField(0, v8::External::New(args.GetIsolate(), eo));
  }

  Eo_Class const* klass;
  std::tuple<F...> constructors;
};

template <typename... F>
v8::Handle<v8::Value> constructor_data(v8::Isolate* isolate, Eo_Class const* klass, F... f)
{
  return v8::External::New
    (isolate
     , new std::function<void(v8::FunctionCallbackInfo<v8::Value> const&)>
     (constructor_caller<F...>{klass, std::tuple<F...>{f...}})
     );
}

} } }

#endif
