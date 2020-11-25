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

EINA_API Eina_Unicode eina_unicode_utf8_get_next(const char *buf, int *iindex)
{
   return eina_unicode_utf8_next_get(buf, iindex);
}

EINA_API unsigned int
eina_mempool_alignof(unsigned int size)
{
   unsigned int align;
   unsigned int mask;

   if (EINA_UNLIKELY(size <= 2))
     {
        align = 1;
        mask = 0x1;
     }
   else if (EINA_UNLIKELY(size < 8))
     {
        align = 2;
        mask = 0x3;
     }
   else
#if __WORDSIZE == 32
     {
        align = 3;
        mask = 0x7;
     }
#else
   if (EINA_UNLIKELY(size < 16))
     {
        align = 3;
        mask = 0x7;
     }
   else
     {
        align = 4;
        mask = 0x15;
     }
#endif

   return ((size >> align) + (size & mask ? 1 : 0)) << align;
}
