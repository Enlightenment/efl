#include "evas_common.h"
#include "evas_private.h"
#include <math.h>

#define RADIAL_EPSILON 0.000030517578125

typedef struct _Radial_Data   Radial_Data;

struct _Radial_Data
{
   int    axx, axy;
   int    ayx, ayy;
   float  cx, cy, rx, ry;
   float  cx0, cy0;
   int    len;
};


static void 
radial_init(void);

static void 
radial_shutdown(void);

static void 
radial_init_geom(RGBA_Gradient2 *gr);

static void 
radial_update_geom(RGBA_Gradient2 *gr);

static void 
radial_free_geom(void *gdata);

static int 
radial_has_alpha(RGBA_Gradient2 *gr, int op);

static int 
radial_has_mask(RGBA_Gradient2 *gr, int op);

static int 
radial_get_map_len(RGBA_Gradient2 *gr);

static Gfx_Func_Gradient2_Fill 
radial_get_fill_func(RGBA_Gradient2 *gr, int op);

static RGBA_Gradient2_Type  radial = {"radial", radial_init, radial_shutdown,
				      radial_init_geom,
				      radial_update_geom, radial_free_geom,
				      radial_has_alpha, radial_has_mask,
				      radial_get_map_len, radial_get_fill_func};


EAPI RGBA_Gradient2_Type  *
evas_common_gradient2_type_radial_get(void)
{
    return &radial;
}

EAPI RGBA_Gradient2 *
evas_common_gradient2_radial_new(void)
{
   RGBA_Gradient2 *gr;

   gr = calloc(1, sizeof(RGBA_Gradient2));
   if (!gr) return NULL;
   gr->references = 1;
   gr->type.id = MAGIC_OBJ_GRADIENT_RADIAL;
   gr->type.geometer = &radial;
   radial_init_geom(gr);
   return gr;
}

EAPI void
evas_common_gradient2_radial_fill_set(RGBA_Gradient2 *gr, float cx, float cy, float rx, float ry)
{
   Radial_Data *radial_data;

   if (!gr) return;
   if (gr->type.id != MAGIC_OBJ_GRADIENT_RADIAL) return;
   if (gr->type.geometer != &radial) return;
   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return;
   if (rx < 0) rx = -rx;  if (ry < 0) ry = -ry;
   radial_data->cx = cx;  radial_data->cy = cy;
   radial_data->rx = 1 + rx;  radial_data->ry = 1 + ry;
}


/** internal functions **/

static void
radial_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                  int x, int y, void *params_data);

static void
radial_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                 int x, int y, void *params_data);

static void
radial_restrict_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                   int x, int y, void *params_data);

static void
radial_restrict_masked_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, void *params_data);

static void
radial_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
              int x, int y, void *params_data);


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
radial_init_geom(RGBA_Gradient2 *gr)
{
   Radial_Data   *radial_data;

   if (!gr || (gr->type.geometer != &radial)) return;

   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data)
     {
	radial_data = calloc(1, sizeof(Radial_Data));
	if (!radial_data)  return;
	radial_data->cx = radial_data->cy = 0;
	radial_data->rx = radial_data->ry = 0;
	radial_data->axx = 65536;  radial_data->axy = 0;
	radial_data->ayx = 0;  radial_data->ayy = 65536;
	radial_data->len = 0;
    }
   gr->type.gdata = radial_data;
}

