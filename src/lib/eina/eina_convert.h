/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric BAIL, Vincent Torri
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_CONVERT_H_
#define EINA_CONVERT_H_

#include "eina_types.h"
#include "eina_error.h"

#include "eina_fp.h"


/**
 * @defgroup Eina_Convert_Group Convert
 * @ingroup Eina_Tools_Group
 *
 * @brief This group discusses the functions that allow you to convert an
 *        integer or real numbers to a string or conversely.
 *
 * To use these functions, you have to call eina_init()
 * first, and then eina_shutdown() when eina is not used anymore.
 *
 * @section Eina_Convert_From_Integer_To_Sring Conversion from integer to string
 *
 * To convert an integer to a string in the decimal base,
 * eina_convert_itoa() should be used. If the hexadecimal base is
 * wanted, eina_convert_xtoa() should be used. They all need a buffer that is
 * sufficiently large to store all the cyphers.
 *
 * Here is an example for use:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 *
 * #include <Eina.h>
 *
 * int main(void)
 * {
 *    char tmp[128];
 *
 *    if (!eina_init())
 *    {
 *        printf ("Error during the initialization of eina.\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    eina_convert_itoa(45, tmp);
 *    printf("value: %s\n", tmp);
 *    eina_convert_xtoa(0xA1, tmp);
 *    printf("value: %s\n", tmp);
 *
 *    eina_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * Compile this code with the following command:
 *
 * @code
 * gcc -Wall -o test_eina_convert test_eina.c `pkg-config --cflags --libs eina`
 * @endcode
 *
 * @remarks  The alphabetical cyphers are in lower case.
 *
 * @section Eina_Convert_Double Conversion double / string
 *
 * To convert a double to a string, eina_convert_dtoa() should be
 * used. Like with the integer functions, a buffer must be used. The
 * resulting string has the following format (which is the result
 * obtained with snprintf() and the modifier):
 *
 * @code
 * [-]0xh.hhhhhp[+-]e
 * @endcode
 *
 * To convert a string to a double, eina_convert_atod() should be
 * used. The format of the string must be as above. Then, the double
 * has the following mantissa and exponent:
 *
 * @code
 * mantiss  : [-]hhhhhh
 * exponent : 2^([+-]e - 4 * n)
 * @endcode
 *
 * with n being the number of cyphers after the point in the string
 * format. To obtain the double number from the mantissa and exponent,
 * use ldexp().
 *
 * Here is an example for use:
 *
 * @code
 * #include <stdlib.h>
 * #include <stdio.h>
 * #include <math.h>
 *
 * #include <Eina.h>
 *
 * int main(void)
 * {
 *    char      tmp[128];
 *    long long int m = 0;
 *    long int  e = 0;
 *    double    r;
 *
 *    if (!eina_init())
 *    {
 *        printf ("Error during the initialization of eina.\n");
 *        return EXIT_FAILURE;
 *    }
 *
 *    printf("initial value : 40.56\n");
 *    eina_convert_dtoa(40.56, tmp);
 *    printf("result dtoa   : %s\n", tmp);

 *    eina_convert_atod(tmp, 128, &m, &e);
 *    r = ldexp((double)m, e);
 *    printf("result atod   : %f\n", r);
 *
 *    eina_shutdown();
 *
 *    return EXIT_SUCCESS;
 * }
 * @endcode
 *
 * Compile this code with the following command:
 *
 * @code
 * gcc -Wall -o test_eina_convert test_eina.c `pkg-config --cflags --libs eina` -lm
 * @endcode
 *
 * @{
 */

/**
 * @var EINA_ERROR_CONVERT_P_NOT_FOUND
 * @brief Error identifier corresponding to string not containing 'p'.
 */

EAPI extern Eina_Error EINA_ERROR_CONVERT_P_NOT_FOUND;

/**
 * @var EINA_ERROR_CONVERT_0X_NOT_FOUND
 * @brief The error identifier corresponding to the string not containing '0x'.
 */
EAPI extern Eina_Error EINA_ERROR_CONVERT_0X_NOT_FOUND;

/**
 * @var EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH
 * @brief The error identifier corresponding to the length of the string being too small.
 */
EAPI extern Eina_Error EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH;

/**
 * @brief Converts an integer number to a string in decimal base.
 *
 * @details This function converts @a n to a null terminated string. The
 *          converted string is in decimal base. As no check is done, @a s must
 *          be a buffer that is sufficiently large to store the integer.
 *
 * @since_tizen 2.3
 *
 * @remarks The returned value is the length of the string, including the null
 *          terminated character.
 *
 * @param[in] n The integer to convert
 * @param[in] s The buffer to store the converted integer
 * @return The length of the string, including the null terminated
 *         character
 *
 */
EAPI int       eina_convert_itoa(int n, char *s)  EINA_ARG_NONNULL(2);

/**
 * @brief Converts an integer number to a string in hexadecimal base.
 *
 * @details This function converts @a n to a null terminated string. The
 *          converted string is in hexadecimal base and the alphabetical
 *          cyphers are in lower case. As no check is done, @a s must be a
 *          buffer that is sufficiently large to store the integer.
 *
 * @since_tizen 2.3
 *
 * @remarks The returned value is the length of the string, including the null
 *          terminated character.
 *
 * @param[in] n The integer to convert
 * @param[in] s The buffer to store the converted integer
 * @return The length of the string, including the null terminated
 *         character
 *
 */
