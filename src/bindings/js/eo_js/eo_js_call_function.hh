#ifndef EFL_EO_JS_CALL_FUNCTION_HH
#define EFL_EO_JS_CALL_FUNCTION_HH

#include <eina_tuple.hh>
#include <eina_tuple_c.hh>
#include <eina_function.hh>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <cassert>
#include <vector>

namespace efl { namespace eo { namespace js {

inline eina::js::compatibility_return_type call_function(eina::js::compatibility_callback_info_type args)
{
  void* data = v8::External::Cast(*args.Data())->Value();
  std::function<eina::js::compatibility_return_type(eina::js::compatibility_callback_info_type)>*
    f = static_cast<std::function<eina::js::compatibility_return_type(eina::js::compatibility_callback_info_type)>*>(data);
  return (*f)(args);
}

template <typename In, std::size_t I>
struct is_in : eina::_mpl::tuple_contains<std::integral_constant<std::size_t, I>, In>
{};

template <typename In, std::size_t I>
struct arg_index : std::integral_constant<std::size_t,
  std::conditional<is_in<In, I>::value, std::integral_constant<std::size_t, 1>, std::integral_constant<std::size_t, 0>>::type::value +
  arg_index<In, I - 1>::value>
{};

template <typename In>
struct arg_index<In, 0> : std::integral_constant<std::size_t, 0>
{};

template <typename In, typename Out, typename Ownership, typename F, typename Return, typename Parameters>
struct method_caller
{
  typedef typename eo::js::eo_function_params<F>::type parameters_t;

  template <std::size_t I>
  struct is_out : eina::_mpl::tuple_contains<std::integral_constant<std::size_t, I>, Out>
  {};

  template <std::size_t I>
  struct is_inout : std::integral_constant<bool,
    eina::_mpl::tuple_contains<std::integral_constant<std::size_t, I>, Out>::value &&
    eina::_mpl::tuple_contains<std::integral_constant<std::size_t, I>, In>::value>
  {};

  template <std::size_t I, typename Outs>
  static
  typename std::tuple_element<I, parameters_t>::type
  get_value(eina::js::compatibility_callback_info_type args, Outs& /*outs*/, v8::Isolate* isolate, char const* class_name
            , std::false_type)
  {
    using Param = typename std::tuple_element<I, Parameters>::type;
    const auto js_index = arg_index<In, I>::value;

    return eina::js::get_value_from_javascript
      (args[js_index], isolate, class_name, eina::js::value_tag<Param>());
  }

  template <std::size_t I, typename Outs>
  static
  typename std::add_pointer<
  typename std::tuple_element
    <eina::_mpl::tuple_find<std::integral_constant<std::size_t, I>, Out>::value
     , Outs>::type>::type
  get_value(eina::js::compatibility_callback_info_type, Outs& outs, v8::Isolate*, char const*
            , std::true_type)
  {
    return &std::get<eina::_mpl::tuple_find<std::integral_constant<std::size_t, I>, Out>::value>
      (outs);
  }

  template <typename OutParameter, std::size_t I, typename R>
  eina::js::compatibility_return_type
  create_return_unique_value(eina::js::compatibility_callback_info_type args
                             , R const& r) const
  {
    return eina::js::compatibility_return(
      eina::js::get_value_from_c(
        eina::js::wrap_value<OutParameter>(r, eina::js::value_tag<OutParameter>{}),
        args.GetIsolate(),
        class_names[I]),
      args);
  }

  template <typename OutParameters, typename Outs>
  eina::js::compatibility_return_type
  create_return_value(eina::js::compatibility_callback_info_type args
                      , Outs const& outs
                      , typename std::enable_if<std::tuple_size<Outs>::value == 1>::type* = 0) const
  {
    using OutParameter = typename std::tuple_element<0, OutParameters>::type;
    return create_return_unique_value<OutParameter, std::tuple_element<0, Out>::type::value>(args, std::get<0u>(outs));
  }

  template <typename OutParameters, typename Outs>
  eina::js::compatibility_return_type
  create_return_value(eina::js::compatibility_callback_info_type
                      , Outs const&
                      , typename std::enable_if<std::tuple_size<Outs>::value == 0>::type* = 0) const
  {
    // nothing
    return eina::js::compatibility_return();
  }

  template <typename OutParameters, typename Outs>
  eina::js::compatibility_return_type
  create_return_value(eina::js::compatibility_callback_info_type args
                      , Outs const& outs
                      , typename std::enable_if<(std::tuple_size<Outs>::value > 1)>::type* = 0) const
  {
    v8::Isolate* isolate = args.GetIsolate();
    int const length = std::tuple_size<Outs>::value;
    v8::Local<v8::Array> ret = eina::js::compatibility_new<v8::Array>(isolate, length);
    set_return<OutParameters, 0u>(isolate, ret, outs, eina::make_index_sequence<std::tuple_size<Outs>::value>());
    return eina::js::compatibility_return(ret, args);
  }

  template <typename OutParameters, typename R, typename Outs>
  eina::js::compatibility_return_type
  create_return_value(eina::js::compatibility_callback_info_type args
                      , R const& r
                      , Outs const&
                      , typename std::enable_if<std::tuple_size<Outs>::value == 0>::type* = 0) const
  {
    return create_return_unique_value<Return, std::tuple_size<Ownership>::value>(args, r);
  }

