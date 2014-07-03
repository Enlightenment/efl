#ifndef ELDBUS_CXX_ELDBUS_MESSAGE_HH
#define ELDBUS_CXX_ELDBUS_MESSAGE_HH

#include <Eldbus.h>
#include <eldbus_signature_traits.hh>

namespace efl { namespace eldbus {

struct message_value
{
  message_value() : _iterator(0) {}
  message_value( ::Eldbus_Message_Iter* iterator) : _iterator(iterator) {}

  template <typename T>
  T get() const
  {
    assert(!!_iterator);
    std::cout << "trying to read " << _detail::signature_traits<T>::sig
              << " has " << eldbus_message_iter_signature_get(_iterator) << std::endl;
    T object;
    char* c = eldbus_message_iter_signature_get(_iterator);
    if(c[0] != _detail::signature_traits<T>::sig)
      throw std::runtime_error("");
    eldbus_message_iter_arguments_get(_iterator, c, &object);
    return object;
  }
  friend inline bool operator==(message_value const& lhs, message_value const& rhs)
  {
    return
      lhs._iterator == rhs._iterator
      || (!lhs._iterator && rhs._iterator && std::strlen(eldbus_message_iter_signature_get(rhs._iterator)) == 0)
      || (!rhs._iterator && lhs._iterator && std::strlen(eldbus_message_iter_signature_get(lhs._iterator)) == 0)
      ;
  }
private:
  friend struct message_iterator;
  ::Eldbus_Message_Iter* _iterator;
};

inline bool operator!=(message_value const& lhs, message_value const& rhs)
{
  return !(lhs == rhs);
}

struct message_iterator
{
  typedef message_value value_type;
  typedef value_type& reference;
  typedef value_type const& const_reference;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef std::ptrdiff_t difference_type;
  typedef std::size_t size_type;
  typedef std::input_iterator_tag iterator_category;

  message_iterator()
  {
  }

  message_iterator(Eldbus_Message_Iter* iterator)
    : value(iterator)
  {}

  reference operator*() const
  {
    return const_cast<reference>(value);
  }
  pointer operator->() const
  {
    return const_cast<pointer>(&value);
  }
  message_iterator& operator++()
  {
    if(!eldbus_message_iter_next(value._iterator))
      {
        value._iterator = 0;
      }
    return *this;
  }
  message_iterator operator++() const
  {
    message_iterator tmp(*this);
    ++*this;
    return tmp;
  }
  friend inline bool operator==(message_iterator const& lhs, message_iterator const& rhs)
  {
    return lhs.value == rhs.value;
  }
private:
  message_value value;
};

inline bool operator!=(message_iterator const& lhs, message_iterator const& rhs)
{
  return !(lhs == rhs);
}

struct reply_t {};
struct error_t {};
struct method_call_t {};

reply_t const reply = {};
error_t const error = {};
method_call_t const method_call = {};

struct const_message
{
  explicit const_message(Eldbus_Message const* message) : _message(message)
  {
  }
  const_message(const_message const& other, reply_t)
    : _message( ::eldbus_message_method_return_new(other.native_handle()))
  {
  }
  const_message(const_message const& other, const char* error_name, const char* error_msg, error_t)
    : _message( ::eldbus_message_error_new(other.native_handle(), error_name, error_msg))
  {
  }
  const_message(const char* destination, const char* path, const char* iface
          , const char* method, method_call_t)
    : _message( ::eldbus_message_method_call_new(destination, path, iface, method))
  {
  }
  const_message(const_message const& other)
    : _message( ::eldbus_message_ref(const_cast<Eldbus_Message*>(other.native_handle())))
  {
  }
  ~const_message()
  {
    eldbus_message_unref(const_cast<Eldbus_Message*>(native_handle()));
  }

  const char* path_get() const
  {
    return ::eldbus_message_path_get(_message);
  }
  const char* interface_get() const
  {
    return ::eldbus_message_interface_get(_message);
  }
  const char* member_get() const
  {
    return ::eldbus_message_member_get(_message);
  }
  const char* destination_get() const
  {
    return ::eldbus_message_destination_get(_message);
  }
  const char* sender_get() const
  {
    return ::eldbus_message_sender_get(_message);
  }
  const char* signature_get() const
  {
    return ::eldbus_message_signature_get(_message);
  }
  bool operator!() const
  {
    bool b (*this);
    return !b;
  }
  explicit operator bool() const
  {
    const char* name, * text;
    return !eldbus_message_error_get(_message, &name, &text);
  }
  std::pair<const char*, const char*> error_get() const
  {
    const char* name, * text;
    eldbus_message_error_get(_message, &name, &text);
    return {name, text};
  }

  typedef message_iterator const_iterator;
  typedef const_iterator iterator;
  typedef Eldbus_Message const* native_handle_type;
  typedef Eldbus_Message const* const_native_handle_type;
  const_native_handle_type native_handle() const { return _message; }
  const_native_handle_type const_native_handle() const { return _message; }

  const_iterator begin() const
  {
    return const_iterator( ::eldbus_message_iter_get(_message));
  }
  const_iterator end() const
  {
    return const_iterator();
  }
private:
  Eldbus_Message const* _message;
};

struct message : const_message
{
  typedef const_message base_type;

  explicit message(Eldbus_Message* message) : const_message(message)
  {
  }
  message(const_message const& other, reply_t r)
    : const_message(other, r)
  {
  }
  message(const_message const& other, const char* error_name, const char* error_msg, error_t e)
    : const_message(other, error_name, error_msg, e)
  {
  }
  message(const char* destination, const char* path, const char* iface
          , const char* method, method_call_t m)
    : const_message(destination, path, iface, method, m)
  {
  }
  message(message const& other)
    : const_message(static_cast<base_type const&>(other))
  {
  }
  typedef Eldbus_Message* native_handle_type;
  native_handle_type native_handle() { return const_cast<native_handle_type>(const_message::native_handle()); }
};

struct const_pending
{
  const_pending() : _pending(0) {}
  const_pending(Eldbus_Pending const* pending) : _pending(pending) {}

  void* data_get(const char* key) const
  {
    return ::eldbus_pending_data_get(_pending, key);
  }
  const char* destination_get() const
  {
    return ::eldbus_pending_destination_get(_pending);
  }
  const char* path_get() const
  {
    return ::eldbus_pending_path_get(_pending);
  }
  const char* interface_get() const
  {
    return ::eldbus_pending_interface_get(_pending);
  }
  const char* method_get() const
  {
    return ::eldbus_pending_method_get(_pending);
  }

  typedef Eldbus_Pending const* native_handle_type;
  typedef Eldbus_Pending const* const_native_handle_type;
  native_handle_type native_handle() const { return _pending; }
  const_native_handle_type const_native_handle() const { return _pending; }
private:
  const_native_handle_type _pending;
};

struct pending : const_pending
{
  pending() : const_pending(0) {}
  pending(Eldbus_Pending* pending) : const_pending(pending) {}

  void data_set(const char* key, const char* data)
  {
    ::eldbus_pending_data_set(native_handle(), key, data);
  }
  void* data_del(const char* key)
  {
    return ::eldbus_pending_data_del(native_handle(), key);
  }
  void cancel()
  {
    ::eldbus_pending_cancel(native_handle());
  }

  typedef Eldbus_Pending* native_handle_type;
  native_handle_type native_handle()
  {
    return const_cast<native_handle_type>(const_pending::native_handle());
  }
};

} }

#endif
