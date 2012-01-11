/* eina_value.c

   Copyright (C) 2001 Christopher Rosendahl    <smugg@fatelabs.com>
                   Nathan Ingersoll         <ningerso@d.umn.edu>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies of the Software and its documentation and acknowledgment shall be
   given in the documentation and software packages that this Software was
   used.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include <stdio.h> /* asprintf() */
#include <inttypes.h> /* PRId64 and PRIu64 */

#include "eina_config.h"
#include "eina_private.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_strbuf.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_value.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

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
static const char eina_value_char_min = -127 - 1;

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
_eina_value_type_uchar_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   unsigned char *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const unsigned char *s = src;
   unsigned char *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_uchar_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const unsigned char *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_uchar_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const unsigned char v = *(const unsigned char *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   unsigned char *tmem = mem;
   *tmem = va_arg(args, unsigned int); /* char is promoted to int for va_arg */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   unsigned char *tmem = mem;
   const unsigned char *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uchar_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const unsigned char *tmem = mem;
   unsigned char *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_UCHAR = {
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
};

static Eina_Bool
_eina_value_type_ushort_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   unsigned short *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const unsigned short *s = src;
   unsigned short *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_ushort_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const unsigned short *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_ushort_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const unsigned short v = *(const unsigned short *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   unsigned short *tmem = mem;
   *tmem = va_arg(args, unsigned int); /* short is promoted to int for va_arg */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   unsigned short *tmem = mem;
   const unsigned short *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ushort_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const unsigned short *tmem = mem;
   unsigned short *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_USHORT = {
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
};

static Eina_Bool
_eina_value_type_uint_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   unsigned int *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const unsigned int *s = src;
   unsigned int *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_uint_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const unsigned int *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_uint_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const unsigned int v = *(const unsigned int *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   unsigned int *tmem = mem;
   *tmem = va_arg(args, unsigned int);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   unsigned int *tmem = mem;
   const unsigned int *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const unsigned int *tmem = mem;
   unsigned int *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_UINT = {
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
};

static Eina_Bool
_eina_value_type_ulong_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   unsigned long *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const unsigned long *s = src;
   unsigned long *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_ulong_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const unsigned long *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_ulong_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const unsigned long v = *(const unsigned long *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   unsigned long *tmem = mem;
   *tmem = va_arg(args, unsigned long);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   unsigned long *tmem = mem;
   const unsigned long *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_ulong_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const unsigned long *tmem = mem;
   unsigned long *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_ULONG = {
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
};


static Eina_Bool
_eina_value_type_uint64_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   uint64_t *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const uint64_t *s = src;
   uint64_t *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_uint64_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const uint64_t *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_uint64_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const uint64_t v = *(const uint64_t *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   uint64_t *tmem = mem;
   *tmem = va_arg(args, uint64_t);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   uint64_t *tmem = mem;
   const uint64_t *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_uint64_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const uint64_t *tmem = mem;
   uint64_t *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_UINT64 = {
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
};

static Eina_Bool
_eina_value_type_char_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   char *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const char *s = src;
   char *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_char_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const char *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_char_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const signed char v = *(const signed char *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   char *tmem = mem;
   *tmem = va_arg(args, int); /* char is promoted to int for va_arg */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   char *tmem = mem;
   const char *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_char_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const char *tmem = mem;
   char *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_CHAR = {
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
};

static Eina_Bool
_eina_value_type_short_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   short *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const short *s = src;
   short *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_short_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const short *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_short_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const short v = *(const short *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   short *tmem = mem;
   *tmem = va_arg(args, int); /* short int is promoted to int for va_arg */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   short *tmem = mem;
   const short *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_short_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const short *tmem = mem;
   short *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_SHORT = {
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
};

static Eina_Bool
_eina_value_type_int_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   int *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const int *s = src;
   int *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_int_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const int *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_int_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const int v = *(const int *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   int *tmem = mem;
   *tmem = va_arg(args, int);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   int *tmem = mem;
   const int *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const int *tmem = mem;
   int *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_INT = {
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
};

static Eina_Bool
_eina_value_type_long_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   long *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const long *s = src;
   long *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_long_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const long *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_long_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const long v = *(const long *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   long *tmem = mem;
   *tmem = va_arg(args, long);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   long *tmem = mem;
   const long *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_long_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const long *tmem = mem;
   long *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_LONG = {
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
};

static Eina_Bool
_eina_value_type_int64_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   int64_t *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const int64_t *s = src;
   int64_t *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_int64_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const int64_t *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_int64_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const int64_t v = *(const int64_t *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   int64_t *tmem = mem;
   *tmem = va_arg(args, int64_t);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   int64_t *tmem = mem;
   const int64_t *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_int64_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const int64_t *tmem = mem;
   int64_t *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_INT64 = {
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
};

static Eina_Bool
_eina_value_type_float_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   float *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const float *s = src;
   float *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_float_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const float *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_float_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const float v = *(const float *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   float *tmem = mem;
   *tmem = va_arg(args, double); /* float is promoted to double for va_args */
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   float *tmem = mem;
   const float *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_float_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const float *tmem = mem;
   float *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_FLOAT = {
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
};

static Eina_Bool
_eina_value_type_double_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   double *tmem = mem;
   *tmem = 0;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_flush(const Eina_Value_Type *type __UNUSED__, void *mem __UNUSED__)
{
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const double *s = src;
   double *d = dst;
   *d = *s;
   return EINA_TRUE;
}

static int
_eina_value_type_double_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const double *ta = a, *tb = b;
   if (*ta < *tb)
     return -1;
   else if (*ta > *tb)
     return 1;
   return 0;
}

static Eina_Bool
_eina_value_type_double_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const double v = *(const double *)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem = v;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   double *tmem = mem;
   *tmem = va_arg(args, double);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   double *tmem = mem;
   const double *p = ptr;
   *tmem = *p;
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_double_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   const double *tmem = mem;
   double *p = ptr;
   *p = *tmem;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_DOUBLE = {
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
};

static Eina_Bool
_eina_value_type_string_common_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   const char **tmem = mem;
   *tmem = NULL;
   return EINA_TRUE;
}

static int
_eina_value_type_string_common_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
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
_eina_value_type_string_common_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const char *v = *(const char **)type_mem;

   eina_error_set(0);

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
   else if (convert == EINA_VALUE_TYPE_ULONG)
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
        char other_mem;
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
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_common_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
{
   memcpy(ptr, mem, sizeof(const char *));
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_stringshare_flush(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   const char **tmem = mem;
   if (*tmem) eina_stringshare_del(*tmem);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_stringshare_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const char * const*s = src;
   const char **d = dst;
   *d = *s;
   eina_stringshare_ref(*d);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_stringshare_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   const char *str = va_arg(args, const char *);
   return eina_stringshare_replace((const char **)mem, str);
}

static Eina_Bool
_eina_value_type_stringshare_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   const char * const *str = ptr;
   return eina_stringshare_replace((const char **)mem, *str);
}

static const Eina_Value_Type _EINA_VALUE_TYPE_STRINGSHARE = {
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
};

static Eina_Bool
_eina_value_type_string_flush(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   char **tmem = mem;
   if (*tmem) free(*tmem);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
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
             eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
             return EINA_FALSE;
          }
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_vset(const Eina_Value_Type *type __UNUSED__, void *mem, va_list args)
{
   char **tmem = mem;
   const char *str = va_arg(args, const char *);
   free(*tmem);
   if (str == NULL)
     *tmem = NULL;
   else
     {
        *tmem = strdup(str);
        if (*tmem == NULL)
          {
             eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
             return EINA_FALSE;
          }
     }
   eina_error_set(0);
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_string_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   char **tmem = mem;
   const char * const *str = ptr;
   free(*tmem);
   if (*str == NULL)
     *tmem = NULL;
   else
     {
        *tmem = strdup(*str);
        if (*tmem == NULL)
          {
             eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
             return EINA_FALSE;
          }
     }
   eina_error_set(0);
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_STRING = {
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
};


static Eina_Bool
_eina_value_type_array_setup(const Eina_Value_Type *type __UNUSED__, void *mem)
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
_eina_value_type_array_flush(const Eina_Value_Type *type __UNUSED__, void *mem)
{
   Eina_Value_Array *tmem = mem;
   Eina_Bool ret =_eina_value_type_array_flush_elements(tmem);

   if (tmem->array) eina_inarray_free(tmem->array);
   tmem->array = NULL;
   tmem->subtype = NULL;
   return ret;
}

static Eina_Bool
_eina_value_type_array_copy(const Eina_Value_Type *type __UNUSED__, const void *src, void *dst)
{
   const Eina_Value_Type *subtype;
   const Eina_Value_Array *s = src;
   Eina_Value_Array *d = dst;
   unsigned int count, sz;
   char *placeholder, *ptr, *ptr_end;

   d->subtype = subtype = s->subtype;
   d->step = s->step;

   if ((!s->array) || (!s->subtype))
     {
        d->array = NULL;
        return EINA_TRUE;
     }

   if (!subtype->copy)
     return EINA_FALSE;

   d->array = eina_inarray_new(subtype->value_size, s->step);
   if (!d->array)
     return EINA_FALSE;

   sz = s->array->member_size;
   placeholder = alloca(sz);
   memset(placeholder, 0, sz);

   count = eina_inarray_count(s->array);
   ptr = s->array->members;
   ptr_end = ptr + (count * sz);

   for (; ptr < ptr_end; ptr += sz)
     {
        int i = eina_inarray_append(d->array, placeholder);
        void *imem = eina_inarray_nth(d->array, i);
        if ((i < 0) || (!imem)) goto error;
        if (!subtype->copy(subtype, ptr, imem)) goto error;
     }

   return EINA_TRUE;

 error:
   _eina_value_type_array_flush_elements(d);
   return EINA_FALSE;
}

static int
_eina_value_type_array_compare(const Eina_Value_Type *type __UNUSED__, const void *a, const void *b)
{
   const Eina_Value_Type *subtype;
   const Eina_Value_Array *eva_a = a, *eva_b = b;
   const char *a_ptr, *a_ptr_end, *b_ptr;
   unsigned int count_a, count_b, count, sz;
   int cmp = 0;

   if (eva_a->subtype != eva_b->subtype)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return -1;
     }

   subtype = eva_a->subtype;
   if (!subtype->compare)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
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
_eina_value_type_array_convert_to(const Eina_Value_Type *type __UNUSED__, const Eina_Value_Type *convert, const void *type_mem, void *convert_mem)
{
   const Eina_Value_Array *tmem = type_mem;
   Eina_Bool ret = EINA_FALSE;

   if ((tmem->array) && (tmem->array->len == 1))
     {
        const Eina_Value_Type *subtype = tmem->subtype;
        void *imem = tmem->array->members;

        if (subtype->convert_to)
          ret = subtype->convert_to(subtype, convert, imem, convert_mem);
        if ((!ret) && (convert->convert_from))
          ret = convert->convert_from(convert, subtype, convert_mem, imem);
     }
   else if ((convert == EINA_VALUE_TYPE_STRING) ||
            (convert == EINA_VALUE_TYPE_STRINGSHARE))
     {
        Eina_Strbuf *str = eina_strbuf_new();
        if (!tmem->array) eina_strbuf_append(str, "[]");
        else
          {
             const Eina_Value_Type *subtype = tmem->subtype;
             unsigned char sz;
             const char *ptr, *ptr_end;
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
             ptr = eina_strbuf_string_get(str);
             ret = eina_value_type_pset(convert, convert_mem, &ptr);
             eina_strbuf_free(str);
          }
     }

   if (!ret)
     {
        eina_error_set(EINA_ERROR_VALUE_FAILED);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_eina_value_type_array_convert_from(const Eina_Value_Type *type, const Eina_Value_Type *convert, void *type_mem, const void *convert_mem)
{
   Eina_Value_Array *tmem = type_mem;
   Eina_Value_Array desc = {convert, tmem->step, NULL};
   char *buf, *placeholder;
   void *imem;

   if (!eina_value_type_pset(type, tmem, &desc))
     return EINA_FALSE;

   buf = alloca(convert->value_size);
   if (!eina_value_type_pget(convert, convert_mem, &buf))
     return EINA_FALSE;

   placeholder = alloca(convert->value_size);
   memset(placeholder, 0, convert->value_size);

   if (eina_inarray_append(tmem->array, placeholder) != 0)
     return EINA_FALSE;
   imem = eina_inarray_nth(tmem->array, 0);
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
_eina_value_type_array_pset(const Eina_Value_Type *type __UNUSED__, void *mem, const void *ptr)
{
   Eina_Value_Array *tmem = mem;
   const Eina_Value_Array *desc = ptr;

   if ((!tmem->subtype) && (!desc->subtype))
     return EINA_TRUE;

   if (tmem->array)
     {
        _eina_value_type_array_flush_elements(tmem);
        eina_inarray_setup(tmem->array, desc->subtype->value_size, desc->step);
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
_eina_value_type_array_pget(const Eina_Value_Type *type __UNUSED__, const void *mem, void *ptr)
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


/**
 * @endcond
 */

static const char EINA_ERROR_VALUE_FAILED_STR[] = "Value check failed.";

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
   _eina_value_log_dom = eina_log_domain_register("eina_value",
                                                  EINA_LOG_COLOR_DEFAULT);
   if (_eina_value_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_value");
        return EINA_FALSE;
     }

   EINA_ERROR_VALUE_FAILED = eina_error_msg_static_register(
         EINA_ERROR_VALUE_FAILED_STR);

   EINA_VALUE_TYPE_UCHAR = &_EINA_VALUE_TYPE_UCHAR;
   EINA_VALUE_TYPE_USHORT = &_EINA_VALUE_TYPE_USHORT;
   EINA_VALUE_TYPE_UINT = &_EINA_VALUE_TYPE_UINT;
   EINA_VALUE_TYPE_ULONG = &_EINA_VALUE_TYPE_ULONG;
   EINA_VALUE_TYPE_UINT64 = &_EINA_VALUE_TYPE_UINT64;
   EINA_VALUE_TYPE_CHAR = &_EINA_VALUE_TYPE_CHAR;
   EINA_VALUE_TYPE_SHORT = &_EINA_VALUE_TYPE_SHORT;
   EINA_VALUE_TYPE_INT = &_EINA_VALUE_TYPE_INT;
   EINA_VALUE_TYPE_LONG = &_EINA_VALUE_TYPE_LONG;
   EINA_VALUE_TYPE_INT64 = &_EINA_VALUE_TYPE_INT64;
   EINA_VALUE_TYPE_FLOAT = &_EINA_VALUE_TYPE_FLOAT;
   EINA_VALUE_TYPE_DOUBLE = &_EINA_VALUE_TYPE_DOUBLE;
   EINA_VALUE_TYPE_STRINGSHARE = &_EINA_VALUE_TYPE_STRINGSHARE;
   EINA_VALUE_TYPE_STRING = &_EINA_VALUE_TYPE_STRING;
   EINA_VALUE_TYPE_ARRAY = &_EINA_VALUE_TYPE_ARRAY;

   return EINA_TRUE;
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

EAPI extern const Eina_Value_Type *EINA_VALUE_TYPE_UCHAR;

EAPI const Eina_Value_Type *EINA_VALUE_TYPE_UCHAR = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_USHORT = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_UINT = NULL;
EAPI const Eina_Value_Type *EINA_VALUE_TYPE_ULONG = NULL;
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
   Eina_Value *value = malloc(sizeof(Eina_Value));
   if (!value)
     {
        eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return NULL;
     }
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
   EINA_SAFETY_ON_NULL_RETURN(value);
   eina_value_flush(value);
   free(value);
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

   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_type_check(subtype), EINA_FALSE);

   value = calloc(1, sizeof(Eina_Value));
   if (!value)
     return NULL;

   if (!eina_value_array_setup(value, subtype, step))
     {
        free(value);
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
