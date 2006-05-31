#include "evas_common.h"
#include <math.h>


typedef struct _Angular_Data   Angular_Data;
struct _Angular_Data
{
   int    sx, sy, s;
   float  an, cy;
   float  off;
};

static Angular_Data  angular_data = {32, 32, 32, -1.0, 1.0, 0.0};


static void 
angular_setup_geom(RGBA_Gradient *gr, int spread);

static int 
angular_has_alpha(RGBA_Gradient *gr, int spread, int op);

static int 
angular_has_mask(RGBA_Gradient *gr, int spread, int op);

static int 
angular_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Fill 
angular_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa);

static RGBA_Gradient_Type  angular = {"angular", &angular_data, angular_setup_geom, angular_has_alpha, angular_has_mask, angular_get_map_len, angular_get_fill_func};


/** internal functions **/

static void
angular_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_reflect_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_repeat_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_reflect_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_restrict_repeat_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
angular_pad_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
   angular_data.off = gr->range_offset;

   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   p = s;
   while ((p = evas_common_gradient_get_key_fval(p, key, &val)))
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
	    break;
	  }
     }
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
angular_has_alpha(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &angular)) return 0;
   if (gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if ((int)angular_data.an > 0)
	return 1;
   if ( ((spread == _EVAS_TEXTURE_RESTRICT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REPEAT))
   	 && (angular_data.cy < 1.0) )
	return 1;
   return 0;
}

static int
angular_has_mask(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &angular)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if ((int)angular_data.an > 0)
	    return 1;
	if ( ((spread == _EVAS_TEXTURE_RESTRICT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REPEAT))
	      && (angular_data.cy < 1.0) )
	    return 1;
     }
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

static Gfx_Func_Gradient_Fill
angular_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa)
{
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &angular)) return sfunc;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_reflect_aa_masked_annulus;
		else
		   sfunc = angular_reflect_aa_annulus;
	      }
	    else
	       sfunc = angular_reflect_aa;
	   }
	 else
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_reflect_masked_annulus;
		else
		   sfunc = angular_reflect_annulus;
	      }
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
	      {
		if (masked_op)
		   sfunc = angular_repeat_aa_masked_annulus;
		else
		   sfunc = angular_repeat_aa_annulus;
	      }
	    else
	       sfunc = angular_repeat_aa;
	   }
	 else
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_repeat_masked_annulus;
		else
		   sfunc = angular_repeat_annulus;
	      }
	    else
	       sfunc = angular_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	 angular_data.off = 0;
      case _EVAS_TEXTURE_RESTRICT_REFLECT:
	{
	 if (aa)
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_reflect_aa_masked_annulus;
		else
		   sfunc = angular_restrict_reflect_aa_annulus;
	      }
	    else if (angular_data.cy < 1.0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_reflect_aa_masked;
		else
		   sfunc = angular_restrict_reflect_aa;
	      }
	    else
	       sfunc = angular_restrict_reflect_aa;
	   }
	 else
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_reflect_masked_annulus;
		else
		   sfunc = angular_restrict_reflect_annulus;
	      }
	    else if (angular_data.cy < 1.0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_reflect_masked;
		else
		   sfunc = angular_restrict_reflect;
	      }
	    else
	       sfunc = angular_restrict_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT_REPEAT:
	{
	 if (aa)
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_repeat_aa_masked_annulus;
		else
		   sfunc = angular_restrict_repeat_aa_annulus;
	      }
	    else if (angular_data.cy < 1.0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_repeat_aa_masked;
		else
		   sfunc = angular_restrict_repeat_aa;
	      }
	    else
	       sfunc = angular_restrict_repeat_aa;
	   }
	 else
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_repeat_masked_annulus;
		else
		   sfunc = angular_restrict_repeat_annulus;
	      }
	    else if (angular_data.cy < 1.0)
	      {
		if (masked_op)
		   sfunc = angular_restrict_repeat_masked;
		else
		   sfunc = angular_restrict_repeat;
	      }
	    else
	       sfunc = angular_restrict_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_PAD:
	{
	 if (aa)
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_pad_aa_masked_annulus;
		else
		   sfunc = angular_pad_aa_annulus;
	      }
	    else
	       sfunc = angular_pad_aa;
	   }
	 else
	   {
	    if ((int)angular_data.an >= 0)
	      {
		if (masked_op)
		   sfunc = angular_pad_masked_annulus;
		else
		   sfunc = angular_pad_annulus;
	      }
	    else
	       sfunc = angular_pad;
	   }
	}
      default:
	sfunc = angular_reflect_aa;
      break;
     }
   return sfunc;
}

