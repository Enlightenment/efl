/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

#ifndef EINA_MAGIC_H_
#define EINA_MAGIC_H_

#ifdef EINA_MAGIC_DEBUG

#include "eina_types.h"

#define EINA_MAGIC_NONE            0x1234fedc

#define EINA_MAGIC	Eina_Magic __magic;

#define EINA_MAGIC_SET(d, m)       (d)->__magic = (m)
#define EINA_MAGIC_CHECK(d, m)     ((d) && ((d)->__magic == (m)))
#define EINA_MAGIC_FAIL(d, m)      eina_magic_fail((void*)(d), (d) ? (d)->__magic : 0, (m), __FILE__, __FUNCTION__, __LINE__);

typedef unsigned int               Eina_Magic;

EAPI int eina_magic_string_init(void);
EAPI int eina_magic_string_shutdown(void);

EAPI const char* eina_magic_string_get(Eina_Magic magic);
EAPI void eina_magic_string_set(Eina_Magic magic, const char *magic_name);
EAPI void eina_magic_fail(void *d, Eina_Magic m, Eina_Magic req_m,
			  const char *file, const char *fnc, int line);

#else

#define EINA_MAGIC_NONE            ((void) 0)
#define EINA_MAGIC
#define EINA_MAGIC_SET(d, m)       ((void) 0)
#define EINA_MAGIC_CHECK(d, m)	   (1)
#define EINA_MAGIC_FAIL(d, m)      ((void) 0)

#define eina_magic_string_get(Magic)                  (NULL)
#define eina_magic_string_set(Magic, Magic_Name)      ((void) 0)
#define eina_magic_fail(d, m, req_m, file, fnx, line) ((void) 0)
#define eina_magic_string_init()   do {} while(0)
#define eina_magic_string_shutdown() do {} while(0)

#endif

#endif /* EINA_MAGIC_H_ */
