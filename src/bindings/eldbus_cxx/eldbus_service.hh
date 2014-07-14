#ifndef ELDBUS_CXX_ELDBUS_SERVICE_HH
#define ELDBUS_CXX_ELDBUS_SERVICE_HH

#include <array>

namespace efl { namespace eldbus {

struct service_interface
{
  service_interface(Eldbus_Service_Interface const* iface)
    : _iface(iface)
  {}

  typedef Eldbus_Service_Interface* native_handle_type;
  typedef Eldbus_Service_Interface const* const_native_handle_type;
  const_native_handle_type native_handle() { return _iface; }
  const_native_handle_type native_handle() const { return _iface; }
private:
  const_native_handle_type _iface;
};

namespace service {

template <typename F, typename Ins, typename Outs>
struct method_specification
{
  const char* name;
  F f;

  typedef F function_type;
  typedef Ins ins_type;
  typedef Outs outs_type;

  ins_type ins;
  outs_type outs;
};

namespace _detail {

template <typename T>
struct const_char_type
{
  typedef const char* type;
};

}

template <typename... Args>
struct ins
{
  ins(typename _detail::const_char_type<Args>::type... names) : _names({{names...}}) {}

  static constexpr std::size_t size() { return sizeof...(Args); }

  typedef std::tuple<Args...> types;
  std::array<const char*, sizeof...(Args)> _names;
};

template <typename... Args>
struct outs
{
  outs(typename _detail::const_char_type<Args>::type... names) : _names({{names...}}) {}

  static constexpr std::size_t size() { return sizeof...(Args); }

