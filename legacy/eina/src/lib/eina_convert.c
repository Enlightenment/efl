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

#include "eina_convert.h"

static const char look_up_table[] = {'0', '1', '2', '3', '4',
				     '5', '6', '7', '8', '9',
				     'a', 'b', 'c', 'd', 'e', 'f'};
static int _init_count = 0;

EAPI Eina_Error EINA_ERROR_CONVERT_P_NOT_FOUND = 0;
EAPI Eina_Error EINA_ERROR_CONVERT_0X_NOT_FOUND = 0;
EAPI Eina_Error EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH = 0;

#define HEXA_TO_INT(Hexa) (Hexa >= 'a') ? Hexa - 'a' + 10 : Hexa - '0'

static inline void reverse(char s[], int length)
{
   int c, i, j;

   for (i = 0, j = length - 1; i < j; i++, j--)
     {
	c = s[i];
	s[i] = s[j];
	s[j] = c;
     }
}

/*
 * Come from the second edition of The C Programming Language ("K&R2") on page 64
 */
EAPI int
eina_convert_itoa(int n, char *s)
{
   int i = 0;
   int r = 0;

   if (n < 0)
     {
	n = -n;
	*s++ = '-';
	r = 1;
     }

   do {
      s[i++] = n % 10 + '0';
   } while ((n /= 10) > 0);

   s[i] = '\0';

   reverse(s, i);

   return i + r;
}

EAPI int
eina_convert_xtoa(unsigned int n, char *s)
{
   int i;

   i = 0;
   do {
      s[i++] = look_up_table[n & 0xF];
   } while ((n >>= 4) > 0);

   s[i] = '\0';

   reverse(s, i);

   return i;
}

/* On Windows (using MinGW or VC++), printf-like functions */
/* rely on MSVCRT, which does not fully support the C99    */
/* specifications. In particular, they do not support the  */
/* modifier character %a.                                  */
EAPI int
eina_convert_init(void)
{
   _init_count++;

   if (_init_count > 1) goto init_out;

   eina_error_init();
   EINA_ERROR_CONVERT_0X_NOT_FOUND = eina_error_register("Error during string convertion to float, First '0x' was not found.");
   EINA_ERROR_CONVERT_P_NOT_FOUND = eina_error_register("Error during string convertion to float, First 'p' was not found.");
   EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH = eina_error_register("Error outrun string limit during convertion string convertion to float.");

 init_out:
   return _init_count;
}

EAPI int
eina_convert_shutdown(void)
{
   _init_count--;

   if (_init_count > 0) goto shutdown_out;

   eina_error_shutdown();

 shutdown_out:
   return _init_count;
}

/* That function converts a string created by a valid %a   */
/* modifier to a double.                                   */
/*                                                         */
/* The string must have the following format:              */
/*                                                         */
/*  [-]0xh.hhhhhp[+-]e                                     */
/*                                                         */
/* where e is a decimal number.                            */
/* If n is the number of cyphers after the point, the      */
/* returned mantisse and exponents are                     */
/*                                                         */
/*  mantisse: [-]hhhhhh                                    */
/*  exponent: 2^([+-]e - 4 * n)                            */
EAPI Eina_Bool
eina_convert_atod(const char *src, int length, long long *m, long *e)
{
   const char *str = src;
   long long   mantisse;
   long        exponent;
   int         nbr_decimals = 0;
   int         sign = 1;

   if (length <= 0) goto on_length_error;

   /* Compute the mantisse. */
   if (*str == '-')
     {
        sign = -1;
        str++;
	length--;
     }

   if (length <= 2) goto on_length_error;

   if (strncmp(str, "0x", 2))
     {
	eina_error_set(EINA_ERROR_CONVERT_0X_NOT_FOUND);
	EINA_ERROR_PDBG("'0x' not found in '%s'\n", src);
        return EINA_FALSE;
     }

   str += 2;
   length -= 2;

   mantisse = HEXA_TO_INT(*str);

   str++;
   length--; if (length <= 0) goto on_length_error;

   if (*str == '.')
     {
	for (str++, length--;
	     length > 0 && *str != 'p';
	     ++str, --length, ++nbr_decimals)
          {
             mantisse <<= 4;
             mantisse += HEXA_TO_INT(*str);
          }
     }
   if (sign < 0) mantisse = -mantisse;

   /* Compute the exponent. */
   if (*str != 'p')
     {
	eina_error_set(EINA_ERROR_CONVERT_P_NOT_FOUND);
	EINA_ERROR_PDBG("'p' not found in '%s'\n", src);
        return EINA_FALSE;
     }
   sign = +1;

   str++;
   length--; if (length <= 0) goto on_length_error;

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

   if (length <= 0) goto on_length_error;

   if (sign < 0)
     exponent = -exponent;

   *m = mantisse;
   *e = exponent - (nbr_decimals << 2);

   return EINA_TRUE;

 on_length_error:
   eina_error_set(EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH);
   return EINA_FALSE;
}

/* That function converts a double to a string that as the */
/* following format:                                       */
/*                                                         */
/*  [-]0xh.hhhhhp[+-]e                                     */
/*                                                         */
/* where h is a hexadecimal number and e a decimal number. */
EAPI int
eina_convert_dtoa(double d, char *des)
{
   int length = 0;
   int p;
   int i;

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
