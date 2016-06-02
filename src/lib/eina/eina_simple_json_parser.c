/* EINA - EFL data type library
 * Copyright (C) 2016 Expertise Solutions
 *                    Larry Lira Jr
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

#include <strings.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_alloca.h"
#include "eina_log.h"
#include "eina_mempool.h"
#include "eina_stringshare.h"
#include "eina_strbuf.h"
#include "eina_simple_json_parser.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

struct _eina_simple_json_content
{
    const char *buf;
    unsigned len;
};

static inline const char *
_eina_simple_json_whitespace_find(const char *itr, const char *itr_end)
{
   for (; itr < itr_end; itr++)
     if (isspace((unsigned char)*itr)) break;
   return itr;
}

static inline const char *
_eina_simple_json_whitespace_skip(const char *itr, const char *itr_end)
{
   for (; itr < itr_end; itr++)
     if (!isspace((unsigned char)*itr)) break;
   return itr;
}

static inline const char *
_eina_simple_json_start_find(const char *itr, const char *itr_end)
{
   for (; itr < itr_end; itr++)
       if ((*itr == '{') || (*itr == '[')) break;
   return itr;
}

static inline const char *
_eina_simple_json_object_start_find(const char *itr, const char *itr_end)
{
   return memchr(itr, '{', itr_end - itr);
}

static inline const char *
_eina_simple_json_array_start_find(const char *itr, const char *itr_end)
{
   return memchr(itr, '[', itr_end - itr);
}

static inline unsigned
_eina_simple_json_object_length(const char *buf, const char *bufend)
{
   Eina_Bool inside_quote = EINA_FALSE;
   const char *itr = buf;
   int count = 0;
   if (*itr != '{') return 0;

   for (; itr < bufend; itr++)
     {
        if (*itr == '"') inside_quote = !inside_quote;
        if (!inside_quote)
          {
             if ((*itr == '{')) count++;
             else if ((*itr == '}'))
               {
                  if (count == 1)
                    return (itr - buf) + 1;
                  count--;
               }
          }
     }
   return 0;
}

static inline unsigned
_eina_simple_json_array_length(const char *buf, const char *bufend)
{
   Eina_Bool inside_quote = EINA_FALSE;
   const char *itr = buf;
   int count = 0;
   if (*itr != '[') return 0;

   for (; itr < bufend; itr++)
     {
        if (*itr == '"') inside_quote = !inside_quote;
        else if (!inside_quote)
          {
             if ((*itr == '[')) count++;
             else if ((*itr == ']'))
               {
                  if (count == 1)
                    return (itr - buf) + 1;
                  count--;
               }
          }
     }
   return 0;
}

static inline unsigned
_eina_simple_json_string_length(const char *buf, const char *bufend)
{
   const char *itr = buf;

   if (itr[0] == '"') return 0;

   for (; itr < bufend; itr++)
     {
        if (*itr == '\\')
          {
             itr++;
             continue;
          }

        if (*itr == '"')
          return itr - buf;
     }
   return 0;
}

static inline unsigned
_eina_simple_json_number_end_find(const char *buf, const char *bufend, Eina_Simple_JSON_Value_Type *type)
{
   const char *itr = buf;
   Eina_Bool is_float = EINA_FALSE;
   Eina_Bool has_e = EINA_FALSE;

   *type = EINA_SIMPLE_JSON_VALUE_INT;

   if (itr[0] == '-')
     {
        if ((itr+1 >= bufend) || (!isdigit((unsigned char)itr[1])))
           return 0;

        itr++;
     }

   for (; itr < bufend; itr++)
     {
        if ((*itr == 'e') || (*itr == 'E'))
          {
             if ((itr+1 >= bufend) || has_e == EINA_TRUE) break;
             has_e = EINA_TRUE;

             if ((itr[1] == '+') || (itr[1] == '-'))
               {
                  itr++;
                  if ((itr+1 < bufend) && (isdigit((unsigned char)itr[1])))
                    continue;

                  break;
               }
             else if (isdigit((unsigned char)itr[1])) continue;
             break;
          }

        if (*itr == '.')
          {
             if ((has_e) || (is_float) || (itr+1 >= bufend) || (!isdigit((unsigned char)itr[1])))
                break;

             *type = EINA_SIMPLE_JSON_VALUE_DOUBLE;
             is_float = EINA_TRUE;
             continue;
          }

        if (!isdigit((unsigned char)*itr))
          return (itr - buf);
     }
   return 0;
}

static inline Eina_Simple_JSON_Value_Type
_eina_simple_json_content_find(const char **buf, const char *bufend, unsigned *len)
{
   const char *itr = *buf;
   Eina_Simple_JSON_Value_Type type = EINA_SIMPLE_JSON_VALUE_ERROR;
   *len = 0;

   for (; itr < bufend; itr++)
     {
        if (!isspace(itr[0]))
          break;
     }

   if (itr == bufend)
     {
       *buf = itr;
       return type;
     }

   if (itr[0] == '"')
     {
        type = EINA_SIMPLE_JSON_VALUE_STRING;
        itr++;
        *len = _eina_simple_json_string_length(itr, bufend);
     }
   else if (itr[0] == '{')
     {
        type = EINA_SIMPLE_JSON_VALUE_OBJECT;
        *len = _eina_simple_json_object_length(itr, bufend);
     }
   else if (itr[0] == '[')
     {
        type = EINA_SIMPLE_JSON_VALUE_ARRAY;
        *len = _eina_simple_json_array_length(itr, bufend);
     }
   else if (((itr[0] == '-' ) && (itr+1 < bufend) && (isdigit((unsigned char)itr[1])))
                   || (isdigit((unsigned char)*itr)))
     {
        type = EINA_SIMPLE_JSON_VALUE_INT;
        *len = _eina_simple_json_number_end_find(itr, bufend, &type);
     }
   else if ((itr + sizeof("true") - 1 < bufend)
                   && (!memcmp(itr, "true", sizeof("true") - 1 )))
     {
        type = EINA_SIMPLE_JSON_VALUE_BOOLEAN;
        *len = sizeof("true") - 1;
     }
   else if ((itr + sizeof("false") - 1 < bufend)
                   && (!memcmp(itr, "false", sizeof("false") - 1 )))
     {
        type = EINA_SIMPLE_JSON_VALUE_BOOLEAN;
        *len = sizeof("false") - 1;
     }
   else if ((itr + sizeof("null") - 1 < bufend)
                   && (!memcmp(itr, "null", sizeof("null") - 1 )))
     {
        type = EINA_SIMPLE_JSON_VALUE_NULL;
        *len = sizeof("null") - 1;
     }

   if (itr + *len >= bufend)
     {
        *len = 0;
        return EINA_SIMPLE_JSON_VALUE_ERROR;
     }

   *buf = itr;
   return type;
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


EAPI Eina_Bool
eina_simple_json_parse(const char *buf, unsigned buflen,
                                Eina_Simple_JSON_Cb func, const void *data)
{
   const char *itr = buf, *itr_end = buf + buflen;

   if (!buf) return EINA_FALSE;
   if (!func) return EINA_FALSE;

#define CB(type, start, l)                                 \
   do                                                      \
     {                                                     \
        Eina_Bool _ret;                                    \
        _ret = func((void*)data, type, start, l);          \
        if (!_ret) return EINA_FALSE;                      \
     }                                                     \
   while (0)

   while (itr < itr_end)
     {
        unsigned len = 0;
        if (itr[0] == '{' || itr[0] == '[')
          {
             Eina_Simple_JSON_Type type;

             if (itr + 1 >= itr_end)
               {
                  CB(EINA_SIMPLE_JSON_ERROR, NULL, 0);
                  return EINA_FALSE;
               }

             if (itr[0] == '{')
               {
                  len = _eina_simple_json_object_length(itr, itr_end);
                  type = EINA_SIMPLE_JSON_OBJECT;
               }
             else
               {
                  len = _eina_simple_json_array_length(itr, itr_end);
                  type = EINA_SIMPLE_JSON_ARRAY;
               }

             if (len <= 1)
               {
                  CB(EINA_SIMPLE_JSON_ERROR, NULL, 0);
                  return EINA_FALSE;
               }

             CB(type, itr, len);
             itr = itr + len;
          }

        itr = _eina_simple_json_start_find(itr, itr_end);
     }

#undef CB

   return EINA_TRUE;
}

EAPI Eina_Bool
eina_simple_json_object_parse(const char *buf, unsigned buflen,
                        Eina_Simple_JSON_Object_Cb func, const void *data)
{
   Eina_Simple_JSON_Value_Type type;
   const char *itr_end = buf + buflen;
   const char *k, *itr = buf;
   unsigned len;
   char *key;

   if (!buf || *buf != '{') return EINA_FALSE;
   if (itr == itr_end) return EINA_TRUE;

   do
     {
        itr = memchr(itr, '"', itr_end - itr);
        if (!itr)
          return EINA_TRUE; // We got an empty object
        k = ++itr;
        itr = memchr(itr, '"', itr_end - itr);
        if (!itr)
          return EINA_FALSE; // Malformed key.

        len = itr - k;
        key = alloca(len + 1);
        memcpy(key, k, len);
        key[len] = '\0';

        itr = memchr(itr, ':', itr_end - itr);
        if ((itr == NULL) || itr >= itr_end )
          return EINA_FALSE;

        itr++;

        len = 0;
        type = _eina_simple_json_content_find(&itr, itr_end, &len);

        if (func((void *)data, type, key, itr, len) == EINA_FALSE)
          {
             fprintf(stderr, "parse end func return false\n");
             return EINA_FALSE;
          }

        itr = itr + len;
        itr = memchr(itr, ',', itr_end - itr);
     }
   while (itr != NULL || itr >= itr_end);

   return EINA_TRUE;
}


EAPI Eina_Bool
eina_simple_json_array_parse(const char *buf, unsigned buflen,
                        Eina_Simple_JSON_Array_Cb func, const void *data)
{
   const char *itr = buf;
   const char *itr_end = buf + buflen;
   Eina_Simple_JSON_Value_Type type;
   unsigned itrlen = 0;

   if (!buf || *buf != '[') return EINA_FALSE;
   if (itr >= itr_end) return EINA_TRUE;

   do
     {
        itr++;
        type = _eina_simple_json_content_find(&itr, itr_end, &itrlen);

        if (!func((void *)data, type, itr, itrlen))
          return EINA_FALSE;

        itr = itr + itrlen;
        itr = memchr(itr, ',', itr_end - itr);
     }
   while (itr != NULL || itr >= itr_end);

   return EINA_TRUE;
}
