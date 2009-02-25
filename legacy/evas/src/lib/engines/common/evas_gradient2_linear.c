#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

#define LINEAR_EPSILON 0.000030517578125
#define LINEAR_INT_TOLERANCE 0.001953125
// 1/512 = 0.001953125 <-- will use this one as our subpixel pos tolerance.
// 1/256 = 0.00390625  <-- though this one would be ok too for our uses.

typedef struct _Linear_Data   Linear_Data;

struct _Linear_Data
{
   float    x0, y0, x1, y1;
   float    fx0, fy0;
   int      ayx, ayy;
   int      len;
   unsigned char int_axis_aligned : 1;
};


static void 
linear_init(void);

static void 
linear_shutdown(void);

static void 
linear_init_geom(RGBA_Gradient2 *gr);

static void
linear_update_geom(RGBA_Gradient2 *gr);

static void 
linear_free_geom(void *gdata);

static int
linear_has_alpha(RGBA_Gradient2 *gr, int op);

static int
linear_has_mask(RGBA_Gradient2 *gr, int op);

static int
linear_get_map_len(RGBA_Gradient2 *gr);

static Gfx_Func_Gradient2_Fill
linear_get_fill_func(RGBA_Gradient2 *gr, int op);

static RGBA_Gradient2_Type  linear = {"linear", linear_init, linear_shutdown,
				      linear_init_geom,
				      linear_update_geom, linear_free_geom,
				      linear_has_alpha, linear_has_mask,
				      linear_get_map_len, linear_get_fill_func};


EAPI RGBA_Gradient2_Type  *
evas_common_gradient2_type_linear_get(void)
{
    return &linear;
}

EAPI RGBA_Gradient2 *
evas_common_gradient2_linear_new(void)
{
   RGBA_Gradient2 *gr;

   gr = calloc(1, sizeof(RGBA_Gradient2));
   if (!gr) return NULL;
   gr->references = 1;
   gr->type.id = MAGIC_OBJ_GRADIENT_LINEAR;
   gr->type.geometer = &linear;
   linear_init_geom(gr);
   return gr;
}

EAPI void
evas_common_gradient2_linear_fill_set(RGBA_Gradient2 *gr, float x0, float y0, float x1, float y1)
{
   Linear_Data *linear_data;

   if (!gr) return;
   if (gr->type.id != MAGIC_OBJ_GRADIENT_LINEAR) return;
   if (gr->type.geometer != &linear) return;
   linear_data = (Linear_Data *)gr->type.gdata;
   if (!linear_data) return;
   linear_data->x0 = x0;  linear_data->y0 = y0;
   linear_data->x1 = x1;  linear_data->y1 = y1;
}



/** internal functions **/

static void
linear_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, void *params_data);

static void
linear_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, void *params_data);

static void
linear_restrict_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, void *params_data);

static void
linear_restrict_masked_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, void *params_data);

static void
linear_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, void *params_data);



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
linear_init_geom(RGBA_Gradient2 *gr)
{
   Linear_Data *linear_data;

   if (!gr || (gr->type.geometer != &linear)) return;
   linear_data = (Linear_Data *)gr->type.gdata;
   if (!linear_data)
     {
	linear_data = calloc(1, sizeof(Linear_Data));
	if (!linear_data)  return;
	linear_data->ayy = 65536;  linear_data->ayx = 0;
     }
   gr->type.gdata = linear_data;
}


