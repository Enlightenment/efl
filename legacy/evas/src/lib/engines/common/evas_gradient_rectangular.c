#include "evas_common.h"
#include <math.h>


typedef struct _Rectangular_Data   Rectangular_Data;
struct _Rectangular_Data
{
   int    sx, sy, s;
   float  r0, off;
};

static Rectangular_Data  rectangular_data = {32, 32, 32, 0.0, 0.0};


static void 
rectangular_setup_geom(RGBA_Gradient *gr, int spread);

static int 
rectangular_has_alpha(RGBA_Gradient *gr, int spread, int op);

static int 
rectangular_has_mask(RGBA_Gradient *gr, int spread, int op);

static int 
rectangular_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Fill 
rectangular_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa);

static RGBA_Gradient_Type  rectangular = {"rectangular", &rectangular_data, rectangular_setup_geom, rectangular_has_alpha, rectangular_has_mask, rectangular_get_map_len, rectangular_get_fill_func};


/** internal functions **/

static void
rectangular_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_pad_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_pad_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
   rectangular_data.off = gr->range_offset;

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
	    break;
	  }
     }
   if (!err)
     {
	if (r0 < 0.0) r0 = 0.0;
	if (r0 > 1.0) r0 = 1.0;
	rectangular_data.r0 = r0;
     }
   free(s);
}


