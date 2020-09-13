#ifndef _EVAS_SCALE_SPAN_H
#define _EVAS_SCALE_SPAN_H


EVAS_API void evas_common_scale_rgba_span                       (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);
EVAS_API void evas_common_scale_rgba_a8_span                    (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);
EVAS_API void evas_common_scale_a8_span                         (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);
EVAS_API void evas_common_scale_clip_a8_span                    (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);

EVAS_API void evas_common_scale_hsva_span                       (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);
EVAS_API void evas_common_scale_hsva_a8_span                    (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);


#endif /* _EVAS_SCALE_SPAN_H */
