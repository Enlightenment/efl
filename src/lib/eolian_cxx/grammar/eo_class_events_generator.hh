
#ifndef EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH

#include <iosfwd>

#include <algorithm>  // std::transform()
#include <ctype.h>    // ::toupper()

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"
#include "parameters_generator.hh"

namespace efl { namespace eolian { namespace grammar {

struct event_callback_add
{
   eo_event const& _event;
   std::string const& _ev_name;
   event_callback_add(eo_event const& event, std::string const& ev_name)
     : _event(event),
       _ev_name(ev_name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, event_callback_add const& x)
{
   // TODO implement
   return out;
}

struct event_callback_del
{
   eo_event const& _event;
   std::string const& _ev_name;
   event_callback_del(eo_event const& event, std::string const& ev_name)
     : _event(event),
       _ev_name(ev_name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, event_callback_del const& x)
{
   // TODO: implement
   return out;
}

struct event_callback_call
{
   eo_event const& _event;
   std::string const& _ev_name;
   event_callback_call(eo_event const& event, std::string const& ev_name)
     : _event(event),
       _ev_name(ev_name)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, event_callback_call const& x)
{
   // TODO implement
   return out;
}

struct events
{
   events_container_type const& _events;
   events(events_container_type const& events) : _events(events) {}
};

inline std::ostream&
operator<<(std::ostream& out, events const& x)
{
   std::string prefix = x._cls.name + "_EVENT_";
   std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper());
   events_container_type::const_iterator it,
     first = x._events.begin(),
     last = x._events.end();
   for (it = first; it != last; ++it)
     {
        std::string ev_name = (*it).name;
        std::transform(ev_name.begin(), ev_name.end(), ev_name.begin(), ::toupper());
        out << event_callback_add((*it), prefix+ev_name)
            << event_callback_del((*it), prefix+ev_name)
            << event_callback_call((*it), prefix+ev_name);
     }
   return out;
}

} } }

#endif // EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH
