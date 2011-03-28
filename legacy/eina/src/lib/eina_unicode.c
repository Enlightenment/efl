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

#include <Eina.h>
#include "eina_unicode.h"

/* FIXME: check if sizeof(wchar_t) == sizeof(Eina_Unicode) if so,
 * probably better to use the standard functions */

/* Maybe I'm too tired, but this is the only thing that actually worked. */
const Eina_Unicode _EINA_UNICODE_EMPTY_STRING[1] = {0};
EAPI const Eina_Unicode *EINA_UNICODE_EMPTY_STRING = _EINA_UNICODE_EMPTY_STRING;
/**
 * @brief Same as the standard strcmp just with Eina_Unicode instead of char.
 */
EAPI int
eina_unicode_strcmp(const Eina_Unicode *a, const Eina_Unicode *b)
{
   for (; *a && *a == *b; a++, b++)
      ;
   if (*a == *b)
      return 0;
   else if (*a < *b)
      return -1;
   else
      return 1;
}

/**
 * @brief Same as the standard strcpy just with Eina_Unicode instead of char.
 */
EAPI Eina_Unicode *
eina_unicode_strcpy(Eina_Unicode *dest, const Eina_Unicode *source)
{
   Eina_Unicode *ret = dest;

   while (*source)
      *dest++ = *source++;
   *dest = 0;
   return ret;
}

/**
 * @brief Same as the standard strncpy just with Eina_Unicode instead of char.
 */
EAPI Eina_Unicode *
eina_unicode_strncpy(Eina_Unicode *dest, const Eina_Unicode *source, size_t n)
{
   Eina_Unicode *ret = dest;

   for ( ; n && *source ; n--)
      *dest++ = *source++;
   for (; n; n--)
      *dest++ = 0;
   return ret;
}

/**
 * @brief Same as the standard strlen just with Eina_Unicode instead of char.
 */
EAPI size_t
eina_unicode_strlen(const Eina_Unicode *ustr)
{
   const Eina_Unicode *end;
   for (end = ustr; *end; end++)
      ;
   return end - ustr;
}

/**
 * @brief Returns the length of a Eina_Unicode string, up to a limit.
 *
 * This function returns the number of characters in string, up to a maximum
 * of n.  If the terminating character is not found in the string, it returns
 * n.
 *
 * @param ustr String to search
 * @param n Max length to search
 * @return Number of characters or n.
 */
EAPI size_t
eina_unicode_strnlen(const Eina_Unicode *ustr, int n)
{
   const Eina_Unicode *end;
   const Eina_Unicode *last = ustr + n; /* technically not portable ;-) */
   for (end = ustr; end < last && *end; end++)
      ;
   return end - ustr;
}




/**
 * @brief Same as strdup but cuts on n. Assumes n < len
 * @since 1.1.0
 */
EAPI Eina_Unicode *
eina_unicode_strndup(const Eina_Unicode *text, size_t n)
{
   Eina_Unicode *ustr;

   ustr = (Eina_Unicode *) malloc((n + 1) * sizeof(Eina_Unicode));
   memcpy(ustr, text, n * sizeof(Eina_Unicode));
   ustr[n] = 0;
   return ustr;
}

/**
 * @brief Same as the standard strdup just with Eina_Unicode instead of char.
 */
EAPI Eina_Unicode *
eina_unicode_strdup(const Eina_Unicode *text)
{
   size_t len;

   len = eina_unicode_strlen(text);
   return eina_unicode_strndup(text, len);
}

/**
 * @brief Same as the standard strstr just with Eina_Unicode instead of char.
 */
EAPI Eina_Unicode *
eina_unicode_strstr(const Eina_Unicode *haystack, const Eina_Unicode *needle)
{
   const Eina_Unicode *i, *j;

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

/**
 * @see eina_str_escape()
 */
EAPI Eina_Unicode *
eina_unicode_escape(const Eina_Unicode *str)
{
   Eina_Unicode *s2, *d;
   const Eina_Unicode *s;

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
#define ERROR_REPLACEMENT_BASE  0xDC80
#define ERROR_REPLACEMENT_END   0xDCFF
#define IS_INVALID_BYTE(x)      ((x == 192) || (x == 193) || (x >= 245))
#define IS_CONTINUATION_BYTE(x) ((x & 0xC0) == 0x80)

/**
 * Reads UTF8 bytes from @buf, starting at *@index and returns
 * the decoded code point at iindex offset, and advances iindex
 * to the next code point after this. iindex is always advanced,
 * unless if the advancement is after the NULL.
 * On error: return a codepoint between DC80 to DCFF where the low 8 bits
 *   are the byte's value.
 *
 * @param buf the string
 * @param iindex the index to look at and return by.
 * @return the codepoint found.
 * @since 1.1.0
 */
EAPI Eina_Unicode
eina_unicode_utf8_get_next(const char *buf, int *iindex)
{
   int ind = *iindex;
   Eina_Unicode r;
   unsigned char d;

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
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0x7F) goto error;
        *iindex = ind;
        return r;
     }
   if ((d & 0xf0) == 0xe0)
     { // 3 byte (16bit) - 1110xxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x0f) << 12;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 6;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0x7FF) goto error;
        *iindex = ind;
        return r;
     }
   if ((d & 0xf8) == 0xf0)
     { // 4 byte (21bit) - 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x07) << 18;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 12;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 6;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0xFFFF) goto error;
        *iindex = ind;
        return r;
     }
   if ((d & 0xfc) == 0xf8)
     { // 5 byte (26bit) - 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x03) << 24;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 18;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 12;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 6;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f);
        if (r <= 0x1FFFFF) goto error;
        *iindex = ind;
        return r;
     }
   if ((d & 0xfe) == 0xfc)
     { // 6 byte (31bit) - 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
        r  = (d & 0x01) << 30;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 24;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 18;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 12;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
        r |= (d & 0x3f) << 6;
        if (((d = buf[ind++]) == 0) || IS_INVALID_BYTE(d) ||
            !IS_CONTINUATION_BYTE(d)) goto error;
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

/**
 * Reads UTF8 bytes from @buf, starting at *@iindex and returns
 * the decoded code point at iindex offset, and moves iindex
 * to the previous code point. iindex is always moved, as long
 * as it's not past the start of the string.
 * On error: return a codepoint between DC80 to DCFF where the low 8 bits
 *   are the byte's value.
 *
 * @param buf the string
 * @param iindex the index to look at and return by.
 * @return the codepoint found.
 * @since 1.1.0
 */
EAPI Eina_Unicode
eina_unicode_utf8_get_prev(const char *buf, int *iindex)
{
   int r;
   int ind = *iindex;
   /* First obtain the codepoint at iindex */
   r = eina_unicode_utf8_get_next(buf, &ind);

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

/**
 * Returns the number of unicode characters in the string. That is,
 * the number of Eina_Unicodes it'll take to store this string in
 * an Eina_Unicode string.
 *
 * @param buf the string
 * @return the number of unicode characters (not bytes) in the string
 * @since 1.1.0
 */
EAPI int
eina_unicode_utf8_get_len(const char *buf)
{
   /* returns the number of utf8 characters (not bytes) in the string */
   int i = 0, len = 0;

   while (eina_unicode_utf8_get_next(buf, &i))
        len++;

   return len;
}

/**
 * Converts a utf-8 string to a newly allocated Eina_Unicode string.
 *
 * @param utf the string in utf-8
 * @param _len the length of the returned Eina_Unicode string.
 * @return the newly allocated Eina_Unicode string.
 * @since 1.1.0
 */
EAPI Eina_Unicode *
eina_unicode_utf8_to_unicode(const char *utf, int *_len)
{
   /* FIXME: Should optimize! */
   int len, i;
   int ind;
   Eina_Unicode *buf, *uind;

   len = eina_unicode_utf8_get_len(utf);
   if (_len)
      *_len = len;
   buf = (Eina_Unicode *) calloc(sizeof(Eina_Unicode), (len + 1));
   if (!buf) return buf;

   for (i = 0, ind = 0, uind = buf ; i < len ; i++, uind++)
     {
        *uind = eina_unicode_utf8_get_next(utf, &ind);
     }

   return buf;
}

/**
 * Converts an Eina_Unicode string to a newly allocated utf-8 string.
 *
 * @param uni the Eina_Unicode string
 * @param _len the length byte length of the return utf8 string.
 * @return the newly allocated utf-8 string.
 * @since 1.1.0
 */
EAPI char *
eina_unicode_unicode_to_utf8(const Eina_Unicode *uni, int *_len)
{
   char *buf;
   const Eina_Unicode *uind;
   char *ind;
   int ulen, len;

   ulen = eina_unicode_strlen(uni);
   buf = (char *) calloc(ulen + 1, EINA_UNICODE_UTF8_BYTES_PER_CHAR);

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
   buf = realloc(buf, len + 1);
   buf[len] = '\0';
   if (_len)
      *_len = len;
   return buf;
}



