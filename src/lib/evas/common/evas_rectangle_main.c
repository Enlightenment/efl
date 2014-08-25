#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_blend_private.h"

static void rectangle_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

EAPI void
evas_common_rectangle_init(void)
{
}

EAPI void
evas_common_rectangle_draw_cb(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h, Evas_Common_Rectangle_Draw_Cb cb)
{
   static Cutout_Rects *rects = NULL;
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
	cb(dst, dc, x, y, w, h);
     }
   else
     {
	evas_common_draw_context_clip_clip(dc, x, y, w, h);
	/* our clip is 0 size.. abort */
	if ((dc->clip.w > 0) && (dc->clip.h > 0))
	  {
             rects = evas_common_draw_context_apply_cutouts(dc, rects);
	     for (i = 0; i < rects->active; ++i)
	       {
		  r = rects->rects + i;
		  evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
		  cb(dst, dc, x, y, w, h);
	       }
	  }
     }
   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
}

EAPI void
evas_common_rectangle_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   evas_common_rectangle_draw_cb(dst, dc, x, y, w, h, rectangle_draw_internal);
}

EAPI Eina_Bool
evas_common_rectangle_draw_prepare(Cutout_Rects **reuse, const RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   if ((w <= 0) || (h <= 0)) return EINA_FALSE;
   if (!(RECTS_INTERSECT(x, y, w, h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return EINA_FALSE;
   /* save out clip info */
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   /* no cutouts - cut right to the chase */
   if (dc->cutout.rects)
     {
       evas_common_draw_context_clip_clip(dc, x, y, w, h);
       /* our clip is 0 size.. abort */
       if ((dc->clip.w > 0) && (dc->clip.h > 0))
	 *reuse = evas_common_draw_context_apply_cutouts(dc, *reuse);
     }

   return EINA_TRUE;
}

EAPI void
evas_common_rectangle_draw_do(const Cutout_Rects *reuse,
                              const Eina_Rectangle *clip,
                              RGBA_Image *dst, RGBA_Draw_Context *dc,
                              int x, int y, int w, int h)
{
   Eina_Rectangle area;
   Cutout_Rect *r;
   int i;

   if (!reuse)
     {
        evas_common_draw_context_clip_clip(dc,
					   clip->x, clip->y,
					   clip->w, clip->h);
        rectangle_draw_internal(dst, dc, x, y, w, h);
        return;
     }

   for (i = 0; i < reuse->active; ++i)
     {
        r = reuse->rects + i;

        EINA_RECTANGLE_SET(&area, r->x, r->y, r->w, r->h);
        if (!eina_rectangle_intersection(&area, clip)) continue ;
        evas_common_draw_context_set_clip(dc, area.x, area.y, area.w, area.h);
        rectangle_draw_internal(dst, dc, x, y, w, h);
     }
}

static void
rectangle_draw_internal(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   RGBA_Gfx_Func func;
   int yy;
   DATA32 *ptr;

   if (!dst->image.data) return;
   RECTS_CLIP_TO_RECT(x, y, w, h, dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
   if ((w <= 0) || (h <= 0)) return;

#ifdef HAVE_PIXMAN
# ifdef PIXMAN_RECT
   pixman_op_t op = PIXMAN_OP_SRC; // _EVAS_RENDER_COPY
   
   if (dc->render_op == _EVAS_RENDER_BLEND)
     op = PIXMAN_OP_OVER;

   if ((dst->pixman.im) && (dc->col.pixman_color_image))
     {
        pixman_image_composite(op, dc->col.pixman_color_image, NULL, 
                               dst->pixman.im, x, y, 0, 0, 
                               x, y, w, h);
     }
   else
# endif     
#endif
     {
        func = evas_common_gfx_func_composite_color_span_get(dc->col.col, dst, w, dc->render_op);
        ptr = dst->image.data + (y * dst->cache_entry.w) + x;
        for (yy = 0; yy < h; yy++)
          {
	    func(NULL, NULL, dc->col.col, ptr, w);

	    ptr += dst->cache_entry.w;
          }
     }
}

EAPI void
evas_common_rectangle_rgba_draw(RGBA_Image *dst, DATA32 color, int render_op, int x, int y, int w, int h)
{
   RGBA_Gfx_Func func;
   DATA32 *ptr;
   int yy;

   func = evas_common_gfx_func_composite_color_span_get(color, dst, w, render_op);
   ptr = dst->image.data + (y * dst->cache_entry.w) + x;
   for (yy = 0; yy < h; yy++)
     {
        func(NULL, NULL, color, ptr, w);
        ptr += dst->cache_entry.w;
     }
}
