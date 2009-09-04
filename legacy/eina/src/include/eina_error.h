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
#include "eina_log.h"

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Error_Group Error
 *
 * @{
 */

/**
 * @deprecated use EINA_LOG_ERR() instead.
 * @def EINA_ERROR_PERR(fmt, ...)
 * Print the error message described with the formatted string @a fmt
 * using the current print callback, file function and line, with the
 * error level EINA_ERROR_LEVEL_ERR.
 */
#define EINA_ERROR_PERR(fmt, ...) \
	eina_error_print(EINA_LOG_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @deprecated use EINA_LOG_INFO() instead.
 * @def EINA_ERROR_PINFO(fmt, ...)
 * Print the error message described with the formatted string @a fmt
 * using the current print callback, file function and line, with the
 * error level EINA_ERROR_LEVEL_INFO.
 */
#define EINA_ERROR_PINFO(fmt, ...) \
	eina_error_print(EINA_LOG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @deprecated use EINA_LOG_WARN() instead.
 * @def EINA_ERROR_PWARN(fmt, ...)
 * Print the error message described with the formatted string @a fmt
 * using the current print callback, file function and line, with the
 * error level EINA_ERROR_LEVEL_WARN.
 */
#define EINA_ERROR_PWARN(fmt, ...) \
	eina_error_print(EINA_LOG_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @deprecated use EINA_LOG_DBG() instead.
 * @def EINA_ERROR_PDBG(fmt, ...)
 * Print the error message described with the formatted string @a fmt
 * using the current print callback, file function and line, with the
 * error level EINA_ERROR_LEVEL_DBG.
 */
#define EINA_ERROR_PDBG(fmt, ...) \
	eina_error_print(EINA_LOG_LEVEL_DBG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)


/**
 * @typedef Eina_Error
 * Error type.
 */
typedef int Eina_Error;

/**
 * @var EINA_ERROR_OUT_OF_MEMORY
 * Error identifier corresponding to a lack of memory.
 */
EAPI extern Eina_Error EINA_ERROR_OUT_OF_MEMORY;

EAPI int eina_error_init(void);
EAPI int eina_error_shutdown(void);
EAPI Eina_Error eina_error_msg_register(const char *msg) EINA_ARG_NONNULL(1);
EAPI Eina_Error eina_error_get(void);
EAPI void eina_error_set(Eina_Error err);
EAPI const char * eina_error_msg_get(Eina_Error error) EINA_PURE;
EAPI void eina_error_print(int level, const char *file,
	        const char *function, int line, const char *fmt, ...) EINA_ARG_NONNULL(2, 3, 5) EINA_PRINTF(5, 6) EINA_DEPRECATED;
EAPI void eina_error_vprint(int level, const char *file,
	        const char *fnc, int line, const char *fmt, va_list args) EINA_ARG_NONNULL(2, 3, 5) EINA_DEPRECATED;
EAPI void eina_error_log_level_set(int level) EINA_DEPRECATED;

/**
 * @}
 */

/**
 * @}
 */

#endif /* EINA_ERROR_H_ */
