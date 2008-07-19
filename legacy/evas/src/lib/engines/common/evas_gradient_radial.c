/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <math.h>

#include "evas_common.h"
#include "evas_gradient_private.h"


typedef struct _Radial_Data   Radial_Data;
struct _Radial_Data
{
   float  r0;

   int    sx, sy, s;
   float  off;
   int    len;
};

static void
radial_init(void);

static void
radial_shutdown(void);

static void
radial_init_geom(RGBA_Gradient *gr);

static void
radial_setup_geom(RGBA_Gradient *gr);

static void
radial_free_geom(void *gdata);

static int
radial_has_alpha(RGBA_Gradient *gr, int op);

static int
radial_has_mask(RGBA_Gradient *gr, int op);

static int
radial_get_map_len(RGBA_Gradient *gr);

static Gfx_Func_Gradient_Fill
radial_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa);

static RGBA_Gradient_Type  radial = {"radial", radial_init, radial_shutdown,
				     radial_init_geom, radial_setup_geom, radial_free_geom,
				     radial_has_alpha, radial_has_mask,
				     radial_get_map_len, radial_get_fill_func};


/** internal functions **/

static void
radial_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);
static void
radial_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_pad_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
radial_pad_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                             int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);


RGBA_Gradient_Type  *
evas_common_gradient_radial_get(void)
{
    return &radial;
}

static void
radial_init(void)
{
}

static void
radial_shutdown(void)
{
}

static void
radial_free_geom(void *gdata)
{
   Radial_Data *data = (Radial_Data *)gdata;
   if (data) free(data);
}

static void
radial_setup_geom(RGBA_Gradient *gr)
{
   Radial_Data   *radial_data;

   if (!gr || (gr->type.geometer != &radial)) return;

   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return;
   radial_data->sx = gr->fill.w;
   radial_data->sy = gr->fill.h;
   radial_data->s = radial_data->sx;
   if (radial_data->sy > radial_data->sx)
	radial_data->s = radial_data->sy;
   radial_data->off = gr->map.offset;
   radial_data->len = radial_data->s - (int)(radial_data->s * radial_data->r0);
}

static void
radial_init_geom(RGBA_Gradient *gr)
{
   Radial_Data   *radial_data;
   int    err = 1;
   char   *s, *p, key[256];
   float  r0;

   if (!gr || (gr->type.geometer != &radial)) return;

   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data)
     {
	radial_data = calloc(1, sizeof(Radial_Data));
	if (!radial_data)  return;
	radial_data->r0 = 0.0;
	radial_data->sx = 32;
	radial_data->sy = 32;
	radial_data->s = 32;
	radial_data->off = 0.0;
	radial_data->len = 32;
     }
   gr->type.gdata = radial_data;

   if (!gr->type.params || !*(gr->type.params))
	return;

   s = strdup(gr->type.params);
   if (!s) return;

   r0 = radial_data->r0;
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
	radial_data->r0 = r0;
     }
   free(s);
}


