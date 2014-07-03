#ifndef ELDBUS_PROXY_CALL_HH
#define ELDBUS_PROXY_CALL_HH

#include <Eina.hh>
#include <eldbus_error.hh>
#include <eldbus_signature_traits.hh>
#include <eldbus_message_arguments.hh>
#include <eldbus_message.hh>
#include <eldbus_raw_tuple.hh>
#include <eina_integer_sequence.hh>
#include <eina_tuple_unwrap.hh>

#include <tuple>

#include <iostream>

namespace efl { namespace eldbus { namespace _detail {

template <typename T> struct tag {};

template <typename Seq, typename Callback, std::size_t... I>
void _on_call_impl(void* data, Eldbus_Message const* message, Eldbus_Pending* pending)
{
  std::unique_ptr<Callback> callback(static_cast<Callback*>(data));

  const char* errname, *errmsg;
  if (eldbus_message_error_get(message, &errname, &errmsg))
    {
      Seq tuple;
      ::eldbus_message_ref(const_cast<Eldbus_Message*>(message));
      eldbus::const_message msg(message);
      eina::error_code ec (eldbus::call_error_code(), eina::eina_error_category());
      eina::call_tuple_unwrap_prefix
        (*callback, tuple, eina::make_index_sequence<std::tuple_size<Seq>::value>()
         , ec, msg, pending);
    }

  typename raw_tuple<Seq>::type tuple;
  if(std::tuple_size<Seq>::value)
    {
      if(!_detail::_init_raw_tuple<0u, Seq>
         (eldbus_message_iter_get(message)
          , tuple, std::integral_constant<bool, (std::tuple_size<Seq>::value == 0)>()))
        {
          Seq tuple;
          ::eldbus_message_ref(const_cast<Eldbus_Message*>(message));
          eldbus::const_message msg(message);
          eina::error_code ec(eldbus::signature_mismatch_error_code(), eina::eina_error_category());
          eina::call_tuple_unwrap_prefix
            (*callback, tuple, eina::make_index_sequence<std::tuple_size<Seq>::value>()
             , ec, msg, pending);
        }
    }
  eina::error_code ec;
  ::eldbus_message_ref(const_cast<Eldbus_Message*>(message));
  eldbus::const_message msg(message);
  eina::call_tuple_unwrap_prefix
    (*callback, tuple, eina::make_index_sequence<std::tuple_size<Seq>::value>()
     , ec, msg, pending);
}

template <typename Seq, typename Callback>
void _on_call(void* data, Eldbus_Message const* message, Eldbus_Pending* pending)
{
  _detail::_on_call_impl<Seq, Callback>(data, message, pending);
}

template <typename R, typename Callback, typename... Args>
void proxy_call_impl2(Eldbus_Proxy* proxy, const char* method, double timeout
                     , Callback&& callback, Args const&... args)
{
  typedef std::tuple<Args...> tuple_args;
  char signature[signature_size<tuple_args>::value +1];
  _detail::init_signature_array<Args...>
    (signature, eina::make_index_sequence<signature_size<tuple_args>::value +1>());

  Callback* c = new Callback(std::move(callback));

  eldbus_proxy_call(proxy, method, &_on_call<R, Callback>, c, timeout, signature
                    , _detail::to_raw(args)...);
}

template <typename R, typename Callback, typename... Args>
void proxy_call_impl(tag<R>, Eldbus_Proxy* proxy, const char* method, double timeout
                     , Callback&& callback, Args const&... args)
{
  typedef std::tuple<R> reply_tuple;
  _detail::proxy_call_impl2<reply_tuple>(proxy, method, timeout, std::move(callback), args...);
}

template <typename... R, typename Callback, typename... Args>
void proxy_call_impl(tag<std::tuple<R...> >, Eldbus_Proxy* proxy, const char* method, double timeout
                     , Callback&& callback, Args const&... args)
{
  typedef std::tuple<R...> reply_tuple;
  _detail::proxy_call_impl2<reply_tuple>(proxy, method, timeout, std::move(callback), args...);
}

template <typename Callback, typename... Args>
void proxy_call_impl(tag<void>, Eldbus_Proxy* proxy, const char* method, double timeout
                     , Callback&& callback, Args const&... args)
{
  typedef std::tuple<> reply_tuple;
  _detail::proxy_call_impl2<reply_tuple>(proxy, method, timeout, std::move(callback), args...);
}

template <typename R, typename Callback, typename... Args>
void proxy_call(Eldbus_Proxy* proxy, const char* method, double timeout
                , Callback&& callback, Args const&... args)
{
  return proxy_call_impl(tag<R>(), proxy, method, timeout, std::move(callback), args...);
}

} } }

#endif

