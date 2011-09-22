/* EINA - EFL data type library
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2010
 *			   Carsten Haitzler,
 *                         Jorge Luis Zapata Muga,
 *                         Cedric Bail,
 *                         Gustavo Sverzut Barbieri
 *                         Tom Hacohen
 *                         Brett Nash
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_stringshare.h"
#include "eina_lock.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_share_common.h"

/* The actual share */
static Eina_Share *stringshare_share;
static const char EINA_MAGIC_STRINGSHARE_NODE_STR[] = "Eina Stringshare Node";

extern Eina_Bool _share_common_threads_activated;
static Eina_Lock _mutex_small;

/* Stringshare optimizations */
static const unsigned char _eina_stringshare_single[512] = {
   0,0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10,0,11,0,12,0,13,0,14,0,15,0,
   16,0,17,0,18,0,19,0,20,0,21,0,22,0,23,0,24,0,25,0,26,0,27,0,28,0,29,0,30,0,
   31,0,32,0,33,0,34,0,35,0,36,0,37,0,38,0,39,0,40,0,41,0,42,0,43,0,44,0,45,0,
   46,0,47,0,48,0,49,0,50,0,51,0,52,0,53,0,54,0,55,0,56,0,57,0,58,0,59,0,60,0,
   61,0,62,0,63,0,64,0,65,0,66,0,67,0,68,0,69,0,70,0,71,0,72,0,73,0,74,0,75,0,
   76,0,77,0,78,0,79,0,80,0,81,0,82,0,83,0,84,0,85,0,86,0,87,0,88,0,89,0,90,0,
   91,0,92,0,93,0,94,0,95,0,96,0,97,0,98,0,99,0,100,0,101,0,102,0,103,0,104,0,
   105,0,
   106,0,107,0,108,0,109,0,110,0,111,0,112,0,113,0,114,0,115,0,116,0,117,0,118,
   0,119,0,120,0,
   121,0,122,0,123,0,124,0,125,0,126,0,127,0,128,0,129,0,130,0,131,0,132,0,133,
   0,134,0,135,0,
   136,0,137,0,138,0,139,0,140,0,141,0,142,0,143,0,144,0,145,0,146,0,147,0,148,
   0,149,0,150,0,
   151,0,152,0,153,0,154,0,155,0,156,0,157,0,158,0,159,0,160,0,161,0,162,0,163,
   0,164,0,165,0,
   166,0,167,0,168,0,169,0,170,0,171,0,172,0,173,0,174,0,175,0,176,0,177,0,178,
   0,179,0,180,0,
   181,0,182,0,183,0,184,0,185,0,186,0,187,0,188,0,189,0,190,0,191,0,192,0,193,
   0,194,0,195,0,
   196,0,197,0,198,0,199,0,200,0,201,0,202,0,203,0,204,0,205,0,206,0,207,0,208,
   0,209,0,210,0,
   211,0,212,0,213,0,214,0,215,0,216,0,217,0,218,0,219,0,220,0,221,0,222,0,223,
   0,224,0,225,0,
   226,0,227,0,228,0,229,0,230,0,231,0,232,0,233,0,234,0,235,0,236,0,237,0,238,
   0,239,0,240,0,
   241,0,242,0,243,0,244,0,245,0,246,0,247,0,248,0,249,0,250,0,251,0,252,0,253,
   0,254,0,255,0
};

typedef struct _Eina_Stringshare_Small Eina_Stringshare_Small;
typedef struct _Eina_Stringshare_Small_Bucket Eina_Stringshare_Small_Bucket;

struct _Eina_Stringshare_Small_Bucket
{
   /* separate arrays for faster lookups */
   const char **strings;
   unsigned char *lengths;
   unsigned short *references;
   int count;
   int size;
};

struct _Eina_Stringshare_Small
{
   Eina_Stringshare_Small_Bucket *buckets[256];
};

#define EINA_STRINGSHARE_SMALL_BUCKET_STEP 8
static Eina_Stringshare_Small _eina_small_share;

static inline int
_eina_stringshare_small_cmp(const Eina_Stringshare_Small_Bucket *bucket,
                            int i,
                            const char *pstr,
                            unsigned char plength)
{
   /* pstr and plength are from second char and on, since the first is
    * always the same.
    *
    * First string being always the same, size being between 2 and 3
    * characters (there is a check for special case length==1 and then
    * small stringshare is applied to strings < 4), we just need to
    * compare 2 characters of both strings.
    */
   const unsigned char cur_plength = bucket->lengths[i] - 1;
   const char *cur_pstr;

   if (cur_plength > plength)
      return 1;
   else if (cur_plength < plength)
      return -1;

   cur_pstr = bucket->strings[i] + 1;

   if (cur_pstr[0] > pstr[0])
      return 1;
   else if (cur_pstr[0] < pstr[0])
      return -1;

   if (plength == 1)
      return 0;

   if (cur_pstr[1] > pstr[1])
      return 1;
   else if (cur_pstr[1] < pstr[1])
      return -1;

   return 0;
}

