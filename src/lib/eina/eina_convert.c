/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric BAIL, Vincent Torri
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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_convert.h"
#include "eina_fp.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

static const char look_up_table[] = {'0', '1', '2', '3', '4',
                                     '5', '6', '7', '8', '9',
                                     'a', 'b', 'c', 'd', 'e', 'f'};
static int _eina_convert_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_convert_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_convert_log_dom, __VA_ARGS__)

#define HEXA_TO_INT(Hexa) (Hexa >= 'a') ? Hexa - 'a' + 10 : Hexa - '0'

static inline void reverse(char s[], int length)
{
   int i, j;
   char c;

   for (i = 0, j = length - 1; i < j; i++, j--)
     {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
     }
}

/**
 * @endcond
 */

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

EAPI Eina_Error EINA_ERROR_CONVERT_P_NOT_FOUND = 0;
EAPI Eina_Error EINA_ERROR_CONVERT_0X_NOT_FOUND = 0;
EAPI Eina_Error EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH = 0;

/**
 * @endcond
 */

/**
 * @internal
 * @brief Initialize the convert module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the convert module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_convert_init(void)
{
   _eina_convert_log_dom = eina_log_domain_register("eina_convert",
                                                    EINA_LOG_COLOR_DEFAULT);
   if (_eina_convert_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_convert");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the convert module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the convert module set up by
 * eina_convert_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_convert_shutdown(void)
{
   eina_log_domain_unregister(_eina_convert_log_dom);
   _eina_convert_log_dom = -1;
   return EINA_TRUE;
}

/*============================================================================*
*                                   API                                      *
*============================================================================*/

/*
 * Come from the second edition of The C Programming Language ("K&R2") on page 64
 */

EAPI int
eina_convert_itoa(int n, char *s)
{
   int i = 0;
   int r = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(s, 0);

   if (n < 0)
     {
        *s++ = '-';
        r = 1;
     }

   do {
        s[i++] = abs(n % 10) + '0';
   } while (n /= 10);

   s[i] = '\0';

   reverse(s, i);

   return i + r;
}

EAPI int
eina_convert_xtoa(unsigned int n, char *s)
{
   int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(s, 0);

   i = 0;
   do {
        s[i++] = look_up_table[n & 0xF];
   } while ((n >>= 4) > 0);

   s[i] = '\0';

   reverse(s, i);

   return i;
}

EAPI Eina_Bool
eina_convert_atod(const char *src, int length, long long *m, long *e)
{
   const char *str = src;
   long long mantisse;
   long exponent;
   int nbr_decimals = 0;
   int sign = 1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(src, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(m,   EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(e,   EINA_FALSE);

   if (length <= 0)
     goto on_length_error;

   /* Compute the mantisse. */
   if (*str == '-')
     {
        sign = -1;
        str++;
        length--;
     }

   if (length <= 2)
     goto on_length_error;

   if (strncmp(str, "0x", 2))
     {
        DBG("'0x' not found in '%s'", src);
        return EINA_FALSE;
     }

   str += 2;
   length -= 2;

   mantisse = HEXA_TO_INT(*str);

   str++;
   length--; if (length <= 0)
      goto on_length_error;

   if (*str == '.')
     for (str++, length--;
          length > 0 && *str != 'p';
          ++str, --length, ++nbr_decimals)
       {
          mantisse <<= 4;
          mantisse += HEXA_TO_INT(*str);
       }

   if (sign < 0)
     mantisse = -mantisse;

   /* Compute the exponent. */
   if (*str != 'p')
     {
        DBG("'p' not found in '%s'", src);
        return EINA_FALSE;
     }

   sign = +1;

   str++;
   length--; if (length <= 0)
      goto on_length_error;

   if (strchr("-+", *str))
     {
        sign = (*str == '-') ? -1 : +1;

        str++; length--;
     }

   for (exponent = 0; length > 0 && *str != '\0'; ++str, --length)
     {
        exponent *= 10;
        exponent += *str - '0';
     }

   if (length < 0)
     goto on_length_error;

   if (sign < 0)
     exponent = -exponent;

   *m = mantisse;
   *e = exponent - (nbr_decimals << 2);

   return EINA_TRUE;

on_length_error:
   return EINA_FALSE;
}

EAPI int
eina_convert_dtoa(double d, char *des)
{
   int length = 0;
   int p;
   int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(des, EINA_FALSE);

   if (d < 0.0)
     {
        *(des++) = '-';
        d = -d;
        length++;
     }

   d = frexp(d, &p);

   if (p)
     {
        d *= 2;
        p -= 1;
     }

   *(des++) = '0';
   *(des++) = 'x';
   *(des++) = look_up_table[(size_t)d];
   *(des++) = '.';
   length += 4;

   for (i = 0; i < 16; i++, length++)
     {
        d -= floor(d);
        d *= 16;
        *(des++) = look_up_table[(size_t)d];
     }

   while (*(des - 1) == '0')
     {
        des--;
        length--;
     }

   if (*(des - 1) == '.')
     {
        des--;
        length--;
     }

   *(des++) = 'p';
   if (p < 0)
     {
        *(des++) = '-';
        p = -p;
     }
   else
     *(des++) = '+';

   length += 2;

   return length + eina_convert_itoa(p, des);
}

EAPI int
eina_convert_fptoa(Eina_F32p32 fp, char *des)
{
   int length = 0;
   int p = 0;
   int i;

   EINA_SAFETY_ON_NULL_RETURN_VAL(des, EINA_FALSE);

   if (fp == 0)
     {
        memcpy(des, "0x0p+0", 7);
        return 7;
     }

   if (fp < 0)
     {
        *(des++) = '-';
        fp = -fp;
        length++;
     }

   /* fp >= 1 */
   if (fp >= 0x0000000100000000LL)
     while (fp >= 0x0000000100000000LL)
       {
          p++;
          /* fp /= 2 */
          fp >>= 1;
       } /* fp < 0.5 */
   else if (fp < 0x80000000)
     while (fp < 0x80000000)
       {
          p--;
          /* fp *= 2 */
          fp <<= 1;
       }

   if (p)
     {
        p--;
        /* fp *= 2 */
        fp <<= 1;
     }

   *(des++) = '0';
   *(des++) = 'x';
   *(des++) = look_up_table[fp >> 32];
   *(des++) = '.';
   length += 4;

   for (i = 0; i < 16; i++, length++)
     {
        fp &= 0x00000000ffffffffLL;
        fp <<= 4; /* fp *= 16 */
        *(des++) = look_up_table[fp >> 32];
     }

   while (*(des - 1) == '0')
     {
        des--;
        length--;
     }

   if (*(des - 1) == '.')
     {
        des--;
        length--;
     }

   *(des++) = 'p';
   if (p < 0)
     {
        *(des++) = '-';
        p = -p;
     }
   else
     *(des++) = '+';

   length += 2;

   return length + eina_convert_itoa(p, des);
}

EAPI Eina_Bool
eina_convert_atofp(const char *src, int length, Eina_F32p32 *fp)
{
   long long m;
   long e;

   if (!eina_convert_atod(src, length, &m, &e))
     return EINA_FALSE;

   if (!fp)
     return EINA_FALSE;

   e += 32;

   if (e > 0)
     *fp = m << e;
   else
     *fp = m >> -e;

   return EINA_TRUE;
}

/**
 * @}
 */
