/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <math.h>

#include "evas_common.h"
#include "evas_gradient_private.h"

#define GRAD_LINEAR_DIAG 1
#define GRAD_LINEAR_CODIAG 2

typedef struct _Linear_Data   Linear_Data;
struct _Linear_Data
{
   int    type;

   int    yy0;
   float  ca, sa;
   float  off;
   int    len;

   unsigned char at_angle : 1;
};

static void
linear_init(void);

static void
linear_shutdown(void);

static void
linear_init_geom(RGBA_Gradient *gr);

static void
linear_setup_geom(RGBA_Gradient *gr);

static void
linear_free_geom(void *gdata);

static int
linear_has_alpha(RGBA_Gradient *gr, int op);

static int
linear_has_mask(RGBA_Gradient *gr, int op);

static int
linear_get_map_len(RGBA_Gradient *gr);

static Gfx_Func_Gradient_Fill
linear_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa);

static RGBA_Gradient_Type  linear = {"linear", linear_init, linear_shutdown,
				     linear_init_geom, linear_setup_geom, linear_free_geom,
				     linear_has_alpha, linear_has_mask,
				     linear_get_map_len, linear_get_fill_func};



/** internal functions **/
static void
linear_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);

static void
linear_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data);



RGBA_Gradient_Type  *
evas_common_gradient_linear_get(void)
{
    return &linear;
}

static void
linear_init(void)
{
}

static void
linear_shutdown(void)
{
}

static void
linear_free_geom(void *gdata)
{
   Linear_Data *data = (Linear_Data *)gdata;
   if (data) free(data);
}

static void
linear_init_geom(RGBA_Gradient *gr)
{
   Linear_Data *linear_data;

   if (!gr || (gr->type.geometer != &linear)) return;
   linear_data = (Linear_Data *)gr->type.gdata;
   if (!linear_data)
     {
	linear_data = calloc(1, sizeof(Linear_Data));
	if (!linear_data)  return;
	linear_data->type = 0;
	linear_data->yy0 = 0;
	linear_data->ca = 1.0;
	linear_data->sa = 0.0;
	linear_data->off = 0.0;
	linear_data->len = 32;
	linear_data->at_angle = 0;
	gr->type.gdata = linear_data;
     }

   linear_data->type = 0;
   if (gr->type.name && !strcmp(gr->type.name, "linear.diag"))
	linear_data->type = GRAD_LINEAR_DIAG;
   else if (gr->type.name && !strcmp(gr->type.name, "linear.codiag"))
	linear_data->type = GRAD_LINEAR_CODIAG;
}

static void
linear_setup_geom(RGBA_Gradient *gr)
{
   Linear_Data *linear_data;
   float  angle, a, ca, sa;
   int    xoff = 0, yoff = 0;

   if (!gr || (gr->type.geometer != &linear)) return;
   linear_data = (Linear_Data *)gr->type.gdata;
   if (!linear_data) return;

   angle = gr->map.angle;
   if (linear_data->type == GRAD_LINEAR_DIAG)
	angle += ((atan2(gr->fill.h - 1, gr->fill.w - 1) * 180.0) / M_PI) - 90.0;
   else if (linear_data->type == GRAD_LINEAR_CODIAG)
	angle -= ((atan2(gr->fill.h - 1, gr->fill.w - 1) * 180.0) / M_PI) - 90.0;
   a = ((angle + 90.0) * M_PI) / 180.0;
   ca = cos(a);
   sa = sin(a);
   if ((sa >= 0) && (ca <= 0))
     {
	ca = -ca;
	xoff = -(gr->fill.w - 1);
     }
   else if ((sa <= 0) && (ca <= 0))
     {
	ca = -ca;  sa = -sa;
	xoff = -(gr->fill.w - 1);
	yoff = -(gr->fill.h - 1);
     }
   else if ((sa <= 0) && (ca >= 0))
     {
	sa = -sa;
	yoff = -(gr->fill.h - 1);
     }
   linear_data->len = 0.9961 + (ca * gr->fill.w) + (sa * gr->fill.h);

   a = (angle * M_PI) / 180.0;
   linear_data->ca = cos(a);
   linear_data->sa = sin(a);

   linear_data->yy0 = (linear_data->ca * (yoff << 16)) - (linear_data->sa * (xoff << 16));
   linear_data->off = gr->map.offset;

   if ((linear_data->ca != 1.0) || (linear_data->sa != 0.0))
	linear_data->at_angle = 1;
}

