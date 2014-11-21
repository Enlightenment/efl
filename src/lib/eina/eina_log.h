/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga, Cedric Bail
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_LOG_H_
#define EINA_LOG_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>

#include "eina_types.h"

#define EINA_COLOR_LIGHTRED  "\033[31;1m"
#define EINA_COLOR_RED       "\033[31m"
#define EINA_COLOR_LIGHTBLUE "\033[34;1m"
#define EINA_COLOR_BLUE      "\033[34m"
#define EINA_COLOR_GREEN     "\033[32;1m"
#define EINA_COLOR_YELLOW    "\033[33;1m"
#define EINA_COLOR_ORANGE    "\033[0;33m"
#define EINA_COLOR_WHITE     "\033[37;1m"
#define EINA_COLOR_LIGHTCYAN "\033[36;1m"
#define EINA_COLOR_CYAN      "\033[36m"
#define EINA_COLOR_RESET     "\033[0m"
#define EINA_COLOR_HIGH      "\033[1m"

/**
 * @brief Full-featured logging system.
 *
 * Eina provides eina_log_print(), a standard function to manage all
 * logging messages. This function may be called directly or by using the
 * helper macros such as EINA_LOG_DBG(), EINA_LOG_ERR(), or those that
 * take a specific domain as an argument such as EINA_LOG_DOM_DBG(),
 * EINA_LOG_DOM_ERR(). Internally, eina_log_print() calls the
 * function defined with eina_log_print_cb_set(), that defaults to
 * eina_log_print_cb_stderr(), but may be changed to do whatever you
 * need, such as networking or syslog logging.
 *
 * The logging system is thread safe once initialized with
 * eina_log_threads_enable(). The thread that calls this function
 * first is considered "main thread" and other threads have their
 * thread ID (pthread_self()) printed in the log message so it is easy
 * to detect from where it is coming.
 *
 * Log domains is the Eina way to differentiate between messages. There might
 * be different domains to represent different modules, different
 * feature-sets, different categories, and so on. Filtering can be
 * applied to domain names by means of @c EINA_LOG_LEVELS environment
 * variable or eina_log_domain_level_set().
 *
 * The different logging levels serve to customize the amount of
 * debugging one might want to take and may be used to automatically call
 * abort() once some given level message is printed. This is
 * controlled by an environment variable @c EINA_LOG_ABORT and the level
 * to be considered critical, which is @c EINA_LOG_ABORT_LEVEL. These can
 * be changed by eina_log_abort_on_critical_set() and
 * eina_log_abort_on_critical_level_set().
 *
 * The default maximum level to print is defined by environment
 * variable @c EINA_LOG_LEVEL, but may be set per-domain with @c
 * EINA_LOG_LEVELS. It defaults to #EINA_LOG_ERR. This can be
 * changed by eina_log_level_set().
 *
 * To use the log system Eina must be initialized with eina_init() and
 * later shut down with eina_shutdown().
 */

/**
 * @internal
 * @defgroup Eina_Log_Group Log
 * @ingroup Eina_Tools_Group
 *
 * @{
 */

/**
 * EINA_LOG_DOMAIN_GLOBAL is the general purpose log domain to be
 * used, it is always registered and available everywhere.
 */
EAPI extern int EINA_LOG_DOMAIN_GLOBAL;

#ifndef EINA_LOG_DOMAIN_DEFAULT

