/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
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

#ifndef EINA_RECTANGLE_H_
#define EINA_RECTANGLE_H_

/**
 * @file
 * @{
 */

#include "eina_types.h"

/**
 * To be documented
 * FIXME: To be fixed
 */
typedef struct _Eina_Rectangle
{
	int	x;
	int	y;
	int	w;
	int	h;
} Eina_Rectangle;

typedef struct _Eina_Rectangle_Pool Eina_Rectangle_Pool;

static inline int eina_spans_intersect(int c1, int l1, int c2, int l2);
static inline Eina_Bool eina_rectangle_is_empty(Eina_Rectangle *r);
static inline void eina_rectangle_coords_from(Eina_Rectangle *r, int x, int y, int w, int h);
static inline Eina_Bool eina_rectangles_intersect(Eina_Rectangle *r1, Eina_Rectangle *r2);
static inline Eina_Bool eina_rectangle_xcoord_inside(Eina_Rectangle *r, int x);
static inline Eina_Bool eina_rectangle_ycoord_inside(Eina_Rectangle *r, int y);
static inline Eina_Bool eina_rectangle_coords_inside(Eina_Rectangle *r, int x, int y);
static inline void eina_rectangle_union(Eina_Rectangle *dst, Eina_Rectangle *src);
static inline Eina_Bool eina_rectangle_intersection(Eina_Rectangle *dst, Eina_Rectangle *src);
static inline void eina_rectangle_rescale_in(Eina_Rectangle *out, Eina_Rectangle *in, Eina_Rectangle *res);
static inline void eina_rectangle_rescale_out(Eina_Rectangle *out, Eina_Rectangle *in, Eina_Rectangle *res);

EAPI Eina_Rectangle_Pool *eina_rectangle_pool_add(int w, int h);
EAPI Eina_Rectangle_Pool *eina_rectangle_pool_get(Eina_Rectangle *rect);
EAPI Eina_Bool eina_rectangle_pool_geometry_get(Eina_Rectangle_Pool *pool, int *w, int *h);
EAPI void *eina_rectangle_pool_data_get(Eina_Rectangle_Pool *pool);
EAPI void eina_rectangle_pool_data_set(Eina_Rectangle_Pool *pool, const void *data);
EAPI void eina_rectangle_pool_delete(Eina_Rectangle_Pool *pool);
EAPI int eina_rectangle_pool_count(Eina_Rectangle_Pool *pool);
EAPI Eina_Rectangle *eina_rectangle_pool_request(Eina_Rectangle_Pool *pool, int w, int h);
EAPI void eina_rectangle_pool_release(Eina_Rectangle *rect);

#include "eina_inline_rectangle.x"

/** @} */

#endif /*_EINA_RECTANGLE_H_*/
