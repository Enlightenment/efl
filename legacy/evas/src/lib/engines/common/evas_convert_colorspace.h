/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_CONVERT_COLORSPACE_H
#define _EVAS_CONVERT_COLORSPACE_H


EAPI void *evas_common_convert_argb8888_to                         (void *data, int w, int h, int stride, Evas_Bool has_alpha, Evas_Colorspace cspace);
EAPI void *evas_common_convert_rgb565_a5p_to                       (void *data, int w, int h, int stride, Evas_Bool has_alpha, Evas_Colorspace cspace);


#endif /* _EVAS_CONVERT_COLORSPACE_H */
