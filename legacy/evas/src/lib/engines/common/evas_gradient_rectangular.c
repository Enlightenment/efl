/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <math.h>

#include "evas_common.h"
#include "evas_gradient_private.h"


typedef struct _Rectangular_Data   Rectangular_Data;
struct _Rectangular_Data
{
   float  r0;

   int    sx, sy, s;
   float  off;
   int    len;
};

static void
rectangular_init(void);

static void
rectangular_shutdown(void);

static void
rectangular_init_geom(RGBA_Gradient *gr);

static void
rectangular_setup_geom(RGBA_Gradient *gr);

static void
rectangular_free_geom(void *gdata);

static int
rectangular_has_alpha(RGBA_Gradient *gr, int op);

static int
rectangular_has_mask(RGBA_Gradient *gr, int op);

static int
rectangular_get_map_len(RGBA_Gradient *gr);

static Gfx_Func_Gradient_Fill
rectangular_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa);

static RGBA_Gradient_Type  rectangular = {"rectangular", rectangular_init, rectangular_shutdown,
					  rectangular_init_geom, rectangular_setup_geom, rectangular_free_geom,
					  rectangular_has_alpha, rectangular_has_mask,
					  rectangular_get_map_len, rectangular_get_fill_func};


/** internal functions **/

static void
rectangular_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_pad_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
rectangular_pad_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

RGBA_Gradient_Type  *
evas_common_gradient_rectangular_get(void)
{
    return &rectangular;
}

static void
rectangular_init(void)
{
}

static void
rectangular_shutdown(void)
{
}

static void
rectangular_free_geom(void *gdata)
{
   Rectangular_Data *data = (Rectangular_Data *)gdata;
   if (data) free(data);
}

static void
rectangular_setup_geom(RGBA_Gradient *gr)
{
   Rectangular_Data   *rectangular_data;

   if (!gr || (gr->type.geometer != &rectangular)) return;

   rectangular_data = (Rectangular_Data *)gr->type.gdata;
   if (!rectangular_data) return;
   rectangular_data->sx = gr->fill.w;
   rectangular_data->sy = gr->fill.h;
   rectangular_data->s = rectangular_data->sx;
   if (rectangular_data->sy > rectangular_data->sx)
	rectangular_data->s = rectangular_data->sy;
   rectangular_data->off = gr->map.offset;
   rectangular_data->len = rectangular_data->s - (int)(rectangular_data->s * rectangular_data->r0);
}

static void
rectangular_init_geom(RGBA_Gradient *gr)
{
   Rectangular_Data   *rectangular_data;
   int    err = 1;
   char   *s, *p, key[256];
   float  r0;

   if (!gr || (gr->type.geometer != &rectangular)) return;

   rectangular_data = (Rectangular_Data *)gr->type.gdata;
   if (!rectangular_data)
     {
	rectangular_data = calloc(1, sizeof(Rectangular_Data));
	if (!rectangular_data)  return;
	rectangular_data->r0 = 0.0;
	rectangular_data->sx = 32;
	rectangular_data->sy = 32;
	rectangular_data->s = 32;
	rectangular_data->off = 0.0;
	rectangular_data->len = 32;
     }
   gr->type.gdata = rectangular_data;

   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   r0 = rectangular_data->r0;
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
	rectangular_data->r0 = r0;
     }
   free(s);
}


