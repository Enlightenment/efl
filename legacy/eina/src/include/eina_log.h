/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga, Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_LOG_H_
#define EINA_LOG_H_

#include <stdarg.h>

#include "eina_types.h"

#define EINA_COLOR_LIGHTRED  "\033[31;1m"
#define EINA_COLOR_RED       "\033[31m"
#define EINA_COLOR_BLUE      "\033[34;1m"
#define EINA_COLOR_GREEN     "\033[32;1m"
#define EINA_COLOR_YELLOW    "\033[33;1m"
#define EINA_COLOR_ORANGE    "\033[0;33m"
#define EINA_COLOR_WHITE     "\033[37;1m"
#define EINA_COLOR_LIGHTBLUE "\033[36;1m"
#define EINA_COLOR_RESET     "\033[0m"
#define EINA_COLOR_HIGH      "\033[1m"

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Log_Group Log
 *
 * @{
 */

EAPI extern int EINA_LOG_DOMAIN_GLOBAL;


/**
 * @def EINA_LOG(DOM, LEVEL, fmt, ...)
 * Logs a message on the specified domain, level and format.
 *
 * @note if @c EINA_LOG_LEVEL_MAXIMUM is defined, then messages larger
 *       than this value will be ignored regardless of current domain
 *       level, the eina_log_print() is not even called! Most
 *       compilers will just detect the two integers make the branch
 *       impossible and remove the branch and function call all
 *       together. Take this as optimization tip and possible remove
 *       debug messages from binaries to be deployed, saving on hot
 *       paths. Never define @c EINA_LOG_LEVEL_MAXIMUM on public
 *       header files.
 */
#ifdef EINA_LOG_LEVEL_MAXIMUM
#define EINA_LOG(DOM, LEVEL, fmt, ...)					\
  do {									\
     if (LEVEL <= EINA_LOG_LEVEL_MAXIMUM)				\
       eina_log_print(DOM, LEVEL, __FILE__, __FUNCTION__, __LINE__,	\
		      fmt, ##__VA_ARGS__);				\
  } while (0)
#else
#define EINA_LOG(DOM, LEVEL, fmt, ...) \
	eina_log_print(DOM, LEVEL, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#endif

/**
 * @def EINA_LOG_DOM_CRIT(DOM, fmt, ...)
 * Logs a message with level CRITICAL on the specified domain and format.
 */
#define EINA_LOG_DOM_CRIT(DOM, fmt, ...) \
	EINA_LOG(DOM, EINA_LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_DOM_ERR(DOM, fmt, ...)
 * Logs a message with level ERROR on the specified domain and format.
 */
#define EINA_LOG_DOM_ERR(DOM, fmt, ...) \
	EINA_LOG(DOM, EINA_LOG_LEVEL_ERR, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_DOM_INFO(DOM, fmt, ...)
 * Logs a message with level INFO on the specified domain and format.
 */
#define EINA_LOG_DOM_INFO(DOM, fmt, ...) \
	EINA_LOG(DOM, EINA_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_DOM_DBG(DOM, fmt, ...)
 * Logs a message with level DEBUG on the specified domain and format.
 */
#define EINA_LOG_DOM_DBG(DOM, fmt, ...) \
	EINA_LOG(DOM, EINA_LOG_LEVEL_DBG, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_DOM_WARN(DOM, fmt, ...)
 * Logs a message with level WARN on the specified domain and format.
 */
#define EINA_LOG_DOM_WARN(DOM, fmt, ...) \
	EINA_LOG(DOM, EINA_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_CRIT(fmt, ...)
 * Logs a message with level CRITICAL on the global domain with the specified
 * format.
 */
#define EINA_LOG_CRIT(fmt, ...) \
	EINA_LOG(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_CRITICAL, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_ERR(fmt, ...)
 * Logs a message with level ERROR on the global domain with the specified
 * format.
 */
#define EINA_LOG_ERR(fmt, ...) \
	EINA_LOG(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_ERR, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_INFO(fmt, ...)
 * Logs a message with level INFO on the global domain with the specified
 * format.
 */
#define EINA_LOG_INFO(fmt, ...) \
	EINA_LOG(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_INFO, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_WARN(fmt, ...)
 * Logs a message with level WARN on the global domain with the specified
 * format.
 */
#define EINA_LOG_WARN(fmt, ...) \
	EINA_LOG(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_WARN, fmt, ##__VA_ARGS__)

/**
 * @def EINA_LOG_DBG(fmt, ...)
 * Logs a message with level DEBUG on the global domain with the specified
 * format.
 */
#define EINA_LOG_DBG(fmt, ...) \
	EINA_LOG(EINA_LOG_DOMAIN_GLOBAL, EINA_LOG_LEVEL_DBG, fmt, ##__VA_ARGS__)

typedef struct _Eina_Log_Domain Eina_Log_Domain;

struct _Eina_Log_Domain
{
   int level;     /**< Max level to log */
   const char *domain_str; /**< Formatted string with color to print */
   const char *name;       /**< Domain name */

   /* Private */
   Eina_Bool deleted:1; /**< Flags deletion of domain, a free slot */
};

/**
 * @typedef Eina_Log_Level
 * List of available logging levels.
 */

/**
 * @enum _Eina_Log_Level
 * List of available logging levels.
 */
typedef enum _Eina_Log_Level
{
	EINA_LOG_LEVEL_CRITICAL,           /**< Critical log level */
	EINA_LOG_LEVEL_ERR,                /**< Error log level */
	EINA_LOG_LEVEL_WARN,               /**< Warning log level */
	EINA_LOG_LEVEL_INFO,               /**< Information log level */
	EINA_LOG_LEVEL_DBG,                /**< Debug log level */
	EINA_LOG_LEVELS,                   /**< Count of default log levels */
	EINA_LOG_LEVEL_UNKNOWN = (-2147483647-1) /**< Unknown level */
} Eina_Log_Level;

/**
 * @typedef Eina_Log_Print_Cb
 * Type for print callbacks.
 */
typedef void (*Eina_Log_Print_Cb)(const Eina_Log_Domain *d, Eina_Log_Level level,
				  const char *file, const char *fnc, int line,
				  const char *fmt, void *data, va_list args);

/**
 * @var EINA_LOG_OUT_OF_MEMORY
 * Log identifier corresponding to a lack of memory.
 */

EAPI int eina_log_init(void);
EAPI int eina_log_shutdown(void);
EAPI void eina_log_threads_enable(void);

/*
 * Customization
 */
EAPI void eina_log_print_cb_set(Eina_Log_Print_Cb cb, void *data) EINA_ARG_NONNULL(1);
EAPI void eina_log_level_set(Eina_Log_Level level);

/*
 * Logging domains
 */
EAPI int  eina_log_domain_register(const char *name, const char *color) EINA_ARG_NONNULL(1);
EAPI void eina_log_domain_unregister(int domain);

/*
 * Logging functions.
 */
EAPI void eina_log_print(int domain, Eina_Log_Level level, const char *file, const char *function, int line, const char *fmt, ...) EINA_ARG_NONNULL(3, 4, 6) EINA_PRINTF(6, 7) EINA_NOINSTRUMENT;
EAPI void eina_log_vprint(int domain, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, va_list args) EINA_ARG_NONNULL(3, 4, 6) EINA_NOINSTRUMENT;


/*
 * Logging methods (change how logging is done).
 */
EAPI void eina_log_print_cb_stdout(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args);
EAPI void eina_log_print_cb_stderr(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args);
EAPI void eina_log_print_cb_file(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_LOG_H_ */
