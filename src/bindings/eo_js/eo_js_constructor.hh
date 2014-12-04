#ifndef EFL_EO_JS_CONSTRUCTOR_HH
#define EFL_EO_JS_CONSTRUCTOR_HH

#if 0
#include <v8.h>
#else
#include <node/v8.h>
#endif

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

#if 0
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
    {
      std::size_t argc = args.Length();
      std::vector<v8::Local<v8::Value> > argv (argc ? argc : 1 );
      for(int i = 0; i != args.Length(); ++i)
        argv[i] = args[i];
      args.Callee()->NewInstance(argc, &argv[0]);
    }
}
#else
inline v8::Handle<v8::Value> constructor(v8::Arguments const& args)
{
  std::cout << "function called " << __func__ << std::endl;
  if(args.IsConstructCall())
    {
      void* data = v8::External::Cast(*args.Data())->Value();
      std::function<v8::Handle<v8::Value>(v8::Arguments const&)>*
        f = static_cast<std::function<v8::Handle<v8::Value>(v8::Arguments const&)>*>(data);
      (*f)(args);
    }
  else
    {
      std::size_t argc = args.Length();
      std::vector<v8::Local<v8::Value> > argv (argc ? argc : 1 );
      for(int i = 0; i != args.Length(); ++i)
        argv[i] = args[i];
      args.Callee()->NewInstance(argc, &argv[0]);
      return v8::Handle<v8::Value>();
    }
}
#endif
      
      //template <typename...F>
struct constructor_caller
{
  struct call
  {
    template <typename T>
    void operator()(T function) const
    {
      std::cout << "function called " << __func__ << std::endl;
      int const parameters
        = std::tuple_size<typename eina::_mpl::function_params<T>::type>::value;
      if(*current + parameters <= args->Length())
        {
          aux(function, eina::make_index_sequence<parameters>());
          *current += parameters;
        }
      else
        {
#if 0
          args->GetIsolate()->
#else
            v8::
#endif
            ThrowException
            (v8::Exception::TypeError
             (v8::String::New/*FromUtf8*/(/*args->GetIsolate(), */"Expected more arguments for this call")));
          throw std::logic_error("");
        }
    }

    template <typename U, std::size_t I>
    static
    typename std::tuple_element<I, typename eina::_mpl::function_params<U>::type>::type
    get_value(v8::Local<v8::Value> v, v8::Isolate* isolate)
    {
      std::cout << "function called " << __func__ << std::endl;
      return js::get_value_from_javascript
        (v, isolate
         , js::value_tag<typename std::tuple_element
         <I, typename eina::_mpl::function_params<U>::type>::type>());
    }
    
    template <typename T, std::size_t... I>
    void aux(T function, eina::index_sequence<I...>) const
    {
      std::cout << "function called " << __func__ << std::endl;
      function(get_value<T, I>((*args)[I + *current], args->GetIsolate())...);
    }

    int* current;
#if 0
    v8::FunctionCallbackInfo<v8::Value> const* args;
#else
    v8::Arguments const* args;
#endif
  };

#if 0
  void operator()(v8::FunctionCallbackInfo<v8::Value> const& args) const
  {
    int current_index = 0;
    try
      {
        Eo* eo = eo_add
          (klass
           , NULL
           // , eina::_mpl::for_each(constructors, call{&current_index, &args})
           );
        assert(eo != 0);
        v8::Local<v8::Object> self = args.This();
        self->SetInternalField(0, v8::External::New(args.GetIsolate(), eo));
      }
    catch(std::logic_error const&) {}
  }
#else
  v8::Handle<v8::Value> operator()(v8::Arguments const& args) const
  {
    std::cout << "function called " << __func__ << std::endl;
    int current_index = 0;
    try
      {
        Eo* eo = eo_add
          (klass
           , NULL
           // , eina::_mpl::for_each(constructors, call{&current_index, &args})
           );
        assert(eo != 0);
        v8::Local<v8::Object> self = args.This();
        self->SetInternalField(0, v8::External::New(/*args.GetIsolate(),*/ eo));
      }
    catch(std::logic_error const&) {}
    return v8::Handle<v8::Value>();
  }
#endif
  
  Eo_Class const* klass;
  //std::tuple<F...> constructors;
};

#if 0
template <typename... F>
v8::Handle<v8::Value> constructor_data(v8::Isolate* isolate, Eo_Class const* klass, F... f)
{
  return v8::External::New
    (isolate
     , new std::function<void(v8::FunctionCallbackInfo<v8::Value> const&)>
     (constructor_caller<F...>{klass, std::tuple<F...>{f...}})
     );
}
#else
template <typename T> void foo(T const&);
      //template <typename... F>
inline
v8::Handle<v8::Value> constructor_data(v8::Isolate* isolate, Eo_Class const* klass/*, F... f*/)
{
  fprintf(stderr, "function called %s\n", __func__); fflush(stderr);
  std::cerr << "function called " << __func__ << std::endl;
  return v8::External::New
    (new std::function<v8::Handle<v8::Value>(v8::Arguments const&)>
     (constructor_caller/*<F...>*/{klass/*, std::tuple<F...>{f...}*/}));
}
inline
/*v8::Handle<v8::Value>*/void constructor_data1(v8::Isolate* isolate, Eo_Class const* klass/*, F... f*/)
{
  fprintf(stderr, "function called %s\n", __func__); fflush(stderr);
  std::cerr << "function called " << __func__ << std::endl;
  // return v8::External::New
   // new std::function<v8::Handle<v8::Value>(v8::Arguments const&)>(
  std::function<v8::Handle<v8::Value>(v8::Arguments const&)>
    ((constructor_caller/*<F...>*/{nullptr/*klass/*, std::tuple<F...>{f...}*/}));
  // return v8::Handle<v8::Value>();
}
#endif

} } }

#endif
