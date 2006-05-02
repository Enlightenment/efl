#include "evas_common.h"
#include <math.h>


typedef struct _Linear_Data   Linear_Data;
struct _Linear_Data
{
   int    sx;
   float  off;
};

static Linear_Data  linear_data = {-1, 0.0};


static void
linear_setup_geom(RGBA_Gradient *gr, int spread);

static int
linear_has_alpha(RGBA_Gradient *gr, int spread, int op);

static int
linear_has_mask(RGBA_Gradient *gr, int spread, int op);

static int
linear_get_map_len(RGBA_Gradient *gr, int spread);

static Gfx_Func_Gradient_Fill
linear_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa);

static RGBA_Gradient_Type  linear = {"linear", &linear_data, linear_setup_geom, linear_has_alpha, linear_has_mask, linear_get_map_len, linear_get_fill_func};


/** internal functions **/
static void
linear_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_reflect_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_reflect_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_reflect_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_reflect_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_pad_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_pad_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_pad_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_pad_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);



RGBA_Gradient_Type  *
evas_common_gradient_linear_get(void)
{
    return &linear;
}


static void
linear_setup_geom(RGBA_Gradient *gr, int spread)
{
   int    err = 1;
   char   *s, *p, key[256];
   float  val = -1;

   if (!gr || (gr->type.geometer != &linear)) return;

   linear_data.sx = -1;
   linear_data.off = gr->range_offset;
   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   p = s;
   while ((p = evas_common_gradient_get_key_fval(p, key, &val)))
     {
	if (!strcmp(key, "extent"))
	    err = 0;
	else
	  {
	    err = 1;
	    break;
	  }
     }
   if (!err)
     {
	if (val < 0.0)
	   val = -val;
	linear_data.sx = val * gr->fill.w;
     }
   free(s);
}


static int
linear_has_alpha(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   if (gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if (linear_data.sx >= 0)
	return 1;
   if ( (spread == _EVAS_TEXTURE_RESTRICT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	return 1;
   return 0;
}

static int
linear_has_mask(RGBA_Gradient *gr, int spread, int op)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if (linear_data.sx >= 0)
	    return 1;
	if ( (spread == _EVAS_TEXTURE_RESTRICT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	    return 1;
     }
   return 0;
}

static int
linear_get_map_len(RGBA_Gradient *gr, int spread)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   return gr->fill.h;
}

static Gfx_Func_Gradient_Fill
linear_get_fill_func(RGBA_Gradient *gr, int spread, int op, unsigned char aa)
{
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &linear))
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
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_reflect_aa_masked_cropped;
		else
		   sfunc = linear_reflect_aa_cropped;
	      }
	    else
	       sfunc = linear_reflect_aa;
	   }
	 else
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_reflect_masked_cropped;
		else
		   sfunc = linear_reflect_cropped;
	      }
	    else
	       sfunc = linear_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_repeat_aa_masked_cropped;
		else
		   sfunc = linear_repeat_aa_cropped;
	      }
	    else
	       sfunc = linear_repeat_aa;
	   }
	 else
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_repeat_masked_cropped;
		else
		   sfunc = linear_repeat_cropped;
	      }
	    else
	       sfunc = linear_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	 linear_data.off = 0;
      case _EVAS_TEXTURE_RESTRICT_REFLECT:
	{
	 if (aa)
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_restrict_reflect_aa_masked_cropped;
		else
		   sfunc = linear_restrict_reflect_aa_cropped;
	      }
	    else
	      {
		if (masked_op)
		   sfunc = linear_restrict_reflect_aa_masked;
		else
		   sfunc = linear_restrict_reflect_aa;
	      }
	   }
	 else
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_restrict_reflect_masked_cropped;
		else
		   sfunc = linear_restrict_reflect_cropped;
	      }
	    else
	      {
		if (masked_op)
		   sfunc = linear_restrict_reflect_masked;
		else
		   sfunc = linear_restrict_reflect;
	      }
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT_REPEAT:
	{
	 if (aa)
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_restrict_repeat_aa_masked_cropped;
		else
		   sfunc = linear_restrict_repeat_aa_cropped;
	      }
	    else
	      {
		if (masked_op)
		   sfunc = linear_restrict_repeat_aa_masked;
		else
		   sfunc = linear_restrict_repeat_aa;
	      }
	   }
	 else
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_restrict_repeat_masked_cropped;
		else
		   sfunc = linear_restrict_repeat_cropped;
	      }
	    else
	      {
		if (masked_op)
		   sfunc = linear_restrict_repeat_masked;
		else
		   sfunc = linear_restrict_repeat;
	      }
	   }
	}
      break;
      case _EVAS_TEXTURE_PAD:
	{
	 if (aa)
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_pad_aa_masked_cropped;
		else
		   sfunc = linear_pad_aa_cropped;
	      }
	    else
	       sfunc = linear_pad_aa;
	   }
	 else
	   {
	    if (linear_data.sx >= 0)
	      {
		if (masked_op)
		   sfunc = linear_pad_masked_cropped;
		else
		   sfunc = linear_pad_cropped;
	      }
	    else
	       sfunc = linear_pad;
	   }
	}
      break;
      default:
	sfunc = linear_reflect_aa;
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

