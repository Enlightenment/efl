#ifndef EINA_RECTANGLE_H_
#define EINA_RECTANGLE_H_

/**
 * @file
 * @{
 */

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

/**
 * To be documented
 * FIXME: To be fixed
 * Is it needed??
 */
static inline int
eina_spans_intersect(int c1, int l1, int c2, int l2)
{
	return (!(((c2 + l2) <= c1) || (c2 >= (c1 + l1))));
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_Bool
eina_rectangle_is_empty(Eina_Rectangle *r)
{
	return ((r->w < 1) || (r->h < 1));
}

/**
 * To be documented
 * FIXME: To be fixed
 */
static inline void
eina_rectangle_coords_from(Eina_Rectangle *r, int x, int y, int w, int h)
{
	r->x = x;
	r->y = y;
	r->w = w;
	r->h = h;
}

/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_Bool
eina_rectangles_intersect(Eina_Rectangle *r1, Eina_Rectangle *r2)
{
	return (eina_spans_intersect(r1->x, r1->w, r2->x, r2->w) && eina_spans_intersect(r1->y, r1->h, r2->y, r2->h));
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_Bool
eina_rectangle_hspan_inside(Eina_Rectangle *r, int x, int l)
{

}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_Bool
eina_rectangle_vspan_inside(Eina_Rectangle *r, int y, int l)
{

}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_Bool
eina_rectangle_xcoord_inside(Eina_Rectangle *r, int x)
{
	return ((x >= r->x) && (x < (r->x + r->w)));
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_Bool
eina_rectangle_ycoord_inside(Eina_Rectangle *r, int y)
{
	return ((y >= r->y) && (y < (r->y + r->h)));
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_Bool
eina_rectangle_coords_inside(Eina_Rectangle *r, int x, int y)
{
	return (eina_rectangle_xcoord_inside(r, x) && eina_rectangle_ycoord_inside(r, y));
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline void
eina_rectangle_union(Eina_Rectangle *dst, Eina_Rectangle *src)
{
	/* left */
	if (dst->x > src->x)
	{
		dst->w += dst->x - src->x;
		dst->x = src->x;
	}
	/* right */
	if ((dst->x + dst->w) < (src->x + src->w))
		dst->w = src->x + src->w;
	/* top */
	if (dst->y > src->y)
	{
		dst->h += dst->y - src->y;
		dst->y = src->y;
	}
	/* bottom */
	if ((dst->y + dst->h) < (src->y + src->h))
		dst->h = src->y + src->h;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline Eina_Bool
eina_rectangle_intersection(Eina_Rectangle *dst, Eina_Rectangle *src)
{
	if (!(eina_rectangles_intersect(dst, src)))
		return EINA_FALSE;

	/* left */
	if (dst->x < src->x)
	{
		dst->w += dst->x - src->x;
		dst->x = src->x;
		if (dst->w < 0)
			dst->w = 0;
	}
	/* right */
	if ((dst->x + dst->w) > (src->x + src->w))
		dst->w = src->x + src->w - dst->x;
	/* top */
	if (dst->y < src->y)
	{
		dst->h += dst->y - src->y;
		dst->y = src->y;
		if (dst->h < 0)
			dst->h = 0;
	}
	/* bottom */
	if ((dst->y + dst->h) > (src->y + src->h))
		dst->h = src->y + src->h - dst->y;

	return EINA_TRUE;
}

/**
 * Rescale the coordinates from @in as if it where relative to @out
 */
static inline void
eina_rectangle_rescale_in(Eina_Rectangle *out, Eina_Rectangle *in, Eina_Rectangle *res)
{
	res->x = in->x - out->x;
	res->y = in->y - out->y;
	res->w = in->w;
	res->h = in->h;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
static inline void
eina_rectangle_rescale_out(Eina_Rectangle *out, Eina_Rectangle *in, Eina_Rectangle *res)
{
	res->x = out->x + in->x;
	res->y = out->y + in->y;
	res->w = out->w;
	res->h = out->h;
}

/** @} */

#endif /*_ENESIM_RECTANGLE_H_*/