/**
 * @def EINA_LOG_DOMAIN_DEFAULT
 * @brief Definition of the macro that defines the domain to use with the macros EINA_LOG_DOM_DBG(),
 *        EINA_LOG_DOM_INFO(), EINA_LOG_DOM_WARN(), EINA_LOG_DOM_ERR(), and
 *        EINA_LOG_DOM_CRIT().
 *
 * @remarks If not defined prior to the inclusion of this header, then it
 *          defaults to #EINA_LOG_DOMAIN_GLOBAL.
 *
 * @remarks  One may like to redefine this in its code to avoid typing too
 *           much. In this case, the recommended way is:
 *
 * @code
 * #include <Eina.h>
 * #undef EINA_LOG_DOMAIN_DEFAULT
 * #define EINA_LOG_DOMAIN_DEFAULT _log_dom
 * static int _log_dom = -1;
 *
 * int main(void)
 * {
 *    eina_init();
 *    _log_dom = eina_log_domain_register("mydom", EINA_COLOR_CYAN);
 *    EINA_LOG_ERR("using my own domain");
 *    return 0;
 * }
 * @endcode
 *
 * @remarks If one defines the domain prior to inclusion of this
 *          header, the defined log domain symbol must be defined
 *          prior as well, otherwise the inlined functions defined by
 *          Eina fail to find the symbol, causing a build failure.
 *
 * @code
 * #define EINA_LOG_DOMAIN_DEFAULT _log_dom
 * static int _log_dom = -1; // must come before inclusion of Eina.h!
 * #include <Eina.h>
 *
 * int main(void)
 * {
 *    eina_init();
 *    _log_dom = eina_log_domain_register("mydom", EINA_COLOR_CYAN);
 *    EINA_LOG_ERR("using my own domain");
 *    return 0;
 * }
 * @endcode
 *
 */
# define EINA_LOG_DOMAIN_DEFAULT EINA_LOG_DOMAIN_GLOBAL

#endif /* EINA_LOG_DOMAIN_DEFAULT */

/**
 * @def EINA_LOG(DOM, LEVEL, fmt, ...)
 * @brief Definition that logs a message on the specified domain, level, and format.
 *
 * @remarks If @c EINA_LOG_LEVEL_MAXIMUM is defined, then messages larger
 *          than this value are ignored regardless of the current domain
 *          level, eina_log_print() is not even called. Most
 *          compilers just detect the two integers that make the branch
 *          impossible and remove the branch and function call all
 *          together. Take this as an optimization tip and possibly remove
 *          debug messages from binaries to be deployed, saving on hot
 *          paths. Never define @c EINA_LOG_LEVEL_MAXIMUM on public
 *          header files.
 */