static void
linear_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = ayx * x + ayy * y;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += ((yy - (l << 16)) >> 15) + off;
	if (l < 0)  l = -l;
	if (l >= map_len)
	  {
	    int  m = (l % (2 * map_len));

	    l = (l % map_len);
	    if (m >= map_len)
	        l = map_len - l - 1;
	  }
	*dst++ = map[l];  yy += ayx;
     }
}

static void
linear_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = ayx * x + ayy * y;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);
	int  a = 1 + ((yy - (l << 16)) >> 8);

	l += off;
	if (l < 0) { l = -l;  a = 257 - a; }
	if (l >= map_len)
	  {
	    int  m = (l % (2 * map_len));

	    l = (l % map_len);
	    if (m >= map_len)
	      { l = map_len - l - 1;  a = 257 - a; }
	  }
	*dst = map[l];
	if (l + 1 < map_len)
	  {
#ifdef BUILD_MMX
	    INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
	    *dst = INTERP_256(a, map[l + 1], *dst);
#endif
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_reflect_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += ((yy - (l << 16)) >> 15) + off;
	   if (l < 0)  l = -l;
	   if (l >= map_len)
	     {
	        int m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		    l = map_len - l - 1;
	     }
	   *dst = map[l];
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_reflect_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	    int  l = (yy >> 16);
	    int  a = 1 + ((yy - (l << 16)) >> 8);

	    l += off;
	    if (l < 0) { l = -l;  a = 257 - a; }
	    if (l >= map_len)
	      {
	        int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		  { l = map_len - l - 1;  a = 257 - a; }
	      }
	    *dst = map[l];
	    if (l + 1 < map_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], *dst);
#endif
	      }
	    if (ex == 0)
	      {
		a = 1 + (xx >> 8);
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	    if (ex == (w - 1))
	      {
		a = 256 - ((xx - (ex << 16)) >> 8);
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_reflect_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += ((yy - (l << 16)) >> 15) + off;
	   if (l < 0)  l = -l;
	   if (l >= map_len)
	     {
	        int m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		    l = map_len - l - 1;
	     }
	   *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_reflect_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	    int  l = (yy >> 16);
	    int  a = 1 + ((yy - (l << 16)) >> 8);

	    l += off;
	    if (l < 0) { l = -l;  a = 257 - a; }
	    if (l >= map_len)
	      {
	        int  m = (l % (2 * map_len));

		l = (l % map_len);
		if (m >= map_len)
		  { l = map_len - l - 1;  a = 257 - a; }
	      }
	    *dst = map[l];  *mask = 255;
	    if (l + 1 < map_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], *dst);
#endif
	      }
	    if (ex == 0)
		*mask = (xx >> 8);
	    if (ex == (w - 1))
		*mask = 255 - ((xx - (ex << 16)) >> 8);
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = ayx * x + ayy * y;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += ((yy - (l << 16)) >> 15) + off;
	l = l % map_len;
	if (l < 0)
	   l += map_len;
	*dst++ = map[l];  yy += ayx;
     }
}

static void
linear_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = ayx * x + ayy * y;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);
	int  a = 1 + ((yy - (l << 16)) >> 8);

	l += off;
	l = l % map_len;
	if (l < 0)
	   l += map_len;
	*dst = map[l];
	if (l + 1 < map_len)
	  {
#ifdef BUILD_MMX
	    INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
	    *dst = INTERP_256(a, map[l + 1], *dst);
#endif
	  }
	if (l == (map_len - 1))
	  {
#ifdef BUILD_MMX
	    INTERP_256_MMX(a, map[0], *dst, *dst, mm3, mm2, mm1, mm0)
#else
	   *dst = INTERP_256(a, map[0], *dst);
#endif
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_repeat_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += ((yy - (l << 16)) >> 15) + off;
	   l = l % map_len;
	   if (l < 0)
	      l += map_len;
	   *dst = map[l];
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_repeat_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);
	   int  a = 1 + ((yy - (l << 16)) >> 8);

	   l += off;
	   l = l % map_len;
	   if (l < 0)
	      l += map_len;
	   *dst = map[l];
	   if (l + 1 < map_len)
	     {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], *dst);
