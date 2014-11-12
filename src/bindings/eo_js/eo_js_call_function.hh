#ifndef EFL_EO_JS_CALL_FUNCTION_HH
#define EFL_EO_JS_CALL_FUNCTION_HH

#include <v8.h>

#include <eina_tuple.hh>
#include <eina_tuple_c.hh>
#include <eina_function.hh>
#include <Eo.h>

#include <eo_js_get_value.hh>
#include <eo_js_get_value_from_c.hh>

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

template <typename In, typename Out, typename Ownership, typename F>
struct method_caller
{
  typedef typename eina::_mpl::function_params<F>::type parameters_t;

  template <std::size_t I>
  struct is_out : eina::_mpl::tuple_contains<std::integral_constant<std::size_t, I>, Out>
  {};

  template <typename U, std::size_t I, typename Outs>
  static
  typename std::tuple_element<I, parameters_t>::type
  get_value(v8::FunctionCallbackInfo<v8::Value>const& args, Outs& /*outs*/, v8::Isolate* isolate
            , std::false_type)
  {
    std::cout << "is NOT out" << std::endl;
    return js::get_value_from_javascript
      (args[I], isolate
       , js::value_tag<typename std::tuple_element
       <I, typename eina::_mpl::function_params<U>::type>::type>());
  }
  template <typename U, std::size_t I, typename Outs>
  static
  typename std::add_pointer<
  typename std::tuple_element
    <eina::_mpl::tuple_find<std::integral_constant<std::size_t, I>, Out>::value
     , Outs>::type>::type
  get_value(v8::FunctionCallbackInfo<v8::Value>const&, Outs& outs, v8::Isolate*
            , std::true_type)
  {
    std::cout << "is out" << std::endl;
    return &std::get<eina::_mpl::tuple_find<std::integral_constant<std::size_t, I>, Out>::value>
      (outs);
  }

  template <typename R>
  void create_return_unique_value(v8::FunctionCallbackInfo<v8::Value> const& args
                                  , R const& r) const
  {
    args.GetReturnValue().Set(js::get_value_from_c(r, args.GetIsolate()));
  }
  
  template <typename Outs>
  void create_return_value(v8::FunctionCallbackInfo<v8::Value> const& args
                           , Outs const& outs
                           , typename std::enable_if<std::tuple_size<Outs>::value == 1>::type* = 0) const
  {
    create_return_unique_value(args, std::get<0u>(outs));
  }

  template <typename Outs>
  void create_return_value(v8::FunctionCallbackInfo<v8::Value> const&
                           , Outs const&
                           , typename std::enable_if<std::tuple_size<Outs>::value == 0>::type* = 0) const
  {
    // nothing
  }

  template <typename Outs>
  void create_return_value(v8::FunctionCallbackInfo<v8::Value> const& args
                           , Outs const& outs
                           , typename std::enable_if<(std::tuple_size<Outs>::value > 1)>::type* = 0) const
  {
    v8::Isolate* isolate = args.GetIsolate();
    int const length = std::tuple_size<Outs>::value;
    v8::Local<v8::Array> ret = v8::Array::New(isolate, length);
    set_return<0u>(isolate, ret, outs, eina::make_index_sequence<std::tuple_size<Outs>::value>());
    args.GetReturnValue().Set(ret);
  }
  
  template <typename R, typename Outs>
  void create_return_value(v8::FunctionCallbackInfo<v8::Value> const& args
                           , R const& r
                           , Outs const&
                           , typename std::enable_if<std::tuple_size<Outs>::value == 0>::type* = 0) const
  {
    create_return_unique_value(args, r);
  }

  template <std::size_t Offset, typename Outs, std::size_t...S>
  void set_return(v8::Isolate* isolate, v8::Local<v8::Array> r
                  , Outs const& outs, eina::index_sequence<S...>) const
  {
    std::initializer_list<int> l
      = {(r->Set(S+Offset, js::get_value_from_c(std::get<S>(outs), isolate)),0)...};
    static_cast<void>(l);
  }

  template <typename R, typename Outs>
  void create_return_value(v8::FunctionCallbackInfo<v8::Value> const& args
                           , R const& r
                           , Outs const& outs
                           , typename std::enable_if<std::tuple_size<Outs>::value != 0>::type* = 0) const
  {
    v8::Isolate* isolate = args.GetIsolate();
    int const length = std::tuple_size<Outs>::value + 1;
    v8::Local<v8::Array> ret = v8::Array::New(isolate, length);
    ret->Set(0, js::get_value_from_c(r, isolate));
    set_return<1u>(isolate, ret, outs, eina::make_index_sequence<std::tuple_size<Outs>::value>());
    args.GetReturnValue().Set(ret);
  }
  
  template <std::size_t... I>
  void aux(v8::FunctionCallbackInfo<v8::Value> const& args, eina::index_sequence<I...>
           , std::true_type) const
  {
    typename eina::_mpl::tuple_transform<Out, out_transform<parameters_t> >::type outs {};
    static_cast<void>(outs);
    
    function(get_value<F, I>(args, outs, args.GetIsolate(), typename is_out<I>::type())...);
    create_return_value(args, outs);
  }

  template <std::size_t... I>
  void aux(v8::FunctionCallbackInfo<v8::Value> const& args, eina::index_sequence<I...>
           , std::false_type) const
  {
    typename eina::_mpl::tuple_transform<Out, out_transform<parameters_t> >::type outs {};
    static_cast<void>(outs);

    typename eina::_mpl::function_return<F>::type r =
      function(get_value<F, I>(args, outs, args.GetIsolate(), typename is_out<I>::type())...);
    create_return_value(args, r, outs);
  }
  
  template <typename P>
  struct out_transform
  {
    template <typename T>
    struct apply
    {
      typedef typename std::remove_pointer<typename std::tuple_element<T::value, P>::type>::type type;
    };
  };
  
  void operator()(v8::FunctionCallbackInfo<v8::Value> const& args)
  {
    int input_parameters = std::tuple_size<In>::value;
    if(input_parameters <= args.Length())
      {
        v8::Local<v8::Object> self = args.This();
        v8::Local<v8::Value> external = self->GetInternalField(0);
        Eo* eo = static_cast<Eo*>(v8::External::Cast(*external)->Value());
        try
          {
            eo_do(eo, aux(args, eina::make_index_sequence<std::tuple_size<parameters_t>::value>()
                          , std::is_same<void, typename eina::_mpl::function_return<F>::type>()));
          }
        catch(std::logic_error const&) {}
      }
    else
      {
        std::cout << "Expected more arguments for this call" << std::endl;
        args.GetIsolate()->ThrowException
          (v8::Exception::TypeError
           (v8::String::NewFromUtf8(args.GetIsolate(), "Expected more arguments for this call")));
      }      
  }

  F function;
};

template <typename In, typename Out, typename Ownership, typename F>
v8::Handle<v8::Value> call_function_data(v8::Isolate* isolate, F f)
{
  return v8::External::New
    (isolate, new std::function<void(v8::FunctionCallbackInfo<v8::Value> const&)>
     (method_caller<In, Out, Ownership, F>{f}));
}

} } }

#endif
