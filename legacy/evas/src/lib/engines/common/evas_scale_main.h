#ifndef _EVAS_SCALE_MAIN_H
#define _EVAS_SCALE_MAIN_H


EAPI void evas_common_scale_init                            (void);

EAPI void evas_common_scale_rgba_in_to_out_clip_smooth      (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

EAPI void evas_common_scale_rgba_in_to_out_clip_sample      (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

EAPI void evas_common_rgba_image_scalecache_dump(void);

EAPI void evas_common_scale_rgba_in_to_out_clip_sample_do   (const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI void evas_common_scale_rgba_in_to_out_clip_smooth_do   (const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI Eina_Bool evas_common_scale_rgba_in_to_out_clip_prepare     (Cutout_Rects *reuse, const RGBA_Image *src, const RGBA_Image *dst, RGBA_Draw_Context *dc, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);


#endif /* _EVAS_SCALE_MAIN_H */
