#include "evas_common.h"
#include "evas_private.h"

EAPI void
evas_common_scale_init(void)
{
}

EAPI Eina_Bool
evas_common_scale_rgba_in_to_out_clip_prepare(Cutout_Rects *reuse, const RGBA_Image *src __UNUSED__,
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
   reuse = evas_common_draw_context_apply_cutouts(dc, reuse);

   return EINA_TRUE;
}