static void 
radial_update_geom(RGBA_Gradient2 *gr)
{
   Radial_Data   *radial_data;
   double f, flen;
   double  fx1, fy1;
   int len;

   if (!gr || (gr->type.geometer != &radial)) return;

   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return;

   if ((radial_data->rx < RADIAL_EPSILON) || (radial_data->ry < RADIAL_EPSILON)) return;

   radial_data->len = 0;
   f = (gr->fill.transform.mxx * (double)gr->fill.transform.myy) - (gr->fill.transform.mxy * (double)gr->fill.transform.myx);
   if (fabs(f) < RADIAL_EPSILON) return;

   f = 1.0 / f;
   radial_data->cx0 = (((gr->fill.transform.myy * (double)radial_data->cx) - (gr->fill.transform.mxy * (double)radial_data->cy)) * f) - gr->fill.transform.mxz;
   radial_data->cy0 = ((-(gr->fill.transform.myx * (double)radial_data->cx) + (gr->fill.transform.mxx * (double)radial_data->cy)) * f) - gr->fill.transform.myz;

   fx1 = (gr->fill.transform.myy * (double)radial_data->rx) * f;
   fy1 = (gr->fill.transform.myx * (double)radial_data->rx) * f;

   flen = hypot(fx1, fy1);

   fx1 = (gr->fill.transform.mxy * (double)radial_data->ry) * f;
   fy1 = (gr->fill.transform.mxx * (double)radial_data->ry) * f;

   flen = sqrt(flen * hypot(fx1, fy1));

   radial_data->len = len = flen + 0.5;
   if (!len) return;

   radial_data->axx = (((double)gr->fill.transform.mxx * 65536) * flen) / radial_data->rx;
   radial_data->axy = (((double)gr->fill.transform.mxy * 65536) * flen) / radial_data->rx;

   radial_data->ayx = (((double)gr->fill.transform.myx * 65536) * flen) / radial_data->ry;
   radial_data->ayy = (((double)gr->fill.transform.myy * 65536) * flen) / radial_data->ry;
}

static int
radial_has_alpha(RGBA_Gradient2 *gr, int op)
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
   if (gr->fill.spread == _EVAS_TEXTURE_RESTRICT)
	return 1;
   return 0;
}

static int
radial_has_mask(RGBA_Gradient2 *gr, int op)
{
   Radial_Data   *radial_data;

   if (!gr || (gr->type.geometer != &radial)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	radial_data = (Radial_Data *)gr->type.gdata;
	if (!radial_data) return 0;
	if (gr->fill.spread == _EVAS_TEXTURE_RESTRICT)
	    return 1;
     }
   return 0;
}

static int
radial_get_map_len(RGBA_Gradient2 *gr)
{
   Radial_Data   *radial_data;

   if (!gr || (gr->type.geometer != &radial)) return 0;
   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return 0;
   return radial_data->len;
}

static Gfx_Func_Gradient2_Fill
radial_get_fill_func(RGBA_Gradient2 *gr, int op)
{
   Radial_Data   *radial_data;
   Gfx_Func_Gradient2_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!gr || (gr->type.geometer != &radial)) return sfunc;
   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return sfunc;

   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (gr->fill.spread)
     {
      case _EVAS_TEXTURE_REPEAT:
	sfunc = radial_repeat_aa;
      break;
      case _EVAS_TEXTURE_REFLECT:
	sfunc = radial_reflect_aa;
      break;
      case _EVAS_TEXTURE_RESTRICT:
	if (masked_op)
	   sfunc = radial_restrict_masked_aa;
	else
	   sfunc = radial_restrict_aa;
      break;
      case _EVAS_TEXTURE_PAD:
	sfunc = radial_pad_aa;
      break;
      default:
	sfunc = radial_repeat_aa;
      break;
     }
   return sfunc;
}


static void
radial_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                 int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy;

   evas_common_cpu_end_opt();
   xx = (gdata->axx * (x - gdata->cx0 + 0.5)) + (gdata->axy * (y - gdata->cy0 + 0.5));
   yy = (gdata->ayx * (x - gdata->cx0 + 0.5)) + (gdata->ayy * (y - gdata->cy0 + 0.5));

   while (dst < dst_end)
     {
	unsigned int  ll = hypot(xx, yy);
	unsigned int  l = (ll >> 16);
	int  a = 1 + ((ll & 0xffff) >> 8);

	if (l >= src_len)
	    l = l % src_len;
	*dst = src[l];
	if (l + 1 < src_len)
	  {
	    *dst = INTERP_256(a, src[l + 1], *dst);
	  }
	if (l == (src_len - 1))
	  {
	    *dst = INTERP_256(a, src[0], *dst);
	  }
	dst++;  xx += gdata->axx;  yy += gdata->ayx;
     }
}