static int
linear_has_alpha(RGBA_Gradient *gr, int op)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if ( (gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
         (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	return 1;
   return 0;
}

static int
linear_has_mask(RGBA_Gradient *gr, int op)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if ( (gr->fill.spread == _EVAS_TEXTURE_RESTRICT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REFLECT) ||
	      (gr->fill.spread == _EVAS_TEXTURE_RESTRICT_REPEAT) )
	    return 1;
     }
   return 0;
}

static int
linear_get_map_len(RGBA_Gradient *gr)
{
   Linear_Data   *linear_data;

   if (!gr || (gr->type.geometer != &linear)) return 0;
   linear_data = (Linear_Data *)gr->type.gdata;
   if (!linear_data) return 0;
   return linear_data->len;
}

static Gfx_Func_Gradient_Fill
linear_get_fill_func(RGBA_Gradient *gr, int op, unsigned char aa)
{
   Linear_Data   *linear_data;
   Gfx_Func_Gradient_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &linear))
	return sfunc;
   linear_data = (Linear_Data *)gr->type.gdata;
   if (!linear_data) return sfunc;

   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) ||
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (gr->fill.spread)
     {
      case _EVAS_TEXTURE_REFLECT:
	{
	 if (aa)
	       sfunc = linear_reflect_aa;
	 else
	       sfunc = linear_reflect;
	}
      break;
      case _EVAS_TEXTURE_REPEAT:
	{
	 if (aa)
	    sfunc = linear_repeat_aa;
	 else
	    sfunc = linear_repeat;
	}
      break;
      case _EVAS_TEXTURE_RESTRICT:
	{
	 if (aa)
	   {
	     if (masked_op)
		sfunc = linear_restrict_aa_masked;
	     else
		sfunc = linear_restrict_aa;
	   }
	 else
	   {
	     if (masked_op)
		sfunc = linear_restrict_masked;
	     else
		sfunc = linear_restrict;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT_REFLECT:
	{
	 if (aa)
	   {
	     if (masked_op)
		sfunc = linear_restrict_reflect_aa_masked;
	     else
		sfunc = linear_restrict_reflect_aa;
	   }
	 else
	   {
	     if (masked_op)
		sfunc = linear_restrict_reflect_masked;
	     else
		sfunc = linear_restrict_reflect;
	   }
	}
      break;
      case _EVAS_TEXTURE_RESTRICT_REPEAT:
	{
	 if (aa)
	   {
	     if (masked_op)
		sfunc = linear_restrict_repeat_aa_masked;
	     else
		sfunc = linear_restrict_repeat_aa;
	   }
	 else
	   {
	     if (masked_op)
		sfunc = linear_restrict_repeat_masked;
	     else
		sfunc = linear_restrict_repeat;
	   }
	}
      break;
      case _EVAS_TEXTURE_PAD:
	{
	 if (aa)
	       sfunc = linear_pad_aa;
	 else
	       sfunc = linear_pad;
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
#define INTERP_256_P2R(a, s, mma, mms, mmd, mmz) \
	    MOV_A2R(a, mma) \
	    MOV_P2R(s, mms, mmz) \
	    INTERP_256_R2R(mma, mms, mmd, mm5)

#define MUL_256_A2R(a, mma, mmd, mmz) \
	MOV_A2R(a, mma) \
	MUL4_256_R2R(mma, mmd)
#endif

#define AXIS_ALIGNED \
   ( ((ayy == 0) || (ayx == 0)) && ((gdata->ca == 0.0) || (gdata->sa == 0.0)) )

#define SETUP_LINEAR_FILL \
   if (gdata->at_angle) \
     { \
	ayx = (-gdata->sa * axx) + (gdata->ca * ayx); \
	ayy = (-gdata->sa * axy) + (gdata->ca * ayy); \
     } \
   yy = (ayx * x) + (ayy * y) + gdata->yy0;


static void
linear_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   SETUP_LINEAR_FILL

   if (ayx == 0)
     {
	DATA32  c;

	y = (yy >> 16) + off;
	if (y < 0)  y = -y;
	if (y >= src_len)
	  {
	    int  m = (y % (2 * src_len));

	    y = (y % src_len);
	    if (m >= src_len)
	        y = src_len - y - 1;
	  }
	c = src[y];
	while (dst < dst_end)
	    *dst++ = c;
	return;
     }

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += ((yy - (l << 16)) >> 15) + off;
	if (l < 0)  l = -l;
	if (l >= src_len)
	  {
	    int  m = (l % (2 * src_len));

	    l = (l % src_len);
	    if (m >= src_len)
	        l = src_len - l - 1;
	  }
	*dst++ = src[l];  yy += ayx;
     }
}

static void
linear_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_reflect(src, src_len, dst, mask, dst_len, x, y,
		       axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);
	int  a = 1 + ((yy - (l << 16)) >> 8);

	l += off;
	if (l < 0) { l = -l;  a = 257 - a; }
	if (l >= src_len)
	  {
	    int  m = (l % (2 * src_len));

	    l = (l % src_len);
	    if (m >= src_len)
	      { l = src_len - l - 1;  a = 257 - a; }
	  }
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
#ifdef BUILD_MMX
	MOV_R2P(mm1, *dst, mm0)
#endif
	dst++;  yy += ayx;
     }
}

