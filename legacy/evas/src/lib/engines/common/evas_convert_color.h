/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_CONVERT_COLOR_H
#define _EVAS_CONVERT_COLOR_H


EAPI void evas_common_convert_argb_premul                          (DATA32 *src, unsigned int len);
EAPI void evas_common_convert_argb_unpremul                        (DATA32 *src, unsigned int len);
EAPI void evas_common_convert_color_argb_premul                    (int a, int *r, int *g, int *b);
EAPI void evas_common_convert_color_argb_unpremul                  (int a, int *r, int *g, int *b);

EAPI void evas_common_convert_color_hsv_to_rgb                     (float h, float s, float v, int *r, int *g, int *b);
EAPI void evas_common_convert_color_rgb_to_hsv                     (int r, int g, int b, float *h, float *s, float *v);
EAPI void evas_common_convert_color_hsv_to_rgb_int                 (int h, int s, int v, int *r, int *g, int *b);
EAPI void evas_common_convert_color_rgb_to_hsv_int                 (int r, int g, int b, int *h, int *s, int *v);


#endif /* _EVAS_CONVERT_COLOR_H */
