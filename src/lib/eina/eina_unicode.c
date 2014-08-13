/* EINA - EFL data type library
 * Copyright (C) 2010 Tom Hacohen,
 *		Brett Nash
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
#include <string.h>

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_unicode.h"

/* FIXME: check if sizeof(wchar_t) == sizeof(Eina_Unicode) if so,
 * probably better to use the standard functions */

/* Maybe I'm too tired, but this is the only thing that actually worked. */
const Eina_Unicode _EINA_UNICODE_EMPTY_STRING[1] = {0};
EAPI const Eina_Unicode *EINA_UNICODE_EMPTY_STRING = _EINA_UNICODE_EMPTY_STRING;
EAPI int
eina_unicode_strcmp(const Eina_Unicode *a, const Eina_Unicode *b)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(a, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(b, -1);

   for (; *a && *a == *b; a++, b++)
      ;
   if (*a == *b)
      return 0;
   else if (*a < *b)
      return -1;
   else
      return 1;
}

EAPI Eina_Unicode *
eina_unicode_strcpy(Eina_Unicode *dest, const Eina_Unicode *source)
{
   Eina_Unicode *ret = dest;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dest, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(source, NULL);

   while (*source)
      *dest++ = *source++;
   *dest = 0;
   return ret;
}

EAPI Eina_Unicode *
eina_unicode_strncpy(Eina_Unicode *dest, const Eina_Unicode *source, size_t n)
{
   Eina_Unicode *ret = dest;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dest, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(source, NULL);

   for ( ; n && *source ; n--)
      *dest++ = *source++;
   for (; n; n--)
      *dest++ = 0;
   return ret;
}

EAPI size_t
eina_unicode_strlen(const Eina_Unicode *ustr)
{
   const Eina_Unicode *end;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ustr, 0);

   for (end = ustr; *end; end++)
      ;
   return end - ustr;
}

EAPI size_t
eina_unicode_strnlen(const Eina_Unicode *ustr, int n)
{
   const Eina_Unicode *end;
   const Eina_Unicode *last = ustr + n; /* technically not portable ;-) */

   EINA_SAFETY_ON_NULL_RETURN_VAL(ustr, 0);

   for (end = ustr; end < last && *end; end++)
      ;
   return end - ustr;
}




EAPI Eina_Unicode *
eina_unicode_strndup(const Eina_Unicode *text, size_t n)
{
   Eina_Unicode *ustr;

   EINA_SAFETY_ON_NULL_RETURN_VAL(text, NULL);

   ustr = malloc((n + 1) * sizeof(Eina_Unicode));
   memcpy(ustr, text, n * sizeof(Eina_Unicode));
   ustr[n] = 0;
   return ustr;
}

EAPI Eina_Unicode *
eina_unicode_strdup(const Eina_Unicode *text)
{
   size_t len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(text, NULL);

   len = eina_unicode_strlen(text);
   return eina_unicode_strndup(text, len);
}

EAPI Eina_Unicode *
eina_unicode_strstr(const Eina_Unicode *haystack, const Eina_Unicode *needle)
{
   const Eina_Unicode *i, *j;

   EINA_SAFETY_ON_NULL_RETURN_VAL(haystack, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(needle, NULL);

   for (i = haystack; *i; i++)
     {
        haystack = i; /* set this location as the base position */
        for (j = needle; *j && *i && *j == *i; j++, i++)
           ;

        if (!*j) /*if we got to the end of j this means we got a full match */
          {
             return (Eina_Unicode *)haystack; /* return the new base position */
          }
     }

   return NULL;
}

EAPI Eina_Unicode *
eina_unicode_escape(const Eina_Unicode *str)
{
   Eina_Unicode *s2, *d;
   const Eina_Unicode *s;

   EINA_SAFETY_ON_NULL_RETURN_VAL(str, NULL);

   s2 = malloc((eina_unicode_strlen(str) * 2) + 1);
   if (!s2)
      return NULL;

   for (s = str, d = s2; *s != 0; s++, d++)
     {
        if ((*s == ' ') || (*s == '\\') || (*s == '\''))
          {
             *d = '\\';
             d++;
          }

        *d = *s;
     }
   *d = 0;
   return s2;
}

/* UTF-8 Handling */

#define EINA_UNICODE_UTF8_BYTES_PER_CHAR 6
/* The replacement range that will be used for bad utf8 chars. */
#define ERROR_REPLACEMENT_END   0xDCFF

EAPI Eina_Unicode
_eina_unicode_utf8_next_get(int ind,
                            unsigned char d,
                            const char *buf, 
			    int *iindex)
{
   Eina_Unicode r;

   if ((d & 0xf0) == 0xe0)
     { // 3 byte (16bit) - 1110xxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x0f) << 12;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 6;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0x7FF) goto error;
        *iindex = ind;
        return r;
     }
   if ((d & 0xf8) == 0xf0)
     { // 4 byte (21bit) - 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x07) << 18;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 12;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 6;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0xFFFF) goto error;
        *iindex = ind;
        return r;
     }
   if ((d & 0xfc) == 0xf8)
     { // 5 byte (26bit) - 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x03) << 24;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 18;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 12;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 6;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0x1FFFFF) goto error;
        *iindex = ind;
        return r;
     }
   if ((d & 0xfe) == 0xfc)
     { // 6 byte (31bit) - 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x01) << 30;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 24;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 18;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 12;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 6;
        if (((d = buf[ind++]) == 0) || EINA_IS_INVALID_BYTE(d) ||
            !EINA_IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0x3FFFFFF) goto error;
        *iindex = ind;
        return r;
     }

/* Gets here where there was an error and we want to replace the char
 * we just use the invalid unicode codepoints 8 lower bits represent
 * the original char */
error:
   d = buf[*iindex];
   (*iindex)++;
   return ERROR_REPLACEMENT_BASE | d;
}

EAPI Eina_Unicode
eina_unicode_utf8_get_prev(const char *buf, int *iindex)
{
   int r, ind;

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, 0);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iindex, 0);

   ind = *iindex;
   /* First obtain the codepoint at iindex */
   r = eina_unicode_utf8_next_get(buf, &ind);

   /* although when ind == 0 there's no previous char, we still want to get
    * the current char */
   if (*iindex <= 0)
     return r;

   /* Next advance iindex to previous codepoint */
   ind = *iindex;
   ind--;
   while ((ind > 0) && ((buf[ind] & 0xc0) == 0x80))
     ind--;

   *iindex = ind;
   return r;
}

