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

#include <climits>
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

namespace _detail {

/* What every handle on one connection shares. Whoever cuts first (a handle,
   or the emitter itself being invalidated) runs the cutter once; every later
   attempt finds nothing to do. It holds no reference on the emitter. Not thread
   safe, like the rest of the binding: a connection is cut from the thread that
   runs the main loop. */
struct connection_link
{
  std::function<void()> cutter;

  void cut()
  {
    if (!cutter) return;
    std::function<void()> c;
    c.swap(cutter);
    c();
  }
};

}

/**
 * @brief A connection made by @ref event_add.
 *
 * The connection lives as long as the emitter is valid: when the emitter is
 * invalidated the callback registration goes and the closure is freed. A handle
 * is optional (drop it and nothing changes) and does not keep the emitter
 * alive. Handles are cheap to copy and any number of them, and of
 * @ref scoped_signal_connection made from them, may disconnect: the first
 * one wins, the others, and a disconnect after the emitter is gone, do
 * nothing.
 */
struct signal_connection
{
  signal_connection(std::function<void()> deleter)
    : _link(std::make_shared<_detail::connection_link>())
  {
    _link->cutter = deleter;
  }
  signal_connection(std::shared_ptr<_detail::connection_link> link)
    : _link(std::move(link)) {}
  void disconnect()
  {
    if(_link)
      _link->cut();
  }
private:
  std::shared_ptr<_detail::connection_link> _link;
  friend struct scoped_signal_connection;
};

struct scoped_signal_connection
{
  scoped_signal_connection(signal_connection const& other)
    : _link(other._link)
  {
  }
  ~scoped_signal_connection()
  {
    disconnect();
  }
  void disconnect()
  {
    if(_link)
      _link->cut();
  }
  scoped_signal_connection(scoped_signal_connection&& other)
    : _link(std::move(other._link))
  {
    other._link.reset();
  }
private:
  std::shared_ptr<_detail::connection_link> _link;

  scoped_signal_connection& operator=(scoped_signal_connection const&) = delete;
  scoped_signal_connection(scoped_signal_connection const&) = delete;
};

namespace _detail {

/* The closure is shared: the registration owns one reference, and every call
   in progress takes another for its duration. A handler may end its own
   emitter or disconnect its own handle, and the closure it is executing must
   survive that. The registration is what both of the Eo callbacks (the
   handler and the end-of-life hook) receive as data; whoever cuts the
   connection deletes it. */
template <typename F>
struct registration
{
  std::shared_ptr<F> closure;
  std::shared_ptr<connection_link> link;
};

/* The emitter is going: the connection is cut and its closure freed.
   Registered with the lowest priority, so every user handler of the same
   event (priority 0) has run first. Cutting unregisters, which is legal at
   any point of the teardown. */
template <typename F>
void emitter_going(void* data, ::Efl_Event const*)
{
   auto link = static_cast<registration<F>*>(data)->link;
   link->cut();
}

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
   /* Our own reference keeps the closure alive if the handler severs the
      connection (or deletes its emitter) while it runs. */
   std::shared_ptr<F> f = static_cast<registration<F>*>(data)->closure;
   _detail::really_call_event<T, P>
     (wrapper, *f, event->info, std::is_void<P>{});
}
}

namespace _detail {

template <typename Event, typename Object, typename F>
std::shared_ptr<connection_link> event_add_link(Event event, Object object, F&& function)
{
  static_assert((eo::is_eolian_object<Object>::value), "Type is not an object");

  typedef typename std::remove_reference<F>::type function_type;
  typedef registration<function_type> registration_type;
  std::unique_ptr<registration_type> r(new registration_type);
  r->closure.reset(new function_type(std::forward<F>(function)));
  r->link = std::make_shared<connection_link>();

  Eo* eo = object._eo_ptr();
  ::Efl_Event_Cb cb = static_cast<Efl_Event_Cb>
    (&event_callback<Object, typename Event::parameter_type, function_type>);
  /* Freed when the emitter is invalidated, i.e. when it loses its parent or
     its last reference, while it is still alive: a closure that captured its
     own emitter must not keep it alive past that. A connection to "del" or
     "destruct" is itself an end-of-life handler and must run, so it is freed
     at "destruct", the last event. */
  Efl_Event_Description const* description = event.description();
  bool late = (description == EFL_EVENT_DEL) || (description == EFL_EVENT_DESTRUCT);
  /* Past its invalidation (begun, from its own handler, or done, while other
     references keep it alive) the object calls no handler but del and
     destruct, and its invalidate event will not be emitted again to cut the
     connection. So the connection is not made: it comes back already cut and
     the closure is freed here. efl_invalidating_get stays set from the start
     of the invalidation; efl_invalidated_get would also answer for an object
     whose parent only is invalidated, which still gets its own event. */
  if (!late && ::efl_invalidating_get(eo))
    return r->link;
  Efl_Event_Description const* end = late ? EFL_EVENT_DESTRUCT : EFL_EVENT_INVALIDATE;
  ::Efl_Event_Cb gone = static_cast<Efl_Event_Cb>
    (&emitter_going<function_type>);
  registration_type* data = r.release();
  std::shared_ptr<connection_link> link = data->link;

  ::efl_event_callback_priority_add(eo, description, 0, cb, data);
  ::efl_event_callback_priority_add(eo, end, SHRT_MAX, gone, data);
  link->cutter = [eo, description, cb, end, gone, data]
    {
      ::efl_event_callback_del(eo, description, cb, data);
      ::efl_event_callback_del(eo, end, gone, data);
      /* Drops the registration's reference; a call in progress holds its own. */
      delete data;
    };
  return link;
}

}

/**
 * @brief Call @p function when @p object emits @p event.
 *
 * The connection lives as long as @p object is valid: when the emitter is
 * invalidated (it loses its parent, or its last reference) the registration
 * is removed and the closure is freed, so a lambda capturing state does not
 * leak it, and one capturing its own emitter does not keep it alive. The
 * closure is freed after the user handlers of Efl.Object's @c invalidate
 * (they run first), and, for a connection to @c del or @c destruct, at the
 * emitter's @c destruct, after those handlers.
 *
 * What is given up: after its invalidation an object no longer calls the
 * handlers connected through this function, except the ones for @c del and
 * @c destruct.
 *
 * An object that is already invalidating (the call comes from its own
 * @c invalidate handler) or invalidated but still referenced takes no new
 * connection but to @c del and @c destruct: the connection is not made, the
 * returned handle is already cut and the closure is freed at once.
 *
 * Two rules follow. An object with no parent whose handler captures it by
 * value (a strong wrapper) is a cycle: the closure holds a reference, so the
 * object is not invalidated until the connection is cut; capture an
 * @c efl::eo::wref, or use the object the handler is given as its first
 * argument. And a handler of @c del or @c destruct must use its argument, not
 * a captured wrapper of the emitter, which would be released while the
 * emitter is being destroyed.
 *
 * The returned @ref signal_connection is optional. It does not keep
 * @p object alive; keep it to cut the connection earlier, from any number of
 * copies (the first disconnect wins). Disconnecting after the emitter is
 * gone does nothing.
 *
 * A handler may disconnect its own connection, or delete its emitter, while
 * it runs: the closure outlives the call and is freed after it.
 */
template <typename Event, typename Object, typename F>
signal_connection event_add(Event event, Object object, F&& function)
{
  return signal_connection(_detail::event_add_link(event, object, std::forward<F>(function)));
}

} }

#endif
