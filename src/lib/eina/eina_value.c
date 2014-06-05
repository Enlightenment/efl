/* EINA - EFL data type library
 * Copyright (C) 2012 ProFUSION embedded systems
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

#include <stdio.h> /* asprintf() */
#include <inttypes.h> /* PRId64 and PRIu64 */
#include <sys/time.h> /* struct timeval */

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_alloca.h"
#include "eina_log.h"
#include "eina_strbuf.h"
#include "eina_mempool.h"
#include "eina_lock.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_value.h"
/* no model for now
#include "eina_model.h" // uses eina_value.h
 */

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

static Eina_Mempool *_eina_value_mp = NULL;
static Eina_Hash *_eina_value_inner_mps = NULL;
static Eina_Lock _eina_value_inner_mps_lock;
static char *_eina_value_mp_choice = NULL;
static int _eina_value_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_value_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_value_log_dom, __VA_ARGS__)

static const unsigned char eina_value_uchar_max = 255U;
static const char eina_value_char_max =  127;
static const signed char eina_value_char_min = -127 - 1;

static const unsigned short eina_value_ushort_max = 65535U;
static const short eina_value_short_max =  32767;
static const short eina_value_short_min = -32767 - 1;

static const unsigned int eina_value_uint_max = 4294967295U;
static const int eina_value_int_max =  2147483647;
static const int eina_value_int_min = -2147483647 - 1;

static const uint64_t eina_value_uint64_max = 18446744073709551615ULL;
static const int64_t eina_value_int64_max =  9223372036854775807LL;
static const int64_t eina_value_int64_min = -9223372036854775807LL - 1LL;

#if __WORDSIZE == 64
static const unsigned long eina_value_ulong_max = 18446744073709551615ULL;
static const long eina_value_long_max =  9223372036854775807LL;
static const long eina_value_long_min = -9223372036854775807LL - 1LL;
#else
static const unsigned long eina_value_ulong_max = 4294967295U;
static const long eina_value_long_max =  2147483647;
static const long eina_value_long_min = -2147483647 - 1;
#endif


