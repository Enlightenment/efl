#ifndef _EVAS_CONVERT_COLOR_H
#define _EVAS_CONVERT_COLOR_H


EVAS_API DATA32 evas_common_convert_ag_premul                          (DATA16 *data, unsigned int len);
EVAS_API void evas_common_convert_ag_unpremul                          (DATA16 *data, unsigned int len);
EVAS_API DATA32 evas_common_convert_argb_premul                        (DATA32 *src, unsigned int len);
EVAS_API void evas_common_convert_argb_unpremul                        (DATA32 *src, unsigned int len);
EVAS_API void evas_common_convert_color_argb_premul                    (int a, int *r, int *g, int *b);
EVAS_API void evas_common_convert_color_argb_unpremul                  (int a, int *r, int *g, int *b);

EVAS_API void evas_common_convert_color_hsv_to_rgb                     (float h, float s, float v, int *r, int *g, int *b);
EVAS_API void evas_common_convert_color_rgb_to_hsv                     (int r, int g, int b, float *h, float *s, float *v);
EVAS_API void evas_common_convert_color_hsv_to_rgb_int                 (int h, int s, int v, int *r, int *g, int *b);
EVAS_API void evas_common_convert_color_rgb_to_hsv_int                 (int r, int g, int b, int *h, int *s, int *v);


#endif /* _EVAS_CONVERT_COLOR_H */