#endif
	     }
	   if (l == (map_len - 1))
	     {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[0], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[0], *dst);
#endif
	     }
	   if (ex == 0)
	     {
		a = 1 + (xx >> 8);
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	     }
	   if (ex == (w - 1))
	     {
		a = 256 - ((xx - (ex << 16)) >> 8);
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	     }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_repeat_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += ((yy - (l << 16)) >> 15) + off;
	   l = l % map_len;
	   if (l < 0)
	      l += map_len;
	   *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_repeat_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);
	   int  a = 1 + ((yy - (l << 16)) >> 8);

	   l += off;
	   l = l % map_len;
	   if (l < 0)
	      l += map_len;
	   *dst = map[l];  *mask = 255;
	   if (l + 1 < map_len)
	     {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], *dst);
#endif
	     }
	   if (l == (map_len - 1))
	     {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[0], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[0], *dst);
#endif
	     }
	   if (ex == 0)
		*mask = (xx >> 8);
	   if (ex == (w - 1))
		*mask = 255 - ((xx - (ex << 16)) >> 8);
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_reflect(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
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
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_reflect_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = ayx * x + ayy * y;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8), a0 = a;
	    int    lp = l + off;

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
	    if (l == 0)
	      {
#ifdef BUILD_MMX
		MUL_A_256_MMX(a0, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	    if (l == (map_len - 1))
	      {
#ifdef BUILD_MMX
		a0 = 257 - a0;
		MUL_A_256_MMX(a0, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(257 - a0, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_reflect_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;
	if ((unsigned)ex < w)
	  {
	    int  l = (yy >> 16);

	    l += (yy - (l << 16)) >> 15;
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
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_reflect_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	    int  l = (yy >> 16);

	    if ((unsigned)l < map_len)
	      {
		DATA32 a = 1 + ((yy - (l << 16)) >> 8), a0 = a;
		int    lp = l + off;

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
		if (l == 0)
		  {
#ifdef BUILD_MMX
		   MUL_A_256_MMX(a0, *dst, mm2, mm1, mm0, mm6)
#else
		   *dst = MUL_A_256(a0, *dst) + (*dst & 0x00ffffff);
#endif
		  }
		if (l == (map_len - 1))
		  {
#ifdef BUILD_MMX
		   a0 = 257 - a0;
		   MUL_A_256_MMX(a0, *dst, mm2, mm1, mm0, mm6)
#else
		   *dst = MUL_A_256(257 - a0, *dst) + (*dst & 0x00ffffff);
#endif
		  }
		if (ex == 0)
		  {
		   a = 1 + (xx >> 8);
#ifdef BUILD_MMX
		   MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		   *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
		  }
		if (ex == (w - 1))
		  {
		   a = 256 - ((xx - (ex << 16)) >> 8);
#ifdef BUILD_MMX
		   MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		   *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
		  }
	      }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_reflect_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += ((yy - (l << 16)) >> 15);
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
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_reflect_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
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
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_reflect_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = ayx * x + ayy * y;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8), a0 = a - 1;
	    int    lp = l + off;

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
	    if (l == 0)
		*mask = a0;
	    if (l == (map_len - 1))
		*mask = 255 - a0;
	  }
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_reflect_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   if ((unsigned)l < map_len)
	     {
		DATA32 a = 1 + ((yy - (l << 16)) >> 8), a0 = a - 1;
		int    lp = l + off;

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
		if (l == 0)
		   *mask = a0;
		if (l == (map_len - 1))
		   *mask = 255 - a0;
		if (ex == 0)
		   *mask = (xx >> 8);
		if (ex == (w - 1))
		   *mask = 255 - ((xx - (ex << 16)) >> 8);
	     }
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_repeat(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    l += off;
	    l = l % map_len;
	    if (l < 0)
		l += map_len;
	    *dst = map[l];
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_repeat_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = ayx * x + ayy * y;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8);
	    int    lp = l + off;

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
	    if (l == 0)
	      {
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	    if (l == (map_len - 1))
	      {
#ifdef BUILD_MMX
		a = 257 - a;
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(257 - a, *dst) + (*dst & 0x00ffffff);
#endif
	      }
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_repeat_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
	   if ((unsigned)l < map_len)
	     {
		l += off;
		l = l % map_len;
		if (l < 0)
		   l += map_len;
		*dst = map[l];
	     }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_repeat_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	    int  l = (yy >> 16);

	    if ((unsigned)l < map_len)
	      {
		DATA32 a = 1 + ((yy - (l << 16)) >> 8);
		int    lp = l + off;

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
		if (l == 0)
		  {
#ifdef BUILD_MMX
		   MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		   *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
		  }
		if (l == (map_len - 1))
		  {
#ifdef BUILD_MMX
		   a = 257 - a;
		   MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		   *dst = MUL_A_256(257 - a, *dst) + (*dst & 0x00ffffff);
#endif
		  }
		if (ex == 0)
		  {
		   a = 1 + (xx >> 8);
#ifdef BUILD_MMX
		   MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		   *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
		  }
		if (ex == (w - 1))
		  {
		   a = 256 - ((xx - (ex << 16)) >> 8);
#ifdef BUILD_MMX
		   MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		   *dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
		  }
	      }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_repeat_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += ((yy - (l << 16)) >> 15);
	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    l += off;
	    l = l % map_len;
	    if (l < 0)
		l += map_len;
	    *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_repeat_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
	   if ((unsigned)l < map_len)
	     {
		l += off;
		l = l % map_len;
		if (l > 0)
		   l += map_len;
		*dst = map[l];  *mask = 255;
	     }
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_restrict_repeat_aa_masked(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = ayx * x + ayy * y;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8);
	    int    lp = l + off;

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
	    if (l == 0)
		*mask = a - 1;
	    if (l == (map_len - 1))
		*mask = 256 - a;
	  }
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_repeat_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;
   int      off = gdata->off * (map_len - 1);

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   if ((unsigned)l < map_len)
	     {
		DATA32 a = 1 + ((yy - (l << 16)) >> 8);
		int    lp = l + off;

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
		if (l == 0)
		   *mask = a - 1;
		if (l == (map_len - 1))
		   *mask = 256 - a;
		if (ex == 0)
		   *mask = (xx >> 8);
		if (ex == (w - 1))
		   *mask = 255 - ((xx - (ex << 16)) >> 8);
	     }
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_pad(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   int      yy = ayx * x + ayy * y;

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	if (l < 0) l = 0;
	if (l >= map_len) l = map_len - 1;
	*dst = map[l];
	dst++;  yy += ayx;
     }
}

static void
linear_pad_aa(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   int      yy = ayx * x + ayy * y;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	if (l < 0) *dst = map[0];
	if (l >= map_len) *dst = map[map_len - 1];
	if ((unsigned)l < map_len)
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8);

	    *dst = map[l];
	    if (l && (l + 1 < map_len))
	      {
#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], *dst, *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], *dst);
#endif
	      }
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_pad_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
	   if (l < 0) l = 0;
	   if (l >= map_len) l = map_len - 1;
	   *dst = map[l];
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_pad_aa_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_256, mm6)
   psrlq_i2r(16, mm6);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   if ((unsigned)l < (map_len - 1))
	     {
		DATA32 a = 1 + ((yy - (l << 16)) >> 8);

#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], map[l], *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], map[l]);
#endif
	     }
	   if (l < 0) *dst = map[0];
	   if (l >= map_len) *dst = map[map_len - 1];
	   if (ex == 0)
	     {
		int a = 1 + (xx >> 8);
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	     }
	   if (ex == (w - 1))
	     {
		int a = 256 - ((xx - (ex << 16)) >> 8);
#ifdef BUILD_MMX
		MUL_A_256_MMX(a, *dst, mm2, mm1, mm0, mm6)
#else
		*dst = MUL_A_256(a, *dst) + (*dst & 0x00ffffff);
#endif
	     }
	  }
	dst++;  yy += ayx;  xx += axx;
     }
}