  template <typename OutParameters, std::size_t Offset, typename Outs, std::size_t...S>
  void set_return(v8::Isolate* isolate, v8::Local<v8::Array> r
                  , Outs const& outs, eina::index_sequence<S...>) const
  {
    std::initializer_list<int> l
      = {(r->Set(S+Offset, eina::js::get_value_from_c(
            eina::js::wrap_value<typename std::tuple_element<S, OutParameters>::type>(std::get<S>(outs), eina::js::value_tag<typename std::tuple_element<S, OutParameters>::type>{}),
            isolate,
            class_names[std::tuple_element<S, Out>::type::value])),0)...};
    static_cast<void>(l);
  }

  template <typename OutParameters, typename R, typename Outs>
  eina::js::compatibility_return_type
  create_return_value(eina::js::compatibility_callback_info_type args
                      , R const& r
                      , Outs const& outs
                      , typename std::enable_if<std::tuple_size<Outs>::value != 0>::type* = 0) const
  {
    v8::Isolate* isolate = args.GetIsolate();
    int const length = std::tuple_size<Outs>::value + 1;
    v8::Local<v8::Array> ret = eina::js::compatibility_new<v8::Array>(isolate, length);
    ret->Set(0, eina::js::get_value_from_c(
      eina::js::wrap_value<Return>(r, eina::js::value_tag<Return>{}),
      isolate,
      class_names[std::tuple_size<Ownership>::value]));
    set_return<OutParameters, 1u>(isolate, ret, outs, eina::make_index_sequence<std::tuple_size<Outs>::value>());
    return eina::js::compatibility_return(ret, args);
  }

  template <std::size_t I, typename Outs>
  static void
  init_inout(eina::js::compatibility_callback_info_type args
            , Outs& outs
            , v8::Isolate* isolate
            , char const* class_name
            , std::true_type)
  {
    using Param = typename std::remove_pointer<typename std::tuple_element<I, Parameters>::type>::type;

    const auto js_index = arg_index<In, I>::value;

    *get_value<I>(args, outs, args.GetIsolate(), class_name, std::true_type()) =
      eina::js::get_value_from_javascript(args[js_index], isolate, class_name, eina::js::value_tag<Param>());;
  }

  template <std::size_t I, typename Outs>
  static void
  init_inout(eina::js::compatibility_callback_info_type
            , Outs&
            , v8::Isolate*
            , char const*
            , std::false_type)
  {
  }

  template <typename OutParameters, std::size_t... I>
  eina::js::compatibility_return_type
  aux(Eo* eo, eina::js::compatibility_callback_info_type args, eina::index_sequence<I...>
      , std::true_type) const
  {
    typename eina::_mpl::tuple_transform<Out, out_transform<parameters_t> >::type outs {};
    static_cast<void>(outs);

    std::initializer_list<int> l =
      {(init_inout<I>(args, outs, args.GetIsolate(), class_names[I], typename is_inout<I>::type()), 0)...};
    static_cast<void>(l);

    function(eo, get_value<I>(args, outs, args.GetIsolate(), class_names[I], typename is_out<I>::type())...);
    return create_return_value<OutParameters>(args, outs);
  }

  template <typename OutParameters, std::size_t... I>
  eina::js::compatibility_return_type
  aux(Eo* eo, eina::js::compatibility_callback_info_type args, eina::index_sequence<I...>
      , std::false_type) const
  {
    typename eina::_mpl::tuple_transform<Out, out_transform<parameters_t> >::type outs {};
    static_cast<void>(outs);

    std::initializer_list<int> l =
      {(init_inout<I>(args, outs, args.GetIsolate(), class_names[I], typename is_inout<I>::type()), 0)...};
    static_cast<void>(l);

    typename eina::_mpl::function_return<F>::type r =
      function(eo, get_value<I>(args, outs, args.GetIsolate(), class_names[I], typename is_out<I>::type())...);
    return create_return_value<OutParameters>(args, r, outs);
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

  eina::js::compatibility_return_type operator()(eina::js::compatibility_callback_info_type args)
  {
    using OutParameters = typename eina::_mpl::tuple_transform<Out, out_transform<Parameters> >::type;

    int input_parameters = std::tuple_size<In>::value;
    if(input_parameters <= args.Length())
      {
        v8::Local<v8::Object> self = args.This();
        v8::Local<v8::Value> external = self->GetInternalField(0);
        Eo* eo = static_cast<Eo*>(v8::External::Cast(*external)->Value());
        try
          {
             aux<OutParameters>(eo, args,
               eina::make_index_sequence<std::tuple_size<parameters_t>::value>(),
               std::is_same<void, Return>());
          }
        catch(std::logic_error const&)
          {
            return eina::js::compatibility_return();
          }
      }
    else
      {
        return eina::js::compatibility_throw
          (v8::Exception::TypeError
           (eina::js::compatibility_new<v8::String>(nullptr, "Expected more arguments for this call")));
      }
  }

  template <typename A>
  method_caller(F f, A&& c)
    : function(f)
    , class_names(std::forward<A>(c))
  {}

  F function;
  /// Hold the names of the type of each argument, with the return's type name at the end
  std::array<const char*, std::tuple_size<Ownership>::value + 1> class_names;
};

template <typename In, typename Out, typename Ownership, typename Return, typename Parameters, std::size_t N, typename F>
v8::Handle<v8::Value> call_function_data(v8::Isolate* isolate, std::array<const char*, N> class_names, F f)
{
  return eina::js::compatibility_new<v8::External>
    (isolate, new std::function<eina::js::compatibility_return_type(eina::js::compatibility_callback_info_type const&)>
     (method_caller<In, Out, Ownership, F, Return, Parameters>{f, class_names}));
}

} } }

#endif