static int
rectangular_has_alpha(RGBA_Gradient *gr, int op)
{
   Rectangular_Data   *rectangular_data;

   if (!gr || (gr->type.geometer != &rectangular)) return 0;
   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   rectangular_data = (Rectangular_Data *)gr->type.gdata;
   if (!rectangular_data) return 0;
   if (rectangular_data->r0 > 0)
	return 1;
   if ( (gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	return 1;
   return 0;
}

static int
rectangular_has_mask(RGBA_Gradient *gr, int op)
{
   Rectangular_Data   *rectangular_data;

   if (!gr || (gr->type.geometer != &rectangular)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	rectangular_data = (Rectangular_Data *)gr->type.gdata;
	if (!rectangular_data) return 0;
	if (rectangular_data->r0 > 0)
	    return 1;
	if ( (gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	    return 1;
     }
   return 0;
}

static int
rectangular_get_map_len(RGBA_Gradient *gr)
{
   Rectangular_Data   *rectangular_data;

   if (!gr || (gr->type.geometer != &rectangular)) return 0;
   rectangular_data = (Rectangular_Data *)gr->type.gdata;
   if (!rectangular_data) return 0;
   return rectangular_data->len;
}

static Gfx_Func_Gradient_Fill
rectangular_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa)
{
   Rectangular_Data   *rectangular_data;
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &rectangular))
	return sfunc;
   rectangular_data = (Rectangular_Data *)gr->type.gdata;
   if (!rectangular_data) return sfunc;

   rectangular_data->off = gr->map.offset;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;
   switch (gr->fill.spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	   {
	    if (rectangular_data->r0 > 0)
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
	    if (rectangular_data->r0 > 0)
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
	    if (rectangular_data->r0 > 0)
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
	    if (rectangular_data->r0 > 0)
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
	 rectangular_data->off = 0;
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
	    if (rectangular_data->r0 > 0)
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
	    if (rectangular_data->r0 > 0)
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
#define INTERP_256_P2R(a, s, mma, mms, mmd, mmz) \
	    MOV_A2R(a, mma) \
	    MOV_P2R(s, mms, mmz) \
	    INTERP_256_R2R(mma, mms, mmd, mm5)

#define MUL_256_A2R(a, mma, mmd, mmz) \
	MOV_A2R(a, mma) \
	MUL4_256_R2R(mma, mmd)
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
rectangular_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    if (l >= src_len)
	      {
	        int  m = (l % (2 * src_len));

		l = (l % src_len);
		if (m >= src_len)
		    l = src_len - l - 1;
	      }
	    *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
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
	    if (lp >= src_len)
	      {
	        int  m = (lp % (2 * src_len));

		lp = (lp % src_len);
		if (m >= src_len)
		  { lp = src_len - lp - 1;  a = 257 - a; }
	      }
#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];
#endif
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a0, mm3, mm1, mm0)
#else
		*dst = MUL_256(a0, *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    if (l >= src_len)
	      {
	        int  m = (l % (2 * src_len));

		l = (l % src_len);
		if (m >= src_len)
		    l = src_len - l - 1;
	      }
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
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
	    if (lp >= src_len)
	      {
	        int  m = (lp % (2 * src_len));

		lp = (lp % src_len);
		if (m >= src_len)
		  { lp = src_len - lp - 1;  a = 257 - a; }
	      }
#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];
#endif
	    *mask = 255;
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	    if ((l == 0) && rr0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	   *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
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
	    lp = lp % src_len;
	    if (lp < 0)
		lp += src_len;
#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];
#endif
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
	    if (lp == (src_len - 1))
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[0], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[0], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a, mm3, mm1, mm0)
#else
		*dst = MUL_256(a, *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
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
	    lp = lp % src_len;
	    if (lp < 0)
		lp += src_len;
#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];
#endif
	    *mask = 255;
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
	    if (lp == (src_len - 1))
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[0], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[0], *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    if (l < 0) l = -l;
	    if (l >= src_len)
	      {
	        int  m = (l % (2 * src_len));

		l = (l % src_len);
		if (m >= src_len)
		   l = src_len - l - 1;
	      }
	    *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy, lp;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a;

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
	    if (lp >= src_len)
	      {
	        int  m = (lp % (2 * src_len));

		lp = (lp % src_len);
		if (m >= src_len)
		  { lp = src_len - lp - 1;  a = 257 - a; }
	      }
#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];
#endif
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
	    if (l == (src_len - 1))
	      {
#ifdef BUILD_MMX
		a = 257 - a0;
		MUL_256_A2R(a, mm3, mm1, mm0)
#else
		*dst = MUL_256(257 - a0, *dst);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a0, mm3, mm1, mm0)
#else
		*dst = MUL_256(a0, *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                    int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    if (l < 0) l = -l;
	    if (l >= src_len)
	      {
	        int  m = (l % (2 * src_len));

		l = (l % src_len);
		if (m >= src_len)
		   l = src_len - l - 1;
	      }
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy, lp;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a0 = a - 1;

	    lp = l + off;
	    if (lp < 0) { lp = -lp;  a = 257 - a; }
	    if (lp >= src_len)
	      {
	        int  m = (lp % (2 * src_len));

		lp = (lp % src_len);
		if (m >= src_len)
		  { lp = src_len - lp - 1;  a = 257 - a; }
	      }
#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];
#endif
	    *mask = 255;
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	    if (l == (src_len - 1))
		*mask = 255 - a0;
	    if ((l == 0) && rr0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                            int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    l = (l % src_len);
	    if (l < 0)
		l += src_len;
	    *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8), a1 = 257 - a;

	    lp = l + off;
	    lp = (lp % src_len);
	    if (lp < 0)
		lp += src_len;
#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];
#endif
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
	    if (lp == (src_len - 1))
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[0], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[0], *dst);
#endif
	      }
	    if (l == (src_len - 1))
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a1, mm3, mm1, mm0)
#else
		*dst = MUL_256(a1, *dst);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a, mm3, mm1, mm0)
#else
		*dst = MUL_256(a, *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

	l += (ll - (l << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    l = (l % src_len);
	    if (l < 0)
		l += src_len;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = (lp % src_len);
	    if (lp < 0)
		lp += src_len;
#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];
#endif
	    *mask = 255;
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
	    if (lp == (src_len - 1))
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[0], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[0], *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	    if (l == (src_len - 1))
		*mask = 256 - a;
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}


static void
rectangular_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
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
	   if (l >= src_len)
	       l = src_len - 1;
	   *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;
	if (l >= src_len) *dst = src[src_len - 1];
	if ((unsigned)l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

#ifdef BUILD_MMX
	    MOV_P2R(src[l], mm1, mm0)
#else
	    *dst = src[l];
#endif
	    if (l + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[l + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[l + 1], *dst);
#endif
	      }
	    if ((l == 0) && rr0)
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a, mm3, mm1, mm0)
#else
		*dst = MUL_256(a, *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_pad_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
	   if (l >= src_len)
	       l = src_len - 1;
	   *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
rectangular_pad_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Rectangular_Data  *gdata = (Rectangular_Data *)params_data;
   int  xx, yy, rr0;

   SETUP_RECT_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  ll = xx, l = yy;

	if (ll < 0)  ll = -ll;
	if (l < 0)  l = -l;
	if (ll < l)  ll = l;
	ll = ll - rr0;  l = ll >> 16;

	*dst = 0;  *mask = 0;
	if (l >= src_len) { *dst = src[src_len - 1];  *mask = 255; }
	if ((unsigned)l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

#ifdef BUILD_MMX
	    MOV_P2R(src[l], mm1, mm0)
#else
	    *dst = src[l];
#endif
	    *mask = 255;
	    if (l + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[l + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[l + 1], *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}