static void
radial_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                  int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy;

   evas_common_cpu_end_opt();
   xx = (gdata->axx * (x - gdata->cx0 + 0.5)) + (gdata->axy * (y - gdata->cy0 + 0.5));
   yy = (gdata->ayx * (x - gdata->cx0 + 0.5)) + (gdata->ayy * (y - gdata->cy0 + 0.5));

   while (dst < dst_end)
     {
	unsigned int  ll = hypot(xx, yy);
	unsigned int  l = (ll >> 16);
	int  a = 1 + ((ll & 0xffff) >> 8);

	if (l >= src_len)
	  {
	    int  m = (l % (2 * src_len));

	    l = (l % src_len);
	    if (m >= src_len)
	      { l = src_len - l - 1;  a = 257 - a; }
	  }
	*dst = src[l];
	if (l + 1 < src_len)
	    *dst = INTERP_256(a, src[l + 1], *dst);

	dst++;  xx += gdata->axx;  yy += gdata->ayx;
     }
}


static void
radial_restrict_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
                   int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy;

   evas_common_cpu_end_opt();
   xx = (gdata->axx * (x - gdata->cx0 + 0.5)) + (gdata->axy * (y - gdata->cy0 + 0.5));
   yy = (gdata->ayx * (x - gdata->cx0 + 0.5)) + (gdata->ayy * (y - gdata->cy0 + 0.5));

   while (dst < dst_end)
     {
	unsigned int  ll = hypot(xx, yy);
	unsigned int  l = (ll >> 16);

	*dst = 0;
	if (l < src_len)
	  {
	    int a = 1 + ((ll & 0xffff) >> 8);

	    *dst = src[l];
	    if (l + 1 < src_len)
		*dst = INTERP_256(a, src[l + 1], *dst);
	    if (l == (src_len - 1))
	      {
		*dst = INTERP_256(a, src[0], *dst);
		*dst = MUL_256(257 - a, *dst);
	      }
	  }
	dst++;  xx += gdata->axx;  yy += gdata->ayx;
     }
}


static void
radial_restrict_masked_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
                          int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy;

   evas_common_cpu_end_opt();
   xx = (gdata->axx * (x - gdata->cx0 + 0.5)) + (gdata->axy * (y - gdata->cy0 + 0.5));
   yy = (gdata->ayx * (x - gdata->cx0 + 0.5)) + (gdata->ayy * (y - gdata->cy0 + 0.5));

   while (dst < dst_end)
     {
	unsigned int  ll = hypot(xx, yy);
	unsigned int  l = (ll >> 16);

	*dst = 0;  *mask = 0;
	if (l < src_len)
	  {
	    int a = 1 + ((ll & 0xffff) >> 8);

	    *dst = src[l];  *mask = 255;
	    if (l + 1 < src_len)
		*dst = INTERP_256(a, src[l + 1], *dst);
	    if (l == (src_len - 1))
	      {
		*dst = INTERP_256(a, src[0], *dst);
		*mask = 256 - a;
	      }
	  }
	dst++;  mask++;  xx += gdata->axx;  yy += gdata->ayx;
     }
}


static void
radial_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask __UNUSED__, int dst_len,
              int x, int y, void *params_data)
{
   DATA32  *dst_end = dst + dst_len;
   Radial_Data  *gdata = (Radial_Data *)params_data;
   int  xx, yy;

   evas_common_cpu_end_opt();
   xx = (gdata->axx * (x - gdata->cx0 + 0.5)) + (gdata->axy * (y - gdata->cy0 + 0.5));
   yy = (gdata->ayx * (x - gdata->cx0 + 0.5)) + (gdata->ayy * (y - gdata->cy0 + 0.5));

   while (dst < dst_end)
     {
	unsigned int  ll = hypot(xx, yy);
	unsigned int  l = (ll >> 16);

	*dst = 0;
	if (l < src_len)
	  {
	    int a = 1 + ((ll & 0xffff) >> 8);

	    *dst = src[l];
	    if (l + 1 < src_len)
		*dst = INTERP_256(a, src[l + 1], src[l]);
	  }
	 if (l == 0)
	   {
	     *dst = src[0];
	   }
	 if (l >= src_len)
	   {
	     *dst = src[src_len - 1];
	   }
	dst++;  xx += gdata->axx;  yy += gdata->ayx;
     }
}
