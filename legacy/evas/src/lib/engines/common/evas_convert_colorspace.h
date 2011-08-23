#ifndef _EVAS_CONVERT_COLORSPACE_H
#define _EVAS_CONVERT_COLORSPACE_H


EAPI void *evas_common_convert_argb8888_to                         (void *data, int w, int h, int stride, Eina_Bool has_alpha, Evas_Colorspace cspace);
EAPI void *evas_common_convert_rgb565_a5p_to                       (void *data, int w, int h, int stride, Eina_Bool has_alpha, Evas_Colorspace cspace);
EAPI void *evas_common_convert_yuv_422P_601_to                     (void *data, int w, int h, Evas_Colorspace cspace);
EAPI void *evas_common_convert_yuv_422_601_to                      (void *data, int w, int h, Evas_Colorspace cspace);

#endif /* _EVAS_CONVERT_COLORSPACE_H */
