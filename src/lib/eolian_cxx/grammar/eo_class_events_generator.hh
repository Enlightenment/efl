
#ifndef EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH

#include <iosfwd>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"

namespace efl { namespace eolian { namespace grammar {

struct event_callback_add
{
   eo_event const& _event;
   eo_class const& _cls;
   event_callback_add(eo_event const& event, eo_class const& cls)
     : _event(event), _cls(cls)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, event_callback_add const& x)
{
   out << tab(1) << "template <typename F>" << endl
       << tab(1) << "::efl::eo::signal_connection" << endl
       << tab(1) << "event_" << x._event.name << "_callback_add(F && callback_," << endl
       << tab(11) << "::efl::eo::callback_priority priority_ =" << endl
       << tab(11) << "::efl::eo::callback_priorities::default_)" << endl
       << tab(1) << "{" << endl
       << tab(2) << "::std::unique_ptr<F> f ( new F(std::move(callback_)) );" << endl
       << tab(2) << "eo_do(_eo_ptr()," << endl
       << tab(4) << "eo_event_callback_priority_add" << endl
       << tab(4) << "(" << x._event.eo_name << ", priority_," << endl
       << tab(4) << "&efl::eo::_detail::event_callback<" << x._cls.name << ", F>, f.get()));" << endl
       << tab(2) << "return ::efl::eo::make_signal_connection" << endl
       << tab(3) << "(f, this->_eo_ptr(), &efl::eo::_detail::event_callback<" << x._cls.name << ", F>," << endl
       << tab(3) << x._event.eo_name << " );" << endl
       << tab(1) << "}" << endl;
   return out;
}

struct event_callback_call
{
   eo_event const& _event;
   event_callback_call(eo_event const& event)
     : _event(event)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, event_callback_call const& x)
{
   out << tab(1) << "template <typename T>" << endl
       << tab(1) << "void" << endl
       << tab(1) << "event_" << x._event.name << "_callback_call(T* info)" << endl
       << tab(1) << "{" << endl
       << tab(2) << "eo_do(_eo_ptr(), eo_event_callback_call" << endl
       << tab(4) << "(" << x._event.eo_name << ", info));" << endl
       << tab(1) << "}" << endl;
   return out;
}

struct events
{
   eo_class const& _cls;
   events(eo_class const& cls) : _cls(cls) {}
};

inline std::ostream&
operator<<(std::ostream& out, events const& x)
{
   for (eo_event const& e : x._cls.events)
     {
        out << event_callback_add(e, x._cls) << endl
            << event_callback_call(e);
     }
   return out;
}

} } }

#endif // EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH
