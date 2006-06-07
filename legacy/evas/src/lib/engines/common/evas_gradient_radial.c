#include "evas_common.h"
#include <math.h>


typedef struct _Radial_Data   Radial_Data;
struct _Radial_Data
{
   int    sx, sy, s;
   float  r0, off;
};

static Radial_Data  radial_data = {32, 32, 32, 0.0, 0.0};


static void 
radial_setup_geom(RGBA_Gradient *gr, int spread);

static int 
radial_has_alpha(RGBA_Gradient *gr, int spread, int op);

static int 
radial_has_mask(RGBA_Gradient *gr, int spread, int op);

static int 
radial_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Fill 
radial_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa);

static RGBA_Gradient_Type  radial = {"radial", &radial_data, radial_setup_geom, radial_has_alpha, radial_has_mask, radial_get_map_len, radial_get_fill_func};


/** internal functions **/

static void
radial_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);
static void
radial_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_pad_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_pad_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);


RGBA_Gradient_Type  *
evas_common_gradient_radial_get(void)
{
    return &radial;
}

static void
radial_setup_geom(RGBA_Gradient *gr, int spread)
{
   int    err = 1;
   char   *s, *p, key[256];
   float  f = 0.0;

   if (!gr || (gr->type.geometer != &radial)) return;

   radial_data.sx = gr->fill.w;
   radial_data.sy = gr->fill.h;
   radial_data.s = radial_data.sx;
   if (radial_data.sy > radial_data.sx)
	radial_data.s = radial_data.sy;
   radial_data.r0 = 0.0;
   radial_data.off = gr->range_offset;

   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   p = s;
   while ((p = evas_common_gradient_get_key_fval(p, key, &f)))
     {
	if (!strcmp(key, "inner_radius"))
	    err = 0;
	else
	  {
	    err = 1;
	    break;
	  }
     }
   if (!err)
     {
	if (f < 0.0) f = 0.0;
	if (f > 1.0) f = 1.0;
	radial_data.r0 = f;
     }
   free(s);
}


