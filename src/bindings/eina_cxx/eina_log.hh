#ifndef EFL_EINA_LOG_HH
#define EFL_EINA_LOG_HH

#include <sstream>

/**
 * @addtogroup Eina_Cxx_Tools_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Log_Group Log
 * @ingroup Eina_Cxx_Tools_Group
 *
 * @{
 */

/**
 * Types to represent each log level.
 *
 * @{
 */
namespace log_level {

/**
 * Type for representing a critical log level.
 */
struct critical_t { static constexpr ::Eina_Log_Level value = ::EINA_LOG_LEVEL_CRITICAL; };
/** Critical log level */
critical_t const critical = {};

/**
 * Type for representing an error log level.
 */
struct error_t { static constexpr ::Eina_Log_Level value = ::EINA_LOG_LEVEL_ERR; };
/** Error log level */
error_t const error = {};

/**
 * Type for representing an information log level.
 */
struct info_t { static constexpr ::Eina_Log_Level value = ::EINA_LOG_LEVEL_INFO; };
/** Information log level */
info_t const info = {};

/**
 * Type for representing a debug log level.
 */
struct debug_t { static constexpr ::Eina_Log_Level value = ::EINA_LOG_LEVEL_DBG; };
/** Debug log level */
debug_t const debug = {};

/**
 * Type for representing a warning log level.
 */
struct warn_t { static constexpr ::Eina_Log_Level value = ::EINA_LOG_LEVEL_WARN; };
/** Warning log level */
warn_t const warning = {};

}
/**
 * @}
 */


/**
 * Base implementation for log domains.
 */
template <typename D>
struct _domain_base
{
  /**
   * @brief Set the domain log level based on the given log level type.
   *
   * @{
   */
  void set_level(log_level::critical_t l) { set_level(l.value); }
  void set_level(log_level::error_t l) { set_level(l.value); }
  void set_level(log_level::info_t l) { set_level(l.value); }
  void set_level(log_level::debug_t l) { set_level(l.value); }
  void set_level(log_level::warn_t l) { set_level(l.value); }
  /**
   * @}
   */

  /**
   * @brief Set the domain log level to the level specified by the given identifier.
   * @param l Eina native identifier to a log level.
   */
  void set_level( ::Eina_Log_Level l)
  {
    ::eina_log_domain_registered_level_set(static_cast<D&>(*this).domain_raw(), l);
  }

  /**
   * @brief Get the domain log level.
   * @return Eina native identifier representing the current log level of the domain.
   */
  ::Eina_Log_Level get_level() const
  {
    return static_cast< ::Eina_Log_Level>
      (::eina_log_domain_registered_level_get(static_cast<D const&>(*this).domain_raw()));
  }
};

/**
 * @internal
 */
struct global_domain : _domain_base<global_domain>
{
  int domain_raw() const { return EINA_LOG_DOMAIN_GLOBAL; }
};

/**
 * General purpose log domain.
 * It is always registered and available everywhere.
 */
struct global_domain const global_domain = {};

/**
 * @internal
 */
struct default_domain : _domain_base<default_domain>
{
  int domain_raw() const { return EINA_LOG_DOMAIN_DEFAULT; }
};

/**
 * Default log domain.
 * If the macro @c EINA_LOG_DOMAIN_DEFAULT is not defined to anything
 * different it will be equivalent to @c global_domain.
 */
struct default_domain const default_domain = {};

/**
 * Class for creating log domains. It register a new domain upon
 * construction and unregister it upon destruction, following the RAII
 * programming idiom.
 */
struct log_domain : _domain_base<log_domain>
{
  /**
   * @brief Creates a new log domain.
   * @param name Name of the domain.
   * @param color Color of the domain name.
   */
  log_domain(char const* name, char const* color = "black")
    : _domain( ::eina_log_domain_register(name, color))
  {
  }

  /**
   * @brief Unregister the domain.
   */
  ~log_domain()
  {
    ::eina_log_domain_unregister(_domain);
  }
  int domain_raw() const { return _domain; }
private:
  /**
   * @internal
   *
   * Member variable that holds the domain identifier.
   */
  int _domain;
};

/**
 * @internal
 */
inline void _log(std::stringstream const& stream, int domain, ::Eina_Log_Level level
                 , const char* file, const char* function, int line)
{
  ::eina_log_print(domain, level, file, function, line
                   , "%s", stream.str().c_str());
}

/**
 * @def EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 *
 * Logs a message with level %p LEVEL on the domain %p DOMAIN.
 *
 * It works like a STL output stream and should be used with the left
 * shift operator. Example:
 * \code{.py}
 * EINA_CXX_DOM_LOG(my_domain, my_log_level) << "My log message.";
 * \endcode
 */
#define EINA_CXX_DOM_LOG(DOMAIN, LEVEL)                                 \
    for( bool run = ::eina_log_domain_level_check((DOMAIN), LEVEL); run;) \
      for(std::stringstream stream; run ;                               \
          ::efl::eina::_log(std::move(stream), (DOMAIN), LEVEL          \
                            , __FILE__, __FUNCTION__, __LINE__), run = false) \
        stream