static Eina_Bool
_eina_value_type_uchar_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   unsigned char *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const unsigned char *s = src;
   unsigned char *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_uchar_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const unsigned char *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_uchar_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const unsigned char v = *(const unsigned char *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned char)eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%hhu", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   unsigned char *tmem = mem;
   *tmem = va_arg(args, unsigned int); /* char is promoted to int for va_arg */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   unsigned char *tmem = mem;
   const unsigned char *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const unsigned char *tmem = mem;
   unsigned char *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   unsigned short *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const unsigned short *s = src;
   unsigned short *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_ushort_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const unsigned short *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_ushort_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const unsigned short v = *(const unsigned short *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned char)eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned short)eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%hu", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   unsigned short *tmem = mem;
   *tmem = va_arg(args, unsigned int); /* short is promoted to int for va_arg */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   unsigned short *tmem = mem;
   const unsigned short *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const unsigned short *tmem = mem;
   unsigned short *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   unsigned int *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const unsigned int *s = src;
   unsigned int *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_uint_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const unsigned int *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_uint_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const unsigned int v = *(const unsigned int *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned char)eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned short)eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned int)eina_value_int_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%u", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   unsigned int *tmem = mem;
   *tmem = va_arg(args, unsigned int);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   unsigned int *tmem = mem;
   const unsigned int *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const unsigned int *tmem = mem;
   unsigned int *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   unsigned long *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const unsigned long *s = src;
   unsigned long *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_ulong_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const unsigned long *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_ulong_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const unsigned long v = *(const unsigned long *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_uint_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned char)eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned short)eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned int)eina_value_int_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned long)eina_value_long_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%lu", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   unsigned long *tmem = mem;
   *tmem = va_arg(args, unsigned long);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   unsigned long *tmem = mem;
   const unsigned long *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const unsigned long *tmem = mem;
   unsigned long *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   uint64_t *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const uint64_t *s = src;
   uint64_t *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_uint64_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const uint64_t *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_uint64_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const uint64_t v = *(const uint64_t *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v > eina_value_uint_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        if (EINA_UNLIKELY((sizeof(other_mem) != sizeof(v)) &&
                          (v > eina_value_ulong_max)))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned char)eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned short)eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned int)eina_value_int_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        if (EINA_UNLIKELY(v > (unsigned long)eina_value_long_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        if (EINA_UNLIKELY(v > (uint64_t)eina_value_int64_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%"PRIu64, v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   uint64_t *tmem = mem;
   *tmem = va_arg(args, uint64_t);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   uint64_t *tmem = mem;
   const uint64_t *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const uint64_t *tmem = mem;
   uint64_t *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   char *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const char *s = src;
   char *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_char_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const char *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_char_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const signed char v = *(const signed char *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%hhd", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   char *tmem = mem;
   *tmem = va_arg(args, int); /* char is promoted to int for va_arg */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   char *tmem = mem;
   const char *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const char *tmem = mem;
   char *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   short *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const short *s = src;
   short *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_short_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const short *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_short_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const short v = *(const short *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_char_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%hd", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   short *tmem = mem;
   *tmem = va_arg(args, int); /* short int is promoted to int for va_arg */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   short *tmem = mem;
   const short *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const short *tmem = mem;
   short *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   int *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const int *s = src;
   int *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_int_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const int *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_int_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const int v = *(const int *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_char_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_short_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%d", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   int *tmem = mem;
   *tmem = va_arg(args, int);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   int *tmem = mem;
   const int *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const int *tmem = mem;
   int *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   long *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const long *s = src;
   long *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_long_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const long *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_long_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const long v = *(const long *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY((unsigned long) v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY((unsigned long) v > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY((unsigned long) v > eina_value_uint_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_char_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_short_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_int_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_int_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%ld", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   long *tmem = mem;
   *tmem = va_arg(args, long);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   long *tmem = mem;
   const long *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const long *tmem = mem;
   long *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   int64_t *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const int64_t *s = src;
   int64_t *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_int64_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const int64_t *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_int64_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const int64_t v = *(const int64_t *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uint_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY((sizeof(other_mem) != sizeof(v)) &&
                          (v > eina_value_ulong_max)))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_char_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_short_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_int_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_int_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_long_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_long_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%"PRId64, v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   int64_t *tmem = mem;
   *tmem = va_arg(args, int64_t);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   int64_t *tmem = mem;
   const int64_t *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const int64_t *tmem = mem;
   int64_t *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   float *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const float *s = src;
   float *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_float_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const float *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_float_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const float v = *(const float *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uint_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY((sizeof(other_mem) != sizeof(v)) &&
                          (v > eina_value_ulong_max)))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uint64_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_char_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_short_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_int_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_int_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_long_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_long_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_int64_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_int64_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%f", v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   float *tmem = mem;
   *tmem = va_arg(args, double); /* float is promoted to double for va_args */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   float *tmem = mem;
   const float *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const float *tmem = mem;
   float *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   double *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const double *s = src;
   double *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_double_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const double *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_double_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const double v = *(const double *)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_uint_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY((sizeof(other_mem) != sizeof(v)) &&
                          (v > eina_value_ulong_max)))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v;
        if (EINA_UNLIKELY(v < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_char_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_short_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_int_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_int_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v;
        if (EINA_UNLIKELY(v < eina_value_long_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v > eina_value_long_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = v;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        const char *other_mem;
        char buf[64];
        snprintf(buf, sizeof(buf), "%g", (double)v);
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   double *tmem = mem;
   *tmem = va_arg(args, double);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   double *tmem = mem;
   const double *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   const double *tmem = mem;
   double *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_common_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   const char **tmem = mem;
   *tmem = NULL;
   return EINA_TRUE;
}

static int
_eina_value_type_string_common_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const char *sa = *(const char **)a;
   const char *sb = *(const char **)b;
   if (sa == sb)
     return 0;
   if (sa == NULL)
     return -1;
   if (sb == NULL)
     return 1;
   return strcmp(sa, sb);
}

static Eina_Bool
_eina_value_type_string_common_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const char *v = *(const char **)type_mem;

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem;
        if ((sscanf(v, "%hhu", &other_mem) != 1) &&
            (sscanf(v, "%hhx", &other_mem) != 1) &&
            (sscanf(v, "%hho", &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem;
        if ((sscanf(v, "%hu", &other_mem) != 1) &&
            (sscanf(v, "%hx", &other_mem) != 1) &&
            (sscanf(v, "%ho", &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem;
        if ((sscanf(v, "%u", &other_mem) != 1) &&
            (sscanf(v, "%x", &other_mem) != 1) &&
            (sscanf(v, "%o", &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem;
        if ((sscanf(v, "%lu", &other_mem) != 1) &&
            (sscanf(v, "%lx", &other_mem) != 1) &&
            (sscanf(v, "%lo", &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem;
        if ((sscanf(v, "%"SCNu64, &other_mem) != 1) &&
            (sscanf(v, "%"SCNx64, &other_mem) != 1) &&
            (sscanf(v, "%"SCNo64, &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem;
        if ((sscanf(v, "%hhd", &other_mem) != 1) &&
            (sscanf(v, "%hhx", &other_mem) != 1) &&
            (sscanf(v, "%hho", &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem;
        if ((sscanf(v, "%hd", &other_mem) != 1) &&
            (sscanf(v, "%hx", &other_mem) != 1) &&
            (sscanf(v, "%ho", &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem;
        if ((sscanf(v, "%d", &other_mem) != 1) &&
            (sscanf(v, "%x", &other_mem) != 1) &&
            (sscanf(v, "%o", &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem;
        if ((sscanf(v, "%ld", &other_mem) != 1) &&
            (sscanf(v, "%lx", &other_mem) != 1) &&
            (sscanf(v, "%lo", &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem;
        if ((sscanf(v, "%"SCNd64, &other_mem) != 1) &&
            (sscanf(v, "%"SCNx64, &other_mem) != 1) &&
            (sscanf(v, "%"SCNo64, &other_mem) != 1))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem;
        if (sscanf(v, "%f", &other_mem) != 1)
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem;
        if (sscanf(v, "%lf", &other_mem) != 1)
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        return eina_value_type_pset(convert, convert_mem, &v);
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_common_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   memcpy(ptr, mem, sizeof(const char *));
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_stringshare_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   const char **tmem = mem;
   if (*tmem)
     {
        eina_stringshare_del(*tmem);
        *tmem = NULL;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_stringshare_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const char * const*s = src;
   const char **d = dst;
   *d = eina_stringshare_add(*s);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_stringshare_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   const char *str = va_arg(args, const char *);
   return eina_stringshare_replace((const char **)mem, str);
}

static Eina_Bool
_eina_value_type_stringshare_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   const char * const *str = ptr;
   return eina_stringshare_replace((const char **)mem, *str);
}

static Eina_Bool
_eina_value_type_string_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   char **tmem = mem;
   if (*tmem)
     {
        free(*tmem);
        *tmem = NULL;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const char * const *s = src;
   char **d = dst;
   if (*s == NULL)
     *d = NULL;
   else
     {
        *d = strdup(*s);
        if (*d == NULL)
          {
             return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   char **tmem = mem;
   const char *str = va_arg(args, const char *);
   
   if (str == *tmem) return EINA_TRUE;
   if (!str)
     {
        free(*tmem);
        *tmem = NULL;
     }
   else
     {
        char *tmp = strdup(str);
        if (!tmp)
          {
             return EINA_FALSE;
          }
        free(*tmem);
        *tmem = tmp;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   char **tmem = mem;
   const char * const *str = ptr;
   
   if (*str == *tmem) return EINA_TRUE;
   if (!*str)
     {
        free(*tmem);
        *tmem = NULL;
     }
   else
     {
        char *tmp = strdup(*str);
        if (!tmp)
          {
             return EINA_FALSE;
          }
        free(*tmem);
        *tmem = tmp;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_array_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   memset(mem, 0, sizeof(Eina_Value_Array));
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_array_flush_elements(Eina_Value_Array *tmem)
{
   const Eina_Value_Type *subtype = tmem->subtype;
   Eina_Bool ret = EINA_TRUE;
   unsigned char sz;
   char *ptr, *ptr_end;

   if (!tmem->array) return EINA_TRUE;

   sz = tmem->array->member_size;
   ptr = tmem->array->members;
   ptr_end = ptr + tmem->array->len * sz;

   for (; ptr < ptr_end; ptr += sz)
     ret &= eina_value_type_flush(subtype, ptr);

   eina_inarray_flush(tmem->array);
   return ret;
}

static Eina_Bool
_eina_value_type_array_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eina_Value_Array *tmem = mem;
   Eina_Bool ret =_eina_value_type_array_flush_elements(tmem);

   if (tmem->array) eina_inarray_free(tmem->array);
   tmem->array = NULL;
   tmem->subtype = NULL;
   return ret;
}

static Eina_Bool
_eina_value_type_array_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const Eina_Value_Type *subtype;
   const Eina_Value_Array *s = src;
   Eina_Value_Array *d = dst;
   unsigned int i, count, sz;
   char *ptr, *ptr_end;

   d->subtype = subtype = s->subtype;
   d->step = s->step;

   if ((!s->array) || (!s->subtype))
     {
        d->array = NULL;
        return EINA_TRUE;
     }

   if (!subtype->copy)
     {
        return EINA_FALSE;
     }

   d->array = eina_inarray_new(subtype->value_size, s->step);
   if (!d->array)
     return EINA_FALSE;

   sz = s->array->member_size;

   count = eina_inarray_count(s->array);
   ptr = s->array->members;
   ptr_end = ptr + (count * sz);

   for (i = 0; ptr < ptr_end; ptr += sz, i++)
     {
        void *imem = eina_inarray_alloc_at(d->array, i, 1);
        if (!imem) goto error;
        if (!subtype->copy(subtype, ptr, imem))
          {
             eina_inarray_pop(d->array);
             goto error;
          }
     }

   return EINA_TRUE;

 error:
   _eina_value_type_array_flush_elements(d);
   return EINA_FALSE;
}

static int
_eina_value_type_array_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const Eina_Value_Type *subtype;
   const Eina_Value_Array *eva_a = a, *eva_b = b;
   const char *a_ptr, *a_ptr_end, *b_ptr;
   unsigned int count_a, count_b, count, sz;
   int cmp = 0;

   if (eva_a->subtype != eva_b->subtype)
     {
        return -1;
     }

   subtype = eva_a->subtype;
   if (!subtype->compare)
     {
        return 0;
     }

   if ((!eva_a->array) && (!eva_b->array))
     return 0;
   else if (!eva_a->array)
     return -1;
   else if (!eva_b->array)
     return 1;

   count_a = eina_inarray_count(eva_a->array);
   count_b = eina_inarray_count(eva_b->array);

   if (count_a <= count_b)
     count = count_a;
   else
     count = count_b;

   sz = eva_a->array->member_size;

   a_ptr = eva_a->array->members;
   a_ptr_end = a_ptr + (count * sz);
   b_ptr = eva_b->array->members;

   for (; (cmp == 0) && (a_ptr < a_ptr_end); a_ptr += sz, b_ptr += sz)
     cmp = subtype->compare(subtype, a_ptr, b_ptr);

   if (cmp == 0)
     {
        if (count_a < count_b)
          return -1;
        else if (count_a > count_b)
          return 1;
        return 0;
     }

   return cmp;
}

static Eina_Bool
_eina_value_type_array_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const Eina_Value_Array *tmem = type_mem;
   Eina_Bool ret = EINA_FALSE;

   if ((convert == EINA_VALUE_TYPE_STRING) ||
       (convert == EINA_VALUE_TYPE_STRINGSHARE))
     {
        Eina_Strbuf *str = eina_strbuf_new();
        const char *ptr;
        if (!tmem->array) eina_strbuf_append(str, "[]");
        else
          {
             const Eina_Value_Type *subtype = tmem->subtype;
             unsigned char sz;
             const char *ptr_end;
             Eina_Value tmp;
             Eina_Bool first = EINA_TRUE;

             eina_value_setup(&tmp, EINA_VALUE_TYPE_STRING);

             eina_strbuf_append_char(str, '[');

             sz = tmem->array->member_size;
             ptr = tmem->array->members;
             ptr_end = ptr + tmem->array->len * sz;
             for (; ptr < ptr_end; ptr += sz)
               {
                  Eina_Bool r = EINA_FALSE;
                  if (subtype->convert_to)
                    {
                       r = subtype->convert_to(subtype, EINA_VALUE_TYPE_STRING,
                                               ptr, tmp.value.buf);
                       if (r)
                         {
                            if (first) first = EINA_FALSE;
                            else eina_strbuf_append_length(str, ", ", 2);
                            eina_strbuf_append(str, tmp.value.ptr);
                            free(tmp.value.ptr);
                            tmp.value.ptr = NULL;
                         }
                    }

                  if (!r)
                    {
                       if (first)
                         {
                            first = EINA_FALSE;
                            eina_strbuf_append_char(str, '?');
                         }
                       else
                         eina_strbuf_append_length(str, ", ?", 3);
                    }
               }

             eina_strbuf_append_char(str, ']');
          }
        ptr = eina_strbuf_string_get(str);
        ret = eina_value_type_pset(convert, convert_mem, &ptr);
        eina_strbuf_free(str);
     }
   else if ((tmem->array) && (tmem->array->len == 1))
     {
        const Eina_Value_Type *subtype = tmem->subtype;
        void *imem = tmem->array->members;

        if (subtype->convert_to)
          ret = subtype->convert_to(subtype, convert, imem, convert_mem);
        if ((!ret) && (convert->convert_from))
          ret = convert->convert_from(convert, subtype, convert_mem, imem);
     }

   if (!ret)
     {
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_array_convert_from(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem)
{
   Eina_Value_Array *tmem = type_mem;
   Eina_Value_Array desc = {convert, tmem->step, NULL};
   char *buf;
   void *imem;

   if (!eina_value_type_pset(type, tmem, &desc))
     return EINA_FALSE;

   buf = alloca(convert->value_size);
   if (!eina_value_type_pget(convert, convert_mem, &buf))
     return EINA_FALSE;

   imem = eina_inarray_alloc_at(tmem->array, 0, 1);
   if (!imem)
     return EINA_FALSE;

   if (!eina_value_type_setup(convert, imem)) goto error_setup;
   if (!eina_value_type_pset(convert, imem, &buf)) goto error_set;
   return EINA_TRUE;

 error_set:
   eina_value_type_flush(convert, imem);
 error_setup:
   eina_inarray_remove_at(tmem->array, 0);
   return EINA_FALSE;
}

static Eina_Bool
_eina_value_type_array_pset(const Eina_Value_Type *type, void *mem, const void *ptr)
{
   Eina_Value_Array *tmem = mem;
   const Eina_Value_Array *desc = ptr;
   Eina_Inarray *desc_array;

   if ((!tmem->subtype) && (!desc->subtype))
     return EINA_TRUE;

   desc_array = desc->array;
   if (desc_array)
     {
        Eina_Value_Array tmp;

        EINA_SAFETY_ON_FALSE_RETURN_VAL
          (desc_array->member_size == desc->subtype->value_size, EINA_FALSE);

        if (desc_array == tmem->array)
          {
             tmem->subtype = desc->subtype;
             return EINA_TRUE;
          }

        if (!_eina_value_type_array_copy(type, desc, &tmp))
          return EINA_FALSE;

        _eina_value_type_array_flush(type, tmem);
        memcpy(tmem, &tmp, sizeof(tmp));
        return EINA_TRUE;
     }

   if (tmem->array)
     {
        _eina_value_type_array_flush_elements(tmem);
        eina_inarray_step_set(tmem->array, sizeof (Eina_Inarray), desc->subtype->value_size, desc->step);
     }
   else
     {
        tmem->array = eina_inarray_new(desc->subtype->value_size, desc->step);
        if (!tmem->array)
          return EINA_FALSE;
     }

   tmem->subtype = desc->subtype;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_array_vset(const Eina_Value_Type *type, void *mem, va_list args)
{
   const Eina_Value_Array desc = va_arg(args, Eina_Value_Array);
   _eina_value_type_array_pset(type, mem, &desc);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_array_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   memcpy(ptr, mem, sizeof(Eina_Value_Array));
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_ARRAY = {
  EINA_VALUE_TYPE_VERSION,
  sizeof(Eina_Value_Array),
  "Eina_Value_Array",
  _eina_value_type_array_setup,
  _eina_value_type_array_flush,
  _eina_value_type_array_copy,
  _eina_value_type_array_compare,
  _eina_value_type_array_convert_to,
  _eina_value_type_array_convert_from,
  _eina_value_type_array_vset,
  _eina_value_type_array_pset,
  _eina_value_type_array_pget
};

static Eina_Bool
_eina_value_type_list_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   memset(mem, 0, sizeof(Eina_Value_List));
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_list_flush_elements(Eina_Value_List *tmem)
{
   const Eina_Value_Type *subtype = tmem->subtype;
   Eina_Bool ret = EINA_TRUE;

   if (!tmem->list) return EINA_TRUE;

   while (tmem->list)
     {
        void *mem = eina_value_list_node_memory_get(tmem->subtype, tmem->list);
        ret &= eina_value_type_flush(subtype, mem);
        eina_value_list_node_memory_flush(tmem->subtype, tmem->list);
        tmem->list = eina_list_remove_list(tmem->list, tmem->list);
     }

   return ret;
}

static Eina_Bool
_eina_value_type_list_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eina_Value_List *tmem = mem;
   Eina_Bool ret =_eina_value_type_list_flush_elements(tmem);

   if (tmem->list) eina_list_free(tmem->list);
   tmem->list = NULL;
   tmem->subtype = NULL;
   return ret;
}

static Eina_Bool
_eina_value_type_list_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const Eina_Value_Type *subtype;
   const Eina_Value_List *s = src;
   Eina_Value_List *d = dst;
   const Eina_List *snode;

   d->subtype = subtype = s->subtype;
   if ((!s->list) || (!s->subtype))
     {
        d->list = NULL;
        return EINA_TRUE;
     }

   if (!subtype->copy)
     {
        return EINA_FALSE;
     }

   d->list = NULL;
   for (snode = s->list; snode != NULL; snode = snode->next)
     {
        const void *ptr = eina_value_list_node_memory_get(subtype, snode);
        Eina_List *dnode;
        void *imem;

        d->list = eina_list_append(d->list, (void*)1L);
        dnode = eina_list_last(d->list);
        EINA_SAFETY_ON_NULL_GOTO(dnode, error);
        EINA_SAFETY_ON_FALSE_GOTO(dnode->data == (void*)1L, error);

        imem = eina_value_list_node_memory_setup(subtype, dnode);
        if (!subtype->copy(subtype, ptr, imem))
          {
             eina_value_list_node_memory_flush(subtype, dnode);
             d->list = eina_list_remove_list(d->list, dnode);
             goto error;
          }
     }
   return EINA_TRUE;

 error:
   _eina_value_type_list_flush_elements(d);
   return EINA_FALSE;
}

static int
_eina_value_type_list_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const Eina_Value_Type *subtype;
   const Eina_Value_List *eva_a = a, *eva_b = b;
   const Eina_List *anode, *bnode;
   int cmp = 0;

   if (eva_a->subtype != eva_b->subtype)
     {
        return -1;
     }

   subtype = eva_a->subtype;
   if (!subtype->compare)
     {
        return 0;
     }

   if ((!eva_a->list) && (!eva_b->list))
     return 0;
   else if (!eva_a->list)
     return -1;
   else if (!eva_b->list)
     return 1;

   for (anode = eva_a->list, bnode = eva_b->list;
        (cmp == 0) && (anode) && (bnode);
        anode = anode->next, bnode = bnode->next)
     {
        const void *amem = eina_value_list_node_memory_get(subtype, anode);
        const void *bmem = eina_value_list_node_memory_get(subtype, bnode);
        cmp = subtype->compare(subtype, amem, bmem);
     }

   if (cmp == 0)
     {
        if ((!anode) && (bnode))
          return -1;
        else if ((anode) && (!bnode))
          return 1;
        return 0;
     }

   return cmp;
}

static Eina_Bool
_eina_value_type_list_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const Eina_Value_List *tmem = type_mem;
   Eina_Bool ret = EINA_FALSE;

   if ((convert == EINA_VALUE_TYPE_STRING) ||
       (convert == EINA_VALUE_TYPE_STRINGSHARE))
     {
        Eina_Strbuf *str = eina_strbuf_new();
        const char *s;
        if (!tmem->list) eina_strbuf_append(str, "[]");
        else
          {
             const Eina_Value_Type *subtype = tmem->subtype;
             const Eina_List *node;
             Eina_Value tmp;
             Eina_Bool first = EINA_TRUE;

             eina_value_setup(&tmp, EINA_VALUE_TYPE_STRING);

             eina_strbuf_append_char(str, '[');

             for (node = tmem->list; node != NULL; node = node->next)
               {
                  Eina_Bool r = EINA_FALSE;

                  if (subtype->convert_to)
                    {
                       const void *ptr;
                       ptr = eina_value_list_node_memory_get(subtype, node);
                       r = subtype->convert_to(subtype, EINA_VALUE_TYPE_STRING,
                                               ptr, tmp.value.buf);
                       if (r)
                         {
                            if (first) first = EINA_FALSE;
                            else eina_strbuf_append_length(str, ", ", 2);
                            eina_strbuf_append(str, tmp.value.ptr);
                            free(tmp.value.ptr);
                            tmp.value.ptr = NULL;
                         }
                    }

                  if (!r)
                    {
                       if (first)
                         {
                            first = EINA_FALSE;
                            eina_strbuf_append_char(str, '?');
                         }
                       else
                         eina_strbuf_append_length(str, ", ?", 3);
                    }
               }

             eina_strbuf_append_char(str, ']');
          }
        s = eina_strbuf_string_get(str);
        ret = eina_value_type_pset(convert, convert_mem, &s);
        eina_strbuf_free(str);
     }
   else if ((tmem->list) && (tmem->list->next == NULL))
     {
        const Eina_Value_Type *subtype = tmem->subtype;
        void *imem = eina_value_list_node_memory_get(subtype, tmem->list);

        if (subtype->convert_to)
          ret = subtype->convert_to(subtype, convert, imem, convert_mem);
        if ((!ret) && (convert->convert_from))
          ret = convert->convert_from(convert, subtype, convert_mem, imem);
     }

   if (!ret)
     {
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_list_convert_from(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem)
{
   Eina_Value_List *tmem = type_mem;
   Eina_Value_List desc = {convert, NULL};
   Eina_List *node;
   char *buf;
   void *imem;

   if (!eina_value_type_pset(type, tmem, &desc))
     return EINA_FALSE;

   buf = alloca(convert->value_size);
   if (!eina_value_type_pget(convert, convert_mem, &buf))
     return EINA_FALSE;

   tmem->list = eina_list_append(tmem->list, (void*)1L);
   node = eina_list_last(tmem->list);
   EINA_SAFETY_ON_NULL_RETURN_VAL(node, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(node->data == (void*)1L, EINA_FALSE);

   imem = eina_value_list_node_memory_setup(tmem->subtype, node);
   if (!imem)
     {
        tmem->list = eina_list_remove_list(tmem->list, node);
        return EINA_FALSE;
     }

   if (!eina_value_type_setup(tmem->subtype, imem)) goto error_setup;
   if (!eina_value_type_pset(tmem->subtype, imem, &buf)) goto error_set;
   return EINA_TRUE;

 error_set:
   eina_value_type_flush(tmem->subtype, imem);
 error_setup:
   eina_value_list_node_memory_flush(tmem->subtype, node);
   tmem->list = eina_list_remove_list(tmem->list, node);
   return EINA_FALSE;
}

static Eina_Bool
_eina_value_type_list_pset(const Eina_Value_Type *type, void *mem, const void *ptr)
{
   Eina_Value_List *tmem = mem;
   const Eina_Value_List *desc = ptr;

   if ((!tmem->subtype) && (!desc->subtype))
     return EINA_TRUE;

   if ((tmem->list) && (tmem->list == desc->list))
     {
        tmem->subtype = desc->subtype;
        return EINA_TRUE;
     }

   if (desc->list)
     {
        Eina_Value_List tmp;

        if (!_eina_value_type_list_copy(type, desc, &tmp))
          return EINA_FALSE;

        _eina_value_type_list_flush(type, tmem);
        memcpy(tmem, &tmp, sizeof(tmp));
        return EINA_TRUE;
     }

   _eina_value_type_list_flush_elements(tmem);

   tmem->subtype = desc->subtype;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_list_vset(const Eina_Value_Type *type, void *mem, va_list args)
{
   const Eina_Value_List desc = va_arg(args, Eina_Value_List);
   _eina_value_type_list_pset(type, mem, &desc);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_list_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   memcpy(ptr, mem, sizeof(Eina_Value_List));
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_LIST = {
  EINA_VALUE_TYPE_VERSION,
  sizeof(Eina_Value_List),
  "Eina_Value_List",
  _eina_value_type_list_setup,
  _eina_value_type_list_flush,
  _eina_value_type_list_copy,
  _eina_value_type_list_compare,
  _eina_value_type_list_convert_to,
  _eina_value_type_list_convert_from,
  _eina_value_type_list_vset,
  _eina_value_type_list_pset,
  _eina_value_type_list_pget
};

static Eina_Bool
_eina_value_type_hash_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   memset(mem, 0, sizeof(Eina_Value_Hash));
   return EINA_TRUE;
}

struct _eina_value_type_hash_flush_each_ctx
{
   const Eina_Value_Type *subtype;
   Eina_Bool ret;
};

static Eina_Bool
_eina_value_type_hash_flush_each(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *mem, void *user_data)
{
   struct _eina_value_type_hash_flush_each_ctx *ctx = user_data;
   ctx->ret &= eina_value_type_flush(ctx->subtype, mem);
   free(mem);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_hash_flush_elements(Eina_Value_Hash *tmem)
{
   struct _eina_value_type_hash_flush_each_ctx ctx = {
     tmem->subtype,
     EINA_TRUE
   };

   if (!tmem->hash) return EINA_TRUE;

   eina_hash_foreach(tmem->hash, _eina_value_type_hash_flush_each, &ctx);
   eina_hash_free(tmem->hash);
   tmem->hash = NULL;
   return ctx.ret;
}

static Eina_Bool
_eina_value_type_hash_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eina_Value_Hash *tmem = mem;
   Eina_Bool ret =_eina_value_type_hash_flush_elements(tmem);
   tmem->subtype = NULL;
   return ret;
}

static unsigned int
_eina_value_hash_key_length(const void *key)
{
   if (!key)
      return 0;
   return (int)strlen(key) + 1;
}

static int
_eina_value_hash_key_cmp(const void *key1, int key1_len, const void *key2, int key2_len)
{
   int r = key1_len - key2_len;
   if (r != 0)
     return r;
   return strcmp(key1, key2);
}

static Eina_Bool
_eina_value_type_hash_create(Eina_Value_Hash *desc)
{
   if (!desc->buckets_power_size)
     desc->buckets_power_size = 5;

   desc->hash = eina_hash_new(_eina_value_hash_key_length,
                              _eina_value_hash_key_cmp,
                              EINA_KEY_HASH(eina_hash_superfast),
                              NULL, desc->buckets_power_size);
   return !!desc->hash;
}

struct _eina_value_type_hash_copy_each_ctx
{
   const Eina_Value_Type *subtype;
   Eina_Value_Hash *dest;
   Eina_Bool ret;
};

static Eina_Bool
_eina_value_type_hash_copy_each(const Eina_Hash *hash EINA_UNUSED, const void *key, void *_ptr, void *user_data)
{
   struct _eina_value_type_hash_copy_each_ctx *ctx = user_data;
   const void *ptr = _ptr;
   void *imem = malloc(ctx->subtype->value_size);
   if (!imem)
     {
        ctx->ret = EINA_FALSE;
        return EINA_FALSE;
     }
   if (!ctx->subtype->copy(ctx->subtype, ptr, imem))
     {
        free(imem);
        ctx->ret = EINA_FALSE;
        return EINA_FALSE;
     }
   if (!eina_hash_add(ctx->dest->hash, key, imem))
     {
        eina_value_type_flush(ctx->subtype, imem);
        free(imem);
        ctx->ret = EINA_FALSE;
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_hash_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const Eina_Value_Hash *s = src;
   Eina_Value_Hash *d = dst;
   struct _eina_value_type_hash_copy_each_ctx ctx = {s->subtype, d, EINA_TRUE};

   d->subtype = s->subtype;
   d->buckets_power_size = s->buckets_power_size;

   if ((!s->hash) || (!s->subtype))
     {
        d->hash = NULL;
        return EINA_TRUE;
     }

   if (!s->subtype->copy)
     {
        return EINA_FALSE;
     }

   if (!_eina_value_type_hash_create(d))
     return EINA_FALSE;

   eina_hash_foreach(s->hash, _eina_value_type_hash_copy_each, &ctx);
   if (!ctx.ret)
     {
        _eina_value_type_hash_flush_elements(d);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

struct _eina_value_type_hash_compare_each_ctx
{
   const Eina_Value_Type *subtype;
   const Eina_Hash *other;
   int cmp;
};

static Eina_Bool
_eina_value_type_hash_compare_each(const Eina_Hash *hash EINA_UNUSED, const void *key, void *_ptr, void *user_data)
{
   struct _eina_value_type_hash_compare_each_ctx *ctx = user_data;
   const void *self_ptr = _ptr;
   const void *other_ptr = eina_hash_find(ctx->other, key);
   if (!other_ptr) return EINA_TRUE;
   ctx->cmp = ctx->subtype->compare(ctx->subtype, self_ptr, other_ptr);
   return ctx->cmp == 0;
}

static int
_eina_value_type_hash_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const Eina_Value_Hash *eva_a = a, *eva_b = b;
   struct _eina_value_type_hash_compare_each_ctx ctx = {
     eva_a->subtype, eva_b->hash, 0
   };

   if (eva_a->subtype != eva_b->subtype)
     {
        return -1;
     }

   if (!eva_a->subtype->compare)
     {
        return 0;
     }

   if ((!eva_a->hash) && (!eva_b->hash))
     return 0;
   else if (!eva_a->hash)
     return -1;
   else if (!eva_b->hash)
     return 1;

   eina_hash_foreach(eva_a->hash, _eina_value_type_hash_compare_each, &ctx);
   if (ctx.cmp == 0)
     {
        unsigned int count_a = eina_hash_population(eva_a->hash);
        unsigned int count_b = eina_hash_population(eva_b->hash);
        if (count_a < count_b)
          return -1;
        else if (count_a > count_b)
          return 1;
        return 0;
     }

   return ctx.cmp;
}

struct _eina_value_type_hash_convert_to_string_each_ctx
{
   const Eina_Value_Type *subtype;
   Eina_Strbuf *str;
   Eina_Value tmp;
   Eina_Bool first;
};

static Eina_Bool
_eina_value_type_hash_convert_to_string_each(const Eina_Hash *hash EINA_UNUSED, const void *_key, void *_ptr, void *user_data)
{
   struct _eina_value_type_hash_convert_to_string_each_ctx *ctx = user_data;
   const char *key = _key;
   const void *ptr = _ptr;
   Eina_Bool r = EINA_FALSE;

   if (ctx->first) ctx->first = EINA_FALSE;
   else eina_strbuf_append_length(ctx->str, ", ", 2);

   eina_strbuf_append(ctx->str, key);
   eina_strbuf_append_length(ctx->str, ": ", 2);

   if (ctx->subtype->convert_to)
     {
        r = ctx->subtype->convert_to(ctx->subtype, EINA_VALUE_TYPE_STRING,
                                     ptr, ctx->tmp.value.buf);
        if (r)
          {
             eina_strbuf_append(ctx->str, ctx->tmp.value.ptr);
             free(ctx->tmp.value.ptr);
             ctx->tmp.value.ptr = NULL;
          }
     }

   if (!r)
     eina_strbuf_append_char(ctx->str, '?');

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_hash_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const Eina_Value_Hash *tmem = type_mem;
   Eina_Bool ret = EINA_FALSE;

   if ((convert == EINA_VALUE_TYPE_STRING) ||
       (convert == EINA_VALUE_TYPE_STRINGSHARE))
     {
        Eina_Strbuf *str = eina_strbuf_new();
        const char *s;
        if (!tmem->hash) eina_strbuf_append(str, "{}");
        else
          {
             struct _eina_value_type_hash_convert_to_string_each_ctx ctx;

             ctx.subtype = tmem->subtype;
             ctx.str = str;
             ctx.first = EINA_TRUE;
             eina_value_setup(&ctx.tmp, EINA_VALUE_TYPE_STRING);

             eina_strbuf_append_char(str, '{');

             eina_hash_foreach(tmem->hash,
                               _eina_value_type_hash_convert_to_string_each,
                               &ctx);

             eina_strbuf_append_char(str, '}');
          }
        s = eina_strbuf_string_get(str);
        ret = eina_value_type_pset(convert, convert_mem, &s);
        eina_strbuf_free(str);
     }
   else if ((tmem->hash) && (eina_hash_population(tmem->hash) == 1))
     {
        const Eina_Value_Type *subtype = tmem->subtype;
	Eina_Iterator *it;
        void *imem = NULL;

	it = eina_hash_iterator_data_new(tmem->hash);

	if (!it || !eina_iterator_next(it, &imem) || !imem) /* shouldn't happen... */
	  {
	    ret = EINA_FALSE;
	  }
        else
          {
             if (subtype->convert_to)
               ret = subtype->convert_to(subtype, convert, imem, convert_mem);
             if ((!ret) && (convert->convert_from))
               ret = convert->convert_from(convert, subtype, convert_mem, imem);
          }
	eina_iterator_free(it);
     }

   if (!ret)
     {
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_hash_pset(const Eina_Value_Type *type, void *mem, const void *ptr)
{
   Eina_Value_Hash *tmem = mem;
   const Eina_Value_Hash *desc = ptr;

   if ((!tmem->subtype) && (!desc->subtype))
     return EINA_TRUE;

   if ((tmem->hash) && (tmem->hash == desc->hash))
     {
        tmem->subtype = desc->subtype;
        return EINA_TRUE;
     }

   if (desc->hash)
     {
        Eina_Value_Hash tmp;

        if (!_eina_value_type_hash_copy(type, desc, &tmp))
          return EINA_FALSE;

        _eina_value_type_hash_flush(type, tmem);
        memcpy(tmem, &tmp, sizeof(tmp));
        return EINA_TRUE;
     }

   if (tmem->hash) _eina_value_type_hash_flush_elements(tmem);

   tmem->subtype = desc->subtype;
   if (!_eina_value_type_hash_create(tmem))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_hash_vset(const Eina_Value_Type *type, void *mem, va_list args)
{
   const Eina_Value_Hash desc = va_arg(args, Eina_Value_Hash);
   _eina_value_type_hash_pset(type, mem, &desc);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_hash_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   memcpy(ptr, mem, sizeof(Eina_Value_Hash));
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_HASH = {
  EINA_VALUE_TYPE_VERSION,
  sizeof(Eina_Value_Hash),
  "Eina_Value_Hash",
  _eina_value_type_hash_setup,
  _eina_value_type_hash_flush,
  _eina_value_type_hash_copy,
  _eina_value_type_hash_compare,
  _eina_value_type_hash_convert_to,
  NULL, /* no convert from */
  _eina_value_type_hash_vset,
  _eina_value_type_hash_pset,
  _eina_value_type_hash_pget
};

static Eina_Bool
_eina_value_type_timeval_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   memset(mem, 0, sizeof(struct timeval));
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_timeval_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_timeval_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const struct timeval *s = src;
   struct timeval *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static inline struct timeval _eina_value_type_timeval_fix(const struct timeval *input)
{
   struct timeval ret = *input;
   if (EINA_UNLIKELY(ret.tv_usec < 0))
     {
        ret.tv_sec -= 1;
        ret.tv_usec += 1e6;
     }
   return ret;
}

static int
_eina_value_type_timeval_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   struct timeval va = _eina_value_type_timeval_fix(a);
   struct timeval vb = _eina_value_type_timeval_fix(b);

   if (va.tv_sec < vb.tv_sec)
     return -1;
   else if (va.tv_sec > vb.tv_sec)
     return 1;

   if (va.tv_usec < vb.tv_usec)
     return -1;
   else if (va.tv_usec > vb.tv_usec)
     return 1;

   return 0;
}

static Eina_Bool
_eina_value_type_timeval_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   struct timeval v = _eina_value_type_timeval_fix(type_mem);

   if (convert == EINA_VALUE_TYPE_UCHAR)
     {
        unsigned char other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v.tv_sec > eina_value_uchar_max))
            return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_USHORT)
     {
        unsigned short other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v.tv_sec > eina_value_ushort_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT)
     {
        unsigned int other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY((unsigned long) v.tv_sec > eina_value_uint_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if ((convert == EINA_VALUE_TYPE_ULONG) || (convert == EINA_VALUE_TYPE_TIMESTAMP))
     {
        unsigned long other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < 0))
          return EINA_FALSE;
        if (EINA_UNLIKELY((sizeof(other_mem) != sizeof(v)) &&
                          ((unsigned long)v.tv_sec > eina_value_ulong_max)))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < 0))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_CHAR)
     {
        signed char other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < eina_value_char_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v.tv_sec > eina_value_char_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_SHORT)
     {
        short other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < eina_value_short_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v.tv_sec > eina_value_short_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT)
     {
        int other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < eina_value_int_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v.tv_sec > eina_value_int_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_LONG)
     {
        long other_mem = v.tv_sec;
        if (EINA_UNLIKELY(v.tv_sec < eina_value_long_min))
          return EINA_FALSE;
        if (EINA_UNLIKELY(v.tv_sec > eina_value_long_max))
          return EINA_FALSE;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_INT64)
     {
        int64_t other_mem = v.tv_sec;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_FLOAT)
     {
        float other_mem = (float)v.tv_sec + (float)v.tv_usec / 1.0e6;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_DOUBLE)
     {
        double other_mem = (double)v.tv_sec + (double)v.tv_usec / 1.0e6;
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
            convert == EINA_VALUE_TYPE_STRING)
     {
        char buf[64];
        const char *other_mem;
        time_t t;

        t = v.tv_sec;
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&t));
        other_mem = buf; /* required due &buf == buf */
        return eina_value_type_pset(convert, convert_mem, &other_mem);
     }
   else
     {
        return EINA_FALSE;
     }
}

static Eina_Bool
_eina_value_type_timeval_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   struct timeval *tmem = mem;
   *tmem = _eina_value_type_timeval_fix(ptr);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_timeval_vset(const Eina_Value_Type *type, void *mem, va_list args)
{
   const struct timeval desc = va_arg(args, struct timeval);
   _eina_value_type_timeval_pset(type, mem, &desc);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_timeval_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   memcpy(ptr, mem, sizeof(struct timeval));
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_TIMEVAL = {
  EINA_VALUE_TYPE_VERSION,
  sizeof(struct timeval),
  "struct timeval",
  _eina_value_type_timeval_setup,
  _eina_value_type_timeval_flush,
  _eina_value_type_timeval_copy,
  _eina_value_type_timeval_compare,
  _eina_value_type_timeval_convert_to,
  NULL, /* no convert from */
  _eina_value_type_timeval_vset,
  _eina_value_type_timeval_pset,
  _eina_value_type_timeval_pget
};

static Eina_Bool
_eina_value_type_blob_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   memset(mem, 0, sizeof(Eina_Value_Blob));
   return EINA_TRUE;
}

static inline const Eina_Value_Blob_Operations *
_eina_value_type_blob_ops_get(const Eina_Value_Blob *blob)
{
   if (!blob) return NULL;
   if (!blob->ops) return NULL;
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (blob->ops->version == EINA_VALUE_BLOB_OPERATIONS_VERSION, NULL);
   return blob->ops;
}

static Eina_Bool
_eina_value_type_blob_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   const Eina_Value_Blob_Operations *ops = _eina_value_type_blob_ops_get(mem);
   Eina_Value_Blob *tmem = mem;
   if ((ops) && (ops->free))
     ops->free(ops, (void *)tmem->memory, tmem->size);
   tmem->memory = NULL;
   tmem->size = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_blob_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const Eina_Value_Blob_Operations *ops = _eina_value_type_blob_ops_get(src);
   const Eina_Value_Blob *s = src;
   Eina_Value_Blob *d = dst;

   *d = *s;

   if ((ops) && (ops->copy))
     {
        d->memory = ops->copy(ops, s->memory, s->size);
        if ((d->memory == NULL) && (s->size > 0))
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static int
_eina_value_type_blob_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const Eina_Value_Blob_Operations *ops = _eina_value_type_blob_ops_get(a);
   const Eina_Value_Blob *ta = a, *tb = b;
   size_t minsize;
   if (ta->ops != tb->ops)
     {
        return -1;
     }
   if ((ops) && (ops->compare))
     return ops->compare(ops, ta->memory, ta->size, tb->memory, tb->size);

   if (ta->size < tb->size)
     minsize = ta->size;
   else
     minsize = tb->size;

   return memcmp(ta->memory, tb->memory, minsize);
}

static Eina_Bool
_eina_value_type_blob_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const Eina_Value_Blob *tmem = type_mem;

   if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
       convert == EINA_VALUE_TYPE_STRING)
     {
        const Eina_Value_Blob_Operations *ops;
        Eina_Strbuf *str;
        const char *other_mem;
        Eina_Bool ret = EINA_FALSE, first = EINA_TRUE;
        const unsigned char *ptr, *ptr_end;

        ops = _eina_value_type_blob_ops_get(tmem);
        if ((ops) && (ops->to_string))
          {
             char *x = ops->to_string(ops, tmem->memory, tmem->size);
             if (x)
               {
                  ret = eina_value_type_pset(convert, convert_mem, &x);
                  free(x);
               }
             return ret;
          }

        str = eina_strbuf_new();
        if (!str)
          return EINA_FALSE;

        if (!eina_strbuf_append_printf(str, "BLOB(%u, [", tmem->size))
          goto error;

        ptr = tmem->memory;
        ptr_end = ptr + tmem->size;
        for (; ptr < ptr_end; ptr++)
          {
             if (first)
               {
                  first = EINA_FALSE;
                  if (!eina_strbuf_append_printf(str, "%02hhx", *ptr))
                    goto error;
               }
             else
               {
                  if (!eina_strbuf_append_printf(str, " %02hhx", *ptr))
                    goto error;
               }
          }

        if (!eina_strbuf_append(str, "])"))
          goto error;

        other_mem = eina_strbuf_string_get(str);
        ret = eina_value_type_pset(convert, convert_mem, &other_mem);

     error:
        eina_strbuf_free(str);
        return ret;
     }
   else
     {
        return EINA_FALSE;
     }
}

static Eina_Bool
_eina_value_type_blob_convert_from(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem)
{
   Eina_Value_Blob desc;
   char *buf;

   desc.ops = EINA_VALUE_BLOB_OPERATIONS_MALLOC;

   if ((convert == EINA_VALUE_TYPE_STRING) ||
       (convert == EINA_VALUE_TYPE_STRINGSHARE))
     {
        const char *str = *(const char **)convert_mem;
        if (!str)
          {
             desc.size = 0;
             desc.memory = NULL;
          }
        else
          {
             desc.size = strlen(str) + 1;
             desc.memory = buf = malloc(desc.size);
             if (!desc.memory)
               {
                  return EINA_FALSE;
               }
             memcpy(buf, str, desc.size);
          }
     }
   else if (convert == EINA_VALUE_TYPE_ARRAY)
     {
        const Eina_Value_Array *a = convert_mem;
        if ((!a->array) || (a->array->len == 0))
          {
             desc.size = 0;
             desc.memory = NULL;
          }
        else
          {
             desc.size = a->array->len * a->array->member_size;
             desc.memory = buf = malloc(desc.size);
             if (!desc.memory)
               {
                  return EINA_FALSE;
               }
             memcpy(buf, a->array->members, desc.size);
          }
     }
   else if (convert == EINA_VALUE_TYPE_BLOB)
     {
        const Eina_Value_Blob *b = convert_mem;
        if (b->size == 0)
          {
             desc.size = 0;
             desc.memory = NULL;
          }
        else
          {
             desc.size = b->size;
             desc.memory = buf = malloc(desc.size);
             if (!desc.memory)
               {
                  return EINA_FALSE;
               }
             memcpy(buf, b->memory, desc.size);
          }
     }
   else
     {
        desc.size = convert->value_size;
        desc.memory = buf = malloc(convert->value_size);
        if (!desc.memory)
          {
             return EINA_FALSE;
          }
        if (!eina_value_type_pget(convert, convert_mem, buf))
          {
             free(buf);
             return EINA_FALSE;
          }
     }
   return eina_value_type_pset(type, type_mem, &desc);
}

static Eina_Bool
_eina_value_type_blob_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   const Eina_Value_Blob_Operations *ops = _eina_value_type_blob_ops_get(mem);
   Eina_Value_Blob *tmem = mem;
   const Eina_Value_Blob *desc = ptr;

   if ((tmem->memory) && (tmem->memory == desc->memory))
     {
        tmem->ops = desc->ops;
        tmem->size = desc->size;
        return EINA_TRUE;
     }

   if ((ops) && (ops->free))
     ops->free(ops, (void *)tmem->memory, tmem->size);

   *tmem = *desc;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_blob_vset(const Eina_Value_Type *type, void *mem, va_list args)
{
   const Eina_Value_Blob desc = va_arg(args, Eina_Value_Blob);
   _eina_value_type_blob_pset(type, mem, &desc);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_blob_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   memcpy(ptr, mem, sizeof(Eina_Value_Blob));
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_BLOB = {
  EINA_VALUE_TYPE_VERSION,
  sizeof(Eina_Value_Blob),
  "Eina_Value_Blob",
  _eina_value_type_blob_setup,
  _eina_value_type_blob_flush,
  _eina_value_type_blob_copy,
  _eina_value_type_blob_compare,
  _eina_value_type_blob_convert_to,
  _eina_value_type_blob_convert_from,
  _eina_value_type_blob_vset,
  _eina_value_type_blob_pset,
  _eina_value_type_blob_pget
};

static int
_eina_value_struct_operations_binsearch_cmp(const void *pa, const void *pb)
{
   const Eina_Value_Struct_Member *a = pa, *b = pb;
   return strcmp(a->name, b->name);
}

static const Eina_Value_Struct_Member *
_eina_value_struct_operations_binsearch_find_member(const Eina_Value_Struct_Operations *ops EINA_UNUSED, const Eina_Value_Struct_Desc *desc, const char *name)
{
   unsigned int count = desc->member_count;
   Eina_Value_Struct_Member search;
   if (count == 0)
     {
        const Eina_Value_Struct_Member *itr = desc->members;
        for (; itr->name != NULL; itr++)
          count++;
     }

   search.name = name;
   return bsearch(&search, desc->members, count,
                  sizeof(Eina_Value_Struct_Member),
                  _eina_value_struct_operations_binsearch_cmp);
}

static Eina_Value_Struct_Operations _EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH = {
  EINA_VALUE_STRUCT_OPERATIONS_VERSION,
  NULL, /* default alloc */
  NULL, /* default free */
  NULL, /* default copy */
  NULL, /* default compare */
  _eina_value_struct_operations_binsearch_find_member
};

static const Eina_Value_Struct_Member *
_eina_value_struct_operations_stringshare_find_member(const Eina_Value_Struct_Operations *ops EINA_UNUSED, const Eina_Value_Struct_Desc *desc, const char *name)
{
   const Eina_Value_Struct_Member *itr = desc->members;

   /* assumes name is stringshared.
    *
    * we do this because it's the recommended usage pattern, moreover
    * we expect to find the member, as users shouldn't look for
    * non-existent members!
    */
   if (desc->member_count > 0)
     {
        const Eina_Value_Struct_Member *itr_end = itr + desc->member_count;
        for (; itr < itr_end; itr++)
          if (itr->name == name)
            return itr;
     }
   else
     {
        for (; itr->name != NULL; itr++)
          if (itr->name == name)
            return itr;
     }

   itr = desc->members;
   name = eina_stringshare_add(name);
   eina_stringshare_del(name); /* we'll not use the contents, this is fine */
   /* stringshare and look again */
   if (desc->member_count > 0)
     {
        const Eina_Value_Struct_Member *itr_end = itr + desc->member_count;
        for (; itr < itr_end; itr++)
          if (itr->name == name)
            return itr;
     }
   else
     {
        for (; itr->name != NULL; itr++)
          if (itr->name == name)
            return itr;
     }

   return NULL;
}

static Eina_Value_Struct_Operations _EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE = {
  EINA_VALUE_STRUCT_OPERATIONS_VERSION,
  NULL, /* default alloc */
  NULL, /* default free */
  NULL, /* default copy */
  NULL, /* default compare */
  _eina_value_struct_operations_stringshare_find_member
};

static inline const Eina_Value_Struct_Operations *
_eina_value_type_struct_ops_get(const Eina_Value_Struct *st)
{
   if (!st) return NULL;
   if (!st->desc) return NULL;
   if (!st->desc->ops) return NULL;
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (st->desc->ops->version == EINA_VALUE_STRUCT_OPERATIONS_VERSION, NULL);
   return st->desc->ops;
}

EAPI const Eina_Value_Struct_Member *
eina_value_struct_member_find(const Eina_Value_Struct *st, const char *name)
{
   const Eina_Value_Struct_Operations *ops;
   const Eina_Value_Struct_Member *itr;

   EINA_SAFETY_ON_NULL_RETURN_VAL(st, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(st->desc, NULL);

   ops = _eina_value_type_struct_ops_get(st);
   if ((ops) && (ops->find_member))
     return ops->find_member(ops, st->desc, name);

   itr = st->desc->members;
   if (st->desc->member_count)
     {
        const Eina_Value_Struct_Member *itr_end = itr + st->desc->member_count;
        for (; itr < itr_end; itr++)
          {
             if (strcmp(name, itr->name) == 0)
               return itr;
          }
        return NULL;
     }
   else
     {
        for (; itr->name != NULL; itr++)
          {
             if (strcmp(name, itr->name) == 0)
               return itr;
          }
        return NULL;
     }
}

static Eina_Bool
_eina_value_type_struct_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   memset(mem, 0, sizeof(Eina_Value_Struct));
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_struct_setup_member(const Eina_Value_Struct_Member *member, Eina_Value_Struct *st)
{
   unsigned char *base = st->memory;
   return eina_value_type_setup(member->type, base + member->offset);
}

static Eina_Bool
_eina_value_type_struct_flush_member(const Eina_Value_Struct_Member *member, Eina_Value_Struct *st)
{
   unsigned char *base = st->memory;
   return eina_value_type_flush(member->type, base + member->offset);
}

static Eina_Bool
_eina_value_type_struct_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   const Eina_Value_Struct_Operations *ops;
   const Eina_Value_Struct_Member *itr;
   Eina_Value_Struct *tmem = mem;
   Eina_Bool ret = EINA_TRUE;

   if ((!tmem->desc) || (!tmem->memory))
     return EINA_TRUE;

   itr = tmem->desc->members;
   if (tmem->desc->member_count > 0)
     {
        const Eina_Value_Struct_Member *itr_end;
        itr_end = itr + tmem->desc->member_count;
        for (; itr < itr_end; itr++)
          ret &= _eina_value_type_struct_flush_member(itr, tmem);
     }
   else
     {
        for (; itr->name != NULL; itr++)
          ret &= _eina_value_type_struct_flush_member(itr, tmem);
     }

   ops = _eina_value_type_struct_ops_get(mem);
   if ((ops) && (ops->free))
     ops->free(ops, tmem->desc, tmem->memory);
   else
     free(tmem->memory);

   tmem->memory = NULL;
   tmem->desc = NULL;

   return ret;
}

static Eina_Bool
_eina_value_type_struct_copy_member(const Eina_Value_Struct_Member *member, const Eina_Value_Struct *s, Eina_Value_Struct *d)
{
   const unsigned char *base_s = s->memory;
   unsigned char *base_d = d->memory;
   return eina_value_type_copy(member->type,
                               base_s + member->offset,
                               base_d + member->offset);
}

static Eina_Bool
_eina_value_type_struct_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const Eina_Value_Struct_Operations *ops;
   const Eina_Value_Struct_Member *itr;
   const Eina_Value_Struct *s = src;
   Eina_Value_Struct *d = dst;

   *d = *s;

   if ((!s->desc) || (!s->memory))
     return EINA_TRUE;

   ops = _eina_value_type_struct_ops_get(src);
   if ((ops) && (ops->copy))
     {
        d->memory = ops->copy(ops, s->desc, s->memory);
        if (d->memory == NULL)
          return EINA_FALSE;
        return EINA_TRUE;
     }

   if ((ops) && (ops->alloc))
     d->memory = ops->alloc(ops, s->desc);
   else
     d->memory = malloc(s->desc->size);
   if (!d->memory)
     {
        return EINA_FALSE;
     }

   itr = s->desc->members;
   if (s->desc->member_count > 0)
     {
        const Eina_Value_Struct_Member *itr_end = itr + s->desc->member_count;
        for (; itr < itr_end; itr++)
          if (!_eina_value_type_struct_copy_member(itr, s, d))
            goto error;
     }
   else
     {
        for (; itr->name != NULL; itr++)
          if (!_eina_value_type_struct_copy_member(itr, s, d))
            goto error;
     }

   return EINA_TRUE;

 error:
   itr--;
   for (; itr >= s->desc->members; itr--)
     _eina_value_type_struct_flush_member(itr, d);

   if ((ops) && (ops->free))
     ops->free(ops, s->desc, d->memory);
   else
     free(d->memory);
   return EINA_FALSE;
}

static inline int
_eina_value_type_struct_compare_member(const Eina_Value_Struct_Member *member, const Eina_Value_Struct *ta, const Eina_Value_Struct *tb)
{
   const unsigned char *base_a = ta->memory;
   const unsigned char *base_b = tb->memory;
   return eina_value_type_compare(member->type,
                                  base_a + member->offset,
                                  base_b + member->offset);
}

static int
_eina_value_type_struct_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const Eina_Value_Struct_Operations *ops = _eina_value_type_struct_ops_get(a);
   const Eina_Value_Struct *ta = a, *tb = b;
   const Eina_Value_Struct_Member *itr;
   int cmp = 0;

   if ((!ta->desc) && (!tb->desc))
     return 0;
   else if (ta->desc != tb->desc)
     {
        return -1;
     }
   if (ta->desc->ops != tb->desc->ops)
     {
        return -1;
     }
   if ((!ta->memory) && (!tb->memory))
     return 0;
   else if (!ta->memory)
     return -1;
   else if (!tb->memory)
     return 1;

   if ((ops) && (ops->compare))
     return ops->compare(ops, ta->desc, ta->memory, tb->memory);

   itr = ta->desc->members;
   if (ta->desc->member_count > 0)
     {
        const Eina_Value_Struct_Member *itr_end = itr + ta->desc->member_count;
        for (; (cmp == 0) && (itr < itr_end); itr++)
          cmp = _eina_value_type_struct_compare_member(itr, ta, tb);
     }
   else
     {
        for (; (cmp == 0) && (itr->name != NULL); itr++)
          cmp = _eina_value_type_struct_compare_member(itr, ta, tb);
     }
   return cmp;
}

static void
_eina_value_type_struct_convert_to_string_member(const Eina_Value_Struct *st, const Eina_Value_Struct_Member *member, Eina_Strbuf *str)
{
   const unsigned char *p = st->memory;
   Eina_Bool first = st->desc->members == member;
   Eina_Bool r = EINA_FALSE;

   if (first) eina_strbuf_append_printf(str, "%s: ", member->name);
   else eina_strbuf_append_printf(str, ", %s: ", member->name);

   if ((member->type) && (member->type->convert_to))
     {
        const Eina_Value_Type *type = member->type;
        char *conv = NULL;

        r = eina_value_type_convert_to(type, EINA_VALUE_TYPE_STRING,
                                       p + member->offset, &conv);
        if (r)
          {
             eina_strbuf_append(str, conv);
             free(conv);
          }
     }

   if (!r)
     eina_strbuf_append_char(str, '?');
}

static Eina_Bool
_eina_value_type_struct_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const Eina_Value_Struct *tmem = type_mem;

   if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
       convert == EINA_VALUE_TYPE_STRING)
     {
        Eina_Strbuf *str = eina_strbuf_new();
        const char *s;
        Eina_Bool ret;

        if (!tmem->memory) eina_strbuf_append(str, "{}");
        else
          {
             const Eina_Value_Struct_Member *itr = tmem->desc->members;

             eina_strbuf_append_char(str, '{');

             if (tmem->desc->member_count > 0)
               {
                  const Eina_Value_Struct_Member *itr_end;

                  itr_end = itr + tmem->desc->member_count;
                  for (; itr < itr_end; itr++)
                    _eina_value_type_struct_convert_to_string_member
                      (tmem, itr, str);
               }
             else
               {
                  for (; itr->name != NULL; itr++)
                    _eina_value_type_struct_convert_to_string_member
                      (tmem, itr, str);
               }

             eina_strbuf_append_char(str, '}');
          }
        s = eina_strbuf_string_get(str);
        ret = eina_value_type_pset(convert, convert_mem, &s);
        eina_strbuf_free(str);
        return ret;
     }
   else
     {
        return EINA_FALSE;
     }
}

static Eina_Bool
_eina_value_type_struct_desc_check(const Eina_Value_Struct_Desc *desc)
{
   unsigned int minsize = 0;
   const Eina_Value_Struct_Member *itr;

   EINA_SAFETY_ON_NULL_RETURN_VAL(desc, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL
     (desc->version == EINA_VALUE_STRUCT_DESC_VERSION, EINA_FALSE);

   itr = desc->members;
   if (desc->member_count > 0)
     {
        const Eina_Value_Struct_Member *itr_end = itr + desc->member_count;
        for (; itr < itr_end; itr++)
          {
             unsigned int member_end;

             EINA_SAFETY_ON_FALSE_RETURN_VAL
               (eina_value_type_check(itr->type), EINA_FALSE);
             EINA_SAFETY_ON_FALSE_RETURN_VAL
               (itr->type->value_size > 0, EINA_FALSE);

             member_end = itr->offset + itr->type->value_size;
             if (minsize < member_end)
               minsize = member_end;
          }
     }
   else
     {
        for (; itr->name != NULL; itr++)
          {
             unsigned int member_end;

             EINA_SAFETY_ON_FALSE_RETURN_VAL
               (eina_value_type_check(itr->type), EINA_FALSE);
             EINA_SAFETY_ON_FALSE_RETURN_VAL
               (itr->type->value_size > 0, EINA_FALSE);

             member_end = itr->offset + itr->type->value_size;
             if (minsize < member_end)
               minsize = member_end;
          }
     }

   EINA_SAFETY_ON_FALSE_RETURN_VAL(minsize > 0, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(desc->size >= minsize, EINA_FALSE);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_struct_pset(const Eina_Value_Type *type, void *mem, const void *ptr)
{
   const Eina_Value_Struct_Operations *ops;
   Eina_Value_Struct *tmem = mem;
   const Eina_Value_Struct *desc = ptr;
   const Eina_Value_Struct_Member *itr;

   if (!_eina_value_type_struct_desc_check(desc->desc))
     {
        return EINA_FALSE;
     }

   if ((tmem->memory) && (tmem->memory == desc->memory))
     {
        tmem->desc = desc->desc;
        return EINA_TRUE;
     }

   if (desc->memory)
     {
        Eina_Value_Struct tmp;

        if (!_eina_value_type_struct_copy(type, desc, &tmp))
          return EINA_FALSE;

        _eina_value_type_struct_flush(type, tmem);
        memcpy(tmem, &tmp, sizeof(tmp));
        return EINA_TRUE;
     }

   if (tmem->memory) _eina_value_type_struct_flush(type, mem);

   tmem->desc = desc->desc;

   ops = _eina_value_type_struct_ops_get(desc);
   if ((ops) && (ops->alloc))
     tmem->memory = ops->alloc(ops, tmem->desc);
   else
     tmem->memory = malloc(tmem->desc->size);

   if (!tmem->memory)
     {
        return EINA_FALSE;
     }

   itr = tmem->desc->members;
   if (tmem->desc->member_count > 0)
     {
        const Eina_Value_Struct_Member *itr_end;
        itr_end = itr + tmem->desc->member_count;
        for (; itr < itr_end; itr++)
          if (!_eina_value_type_struct_setup_member(itr, tmem))
            goto error;
     }
   else
     {
        for (; itr->name != NULL; itr++)
          if (!_eina_value_type_struct_setup_member(itr, tmem))
            goto error;
     }

   return EINA_TRUE;

 error:
   itr--;
   for (; itr >= tmem->desc->members; itr--)
     _eina_value_type_struct_flush_member(itr, tmem);

   if ((ops) && (ops->free))
     ops->free(ops, tmem->desc, tmem->memory);
   else
     free(tmem->memory);
   tmem->memory = NULL;
   tmem->desc = NULL;
   return EINA_FALSE;
}

static Eina_Bool
_eina_value_type_struct_vset(const Eina_Value_Type *type, void *mem, va_list args)
{
   const Eina_Value_Struct desc = va_arg(args, Eina_Value_Struct);
   _eina_value_type_struct_pset(type, mem, &desc);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_struct_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   memcpy(ptr, mem, sizeof(Eina_Value_Struct));
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_STRUCT = {
  EINA_VALUE_TYPE_VERSION,
  sizeof(Eina_Value_Struct),
  "Eina_Value_Struct",
  _eina_value_type_struct_setup,
  _eina_value_type_struct_flush,
  _eina_value_type_struct_copy,
  _eina_value_type_struct_compare,
  _eina_value_type_struct_convert_to,
  NULL, /* no convert from */
  _eina_value_type_struct_vset,
  _eina_value_type_struct_pset,
  _eina_value_type_struct_pget
};

/* no model for now
static Eina_Bool
_eina_value_type_model_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eina_Model **tmem = mem;
   *tmem = NULL;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_model_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Eina_Model **tmem = mem;
   if (*tmem)
     {
        eina_model_unref(*tmem);
        *tmem = NULL;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_model_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   const Eina_Model * const *s = src;
   Eina_Model **d = dst;
   if (*s)
     *d = eina_model_copy(*s); // is it better to deep-copy?
   else
     *d = NULL;
   return EINA_TRUE;
}

static int
_eina_value_type_model_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   const Eina_Model * const *ta = a;
   const Eina_Model * const *tb = b;

   if ((!*ta) && (!*tb)) return 0;
   else if (!*ta) return 1;
   else if (!*tb) return -1;
   else return eina_model_compare(*ta, *tb);
}

static Eina_Bool
_eina_value_type_model_convert_to(const Eina_Value_Type *type EINA_UNUSED, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const Eina_Model *v = *(const Eina_Model **)type_mem;

   if (convert == EINA_VALUE_TYPE_STRINGSHARE ||
       convert == EINA_VALUE_TYPE_STRING)
     {
        char *other_mem = v ? eina_model_to_string(v) : NULL;
        Eina_Bool ret = eina_value_type_pset(convert, convert_mem, &other_mem);
        free(other_mem);
        return ret;
     }
   else
     {
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_model_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   Eina_Model **tmem = mem, *tmp;

   tmp = va_arg(args, Eina_Model *);

   if (tmp) eina_model_ref(tmp);
   if (*tmem) eina_model_unref(*tmem);

   *tmem = tmp;

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_model_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   Eina_Model **tmem = mem;
   Eina_Model **p = (Eina_Model **)ptr;

   if (*tmem == *p) return EINA_TRUE;

   if (*p) eina_model_ref(*p);
   if (*tmem) eina_model_unref(*tmem);

   *tmem = *p;

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_model_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   Eina_Model **tmem = (Eina_Model **)mem;
   Eina_Model **p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_MODEL = {
  EINA_VALUE_TYPE_VERSION,
  sizeof(Eina_Model *),
  "Eina_Model",
  _eina_value_type_model_setup,
  _eina_value_type_model_flush,
  _eina_value_type_model_copy,
  _eina_value_type_model_compare,
  _eina_value_type_model_convert_to,
  NULL, // no convert from
  _eina_value_type_model_vset,
  _eina_value_type_model_pset,
  _eina_value_type_model_pget
};
*/

/* keep all basic types inlined in an array so we can compare if it's
 * a basic type using pointer arithmetic.
 *
 * NOTE-1: JUST BASIC TYPES, DO NOT ADD MORE TYPES HERE!!!
 * NOTE-2: KEEP ORDER, see eina_value_init()
 */
static const Eina_Value_Type _EINA_VALUE_TYPE_BASICS[] = {
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(unsigned char),
    "unsigned char",
    _eina_value_type_uchar_setup,
    _eina_value_type_uchar_flush,
    _eina_value_type_uchar_copy,
    _eina_value_type_uchar_compare,
    _eina_value_type_uchar_convert_to,
    NULL, /* no convert from */
    _eina_value_type_uchar_vset,
    _eina_value_type_uchar_pset,
    _eina_value_type_uchar_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(unsigned short),
    "unsigned short",
    _eina_value_type_ushort_setup,
    _eina_value_type_ushort_flush,
    _eina_value_type_ushort_copy,
    _eina_value_type_ushort_compare,
    _eina_value_type_ushort_convert_to,
    NULL, /* no convert from */
    _eina_value_type_ushort_vset,
    _eina_value_type_ushort_pset,
    _eina_value_type_ushort_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(unsigned int),
    "unsigned int",
    _eina_value_type_uint_setup,
    _eina_value_type_uint_flush,
    _eina_value_type_uint_copy,
    _eina_value_type_uint_compare,
    _eina_value_type_uint_convert_to,
    NULL, /* no convert from */
    _eina_value_type_uint_vset,
    _eina_value_type_uint_pset,
    _eina_value_type_uint_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(unsigned long),
    "unsigned long",
    _eina_value_type_ulong_setup,
    _eina_value_type_ulong_flush,
    _eina_value_type_ulong_copy,
    _eina_value_type_ulong_compare,
    _eina_value_type_ulong_convert_to,
    NULL, /* no convert from */
    _eina_value_type_ulong_vset,
    _eina_value_type_ulong_pset,
    _eina_value_type_ulong_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(uint64_t),
    "uint64_t",
    _eina_value_type_uint64_setup,
    _eina_value_type_uint64_flush,
    _eina_value_type_uint64_copy,
    _eina_value_type_uint64_compare,
    _eina_value_type_uint64_convert_to,
    NULL, /* no convert from */
    _eina_value_type_uint64_vset,
    _eina_value_type_uint64_pset,
    _eina_value_type_uint64_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(char),
    "char",
    _eina_value_type_char_setup,
    _eina_value_type_char_flush,
    _eina_value_type_char_copy,
    _eina_value_type_char_compare,
    _eina_value_type_char_convert_to,
    NULL, /* no convert from */
    _eina_value_type_char_vset,
    _eina_value_type_char_pset,
    _eina_value_type_char_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(short),
    "short",
    _eina_value_type_short_setup,
    _eina_value_type_short_flush,
    _eina_value_type_short_copy,
    _eina_value_type_short_compare,
    _eina_value_type_short_convert_to,
    NULL, /* no convert from */
    _eina_value_type_short_vset,
    _eina_value_type_short_pset,
    _eina_value_type_short_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(int),
    "int",
    _eina_value_type_int_setup,
    _eina_value_type_int_flush,
    _eina_value_type_int_copy,
    _eina_value_type_int_compare,
    _eina_value_type_int_convert_to,
    NULL, /* no convert from */
    _eina_value_type_int_vset,
    _eina_value_type_int_pset,
    _eina_value_type_int_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(long),
    "long",
    _eina_value_type_long_setup,
    _eina_value_type_long_flush,
    _eina_value_type_long_copy,
    _eina_value_type_long_compare,
    _eina_value_type_long_convert_to,
    NULL, /* no convert from */
    _eina_value_type_long_vset,
    _eina_value_type_long_pset,
    _eina_value_type_long_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(int64_t),
    "int64_t",
    _eina_value_type_int64_setup,
    _eina_value_type_int64_flush,
    _eina_value_type_int64_copy,
    _eina_value_type_int64_compare,
    _eina_value_type_int64_convert_to,
    NULL, /* no convert from */
    _eina_value_type_int64_vset,
    _eina_value_type_int64_pset,
    _eina_value_type_int64_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(float),
    "float",
    _eina_value_type_float_setup,
    _eina_value_type_float_flush,
    _eina_value_type_float_copy,
    _eina_value_type_float_compare,
    _eina_value_type_float_convert_to,
    NULL, /* no convert from */
    _eina_value_type_float_vset,
    _eina_value_type_float_pset,
    _eina_value_type_float_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(double),
    "double",
    _eina_value_type_double_setup,
    _eina_value_type_double_flush,
    _eina_value_type_double_copy,
    _eina_value_type_double_compare,
    _eina_value_type_double_convert_to,
    NULL, /* no convert from */
    _eina_value_type_double_vset,
    _eina_value_type_double_pset,
    _eina_value_type_double_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(const char *),
    "stringshare",
    _eina_value_type_string_common_setup,
    _eina_value_type_stringshare_flush,
    _eina_value_type_stringshare_copy,
    _eina_value_type_string_common_compare,
    _eina_value_type_string_common_convert_to,
    NULL, /* no convert from */
    _eina_value_type_stringshare_vset,
    _eina_value_type_stringshare_pset,
    _eina_value_type_string_common_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(char *),
    "string",
    _eina_value_type_string_common_setup,
    _eina_value_type_string_flush,
    _eina_value_type_string_copy,
    _eina_value_type_string_common_compare,
    _eina_value_type_string_common_convert_to,
    NULL, /* no convert from */
    _eina_value_type_string_vset,
    _eina_value_type_string_pset,
    _eina_value_type_string_common_pget
  },
  {
    EINA_VALUE_TYPE_VERSION,
    sizeof(unsigned long),
    "timestamp",
    _eina_value_type_ulong_setup,
    _eina_value_type_ulong_flush,
    _eina_value_type_ulong_copy,
    _eina_value_type_ulong_compare,
    _eina_value_type_ulong_convert_to,
    NULL, /* no convert from */
    _eina_value_type_ulong_vset,
    _eina_value_type_ulong_pset,
    _eina_value_type_ulong_pget
  }
};

static void
_eina_value_blob_operations_malloc_free(const Eina_Value_Blob_Operations *ops EINA_UNUSED, void *memory, size_t size EINA_UNUSED)
{
   free(memory);
}

static void *
_eina_value_blob_operations_malloc_copy(const Eina_Value_Blob_Operations *ops EINA_UNUSED, const void *memory, size_t size)
{
   void *ret = malloc(size);
   if (!ret) return NULL;
   memcpy(ret, memory, size);
   return ret;
}

static const Eina_Value_Blob_Operations _EINA_VALUE_BLOB_OPERATIONS_MALLOC = {
  EINA_VALUE_BLOB_OPERATIONS_VERSION,
  _eina_value_blob_operations_malloc_free,
  _eina_value_blob_operations_malloc_copy,
  NULL,
  NULL
};

typedef struct _Eina_Value_Inner_Mp Eina_Value_Inner_Mp;
struct _Eina_Value_Inner_Mp
{
   Eina_Mempool *mempool;
   int references;
};

/**
 * @endcond
 */

/**
 */

static inline void
_eina_value_inner_mp_dispose(int size, Eina_Value_Inner_Mp *imp)
{
   EINA_SAFETY_ON_FALSE_RETURN(imp->references == 0);

   eina_hash_del_by_key(_eina_value_inner_mps, &size);
   eina_mempool_del(imp->mempool);
   free(imp);
}

static inline Eina_Value_Inner_Mp *
_eina_value_inner_mp_get(int size)
{
   Eina_Value_Inner_Mp *imp = eina_hash_find(_eina_value_inner_mps, &size);
   if (imp) return imp;

   imp = malloc(sizeof(Eina_Value_Inner_Mp));
   if (!imp)
     return NULL;

   imp->references = 0;

   imp->mempool = eina_mempool_add(_eina_value_mp_choice,
                                   "Eina_Value_Inner_Mp", NULL, size, 16);
   if (!imp->mempool)
     {
        free(imp);
        return NULL;
     }

   if (!eina_hash_add(_eina_value_inner_mps, &size, imp))
     {
        eina_mempool_del(imp->mempool);
        free(imp);
        return NULL;
     }

   return imp;
}

static inline void *
_eina_value_inner_alloc_internal(int size)
{
   Eina_Value_Inner_Mp *imp;
   void *mem;

   imp = _eina_value_inner_mp_get(size);
   if (!imp) return NULL;

   mem = eina_mempool_malloc(imp->mempool, size);
   if (mem) imp->references++;
   else if (imp->references == 0) _eina_value_inner_mp_dispose(size, imp);

   return mem;
}

static inline void
_eina_value_inner_free_internal(int size, void *mem)
{
   Eina_Value_Inner_Mp *imp = eina_hash_find(_eina_value_inner_mps, &size);
   EINA_SAFETY_ON_NULL_RETURN(imp);

   eina_mempool_free(imp->mempool, mem);

   imp->references--;
   if (imp->references > 0) return;
   _eina_value_inner_mp_dispose(size, imp);
}

EAPI void *
eina_value_inner_alloc(size_t size)
{
   void *mem;

   if (size > 256) return malloc(size);

   eina_lock_take(&_eina_value_inner_mps_lock);
   mem = _eina_value_inner_alloc_internal(size);
   eina_lock_release(&_eina_value_inner_mps_lock);

   return mem;
}

EAPI void
eina_value_inner_free(size_t size, void *mem)
{
   if (size > 256)
     {
        free(mem);
        return;
     }

   eina_lock_take(&_eina_value_inner_mps_lock);
   _eina_value_inner_free_internal(size, mem);
   eina_lock_release(&_eina_value_inner_mps_lock);
}

/**
 * @internal
 * @brief Initialize the value module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the value module of Eina. It is called
 * by eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_value_init(void)
{
   const char *choice, *tmp;

   _eina_value_log_dom = eina_log_domain_register("eina_value",
                                                  EINA_LOG_COLOR_DEFAULT);
   if (_eina_value_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_value");
        return EINA_FALSE;
     }

#ifdef EINA_DEFAULT_MEMPOOL
   choice = "pass_through";
#else
   choice = "chained_mempool";
#endif
   tmp = getenv("EINA_MEMPOOL");
   if (tmp && tmp[0])
     choice = tmp;

   _eina_value_mp_choice = strdup(choice);

   _eina_value_mp = eina_mempool_add
      (_eina_value_mp_choice, "value", NULL, sizeof(Eina_Value), 32);
   if (!_eina_value_mp)
     {
        ERR("Mempool for value cannot be allocated in value init.");
        goto on_init_fail_mp;
     }

   if (!eina_lock_new(&_eina_value_inner_mps_lock))
     {
        ERR("Cannot create lock in value init.");
        goto on_init_fail_lock;
     }
   _eina_value_inner_mps = eina_hash_int32_new(NULL);
   if (!_eina_value_inner_mps)
     {
        ERR("Cannot create hash for inner mempools in value init.");
        goto on_init_fail_hash;
     }

   EINA_VALUE_TYPE_UCHAR       = _EINA_VALUE_TYPE_BASICS +  0;
   EINA_VALUE_TYPE_USHORT      = _EINA_VALUE_TYPE_BASICS +  1;
   EINA_VALUE_TYPE_UINT        = _EINA_VALUE_TYPE_BASICS +  2;
   EINA_VALUE_TYPE_ULONG       = _EINA_VALUE_TYPE_BASICS +  3;
   EINA_VALUE_TYPE_UINT64      = _EINA_VALUE_TYPE_BASICS +  4;
   EINA_VALUE_TYPE_CHAR        = _EINA_VALUE_TYPE_BASICS +  5;
   EINA_VALUE_TYPE_SHORT       = _EINA_VALUE_TYPE_BASICS +  6;
   EINA_VALUE_TYPE_INT         = _EINA_VALUE_TYPE_BASICS +  7;
   EINA_VALUE_TYPE_LONG        = _EINA_VALUE_TYPE_BASICS +  8;
   EINA_VALUE_TYPE_INT64       = _EINA_VALUE_TYPE_BASICS +  9;
   EINA_VALUE_TYPE_FLOAT       = _EINA_VALUE_TYPE_BASICS + 10;
   EINA_VALUE_TYPE_DOUBLE      = _EINA_VALUE_TYPE_BASICS + 11;
   EINA_VALUE_TYPE_STRINGSHARE = _EINA_VALUE_TYPE_BASICS + 12;
   EINA_VALUE_TYPE_STRING      = _EINA_VALUE_TYPE_BASICS + 13;
   EINA_VALUE_TYPE_TIMESTAMP   = _EINA_VALUE_TYPE_BASICS +  14;

   _EINA_VALUE_TYPE_BASICS_START = _EINA_VALUE_TYPE_BASICS +  0;
   _EINA_VALUE_TYPE_BASICS_END   = _EINA_VALUE_TYPE_BASICS + 14;

   EINA_SAFETY_ON_FALSE_RETURN_VAL((sizeof(_EINA_VALUE_TYPE_BASICS)/sizeof(_EINA_VALUE_TYPE_BASICS[0])) == 15, EINA_FALSE);


   EINA_VALUE_TYPE_ARRAY = &_EINA_VALUE_TYPE_ARRAY;
   EINA_VALUE_TYPE_LIST = &_EINA_VALUE_TYPE_LIST;
   EINA_VALUE_TYPE_HASH = &_EINA_VALUE_TYPE_HASH;
   EINA_VALUE_TYPE_TIMEVAL = &_EINA_VALUE_TYPE_TIMEVAL;
   EINA_VALUE_TYPE_BLOB = &_EINA_VALUE_TYPE_BLOB;
   EINA_VALUE_TYPE_STRUCT = &_EINA_VALUE_TYPE_STRUCT;
/* no model for now
   EINA_VALUE_TYPE_MODEL = &_EINA_VALUE_TYPE_MODEL;
 */
   
   EINA_VALUE_BLOB_OPERATIONS_MALLOC = &_EINA_VALUE_BLOB_OPERATIONS_MALLOC;

   EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH = &_EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH;
   EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE = &_EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE;

   return EINA_TRUE;

 on_init_fail_hash:
   eina_lock_free(&_eina_value_inner_mps_lock);
 on_init_fail_lock:
   eina_mempool_del(_eina_value_mp);
 on_init_fail_mp:
   free(_eina_value_mp_choice);
   _eina_value_mp_choice = NULL;
   eina_log_domain_unregister(_eina_value_log_dom);
   _eina_value_log_dom = -1;
   return EINA_FALSE;
}

/**
 * @internal
 * @brief Shut down the value module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the value module set up by
 * eina_value_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_value_shutdown(void)
{
   eina_lock_take(&_eina_value_inner_mps_lock);
   if (eina_hash_population(_eina_value_inner_mps) != 0)
     ERR("Cannot free eina_value internal memory pools -- still in use!");
   else
     eina_hash_free(_eina_value_inner_mps);
   eina_lock_release(&_eina_value_inner_mps_lock);
   eina_lock_free(&_eina_value_inner_mps_lock);

   free(_eina_value_mp_choice);
   _eina_value_mp_choice = NULL;
   eina_mempool_del(_eina_value_mp);
   eina_log_domain_unregister(_eina_value_log_dom);
   _eina_value_log_dom = -1;
   return EINA_TRUE;
}

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI const Eina_Value_Type *_EINA_VALUE_TYPE_BASICS_START = NULL;
EAPI const Eina_Value_Type *_EINA_VALUE_TYPE_BASICS_END = NULL;

EAPI const Eina_Value_Type *EINA_VALUE_TYPE_UCHAR = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_USHORT = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_UINT = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_ULONG = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_TIMESTAMP = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_UINT64 = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_CHAR = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_SHORT = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_INT = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_LONG = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_INT64 = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_FLOAT = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_DOUBLE = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_STRINGSHARE = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_STRING = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_ARRAY = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_LIST = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_HASH = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_TIMEVAL = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_BLOB = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_STRUCT = NULL;
/* no model for now
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_MODEL = NULL;
 */

EAPI const Eina_Value_Blob_Operations *EINA_VALUE_BLOB_OPERATIONS_MALLOC = NULL;

EAPI const Eina_Value_Struct_Operations *EINA_VALUE_STRUCT_OPERATIONS_BINSEARCH = NULL;
EAPI const Eina_Value_Struct_Operations *EINA_VALUE_STRUCT_OPERATIONS_STRINGSHARE = NULL;

EAPI Eina_Error EINA_ERROR_VALUE_FAILED = 0;

EAPI const unsigned int eina_prime_table[] =
{
   17, 31, 61, 127, 257, 509, 1021,
   2053, 4093, 8191, 16381, 32771, 65537, 131071, 262147, 524287, 1048573,
   2097143, 4194301, 8388617, 16777213
};

EAPI Eina_Value *
eina_value_new(const Eina_Value_Type *type)
{
   Eina_Value *value = eina_mempool_malloc(_eina_value_mp, sizeof(Eina_Value));
   if (!value) return NULL;
   if (!eina_value_setup(value, type))
     {
        free(value);
        return NULL;
     }
   return value;
}

EAPI void
eina_value_free(Eina_Value *value)
{
   if (!value) return;
   eina_value_flush(value);
   eina_mempool_free(_eina_value_mp, value);
}


EAPI Eina_Bool
eina_value_copy(const Eina_Value *value, Eina_Value *copy)
{
   const Eina_Value_Type *type;
   const void *src;
   void *dst;
   Eina_Bool ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(value->type),
                                   EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(copy, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value->type->copy, EINA_FALSE);

   type = value->type;
   if (!eina_value_setup(copy, type))
     return EINA_FALSE;

   src = eina_value_memory_get(value);
   dst = eina_value_memory_get(copy);
   ret = type->copy(type, src, dst);
   if (!ret)
     eina_value_flush(copy);

   return ret;
}

EAPI Eina_Bool
eina_value_convert(const Eina_Value *value, Eina_Value *convert)
{
   Eina_Bool ret = EINA_FALSE;
   const Eina_Value_Type *type, *convert_type;
   const void *type_mem;
   void *convert_mem;

   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(convert, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(value->type),
                                   EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(convert->type),
                                   EINA_FALSE);

   type = value->type;
   convert_type = convert->type;

   type_mem = eina_value_memory_get(value);
   convert_mem = eina_value_memory_get(convert);

   if (type->convert_to)
     ret = type->convert_to(type, convert_type, type_mem, convert_mem);

   if ((!ret) && (convert_type->convert_from))
     ret = convert_type->convert_from(convert_type, type, convert_mem,
                                      type_mem);

   return ret;
}

EAPI char *
eina_value_to_string(const Eina_Value *value)
{
   Eina_Value tmp;

   EINA_SAFETY_ON_NULL_RETURN_VAL(value, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(value->type), NULL);

   if (!eina_value_setup(&tmp, EINA_VALUE_TYPE_STRING))
     return NULL;
   if (!eina_value_convert(value, &tmp))
     return NULL;

   return tmp.value.ptr; /* steal value */
}

EAPI Eina_Value *
eina_value_array_new(const Eina_Value_Type *subtype, unsigned int step)
{
   Eina_Value *value;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(subtype), NULL);

   value = eina_mempool_malloc(_eina_value_mp, sizeof(Eina_Value));
   if (!value)
     return NULL;

   if (!eina_value_array_setup(value, subtype, step))
     {
        eina_mempool_free(_eina_value_mp, value);
        return NULL;
     }

   return value;
}

EAPI Eina_Value *
eina_value_list_new(const Eina_Value_Type *subtype)
{
   Eina_Value *value;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(subtype), NULL);

   value = eina_mempool_malloc(_eina_value_mp, sizeof(Eina_Value));
   if (!value)
     return NULL;

   if (!eina_value_list_setup(value, subtype))
     {
        eina_mempool_free(_eina_value_mp, value);
        return NULL;
     }

   return value;
}

EAPI Eina_Value *
eina_value_hash_new(const Eina_Value_Type *subtype, unsigned int buckets_power_size)
{
   Eina_Value *value;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(subtype), NULL);

   value = eina_mempool_malloc(_eina_value_mp, sizeof(Eina_Value));
   if (!value)
     return NULL;

   if (!eina_value_hash_setup(value, subtype, buckets_power_size))
     {
        eina_mempool_free(_eina_value_mp, value);
        return NULL;
     }

   return value;
}

EAPI Eina_Value *
eina_value_struct_new(const Eina_Value_Struct_Desc *desc)
{
   Eina_Value *value;

   value = eina_mempool_malloc(_eina_value_mp, sizeof(Eina_Value));
   if (!value)
     return NULL;

   if (!eina_value_struct_setup(value, desc))
     {
        eina_mempool_free(_eina_value_mp, value);
        return NULL;
     }

   return value;
}

EAPI Eina_Bool
eina_value_type_check(const Eina_Value_Type *type)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(type, EINA_FALSE);
   return type->version == EINA_VALUE_TYPE_VERSION;
}

EAPI const char *
eina_value_type_name_get(const Eina_Value_Type *type)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(type), NULL);
   return type->name;
}
