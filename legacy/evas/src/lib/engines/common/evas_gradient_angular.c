#include "evas_common.h"
#include <math.h>


typedef struct _Angular_Data   Angular_Data;
struct _Angular_Data
{
   int    sx, sy, s;
   float  an, cy;
};

static Angular_Data  angular_data = {32, 32, 32, -1.0, 1.0};


static void 
angular_setup_geom(RGBA_Gradient *gr, int spread);

static int 
angular_has_alpha(RGBA_Gradient *gr, int spread);

static int 
angular_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Span 
angular_get_span_func(RGBA_Gradient *gr, int spread, unsigned char aa);

static RGBA_Gradient_Type  angular = {"angular", &angular_data, angular_setup_geom, angular_has_alpha, angular_get_map_len, angular_get_span_func};


/** internal functions **/

static void
angular_reflect_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);
static void
angular_restrict_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);



RGBA_Gradient_Type  *
evas_common_gradient_angular_get(void)
{
    return &angular;
}

static void
angular_setup_geom(RGBA_Gradient *gr, int spread)
{
   int    err = 1;
   char   *s, *p, key[256];
   float  val, an = -1.0, cy = 1.0;

   if (!gr || (gr->type.geometer != &angular)) return;

   angular_data.sx = gr->fill.w;
   angular_data.sy = gr->fill.h;
   angular_data.s = angular_data.sx;
   if (angular_data.sy > angular_data.sx)
	angular_data.s = angular_data.sy;
   angular_data.an = -1.0;
   angular_data.cy = 1.0;

   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   p = s;
   while (p = evas_common_gradient_get_key_fval(p, key, &val))
     {
	if (!strcmp(key, "annulus"))
	  {
	    err = 0;
	    an = val;
	  }
	else if (!strcmp(key, "wrap"))
	  {
	    err = 0;
	    cy = val;
	  }
	else
	  {
	    err = 1;
	    goto done;
	  }
     }
   done:
   if (!err)
     {
	if (an < 0.0) an = 0.0;
	if (an > 1.0) an = 1.0;
	angular_data.an = an;
	if (cy < 0.0) cy = 0.0;
	if (cy > 1.0) cy = 1.0;
	angular_data.cy = cy;
     }
   free(s);
}


static int
angular_has_alpha(RGBA_Gradient *gr, int spread)
{
   if (!gr || (gr->type.geometer != &angular)) return 0;
   if (gr->map.has_alpha)
	return 1;
   if ((int)angular_data.an >= 0)
	return 1;
   if ((spread == _EVAS_TEXTURE_RESTRICT) && (angular_data.cy < 1.0))
	return 1;
   return 0;
}

static int
angular_get_map_len(RGBA_Gradient *gr, int spread)
{
   int l;

   if (!gr || (gr->type.geometer != &angular)) return 0;
   l = (2 * M_PI) * angular_data.s * angular_data.cy;
   return l;
}