static void
linear_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   SETUP_LINEAR_FILL

   if (ayx == 0)
     {
	DATA32  c;

	y = (yy >> 16) + off;
	y = y % src_len;
	if (y < 0)
	   y += src_len;
	c = src[y];
	while (dst < dst_end)
	    *dst++ = c;
	return;
     }

   if (ayy == 0)
     {
	Gfx_Func_Copy  func;
	int  l;

	x = (yy >> 16) + off;
	x = x % src_len;
	if (x < 0)
	   x += src_len;
	if (ayx < 0)
	  {
	    l = x + 1;  x = 0;
	  }
	else
	    l = src_len - x;
	if (l > dst_len) l = dst_len;
	func = evas_common_draw_func_copy_get(1, (ayx < 0 ? -1 : 0));
	func(src + x, dst, l);
	if (l == dst_len) return;
	dst += l;  dst_len -= l;
	l = dst_len / src_len;
	while (l--)
	  {
	    func(src, dst, src_len);
	    dst += src_len;
	  }
	l = dst_len % src_len;
	if (ayx < 0)
	   src += src_len - l;
	func(src, dst, l);
	return;
     }

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += ((yy - (l << 16)) >> 15) + off;
	l = l % src_len;
	if (l < 0)
	   l += src_len;
	*dst++ = src[l];  yy += ayx;
     }
}

static void
linear_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_repeat(src, src_len, dst, mask, dst_len, x, y,
		      axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);
	int  a = 1 + ((yy - (l << 16)) >> 8);

	l += off;
	l = l % src_len;
	if (l < 0)
	   l += src_len;
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
	if (l == (src_len - 1))
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
	dst++;  yy += ayx;
     }
}

static void
linear_restrict(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   SETUP_LINEAR_FILL

   if (ayx == 0)
     {
	DATA32  c;

	y = (yy >> 16);
	if ((y < 0) || (y >= src_len))
	  {
	    memset(dst, 0, sizeof(DATA32) * dst_len);
	    return;
	  }
	c = src[y];
	while (dst < dst_end)
	    *dst++ = c;
	return;
     }

   if (ayy == 0)
     {
	Gfx_Func_Copy  func;

	x = yy >> 16;
	ayx = (ayx < 0 ? -1 : 1);
	if ((unsigned)x < src_len)
	  {
	    if ((ayx > 0) && ((src_len - x) >= dst_len))
	      {
		func = evas_common_draw_func_copy_get(dst_len, 0);
		func(src + x, dst, dst_len);
		return;
	      }
	    if ((ayx < 0) && (x >= (dst_len - 1)))
	      {
		func = evas_common_draw_func_copy_get(dst_len, -1);
		func(src + x - (dst_len - 1), dst, dst_len);
		return;
	      }
	  }
	while (dst < dst_end)
	  {
	    *dst = 0;
	    if ((unsigned)x < src_len)
		*dst = src[x];
	    dst++;  x += ayx;
	  }
	return;
     }

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	*dst = 0;
	if ((unsigned)l < src_len)
	    *dst = src[l];
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_restrict(src, src_len, dst, mask, dst_len, x, y,
			axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;
	if ((unsigned)(l + 1) < (src_len + 1))
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8);
	    int    lp = l;

	    if (l == -1) lp = 0;
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
	    if (l == -1)
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a, mm3, mm1, mm0)
#else
		*dst = MUL_256(a, *dst);