#ifdef EINA_ENABLE_LOG
# ifdef EINA_LOG_LEVEL_MAXIMUM
# define EINA_LOG(DOM, LEVEL, fmt, ...)					\
  do {									\
     if (LEVEL <= EINA_LOG_LEVEL_MAXIMUM) {				\
        eina_log_print(DOM, LEVEL, __FILE__, __FUNCTION__, __LINE__,	\
                       fmt, ## __VA_ARGS__); }				\
  } while (0)
# else
# define EINA_LOG(DOM, LEVEL, fmt, ...) \
  eina_log_print(DOM,                   \
                 LEVEL,                 \
                 __FILE__,              \
                 __FUNCTION__,          \
                 __LINE__,              \
                 fmt,                   \
                 ## __VA_ARGS__)
# endif
#else
#define EINA_LOG(DOM, LEVEL, fmt, ...)          \
  do { (void) DOM; (void) LEVEL; (void) fmt; } while (0)
#endif

/**
 * @def EINA_LOG_DOM_CRIT(DOM, fmt, ...)
 * @brief Definition that logs a message with level as CRITICAL on the specified domain and format.
 */
#define EINA_LOG_DOM_CRIT(DOM, fmt, ...) \
  EINA_LOG(DOM, EINA_LOG_LEVEL_CRITICAL, fmt, ## __VA_ARGS__)

/**
 * @def EINA_LOG_DOM_ERR(DOM, fmt, ...)
 * @brief Definition that logs a message with level as ERROR on the specified domain and format.
 */
#define EINA_LOG_DOM_ERR(DOM, fmt, ...) \
  EINA_LOG(DOM, EINA_LOG_LEVEL_ERR, fmt, ## __VA_ARGS__)

/**
 * @def EINA_LOG_DOM_INFO(DOM, fmt, ...)
 * @brief Definition that logs a message with level as INFO on the specified domain and format.
 */
#define EINA_LOG_DOM_INFO(DOM, fmt, ...) \
  EINA_LOG(DOM, EINA_LOG_LEVEL_INFO, fmt, ## __VA_ARGS__)

/**
 * @def EINA_LOG_DOM_DBG(DOM, fmt, ...)
 * @brief Definition that logs a message with level as DEBUG on the specified domain and format.
 */
#define EINA_LOG_DOM_DBG(DOM, fmt, ...) \
  EINA_LOG(DOM, EINA_LOG_LEVEL_DBG, fmt, ## __VA_ARGS__)

/**
 * @def EINA_LOG_DOM_WARN(DOM, fmt, ...)
 * @brief Definition that logs a message with level as WARN on the specified domain and format.
 */
#define EINA_LOG_DOM_WARN(DOM, fmt, ...) \
  EINA_LOG(DOM, EINA_LOG_LEVEL_WARN, fmt, ## __VA_ARGS__)

/**
 * @def EINA_LOG_CRIT(fmt, ...)
 * @brief Definition that logs a message with level as CRITICAL on the default domain with the specified
 *        format.
 */
#define EINA_LOG_CRIT(fmt, ...)     \
  EINA_LOG(EINA_LOG_DOMAIN_DEFAULT, \
           EINA_LOG_LEVEL_CRITICAL, \
           fmt,                     \
           ## __VA_ARGS__)

/**
 * @def EINA_LOG_ERR(fmt, ...)
 * @brief Definition that logs a message with level as ERROR on the default domain with the specified
 *        format.
 */
#define EINA_LOG_ERR(fmt, ...) \
  EINA_LOG(EINA_LOG_DOMAIN_DEFAULT, EINA_LOG_LEVEL_ERR, fmt, ## __VA_ARGS__)

/**
 * @def EINA_LOG_INFO(fmt, ...)
 * @brief Definition that logs a message with level as INFO on the default domain with the specified
 *        format.
 */
#define EINA_LOG_INFO(fmt, ...) \
  EINA_LOG(EINA_LOG_DOMAIN_DEFAULT, EINA_LOG_LEVEL_INFO, fmt, ## __VA_ARGS__)

/**
 * @def EINA_LOG_WARN(fmt, ...)
 * @brief Definition that logs a message with level as WARN on the default domain with the specified
 *        format.
 */
#define EINA_LOG_WARN(fmt, ...) \
  EINA_LOG(EINA_LOG_DOMAIN_DEFAULT, EINA_LOG_LEVEL_WARN, fmt, ## __VA_ARGS__)

/**
 * @def EINA_LOG_DBG(fmt, ...)
 * @brief Definition that logs a message with level as DEBUG on the default domain with the specified
 *        format.
 */
#define EINA_LOG_DBG(fmt, ...) \
  EINA_LOG(EINA_LOG_DOMAIN_DEFAULT, EINA_LOG_LEVEL_DBG, fmt, ## __VA_ARGS__)

/**
 * @typedef Eina_Log_Domain
 * @brief The structure type containing the domain used for logging.
 */
typedef struct _Eina_Log_Domain Eina_Log_Domain;

/**
 * @struct _Eina_Log_Domain
 * @brief The structure type containing the domain used for logging.
 */
struct _Eina_Log_Domain
{
   int         level; /**< Maximum level to log */
   const char *domain_str; /**< Formatted string with color to print */
   const char *name; /**< Domain name */
   size_t      namelen; /**< strlen(name) */

   /* Private */
   Eina_Bool   deleted : 1; /**< Flags deletion of domain, a free slot */
};

/**
 * @brief Enables logging module to handle threads.
 *
 * @remarks There is no disable option on purpose, if it is enabled, there is
 *          no way back until you call the last eina_shutdown().
 *
 * @remarks There is no function to retrieve if threads are enabled as one is
 *          not supposed to know this from outside.
 *
 * @remarks After this call is executed at least once, if Eina is compiled
 *          with threads support then logging locks around debug messages
 *          and threads that are not the main thread is going to have its identifier
 *          printed.
 *
 *          The main thread is considered the thread where the first
 *          eina_init() was called.
 */
EAPI void eina_log_threads_enable(void);

/**
 * @enum _Eina_Log_Level
 * @brief Enumeration for the list of available logging levels.
 */
typedef enum _Eina_Log_Level
{
   EINA_LOG_LEVEL_CRITICAL, /**< Critical log level */
   EINA_LOG_LEVEL_ERR, /**< Error log level */
   EINA_LOG_LEVEL_WARN, /**< Warning log level */
   EINA_LOG_LEVEL_INFO, /**< Information log level */
   EINA_LOG_LEVEL_DBG, /**< Debug log level */
   EINA_LOG_LEVELS, /**< Count of default log levels */
   EINA_LOG_LEVEL_UNKNOWN = (-2147483647 - 1) /**< Unknown level */
} Eina_Log_Level;

/**
 * @typedef Eina_Log_Print_Cb
 * Called to print callbacks.
 */
typedef void (*Eina_Log_Print_Cb)(const Eina_Log_Domain *d,
                                  Eina_Log_Level level,
                                  const char *file, const char *fnc, int line,
                                  const char *fmt, void *data, va_list args);

/*
 * Customization
 */

/**
 * @brief Sets the logging function to use.
 *
 * @since_tizen 2.3
 *
 * @remarks By default, eina_log_print_cb_stderr() is used.
 * @remarks It is safe to call from any thread.
 * @remarks The given function @a cb is called protected by mutex.
 *          This means you're safe from other calls but you should never
 *          call eina_log_print(), directly or indirectly.
 *
 * @param[in] cb The callback to call when printing a log
 * @param[in] data The data to pass to the callback
 *
 */
EAPI void eina_log_print_cb_set(Eina_Log_Print_Cb cb, void *data) EINA_ARG_NONNULL(1);


/**
 * @brief Sets the default log level.
 *
 * @details This function sets the log level @a level. It is used in
 *          eina_log_print().
 *
 * @since_tizen 2.3
 *
 * @remarks This is initially set to envvar EINA_LOG_LEVEL by eina_init().
 *
 * @param[in] level The log level
 *
 * @see eina_log_level_get()
 */
EAPI void eina_log_level_set(int level);

/**
 * @brief Gets the default log level.
 *
 * @since_tizen 2.3
 *
 * @return The log level that limits eina_log_print()
 *
 * @see eina_log_level_set()
 */
EAPI int  eina_log_level_get(void) EINA_WARN_UNUSED_RESULT;

static inline Eina_Bool eina_log_level_check(int level);

/**
 * @brief Checks whether the current thread is the main thread.
 *
 * @since_tizen 2.3
 *
 * @return @c EINA_TRUE if threads are enabled and the current thread
 *         is the one that called eina_log_threads_init() \n
 *         If there is no thread support (compiled with --disable-pthreads) or
 *         they are not enabled, then @c EINA_TRUE is also
 *         returned \n
 *         The only case where @c EINA_FALSE is returned is
 *         when threads are successfully enabled but the current
 *         thread is not the main (one that called
 *         eina_log_threads_init()).
 */
EAPI Eina_Bool          eina_log_main_thread_check(void) EINA_CONST EINA_WARN_UNUSED_RESULT;


/**
 * @brief Sets whether color logging should be disabled.
 *
 * @since_tizen 2.3
 *
 * @remarks This is initially set to envvar EINA_LOG_COLOR_DISABLE by eina_init().
 *
 * @param[in] disabled If @c EINA_TRUE color logging should be disabled
 *
 * @see eina_log_color_disable_get()
 */
EAPI void               eina_log_color_disable_set(Eina_Bool disabled);

/**
 * @brief Gets whether color logging should be disabled.
 *
 * @since_tizen 2.3
 *
 * @return @c EINA_TRUE if color logging should be disabled
 *
 * @see eina_log_color_disable_set()
 */
EAPI Eina_Bool          eina_log_color_disable_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Sets whether originating file name logging should be disabled.
 *
 * @since_tizen 2.3
 *
 * @remarks This is initially set to envvar EINA_LOG_FILE_DISABLE by eina_init().
 *
 * @param[in] disabled If @c EINA_TRUE file name logging should be disabled
 *
 * @see eina_log_file_disable_get()
 */
EAPI void               eina_log_file_disable_set(Eina_Bool disabled);

/**
 * @brief Gets whether originating file name logging should be disabled.
 *
 * @since_tizen 2.3
 *
 * @return @c EINA_TRUE if file name logging should be disabled
 *
 * @see eina_log_file_disable_set()
 */
EAPI Eina_Bool          eina_log_file_disable_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Sets whether originating function name logging should be disabled.
 *
 * @since_tizen 2.3
 *
 * @remarks This is initially set to envvar EINA_LOG_FUNCTION_DISABLE by
 *          eina_init().
 *
 * @param[in] disabled If @c EINA_TRUE function name logging should be disabled
 *
 * @see eina_log_function_disable_get()
 */
EAPI void               eina_log_function_disable_set(Eina_Bool disabled);

/**
 * @brief Gets whether originating function name logging should be disabled.
 *
 * @return @c EINA_TRUE if function name logging should be disabled
 *
 * @see eina_log_function_disable_set()
 */
EAPI Eina_Bool          eina_log_function_disable_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Sets whether critical messages should abort the program.
 *
 * @remarks This is initially set to envvar EINA_LOG_ABORT by
 *          eina_init().
 *
 * @param[in] abort_on_critical If @c EINA_TRUE, messages with level equal to
 *                          or smaller than eina_log_abort_on_critical_level_get()
 *                          abort the program.
 *
 * @see eina_log_abort_on_critical_get()
 * @see eina_log_abort_on_critical_level_set()
 */
EAPI void               eina_log_abort_on_critical_set(Eina_Bool abort_on_critical);

/**
 * @brief Gets whether critical messages should abort the program.
 *
 * @since_tizen 2.3
 *
 * @return @c EINA_TRUE if messages with level equal to or smaller
 *         than eina_log_abort_on_critical_level_get() abort the
 *         program
 *
 * @see eina_log_abort_on_critical_set()
 * @see eina_log_abort_on_critical_level_set()
 */
EAPI Eina_Bool          eina_log_abort_on_critical_get(void) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Sets the level that triggers abort if abort-on-critical is set.
 *
 * @since_tizen 2.3
 *
 * @remarks This is initially set to envvar EINA_LOG_ABORT_LEVEL by
 *          eina_init().
 *
 * @param[in] critical_level If @c EINA_TRUE, levels equal to or smaller than
 *                       eina_log_abort_on_critical_get() triggers program abortion
 *
 * @see eina_log_abort_on_critical_level_get()
 * @see eina_log_abort_on_critical_get()
 */
EAPI void               eina_log_abort_on_critical_level_set(int critical_level);

/**
 * @brief Gets the level that triggers abort if abort-on-critical is set.
 *
 * @since_tizen 2.3
 *
 * @return @c EINA_TRUE if the critical level equal to or smaller than
 *         eina_log_abort_on_critical_get() triggers program abortion
 *
 * @see eina_log_abort_on_critical_level_set()
 * @see eina_log_abort_on_critical_get()
 */
EAPI int                eina_log_abort_on_critical_level_get(void) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Sets the domain level given that its name is provided.
 *
 * @since_tizen 2.3
 *
 * @remarks This call has the same effect as setting
 *          EINA_LOG_LEVELS=&lt;@a domain_name&gt;:&lt;@a level&gt;
 *
 * @param[in] domain_name The domain name to change the level \n
 *                    It may be of a still not registered domain \n
 *                    If the domain is not registered
 *                    yet, it is saved as a pending set and applied upon
 *                    registration.
 * @param[in] level The level to use to limit eina_log_print() for the given domain
 */
EAPI void               eina_log_domain_level_set(const char *domain_name, int level) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the domain level given that its name is provided.
 *
 * @since_tizen 2.3
 *
 * @param[in] domain_name The domain name to retrieve the level \n
 *                    It may be of a still not registered domain \n
 *                    If the domain is not registered yet, but there is a pending value either from
 *                    eina_log_domain_level_set(), EINA_LOG_LEVELS environment
 *                    variable, or EINA_LOG_LEVELS_GLOB, these are
 *                    returned \n
 *                    If nothing else is found, then the global/default
 *                    level (eina_log_level_get()) is returned.
 *
 * @return The level to use to limit eina_log_print() for the given
 *         domain \n
 *         On error (@a domain_name == NULL),
 *         EINA_LOG_LEVEL_UNKNOWN is returned.
 *
 * @see eina_log_domain_level_set()
 * @see eina_log_domain_registered_level_get()
 */
EAPI int                eina_log_domain_level_get(const char *domain_name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Gets the domain level given that its identifier is provided.
 *
 * @since_tizen 2.3
 *
 * @param[in] domain The domain identifier, so it must be previously registered with
 *               eina_log_domain_register() \n
 *               It's a much faster version of eina_log_domain_level_get(),
 *               but relies on the domain being present.
 *
 * @return @c EINA_TRUE if the level should be printed, otherwise @c EINA_FALSE if not
 *         (domain's level is greater than or equal to @a level)
 */
EAPI int                eina_log_domain_registered_level_get(int domain) EINA_WARN_UNUSED_RESULT;

static inline Eina_Bool eina_log_domain_level_check(int domain, int level);

/*
 * Logging domains
 */

/**
 * @brief Logs domains
 *
 * @since_tizen 2.3
 *
 * @remarks It is safe to call from any thread.
 *
 * @param[in] name The domain name
 * @param[in] color The color of the domain name
 *
 * @return The domain index that is used as the DOMAIN parameter on log
 *         macros \n
 *         A negative return value means a log occurred.
 */
EAPI int  eina_log_domain_register(const char *name, const char *color) EINA_ARG_NONNULL(1);

/**
 * @brief Forgets about a logging domain registered by eina_log_domain_register().
 *
 * @since_tizen 2.3
 *
 * @remarks It is safe to call from any thread.
 *
 * @param[in] domain The domain identifier as reported by eina_log_domain_register(),
 *               must be >= 0
 */
EAPI void eina_log_domain_unregister(int domain);

/*
 * Logging functions.
 */

/**
 * @brief Prints out log messages using the given domain and level.
 *
 * @details This function may be called from different threads if
 *          eina_log_threads_enable() is called before.
 *
 * @since_tizen 2.3
 *
 * @remarks Usually you do not use this function directly but the helper
 *          macros EINA_LOG(), EINA_LOG_DOM_CRIT(), EINA_LOG_CRIT(), and
 *         so on. See eina_log.h
 *
 * @param[in] domain The logging domain to use, otherwise @c EINA_LOG_DOMAIN_GLOBAL if
 *               you registered none \n
 *               It is recommended that modules and
 *               applications have their own logging domain.
 * @param[in] level  The message level, those with levels greater than the user
 *               specified value (eina_log_level_set() or environment
 *               variables like EINA_LOG_LEVEL, EINA_LOG_LEVELS) are ignored
 * @param[in] file   The filename that originated the call, must @b not be @c NULL
 * @param[in] function The function that originated the call, must @b not be @c NULL
 * @param[in] line 	 The originating line in @a file
 * @param[in] fmt 	 The printf-like format to use, should not provide trailing
 *               '\n' as it is automatically included
 */
EAPI void eina_log_print(int            domain,
                         Eina_Log_Level level,
                         const char    *file,
                         const char    *function,
                         int            line,
                         const char    *fmt,
                         ...) EINA_ARG_NONNULL(3, 4, 6) EINA_PRINTF(6, 7) EINA_NOINSTRUMENT;

/**
 * @brief Prints out log messages using the given domain and level.
 *
 * @details This function may be called from different threads if
 *          eina_log_threads_enable() is called before.
 *
 * @since_tizen 2.3
 *
 * @remarks Usually you do not use this function directly, but the helper
 *          macros like EINA_LOG(), EINA_LOG_DOM_CRIT(), EINA_LOG_CRIT(), and
 *          so on. See eina_log.h
 *
 * @param[in] domain The logging domain to use, otherwise @c EINA_LOG_DOMAIN_GLOBAL if
 *               you registered none \n
 *               It is recommended that modules and
 *               applications have their own logging domain.
 * @param[in] level  The message level, those with levels greater than the user
 *               specified value (eina_log_level_set() or environment
 *               variables like EINA_LOG_LEVEL, EINA_LOG_LEVELS) are ignored
 * @param[in] file 	 The filename that originated the call, must @b not be @c NULL
 * @param[in] fnc 	 The function that originated the call, must @b not be @c NULL
 * @param[in] line 	 The originating line in @a file.
 * @param[in] fmt 	 The printf-like format to use, should not provide trailing
 *               '\n' as it is automatically included
 * @param[in] args 	 The arguments needed by the format.
 *
 * @see eina_log_print()
 */
EAPI void eina_log_vprint(int            domain,
                          Eina_Log_Level level,
                          const char    *file,
                          const char    *fnc,
                          int            line,
                          const char    *fmt,
                          va_list        args) EINA_ARG_NONNULL(3, 4, 6) EINA_NOINSTRUMENT;

/*
 * Logging methods (change how logging is done).
 */

/**
 * @brief Alternative logging function, this outputs to the standard output stream.
 *
 * @details This function colorizes the output based on the domain provided color and
 *          the message logging level. To disable color, set the environment variable
 *          EINA_LOG_COLOR_DISABLE=1. Similarly, to disable file and line
 *          information, set EINA_LOG_FILE_DISABLE=1 or
 *          EINA_LOG_FUNCTION_DISABLE=1 to avoid function name in the output. It is
 *          not acceptable to have both EINA_LOG_FILE_DISABLE and
 *          EINA_LOG_FUNCTION_DISABLE at the same time, in this case just
 *          EINA_LOG_FUNCTION_DISABLE is considered and file information
 *          is printed anyways.
 *
 * @since_tizen 2.3
 *
 * @remarks If threads are enabled, this function is called within locks.
 * @remarks Threads different from the main thread have a thread ID
 *          appended to the domain name.
 *
 * @param[in] d The domain
 * @param[in] level The level
 * @param[in] file The file that is logged
 * @param[in] fnc The function that is logged
 * @param[in] line The line that is logged
 * @param[in] fmt The ouptut format to use
 * @param[in] data Not used
 * @param[in] args The arguments needed by the format
 *
 */
EAPI void eina_log_print_cb_stdout(const Eina_Log_Domain *d,
                                   Eina_Log_Level         level,
                                   const char            *file,
                                   const char            *fnc,
                                   int                    line,
                                   const char            *fmt,
                                   void                  *data,
                                   va_list                args);

/**
 * @brief Default logging function, this outputs to the standard error stream.
 *
 * @details This function colorizes the output based on the domain provided color and
 *          the message logging level.
 *
 *          To disable color, set the environment variable
 *          EINA_LOG_COLOR_DISABLE=1. To enable color, even if directing to a
 *          file or when using a non-supported color terminal, use
 *          EINA_LOG_COLOR_DISABLE=0. If EINA_LOG_COLOR_DISABLE is unset (or
 *          -1), then Eina disables color if terminal ($TERM) is
 *          unsupported or redirecting to a file.
 *
 *          Similarly, to disable file and line
 *          information, set EINA_LOG_FILE_DISABLE=1 or
 *          EINA_LOG_FUNCTION_DISABLE=1 to avoid function name in the output. It is
 *          not acceptable to have both EINA_LOG_FILE_DISABLE and
 *          EINA_LOG_FUNCTION_DISABLE at the same time, in this case just
 *          EINA_LOG_FUNCTION_DISABLE is considered and file information
 *          is printed anyways.
 *
 * @since_tizen 2.3
 *
 * @remarks If threads are enabled, this function is called within locks.
 * @remarks Threads different from the main thread have a thread ID
 *          appended to the domain name.
 *
 * @param[in] d The domain
 * @param[in] level The level
 * @param[in] file The file that is logged
 * @param[in] fnc The function that is logged
 * @param[in] line The line that is logged
 * @param[in] fmt The ouptut format to use
 * @param[in] data Not used
 * @param[in] args The arguments needed by the format
 *
 */
EAPI void eina_log_print_cb_stderr(const Eina_Log_Domain *d,
                                   Eina_Log_Level         level,
                                   const char            *file,
                                   const char            *fnc,
                                   int                    line,
                                   const char            *fmt,
                                   void                  *data,
                                   va_list                args);

/**
 * @brief Alternative logging function, this outputs to the given file stream.
 *
 * @details This function never gives color as the output.
 *
 * @since_tizen 2.3
 *
 * @remarks If threads are enabled, this function is called within locks.
 * @remarks Threads different from the main thread have a thread ID
 *          appended to the domain name.
 *
 * @param[in] d The domain
 * @param[in] level Not used
 * @param[in] file The file that is logged
 * @param[in] fnc The function that is logged
 * @param[in] line The line that is logged
 * @param[in] fmt The ouptut format to use
 * @param[in] data The file that stores the output (as a FILE *)
 * @param[in] args The arguments needed by the format
 *
 */
EAPI void eina_log_print_cb_file(const Eina_Log_Domain *d,
                                 Eina_Log_Level         level,
                                 const char            *file,
                                 const char            *fnc,
                                 int                    line,
                                 const char            *fmt,
                                 void                  *data,
                                 va_list                args);

/*--- TIZEN_ONLY : begin ---*/
/**
 * @brief Alternative logging function, this outputs to the system log.
 *
 * @details This function never gives color as the output.
 *
 * @param[in] d The domain
 * @param[in] level Not used
 * @param[in] file The file that is logged
 * @param[in] fnc The function that is logged
 * @param[in] line The line that is logged
 * @param[in] fmt The ouptut format to use
 * @param[in] data Not Used
 * @param[in] args The arguments needed by the format
 *
 */
EAPI void eina_log_print_cb_syslog(const Eina_Log_Domain *d,
                                   Eina_Log_Level         level,
                                   const char            *file,
                                   const char            *fnc,
                                   int                    line,
                                   const char            *fmt,
                                   void                  *data,
                                   va_list                args);

#ifdef HAVE_DLOG
/**
 * @brief Alternative logging function, this outputs to the dlog.
 *
 * @since_tizen 2.3
 *
 * @param[in] d The domain
 * @param[in] level Not used
 * @param[in] file The file that is logged
 * @param[in] fnc The function that is logged
 * @param[in] line The line that is logged
 * @param[in] fmt The ouptut format to use
 * @param[in] data Not Used
 * @param[in] args The arguments needed by the format
 *
 */
EAPI void eina_log_print_cb_dlog(const Eina_Log_Domain *d,
                                 Eina_Log_Level         level,
                                 const char            *file,
                                 const char            *fnc,
                                 int                    line,
                                 const char            *fmt,
                                 void                  *data,
                                 va_list                args);
#endif

/*--- TIZEN_ONLY : end ---*/

/**
 * @brief Configures the console color of the given file.
 *
 * @since 1.7
 *
 * @since_tizen 2.3
 *
 * @remarks If color is disabled, nothing is done.
 * @remarks In windows, both @a fp and @a color are converted automatically.
 *
 * @param[in] fp The file to configure the console color of(usually stderr or stdout)
 * @param[in] color A VT color code such as EINA_COLOR_RED or EINA_COLOR_RESET
 *
 *
 * @see eina_log_color_disable_get()
 */
EAPI void eina_log_console_color_set(FILE *fp,
                                     const char *color) EINA_ARG_NONNULL(1, 2);

#include "eina_inline_log.x"

/**
 * @}
 */

#endif /* EINA_LOG_H_ */
