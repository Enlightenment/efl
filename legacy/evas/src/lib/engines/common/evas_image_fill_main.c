#include "evas_common.h"
#include "evas_private.h"
#include "evas_object_image.h"
#include "./evas_image_private.h"
#include <math.h>

static Gfx_Func_Image_Fill image_repeat_affine_get(void *pim, int render_op);
static Gfx_Func_Image_Fill image_reflect_affine_get(void *pim, int render_op);
static Gfx_Func_Image_Fill image_restrict_affine_get(void *pim, int render_op);
static Gfx_Func_Image_Fill image_restrict_masked_affine_get(void *pim, int render_op);
static Gfx_Func_Image_Fill image_pad_affine_get(void *pim, int render_op);

int
evas_common_image_draw_data_setup(void *pim, Image_Draw_Data *idata)
{
   Evas_Object_Image *oim = pim;
   int fw, fh, sw, sh;
   int l, r, t, b;
   int smooth_scale;

   if (!oim || !idata) return 0;
   sw = oim->cur.image.w;  sh = oim->cur.image.h;
   if ((sw < 1) || (sh < 1)) return 0;

   fw = oim->cur.fill.w;  fh = oim->cur.fill.h;
   if ((fw < 1) || (fh < 1)) return 0;

   evas_common_cpu_end_opt();

   if (fabs( (oim->cur.fill.transform.mxx * (double)oim->cur.fill.transform.myy) - 
             (oim->cur.fill.transform.mxy * (double)oim->cur.fill.transform.myx) ) < 0.000030517578125)
	return 0;

   idata->has_alpha = oim->cur.has_alpha;
   idata->border_center_fill = oim->cur.border.center_fill;
   idata->fw = fw;  idata->fh = fh;

   idata->axx = oim->cur.fill.transform.mxx * 65536;
   idata->axy = oim->cur.fill.transform.mxy * 65536;
   idata->axz = (oim->cur.fill.transform.mxz - oim->cur.fill.x) * 65536;
   idata->ayx = oim->cur.fill.transform.myx * 65536;
   idata->ayy = oim->cur.fill.transform.myy * 65536;
   idata->ayz = (oim->cur.fill.transform.myz - oim->cur.fill.y) * 65536;

   l = oim->cur.border.l;  r = oim->cur.border.r;
   t = oim->cur.border.t;  b = oim->cur.border.b;
   if (l < 0) l = 0;  if (r < 0) r = 0;
   if (t < 0) t = 0;  if (b < 0) b = 0;

   if (((sw == fw) && (sh == fh)) && idata->border_center_fill)
	l = r = t = b = 0;

   if ((l > 0) || (r > 0) || (t > 0) || (b > 0))
     {
	int  bw = MIN(sw, fw);
	int  bh = MIN(sh, fh);

	if ((l + r) > bw)
	  {
	    l = (l + (bw - r) + 1) / 2;
	    r = bw - l;
	  }
	if ((t + b) > bh)
	  {
	    t = (t + (bh - b) + 1) / 2;
	    b = bh - t;
	  }
     }
   idata->l = l;  idata->r = r;
   idata->t = t;  idata->b = b;

   idata->iaxx = idata->iayy = idata->ibxx = idata->ibyy = 65536;
   idata->dax = idata->day = 256;

   if ((sw == fw) && (sh == fh))  // no fill scaling
	return 1;

   if (!oim->cur.smooth_scale || ((sw <= fw) && (sh <= fh)))  // up-x, up-y
     {
	if (((sw - (l + r)) > 1) & ((fw - (l + r)) > 1))
	   idata->iaxx = (((sw - (l + r)) - 1) << 16) / ((fw - (l + r)) - 1);
	else if (((sw - (l + r)) > 0) & ((fw - (l + r)) > 0))
	   idata->iaxx = ((sw - (l + r)) << 16) / (fw - (l + r));
	else
	   idata->iaxx = 0;
	if (((sh - (t + b)) > 1) & ((fh - (t + b)) > 1))
	   idata->iayy = (((sh - (t + b)) - 1) << 16) / ((fh - (t + b)) - 1);
	else if (((sh - (t + b)) > 0) & ((fh - (t + b)) > 0))
	   idata->iayy = ((sh - (t + b)) << 16) / (fh - (t + b));
	else
	   idata->iayy = 0;
	return 1;
     }
   if ((sw > fw) && (sh <= fh))  // down-x, up-y
     {
	if (((sw - (l + r)) > 0) & ((fw - (l + r)) > 0))
	  {
	    idata->iaxx = ((sw - (l + r) + 0.5) * 65536) / (fw - (l + r) + 0.5);
	    idata->ibxx = ((fw - (l + r) + 0.5) * 65536) / (sw - (l + r) + 0.5);
	  }
	else
	  {
	    idata->iaxx = 0;
	    idata->ibxx = 0;
	  }
	idata->dax = idata->ibxx >> 8;
	if (((sh - (t + b)) > 1) & ((fh - (t + b)) > 1))
	   idata->iayy = (((sh - (t + b)) - 1) << 16) / ((fh - (t + b)) - 1);
	else if (((sh - (t + b)) > 0) & ((fh - (t + b)) > 0))
	   idata->iayy = ((sh - (t + b)) << 16) / (fh - (t + b));
	else
	   idata->iayy = 0;
	return 1;
     }
   if ((sw <= fw) && (sh > fh))  // up-x, down-y
     {
	if (((sw - (l + r)) > 1) & ((fw - (l + r)) > 1))
	   idata->iaxx = (((sw - (l + r)) - 1) << 16) / ((fw - (l + r)) - 1);
	else if (((sw - (l + r)) > 0) & ((fw - (l + r)) > 0))
	   idata->iaxx = ((sw - (l + r)) << 16) / (fw - (l + r));
	else
	   idata->iaxx = 0;
	if (((sh - (t + b)) > 0) & ((fh - (t + b)) > 0))
	  {
	    idata->iayy = ((sh - (t + b) + 0.5) * 65536) / (fh - (t + b) + 0.5);
	    idata->ibyy = ((fh - (t + b) + 0.5) * 65536) / (sh - (t + b) + 0.5);
	  }
	else
	  {
	    idata->iayy = 0;
	    idata->ibyy = 0;
	  }
	idata->day = idata->ibyy >> 8;
	return 1;
     }
   if ((sw > fw) && (sh > fh))   // down-x, down-y
     {
	if (((sw - (l + r)) > 0) & ((fw - (l + r)) > 0))
	  {
	    idata->iaxx = ((sw - (l + r) + 0.5) * 65536) / (fw - (l + r) + 0.5);
	    idata->ibxx = ((fw - (l + r) + 0.5) *65536) / (sw - (l + r) + 0.5);
	  }
	else
	  {
	    idata->iaxx = 0;
	    idata->ibxx = 0;
	  }
	idata->dax = idata->ibxx >> 8;
	if (((sh - (t + b)) > 0) & ((fh - (t + b)) > 0))
	  {
	    idata->iayy = ((sh - (t + b) + 0.5) * 65536) / (fh - (t + b) + 0.5);
	    idata->ibyy = ((fh - (t + b) + 0.5) * 65536) / (sh - (t + b) + 0.5);
	  }
	else
	  {
	    idata->iayy = 0;
	    idata->ibyy = 0;
	  }
	idata->day = idata->ibyy >> 8;
	return 1;
     }
   return 0;
}


