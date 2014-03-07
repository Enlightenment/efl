#ifndef _EINA_ERROR_HH
#define _EINA_ERROR_HH

#include <Eina.h>

#include <system_error>

namespace efl { namespace eina {

using std::errc;
using std::system_error;
using std::error_code;
using std::error_condition;
typedef std::error_category system_error_category;

inline Eina_Error unknown_error()
{
  static Eina_Error error = eina_error_msg_static_register("Error from C++ from another value category error");
  return error;
}

inline system_error_category const& get_generic_category()
{
  return ::std::generic_category();
}
inline system_error_category const& get_system_category()
{
  return ::std::system_category();
}

enum error_type {};

struct error_category : system_error_category
{
  const char* name() const throw()
  {
    return "eina";
  }

  bool equivalent(int code, eina::error_condition const& condition) const throw()
  {
    return code == condition.value();
  }

  bool equivalent(eina::error_code const& code, int condition) const throw()
  {
    return code.value() == condition;
  }

  std::string message(int condition) const
  {
    const char* e = ::eina_error_msg_get(condition);
    return e? e : "::eina_error_msg_get returned NULL. No error message available";
  }
};

inline eina::system_error_category& eina_error_category()
{
  static error_category _error_category;
  return _error_category;
}

inline eina::error_code get_error_code()
{
  Eina_Error error = eina_error_get();
  if(error)
    {
      eina_error_set(0);
      return eina::error_code(error, eina_error_category());
    }
  else
    return eina::error_code();
}

inline void set_error_code(eina::error_code const& e)
{
  if(e.category() == eina_error_category())
    eina_error_set(e.value());
  else
    eina_error_set(unknown_error());
}

inline eina::error_condition get_error_condition()
{
  Eina_Error error = eina_error_get();
  if(error)
    {
      eina_error_set(0);
      return eina::error_condition(error, eina_error_category());
    }
  else
    return eina::error_condition();
}

inline error_type get_error_code_enum()
{
  return static_cast<error_type>( ::eina_error_get() );
}


inline void throw_on_error()
{
  eina::error_code ec = get_error_code();
  if(ec)
  {
    throw eina::system_error(ec, "EFL Eina Error");
  }
}

} }

namespace std {

template <> struct is_error_condition_enum< ::efl::eina::error_type> : true_type {};
template <> struct is_error_code_enum< ::efl::eina::error_type> : true_type {};

}

#endif