  typedef std::tuple<Args...> types;
  std::array<const char*, sizeof...(Args)> _names;
};

void foo(int);

template <typename F, typename Ins, typename Outs>
method_specification<F, Ins, Outs>
method(const char* name, F f, Ins ins, Outs outs)
{
  return method_specification<F, Ins, Outs>
    {name, f, ins, outs}
  ;
}

template <typename F, typename... Outs>
method_specification<F, service::ins<>, service::outs<Outs...> >
method(const char* name, F f, service::outs<Outs...> outs)
{
  return method_specification<F, service::ins<>, service::outs<Outs...> >
    {name, f, service::ins<>(), outs}
  ;
}

template <typename F, typename... Ins>
method_specification<F, service::ins<Ins...>, service::outs<> >
method(const char* name, F f, service::ins<Ins...> ins)
{
  return method_specification<F, service::ins<Ins...>, service::outs<> >
    {name, f, ins, service::outs<>()}
  ;
}

}

template <std::size_t I, std::size_t N, template <typename...> class params
          , typename ... Args>
int _fill_method_impl(std::array<Eldbus_Arg_Info, N>& array
                      , params<Args...> const&
                      , std::true_type)
{
  array[I] = Eldbus_Arg_Info {0, 0};
  return 0;
}


template <std::size_t I, std::size_t N, template <typename...> class params
          , typename ... Args>
int _fill_method_impl(std::array<Eldbus_Arg_Info, N>& array
                      , params<Args...> const& directional_methods
                      , std::false_type)
{
  typedef std::tuple<Args...> params_type;
  static const char sig[2]
    = {_detail::signature_traits
       <
         typename std::tuple_element<I, params_type>::type
       >::sig
       , '\0'
      };
  array[I] = {sig
              , directional_methods._names[I]
             };
  return 0;
}


template <std::size_t I, std::size_t N, template <typename...> class params
          , typename ... Args>
int _fill_method(std::array<Eldbus_Arg_Info, N>& array
                 , params<Args...> const& directional_methods)
{
  _fill_method_impl<I>(array, directional_methods
                       , std::integral_constant<bool, N-1 == I>());
  return 0;
}

template <typename... T>
void _foo(T...) {}

template <std::size_t N, template <typename...> class params, typename ... Args
          , std::size_t... Seq>
void _fill_methods_impl(std::array<Eldbus_Arg_Info, N>& array
                        , params<Args...> const& directional_methods
                        , eina::index_sequence<Seq...>
                        )
{
  eldbus::_foo(eldbus::_fill_method<Seq>(array, directional_methods) ...);
}

template <std::size_t N, template <typename...> class params, typename ... Args>
void _fill_methods(std::array<Eldbus_Arg_Info, N>& array
                   , params<Args...> const& directional_methods)
{
  eldbus::_fill_methods_impl(array, directional_methods
                             , eina::make_index_sequence<N>());
}

template <typename Tuple, std::size_t I>
void _create_methods_specification(Tuple const&
                                   , std::array<Eldbus_Method2, std::tuple_size<Tuple>::value+1>& methods
                                   , std::integral_constant<std::size_t, I>
                                   , std::true_type)
{
  methods[I] = Eldbus_Method2 {0, 0, 0, 0, 0, 0};
};

template <std::size_t, typename T>
T get_in(Eldbus_Message const* msg)
{
  typename _detail::signature_traits<T>::raw_type object;
  
  const char sig[2] = {_detail::signature_traits<T>::sig, 0};
  if (!eldbus_message_arguments_get(msg, sig, &object))
    {
      printf("eldbus_message_arguments_get() error\n");
      throw std::runtime_error("");
    }
  else
    {
      return object;
    }
}

template <typename Callable, typename... Ins, typename... Outs
          , std::size_t... IndexIns, std::size_t... IndexOuts
          , std::size_t... IndexOuts2>
Eldbus_Message* _method_callback_call
(Callable const& callable, service::ins<Ins...> const&, service::outs<Outs...> const&
 , eina::index_sequence<IndexIns...>, eina::index_sequence<IndexOuts...>
 , eina::index_sequence<IndexOuts2...>
 , Eldbus_Service_Interface const* iface, Eldbus_Message const* message
 , typename std::enable_if
 <!std::is_void
 <decltype(callable
           (*static_cast<eldbus::message*>(nullptr)
            , *static_cast<eldbus::service_interface*>(nullptr)
            , std::get<IndexIns>(std::tuple<Ins...>())...
            , &std::get<IndexOuts2>
            (*static_cast<std::tuple<Outs...>*>(nullptr))...
           )
          )>::value, int>::type)
{
  typedef std::tuple<Ins...> tuple_ins;
  typename _detail::raw_tuple<tuple_ins>::type tuple_ins_raw;
  _detail::_init_raw_tuple<0u, tuple_ins>
    (eldbus_message_iter_get(message), tuple_ins_raw
     , std::integral_constant<bool, (std::tuple_size<tuple_ins>::value == 0u)>());

  std::tuple<Outs...> tuple_outs;

  eldbus::const_message msg( ::eldbus_message_ref(const_cast<Eldbus_Message*>(message)));
  eldbus::service_interface siface(iface);
  
  // If you have an error here, then you probably got your ins<...>
  // and/or outs<...> different from the arguments of your callable
  //typedef _detail::signature_traits<result_type> traits;
  std::get<0u>(tuple_outs) = 
    callable(msg, siface, std::get<IndexIns>(tuple_ins_raw)..., &std::get<IndexOuts2>(tuple_outs)...);

  Eldbus_Message *reply_ = eldbus_message_method_return_new(message);
  _detail::_append_tuple<0u>(reply_, tuple_outs, std::false_type());
  return reply_;
}

template <typename Callable, typename... Ins, typename... Outs
          , std::size_t... IndexIns, std::size_t... IndexOuts
          , std::size_t... IndexOuts2>
Eldbus_Message* _method_callback_call
(Callable const& callable, service::ins<Ins...> const&, service::outs<Outs...> const&
 , eina::index_sequence<IndexIns...>, eina::index_sequence<IndexOuts...> 
 , eina::index_sequence<IndexOuts2...>
 , Eldbus_Service_Interface const* iface, Eldbus_Message const* message
 , long)
{
  typedef std::tuple<Ins...> tuple_ins;
  typename _detail::raw_tuple<tuple_ins>::type tuple_ins_raw;
  _detail::_init_raw_tuple<0u, tuple_ins>
    (eldbus_message_iter_get(message), tuple_ins_raw
     , std::integral_constant<bool, (std::tuple_size<tuple_ins>::value == 0u)>());

  typedef std::tuple<Outs...> tuple_outs_type;
  tuple_outs_type tuple_outs;

  eldbus::const_message msg( ::eldbus_message_ref(const_cast<Eldbus_Message*>(message)));
  eldbus::service_interface siface(iface);

  // If you have an error here, then you probably got your ins<...>
  // and/or outs<...> different from the arguments of your callable
  callable(msg, siface, std::get<IndexIns>(tuple_ins_raw)..., &std::get<IndexOuts>(tuple_outs)...);

  Eldbus_Message *reply_ = eldbus_message_method_return_new(message);
  _detail::_append_tuple<0u>
    (reply_, tuple_outs
     , std::integral_constant<bool, std::tuple_size<tuple_outs_type>::value == 0u>());
  return reply_;
}

template <typename Callable, typename Ins, typename Outs>
Eldbus_Message* _method_callback(void* data, Eldbus_Service_Interface const* iface, Eldbus_Message const* msg)
{
  std::tuple<Callable, Ins, Outs>*
    tuple = static_cast<std::tuple<Callable, Ins, Outs>*>(data);

  return _method_callback_call(std::get<0u>(*tuple), std::get<1u>(*tuple), std::get<2u>(*tuple)
                               , eina::make_index_sequence
                               <std::tuple_size<typename Ins::types>::value>()
                               , eina::make_index_sequence
                               <std::tuple_size<typename Outs::types>::value>()
                               , eina::pop_integer_sequence
                               <eina::make_index_sequence<0u>
                               , eina::make_index_sequence
                               <std::tuple_size<typename Outs::types>::value>
                               >()
                               , iface, msg, 0);
}

template <typename Method>
void _create_methods_specification_impl(Method const& method, Eldbus_Method2& eldbus_method, std::false_type)
{
  std::array<Eldbus_Arg_Info, Method::ins_type::size()+1>*
    in_params = new std::array<Eldbus_Arg_Info, Method::ins_type::size()+1>;

  eldbus::_fill_methods(*in_params, method.ins);

  std::array<Eldbus_Arg_Info, Method::outs_type::size()+1>*
    out_params = new std::array<Eldbus_Arg_Info, Method::outs_type::size()+1>;

  eldbus::_fill_methods(*out_params, method.outs);

  eldbus_method = {method.name, &(*in_params)[0], &(*out_params)[0]
                   , reinterpret_cast<Eldbus_Method_Cb>
                    (static_cast<Eldbus_Method_Data_Cb>
                     (&_method_callback<typename Method::function_type
                      , typename Method::ins_type, typename Method::outs_type>))
                   , ELDBUS_METHOD_FLAG_HAS_DATA
                   , new std::tuple<typename Method::function_type
                   , typename Method::ins_type, typename Method::outs_type
                   >(std::move(method.f), method.ins, method.outs)};
}

template <typename Method>
void _create_methods_specification_impl(Method const& method, Eldbus_Method2& eldbus_method, std::true_type)
{
  std::array<Eldbus_Arg_Info, Method::ins_type::size()+1>*
    in_params = new std::array<Eldbus_Arg_Info, Method::ins_type::size()+1>;

  eldbus::_fill_methods(*in_params, method.ins);

  std::array<Eldbus_Arg_Info, Method::outs_type::size()+1>*
    out_params = new std::array<Eldbus_Arg_Info, Method::outs_type::size()+1>;

  eldbus::_fill_methods(*out_params, method.outs);

  eldbus_method = {method.name, &(*in_params)[0], &(*out_params)[0], method.f, 0, 0};
}

template <typename Tuple, std::size_t I>
void _create_methods_specification(Tuple const& tuple
                                   , std::array<Eldbus_Method2, std::tuple_size<Tuple>::value+1>& methods
                                   , std::integral_constant<std::size_t, I>
                                   , std::false_type)
{
  typedef Eldbus_Message*(*function_type)(Eldbus_Service_Interface const*, Eldbus_Message const*);

  typedef typename std::tuple_element<I, Tuple>::type method_type;

  _create_methods_specification_impl(std::get<I>(tuple), methods[I]
                                     , std::is_convertible<typename method_type::function_type
                                     , function_type>());

  return _create_methods_specification(tuple, methods
                                       , std::integral_constant<std::size_t, I+1>()
                                       , std::integral_constant<bool, I+1 == std::tuple_size<Tuple>::value>());
};

template <typename Tuple>
std::array<Eldbus_Method2, std::tuple_size<Tuple>::value+1> _create_methods_specification
  (Tuple const& tuple
   )
{
  typedef std::tuple_size<Tuple> tuple_size;
  std::array<Eldbus_Method2, tuple_size::value+1> array;

  _create_methods_specification(tuple, array
                                , std::integral_constant<std::size_t, 0u>()
                                , std::integral_constant<bool, 0 == std::tuple_size<Tuple>::value>());
  return array;
};

template <typename... Args>
service_interface service_interface_register(connection& c, const char* path
                                             , const char* interface
                                             , Args... args
                                             )
{
  std::array<Eldbus_Method2, sizeof...(Args) + 1>* methods
    = new std::array<Eldbus_Method2, sizeof...(Args) + 1>
    (
     _create_methods_specification(std::make_tuple(args...))
    );
  Eldbus_Service_Interface_Desc2 description =
    {
      {interface, 0, 0, 0, 0, 0}, ELDBUS_INTERFACE_DESCRIPTOR_VERSION, &(*methods)[0]
    };

  Eldbus_Service_Interface* iface
    = ::eldbus_service_interface_register2(c.native_handle(), path, &description);
  return service_interface(iface);
}

} }

#endif
