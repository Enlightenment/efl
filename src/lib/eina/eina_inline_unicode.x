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

#ifndef EINA_INLINE_UNICODE_
# define EINA_INLINE_UNICODE_

EAPI Eina_Unicode _eina_unicode_utf8_next_get(int ind,
					      unsigned char d,
					      const char *buf, 
					      int *iindex);

#define ERROR_REPLACEMENT_BASE  0xDC80
#define EINA_IS_INVALID_BYTE(x)      ((x == 192) || (x == 193) || (x >= 245))
#define EINA_IS_CONTINUATION_BYTE(x) ((x & 0xC0) == 0x80)

static inline Eina_Unicode
eina_unicode_utf8_next_get(const char *buf, int *iindex)
{
   int ind;
   Eina_Unicode r;
   unsigned char d;

   ind = *iindex;

   /* if this char is the null terminator, exit */
   if ((d = buf[ind++]) == 0) return 0;

   if ((d & 0x80) == 0)
     { // 1 byte (7bit) - 0xxxxxxx
        *iindex = ind;
        return d;
     }
   
   if ((d & 0xe0) == 0xc0)
     { // 2 byte (11bit) - 110xxxxx 10xxxxxx
        r  = (d & 0x1f) << 6;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0x7F) goto error;
        *iindex = ind;
        return r;
     }

   return _eina_unicode_utf8_next_get(ind, d, buf, iindex);

/* Gets here where there was an error and we want to replace the char
 * we just use the invalid unicode codepoints 8 lower bits represent
 * the original char */
error:
   d = buf[*iindex];
   (*iindex)++;
   return ERROR_REPLACEMENT_BASE | d;
}

#endif
