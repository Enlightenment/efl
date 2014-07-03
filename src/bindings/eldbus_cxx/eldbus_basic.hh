#ifndef ELDBUS_CXX_ELDBUS_BASIC_HH
#define ELDBUS_CXX_ELDBUS_BASIC_HH

#include <Eina.hh>
#include <Eldbus.h>

#include <eldbus_proxy_call.hh>

namespace efl { namespace eldbus {

struct unknown_t {};
struct session_t {};
struct system_t {};
struct start_t {};
struct address_t {};

unknown_t const unknown = unknown_t();
session_t const session = session_t();
system_t const system = system_t();
start_t const start = start_t();
address_t const address = address_t();

struct eldbus_init
{
  eldbus_init()
  {
    ::eldbus_init();
  }
  ~eldbus_init()
  {
    ::eldbus_shutdown();
  }
};

struct proxy
{
  typedef Eldbus_Proxy* native_handle_type;
  typedef Eldbus_Proxy const* const_native_handle_type;

  proxy(native_handle_type native)
    : _proxy(native) {}

  template <typename R, typename Callback, typename... Args>
  void call(const char* method, double timeout, Callback&& callback, Args... args) const
  {
    eldbus::_detail::proxy_call<R>(_proxy, method, timeout, std::move(callback), args...);
  }

  template <typename Callback, typename... Args>
  void call(const char* method, double timeout, Callback&& callback, Args... args) const
  {
    eldbus::_detail::proxy_call<void>(_proxy, method, timeout, std::move(callback), args...);
  }

  native_handle_type native_handle() { return _proxy; }
  const_native_handle_type native_handle() const { return _proxy; }
private:
  native_handle_type _proxy;
};

struct object
{
  typedef Eldbus_Object* native_handle_type;
  typedef Eldbus_Object const* const_native_handle_type;
  object(native_handle_type o)
    : _object(o) {}

  proxy get_proxy(const char* interface_) const
  {
    Eldbus_Proxy* p = eldbus_proxy_get
      (const_cast<object*>(this)->native_handle(), interface_);
    if(!p)
      {
        eina::error_code ec = eina::get_error_code();
        assert(!!ec);
        throw eina::system_error(ec, "eldbus");
      }
    return proxy(p);
  }
  
  native_handle_type native_handle() { return _object; }
  const_native_handle_type const_native_handle() { return _object; }
private:
  native_handle_type _object;
};

struct connection
{
  typedef Eldbus_Connection* native_handle_type;
  typedef Eldbus_Connection const* const_native_handle_type;

  explicit connection(native_handle_type connection_)
    : _connection(connection_) {}
  ~connection()
  {
    eldbus_connection_unref(_connection);
  }
  connection(unknown_t)
    : _connection(eldbus_connection_get(ELDBUS_CONNECTION_TYPE_UNKNOWN))
  {
    _check_connection();
  }
  connection(session_t)
    : _connection(eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION))
  {
    _check_connection();
  }
  connection(system_t)
    : _connection(eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM))
  {
    _check_connection();
  }
  connection(address_t)
    : _connection(eldbus_connection_get(ELDBUS_CONNECTION_TYPE_ADDRESS))
  {
    _check_connection();
  }

  object get_object(const char* bus, const char* path) const
  {
    Eldbus_Object* o = eldbus_object_get
      (const_cast<connection*>(this)->native_handle(), bus, path);
    if(!o)
      {
        eina::error_code ec = eina::get_error_code();
        assert(!!ec);
        throw eina::system_error(ec, "eldbus");
      }
    return object(o);
  }
  
  native_handle_type native_handle() { return _connection; }
  const_native_handle_type native_handle() const { return _connection; }
private:
  void _check_connection()
  {
    if(!_connection)
      {
        eina::error_code ec = eina::get_error_code();
        assert(!!ec);
        throw eina::system_error(ec, "eldbus");
      }
  }

  native_handle_type _connection;
};

} }

#endif
