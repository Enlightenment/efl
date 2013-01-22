/* EINA - EFL data type library
 * Copyright (C) 2013 Cedric Bail
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_unicode.h"
#include "eina_safety_checks.h"

#if EINA_SIZEOF_WCHAR_T >= 4
# include <wchar.h>
typedef wchar_t Eina_Unicode;
#else
# include <inttypes.h>
typedef uint32_t Eina_Unicode;
#endif

#define ERROR_REPLACEMENT_BASE  0xDC80
#define EINA_IS_INVALID_BYTE(x)      ((x == 192) || (x == 193) || (x >= 245))
#define EINA_IS_CONTINUATION_BYTE(x) ((x & 0xC0) == 0x80)

EAPI Eina_Unicode eina_unicode_utf8_get_next(const char *buf, int *iindex)
{
   return eina_unicode_utf8_next_get(buf, iindex);
}

