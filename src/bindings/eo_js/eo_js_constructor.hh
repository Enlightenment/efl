#ifndef EFL_EO_JS_CONSTRUCTOR_HH
#define EFL_EO_JS_CONSTRUCTOR_HH

#include <v8.h>

#include <eina_tuple.hh>
#include <eina_function.hh>
#include <Eo.h>

#include <eo_js_get_value.hh>

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
      aux(function, eina::make_index_sequence<std::tuple_size
          <typename eina::_mpl::function_params<T>::type>::value>());
    }

    template <typename U, std::size_t I>
    static
    typename std::tuple_element<I, typename eina::_mpl::function_params<U>::type>::type
    get_value(v8::Local<v8::Value> v)
    {
      return js::get_value_from_javascript
        (v, js::value_tag<typename std::tuple_element<I, typename eina::_mpl::function_params<U>::type>::type>());
    }
    
    template <typename T, std::size_t... I>
    void aux(T function, eina::index_sequence<I...>) const
    {
      function(get_value<T, I>((*args)[I + *current])...);
      std::cout << " should call " << typeid(function).name() << std::endl;
    }

    std::size_t* current;
    v8::FunctionCallbackInfo<v8::Value> const* args;
  };

  void operator()(v8::FunctionCallbackInfo<v8::Value> const& args)
  {
    std::cout << "Constructing" << std::endl;
    std::size_t current_index = 0;
    Eo* eo = eo_add
      (klass
       , NULL
       , eina::_mpl::for_each(constructors, call{&current_index, &args})
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