EAPI int       eina_convert_xtoa(unsigned int n, char *s) EINA_ARG_NONNULL(2);


/**
 * @brief Converts a double to a string.
 *
 * @details This function converts the double @a d to a string. The string is
 *          stored in the buffer pointed by @a des and must be sufficiently
 *          large to contain the converted double. The returned string is null
 *          terminated and has the following format:
 *
 * @since_tizen 2.3
 *
 * @code
 * [-]0xh.hhhhhp[+-]e
 * @endcode
 *
 * where h are the hexadecimal cyphers of the mantissa and e is the
 * exponent (a decimal number).
 *
 * @remarks The returned value is the length of the string, including the null
 *          terminated character.
 *
 * @param[in] d The double to convert
 * @param[in] des The destination buffer to store the converted double
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI int       eina_convert_dtoa(double d, char *des) EINA_ARG_NONNULL(2);

/**
 * @brief Converts a string to a double.
 *
 * @details This function converts the string @a s of length @a length that
 *          represents a double in hexadecimal base to a double. It is used to
 *          replace the use of snprintf() with the modifier, which is
 *          missing on some platform (like Windows (tm) or OpenBSD).
 *
 * @since_tizen 2.3
 *
 * The string must have the following format:
 *
 * @code
 * [-]0xh.hhhhhp[+-]e
 * @endcode
 *
 * where h are the hexadecimal cyphers of the mantissa and e is the
 * exponent (a decimal number). If n is the number of cyphers after the
 * point, the returned mantissa and exponent are:
 *
 * @code
 * mantiss  : [-]hhhhhh
 * exponent : 2^([+-]e - 4 * n)
 * @endcode
 *
 * The mantissa and exponent are stored in the buffers pointed
 * by @a m and @a e respectively.
 *
 * If the string is invalid, the error is set to:
 *
 * @li #EINA_ERROR_CONVERT_0X_NOT_FOUND - If no 0x is found,
 * @li #EINA_ERROR_CONVERT_P_NOT_FOUND - If no p is found,
 * @li #EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH If @a length is not correct.
 *
 * In those cases, @c EINA_FALSE is returned, otherwise @c EINA_TRUE is
 * returned.
 *
 * @param[in] src The string to convert
 * @param[in] length The length of the string
 * @param[out] m The mantissa
 * @param[out] e The exponent
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_convert_atod(const char *src,
                                 int         length,
                                 long long  *m,
                                 long       *e) EINA_ARG_NONNULL(1, 3, 4);


/**
 * @brief Converts a 32.32 fixed point number to a string.
 *
 * @details This function converts the 32.32 fixed point number @a fp to a
 *          string. The string is stored in the buffer pointed by @a des and
 *          must be sufficiently large to contain the converted fixed point
 *          number. The returned string is terminated and has the following
 *          format:
 *
 * @code
 * [-]0xh.hhhhhp[+-]e
 * @endcode
 *
 * where h are the hexadecimal cyphers of the mantissa and e is the
 * exponent (a decimal number).
 *
 * @since_tizen 2.3
 *
 * @remarks The returned value is the length of the string, including the null
 *          terminating character.
 *
 * @remarks The code is the same as eina_convert_dtoa() except that it
 *          implements the frexp() function for fixed point numbers and does
 *          some optimisations.
 *
 * @param[in] fp The fixed point number to convert
 * @param[in] des The destination buffer to store the converted fixed point number
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI int       eina_convert_fptoa(Eina_F32p32 fp,
                                  char       *des) EINA_ARG_NONNULL(2);

/**
 * @brief Converts a string to a 32.32 fixed point number.
 *
 * @details This function converts the string @a src of length @a length that
 *          represents a double in hexadecimal base to a 32.32 fixed point
 *          number stored in @a fp. The function always tries to convert the
 *          string with eina_convert_atod().
 *
 * The string must have the following format:
 *
 * @code
 * [-]0xh.hhhhhp[+-]e
 * @endcode
 *
 * where h are the hexadecimal cyphers of the mantissa and e is the
 * exponent (a decimal number). If n is the number of cyphers after the
 * point, the returned mantissa and exponent are:
 *
 * @code
 * mantiss  : [-]hhhhhh
 * exponent : 2^([+-]e - 4 * n)
 * @endcode
 *
 * The mantissa and exponent are stored in the buffers pointed
 * by @a m and @a e respectively.
 *
 * @remarks If the string is invalid, the error is set to:
 *
 * @li #EINA_ERROR_CONVERT_0X_NOT_FOUND - If no 0x is found,
 * @li #EINA_ERROR_CONVERT_P_NOT_FOUND If no p is found,
 * @li #EINA_ERROR_CONVERT_OUTRUN_STRING_LENGTH If @a length is not correct.
 *
 * In those cases, or if @a fp is @c NULL, @c EINA_FALSE is returned,
 * otherwise @a fp is computed and @c EINA_TRUE is returned.
 *
 * @since_tizen 2.3
 *
 * @remarks The code uses eina_convert_atod() and does the correct bit
 *          shift to compute the fixed point number.
 *
 * @param[in] src The string to convert
 * @param[in] length The length of the string
 * @param[out] fp The fixed point number
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_convert_atofp(const char  *src,
                                  int          length,
                                  Eina_F32p32 *fp) EINA_ARG_NONNULL(1, 3);

/**
 * @}
 */

#endif /* EINA_CONVERT_H_ */
