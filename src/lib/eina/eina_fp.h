/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
 * Copyright (C) 2009 Cedric BAIL
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

#ifndef EINA_FP_H_
# define EINA_FP_H_

/**
 * @addtogroup Eina_Fp_Group Fp
 *
 * @brief Floating point numbers data type management.
 */

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @defgroup Eina_Fp_Group Fp
 *
 * @{
 */

#include "eina_types.h"

#ifdef _MSC_VER
typedef unsigned __int64 uint64_t;
typedef signed __int64   int64_t;
typedef signed int       int32_t;
#else
# include <stdint.h>
#endif

/**
 * @def EINA_F32P32_PI
 * @brief Yields the 32-bit PI constant
 */
#define EINA_F32P32_PI 0x00000003243f6a89

/**
 * @typedef Eina_F32p32
 * Type for floating point number where the size of the integer part is 32-bit
 * and the size of the decimal part is 32-bit
 */
typedef int64_t Eina_F32p32;

/**
 * @typedef Eina_F16p16
 * Type for floating point number where the size of the integer part is 16-bit
 * and the size of the decimal part is 16-bit
 */
typedef int32_t Eina_F16p16;

/**
 * @typedef Eina_F8p24
 * Type for floating point number where the size of the integer part is 8-bit
 * and the size of the decimal part is 24bits
 */
typedef int32_t Eina_F8p24;

/**
 * @brief Creates a new Eina_F32p32 floating point number from standard 32-bit
 * integer
 *
 * @param[in] v 32-bit integer value to convert
 * @return The value converted into Eina_F32p32 format
 */
static inline Eina_F32p32  eina_f32p32_int_from(int32_t v);

/**
 * @brief Creates a new standard 32-bit integer from Eina_F32p32 floating point
 * number
 *
 * @param[in] v Eina_F32p32 value to convert
 * @return The value converted into 32-bit integer
 */
static inline int32_t      eina_f32p32_int_to(Eina_F32p32 v);

/**
 * @brief Creates a new Eina_F32p32 floating point number from standard double
 *
 * @param[in] v Double value to convert
 * @return The value converted into Eina_F32p32 format
 */
static inline Eina_F32p32  eina_f32p32_double_from(double v);

/**
 * @brief Creates a new standard double from Eina_F32p32 floating point
 * number
 *
 * @param[in] v Eina_F32p32 value to convert
 * @return The value converted into double
 */
static inline double       eina_f32p32_double_to(Eina_F32p32 v);

/**
 * @brief Calculates the sum of two Eina_F32p32 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The second number
 * @return The sum result of the two numbers @p a + @p b
 */
static inline Eina_F32p32  eina_f32p32_add(Eina_F32p32 a, Eina_F32p32 b);

/**
 * @brief Calculates the subtraction of two Eina_F32p32 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The subtracted number
 * @return The subtraction result of the two numbers @p a - @p b
 */
static inline Eina_F32p32  eina_f32p32_sub(Eina_F32p32 a, Eina_F32p32 b);

/**
 * @brief Calculates the multiplication of two Eina_F32p32 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The second number
 * @return The multiplication result of the two numbers @p a * @p b
 *
 * To prevent overflow during multiplication we need to reduce the precision of
 * the fraction part Shift both values to only contain 16 bit of the fraction
 * part (rounded).  After multiplication we again have a value with a 32-bit
 * fraction part.
 */
static inline Eina_F32p32  eina_f32p32_mul(Eina_F32p32 a, Eina_F32p32 b);

/**
 * @brief Calculates the scale multiplication of one Eina_F32p32 floating point
 * number with an integer 
 *
 * @param[in] a The Eina_F32p32 number
 * @param[in] b The integer value
 * @return The multiplication result of the two numbers @p a * @p b
 */
static inline Eina_F32p32  eina_f32p32_scale(Eina_F32p32 a, int b);

/**
 * @brief Calculates the division of two Eina_F32p32 floating point numbers
 *
 * @param[in] a The numerator number
 * @param[in] b The denominator number
 * @return The division result of the two numbers @p a / @p b
 */
static inline Eina_F32p32  eina_f32p32_div(Eina_F32p32 a, Eina_F32p32 b);

/**
 * @brief Calculates the square root of an Eina_F32p32 floating point number
 *
 * @param[in] a The number to calculate the square root from
 * @return The square root result for the number @p a
 */
static inline Eina_F32p32  eina_f32p32_sqrt(Eina_F32p32 a);

/**
 * @brief Gets the absolute value of the integer part of and Eina_F32p32 floating
 * point number
 *
 * @param[in] v The floating point number
 * @return The positive integer part of the number @p v
 */
static inline unsigned int eina_f32p32_fracc_get(Eina_F32p32 v);

/**
 * @brief Gets the absolute value of an Eina_F32p32 floating point number
 *
 * @param[in] a The floating point number
 * @return The absolute value for the number @p a
 * @warning Has known issues on 64-bit architecture, prefer
 * eina_f32p32_fracc_get() instead
 */
#define eina_fp32p32_llabs(a) ((a < 0) ? -(a) : (a))

/**
 * @brief Calculates the cosine of a floating point number
 *
 * @param[in] a The angle in radians to calculate the cosine from.
 * @return The cosine of the angle @p a
 */
EAPI Eina_F32p32           eina_f32p32_cos(Eina_F32p32 a);

/**
 * @brief Calculates the sine of a floating point number
 *
 * @param[in] a The angle in radians to calculate the sine from.
 * @return The cosine of the angle @p a
 */
EAPI Eina_F32p32           eina_f32p32_sin(Eina_F32p32 a);


/**
 * @def EINA_F16P16_ONE
 *
 * Yields the maximum 16-bit unsigned integer size (= 65536)
 */
#define EINA_F16P16_ONE (1 << 16)

/**
 * @def EINA_F16P16_HALF
 *
 * Yields the maximum 16-bit signed integer size (= 32768)
 */
#define EINA_F16P16_HALF (1 << 15)

/**
 * @brief Creates a new Eina_F16p316 floating point number from standard 32-bit
 * integer
 *
 * @param[in] v 32-bit integer value to convert
 * @return The value converted into Eina_F16p16 format
 */
static inline Eina_F16p16  eina_f16p16_int_from(int32_t v);

/**
 * @brief Creates a new standard 32-bit integer from Eina_F16p16 floating point
 * number 
 *
 * @param[in] v Eina_F16p16 value to convert
 * @return The value converted into 32-bit integer
 */
static inline int32_t      eina_f16p16_int_to(Eina_F16p16 v);

/**
 * @brief Creates a new Eina_F16p16 floating point number from standard double
 *
 * @param[in] v Double value to convert
 * @return The value converted into Eina_F16p16 format
 */
static inline Eina_F16p16  eina_f16p16_double_from(double v);

/**
 * @brief Creates a new standard double from Eina_F16p16 floating point
 * number 
 *
 * @param[in] v Eina_F16p16 value to convert
 * @return The value converted into double
 */
static inline double       eina_f16p16_double_to(Eina_F16p16 v);

/**
 * @brief Creates a new Eina_F16p16 floating point number from standard float
 *
 * @param[in] v Float value to convert
 * @return The value converted into Eina_F16p16 format
 */
static inline Eina_F16p16  eina_f16p16_float_from(float v);

/**
 * @brief Creates a new standard float from Eina_F16p16 floating point
 * number
 *
 * @param[in] v Eina_F16p16 value to convert
 * @return The value converted into float
 */
static inline float        eina_f16p16_float_to(Eina_F16p16 v);

/**
 * @brief Calculates the sum of two Eina_F16p16 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The second number
 * @return The sum result of the two numbers @p a + @p b
 */
static inline Eina_F16p16  eina_f16p16_add(Eina_F16p16 a, Eina_F16p16 b);

/**
 * @brief Calculates the subtraction of two Eina_F16p16 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The subtracted number
 * @return The subtraction result of the two numbers @p a - @p b
 */
static inline Eina_F16p16  eina_f16p16_sub(Eina_F16p16 a, Eina_F16p16 b);

/**
 * @brief Calculates the multiplication of two Eina_F16p16 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The second number
 * @return The multiplication result of the two numbers @p a * @p b
 */
static inline Eina_F16p16  eina_f16p16_mul(Eina_F16p16 a, Eina_F16p16 b);

/**
 * @brief Calculates the scale multiplication of one Eina_F16p16 floating point
 * number with an integer
 *
 * @param[in] a The Eina_F16p16 number
 * @param[in] b The integer value
 * @return The multiplication result of the two numbers @p a * @p b
 */
static inline Eina_F16p16  eina_f16p16_scale(Eina_F16p16 a, int b);

/**
 * @brief Calculates the division of two Eina_F16p16 floating point numbers
 *
 * @param[in] a The numerator number
 * @param[in] b The denominator number
 * @return The division result of the two numbers @p a / @p b
 */
static inline Eina_F16p16  eina_f16p16_div(Eina_F16p16 a, Eina_F16p16 b);

/**
 * @brief Calculates the square root of an Eina_F16p16 floating point number
 *
 * @param[in] a The number to calculate the square root from
 * @return The square root result for the number @p a
 */
static inline Eina_F16p16  eina_f16p16_sqrt(Eina_F16p16 a);

/**
 * @brief Gets the absolute value of the integer part of and Eina_F16p16 floating
 * point number
 *
 * @param[in] v The floating point number
 * @return The positive integer part of the number @p v
 */
static inline unsigned int eina_f16p16_fracc_get(Eina_F16p16 v);


/**
 * @brief Creates a new Eina_F16p316 floating point number from standard 32-bit
 * integer
 *
 * @param[in] v 32-bit integer value to convert
 * @return The value converted into Eina_F8p24 format
 */
static inline Eina_F8p24   eina_f8p24_int_from(int32_t v);

/**
 * @brief Creates a new standard 32-bit integer from Eina_F8p24 floating point
 * number
 *
 * @param[in] v Eina_F8p24 value to convert
 * @return The value converted into 32-bit integer
 */
static inline int32_t      eina_f8p24_int_to(Eina_F8p24 v);

/**
 * @brief Creates a new Eina_F8p24 floating point number from standard float
 *
 * @param[in] v Float value to convert
 * @return The value converted into Eina_F8p24 format
 */
static inline Eina_F8p24   eina_f8p24_float_from(float v);

/**
 * @brief Create a new standard float from Eina_F8p24 floating point number
 *
 * @param[in] v Eina_F8p24 value to convert
 * @return The value converted into float
 */
static inline float        eina_f8p24_float_to(Eina_F8p24 v);

/**
 * @brief Calculates the sum of two Eina_F8p24 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The second number
 * @return The sum result of the two numbers @p a + @p b
 */
static inline Eina_F8p24   eina_f8p24_add(Eina_F8p24 a, Eina_F8p24 b);

/**
 * @brief Calculates the subtraction of two Eina_F8p24 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The subtracted number
 * @return The subtraction result of the two numbers @p a - @p b
 */
static inline Eina_F8p24   eina_f8p24_sub(Eina_F8p24 a, Eina_F8p24 b);

/**
 * @brief Calculates the multiplication of two Eina_F8p24 floating point numbers
 *
 * @param[in] a The first number
 * @param[in] b The second number
 * @return The multiplication result of the two numbers @p a * @p b
 */
static inline Eina_F8p24   eina_f8p24_mul(Eina_F8p24 a, Eina_F8p24 b);

/**
 * @brief Calculates the scale multiplication of one Eina_F8p24 floating point
 * number with an integer
 *
 * @param[in] a The Eina_F16p16 number
 * @param[in] b The integer value
 * @return The multiplication result of the two numbers @p a * @p b
 */
static inline Eina_F8p24   eina_f8p24_scale(Eina_F8p24 a, int b);

/**
 * @brief Calculates the division of two Eina_F8p24 floating point numbers
 *
 * @param[in] a The numerator number
 * @param[in] b The denominator number
 * @return The division result of the two numbers @p a / @p b
 */
static inline Eina_F8p24   eina_f8p24_div(Eina_F8p24 a, Eina_F8p24 b);

/**
 * @brief Calculates the square root of an Eina_F8p24 floating point number
 *
 * @param[in] a The number to calculate the square root from
 * @return The square root result for the number @p a
 */
static inline Eina_F8p24   eina_f8p24_sqrt(Eina_F8p24 a);

/**
 * @brief Gets the absolute value of the integer part of and Eina_F8p24 floating
 * point number
 *
 * @param[in] v The floating point number
 * @return The positive integer part of the number @p v
 */
static inline unsigned int eina_f8p24_fracc_get(Eina_F8p24 v);

/**
 * @brief Converts an Eina_F16p16 floating point number into Eina_F32p32 format
 *
 * @param[in] a The Eina_F16p16 floating point number
 * @return The converted Eina_F32p32 floating point number
 */
static inline Eina_F32p32  eina_f16p16_to_f32p32(Eina_F16p16 a);

/**
 * @brief Converts an Eina_F8p24 floating point number into Eina_F32p32 format
 *
 * @param[in] a The Eina_F8p24 floating point number
 * @return The converted Eina_F32p32 floating point number
 */
static inline Eina_F32p32  eina_f8p24_to_f32p32(Eina_F8p24 a);

/**
 * @brief Converts an Eina_F32p32 floating point number into Eina_F16p16 format
 *
 * @param[in] a The Eina_F32p32 floating point number
 * @return The converted Eina_F16p16 floating point number
 */
static inline Eina_F16p16  eina_f32p32_to_f16p16(Eina_F32p32 a);

/**
 * @brief Converts an Eina_F8p24 floating point number into Eina_F16p16 format
 *
 * @param[in] a The Eina_F8p24 floating point number
 * @return The converted Eina_F16p16 floating point number
 */
static inline Eina_F16p16  eina_f8p24_to_f16p16(Eina_F8p24 a);

/**
 * @brief Converts an Eina_F32p32 floating point number into Eina_F8p24 format
 *
 * @param[in] a The Eina_F32p32 floating point number
 * @return The converted Eina_F8p16 floating point number
 */
static inline Eina_F8p24   eina_f32p32_to_f8p24(Eina_F32p32 a);

/**
 * @brief Converts an Eina_F16p16 floating point number into Eina_F8p16 format
 *
 * @param[in] a The Eina_F16p16 floating point number
 * @return The converted Eina_F8p16 floating point number
 */
static inline Eina_F8p24   eina_f16p16_to_f8p24(Eina_F16p16 a);

#include "eina_inline_f32p32.x"
#include "eina_inline_f16p16.x"
#include "eina_inline_f8p24.x"
#include "eina_inline_fp.x"

/**
 * @}
 */

/**
 * @}
 */

#endif
