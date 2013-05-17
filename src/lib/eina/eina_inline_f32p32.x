/* EINA - EFL data type library
 * Copyright (C) 2007-2009 Jorge Luis Zapata Muga, Cedric BAIL
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

#ifndef EINA_INLINE_F32P32_X_
# define EINA_INLINE_F32P32_X_

#include <stdlib.h>

static inline Eina_F32p32
eina_f32p32_add(Eina_F32p32 a, Eina_F32p32 b)
{
   return a + b;
}

static inline Eina_F32p32
eina_f32p32_sub(Eina_F32p32 a, Eina_F32p32 b)
{
   return a - b;
}

static inline Eina_F32p32
eina_f32p32_mul(Eina_F32p32 a, Eina_F32p32 b)
{
   /* To prevent overflow during multiplication
    * we need to reduce the precision of the fraction part
    * Shift both values to only contain 16 bit of the fraction part
    * (rounded).
    * After multiplication we again have a value with a 32-bit
    * fraction part. See also
    * http://en.wikipedia.org/wiki/Fixed-point_arithmetic#Operations
    */

   Eina_F32p32 up;
   Eina_F32p32 result;
   uint64_t as, bs;
   Eina_F32p32 sign;

   sign = a ^ b;
   as = eina_fp32p32_llabs(a);
   bs = eina_fp32p32_llabs(b);

   /* Reduce fraction to 16-bit (rounded) */
   as = (as >> 16) + ((as >> 15) & 0x1);
   bs = (bs >> 16) + ((bs >> 15) & 0x1);

   up = as * bs;

   result = up;

   return sign < 0 ? - result : result;
}

static inline Eina_F32p32
eina_f32p32_scale(Eina_F32p32 a, int b)
{
   return a * b;
}

static inline Eina_F32p32
eina_f32p32_div(Eina_F32p32 a, Eina_F32p32 b)
{
   Eina_F32p32 sign;
   Eina_F32p32 result;

   sign = a ^ b;

   /* FIXME: This should probably map to +/-inf when converting to double
      or we should abort... */
   if (b == 0)
     return sign < 0 ? (Eina_F32p32) 0x8000000000000000ull : (Eina_F32p32) 0x7FFFFFFFFFFFFFFFull;

   result = (eina_f32p32_mul(eina_fp32p32_llabs(a),  (((uint64_t) 1 << 62) / ((uint64_t)(eina_fp32p32_llabs(b)) >> 2))));

   return sign < 0 ? - result : result;
}

static inline Eina_F32p32
eina_f32p32_sqrt(Eina_F32p32 a)
{
   uint64_t root, remHi, remLo, testDiv, count;

   root = 0; /* Clear root */
   remHi = 0; /* Clear high part of partial remainder */
   remLo = a; /* Get argument into low part of partial remainder */
   count = (31 + (32 >> 1)); /* Load loop counter */
   do {
      remHi = (remHi << 2) | (remLo >> 30);
      remLo <<= 2; /* get 2 bits of arg */
      root <<= 1; /* Get ready for the next bit in the root */
      testDiv = (root << 1) + 1; /* Test radical */
      if (remHi >= testDiv) {
	 remHi -= testDiv;
	 root++;
      }
   } while (count-- != 0);

   return root;
}

static inline unsigned int
eina_f32p32_fracc_get(Eina_F32p32 v)
{
   return (unsigned int)v;
}

#endif