static void
linear_update_geom(RGBA_Gradient2 *gr)
{
   Linear_Data *linear_data;
   double f;
   double fx0, fy0, fx1, fy1;
   int len;

   if (!gr || (gr->type.geometer != &linear)) return;
   linear_data = (Linear_Data *)gr->type.gdata;
   if (!linear_data) return;

   linear_data->int_axis_aligned = 0;
   linear_data->len = 0;
   f = (gr->fill.transform.mxx * (double)gr->fill.transform.myy) - (gr->fill.transform.mxy * (double)gr->fill.transform.myx);
   if (fabs(f) < LINEAR_EPSILON) return;

   f = 1.0 / f;
   fx0 = (((gr->fill.transform.myy * (double)linear_data->x0) - (gr->fill.transform.mxy * (double)linear_data->y0)) * f) - gr->fill.transform.mxz;
   fy0 = ((-(gr->fill.transform.myx * (double)linear_data->x0) + (gr->fill.transform.mxx * (double)linear_data->y0)) * f) - gr->fill.transform.myz;

   fx1 = (((gr->fill.transform.myy * (double)linear_data->x1) - (gr->fill.transform.mxy * (double)linear_data->y1)) * f) - gr->fill.transform.mxz;
   fy1 = ((-(gr->fill.transform.myx * (double)linear_data->x1) + (gr->fill.transform.mxx * (double)linear_data->y1)) * f) - gr->fill.transform.myz;

   f = hypot(fx1 - fx0, fy1 - fy0);
   linear_data->len = len = f + 0.5;
   if (!len) return;

   linear_data->ayx = ((fx1 - fx0) / f) * 65536;
   linear_data->ayy = ((fy1 - fy0) / f) * 65536;

   if (fabs(fy0 - fy1) < LINEAR_INT_TOLERANCE)
     {
	if ( (fabs(((int)fy0) - fy0) < LINEAR_INT_TOLERANCE) &&
	    (fabs(((int)fy1) - fy1) < LINEAR_INT_TOLERANCE) )
	   { linear_data->int_axis_aligned = 1;  linear_data->ayy = 0; }
     }
   else if (fabs(fx0 - fx1) < LINEAR_INT_TOLERANCE)
     {
	if ( (fabs(((int)fx0) - fx0) < LINEAR_INT_TOLERANCE) &&
	    (fabs(((int)fx1) - fx1) < LINEAR_INT_TOLERANCE) )
	   { linear_data->int_axis_aligned = 1;  linear_data->ayx = 0; }
     }
   linear_data->fx0 = fx0;
   linear_data->fy0 = fy0;
}

static int
linear_has_alpha(RGBA_Gradient2 *gr, int op)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if (gr->fill.spread == _EVAS_TEXTURE_RESTRICT)
	return 1;
   return 0;
}

static int
linear_has_mask(RGBA_Gradient2 *gr, int op)
{
   if (!gr || (gr->type.geometer != &linear)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if (gr->fill.spread == _EVAS_TEXTURE_RESTRICT)
	    return 1;
     }
   return 0;
}

static int
linear_get_map_len(RGBA_Gradient2 *gr)
{
   Linear_Data   *linear_data;

   if (!gr || (gr->type.geometer != &linear)) return 0;
   linear_data = (Linear_Data *)gr->type.gdata;
   if (!linear_data) return 0;
   return linear_data->len;
}

static Gfx_Func_Gradient2_Fill
linear_get_fill_func(RGBA_Gradient2 *gr, int op)
{
   Linear_Data   *linear_data;
   Gfx_Func_Gradient2_Fill  sfunc = NULL;
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
      case _EVAS_TEXTURE_REPEAT:
	sfunc = linear_repeat_aa;
      break;
      case _EVAS_TEXTURE_REFLECT:
	sfunc = linear_reflect_aa;
      break;
      case _EVAS_TEXTURE_RESTRICT:
	if (masked_op)
	   sfunc = linear_restrict_masked_aa;
	else
	   sfunc = linear_restrict_aa;
      break;
      case _EVAS_TEXTURE_PAD:
	sfunc = linear_pad_aa;
      break;
      default:
	sfunc = linear_repeat_aa;
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

static void
linear_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                 int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   evas_common_cpu_end_opt();
   yy = (gdata->ayx * (x - gdata->fx0 + 0.5)) + (gdata->ayy * (y - gdata->fy0 + 0.5)) - 32768;

   if (gdata->int_axis_aligned && (gdata->ayx == 0))
     {
	DATA32  c;

	y = (yy >> 16);
	y = y % src_len;
	if (y < 0)
	   y += src_len;
	c = src[y];
	while (dst < dst_end)
	    *dst++ = c;
	return;
     }

   if (gdata->int_axis_aligned && (gdata->ayy == 0))
     {
	Gfx_Func_Copy  func;
	int  l;

	x = (yy >> 16);
	x = x % src_len;
	if (x < 0)
	   x += src_len;
	if (gdata->ayx < 0)
	  {
	    l = x + 1;  x = 0;
	  }
	else
	    l = src_len - x;
	if (l > dst_len) l = dst_len;
	func = evas_common_draw_func_copy_get(1, (gdata->ayx < 0 ? -1 : 0));
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
	if (gdata->ayx < 0)
	   src += src_len - l;
	func(src, dst, l);
	return;
     }

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);
	int  a = 1 + ((yy & 0xffff) >> 8);

	if ((l >= src_len) || (l < 0))
	   { l = l % src_len;  if (l < 0) l += src_len; }

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
	dst++;  yy += gdata->ayx;
     }
}


