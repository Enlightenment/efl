#ifndef _EVAS_IMAGE_H
#define _EVAS_IMAGE_H


EAPI void              evas_common_image_init                      (void);
EAPI void              evas_common_image_shutdown                  (void);

EAPI void              evas_common_image_colorspace_normalize      (RGBA_Image *im);
EAPI void              evas_common_image_colorspace_dirty          (RGBA_Image *im);
EAPI void              evas_common_image_cache_free                (void); /*2*/
EAPI void              evas_common_image_premul                    (Image_Entry *ie); /*2*/
EAPI void              evas_common_image_set_alpha_sparse          (Image_Entry *ie); /*2*/
/* EAPI RGBA_Image   *evas_common_image_alpha_create      (int w, int h); */
/* EAPI RGBA_Image   *evas_common_image_create            (int w, int h); */
EAPI RGBA_Image       *evas_common_image_new                       (void);
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

EAPI void              evas_common_image_render_pre       (RGBA_Draw_Context *dc, void *pim);
EAPI void              evas_common_image_draw2            (RGBA_Image *dst, RGBA_Draw_Context *dc, void *pim, int x, int y, int w, int h);
EAPI int               evas_common_image_is_opaque(RGBA_Draw_Context *dc, void *pim, int x, int y, int w, int h);
EAPI Gfx_Func_Image_Fill evas_common_image_fill_func_get(void *pim, int render_op);

#endif /* _EVAS_IMAGE_H */
