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

#ifndef EINA_INLINE_RECTANGLE_H__
#define EINA_INLINE_RECTANGLE_H__

static inline Eina_Bool
eina_rectangle_is_valid(const Eina_Rectangle *r)
{
   if ((r->w > 0) && (r->h > 0)) return EINA_TRUE;
   return EINA_FALSE;
}

static inline int
eina_spans_intersect(int c1, int l1, int c2, int l2)
{
	return (!(((c2 + l2) <= c1) || (c2 >= (c1 + l1))));
}

static inline Eina_Bool
eina_rectangle_is_empty(const Eina_Rectangle *r)
{
   return !eina_rectangle_is_valid(r);
}

static inline void
eina_rectangle_coords_from(Eina_Rectangle *r, int x, int y, int w, int h)
{
	r->x = x;
	r->y = y;
	r->w = w;
	r->h = h;
}

static inline Eina_Bool
eina_rectangle_equal(const Eina_Rectangle *rect1, const Eina_Rectangle *rect2)
{
   return ((rect1->x == rect2->x) && (rect1->y == rect2->y) &&
           (rect1->w == rect2->w) && (rect1->h == rect2->h));
}

static inline Eina_Bool
eina_rectangles_intersect(const Eina_Rectangle *r1, const Eina_Rectangle *r2)
{
	return (eina_spans_intersect(r1->x, r1->w, r2->x, r2->w) && eina_spans_intersect(r1->y, r1->h, r2->y, r2->h)) ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Bool
eina_rectangle_xcoord_inside(const Eina_Rectangle *r, int x)
{
	return ((x >= r->x) && (x < (r->x + r->w))) ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Bool
eina_rectangle_ycoord_inside(const Eina_Rectangle *r, int y)
{
	return ((y >= r->y) && (y < (r->y + r->h))) ? EINA_TRUE : EINA_FALSE;
}

static inline Eina_Bool
eina_rectangle_coords_inside(const Eina_Rectangle *r, int x, int y)
{
	return (eina_rectangle_xcoord_inside(r, x) && eina_rectangle_ycoord_inside(r, y)) ? EINA_TRUE : EINA_FALSE;
}

static inline void
eina_rectangle_union(Eina_Rectangle *dst, const Eina_Rectangle *src)
{
	/* left */
	if (dst->x > src->x)
	{
		dst->w += dst->x - src->x;
		dst->x = src->x;
	}
	/* right */
	if ((dst->x + dst->w) < (src->x + src->w))
		dst->w = src->x + src->w - dst->x;
	/* top */
	if (dst->y > src->y)
	{
		dst->h += dst->y - src->y;
		dst->y = src->y;
	}
	/* bottom */
	if ((dst->y + dst->h) < (src->y + src->h))
		dst->h = src->y + src->h - dst->y;
}

static inline Eina_Bool
eina_rectangle_intersection(Eina_Rectangle *dst, const Eina_Rectangle *src)
{
   if (eina_rectangle_is_valid(dst) && eina_rectangle_is_valid(src) &&
       eina_rectangles_intersect(dst, src))
     {
        /* left */
        if (dst->x < src->x)
          {
           dst->w += dst->x - src->x;
           dst->x = src->x;
           if (dst->w < 0) dst->w = 0;
          }
        /* right */
        if ((dst->x + dst->w) > (src->x + src->w))
          dst->w = src->x + src->w - dst->x;
        /* top */
        if (dst->y < src->y)
          {
             dst->h += dst->y - src->y;
             dst->y = src->y;
             if (dst->h < 0) dst->h = 0;
          }
        /* bottom */
        if ((dst->y + dst->h) > (src->y + src->h))
          dst->h = src->y + src->h - dst->y;

        if ((dst->w == 0) || (dst->h == 0)) return EINA_FALSE;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static inline void
eina_rectangle_rescale_in(const Eina_Rectangle *out, const Eina_Rectangle *in, Eina_Rectangle *res)
{
	res->x = in->x - out->x;
	res->y = in->y - out->y;
	res->w = in->w;
	res->h = in->h;
}

static inline void
eina_rectangle_rescale_out(const Eina_Rectangle *out, const Eina_Rectangle *in, Eina_Rectangle *res)
{
	res->x = out->x + in->x;
	res->y = out->y + in->y;
	res->w = out->w;
	res->h = out->h;
}

static inline int
eina_rectangle_max_x(Eina_Rectangle *thiz)
{
	return thiz->x + thiz->w;
}

static inline int
eina_rectangle_max_y(Eina_Rectangle *thiz)
{
	return thiz->y + thiz->h;
}

static inline Eina_Bool
eina_rectangle_x_cut(Eina_Rectangle *thiz, Eina_Rectangle *slice, Eina_Rectangle *leftover, int amount)
{
	Eina_Rectangle tmp1, tmp2;
	if (amount > thiz->w)
		return EINA_FALSE;
	eina_rectangle_coords_from(&tmp1, thiz->x, thiz->y, amount, thiz->h);
	eina_rectangle_coords_from(&tmp2, thiz->x + amount, thiz->y, thiz->w - amount, thiz->h);
	if (slice) *slice = tmp1;
	if (leftover) *leftover = tmp2;
	return EINA_TRUE;
}

static inline Eina_Bool
eina_rectangle_y_cut(Eina_Rectangle *thiz, Eina_Rectangle *slice, Eina_Rectangle *leftover, int amount)
{
	Eina_Rectangle tmp1, tmp2;
	if (amount > thiz->h)
		return EINA_FALSE;
	eina_rectangle_coords_from(&tmp1, thiz->x, thiz->y, thiz->w, amount);
	eina_rectangle_coords_from(&tmp2, thiz->x, thiz->y + amount, thiz->w, thiz->h - amount);
	if (slice) *slice = tmp1;
	if (leftover) *leftover = tmp2;
	return EINA_TRUE;
}

static inline Eina_Bool
eina_rectangle_width_cut(Eina_Rectangle *thiz, Eina_Rectangle *slice, Eina_Rectangle *leftover, int amount)
{
	Eina_Rectangle tmp1, tmp2;
	if (thiz->w - amount < 0)
		return EINA_FALSE;
	eina_rectangle_coords_from(&tmp1, thiz->x + (thiz->w - amount), thiz->y, amount, thiz->h);
	eina_rectangle_coords_from(&tmp2, thiz->x, thiz->y, thiz->w - amount, thiz->h);
	if (slice) *slice = tmp1;
	if (leftover) *leftover = tmp2;
	return EINA_TRUE;
}

static inline Eina_Bool
eina_rectangle_height_cut(Eina_Rectangle *thiz, Eina_Rectangle *slice, Eina_Rectangle *leftover, int amount)
{
	Eina_Rectangle tmp1, tmp2;
	if (thiz->h - amount < 0)
		return EINA_FALSE;
	eina_rectangle_coords_from(&tmp1, thiz->x, thiz->y + (thiz->h - amount), thiz->w, amount);
	eina_rectangle_coords_from(&tmp2, thiz->x, thiz->y, thiz->w, thiz->h - amount);
	if (slice) *slice = tmp1;
	if (leftover) *leftover = tmp2;
	return EINA_TRUE;
}

static inline Eina_Bool
eina_rectangle_subtract(Eina_Rectangle *thiz, Eina_Rectangle *other, Eina_Rectangle out[4])
{
   Eina_Rectangle intersection;
   Eina_Rectangle leftover = EINA_RECTANGLE_INIT;
   Eina_Rectangle tmp;
   int cut = 0;

   if (eina_rectangle_is_valid(thiz))
     {
        eina_rectangle_coords_from(&out[0], 0, 0, 0, 0);
        eina_rectangle_coords_from(&out[1], 0, 0, 0, 0);
        eina_rectangle_coords_from(&out[2], 0, 0, 0, 0);
        eina_rectangle_coords_from(&out[3], 0, 0, 0, 0);
        intersection = *thiz;
        if (!eina_rectangle_intersection(&intersection, other))
          {
             out[0] = *thiz;
             return EINA_TRUE;
          }

        /* cut in height */
          {
             cut = thiz->h - (intersection.y - thiz->y);
             if (cut > thiz->h) cut = thiz->h;
             eina_rectangle_height_cut(thiz, &leftover, &out[0], cut);
          }
        /* cut in y */
        tmp = leftover;
        if (eina_rectangle_intersection(&tmp, &intersection))
          {
             cut = leftover.h - (eina_rectangle_max_y(&leftover) - eina_rectangle_max_y(&tmp));
             if (cut > leftover.h) cut = leftover.h;
             eina_rectangle_y_cut(&leftover, &leftover, &out[1], cut);
          }
        /* cut in width */
        tmp = leftover;
        if (eina_rectangle_intersection(&tmp, &intersection))
          {
             cut = leftover.w - (tmp.x - leftover.x);
             if (cut > leftover.w) cut = leftover.w;
             eina_rectangle_width_cut(&leftover, &leftover, &out[2], cut);
          }
        /* cut in x */
        tmp = leftover;
        if (eina_rectangle_intersection(&tmp, &intersection))
          {
             cut = leftover.w - (eina_rectangle_max_x(&leftover) - eina_rectangle_max_x(&tmp));
             if (cut > leftover.w) cut = leftover.w;
             eina_rectangle_x_cut(&leftover, &leftover, &out[3], cut);
          }
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

#endif
