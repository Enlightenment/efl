#include "evas_common.h"
#include "evas_blend_private.h"
#include "evas_private.h"
#include "evas_object_image.h"
#include "./evas_image_private.h"
#include <math.h>


/* simple func that assumes a bunch of stuff... */
static void
_evas_common_image_draw_simple(RGBA_Image *dst, RGBA_Draw_Context *dc,
                               RGBA_Image *im, int sx, int sy, int x, int y, int w, int h)
{
   RGBA_Gfx_Func      bfunc;
   int                sw, dw;
   DATA32            *pdst, *dst_end, *src;
//   DATA8            *mask = NULL;
   int                use_col = 1;
   DATA32             col;

   sw = im->cache_entry.w;  dw = dst->cache_entry.w;
   col = dc->mul.col;
   if ((!dc->mul.use) || (dc->mul.col == 0xffffffff))
	use_col = 0;
   if (!use_col)
	bfunc = evas_common_gfx_func_composite_pixel_span_get(im, dst, w, dc->render_op);
   else
	bfunc = evas_common_gfx_func_composite_pixel_color_span_get(im, col, dst, w, dc->render_op);
   if (!bfunc) return;

   src = im->image.data + (sy * im->cache_entry.w) + sx;
   pdst = dst->image.data + (y * dst->cache_entry.w) + x;
   dst_end = pdst + (h * dst->cache_entry.w);

   while (pdst < dst_end)
     {
#ifdef EVAS_SLI
	if ((y % dc->sli.h) == dc->sli.y)
	  {
#endif
	     bfunc(src, NULL, col, pdst, w);
#ifdef EVAS_SLI
	     y++;
	  }
#endif
	src += sw;  pdst += dw;
     }
}

