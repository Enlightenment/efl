#ifndef _EVAS_IMAGE_H
#define _EVAS_IMAGE_H


EAPI void              evas_common_image_init                      (void);
EAPI void              evas_common_image_shutdown                  (void);

EAPI void              evas_common_rgba_image_free                 (Image_Entry *ie);
EAPI void              evas_common_image_colorspace_normalize      (RGBA_Image *im);
EAPI void              evas_common_image_colorspace_dirty          (RGBA_Image *im);
EAPI void              evas_common_image_cache_free                (void); /*2*/
EAPI void              evas_common_image_premul                    (Image_Entry *ie); /*2*/
EAPI void              evas_common_image_set_alpha_sparse          (Image_Entry *ie); /*2*/
/* EAPI RGBA_Image   *evas_common_image_alpha_create      (int w, int h); */
/* EAPI RGBA_Image   *evas_common_image_create            (int w, int h); */
EAPI RGBA_Image       *evas_common_image_new                       (int w, int h, int alpha);
EAPI Evas_Cache_Image *evas_common_image_cache_get                 (void);

EAPI void              evas_common_image_set_cache                 (int size);
EAPI int               evas_common_image_get_cache                 (void);

EAPI RGBA_Image       *evas_common_image_line_buffer_obtain        (int len);
EAPI void              evas_common_image_line_buffer_release       (RGBA_Image *im);

EAPI RGBA_Image       *evas_common_image_alpha_line_buffer_obtain  (int len);
EAPI void              evas_common_image_alpha_line_buffer_release (RGBA_Image *im);
EAPI void              evas_common_image_alpha_line_buffer_free    (RGBA_Image *im);

EAPI RGBA_Image       *evas_common_load_image_from_file            (const char *file, const char *key, RGBA_Image_Loadopts *lo);
EAPI int               evas_common_save_image_to_file              (RGBA_Image *im, const char *file, const char *key, int quality, int compress);

EAPI void
  evas_common_rgba_image_scalecache_prepare(Image_Entry *ie, RGBA_Image *dst,
                                            RGBA_Draw_Context *dc, int smooth,
                                            int src_region_x, int src_region_y,
                                            int src_region_w, int src_region_h,
                                            int dst_region_x, int dst_region_y,
                                            int dst_region_w, int dst_region_h);
EAPI void
  evas_common_rgba_image_scalecache_do(Image_Entry *ie, RGBA_Image *dst,
                                       RGBA_Draw_Context *dc, int smooth,
                                       int src_region_x, int src_region_y,
                                       int src_region_w, int src_region_h,
                                       int dst_region_x, int dst_region_y,
                                       int dst_region_w, int dst_region_h);
#endif /* _EVAS_IMAGE_H */
