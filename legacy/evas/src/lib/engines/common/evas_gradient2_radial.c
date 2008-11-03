#include "evas_common.h"
#include "evas_private.h"
#include "evas_object_gradient2.h"
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
radial_update_geom(void *pgr);

static void 
radial_free_geom(void *gdata);

static int 
radial_has_alpha(void *pgr, int op);

static int 
radial_has_mask(void *pgr, int op);

static Gfx_Func_Gradient2_Fill 
radial_get_fill_func(void *pgr, int op);

static RGBA_Gradient2_Type  radial = {"radial", radial_init, radial_shutdown,
				      radial_free_geom,
				      radial_has_alpha, radial_has_mask,
				      radial_get_fill_func};


EAPI RGBA_Gradient2_Type  *
evas_common_gradient2_type_radial_get(void)
{
    return &radial;
}

EAPI RGBA_Gradient2 *
evas_common_rgba_gradient2_radial_new(void)
{
   RGBA_Gradient2 *gr;
   Radial_Data *radial_data;

   gr = calloc(1, sizeof(RGBA_Gradient2));
   if (!gr) return NULL;
   radial_data = calloc(1, sizeof(Radial_Data));
   if (!radial_data)  return;
   gr->references = 1;
   gr->type.id = MAGIC_OBJ_GRADIENT_RADIAL;
   gr->type.geometer = &radial;

   radial_data->cx = radial_data->cy = 0;
   radial_data->rx = radial_data->ry = 0;
   radial_data->axx = 65536;  radial_data->axy = 0;
   radial_data->ayx = 0;  radial_data->ayy = 65536;
   radial_data->len = 0;
   gr->type.gdata = radial_data;

   return gr;
}

EAPI void
evas_common_gradient2_radial_render_pre(RGBA_Draw_Context *dc, void *pgr)
{
   Evas_Object_Gradient2 *ogr = pgr;
   Evas_Object_Gradient2_Radial *rgr;
   RGBA_Gradient2 *gr;
   Radial_Data *radial_data;
   float rx, ry;

   if (!dc || !ogr) return;
   rgr = (Evas_Object_Gradient2_Radial *)(ogr);
   if (!rgr) return;
   gr = ogr->engine_data;
   if (!gr) return;
   if (gr->type.id != MAGIC_OBJ_GRADIENT_RADIAL) return;
   if (gr->type.geometer != &radial) return;
   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return;
   radial_data->cx = rgr->cur.fill.cx;  radial_data->cy = rgr->cur.fill.cy;
   rx = rgr->cur.fill.rx;  ry = rgr->cur.fill.ry;
   if (rx < 0) rx = -rx;  if (ry < 0) ry = -ry;
   radial_data->rx = 1 + rx;  radial_data->ry = 1 + ry;

   radial_update_geom(ogr);

   evas_common_gradient2_map(dc, ogr, radial_data->len);
}

EAPI void
evas_common_gradient2_radial_render_post(void *pgr)
{
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
   Radial_Data *radial_data = (Radial_Data *)gdata;
   if (radial_data) free(radial_data);
}


static void 
radial_update_geom(void *pgr)
{
   Evas_Object_Gradient2 *ogr = pgr;
   RGBA_Gradient2 *gr;
   Radial_Data   *radial_data;
   double f, flen;
   double  fx1, fy1;
   int len;

   if (!ogr) return;
   gr = ogr->engine_data;
   if (!gr || (gr->type.geometer != &radial)) return;

   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return;

   if ((radial_data->rx < RADIAL_EPSILON) || (radial_data->ry < RADIAL_EPSILON)) return;

   radial_data->len = 0;
   f = (ogr->cur.fill.transform.mxx * (double)ogr->cur.fill.transform.myy) - (ogr->cur.fill.transform.mxy * (double)ogr->cur.fill.transform.myx);
   if (fabs(f) < RADIAL_EPSILON) return;

   f = 1.0 / f;
   radial_data->cx0 = (((ogr->cur.fill.transform.myy * (double)radial_data->cx) - (ogr->cur.fill.transform.mxy * (double)radial_data->cy)) * f) - ogr->cur.fill.transform.mxz;
   radial_data->cy0 = ((-(ogr->cur.fill.transform.myx * (double)radial_data->cx) + (ogr->cur.fill.transform.mxx * (double)radial_data->cy)) * f) - ogr->cur.fill.transform.myz;

   fx1 = (ogr->cur.fill.transform.myy * (double)radial_data->rx) * f;
   fy1 = (ogr->cur.fill.transform.myx * (double)radial_data->rx) * f;

   flen = hypot(fx1, fy1);

   fx1 = (ogr->cur.fill.transform.mxy * (double)radial_data->ry) * f;
   fy1 = (ogr->cur.fill.transform.mxx * (double)radial_data->ry) * f;

   flen = sqrt(flen * hypot(fx1, fy1));

   radial_data->len = len = flen + 0.5;
   if (!len) return;

   radial_data->axx = (((double)ogr->cur.fill.transform.mxx * 65536) * flen) / radial_data->rx;
   radial_data->axy = (((double)ogr->cur.fill.transform.mxy * 65536) * flen) / radial_data->rx;

   radial_data->ayx = (((double)ogr->cur.fill.transform.myx * 65536) * flen) / radial_data->ry;
   radial_data->ayy = (((double)ogr->cur.fill.transform.myy * 65536) * flen) / radial_data->ry;
}

static int
radial_has_alpha(void *pgr, int op)
{
   Evas_Object_Gradient2 *ogr = pgr;
   RGBA_Gradient2 *gr;

   if (!ogr) return 0;
   gr = ogr->engine_data;
   if (!gr || (gr->type.geometer != &radial)) return 0;
   if (gr->has_alpha | gr->map.has_alpha)
	return 1;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	return 0;
   if (ogr->cur.fill.spread == _EVAS_TEXTURE_RESTRICT)
	return 1;
   return 0;
}

static int
radial_has_mask(void *pgr, int op)
{
   Evas_Object_Gradient2 *ogr= pgr;
   RGBA_Gradient2 *gr;

   if (!ogr) return 0;
   gr = ogr->engine_data;
   if (!gr || (gr->type.geometer != &radial)) return 0;
   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
     {
	if (ogr->cur.fill.spread == _EVAS_TEXTURE_RESTRICT)
	    return 1;
     }
   return 0;
}

static Gfx_Func_Gradient2_Fill
radial_get_fill_func(void *pgr, int op)
{
   Evas_Object_Gradient2 *ogr = pgr;
   RGBA_Gradient2 *gr;
   Radial_Data   *radial_data;
   Gfx_Func_Gradient2_Fill  sfunc = NULL;
   int masked_op = 0;

   if (!ogr) return 0;
   gr = ogr->engine_data;
   if (!gr || (gr->type.geometer != &radial)) return sfunc;
   radial_data = (Radial_Data *)gr->type.gdata;
   if (!radial_data) return sfunc;

   if ( (op == _EVAS_RENDER_COPY) || (op == _EVAS_RENDER_COPY_REL) || 
         (op == _EVAS_RENDER_MASK) || (op == _EVAS_RENDER_MUL) )
	masked_op = 1;

   switch (ogr->cur.fill.spread)
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
radial_repeat_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
radial_reflect_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
radial_restrict_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
radial_pad_aa(DATA32 *src, int src_len, DATA32 *dst, DATA8 *mask, int dst_len,
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
