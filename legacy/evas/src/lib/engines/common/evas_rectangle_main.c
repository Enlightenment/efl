/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"

static void rectangle_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EAPI void
evas_common_rectangle_init(void)
{
}

EAPI void
evas_common_rectangle_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;
   /* handle cutouts here! */

   if ((w <= 0) || (h <= 0)) return;
   if (!(RECTS_INTERSECT(x, y, w, h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return;
   /* save out clip info */
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
	rectangle_draw_internal(dst, dc, x, y, w, h);
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
		  rectangle_draw_internal(dst, dc, x, y, w, h);
	       }
	     evas_common_draw_context_apply_clear_cutouts(rects);
	  }
     }
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}

static void
rectangle_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   RGBA_Gfx_Func func;
   int yy;
   DATA32 *ptr;

   RECTS_CLIP_TO_RECT(x, y, w, h, dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
   if ((w <= 0) || (h <= 0)) return;

   func = evas_common_gfx_func_composite_color_span_get(dc->col.col, dst, w, dc->render_op);
   ptr = dst->image.data + (y * dst->cache_entry.w) + x;
   for (yy = 0; yy < h; yy++)
     {
#ifdef EVAS_SLI
	if (((yy + y) % dc->sli.h) == dc->sli.y)
#endif
	  {
	     func(NULL, NULL, dc->col.col, ptr, w);
	  }
	ptr += dst->cache_entry.w;
     }
}
