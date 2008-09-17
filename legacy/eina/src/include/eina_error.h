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

#ifndef EINA_ERROR_H_
#define EINA_ERROR_H_

#include <stdarg.h>

#include "eina_types.h"

/**
 * @addtogroup Eina_Tools_Group Tools Modules
 *
 * @{
 */

/**
 * @defgroup Eina_Error_Group Error Functions
 *
 * @{
 */

/**
 * @def EINA_ERROR_PERR(fmt, ...)
 * Print the error message described with the formatted string @a fmt
 * using the current print callback, file function and line, with the
 * error level EINA_ERROR_LEVEL_ERR.
 */
#define EINA_ERROR_PERR(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @def EINA_ERROR_PINFO(fmt, ...)
 * Print the error message described with the formatted string @a fmt
 * using the current print callback, file function and line, with the
 * error level EINA_ERROR_LEVEL_INFO.
 */
#define EINA_ERROR_PINFO(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @def EINA_ERROR_PWARN(fmt, ...)
 * Print the error message described with the formatted string @a fmt
 * using the current print callback, file function and line, with the
 * error level EINA_ERROR_LEVEL_WARN.
 */
#define EINA_ERROR_PWARN(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @def EINA_ERROR_PDBG(fmt, ...)
 * Print the error message described with the formatted string @a fmt
 * using the current print callback, file function and line, with the
 * error level EINA_ERROR_LEVEL_DBG.
 */
#define EINA_ERROR_PDBG(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_DBG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @typedef Eina_Error_Level
 * List of available error levels.
 */

/**
 * @enum _Eina_Error_Level
 * List of available error levels.
 */
typedef enum _Eina_Error_Level
{
	EINA_ERROR_LEVEL_ERR,   /**< Error error level */
	EINA_ERROR_LEVEL_WARN,  /**< Warning error level */
	EINA_ERROR_LEVEL_INFO,  /**< Information error level */
	EINA_ERROR_LEVEL_DBG,   /**< Debug error level */
	EINA_ERROR_LEVELS       /**< Count of error level */
} Eina_Error_Level;

/**
 * @typedef Eina_Error
 * Error type.
 */
typedef int Eina_Error;

/**
 * @typedef Eina_Error_Print_Cb
 * Type for print callbacks.
 */
typedef void (*Eina_Error_Print_Cb)(Eina_Error_Level level, const char *file,
                const char *fnc, int line, const char *fmt, void *data,
		va_list args);

/**
 * @var EINA_ERROR_OUT_OF_MEMORY
 * Error identifier corresponding to a lack of memory.
 */
EAPI extern Eina_Error EINA_ERROR_OUT_OF_MEMORY;

EAPI int eina_error_init(void);
EAPI int eina_error_shutdown(void);
EAPI Eina_Error eina_error_msg_register(const char *msg);
EAPI Eina_Error eina_error_get(void);
EAPI void eina_error_set(Eina_Error err);
EAPI const char * eina_error_msg_get(Eina_Error error);
EAPI void eina_error_print(Eina_Error_Level level, const char *file,
		const char *function, int line, const char *fmt, ...);
EAPI void eina_error_print_cb_stdout(Eina_Error_Level level, const char *file,
                const char *fnc, int line, const char *fmt, void *data,
		va_list args);
EAPI void eina_error_print_cb_file(Eina_Error_Level level, const char *file,
                const char *fnc, int line, const char *fmt, void *data,
                va_list args);
EAPI void eina_error_print_cb_set(Eina_Error_Print_Cb cb, void *data);
EAPI void eina_error_log_level_set(Eina_Error_Level level);

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_ERROR_H_ */
