#ifndef EFL_ELDBUS_CXX_ELDBUS_FREEDESKTOP_HH
#define EFL_ELDBUS_CXX_ELDBUS_FREEDESKTOP_HH

#include <eldbus_message.hh>
#include <eldbus_service.hh>
#include <eldbus_basic.hh>
#include <eldbus_raw_tuple.hh>

namespace efl { namespace eldbus { namespace _detail {

template <typename Callback, typename... Ins>
void _callback_wrapper(void* data, Eldbus_Message const* message, Eldbus_Pending* pending)
{
  std::cout << "_callback_wrapper" << std::endl;
  Callback* callback(static_cast<Callback*>(data));

  const char* errname, *errmsg;
  if (eldbus_message_error_get(message, &errname, &errmsg))
    {
      std::cout << "error " << errname << " " << errmsg << std::endl;
      std::tuple<Ins...> tuple;
      eldbus::const_message msg( ::eldbus_message_ref(const_cast<Eldbus_Message*>(message)));
      eina::error_code ec (eldbus::call_error_code(), eina::eina_error_category());
      eina::call_tuple_unwrap_prefix
        (*callback, tuple, eina::make_index_sequence<sizeof...(Ins)>()
         , ec, msg, pending);
    }

  typename raw_tuple<std::tuple<Ins...> >::type tuple;
  if(sizeof...(Ins))
    {
      if(!_detail::_init_raw_tuple<0u, std::tuple<Ins...> >
         (eldbus_message_iter_get(message)
          , tuple, std::integral_constant<bool, (sizeof...(Ins) == 0)>()))
        {
          std::cout << "error init raw tuple" << std::endl;
          std::tuple<Ins...> tuple_;
          eldbus::const_message msg( ::eldbus_message_ref(const_cast<Eldbus_Message*>(message)));
          eina::error_code ec(eldbus::signature_mismatch_error_code(), eina::eina_error_category());
          eina::call_tuple_unwrap_prefix
            (*callback, tuple_, eina::make_index_sequence<sizeof...(Ins)>()
             , ec, msg, pending);
        }
    }
  std::cout << "OK go" << std::endl;
  eina::error_code ec;
  eldbus::const_message msg( ::eldbus_message_ref(const_cast<Eldbus_Message*>(message)));
  eina::call_tuple_unwrap_prefix
    (*callback, tuple, eina::make_index_sequence<sizeof...(Ins)>()
     , ec, msg, pending);
}

template <typename F>
void _free_cb(void* data, const void*)
{
  delete static_cast<F*>(data);
}

}

template <typename... Ins, typename F>
pending name_request(connection& c, const char* bus, unsigned int flags, F&& function)
{
  F* f = new F(std::move(function));
  pending r = ::eldbus_name_request(c.native_handle(), bus, flags
                                    , &_detail::_callback_wrapper<F, Ins...>, f);
  eldbus_pending_free_cb_add(r.native_handle(), &_detail::_free_cb<F>, f);
  return r;
}

} }

#endif