static void
_evas_common_image_draw2(RGBA_Image *dst, RGBA_Draw_Context *dc,
                         void *pim, int x, int y, int w, int h)
{
   Evas_Object_Image *oim = pim;
   RGBA_Image        *im;
   Gfx_Func_Image_Fill      ifunc;
   RGBA_Gfx_Func            bfunc;
   int              xin, yin, xoff, yoff;
   int              clx, cly, clw, clh;
   int              sw, sh, dw;
   DATA32           *pdst, *dst_end, *buf, *src;
   RGBA_Image       argb_buf;
//   DATA8            *mask = NULL;
   Image_Draw_Data  idata;
   int              direct_copy = 0, buf_step = 0;
   int              buf_has_alpha, use_col = 1;
   DATA32           col;

   if (!dc || !oim || !dst || !dst->image.data) return;
   if ((w < 1) || (h < 1)) return;

   im = oim->engine_data;
   if (!im || !im->image.data) return;
   sw = im->cache_entry.w;  sh = im->cache_entry.h;
   if ((sw < 1) || (sh < 1)) return;

   clx = 0;  cly = 0;  clw = dst->cache_entry.w;  clh = dst->cache_entry.h;
   if ((clw < 1) || (clh < 1)) return;

   if (dc->clip.use)
     RECTS_CLIP_TO_RECT(clx,cly,clw,clh, dc->clip.x,dc->clip.y,dc->clip.w,dc->clip.h);
   if ((clw < 1) || (clh < 1)) return;

   xin = x;  yin = y;
   RECTS_CLIP_TO_RECT(x,y,w,h, clx,cly,clw,clh);
   if ((w < 1) || (h < 1)) return;

   if ((oim->cur.fill.spread == _EVAS_TEXTURE_RESTRICT) && (dc->render_op != _EVAS_RENDER_FILL))
     {
	int  x0 = 0, y0 = 0, w0 = 0, h0 = 0;

	/* use one pixel border around fill to account for fuzzies...  */
	evas_common_transform_affine_rect_bounds_get(&oim->cur.fill.transform,
	                                             oim->cur.fill.x - 1, oim->cur.fill.y - 1,
	                                             oim->cur.fill.w + 2, oim->cur.fill.h + 2,
	                                             &x0, &y0, &w0, &h0);
	RECTS_CLIP_TO_RECT(x,y,w,h, x0 + xin,y0 + yin,w0,h0);
	if ((w < 1) || (h < 1)) return;
     }

   xoff = (x - xin);
   yoff = (y - yin);

   /*
     a quick hackism to take care of a simple but somewhat important case,
     will have a better go at this a bit later.
  */
   if ( oim->cur.fill.transform.is_identity && (oim->cur.fill.w == sw) && (oim->cur.fill.h == sh) &&
        (xoff >= oim->cur.fill.x) && (yoff >= oim->cur.fill.y) &&
        ((oim->cur.fill.x + sw) >= (xoff + w)) && ((oim->cur.fill.y + sh) >= (yoff + h)) )
     {
	_evas_common_image_draw_simple(dst, dc, im,
	                               xoff - oim->cur.fill.x, yoff - oim->cur.fill.y, x, y, w, h);
	return;
     }

   if (!evas_common_image_draw_data_setup(oim, &idata)) return;

   buf_has_alpha = im->cache_entry.flags.alpha;
   if ( (oim->cur.fill.spread == _EVAS_TEXTURE_RESTRICT) || (!oim->cur.border.center_fill) )
	buf_has_alpha = 1;

   /* we'll ignore masked stuff for now. */

   dw = dst->cache_entry.w;
   col = dc->mul.col;
   if ((!dc->mul.use) || (dc->mul.col == 0xffffffff))
	use_col = 0;
   if (!use_col && ((dc->render_op == _EVAS_RENDER_FILL) || (dc->render_op == _EVAS_RENDER_COPY)))
     {
	direct_copy = 1;  buf_step = dw;
	if (buf_has_alpha)
	   dst->cache_entry.flags.alpha = 1;
     }
   else if (!use_col && (dc->render_op == _EVAS_RENDER_BLEND) && (!buf_has_alpha))
     {
	direct_copy = 1;  buf_step = dw;
     }

   if (!direct_copy)
     {
	buf = alloca(w * sizeof(DATA32));
	if (!buf) return;
	argb_buf.cache_entry.flags.alpha = buf_has_alpha;
	argb_buf.cache_entry.flags.alpha_sparse = im->cache_entry.flags.alpha_sparse;
	if ( (oim->cur.fill.spread == _EVAS_TEXTURE_RESTRICT) || (!oim->cur.border.center_fill) )
	   argb_buf.cache_entry.flags.alpha_sparse = 1;

	if (!use_col)
	   bfunc = evas_common_gfx_func_composite_pixel_span_get(&argb_buf, dst, w, dc->render_op);
	else
	   bfunc = evas_common_gfx_func_composite_pixel_color_span_get(&argb_buf, col, dst, w, dc->render_op);
	if (!bfunc) return;
     }

   ifunc = evas_common_image_fill_func_get(oim, dc->render_op);
   if (!ifunc) return;

   src = im->image.data;
   pdst = dst->image.data + (y * dw) + x;
   dst_end = pdst + (h * dw);
   if (direct_copy) buf = pdst;

   while (pdst < dst_end)
     {
#ifdef EVAS_SLI
	if ((y % dc->sli.h) == dc->sli.y)
	  {
#endif
	     ifunc(src, sw, sh, buf, NULL, w, xoff, yoff, &idata);
	     if (!direct_copy)
	       bfunc(buf, NULL, col, pdst, w);
#ifdef EVAS_SLI
	     y++;
	  }
#endif
	buf += buf_step;
	pdst += dw;
	yoff++;
     }
}


EAPI void
evas_common_image_render_pre(RGBA_Draw_Context *dc, void *pim)
{
   Evas_Object_Image *oim = pim;
   RGBA_Image *im;

   if (!oim) return;
   im = oim->engine_data;
   if (!im) return;
   if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
     evas_cache_image_load_data(&im->cache_entry);
   evas_common_image_colorspace_normalize(im);
}


EAPI void
evas_common_image_draw2(RGBA_Image *dst, RGBA_Draw_Context *dc,
                        void *pim, int x, int y, int w, int h)
{
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;

   /* handle cutouts here! */

   if (!dc || !dst || !dst->image.data) return;
   if ((w < 1) || (h < 1)) return;
   if (!(RECTS_INTERSECT(x, y, w, h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return;
   /* save out clip info */
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
	_evas_common_image_draw2(dst, dc, pim, x, y, w, h);
     }
   else
     {
	evas_common_draw_context_clip_clip(dc, x, y, w, h);
	/* our clip is 0 size.. abort */
	if ((dc->clip.w > 0) && (dc->clip.h > 0))
	  {
	     rects = evas_common_draw_context_apply_cutouts(dc);
	     for (i = 0; i < rects->active; ++i)
	       {
		  r = rects->rects + i;
		  evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
		  _evas_common_image_draw2(dst, dc, pim, x, y, w, h);
	       }
	     evas_common_draw_context_apply_clear_cutouts(rects);
	  }
     }
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
   evas_common_cpu_end_opt();
}