#endif
	      }
	    if (l == (src_len - 1))
	      {
#ifdef BUILD_MMX
		a = 257 - a;
		MUL_256_A2R(a, mm3, mm1, mm0)
#else
		*dst = MUL_256(257 - a, *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   SETUP_LINEAR_FILL

   if (ayx == 0)
     {
	DATA32  c;

	y = (yy >> 16);
	if ((y < 0) || (y >= src_len))
	  {
	    memset(dst, 0, sizeof(DATA32) * dst_len);
	    memset(mask, 0, sizeof(DATA8) * dst_len);
	    return;
	  }
	c = src[y];
	while (dst < dst_end)
	  {
	    *dst++ = c;  *mask++ = 255;
	  }
	return;
     }

   if (ayy == 0)
     {
	Gfx_Func_Copy  func;

	x = yy >> 16;
	ayx = (ayx < 0 ? -1 : 1);
	if ((unsigned)x < src_len)
	  {
	    if ((ayx > 0) && ((src_len - x) >= dst_len))
	      {
		func = evas_common_draw_func_copy_get(dst_len, 0);
		func(src + x, dst, dst_len);
		memset(mask, 255, sizeof(DATA8) * dst_len);
		return;
	      }
	    if ((ayx < 0) && (x >= (dst_len - 1)))
	      {
		func = evas_common_draw_func_copy_get(dst_len, -1);
		func(src + x - (dst_len - 1), dst, dst_len);
		memset(mask, 255, sizeof(DATA8) * dst_len);
		return;
	      }
	  }
	while (dst < dst_end)
	  {
	    *dst = 0;  *mask = 0;
	    if ((unsigned)x < src_len)
	      {
		*dst = src[x];  *mask = 255;
	      }
	    dst++;  mask++;  x += ayx;
	  }
	return;
     }

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_restrict_masked(src, src_len, dst, mask, dst_len, x, y,
			       axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)(l + 1) < (src_len + 1))
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8);
	    int    lp = l;

	    if (l == -1) lp = 0;

#ifdef BUILD_MMX
	    MOV_P2R(src[lp], mm1, mm0)
#else
	    *dst = src[lp];  *mask = 255;
#endif
	    if (lp + 1 < src_len)
	      {
#ifdef BUILD_MMX
		INTERP_256_P2R(a, src[lp + 1], mm3, mm2, mm1, mm0)
#else
		*dst = INTERP_256(a, src[lp + 1], *dst);
#endif
	      }
	    if (l == -1)
		*mask = a - 1;
	    if (l == (src_len - 1))
		*mask = 256 - a;
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_reflect(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                        int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   SETUP_LINEAR_FILL

   if (ayx == 0)
     {
	DATA32  c;

	y = (yy >> 16);
	if ((y < 0) || (y >= src_len))
	  {
	    memset(dst, 0, sizeof(DATA32) * dst_len);
	    return;
	  }
	y += off;
	if (y < 0) y = -y;
	if (y >= src_len)
	  {
	    int  m = (y % (2 * src_len));

	    y = (y % src_len);
	    if (m >= src_len)
		y = src_len - y - 1;
	  }
	c = src[y];
	while (dst < dst_end)
	    *dst++ = c;
	return;
     }

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
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
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_restrict_reflect(src, src_len, dst, mask, dst_len, x, y,
				axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;
	if ((unsigned)(l + 1) < (src_len + 1))
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8), a0 = a;
	    int    lp = l + off;

	    if (l == -1) lp = off;
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
	    if (l == -1)
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a0, mm3, mm1, mm0)
#else
		*dst = MUL_256(a0, *dst);