static const char *
_eina_stringshare_small_bucket_find(const Eina_Stringshare_Small_Bucket *bucket,
                                    const char *str,
                                    unsigned char length,
                                    int *idx)
{
   const char *pstr = str + 1; /* skip first letter, it's always the same */
   unsigned char plength = length - 1;
   int i, low, high;

   if (bucket->count == 0)
     {
        *idx = 0;
        return NULL;
     }

   low = 0;
   high = bucket->count;

   while (low < high)
     {
        int r;

        i = (low + high - 1) / 2;

        r = _eina_stringshare_small_cmp(bucket, i, pstr, plength);
        if (r > 0)
           high = i;
        else if (r < 0)
           low = i + 1;
        else
          {
             *idx = i;
             return bucket->strings[i];
          }
     }

   *idx = low;
   return NULL;
}

static Eina_Bool
_eina_stringshare_small_bucket_resize(Eina_Stringshare_Small_Bucket *bucket,
                                      int size)
{
   void *tmp;

   tmp = realloc((void *)bucket->strings, size * sizeof(bucket->strings[0]));
   if (!tmp)
     {
             eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return 0;
     }

   bucket->strings = tmp;

   tmp = realloc(bucket->lengths, size * sizeof(bucket->lengths[0]));
   if (!tmp)
     {
             eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return 0;
     }

   bucket->lengths = tmp;

   tmp = realloc(bucket->references, size * sizeof(bucket->references[0]));
   if (!tmp)
     {
             eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return 0;
     }

   bucket->references = tmp;

   bucket->size = size;
   return 1;
}

static const char *
_eina_stringshare_small_bucket_insert_at(
   Eina_Stringshare_Small_Bucket **p_bucket,
   const char *str,
   unsigned char length,
   int idx)
{
   Eina_Stringshare_Small_Bucket *bucket = *p_bucket;
   int todo, off;
   char *snew;

   if (!bucket)
     {
        *p_bucket = bucket = calloc(1, sizeof(*bucket));
        if (!bucket)
          {
             eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
             return NULL;
          }
     }

   if (bucket->count + 1 >= bucket->size)
     {
        int size = bucket->size + EINA_STRINGSHARE_SMALL_BUCKET_STEP;
        if (!_eina_stringshare_small_bucket_resize(bucket, size))
           return NULL;
     }

   snew = malloc(length + 1);
   if (!snew)
     {
             eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return NULL;
     }

   memcpy(snew, str, length);
   snew[length] = '\0';

   off = idx + 1;
   todo = bucket->count - idx;
   if (todo > 0)
     {
        memmove((void *)(bucket->strings + off), bucket->strings + idx,
                todo * sizeof(bucket->strings[0]));
        memmove(bucket->lengths + off,           bucket->lengths + idx,
                todo * sizeof(bucket->lengths[0]));
        memmove(bucket->references + off,        bucket->references + idx,
                todo * sizeof(bucket->references[0]));
     }

   bucket->strings[idx] = snew;
   bucket->lengths[idx] = length;
   bucket->references[idx] = 1;
   bucket->count++;

   return snew;
}

static void
_eina_stringshare_small_bucket_remove_at(
   Eina_Stringshare_Small_Bucket **p_bucket,
   int idx)
{
   Eina_Stringshare_Small_Bucket *bucket = *p_bucket;
   int todo, off;

   if (bucket->references[idx] > 1)
     {
        bucket->references[idx]--;
        return;
     }

        free((char *)bucket->strings[idx]);

   if (bucket->count == 1)
     {
        free((void *)bucket->strings);
        free(bucket->lengths);
        free(bucket->references);
        free(bucket);
        *p_bucket = NULL;
        return;
     }

   bucket->count--;
   if (idx == bucket->count)
      goto end;

   off = idx + 1;
   todo = bucket->count - idx;

   memmove((void *)(bucket->strings + idx), bucket->strings + off,
           todo * sizeof(bucket->strings[0]));
   memmove(bucket->lengths + idx,           bucket->lengths + off,
           todo * sizeof(bucket->lengths[0]));
   memmove(bucket->references + idx,        bucket->references + off,
           todo * sizeof(bucket->references[0]));

end:
   if (bucket->count + EINA_STRINGSHARE_SMALL_BUCKET_STEP < bucket->size)
     {
        int size = bucket->size - EINA_STRINGSHARE_SMALL_BUCKET_STEP;
        _eina_stringshare_small_bucket_resize(bucket, size);
     }
}

