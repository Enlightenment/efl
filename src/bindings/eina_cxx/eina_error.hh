#ifndef _EINA_ERROR_HH
#define _EINA_ERROR_HH

#include <Eina.h>

#include <system_error>

/**
 * @addtogroup Eina_Cxx_Tools_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Error_Group Error Handling
 * @ingroup Eina_Cxx_Tools_Group
 *
 * @brief Functions for handling Eina errors.
 *
 * Integrates the Eina errors with the standard error representation
 * defined in the @c system_error library.
 *
 * @{
 */

/** <tt>std::errc</tt> for Eina errors. */
using std::errc;

/** <tt>std::system_error</tt> for Eina errors. */
using std::system_error;

/** <tt>std::error_code</tt> for Eina errors. */
using std::error_code;

/** <tt>std::error_condition</tt> for Eina errors. */
using std::error_condition;

/** <tt>std::error_category</tt> for Eina errors. */
typedef std::error_category system_error_category;

/**
 * @brief Return a @c Eina_Error for an unknown error.
 * @return @c Eina_Error indicating a unknown/external error condition.
 *
 * This function returns an @c Eina_Error indicating a unknown/external
 * error condition. When first called, this function will register the
 * said error within the other Eina errors, together with a error
 * message.
 */
inline Eina_Error unknown_error()
{
  static Eina_Error error = eina_error_msg_static_register("Error from C++ from another value category error");
  return error;
}

/**
 * @brief Gets a <tt>std::generic_category</tt> instance as a <tt>eina::system_error_category</tt>.
 * @return a <tt>std::generic_category</tt> instance as a <tt>eina::system_error_category</tt>.
 */
inline system_error_category const& get_generic_category()
{
  return ::std::generic_category();
}

/**
 * @brief Gets a <tt>std::system_category</tt> instance as a <tt>eina::system_error_category</tt>.
 * @return <tt>std::system_category</tt> instance as a <tt>eina::system_error_category</tt>.
 */
inline system_error_category const& get_system_category()
{
  return ::std::system_category();
}

/**
 * @brief Typesafe representation of an @c Eina_Error.
 *
 * Used for improved compatibility with @e system_error library.
 */
enum error_type {};

/**
 * @brief Specialized error category for Eina errors.
 */
struct error_category : system_error_category
{
  /**
   * @brief Name of the error category.
   * @return String containing the word "eina"
   */
  const char* name() const throw()
  {
    return "eina";
  }

  /**
   * @brief Check if the given error code is equivalent to the given error condition.
   * @param code Integer representing the error code.
   * @param condition <tt>eina::error_condition</tt> object.
   * @return @c true if @c code is equivalent to @c condition.
   */
  bool equivalent(int code, eina::error_condition const& condition) const throw()
  {
    return code == condition.value();
  }

  /**
   * @brief Check if the given error code is equivalent to the given error condition.
   * @param code <tt>eina::error_code</tt> object.
   * @param condition Integer representing the error condition.
   * @return @c true if @c code is equivalent to @c condition.
   */
  bool equivalent(eina::error_code const& code, int condition) const throw()
  {
    return code.value() == condition;
  }

  /**
   * @brief Get the message related with the given error condition.
   * @param condition Eina error condition.
   * @return String containing the message related with the given error condition.
   *
   * This member function returns the error message related with the
   * given error condition code.
   *
   * @note When the given condition code is not registered within the
   * Eina errors it will return a string indicating that an error
   * message is not available.
   */
  std::string message(int condition) const
  {
    const char* e = ::eina_error_msg_get(condition);
    return e? e : "::eina_error_msg_get returned NULL. No error message available";
  }
};

/**
 * @brief Get a default <tt>eina::error_category</tt> object.
 * @return Reference to a static instance of an <tt>eina::error_category</tt>.
 */
inline eina::system_error_category& eina_error_category()
{
  static error_category _error_category;
  return _error_category;
}

/**
 * @brief Gets the error code for the last Eina error.
 * @return <tt>eina::error_code</tt> for the last Eina error.
 *
 * This function gets the error code for the last Eina error and
 * consumes it. The category of the returned <tt>eina::error_code</tt>
 * is @c eina_error_category.
 *
 * @note If no errors have been occurred or if this functions have
 * already been called after the last error occurrence a call to this
 * function will return a default <tt>eina::error_code</tt> to indicates
 * that there is no unconsumed error.
 */
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

/**
 * @brief Sets an error code in the Eina library.
 * @param e Error code. Should be an @c eina_error_category error.
 *
 * This function sets an error code in the Eina library. If the category
 * of the given error code is not @c eina_error_category it will
 * register an unknown error instead.
 */
inline void set_error_code(eina::error_code const& e)
{
  if(e.category() == eina_error_category())
    eina_error_set(e.value());
  else
    eina_error_set(unknown_error());
}

/**
 * @brief Gets the error condition for the last Eina error.
 * @return <tt>eina::error_condition</tt> for the last Eina error.
 *
 * This function works exactly like @ref get_error_code but returns an
 * <tt>eina::error_condition</tt> object instead.
 */
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

/**
 * @brief Gets the enum value of the last Eina error.
 * @return Value of the last Eina error as an @c error_type.
 *
 * This function returns the error code for the last Eina error.
 *
 * Differently from @ref get_error_code and @ref get_error_condition,
 * this function does not consume the last error.
 */
inline error_type get_error_code_enum()
{
  return static_cast<error_type>( ::eina_error_get() );
}


/**
 * @brief Throw an exception if there is a error set in Eina library.
 * @throw <tt>eina::system_error</tt> containing the error identifier.
 *
 * This function is meant to be used after executing a operation that
 * may set an Eina error. If an error code has been set this function
 * will throw an exception.
 *
 * The thrown exception holds an <tt>eina::error_code</tt> equivalent to
 * the one returned by @ref get_error_code.
 *
 * Like the @ref get_error_code function, this one consumes the last
 * error code.
 */
inline void throw_on_error()
{
  eina::error_code ec = get_error_code();
  if(ec)
    {
       EFL_CXX_THROW(eina::system_error(ec, "EFL Eina Error"));
    }
}

/**
 * @}
 */

} }

/**
 * @internal
 * Template specialization for interoperability with the @e system_error
 * standard library.
 * @{
 */
namespace std {
template <> struct is_error_condition_enum< ::efl::eina::error_type> : true_type {};
template <> struct is_error_code_enum< ::efl::eina::error_type> : true_type {};
}
/**
 * @}
 */

/**
 * @}
 */

#endif