static void
angular_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = (ll >> 16);

	l += ((ll - (l << 16)) >> 15) + off;
	if (l < 0) l = -l;
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
angular_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = (ll >> 16), lp;
	DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	lp = l + off;
	if (lp < 0) { lp = -lp;  a = 257 - a; }
	if (lp >= map_len)
	  {
	    int  m = (lp % (2 * map_len));

	    lp = (lp % map_len);
	    if (m >= map_len)
		{ lp = map_len - lp - 1;  a = 257 - a; }
	  }
	*dst = map[lp];
	if (lp + 1 < map_len)
	   *dst = INTERP_256(a, map[lp + 1], *dst);
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (map_len - 1);

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
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = (ll >> 16);

	    l += ((ll - (l << 16)) >> 15) + off;
	    if (l < 0) l = -l;
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
angular_reflect_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (map_len - 1);

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
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = (ll >> 16), lp;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
	    if (lp >= map_len)
	      {
		int  m = (lp % (2 * map_len));

		lp = (lp % map_len);
		if (m >= map_len)
		  { lp = map_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = map[lp];
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (r == r0)
	      {
		a = 1 + ((rr - rr0) >> 8);

		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	    if (r == r1)
	      {
		a = 256 - ((rr - rr1) >> 8);

		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx)), l = ll >> 16;

	    l += ((ll - (l << 16)) >> 15) + off;
	    if (l < 0) l = -l;
	    if (l >= map_len)
	      {
		int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		    l = map_len - l - 1;
	      }
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_reflect_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = (ll >> 16), lp;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
	    if (lp >= map_len)
	      {
		int  m = (lp % (2 * map_len));

		lp = (lp % map_len);
		if (m >= map_len)
		  { lp = map_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (r == r0)
		*mask = ((rr - rr0) >> 8);
	    if (r == r1)
		*mask = 255 - ((rr - rr1) >> 8);

	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16;

	l += ((ll - (l << 16)) >> 15) + off;
	l = l % map_len;
	if (l < 0)
	   l += map_len;
	*dst = map[l];
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = (ll >> 16), lp;
	DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	lp = l + off;
	lp = lp % map_len;
	if (lp < 0)
	   lp += map_len;
	*dst = map[lp];
	if (lp + 1 < map_len)
	   *dst = INTERP_256(a, map[lp + 1], *dst);
	if (lp == (map_len - 1))
	   *dst = INTERP_256(a, map[0], *dst);
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (map_len - 1);

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
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += ((ll - (l << 16)) >> 15) + off;
	    l = l % map_len;
	    if (l < 0)
		l += map_len;
	   *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (map_len - 1);

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
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16, lp;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = lp % map_len;
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == (map_len - 1))
		*dst = INTERP_256(a, map[0], *dst);
	    if (r == r0)
	      {
		a = 1 + ((rr - rr0) >> 8);

		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	    if (r == r1)
	      {
		a = 256 - ((rr - rr1) >> 8);

		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }

	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += ((ll - (l << 16)) >> 15) + off;
	    l = l % map_len;
	    if (l < 0)
		l += map_len;
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_repeat_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	if ((r >= r0) && (r <= r1))
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16, lp;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = lp % map_len;
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == (map_len - 1))
		*dst = INTERP_256(a, map[0], *dst);
	    if (r == r0)
		*mask = ((rr - rr0) >> 8);
	    if (r == r1)
		*mask = 255 - ((rr - rr1) >> 8);
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16;

	*dst = 0;
	l += ((ll - (l << 16)) >> 15);
	if (l < map_len)
	  {
	    l += off;
	    if (l < 0) l = -l;
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
angular_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16, lp;

	*dst = 0;
	if (l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
	    if (lp >= map_len)
	      {
		int  m = (lp % (2 * map_len));

		lp = (lp % map_len);
		if (m >= map_len)
		  { lp = map_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = map[lp];
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (l == (map_len - 1))
		*dst = MUL_A_256(257 - a0, *dst) + (*dst & 0x00ffffff);
	    if (l == 0)
		*dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	l += (ll - (l << 16)) >> 15;
	if (l < map_len)
	  {
	    l += off;
	    if (l < 0) l = -l;
	    if (l >= map_len)
	      {
		int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		   l = map_len - l - 1;
	      }
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI +  atan2(yy, xx));
	int  l = ll >> 16, lp;

	*dst = 0;  *mask = 0;
	if (l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
	    if (lp >= map_len)
	      {
		int  m = (lp % (2 * map_len));

		lp = (lp % map_len);
		if (m >= map_len)
		  { lp = map_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (l == (map_len - 1))
		*mask = 255 - a0;
	    if (l == 0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (map_len - 1);

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
	    int  ll = ss * (M_PI +  atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l < map_len)
	      {
		l += off;
		if (l < 0) l = -l;
		if (l >= map_len)
	          {
		    int  m = (l % (2 * map_len));

		    l = (l % map_len);
		    if (m >= map_len)
			l = map_len - l - 1;
	          }
		*dst = map[l];
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (map_len - 1);

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
	   int  ll = ss * (M_PI + atan2(yy, xx));
	   int  l = ll >> 16, lp;

	   if (l < map_len)
	     {
		DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

		lp = l + off;
		if (lp < 0) { lp = -lp;  257 - a; }
		if (lp >= map_len)
	          {
		    int  m = (lp % (2 * map_len));

		    lp = (lp % map_len);
		    if (m >= map_len)
			{ lp = map_len - lp - 1;  257 - a; }
	          }
	       *dst = map[lp];
	       if (lp + 1 < map_len)
		  *dst = INTERP_256(a, map[lp + 1], *dst);
	       if (l == (map_len - 1))
		   *dst = MUL_A_256(257 - a0, *dst) + (*dst & 0x00ffffff);
	       if (l == 0)
		   *dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
	       if (r == r0)
	         {
		   a = 1 + ((rr - rr0) >> 8);

		   *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	         }
	       if (r == r1)
	         {
		   a = 256 - ((rr - rr1) >> 8);

		   *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	         }
	     }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l < map_len)
	      {
		l += off;
		if (l < 0) l = -l;
		if (l >= map_len)
	          {
		    int  m = (l % (2 * map_len));

		    l = (l % map_len);
		    if (m >= map_len)
			l = map_len - l - 1;
	          }
		*dst = map[l];  *mask = 255;
	      }
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_reflect_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	   int  ll = ss * (M_PI + atan2(yy, xx));
	   int  l = ll >> 16, lp;

	   if (l < map_len)
	     {
		DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

		lp = l + off;
		if (lp < 0) { lp = -lp;  257 - a; }
		if (lp >= map_len)
	          {
		    int  m = (lp % (2 * map_len));

		    lp = (lp % map_len);
		    if (m >= map_len)
			{ lp = map_len - lp - 1;  257 - a; }
	          }
	       *dst = map[lp];  *mask = 255;
	       if (lp + 1 < map_len)
		  *dst = INTERP_256(a, map[lp + 1], *dst);
	       if (l == (map_len - 1))
		   *mask = 255 - a0;
	       if (l == 0)
		   *mask = a0;
	       if (r == r0)
		   *mask = ((rr - rr0) >> 8);
	       if (r == r1)
		   *mask = 255 - ((rr - rr1) >> 8);
	     }
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16;

	*dst = 0;
	l += ((ll - (l << 16)) >> 15);
	if (l < map_len)
	  {
	    l += off;
	    l = l % map_len;
	    if (l < 0)
		l += map_len;
	    *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI + atan2(yy, xx));
	int  l = ll >> 16, lp;

	*dst = 0;
	if (l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

	    lp = l + off;
	    lp = lp % map_len;
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == (map_len - 1))
		*dst = INTERP_256(a, map[0], *dst);
	    if (l == (map_len - 1))
		*dst = MUL_A_256(257 - a0, *dst) + (*dst & 0x00ffffff);
	    if (l == 0)
		*dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	l += (ll - (l << 16)) >> 15;
	if (l < map_len)
	  {
	    l += off;
	    l = l % map_len;
	    if (l < 0)
		l += map_len;
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   float   off = gdata->off * (map_len - 1);

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
	int  ll = ss * (M_PI +  atan2(yy, xx));
	int  l = ll >> 16, lp;

	*dst = 0;  *mask = 0;
	if (l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

	    lp = l + off;
	    lp = lp % map_len;
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == (map_len - 1))
		*dst = INTERP_256(a, map[0], *dst);
	    if (l == (map_len - 1))
		*mask = 255 - a0;
	    if (l == 0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (map_len - 1);

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
	    int  ll = ss * (M_PI +  atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l < map_len)
	      {
		l += off;
		l = l % map_len;
		if (l < 0)
		   l += map_len;
		*dst = map[l];
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (map_len - 1);

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
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16, lp;

	    if (l < map_len)
	      {
		DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

		lp = l + off;
		lp = lp % map_len;
		if (lp < 0)
		   lp += map_len;
		*dst = map[lp];
		if (lp + 1 < map_len)
		   *dst = INTERP_256(a, map[lp + 1], *dst);
		if (lp == (map_len - 1))
		   *dst = INTERP_256(a, map[0], *dst);
		if (l == (map_len - 1))
		   *dst = MUL_A_256(257 - a0, *dst) + (*dst & 0x00ffffff);
		if (l == 0)
		   *dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
		if (r == r0)
	          {
		    a = 1 + ((rr - rr0) >> 8);

		    *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	          }
		if (r == r1)
	          {
		    a = 256 - ((rr - rr1) >> 8);

		    *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	          }
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l < map_len)
	      {
		l += off;
		l = l % map_len;
		if (l < 0)
		   l += map_len;
		*dst = map[l];  *mask = 255;
	      }
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_restrict_repeat_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;
   float   off = gdata->off * (map_len - 1);

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

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16, lp;

	    if (l < map_len)
	      {
		DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

		lp = l + off;
		lp = lp % map_len;
		if (lp < 0)
		   lp += map_len;
		*dst = map[lp];  *mask = 255;
		if (lp + 1 < map_len)
		   *dst = INTERP_256(a, map[lp + 1], *dst);
		if (lp == (map_len - 1))
		   *dst = INTERP_256(a, map[0], *dst);
		if (l == (map_len - 1))
		   *mask = 255 - a0;
		if (l == 0)
		   *mask = a0;
		if (r == r0)
		   *mask = ((rr - rr0) >> 8);
		if (r == r1)
		   *mask = 255 - ((rr - rr1) >> 8);
	      }
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;

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
	int  ll = ss * (M_PI +  atan2(yy, xx));
	int  l = ll >> 16;

	l += (ll - (l << 16)) >> 15;
	if (l >= map_len)
	    l = map_len - 1;
	*dst++ = map[l];  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;

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
	int  ll = ss * (M_PI +  atan2(yy, xx));
	int  l = ll >> 16;
	DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	if (l + 1 < map_len)
	   *dst = INTERP_256(a, map[l + 1], map[l]);
	else
	   *dst = map[map_len - 1];
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;

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
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l >= map_len)
		l = map_len - 1;
	    *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_aa_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;

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
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    if (l + 1 < map_len)
		*dst = INTERP_256(a, map[l + 1], map[l]);
	    else
		*dst = map[map_len - 1];
	    if (r == r0)
	      {
		a = 1 + ((rr - rr0) >> 8);

		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	    if (r == r1)
	      {
		a = 256 - ((rr - rr1) >> 8);

		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;

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

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;

	    l += (ll - (l << 16)) >> 15;
	    if (l >= map_len)
		l = map_len - 1;
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
angular_pad_aa_masked_annulus(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Angular_Data  *gdata = (Angular_Data *)params_data;
   int     xx, yy;
   int     ss = (gdata->s) << 16;
   int     r1 = gdata->s, r0 = gdata->an * r1;
   int     rr0 = r0 << 16, rr1 = r1 << 16;

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

	*dst = 0;  *mask = 0;
	if ( (r >= r0) && (r <= r1) )
	  {
	    int  ll = ss * (M_PI + atan2(yy, xx));
	    int  l = ll >> 16;
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    if (l + 1 < map_len)
		*dst = INTERP_256(a, map[l + 1], map[l]);
	    else
		*dst = map[map_len - 1];

	    *mask = 255;
	    if (r == r0)
		*mask = ((rr - rr0) >> 8);
	    if (r == r1)
		*mask = 255 - ((rr - rr1) >> 8);
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}
