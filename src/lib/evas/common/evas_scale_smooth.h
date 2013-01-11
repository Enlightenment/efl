#ifndef _EVAS_SCALE_SMOOTH_H
#define _EVAS_SCALE_SMOOTH_H

EAPI Eina_Bool evas_common_scale_rgba_in_to_out_clip_smooth_mmx  (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI Eina_Bool evas_common_scale_rgba_in_to_out_clip_smooth_c    (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

#endif /* _EVAS_SCALE_SMOOTH_H */
