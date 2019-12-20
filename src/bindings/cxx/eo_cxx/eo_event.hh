/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

///
/// @file eo_event.hh
///

#ifndef EFL_CXX_EO_EVENT_HH
#define EFL_CXX_EO_EVENT_HH

#include <Eo.h>
#include <Ecore.h>
#include "eo_concrete.hh"
#include "eo_cxx_interop.hh"

#include <functional>
#include <memory>

namespace efl { namespace eolian {

 typedef ::Efl_Callback_Priority callback_priority;
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
  _event_deleter(F* data, Eo* eo, ::Efl_Event_Cb cb, Efl_Event_Description const* description)
    : _data(data), _eo( ::efl_ref(eo)), _cb(cb), _description(description)
  {
  }
  ~_event_deleter()
  {
    ::efl_unref(_eo);
  }
  _event_deleter(_event_deleter const& other)
    : _data(other._data), _eo( ::efl_ref(other._eo)), _cb(other._cb), _description(other._description)
  {}
  _event_deleter& operator=(_event_deleter const& other)
  {
    ::efl_unref( _eo);
    _data = other._data;
    _eo = ::efl_ref(other._eo);
    _cb = other._cb;
    _description = other._description;
    return *this;
  }

  void operator()() const
  {
    ::efl_event_callback_del(_eo, _description, _cb, _data);
    ::ecore_main_loop_thread_safe_call_async(&_deleter_call, _data);
  }

private:
  static void _deleter_call(void* data)
  {
    delete static_cast<F*>(data);
  }

  F* _data;
  Eo* _eo;
  ::Efl_Event_Cb _cb;
  Efl_Event_Description const* _description;
};

template <typename F>
signal_connection make_signal_connection(std::unique_ptr<F>& data, Eo* eo, ::Efl_Event_Cb cb, Efl_Event_Description const* description)
{
  signal_connection c(_event_deleter<F>(data.get(), eo, cb, description));
  data.release();
  return c;
}

namespace _detail {

template <typename T, typename P, typename F>
void really_call_event(T& wrapper, F& f, void *, std::true_type)
{
   f(wrapper);
}
template <typename T, typename P, typename F>
void really_call_event(T& wrapper, F& f, void *info, std::false_type)
{
   f(wrapper, convert_to_event<P>(info));
}

template <typename T, typename P, typename F>
void event_callback(void *data, ::Efl_Event const* event)
{
   T wrapper(::efl_ref(event->object));
   F *f = static_cast<F*>(data);
   _detail::really_call_event<T, P>
     (wrapper, *f, event->info, std::is_void<P>{});
}
}

template <typename Event, typename Object, typename F>
signal_connection event_add(Event event, Object object, F&& function)
{
  static_assert((eo::is_eolian_object<Object>::value), "Type is not an object");

  typedef typename std::remove_reference<F>::type function_type;
  std::unique_ptr<function_type> f(new function_type(std::forward<F>(function)));

  ::efl_event_callback_priority_add
      (object._eo_ptr(), event.description(), 0
       , static_cast<Efl_Event_Cb>
       (&_detail::event_callback<Object, typename Event::parameter_type, function_type>)
       , f.get());
  return make_signal_connection
    (f, object._eo_ptr()
     , static_cast<Efl_Event_Cb>
     (&_detail::event_callback<Object, typename Event::parameter_type, function_type>)
     , event.description());
}

} }

#endif