static Gfx_Func_Gradient_Span
angular_get_span_func(RGBA_Gradient *gr, int spread, unsigned char aa)
{
   Gfx_Func_Gradient_Span  sfunc = NULL;

   if (!gr || (gr->type.geometer != &angular)) return sfunc;
   switch (spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	   {
	    if ((int)angular_data.an >= 0)
	       sfunc = angular_reflect_aa_annulus;
	    else
	       sfunc = angular_reflect;
	   }
	 else
	   {
	    if ((int)angular_data.an >= 0)
	       sfunc = angular_reflect_annulus;
	    else
	       sfunc = angular_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	   {
	    if ((int)angular_data.an >= 0)
	       sfunc = angular_repeat_aa_annulus;
	    else
	       sfunc = angular_repeat_aa;
	   }
	 else
	   {
	    if ((int)angular_data.an >= 0)
	       sfunc = angular_repeat_annulus;
	    else
	       sfunc = angular_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	{
	 if (aa)
	   {
	    if ((int)angular_data.an >= 0)
	       sfunc = angular_restrict_aa_annulus;
	    else
	       sfunc = angular_restrict_aa;
	   }
	 else
	   {
	    if ((int)angular_data.an >= 0)
	       sfunc = angular_restrict_annulus;
	    else
	       sfunc = angular_restrict;
	   }
	}
      break;
      default:
	sfunc = angular_reflect;
      break;
     }
   return sfunc;
}

static void
angular_reflect_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;
   int  r1 = gdata->s, r0 = gdata->an * r1;
   int  rr0 = r0 << 16, rr1 = r1 << 16;

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

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx)), l = (ll >> 16);
	    
	    if (l >= map_len)
	      {
		int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		    l = map_len - l - 1;
	      }
	    *dst = map[l];
	    if (r == r0)
	      {
		int a = 1 + ((rr - rr0) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	    if (r == r1)
	      {
		int a = 256 - ((rr - rr1) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;

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

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx)), l = (ll >> 16);
	    
	if (l >= map_len)
	  {
	    int  m = (l % (2 * map_len));

	    l = (l % map_len);
	    if (m >= map_len)
		l = map_len - l - 1;
	  }
	*dst = map[l];
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;
   int  r1 = gdata->s, r0 = gdata->an * r1;

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

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI +  atan2(yy, xx)), l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
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
angular_repeat_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;
   int  r1 = gdata->s, r0 = gdata->an * r1;
   int  rr0 = r0 << 16, rr1 = r1 << 16;

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

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx)), l = ll >> 16;
	  
	    if (l >= map_len)
		l = (l % map_len);
	    *dst = map[l];
	    if (r == r0)
	      {
		int a = 1 + ((rr - rr0) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	    if (r == r1)
	      {
		int a = 256 - ((rr - rr1) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }

	    if (l == (map_len - 1))
	      {
		int a = 1 + (ll >> 8);

		*dst += ARGB_JOIN((a * (A_VAL(map) - A_VAL(dst))) >> 8,
		                  (a * (R_VAL(map) - R_VAL(dst))) >> 8,
		                  (a * (G_VAL(map) - G_VAL(dst))) >> 8,
		                  (a * (B_VAL(map) - B_VAL(dst))) >> 8);
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;

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

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI + atan2(yy, xx)), l = ll >> 16;
	  
	if (l >= map_len)
	    l = (l % map_len);
	*dst = map[l];
	if (l == (map_len - 1))
	  {
	    int a = 1 + (ll >> 8);

	    *dst += ARGB_JOIN((a * (A_VAL(map) - A_VAL(dst))) >> 8,
		              (a * (R_VAL(map) - R_VAL(dst))) >> 8,
		              (a * (G_VAL(map) - G_VAL(dst))) >> 8,
		              (a * (B_VAL(map) - B_VAL(dst))) >> 8);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;
   int  r1 = gdata->s, r0 = gdata->an * r1;

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

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ((r >= r0) && (r <= r1))
	  {
	   int  ll = ss * (M_PI +  atan2(yy, xx)), l = ll >> 16;

	   l += (ll - (l << 16)) >> 15;
	   if (l >= map_len)
	       l = (l % map_len);
	   *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;

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

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI +  atan2(yy, xx)), l = ll >> 16;

	l += (ll - (l << 16)) >> 15;
	if (l >= map_len)
	    l = (l % map_len);
	*dst = map[l];
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;
   int  r1 = gdata->s, r0 = gdata->an * r1;
   int  rr0 = r0 << 16, rr1 = r1 << 16;

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

   while (dst < dst_end)
     {
	int rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	   int  ll = ss * (M_PI +  atan2(yy, xx)), l = ll >> 16;
	   
	   if (l < map_len)
	     {
	       *dst = map[l];
	       if (r == r0)
	         {
		   int a = 1 + ((rr - rr0) >> 8);

		   A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	         }
	       if (r == r1)
	         {
		   int a = 256 - ((rr - rr1) >> 8);

		   A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	         }
	       if (l == (map_len - 1))
	         {
		   int a = 256 - ((ll - (l << 16)) >> 8);

		   A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	         }
	       if (l == 0)
	         {
		   int a = 1 + (ll >> 8);

		   A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	         }
	     }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_aa(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;

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

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI +  atan2(yy, xx)), l = ll >> 16;
	   
	*dst = 0;
	if (l < map_len)
	  {
	    *dst = map[l];
	    if (l == (map_len - 1))
	      {
		int a = 256 - ((ll - (l << 16)) >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	    if (l == 0)
	      {
		int a = 1 + (ll >> 8);

		A_VAL(dst) = (A_VAL(dst) * a) >> 8;
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_annulus(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;
   int  r1 = gdata->s, r0 = gdata->an * r1;

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

   while (dst < dst_end)
     {
	int  rr = hypot(xx, yy), r = (rr >> 16);

	*dst = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI +  atan2(yy, xx)), l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;  
	    if (l < map_len)
		*dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict(DATA32 *map, int map_len, DATA32 *dst, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;

   int  xx, yy;
   int  ss = (gdata->s) << 16;

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

   while (dst < dst_end)
     {
	int  ll = ss * (M_PI +  atan2(yy, xx)), l = ll >> 16;
	   
	*dst = 0;
	l += (ll - (l << 16)) >> 15;
	if (l < map_len)
	    *dst = map[l];
	dst++;  xx += axx;  yy += ayx;
     }
}

