#ifndef _EVAS_SCALE_MAIN_H
#define _EVAS_SCALE_MAIN_H

typedef Eina_Bool (*Evas_Common_Scale_In_To_Out_Clip_Cb)(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

EAPI void evas_common_scale_init                            (void);

EAPI Eina_Bool evas_common_scale_rgba_in_to_out_clip_cb          (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h, Evas_Common_Scale_In_To_Out_Clip_Cb cb);
EAPI Eina_Bool evas_common_scale_rgba_in_to_out_clip_smooth      (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI Eina_Bool evas_common_scale_rgba_in_to_out_clip_sample      (RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

EAPI void evas_common_rgba_image_scalecache_dump(void);

EAPI void evas_common_scale_rgba_in_to_out_clip_sample_do   (const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI void evas_common_scale_rgba_in_to_out_clip_smooth_do   (const Cutout_Rects *reuse, const Eina_Rectangle *clip, RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI void evas_common_scale_rgba_sample_draw                (RGBA_Image *src, RGBA_Image *dst, int dst_clip_x, int dst_clip_y, int dst_clip_w, int dst_clip_h, DATA32 mul_col, int render_op, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI void evas_common_scale_rgba_smooth_draw                (RGBA_Image *src, RGBA_Image *dst, int dst_clip_x, int dst_clip_y, int dst_clip_w, int dst_clip_h, DATA32 mul_col, int render_op, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);
EAPI Eina_Bool evas_common_scale_rgba_in_to_out_clip_prepare     (Cutout_Rects **reuse, const RGBA_Image *src, const RGBA_Image *dst, RGBA_Draw_Context *dc, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

#endif /* _EVAS_SCALE_MAIN_H */