static int
rectangular_has_alpha(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &rectangular)) return 0;
   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if (rectangular_data.r0 > 0)
	return 1;
   if ( (spread == _EVAS_TEXTURE_RESTRICT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	return 1;
   return 0;
}

static int
rectangular_has_mask(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &rectangular)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if (rectangular_data.r0 > 0)
	    return 1;
	if ( (spread == _EVAS_TEXTURE_RESTRICT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	    return 1;
     }
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

static Gfx_Func_Gradient_Fill
rectangular_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa)
{
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &rectangular))
	return sfunc;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;
   switch (spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	   {
	    if (rectangular_data.r0 > 0)
	      {
		if (masked_op)
		   sfunc = rectangular_reflect_aa_masked;
		else
		   sfunc = rectangular_reflect_aa;
	      }
	    else
		sfunc = rectangular_reflect_aa;
	   }
	 else
	   {
	    if (rectangular_data.r0 > 0)
	      {
		if (masked_op)
		   sfunc = rectangular_reflect_masked;
		else
		   sfunc = rectangular_reflect;
	      }
	    else
		sfunc = rectangular_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	   {
	    if (rectangular_data.r0 > 0)
	      {
		if (masked_op)
		   sfunc = rectangular_repeat_aa_masked;
		else
		   sfunc = rectangular_repeat_aa;
	      }
	    else
		sfunc = rectangular_repeat_aa;
	   }
	 else
	   {
	    if (rectangular_data.r0 > 0)
	      {
		if (masked_op)
		   sfunc = rectangular_repeat_masked;
		else
		   sfunc = rectangular_repeat;
	      }
	    else
		sfunc = rectangular_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	 rectangular_data.off = 0;
      case _EVAS_TEXTURE_RESTRICT_REFLECT:
	{
	 if (aa)
	   {
	    if (masked_op)
		sfunc = rectangular_restrict_reflect_aa_masked;
	    else
		sfunc = rectangular_restrict_reflect_aa;
	   }
	 else
	   {
	    if (masked_op)
		sfunc = rectangular_restrict_reflect_masked;
	    else
		sfunc = rectangular_restrict_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT_REPEAT:
	{
	 if (aa)
	   {
	    if (masked_op)
		sfunc = rectangular_restrict_repeat_aa_masked;
	    else
		sfunc = rectangular_restrict_repeat_aa;
	   }
	 else
	   {
	    if (masked_op)
		sfunc = rectangular_restrict_repeat_masked;
	    else
		sfunc = rectangular_restrict_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_PAD:
	{
	 if (aa)
	   {
	    if (rectangular_data.r0 > 0)
	      {
		if (masked_op)
		   sfunc = rectangular_pad_aa_masked;
		else
		   sfunc = rectangular_pad_aa;
	      }
	    else
		sfunc = rectangular_pad_aa;
	   }
	 else
	   {
	    if (rectangular_data.r0 > 0)
	      {
		if (masked_op)
		   sfunc = rectangular_pad_masked;
		else
		   sfunc = rectangular_pad;
	      }
	    else
		sfunc = rectangular_pad;
	   }
	}
      default:
	sfunc = rectangular_reflect;
      break;
     }
   return sfunc;
}

/* the fill functions */

#ifdef BUILD_MMX
#define INTERP_256_MMX(a, s0, s1, d, mma, mms, mmd, mmz) \
	    MOV_A2R(a, mma) \
	    MOV_P2R(s0, mms, mmz) \
	    MOV_P2R(s1, mmd, mmz) \
	    INTERP_256_R2R(mma, mms, mmd) \
	    MOV_R2P(mmd, d, mmz)
#endif

#ifdef BUILD_MMX
#define MUL_A_256_MMX(a, d, mma, mmd, mmz, mmrgb) \
	movd_m2r(a, mma); \
	psllq_i2r(48, mma); \
	por_r2r(mmrgb, mma); \
	MOV_P2R(d, mmd, mmz) \
	MUL4_256_R2R(mma, mmd) \
	MOV_R2P(mmd, d, mmz)
#endif

#define SETUP_RECT_FILL \
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
rectangular_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

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
rectangular_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy, lp;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[lp + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[lp + 1], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_A_256_MMX(a0, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
rectangular_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy, lp;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[lp + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[lp + 1], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

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
rectangular_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy, lp;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[lp + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[lp + 1], *dst);
#endif
	      }
	    if (lp == (map_len - 1))
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[0], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[0], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
rectangular_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy, lp;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;  *mask = 0;
	if (l >= 0)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = lp % map_len;
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];  *mask = 255;
	    if (lp + 1 < map_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[lp + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[lp + 1], *dst);
#endif
	      }
	    if (lp == (map_len - 1))
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[0], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[0], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

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
rectangular_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy, lp;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[lp + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[lp + 1], *dst);
#endif
	      }
	    if (l == (map_len - 1))
	      {
#ifdef BUILD_MMX
		a = 257 - a0;
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(257 - a0, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_A_256_MMX(a0, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
rectangular_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy, lp;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[lp + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[lp + 1], *dst);
#endif
	      }
	    if (l == (map_len - 1))
		*mask = 255 - a0;
	    if ((l == 0) && rr0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

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
rectangular_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a1 = 257 - a;

	    lp = l + off;
	    lp = (lp % map_len);
	    if (lp < 0)
		lp += map_len;
	    *dst = map[lp];
	    if (lp + 1 < map_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[lp + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[lp + 1], *dst);
#endif
	      }
	    if (lp == (map_len - 1))
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[0], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[0], *dst);
#endif
	      }
	    if (l == (map_len - 1))
	      {
#ifdef BUILD_MMX
		MUL_A_256_MMX(a1, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a1, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

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
rectangular_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (map_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
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
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[lp + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[lp + 1], *dst);
#endif
	      }
	    if (lp == (map_len - 1))
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[0], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[0], *dst);
#endif
	      }
	    if (l == (map_len - 1))
		*mask = 256 - a;
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}


static void
rectangular_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RECT_FILL

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
	       l = map_len - 1;
	   *dst = map[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;
	if (l >= map_len) *dst = map[map_len - 1];
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    *dst = map[l];
	    if (l + 1 < map_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_pad_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RECT_FILL

   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

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
rectangular_pad_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;  *mask = 0;
	if (l >= map_len) { *dst = map[map_len - 1];  *mask = 255; }
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    *dst = map[l];  *mask = 255;
	    if (l + 1 < map_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}