static int
radial_has_alpha(RGBA_Gradient *gr, int op)
{
   Radial_Data   *radial_data;

   if (!gr || (gr->type.geometer != &radial)) return 0;
   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return 0;
   if (radial_data->r0 > 0)
	return 1;
   if ( (gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	return 1;
   return 0;
}

static int
radial_has_mask(RGBA_Gradient *gr, int op)
{
   Radial_Data   *radial_data;

   if (!gr || (gr->type.geometer != &radial)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	radial_data = (Radial_Data *)gr->type.gdata;
	if (!radial_data) return 0;
	if (radial_data->r0 > 0)
	    return 1;
	if ( (gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	    return 1;
     }
   return 0;
}

static int
radial_get_map_len(RGBA_Gradient *gr)
{
   Radial_Data   *radial_data;

   if (!gr || (gr->type.geometer != &radial)) return 0;
   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return 0;
   return radial_data->len;
}

static Gfx_Func_Gradient_Fill
radial_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa)
{
   Radial_Data   *radial_data;
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &radial)) return sfunc;
   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return sfunc;

   radial_data->off = gr->map.offset;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (gr->fill.spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	   {
	     if (radial_data->r0 > 0)
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
	     if (radial_data->r0 > 0)
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
	     if (radial_data->r0 > 0)
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
	     if (radial_data->r0 > 0)
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
	 radial_data->off = 0;
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
radial_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
radial_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    if (lp >= src_len)
	      {
	        int  m = (lp % (2 * src_len));

		lp = (lp % src_len);
		if (m >= src_len)
		  { lp = src_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = src[lp];
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if ((l == 0) && rr0)
		*dst = MUL_256(a0, *dst);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                      int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
radial_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                         int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    if (lp >= src_len)
	      {
	        int  m = (lp % (2 * src_len));

		lp = (lp % src_len);
		if (m >= src_len)
		  { lp = src_len - lp - 1;  a = 257 - a; }
	      }
	    *dst = src[lp];  *mask = 255;
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if ((l == 0) && rr0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    lp = lp % src_len;
	    if (lp < 0)
		lp += src_len;
	    *dst = src[lp];
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == src_len - 1)
		*dst = INTERP_256(a, src[0], *dst);
	    if ((l == 0) && rr0)
		*dst = MUL_256(a, *dst);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                     int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

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
	    lp = lp % src_len;
	    if (lp < 0)
		lp += src_len;
	    *dst = src[lp];  *mask = 255;
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == src_len - 1)
		*dst = INTERP_256(a, src[0], *dst);
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

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
radial_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

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
	    *dst = src[lp];
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (l == (src_len - 1))
		*dst = MUL_256(257 - a0, *dst);
	    if ((l == 0) && rr0)
		*dst = MUL_256(a0, *dst);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16);

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
radial_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

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
	    *dst = src[lp];  *mask = 255;
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (l == (src_len - 1))
		*mask = 255 - a0;
	    if ((l == 0) && rr0)
		*mask = a0;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RADIAL_FILL

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
radial_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RADIAL_FILL

   while (dst < dst_end)
     {
	int  ll = (hypot(xx, yy) - rr0);
	int  l = (ll >> 16), lp;

	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    DATA32 a = 1 + ((ll - (l << 16)) >> 8);

	    lp = l + off;
	    lp = (lp % src_len);
	    if (lp < 0)
		lp += src_len;
	    *dst = src[lp];
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == (src_len - 1))
		*dst = INTERP_256(a, src[0], *dst);
	    if (l == (src_len - 1))
		*dst = MUL_256(257 - a, *dst);
	    if ((l == 0) && rr0)
		*dst = MUL_256(a, *dst);
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RADIAL_FILL

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
radial_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy, rr0;
   int  off = gdata->off * (src_len - 1);

   SETUP_RADIAL_FILL

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
	    *dst = src[lp];  *mask = 255;
	    if (lp + 1 < src_len)
		*dst = INTERP_256(a, src[lp + 1], *dst);
	    if (lp == (src_len - 1))
		*dst = INTERP_256(a, src[0], *dst);
	    if (l == (src_len - 1))
		*mask = 256 - a;
	    if ((l == 0) && rr0)
		*mask = a - 1;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
	    if (l >= src_len)
		l = src_len - 1;
	    *dst = src[l];
	  }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
	if ((unsigned)l < src_len)
	  {
	    *dst = src[l];
	    if (l + 1 < src_len)
		*dst = INTERP_256(a, src[l + 1], src[l]);
	  }
	 if (l == 0)
	   {
	     *dst = src[0];
	     if (rr0)
		*dst = MUL_256(a, *dst);
	   }
	 if (l >= src_len)
	   {
	     *dst = src[src_len - 1];
	   }
	dst++;  xx += axx;  yy += ayx;
     }
}

static void
radial_pad_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
	    if (l >= src_len)
		l = src_len - 1;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}

static void
radial_pad_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
	if ((unsigned)l < src_len)
	  {
	    *dst = src[l];
	    if (l + 1 < src_len)
		*dst = INTERP_256(a, src[l + 1], src[l]);
	  }
	if (l == 0)
	  {
	    *dst = src[0];  *mask = 255;
	    if (rr0)
		*mask = a - 1;
	  }
	if (l >= src_len)
	  {
	    *dst = src[src_len - 1];  *mask = 255;
	  }
	dst++;  mask++;  xx += axx;  yy += ayx;
     }
}
