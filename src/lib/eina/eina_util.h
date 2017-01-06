/* EINA - EFL data type library
 * Copyright (C) 2015 Vincent Torri
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

#ifndef EINA_UTIL_H_
#define EINA_UTIL_H_

#include <float.h>

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @brief Return the content of the environment refered by HOME on this system.
 * @return A temporary string to the content refered by HOME on this system.
 *
 * @note The result of this call is highly system dependent and you better use
 * it instead of the naive getenv("HOME").
 *
 * @since 1.15
 */
EAPI const char *eina_environment_home_get(void);

/**
 * @brief Return the content of the environment refered as TMPDIR on this system.
 * @return A temporary string to the content refered by TMPDIR on this system.
 *
 * @note The result of this call is highly system dependent and you better use
 * it instead of the naive getenv("TMPDIR").
 *
 * @since 1.15
 */
EAPI const char *eina_environment_tmp_get(void);

/**
 * @brief Warningless comparison of doubles using ==
 * @param a First member to compare
 * @param b Second member to compare
 *
 * @return @c true if two doubles match
 * @since 1.19
 */
EAPI Eina_Bool eina_dbleq(double a, double b);

/**
 * @brief Warningless comparison of floats using ==
 * @param a First member to compare
 * @param b Second member to compare
 *
 * @return @c true if two floats match
 * @since 1.19
 */
EAPI Eina_Bool eina_flteq(float a, float b);

/**
 * @brief Safe comparison of float
 * @param a First member to compar
 * @param b Second member to compar
 *
 * @return @c true if two floats match
 */
#define EINA_FLT_CMP(a, b) (fabsf((float)a - (float)b) <= FLT_EPSILON)

/**
 * @brief Determine if a float is nonzero
 * @param a The float
 *
 * @return @c true if float is nonzero
 * @since 1.19
 */
#define EINA_FLT_NONZERO(a) (!EINA_FLT_CMP((a), 0.0))

/**
 * @brief Safe comparison of double
 * @param a First member to compar
 * @param b Second member to compar
 *
 * @return @c true if two double match
 */
#define EINA_DBL_CMP(a, b) (fabs((double)a - (double)b) <= DBL_EPSILON)

/**
 * @brief Determine if a double is nonzero
 * @param a The double
 *
 * @return @c true if double is nonzero
 * @since 1.19
 */
#define EINA_DBL_NONZERO(a) (!EINA_DBL_CMP((a), 0.0))

/**
 * @}
 */

#endif
