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

#ifndef EINA_F32P32_H_
#define EINA_F32P32_H_

#ifdef _MSC_VER
# include <Evil.h>
#else
# include <stdint.h>
#endif

/**
 * To be documented
 * FIXME: To be fixed
 */
typedef int64_t Eina_F32p32;

/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F32p32 eina_f32p32_int_from(int32_t v)
{
	return (Eina_F32p32) v << 32;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline int32_t eina_f32p32_int_to(Eina_F32p32 v)
{
	return v >> 32;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F32p32 eina_f32p32_double_from(double v)
{
	Eina_F32p32 r;
	r = (Eina_F32p32)(v * 4294967296.0 + (v < 0 ? -0.5 : 0.5));
	return r;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline double eina_f32p32_double_to(Eina_F32p32 v)
{
	double r;
	r = v / 4294967296.0;
	return r;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F32p32 eina_f32p32_add(Eina_F32p32 a, Eina_F32p32 b)
{
	return a + b;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F32p32 eina_f32p32_sub(Eina_F32p32 a, Eina_F32p32 b)
{
	return a - b;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F32p32 eina_f32p32_mul(Eina_F32p32 a, Eina_F32p32 b)
{
	int64_t low;
	int64_t high;

	low = a * (b & 0xffffffff);
	high = a * (b >> 32);

	return (low >> 32) + high;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F32p32 eina_f32p32_sqrt(Eina_F32p32 a)
{
	uint64_t root, remHi, remLo, testDiv, count;

	root = 0; /* Clear root */
	remHi = 0; /* Clear high part of partial remainder */
	remLo = a; /* Get argument into low part of partial remainder */
	count = (31 + (32 >> 1)); /* Load loop counter */
	do
	{
		remHi = (remHi << 2) | (remLo >> 30);
		remLo <<= 2; /* get 2 bits of arg */
		root <<= 1; /* Get ready for the next bit in the root */
		testDiv = (root << 1) + 1; /* Test radical */
		if (remHi >= testDiv)
		{
			remHi -= testDiv;
			root++;
		}
	} while (count-- != 0);
	return (root);
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline unsigned int eina_f32p32_fracc_get(Eina_F32p32 v)
{
	return (unsigned int)v;
}

#endif /*EINA_F32P32_H_*/