#endif
	      }
	    if (l == (src_len - 1))
	      {
#ifdef BUILD_MMX
		a0 = 257 - a0;
		MUL_256_A2R(a0, mm3, mm1, mm0)
#else
		*dst = MUL_256(257 - a0, *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_reflect_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                               int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   SETUP_LINEAR_FILL

   if (ayx == 0)
     {
	DATA32  c;

	y = (yy >> 16);
	if ((y < 0) || (y >= src_len))
	  {
	    memset(dst, 0, sizeof(DATA32) * dst_len);
	    memset(mask, 0, sizeof(DATA8) * dst_len);
	    return;
	  }
	y += off;
	if (y < 0) y = -y;
	if (y >= src_len)
	  {
	    int  m = (y % (2 * src_len));

	    y = (y % src_len);
	    if (m >= src_len)
		y = src_len - y - 1;
	  }
	c = src[y];
	while (dst < dst_end)
	  { *dst++ = c;  *mask++ = 255; }
	return;
     }

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += ((yy - (l << 16)) >> 15);
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
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_reflect_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                  int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_restrict_reflect_masked(src, src_len, dst, mask, dst_len, x, y,
				       axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)(l + 1) < (src_len + 1))
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8), a0 = a - 1;
	    int    lp = l + off;

	    if (l == -1) lp = off;
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
	    if (l == -1)
		*mask = a0;
	    if (l == (src_len - 1))
		*mask = 255 - a0;
	  }
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_repeat(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                       int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   SETUP_LINEAR_FILL

   if (ayx == 0)
     {
	DATA32  c;

	y = (yy >> 16);
	if ((y < 0) || (y >= src_len))
	  {
	    memset(dst, 0, sizeof(DATA32) * dst_len);
	    return;
	  }
	y += off;
	y = y % src_len;
	if (y < 0)
	   y += src_len;
	c = src[y];
	while (dst < dst_end)
	    *dst++ = c;
	return;
     }

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	*dst = 0;
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_restrict_repeat(src, src_len, dst, mask, dst_len, x, y,
			       axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;
	if ((unsigned)(l + 1) < (src_len + 1))
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8);
	    int    lp = l + off;

	    if (l == -1) lp = off;
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
	    if (l == -1)
	      {
#ifdef BUILD_MMX
		MUL_256_A2R(a, mm3, mm1, mm0)
#else
		*dst = MUL_256(a, *dst);
#endif
	      }
	    if (l == (src_len - 1))
	      {
#ifdef BUILD_MMX
		a = 257 - a;
		MUL_256_A2R(a, mm3, mm1, mm0)
#else
		*dst = MUL_256(257 - a, *dst);
#endif
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	  }
	dst++;  yy += ayx;
     }
}

static void
linear_restrict_repeat_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   SETUP_LINEAR_FILL

   if (ayx == 0)
     {
	DATA32  c;

	y = (yy >> 16);
	if ((y < 0) || (y >= src_len))
	  {
	    memset(dst, 0, sizeof(DATA32) * dst_len);
	    memset(mask, 0, sizeof(DATA8) * dst_len);
	    return;
	  }
	y += off;
	y = y % src_len;
	if (y < 0)
	   y += src_len;
	c = src[y];
	while (dst < dst_end)
	  { *dst++ = c;  *mask++ = 255; }
	return;
     }

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += ((yy - (l << 16)) >> 15);
	*dst = 0;  *mask = 0;
	if ((unsigned)l < src_len)
	  {
	    l += off;
	    l = l % src_len;
	    if (l < 0)
		l += src_len;
	    *dst = src[l];  *mask = 255;
	  }
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_restrict_repeat_aa_masked(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                                 int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      off = gdata->off * (src_len - 1);
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_restrict_repeat_masked(src, src_len, dst, mask, dst_len, x, y,
				      axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	*dst = 0;  *mask = 0;
	if ((unsigned)(l + 1) < (src_len + 1))
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8);
	    int    lp = l + off;

	    if (l == -1) lp = off;
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
	    if (l == -1)
		*mask = a - 1;
	    if (l == (src_len - 1))
		*mask = 256 - a;
	  }
	dst++;  mask++;  yy += ayx;
     }
}

static void
linear_pad(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
           int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   SETUP_LINEAR_FILL

   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	l += (yy - (l << 16)) >> 15;
	if (l < 0) l = 0;
	if (l >= src_len) l = src_len - 1;
	*dst = src[l];
	dst++;  yy += ayx;
     }
}

static void
linear_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, int axx, int axy, int ayx, int ayy, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   if (AXIS_ALIGNED)
     {
	linear_pad(src, src_len, dst, mask, dst_len, x, y,
		   axx, axy, ayx, ayy, params_data);
	return;
     }

   SETUP_LINEAR_FILL

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);

	if (l < 0) *dst = src[0];
	if (l >= src_len) *dst = src[src_len - 1];
	if ((unsigned)l < src_len)
	  {
	    DATA32 a = 1 + ((yy - (l << 16)) >> 8);

#ifdef BUILD_MMX
	    MOV_P2R(src[l], mm1, mm0)
#else
	    *dst = src[l];
#endif
	    if (l && (l + 1 < src_len))
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
	  }
	dst++;  yy += ayx;
     }
}
