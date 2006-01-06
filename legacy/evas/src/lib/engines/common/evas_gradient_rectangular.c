#include "evas_common.h"
#include <math.h>


typedef struct _Rectangular_Data   Rectangular_Data;
struct _Rectangular_Data
{
   int    sx, sy, s;
   float  r0;
};

static Rectangular_Data  rectangular_data = {32, 32, 32, 0.0};


static void 
rectangular_setup_geom(RGBA_Gradient *gr, int spread);

static int 
rectangular_has_alpha(RGBA_Gradient *gr, int spread);

static int 
rectangular_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Span 
rectangular_get_span_func(RGBA_Gradient *gr, int spread, unsigned char aa);

static RGBA_Gradient_Type  rectangular = {"rectangular", &rectangular_data, rectangular_setup_geom, rectangular_has_alpha, rectangular_get_map_len, rectangular_get_span_func};


/** internal functions **/

static void
rectangular_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_reflect(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

RGBA_Gradient_Type  *
evas_common_gradient_rectangular_get(void)
{
    return &rectangular;
}

static void
rectangular_setup_geom(RGBA_Gradient *gr, int spread)
{
   int    err = 1;
   char   *s, *p, key[256];
   float  r0 = 0.0;

   if (!gr || (gr->type.geometer != &rectangular)) return;

   rectangular_data.sx = gr->fill.w;
   rectangular_data.sy = gr->fill.h;
   rectangular_data.s = rectangular_data.sx;
   if (rectangular_data.sy > rectangular_data.sx)
	rectangular_data.s = rectangular_data.sy;
   rectangular_data.r0 = 0.0;

   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;
   
   p = s;
   while ((p = evas_common_gradient_get_key_fval(p, key, &r0)))
     {
	if (!strcmp(key, "inner_radius"))
	    err = 0;
	else
	  {
	    err = 1;
	    goto done;
	  }
     }
   done:
   if (!err)
     {
	if (r0 < 0.0) r0 = 0.0;
	if (r0 > 1.0) r0 = 1.0;
	rectangular_data.r0 = r0;
     }
   free(s);
}


static int
rectangular_has_alpha(RGBA_Gradient *gr, int spread)
{
   if (!gr || (gr->type.geometer != &rectangular)) return 0;
   if (gr->map.has_alpha)
	return 1;
   if (rectangular_data.r0 > 0.0)
	return 1;
   if (spread == _EVAS_TEXTURE_RESTRICT)
	return 1;
   return 0;
}

static int
rectangular_get_map_len(RGBA_Gradient *gr, int spread)
{
   int l;

   if (!gr || (gr->type.geometer != &rectangular)) return 0;
   l = rectangular_data.s;
   l -= (int)(l * rectangular_data.r0);
   return l;
}

static Gfx_Func_Gradient_Span
rectangular_get_span_func(RGBA_Gradient *gr, int spread, unsigned char aa)
{
   Gfx_Func_Gradient_Span  sfunc = NULL;

   if (!gr || (gr->type.geometer != &rectangular)) return sfunc;
   switch (spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	    sfunc = rectangular_reflect_aa;
	 else
	    sfunc = rectangular_reflect;
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	    sfunc = rectangular_repeat_aa;
	 else
	    sfunc = rectangular_repeat;
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	{
	 if (aa)
	    sfunc = rectangular_restrict_aa;
	 else
	    sfunc = rectangular_restrict;
	}
      break;
      default:
	sfunc = rectangular_reflect;
      break;
     }
   return sfunc;
}

static void
rectangular_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;

   int  xx, yy, rr0;

   if (gdata->sx != gdata->s)
     {
	axx = (gdata->s * axx) / gdata->sx;
	axy = (gdata->s * axy) / gdata->sx;
     }
   if (gdata->sy != gdata->s)
     {
	ayy = (gdata->s * ayy) / gdata->sy;
	ayx = (gdata->s * ayx) / gdata->sy;
     }

   xx = (axx * x) + (axy * y);
   yy = (ayx * x) + (ayy * y);
   rr0 = gdata->r0 * gdata->s;
   rr0 <<= 16;

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;
	if (l == 0)
	  {
	    int a = 1 + (ll >> 8);

	    *dst = map[0];
	    A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	  }
	if (l > 0)
	  {
	    if (l >= map_len)
	      {
	        int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		    l = map_len - l - 1;
	      }
	   *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_reflect(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;

   int  xx, yy, rr0;

   if (gdata->sx != gdata->s)
     {
	axx = (gdata->s * axx) / gdata->sx;
	axy = (gdata->s * axy) / gdata->sx;
     }
   if (gdata->sy != gdata->s)
     {
	ayy = (gdata->s * ayy) / gdata->sy;
	ayx = (gdata->s * ayx) / gdata->sy;
     }

   xx = (axx * x) + (axy * y);
   yy = (ayx * x) + (ayy * y);
   rr0 = gdata->r0 * gdata->s;
   rr0 <<= 16;

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if (l >= 0)
	  {
	    if (l >= map_len)
	      {
	        int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		    l = map_len - l - 1;
	      }
	    *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;

   int xx, yy, rr0;

   if (gdata->sx != gdata->s)
     {
	axx = (gdata->s * axx) / gdata->sx;
	axy = (gdata->s * axy) / gdata->sx;
     }
   if (gdata->sy != gdata->s)
     {
	ayy = (gdata->s * ayy) / gdata->sy;
	ayx = (gdata->s * ayx) / gdata->sy;
     }

   xx = (axx * x) + (axy * y);
   yy = (ayx * x) + (ayy * y);
   rr0 = gdata->r0 * gdata->s;
   rr0 <<= 16;

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;
	 if (l == 0)
	   {
	     int a = 1 + (ll >> 8);

	     *dst = map[0];
	     A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	   }
	if (l > 0)
	  {
	    if (l >= map_len)
		l = (l % map_len);
	    *dst = map[l];
	    if (l == 0)
	      {
		int a = 256 - ((ll - ((ll >> 16) << 16)) >> 8);
		DATA32  *c = map + (map_len - 1);
		
		*dst += ARGB_JOIN((a * (A_VAL(c) - A_VAL(dst))) >> 8,
		                  (a * (R_VAL(c) - R_VAL(dst))) >> 8,
		                  (a * (G_VAL(c) - G_VAL(dst))) >> 8,
		                  (a * (B_VAL(c) - B_VAL(dst))) >> 8);
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_repeat(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;

   int  xx, yy, rr0;

   if (gdata->sx != gdata->s)
     {
	axx = (gdata->s * axx) / gdata->sx;
	axy = (gdata->s * axy) / gdata->sx;
     }
   if (gdata->sy != gdata->s)
     {
	ayy = (gdata->s * ayy) / gdata->sy;
	ayx = (gdata->s * ayx) / gdata->sy;
     }

   xx = (axx * x) + (axy * y);
   yy = (ayx * x) + (ayy * y);
   rr0 = gdata->r0 * gdata->s;
   rr0 <<= 16;

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if (l >= 0)
	  {
	   if (l >= map_len)
	       l = (l % map_len);
	   *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;

   int  xx, yy, rr0;

   if (gdata->sx != gdata->s)
     {
	axx = (gdata->s * axx) / gdata->sx;
	axy = (gdata->s * axy) / gdata->sx;
     }
   if (gdata->sy != gdata->s)
     {
	ayy = (gdata->s * ayy) / gdata->sy;
	ayx = (gdata->s * ayx) / gdata->sy;
     }

   xx = (axx * x) + (axy * y);
   yy = (ayx * x) + (ayy * y);
   rr0 = gdata->r0 * gdata->s;
   rr0 <<= 16;

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	   *dst = map[l];
	   if (l == (map_len - 1))
	     {
		int a = 256 - ((ll - (l << 16)) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	     }
	   if ((l == 0) && rr0)
	     {
		int a = 1 + (ll >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	     }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;

   int  xx, yy, rr0;

   if (gdata->sx != gdata->s)
     {
	axx = (gdata->s * axx) / gdata->sx;
	axy = (gdata->s * axy) / gdata->sx;
     }
   if (gdata->sy != gdata->s)
     {
	ayy = (gdata->s * ayy) / gdata->sy;
	ayx = (gdata->s * ayx) / gdata->sy;
     }

   xx = (axx * x) + (axy * y);
   yy = (ayx * x) + (ayy * y);
   rr0 = gdata->r0 * gdata->s;
   rr0 <<= 16;

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if ((unsigned)l < map_len)
	   *dst = map[l];
	dst++;  xx += axx;  yy += ayx;
     }
}