static int
radial_has_alpha(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &radial)) return 0;
   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if (radial_data.r0 > 0)
	return 1;
   if ( (spread == _EVAS_TEXTURE_RESTRICT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	return 1;
   return 0;
}

static int
radial_has_mask(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &radial)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if (radial_data.r0 > 0)
	    return 1;
	if ( (spread == _EVAS_TEXTURE_RESTRICT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	    return 1;
     }
   return 0;
}

static int
radial_get_map_len(RGBA_Gradient *gr, int spread)
{
   int l;

   if (!gr || (gr->type.geometer != &radial)) return 0;
   l = radial_data.s;
   l -= (int)(l * radial_data.r0);
   return l;
}

static Gfx_Func_Gradient_Fill
radial_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa)
{
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &radial)) return sfunc;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	   {
	     if (radial_data.r0 > 0)
	       {
		if (masked_op)
		   sfunc = radial_reflect_aa_masked;
		else
		   sfunc = radial_reflect_aa;
	       }
	     else
		sfunc = radial_reflect_aa;
	   }
	 else
	   {
	     if (radial_data.r0 > 0)
	       {
		if (masked_op)
		   sfunc = radial_reflect_masked;
		else
		   sfunc = radial_reflect;
	       }
	     else
		sfunc = radial_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	   {
	     if (radial_data.r0 > 0)
	       {
		if (masked_op)
		   sfunc = radial_repeat_aa_masked;
		else
		   sfunc = radial_repeat_aa;
	       }
	     else
		sfunc = radial_repeat_aa;
	   }
	 else
	   {
	     if (radial_data.r0 > 0)
	       {
		if (masked_op)
		   sfunc = radial_repeat_masked;
		else
		   sfunc = radial_repeat;
	       }
	     else
		sfunc = radial_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	 radial_data.off = 0;
      case _EVAS_TEXTURE_RESTRICT_REFLECT:
	{
	 if (aa)
	   {
	     if (masked_op)
		sfunc = radial_restrict_reflect_aa_masked;
	     else
		sfunc = radial_restrict_reflect_aa;
	   }
	 else
	   {
	     if (masked_op)
		sfunc = radial_restrict_reflect_masked;
	     else
		sfunc = radial_restrict_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT_REPEAT:
	{
	 if (aa)
	   {
	     if (masked_op)
		sfunc = radial_restrict_repeat_aa_masked;
	     else
		sfunc = radial_restrict_repeat_aa;
	   }
	 else
	   {
	     if (masked_op)
		sfunc = radial_restrict_repeat_masked;
	     else
		sfunc = radial_restrict_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_PAD:
	{
	 if (aa)
	   {
	     if (masked_op)
		sfunc = radial_pad_aa_masked;
	     else
		sfunc = radial_pad_aa;
	   }
	 else
	   {
	     if (masked_op)
		sfunc = radial_pad_masked;
	     else
		sfunc = radial_pad;
	   }
	}
      break;
      default:
	sfunc = radial_reflect_aa;
      break;
     }
   return sfunc;
}

#define SETUP_RADIAL_FILL \
   if (gdata->sx != gdata->s) \
     { \
	axx = (gdata->s * axx) / gdata->sx; \
	axy = (gdata->s * axy) / gdata->sx; \
     } \
   if (gdata->sy != gdata->s) \
     { \
	ayy = (gdata->s * ayy) / gdata->sy; \
	ayx = (gdata->s * ayx) / gdata->sy; \
     } \
   xx = (axx * x) + (axy * y); \
   yy = (ayx * x) + (ayy * y); \
   rr0 = gdata->r0 * gdata->s; \
   rr0 <<= 16;


static void
radial_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if (l >= 0)
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
radial_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;
	if (l >= 0)
	  {
	    DATA32  a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

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
	    if ((l == 0) && rr0)
		*dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if (l >= 0)
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
radial_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;  *mask = 0;
	if (l >= 0)
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
	    if ((l == 0) && rr0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if (l >= 0)
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
radial_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;
	if (l >= 0)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = lp % map_len;
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == map_len - 1)
		*dst = INTERP_256(a, map[0], *dst);
	    if ((l == 0) && rr0)
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if (l >= 0)
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
radial_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;
	if (l >= 0)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = lp % map_len;
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == map_len - 1)
		*dst = INTERP_256(a, map[0], *dst);
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if ((unsigned)l < map_len)
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
radial_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;
	if ((unsigned)l < map_len)
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
	    if ((l == 0) && rr0)
		*dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
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
radial_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
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
	    if ((l == 0) && rr0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    l += off;
	    l = (l % map_len);
	    if (l < 0)
		l += map_len;
	    *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = (lp % map_len);
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == (map_len - 1))
		*dst = INTERP_256(a, map[0], *dst);
	    if (l == (map_len - 1))
		*dst = MUL_A_256(257 - a, *dst) + (*dst & 0x00ffffff);
	    if ((l == 0) && rr0)
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    l += off;
	    l = (l % map_len);
	    if (l < 0)
		l += map_len;
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = (lp % map_len);
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
		*dst = INTERP_256(a, map[lp + 1], *dst);
	    if (lp == (map_len - 1))
		*dst = INTERP_256(a, map[0], *dst);
	    if (l == (map_len - 1))
		*mask = 256 - a;
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if (l >= 0)
	  {
	    if (l >= map_len)
		l = map_len - 1;
	    *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);
	DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    *dst = map[l];
	    if (l + 1 < map_len)
		*dst = INTERP_256(a, map[l + 1], map[l]);
	  }
	 if (l == 0)
	   {
	     *dst = map[0];
	     if (rr0)
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
	   }
	 if (l >= map_len)
	   {
	     *dst = map[map_len - 1];
	   }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_pad_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if (l >= 0)
	  {
	    if (l >= map_len)
		l = map_len - 1;
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_pad_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);
	DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    *dst = map[l];
	    if (l + 1 < map_len)
		*dst = INTERP_256(a, map[l + 1], map[l]);
	  }
	if (l == 0)
	  {
	    *dst = map[0];  *mask = 255;
	    if (rr0)
		*mask = a - 1;
	  }
	if (l >= map_len)
	  {
	    *dst = map[map_len - 1];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}
