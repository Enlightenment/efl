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

#include "eina_types.h"

#define EINA_ERROR_PERR(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_ERR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EINA_ERROR_PINFO(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EINA_ERROR_PWARN(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_WARN, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define EINA_ERROR_PDBG(fmt, ...) \
	eina_error_print(EINA_ERROR_LEVEL_DBG, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum _Eina_Error_Level
{
	EINA_ERROR_LEVEL_ERR,
	EINA_ERROR_LEVEL_WARN,
	EINA_ERROR_LEVEL_INFO,
	EINA_ERROR_LEVEL_DBG,
	EINA_ERROR_LEVELS
} Eina_Error_Level;

typedef int Eina_Error;

EAPI extern int EINA_ERROR_OUT_OF_MEMORY;

EAPI int eina_error_init(void);
EAPI int eina_error_shutdown(void);
EAPI Eina_Error eina_error_register(const char *msg);
EAPI Eina_Error eina_error_get(void);
EAPI void eina_error_set(Eina_Error err);
EAPI const char * eina_error_msg_get(Eina_Error error);
EAPI void eina_error_print(Eina_Error_Level level, const char *file,
		const char *function, int line, const char *fmt, ...);
EAPI void eina_error_log_level_set(Eina_Error_Level level);

#endif /*EINA_ERROR_H_*/