static const char *
_eina_stringshare_small_add(const char *str, unsigned char length)
{
   Eina_Stringshare_Small_Bucket **bucket;
   int i;

   bucket = _eina_small_share.buckets + (unsigned char)str[0];
   if (!*bucket)
      i = 0;
   else
     {
        const char *ret;
        ret = _eina_stringshare_small_bucket_find(*bucket, str, length, &i);
        if (ret)
          {
             (*bucket)->references[i]++;
             return ret;
          }
     }

   return _eina_stringshare_small_bucket_insert_at(bucket, str, length, i);
}

static void
_eina_stringshare_small_del(const char *str, unsigned char length)
{
   Eina_Stringshare_Small_Bucket **bucket;
   const char *ret;
   int i;

   bucket = _eina_small_share.buckets + (unsigned char)str[0];
   if (!*bucket)
      goto error;

   ret = _eina_stringshare_small_bucket_find(*bucket, str, length, &i);
   if (!ret)
      goto error;

   _eina_stringshare_small_bucket_remove_at(bucket, i);
   return;

error:
   CRITICAL("EEEK trying to del non-shared stringshare \"%s\"", str);
}

static void
_eina_stringshare_small_init(void)
{
   eina_lock_new(&_mutex_small);
   memset(&_eina_small_share, 0, sizeof(_eina_small_share));
}

static void
_eina_stringshare_small_shutdown(void)
{
   Eina_Stringshare_Small_Bucket **p_bucket, **p_bucket_end;

   p_bucket = _eina_small_share.buckets;
   p_bucket_end = p_bucket + 256;

   for (; p_bucket < p_bucket_end; p_bucket++)
     {
        Eina_Stringshare_Small_Bucket *bucket = *p_bucket;
        char **s, **s_end;

        if (!bucket)
           continue;

        s = (char **)bucket->strings;
        s_end = s + bucket->count;
        for (; s < s_end; s++)
           free(*s);

           free((void *)bucket->strings);
           free(bucket->lengths);
           free(bucket->references);
           free(bucket);
        *p_bucket = NULL;
     }

   eina_lock_free(&_mutex_small);
}

static void
_eina_stringshare_small_bucket_dump(Eina_Stringshare_Small_Bucket *bucket,
                                    struct dumpinfo *di)
{
   const char **s = bucket->strings;
   unsigned char *l = bucket->lengths;
   unsigned short *r = bucket->references;
   int i;

   di->used += sizeof(*bucket);
   di->used += bucket->count * sizeof(*s);
   di->used += bucket->count * sizeof(*l);
   di->used += bucket->count * sizeof(*r);
   di->unique += bucket->count;

   for (i = 0; i < bucket->count; i++, s++, l++, r++)
     {
        int dups;
#ifdef _WIN32
        printf("DDD: %5hu %5hu '%s'\n",  *l, *r, *s);
#else
        printf("DDD: %5hhu %5hu '%s'\n", *l, *r, *s);
#endif

        dups = (*r - 1);

        di->used += *l;
        di->saved += *l * dups;
        di->dups += dups;
     }
}

static void
_eina_stringshare_small_dump(struct dumpinfo *di)
{
   Eina_Stringshare_Small_Bucket **p_bucket, **p_bucket_end;

   p_bucket = _eina_small_share.buckets;
   p_bucket_end = p_bucket + 256;

   for (; p_bucket < p_bucket_end; p_bucket++)
     {
        Eina_Stringshare_Small_Bucket *bucket = *p_bucket;

        if (!bucket)
           continue;

        _eina_stringshare_small_bucket_dump(bucket, di);
     }
}


/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @internal
 * @brief Initialize the share_common module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the share_common module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_stringshare_init(void)
{
   Eina_Bool ret;
   ret = eina_share_common_init(&stringshare_share,
                                EINA_MAGIC_STRINGSHARE_NODE,
                                EINA_MAGIC_STRINGSHARE_NODE_STR);
   if (ret)
      _eina_stringshare_small_init();

   return ret;
}