static void
linear_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                  int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   evas_common_cpu_end_opt();
   yy = (gdata->ayx * (x - gdata->fx0 + 0.5)) + (gdata->ayy * (y - gdata->fy0 + 0.5)) - 32768;

   if (gdata->int_axis_aligned && (gdata->ayx == 0))
     {
	DATA32  c;

	y = (yy >> 16);
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

   if (gdata->int_axis_aligned && (gdata->ayy == 0))
     {
	Gfx_Func_Copy  func, ofunc;
	int  l, sgn;

	x = (yy >> 16);
	sgn = (gdata->ayx < 0 ? -1 : 1);
	if (x < 0)
	  {
	    x = -x;  sgn *= -1;
	  }
	if (x >= src_len)
	  {
	    int  m = (x % (2 * src_len));

	    x = (x % src_len);
	    if (m >= src_len)
	      { x = src_len - x - 1;  sgn *= -1; }
	  }

	if (sgn < 0)
	  {
	    l = x + 1;  x = 0;
	  }
	else
	    l = src_len - x;

	if (l > dst_len) l = dst_len;
	func = evas_common_draw_func_copy_get(1, 0);
	ofunc = evas_common_draw_func_copy_get(1, -1);

	if (sgn > 0)
	   func(src + x, dst, l);
	else
	   ofunc(src + x, dst, l);
	if (l == dst_len) return;

	dst += l;  dst_len -= l;
	l = dst_len / src_len;
	sgn *= -1;
	while (l--)
	  {
	    if (sgn > 0)
	       func(src, dst, src_len);
	    else
	       ofunc(src, dst, src_len);
	    sgn *= -1;
	    dst += src_len;
	  }
	l = dst_len % src_len;
	if (sgn < 0)
	  {
	    src += src_len - l;
	    ofunc(src, dst, l);
	  }
	else
	   func(src, dst, l);
	return;
     }

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (dst < dst_end)
     {
	int  l = (yy >> 16);
	int  a = 1 + ((yy & 0xffff) >> 8);

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
	dst++;  yy += gdata->ayx;
     }
}


static void
linear_restrict_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                   int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   evas_common_cpu_end_opt();
   yy = (gdata->ayx * (x - gdata->fx0 + 0.5)) + (gdata->ayy * (y - gdata->fy0 + 0.5)) - 32768;

   if (gdata->int_axis_aligned && (gdata->ayx == 0))
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

   if (gdata->int_axis_aligned && (gdata->ayy == 0))
     {
	Gfx_Func_Copy  func;
	int sgn;

	x = yy >> 16;
	sgn = (gdata->ayx < 0 ? -1 : 1);
	if ((unsigned)x < src_len)
	  {
	    if ((sgn > 0) && ((src_len - x) >= dst_len))
	      {
		func = evas_common_draw_func_copy_get(dst_len, 0);
		func(src + x, dst, dst_len);
		return;
	      }
	    if ((sgn < 0) && (x >= (dst_len - 1)))
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
	    dst++;  x += sgn;
	  }
	return;
     }

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
	    int  a = 1 + ((yy & 0xffff) >> 8);
	    int  lp = l;

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
	  }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	dst++;  yy += gdata->ayx;
     }
}


static void
linear_restrict_masked_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   evas_common_cpu_end_opt();
   yy = (gdata->ayx * (x - gdata->fx0 + 0.5)) + (gdata->ayy * (y - gdata->fy0 + 0.5)) - 32768;

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
	    int  a = 1 + ((yy & 0xffff) >> 8);
	    int  lp = l;

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
#ifdef BUILD_MMX
	    MOV_R2P(mm1, *dst, mm0)
#endif
	    if (l == -1)
		*mask = a - 1;
	    if (l == (src_len - 1))
		*mask = 256 - a;
	  }
	dst++;  mask++;  yy += gdata->ayx;
     }
}


static void
linear_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
              int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Linear_Data  *gdata = (Linear_Data *)params_data;
   int      yy;

   evas_common_cpu_end_opt();
   yy = (gdata->ayx * (x - gdata->fx0 + 0.5)) + (gdata->ayy * (y - gdata->fy0 + 0.5)) - 32768;

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
	    int  a = 1 + ((yy & 0xffff) >> 8);

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
	dst++;  yy += gdata->ayx;
     }
}
