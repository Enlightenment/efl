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

#ifndef EINA_F8P24_H_
#define EINA_F8P24_H_

#ifdef _MSC_VER
# include <Evil.h>
#else
# include <stdint.h>
#endif

/**
 * To be documented
 * FIXME: To be fixed
 */
typedef int32_t Eina_F8p24;

/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F8p24 eina_f8p24_int_from(int32_t v)
{
	return v << 24;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline int32_t eina_f8p24_int_to(Eina_F8p24 v)
{
	return v >> 24;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F8p24 eina_f8p24_float_from(float v)
{
	Eina_F8p24 r;
	r = (Eina_F8p24)(v * 16777216.0f + (v < 0 ? -0.5f : 0.5f));
	return r;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline float eina_f8p24_float_to(Eina_F8p24 v)
{
	float r;
	r = v / 16777216.0f;
	return r;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F8p24 eina_f8p24_add(Eina_F8p24 a, Eina_F8p24 b)
{
	return a + b;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F8p24 eina_f8p24_sub(Eina_F8p24 a, Eina_F8p24 b)
{
	return a - b;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F8p24 eina_f8p24_mul(Eina_F8p24 a, Eina_F8p24 b)
{
	return ((int64_t)a * (int64_t)b) >> 24;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_F8p24 eina_f8p24_sqrt(Eina_F8p24 a)
{
	unsigned int root, remHi, remLo, testDiv, count;

	root = 0; /* Clear root */
	remHi = 0; /* Clear high part of partial remainder */
	remLo = a; /* Get argument into low part of partial remainder */
	count = (23 + (24 >> 1)); /* Load loop counter */
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
static inline unsigned int eina_f8p24_fracc_get(Eina_F8p24 v)
{
	return (v & 0xffffff);
}

#endif /*EINA_F8P24_H_*/