static void
linear_pad_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	ex += (xx - (ex << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   l += (yy - (l << 16)) >> 15;
	   if (l < 0) l = 0;
	   if (l >= map_len) l = map_len - 1;
	   *dst = map[l];  *mask = 255;
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}

static void
linear_pad_aa_masked_cropped(DATA32 *map, int map_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy = (ayx * x) + (ayy * y);
   int      xx = (axx * x) + (axy * y);
   int      w = gdata->sx;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
#endif
   while (dst < dst_end)
     {
	int  ex = (xx >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)ex < w)
	  {
	   int  l = (yy >> 16);

	   if ((unsigned)l < (map_len - 1))
	     {
		DATA32 a = 1 + ((yy - (l << 16)) >> 8);

#ifdef BUILD_MMX
		INTERP_256_MMX(a, map[l + 1], map[l], *dst, mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, map[l + 1], map[l]);
#endif
	     }
	   if (l < 0) *dst = map[0];
	   if (l >= map_len) *dst = map[map_len - 1];
	   *mask = 255;
	   if (ex == 0)
		*mask = (xx >> 8);
	   if (ex == (w - 1))
		*mask = 255 - ((xx - (ex << 16)) >> 8);
	  }
	dst++;  mask++;  yy += ayx;  xx += axx;
     }
}