EAPI int
eina_unicode_utf8_get_len(const char *buf)
{
   /* returns the number of utf8 characters (not bytes) in the string */
   int i = 0, len = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, 0);

   while (eina_unicode_utf8_next_get(buf, &i))
        len++;

   return len;
}

EAPI Eina_Unicode *
eina_unicode_utf8_to_unicode(const char *utf, int *_len)
{
   /* FIXME: Should optimize! */
   int len, i;
   int ind;
   Eina_Unicode *buf, *uind;

   EINA_SAFETY_ON_NULL_RETURN_VAL(utf, NULL);

   len = eina_unicode_utf8_get_len(utf);
   if (_len) *_len = len;
   buf = malloc(sizeof(Eina_Unicode) * (len + 1));
   if (!buf) return buf;

   for (i = 0, ind = 0, uind = buf ; i < len ; i++, uind++)
     {
        *uind = eina_unicode_utf8_next_get(utf, &ind);
     }
   *uind = 0;

   return buf;
}

EAPI char *
eina_unicode_unicode_to_utf8(const Eina_Unicode *uni, int *_len)
{
   char *buf, *buf2;
   const Eina_Unicode *uind;
   char *ind;
   int ulen, len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(uni, NULL);

   ulen = eina_unicode_strlen(uni);
   buf = malloc((ulen + 1) * EINA_UNICODE_UTF8_BYTES_PER_CHAR);
   if (!buf) return NULL;

   len = 0;
   for (uind = uni, ind = buf ; *uind ; uind++)
     {
        if (*uind <= 0x7F) /* 1 byte char */
          {
             *ind++ = *uind;
             len += 1;
          }
        else if (*uind <= 0x7FF) /* 2 byte char */
          {
             *ind++ = 0xC0 | (unsigned char) (*uind >> 6);
             *ind++ = 0x80 | (unsigned char) (*uind & 0x3F);
             len += 2;
          }
        else if (*uind <= 0xFFFF) /* 3 byte char */
          {
             /* If it's a special replacement codepoint */
             if (*uind >= ERROR_REPLACEMENT_BASE &&
                 *uind <= ERROR_REPLACEMENT_END)
               {
                  *ind++ = *uind & 0xFF;
                  len += 1;
               }
             else
               {
                  *ind++ = 0xE0 | (unsigned char) (*uind >> 12);
                  *ind++ = 0x80 | (unsigned char) ((*uind >> 6) & 0x3F);
                  *ind++ = 0x80 | (unsigned char) (*uind & 0x3F);
                  len += 3;
               }
          }
        else if (*uind <= 0x1FFFFF) /* 4 byte char */
          {
             *ind++ = 0xF0 | (unsigned char) ((*uind >> 18) & 0x07);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 12) & 0x3F);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 6) & 0x3F);
             *ind++ = 0x80 | (unsigned char) (*uind & 0x3F);
             len += 4;
          }
        else if (*uind <= 0x3FFFFFF) /* 5 byte char */
          {
             *ind++ = 0xF8 | (unsigned char) ((*uind >> 24) & 0x03);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 18) & 0x3F);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 12) & 0x3F);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 6) & 0x3F);
             *ind++ = 0x80 | (unsigned char) (*uind & 0x3F);
             len += 5;
          }
        else if (*uind <= 0x7FFFFFFF) /* 6 byte char */
          {
             *ind++ = 0xFC | (unsigned char) ((*uind >> 30) & 0x01);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 24) & 0x3F);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 18) & 0x3F);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 12) & 0x3F);
             *ind++ = 0x80 | (unsigned char) ((*uind >> 6) & 0x3F);
             *ind++ = 0x80 | (unsigned char) (*uind & 0x3F);
             len += 6;
          }
        else /* error */
          {
             /* Do something */
          }
     }
   buf2 = realloc(buf, len + 1);
   if (!buf2)
     {
        free(buf);
        return NULL;
     }
   buf2[len] = 0;
   if (_len) *_len = len;
   return buf;
}



