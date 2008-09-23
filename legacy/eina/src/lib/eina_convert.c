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

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static const char look_up_table[] = {'0', '1', '2', '3', '4',
				     '5', '6', '7', '8', '9',
				     'a', 'b', 'c', 'd', 'e', 'f'};
static int _eina_convert_init_count = 0;

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

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
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
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @addtogroup Eina_Convert_Group Convert
 *
 * These functions allow you to convert integer or real numbers to
 * string or conversely.
 *
 * To use these function, you have to call eina_convert_init()
 * first, and eina_convert_shutdown() when they are not used anymore.
 *
 * @section Eina_Convert_From_Integer_To_Sring Conversion from integer to string
 *
 * To convert an integer to a string in the decimal base,
 * eina_convert_itoa() should be used. If the hexadecimal base is
 * wanted, eina_convert_xtoa() should be used. They all need a bufffer
 * sufficiently large to store all the cyphers.
 *
 * Here is an exemple of use:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_convert.h>
 *
 * int main(void)
 * {
 *    char *tmp[128];
 *
 *    if (!eina_convert_init())
 *    {
 *        printf ("Error during the initialization of eina_convert module\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    eina_convert_itoa(45, tmp);
 *    printf("value: %s\n", tmp);

 *    eina_convert_xtoa(0xA1, tmp);
 *    printf("value: %s\n", tmp);
 *
 *    eina_convert_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * Compile this code with the following commant:
 *
 * @code
 * gcc -Wall -o test_eina_convert test_eina.c `pkg-config --cflags --libs eina`
 * @endcode
 *
 * @note
 * The alphabetical cyphers are in lower case.
 *
 * @section Eina_Convert_Double Conversion double / string
 *
 * To convert a double to a string, eina_convert_dtoa() should be
 * used. Like with the integer functions, a buffer must be used. The
 * resulting string ghas the following format (which is the result
 * obtained with snprintf() and the @%a modifier):
 *
 * @code
 * [-]0xh.hhhhhp[+-]e
 * @endcode
 *
 * To convert a string to a double, eina_convert_atod() should be
 * used. The format of the string must be as above. Then, the double
 * has the following mantiss and exponent:
 *
 * @code
 * mantiss  : [-]hhhhhh
 * exponent : 2^([+-]e - 4 * n)
 * @endcode
 *
 * with n being number of cypers after the point in the string
 * format. To obtain the double number from the mantiss and exponent,
 * use ldexp().
 *
 * Here is an exemple of use:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <eina_convert.h>
 *
 * int main(void)
 * {
 *    char     *tmp[128];
 *    long long int m = 0;
 *    long int  e = 0;
 *    doule     r;
 *
 *    if (!eina_convert_init())
 *    {
 *        printf ("Error during the initialization of eina_convert module\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    eina_convert_dtoa(40.56, tmp);
 *    printf("value: %s\n", tmp);

 *    eina_convert_atod(tmp, 128, &m, &e);
 *    r = ldexp((double)m, e);
 *    printf("value: %s\n", tmp);
 *
 *    eina_convert_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * Compile this code with the same command as above.
 *
 * @{
 */

/**
 * @brief Initialize the eina convert internal structure.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up the error module of Eina and registers the
 * errors #EINA_ERROR_CONVERT_0X_NOT_FOUND,
 * #EINA_ERROR_CONVERT_P_NOT_FOUND and
 * #EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH. It is also called by
 * eina_init(). It returns 0 on failure, otherwise it returns the
 * number of times it has already been called.
 */
EAPI int
eina_convert_init(void)
{
   _eina_convert_init_count++;

   if (_eina_convert_init_count > 1) goto init_out;

   eina_error_init();
   EINA_ERROR_CONVERT_0X_NOT_FOUND = eina_error_msg_register("Error during string convertion to float, First '0x' was not found.");
   EINA_ERROR_CONVERT_P_NOT_FOUND = eina_error_msg_register("Error during string convertion to float, First 'p' was not found.");
   EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH = eina_error_msg_register("Error outrun string limit during convertion string convertion to float.");

 init_out:
   return _eina_convert_init_count;
}

/**
 * @brief Shut down the eina convert internal structures
 *
 * @return 0 when the convert module is completely shut down, 1 or
 * greater otherwise.
 *
 * This function just shuts down the error module. It is also called by
 * eina_shutdown(). It returns 0 when it is called the same number of
 * times than eina_convert_init().
 */
EAPI int
eina_convert_shutdown(void)
{
   _eina_convert_init_count--;

   if (_eina_convert_init_count > 0) goto shutdown_out;

   eina_error_shutdown();

 shutdown_out:
   return _eina_convert_init_count;
}

/*
 * Come from the second edition of The C Programming Language ("K&R2") on page 64
 */

/**
 * @brief Convert an integer number to a string in decimal base.
 *
 * @param n The integer to convert.
 * @param s The buffer to store the converted integer.
 * @return The length of the string, including the nul terminated
 * character.
 *
 * This function converts @p n to a nul terminated string. The
 * converted string is in decimal base. As no check is done, @p s must
 * be a buffer that is sufficiently large to store the integer.
 *
 * The returned value is the length os the string, including the nul
 * terminated character.
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

/**
 * @brief Convert an integer number to a string in hexadecimal base.
 *
 * @param n The integer to convert.
 * @param s The buffer to store the converted integer.
 * @return The length of the string, including the nul terminated
 * character.
 *
 * This function converts @p n to a nul terminated string. The
 * converted string is in hexadecimal base and the alphabetical
 * cyphers are in lower case. As no check is done, @p s must be a
 * buffer that is sufficiently large to store the integer.
 *
 * The returned value is the length os the string, including the nul
 * terminated character.
 */
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

/**
 * @brief Convert a string to a double
 *
 * @param src The string to convert.
 * @param length The length of the string.
 * @param m The mantisse.
 * @param e The exponent.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function converts the string @p s of length @p length that
 * represent a double in hexadecimal base to a double. It is used to
 * replace the use of snprintf() with the \%a modifier, which is
 * missing on some platform (like Windows (tm) or OpenBSD).
 *
 * The string must have the following format:
 *
 * @code
 * [-]0xh.hhhhhp[+-]e
 * @endcode
 *
 * where the h are the hexadecimal cyphers of the mantiss and e the
 * exponent (a decimal number). If n is the number of cypers after the
 * point, the returned mantiss and exponents are:
 *
 * @code
 * mantiss  : [-]hhhhhh
 * exponent : 2^([+-]e - 4 * n)
 * @endcode
 *
 * The mantiss and exponent are stored in the buffers pointed
 * respectively by @p m and @p e.
 *
 * If the string is invalid, the error is set to:
 *
 * @li #EINA_ERROR_CONVERT_0X_NOT_FOUND if no 0x is found,
 * @li #EINA_ERROR_CONVERT_P_NOT_FOUND if no p is found,
 * @li #EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH if @p length is not
 * correct.
 *
 * In those cases, #EINA_FALSE is returned, otherwise #EINA_TRUE is
 * returned.
 */
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

/**
 * @brief Convert a double to a string
 *
 * @param d The double to convert.
 * @param des The destination buffer to store the converted double.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function converts the double @p d to a string. The string is
 * stored in the buffer pointed by @p des and must be sufficiently
 * large to contain the converted double. The returned string is
 * terminated and has the following format:
 *
 * @code
 * [-]0xh.hhhhhp[+-]e
 * @endcode
 *
 * where the h are the hexadecimal cyphers of the mantiss and e the
 * exponent (a decimal number).
 *
 * The returned value is the length of the string, including the nul
 * character.
 */
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

/**
 * @}
 */

/**
 * @}
 */