EAPI Gfx_Func_Image_Fill
evas_common_image_fill_func_get(void *pim, int render_op)
{
  Evas_Object_Image *oim = pim;
  RGBA_Image *im;
  Gfx_Func_Image_Fill  sfunc = NULL;
  int masked_op = 0;
  
  if (!oim) return sfunc;
  im = oim->engine_data;
  if (!im) return sfunc;
  
  if ( (render_op == _EVAS_RENDER_COPY) || (render_op == _EVAS_RENDER_COPY_REL) || 
       (render_op == _EVAS_RENDER_MASK) || (render_op == _EVAS_RENDER_MUL) )
    masked_op = 1;
  
  switch (oim->cur.fill.spread)
    {
    case _EVAS_TEXTURE_REPEAT:
      sfunc = image_repeat_affine_get(oim, render_op);
      break;
    case _EVAS_TEXTURE_REFLECT:
      sfunc = image_reflect_affine_get(oim, render_op);
      break;
    case _EVAS_TEXTURE_RESTRICT:
      if (masked_op)
        sfunc = image_restrict_masked_affine_get(oim, render_op);
      else
        sfunc = image_restrict_affine_get(oim, render_op);
      break;
    case _EVAS_TEXTURE_PAD:
      sfunc = image_pad_affine_get(oim, render_op);
      break;
    default:
      sfunc = image_repeat_affine_get(oim, render_op);
      break;
    }
  return sfunc;
}


/* FIXME: optimize identity transform case for all modes */
/* FIXME: have direct support for 'no-center-fill' for all modes */

#include "./evas_image_fill_restrict.c"
#include "./evas_image_fill_repeat.c"
#include "./evas_image_fill_reflect.c"
#include "./evas_image_fill_pad.c"
