
///
/// @file eo_event.hh
///

#ifndef EFL_CXX_EO_EVENT_HH
#define EFL_CXX_EO_EVENT_HH

#include <Eo.h>

#include <functional>
#include <memory>

namespace efl { namespace eo {

typedef ::Eo_Callback_Priority callback_priority;
namespace callback_priorities
{
static const callback_priority before = -100;
static const callback_priority default_ = 0;
static const callback_priority  after = 100;
}

struct signal_connection
{
  signal_connection(std::function<void()> deleter)
    : _deleter(deleter) {}
  void disconnect()
  {
    if(_deleter)
      {
        _deleter();
        _deleter = std::function<void()>();
      }
  }
private:
  std::function<void()> _deleter;
  friend struct scoped_signal_connection;
};

struct scoped_signal_connection
{
  scoped_signal_connection(signal_connection const& other)
    : _deleter(other._deleter)
  {
  }
  ~scoped_signal_connection()
  {
    disconnect();
  }
  void disconnect()
  {
    if(_deleter)
      {
        _deleter();
        _deleter = std::function<void()>();
      }
  }
  scoped_signal_connection(scoped_signal_connection&& other)
    : _deleter(other._deleter)
  {
    other._deleter = std::function<void()>();
  }
private:
  std::function<void()> _deleter;

  scoped_signal_connection& operator=(scoped_signal_connection const&) = delete;
  scoped_signal_connection(scoped_signal_connection const&) = delete;
};

template <typename F>
struct _event_deleter
{
  _event_deleter(F* data, Eo* eo, ::Eo_Event_Cb cb, Eo_Event_Description const* description)
    : _data(data), _eo( ::eo_ref(eo)), _cb(cb), _description(description)
  {
  }
  ~_event_deleter()
  {
    ::eo_unref(_eo);
  }
  _event_deleter(_event_deleter const& other)
    : _data(other._data), _eo( ::eo_ref(other._eo)), _cb(other._cb), _description(other._description)
  {}
  _event_deleter& operator=(_event_deleter const& other)
  {
    ::eo_unref( _eo);
    _data = other._data;
    _eo = ::eo_ref(other._eo);
    _cb = other._cb;
    _description = other._description;
    return *this;
  }

  void operator()() const
  {
    eo_do(_eo, ::eo_event_callback_del(_description, _cb, _data));
    delete _data;
  }

  F* _data;
  Eo* _eo;
  ::Eo_Event_Cb _cb;
  Eo_Event_Description const* _description;
};

template <typename F>
signal_connection make_signal_connection(std::unique_ptr<F>& data, Eo* eo, ::Eo_Event_Cb cb, Eo_Event_Description const* description)
{
  signal_connection c(_event_deleter<F>(data.get(), eo, cb, description));
  data.release();
  return std::move(c);
}

namespace _detail {

template <typename T, typename F>
Eina_Bool really_call_event(T& wrapper, F& f, Eo_Event_Description const& desc, void *info
                            , std::true_type)
{
   f(wrapper, desc, info);
   return true;
}
template <typename T, typename F>
Eina_Bool really_call_event(T& wrapper, F& f, Eo_Event_Description const& desc, void *info
                            , std::false_type)
{
   return f(wrapper, desc, info);
}

template <typename T, typename F>
Eina_Bool
event_callback(void *data, Eo *obj, Eo_Event_Description const* desc, void *info)
{
   T wrapper(::eo_ref(obj));
   F *f = static_cast<F*>(data);
   return _detail::really_call_event(wrapper, *f, *desc, info
                                     , std::is_void<decltype((*f)(wrapper, *desc, info))>());
}

}

} }

#endif
