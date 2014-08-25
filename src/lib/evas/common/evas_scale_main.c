#include "evas_common_private.h"
#include "evas_private.h"

EAPI void
evas_common_scale_init(void)
{
}

EAPI Eina_Bool
evas_common_scale_rgba_in_to_out_clip_prepare(Cutout_Rects **reuse, const RGBA_Image *src EINA_UNUSED,
					      const RGBA_Image *dst,
					      RGBA_Draw_Context *dc,
					      int dst_region_x, int dst_region_y,
					      int dst_region_w, int dst_region_h)
{
   /* handle cutouts here! */
   if ((dst_region_w <= 0) || (dst_region_h <= 0)) return EINA_FALSE;
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h,
			 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return EINA_FALSE;
   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects) return EINA_TRUE;

   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   evas_common_draw_context_clip_clip(dc, dst_region_x, dst_region_y, dst_region_w, dst_region_h);
   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     return EINA_FALSE;
   *reuse = evas_common_draw_context_apply_cutouts(dc, *reuse);

   return EINA_TRUE;
}

EAPI Eina_Bool
evas_common_scale_rgba_in_to_out_clip_cb(RGBA_Image *src, RGBA_Image *dst,
                                         RGBA_Draw_Context *dc,
                                         int src_region_x, int src_region_y,
                                         int src_region_w, int src_region_h,
                                         int dst_region_x, int dst_region_y,
                                         int dst_region_w, int dst_region_h,
                                         Evas_Common_Scale_In_To_Out_Clip_Cb cb)
{
   static Cutout_Rects *rects = NULL;
   Cutout_Rect  *r;
   int          c, cx, cy, cw, ch;
   int          i;
   Eina_Bool ret = EINA_FALSE;

   /* handle cutouts here! */
   if ((dst_region_w <= 0) || (dst_region_h <= 0)) return EINA_FALSE;
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return EINA_FALSE;

   /* no cutouts - cut right to the chase */
   if (!dc->cutout.rects)
     {
        return cb(src, dst, dc,
                  src_region_x, src_region_y, src_region_w, src_region_h,
                  dst_region_x, dst_region_y, dst_region_w, dst_region_h);
     }

   /* save out clip info */
   c = dc->clip.use; cx = dc->clip.x; cy = dc->clip.y; cw = dc->clip.w; ch = dc->clip.h;
   evas_common_draw_context_clip_clip(dc, 0, 0, dst->cache_entry.w, dst->cache_entry.h);
   evas_common_draw_context_clip_clip(dc, dst_region_x, dst_region_y, dst_region_w, dst_region_h);

   /* our clip is 0 size.. abort */
   if ((dc->clip.w <= 0) || (dc->clip.h <= 0))
     {
	dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;
	return EINA_FALSE;
     }

   rects = evas_common_draw_context_apply_cutouts(dc, rects);
   for (i = 0; i < rects->active; ++i)
     {
	r = rects->rects + i;
	evas_common_draw_context_set_clip(dc, r->x, r->y, r->w, r->h);
        ret |= cb(src, dst, dc,
                  src_region_x, src_region_y, src_region_w, src_region_h,
                  dst_region_x, dst_region_y, dst_region_w, dst_region_h);
     }

   /* restore clip info */
   dc->clip.use = c; dc->clip.x = cx; dc->clip.y = cy; dc->clip.w = cw; dc->clip.h = ch;

   return ret;
}