/**
 * @def EINA_CXX_DOM_LOG_CRIT(DOMAIN)
 *
 * Logs a message with level %c EINA_LOG_LEVEL_CRITICAL on the domain
 * %p DOMAIN.
 *
 * It is a short for EINA_CXX_DOM_LOG(DOMAIN, ::EINA_LOG_LEVEL_CRITICAL).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_DOM_LOG_CRIT(DOMAIN)                     \
    EINA_CXX_DOM_LOG(DOMAIN.domain_raw(), ::EINA_LOG_LEVEL_CRITICAL)

/**
 * @def EINA_CXX_DOM_LOG_ERR(DOMAIN)
 *
 * Logs a message with level %c EINA_LOG_LEVEL_ERR on the domain
 * %p DOMAIN.
 *
 * It is a short for EINA_CXX_DOM_LOG(DOMAIN, ::EINA_LOG_LEVEL_ERR).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_DOM_LOG_ERR(DOMAIN)                      \
    EINA_CXX_DOM_LOG(DOMAIN.domain_raw(), ::EINA_LOG_LEVEL_ERR)

/**
 * @def EINA_CXX_DOM_LOG_INFO(DOMAIN)
 *
 * Logs a message with level %c EINA_LOG_LEVEL_INFO on the domain
 * %p DOMAIN.
 *
 * It is a short for EINA_CXX_DOM_LOG(DOMAIN, ::EINA_LOG_LEVEL_INFO).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_DOM_LOG_INFO(DOMAIN)                      \
    EINA_CXX_DOM_LOG(DOMAIN.domain_raw(), ::EINA_LOG_LEVEL_INFO)

/**
 * @def EINA_CXX_DOM_LOG_DBG(DOMAIN)
 *
 * Logs a message with level %c EINA_LOG_LEVEL_DBG on the domain
 * %p DOMAIN.
 *
 * It is a short for EINA_CXX_DOM_LOG(DOMAIN, ::EINA_LOG_LEVEL_DBG).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_DOM_LOG_DBG(DOMAIN)                      \
    EINA_CXX_DOM_LOG(DOMAIN.domain_raw(), ::EINA_LOG_LEVEL_DBG)

/**
 * @def EINA_CXX_DOM_LOG_WARN(DOMAIN)
 *
 * Logs a message with level %c EINA_LOG_LEVEL_WARN on the domain
 * %p DOMAIN.
 *
 * It is a short for EINA_CXX_DOM_LOG(DOMAIN, ::EINA_LOG_LEVEL_WARN).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_DOM_LOG_WARN(DOMAIN)                      \
    EINA_CXX_DOM_LOG(DOMAIN.domain_raw(), ::EINA_LOG_LEVEL_WARN)

/**
 * @def EINA_CXX_LOG(LEVEL)
 *
 * Logs a message with level %p LEVEL on the default domain
 * %c EINA_LOG_DOMAIN_DEFAULT.
 *
 * It is a short for EINA_CXX_DOM_LOG(EINA_LOG_DOMAIN_DEFAULT, LEVEL).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_LOG(LEVEL)                               \
    EINA_CXX_DOM_LOG(EINA_LOG_DOMAIN_DEFAULT, LEVEL)

/**
 * @def EINA_CXX_LOG_CRIT()
 *
 * Logs a message with level %c EINA_LOG_LEVEL_CRITICAL on the default
 * domain %c EINA_LOG_DOMAIN_DEFAULT.
 *
 * It is a short for EINA_CXX_DOM_LOG(EINA_LOG_DOMAIN_DEFAULT, ::EINA_LOG_LEVEL_CRITICAL).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_LOG_CRIT()                 \
    EINA_CXX_LOG(EINA_LOG_LEVEL_CRITICAL)

/**
 * @def EINA_CXX_LOG_ERR()
 *
 * Logs a message with level %c EINA_LOG_LEVEL_ERR on the default
 * domain %c EINA_LOG_DOMAIN_DEFAULT.
 *
 * It is a short for EINA_CXX_DOM_LOG(EINA_LOG_DOMAIN_DEFAULT, ::EINA_LOG_LEVEL_ERR).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_LOG_ERR()                  \
    EINA_CXX_LOG(EINA_LOG_LEVEL_ERR)

/**
 * @def EINA_CXX_LOG_INFO()
 *
 * Logs a message with level %c EINA_LOG_LEVEL_INFO on the default
 * domain %c EINA_LOG_DOMAIN_DEFAULT.
 *
 * It is a short for EINA_CXX_DOM_LOG(EINA_LOG_DOMAIN_DEFAULT, ::EINA_LOG_LEVEL_INFO).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_LOG_INFO()                 \
    EINA_CXX_LOG(EINA_LOG_LEVEL_INFO)

/**
 * @def EINA_CXX_LOG_DBG()
 *
 * Logs a message with level %c EINA_LOG_LEVEL_DBG on the default
 * domain %c EINA_LOG_DOMAIN_DEFAULT.
 *
 * It is a short for EINA_CXX_DOM_LOG(EINA_LOG_DOMAIN_DEFAULT, ::EINA_LOG_LEVEL_DBG).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_LOG_DBG()                  \
    EINA_CXX_LOG(EINA_LOG_LEVEL_DBG)

/**
 * @def EINA_CXX_LOG_WARN()
 *
 * Logs a message with level %c EINA_LOG_LEVEL_WARN on the default
 * domain %c EINA_LOG_DOMAIN_DEFAULT.
 *
 * It is a short for EINA_CXX_DOM_LOG(EINA_LOG_DOMAIN_DEFAULT, ::EINA_LOG_LEVEL_WARN).
 *
 * @see EINA_CXX_DOM_LOG(DOMAIN, LEVEL)
 */
#define EINA_CXX_LOG_WARN()                 \
    EINA_CXX_LOG(EINA_LOG_LEVEL_WARN)

/**
 * @}
 */

} }

/**
 * @}
 */

#endif
