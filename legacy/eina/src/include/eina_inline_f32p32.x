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
   /* Prevent overflow and do '(a * b) >> 32' */
   /* f32p32 * f32p32 = f64p64 */
   /* f16p16 * f16p16 = f32p32 */
   Eina_F32p32 up;
   Eina_F32p32 down;
   Eina_F32p32 result;

   up = (a >> 16) * (b >> 16);
   down = (a & 0xFFFF) * (b & 0xFFFF);

   result = up + (down >> 32);

   return result;
}

static inline Eina_F32p32
eina_f32p32_scale(Eina_F32p32 a, int b)
{
   return a * b;
}

static inline Eina_F32p32
eina_f32p32_div(Eina_F32p32 a, Eina_F32p32 b)
{
   /* Prevent overflow and do '(a << 32) / b' */
   /* f32p64 / f32p32 = f32p32 */
   /* f32p32 / f32p32 = f32p0 */
   Eina_F32p32 up;
   Eina_F32p32 down;
   Eina_F32p32 result;

   up = (a / b) << 32;
   down = (a << 32) / b;

   result = up + down;

   return result;
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