/**
 * @internal
 * @brief Shut down the share_common module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the share_common module set up by
 * eina_share_common_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_stringshare_shutdown(void)
{
   Eina_Bool ret;
   _eina_stringshare_small_shutdown();
   ret = eina_share_common_shutdown(&stringshare_share);
   return ret;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EAPI void
eina_stringshare_del(const char *str)
{
   int slen;

   if (!str)
      return;

   /* special cases */
   if      (str[0] == '\0')
      slen = 0;
   else if (str[1] == '\0')
      slen = 1;
   else if (str[2] == '\0')
      slen = 2;
   else if (str[3] == '\0')
      slen = 3;
   else
      slen = 4;  /* handled later */

   if (slen < 2)
      return;
   else if (slen < 4)
     {
        eina_share_common_population_del(stringshare_share, slen);
        eina_lock_take(&_mutex_small);
        _eina_stringshare_small_del(str, slen);
        eina_lock_release(&_mutex_small);
        return;
     }

   eina_share_common_del(stringshare_share, str);
}

EAPI const char *
eina_stringshare_add_length(const char *str, unsigned int slen)
{
   if ((!str) || (slen <= 0))
      return "";
   else if (slen == 1)
      return (const char *)_eina_stringshare_single + ((*str) << 1);
   else if (slen < 4)
     {
        const char *s;

        eina_lock_take(&_mutex_small);
        s = _eina_stringshare_small_add(str, slen);
        eina_lock_release(&_mutex_small);
        return s;
     }

   return eina_share_common_add_length(stringshare_share, str, slen *
                                       sizeof(char), sizeof(char));
}

EAPI const char *
eina_stringshare_add(const char *str)
{
   int slen;
   if (!str)
      return NULL;

   if      (str[0] == '\0')
      slen = 0;
   else if (str[1] == '\0')
      slen = 1;
   else if (str[2] == '\0')
      slen = 2;
   else if (str[3] == '\0')
      slen = 3;
   else
      slen = 3 + (int)strlen(str + 3);

   return eina_stringshare_add_length(str, slen);
}

EAPI const char *
eina_stringshare_printf(const char *fmt, ...)
{
   va_list args;
   char *tmp;
   const char *ret;
   int len;

   if (!fmt)
      return NULL;

   va_start(args, fmt);
   len = vasprintf(&tmp, fmt, args);
   va_end(args);

   if (len < 1)
      return NULL;

   ret = eina_stringshare_add_length(tmp, len);
   free(tmp);

   return ret;
}

EAPI const char *
eina_stringshare_vprintf(const char *fmt, va_list args)
{
   char *tmp;
   const char *ret;
   int len;

   if (!fmt)
      return NULL;

   len = vasprintf(&tmp, fmt, args);

   if (len < 1)
      return NULL;

   ret = eina_stringshare_add_length(tmp, len);
   free(tmp);

   return ret;
}

EAPI const char *
eina_stringshare_nprintf(unsigned int len, const char *fmt, ...)
{
   va_list args;
   char *tmp;
   int size;

   if (!fmt)
      return NULL;

   if (len < 1)
      return NULL;

   tmp = alloca(sizeof(char) * len + 1);

   va_start(args, fmt);
   size = vsnprintf(tmp, len, fmt, args);
   va_end(args);

   if (size < 1)
      return NULL;

   return eina_stringshare_add_length(tmp, len);
}

EAPI const char *
eina_stringshare_ref(const char *str)
{
   int slen;

   if (!str)
      return eina_share_common_ref(stringshare_share, str);

   /* special cases */
   if      (str[0] == '\0')
      slen = 0;
   else if (str[1] == '\0')
      slen = 1;
   else if (str[2] == '\0')
      slen = 2;
   else if (str[3] == '\0')
      slen = 3;
   else
      slen = 3 + (int)strlen(str + 3);

   if (slen < 2)
     {
        eina_share_common_population_add(stringshare_share, slen);

        return str;
     }
   else if (slen < 4)
     {
        const char *s;
        eina_share_common_population_add(stringshare_share, slen);

        eina_lock_take(&_mutex_small);
        s = _eina_stringshare_small_add(str, slen);
        eina_lock_release(&_mutex_small);

        return s;
     }

   return eina_share_common_ref(stringshare_share, str);
}

EAPI int
eina_stringshare_strlen(const char *str)
{
   int len;
   /* special cases */
   if (str[0] == '\0')
      return 0;

   if (str[1] == '\0')
      return 1;

   if (str[2] == '\0')
      return 2;

   if (str[3] == '\0')
      return 3;

   len = eina_share_common_length(stringshare_share, (const char *)str);
   len = (len > 0) ? len / (int)sizeof(char) : -1;
   return len;
}

EAPI void
eina_stringshare_dump(void)
{
   eina_share_common_dump(stringshare_share,
                          _eina_stringshare_small_dump,
                          sizeof(_eina_stringshare_single));
}
