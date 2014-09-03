
#ifndef EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH
#define EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH

#include <iosfwd>

#include "eo_types.hh"
#include "tab.hh"
#include "comment.hh"

namespace efl { namespace eolian { namespace grammar {

struct add_cast_to_t
{
  add_cast_to_t(bool b)
    : _b(b)
  {
  }

  bool _b;
};

inline std::ostream&
operator<<(std::ostream& out, add_cast_to_t x)
{
  if(x._b)
    out << "static_cast<U*>(this)->";
  return out;
}
      
struct event_callback_add
{
   eo_event const& _event;
   eo_class const& _cls;
   bool _add_cast_to_t;
   event_callback_add(eo_event const& event, eo_class const& cls
                      , bool add_cast_to_t)
     : _event(event), _cls(cls), _add_cast_to_t(add_cast_to_t)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, event_callback_add const& x)
{
   out << tab(1) << "template <typename F>" << endl
       << tab(1) << "::efl::eo::signal_connection" << endl
       << tab(1) << "callback_" << x._event.name << "_add(F && callback_," << endl
       << tab(8) << "::efl::eo::callback_priority priority_ =" << endl
       << tab(8) << "::efl::eo::callback_priorities::default_)" << endl
       << tab(1) << "{" << endl
       << tab(2) << "typedef typename std::remove_reference<F>::type function_type;" << endl
       << tab(2) << "::std::unique_ptr<function_type> f ( new function_type(std::move(callback_)) );" << endl
       << tab(2) << "eo_do(" << add_cast_to_t(x._add_cast_to_t) << "_eo_ptr()," << endl
       << tab(4) << "eo_event_callback_priority_add" << endl
       << tab(4) << "(" << x._event.eo_name << ", priority_," << endl
       << tab(4) << "&efl::eo::_detail::event_callback<" << x._cls.name_space << "::" << x._cls.name << ", function_type>, f.get()));" << endl
       << tab(2) << "return ::efl::eo::make_signal_connection" << endl
       << tab(3) << "(f, " << add_cast_to_t(x._add_cast_to_t)
       << "_eo_ptr(), &efl::eo::_detail::event_callback<"
       << x._cls.name_space << "::" << x._cls.name << ", function_type>," << endl
       << tab(3) << x._event.eo_name << " );" << endl
       << tab(1) << "}" << endl;
   return out;
}

struct event_callback_call
{
   eo_event const& _event;
   bool _add_cast_to_t;
   event_callback_call(eo_event const& event, bool add_cast_to_t)
     : _event(event), _add_cast_to_t(add_cast_to_t)
   {}
};

inline std::ostream&
operator<<(std::ostream& out, event_callback_call const& x)
{
   out << tab(1) << "template <typename T>" << endl
       << tab(1) << "void" << endl
       << tab(1) << "callback_" << x._event.name << "_call(T* info)" << endl
       << tab(1) << "{" << endl
       << tab(2) << "eo_do(" << add_cast_to_t(x._add_cast_to_t) << "_eo_ptr(), eo_event_callback_call" << endl
       << tab(4) << "(" << x._event.eo_name << ", info));" << endl
       << tab(1) << "}" << endl;
   return out;
}

struct events
{
   eo_class const& _cls;
   bool _add_cast_to_t;
   events(eo_class const& cls, bool add_cast_to_t = false)
     : _cls(cls), _add_cast_to_t(add_cast_to_t) {}
};

inline std::ostream&
operator<<(std::ostream& out, events const& x)
{
   for (eo_event const& e : x._cls.events)
     {
        out << event_callback_add(e, x._cls, x._add_cast_to_t) << endl
           << event_callback_call(e, x._add_cast_to_t);
     }
   return out;
}

} } }

#endif // EOLIAN_CXX_STD_EO_CLASS_EVENTS_GENERATOR_HH
