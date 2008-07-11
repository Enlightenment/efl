/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_SCALE_SPAN_H
#define _EVAS_SCALE_SPAN_H


EAPI void evas_common_scale_rgba_span                       (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);
EAPI void evas_common_scale_rgba_a8_span                    (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);
EAPI void evas_common_scale_a8_span                         (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);
EAPI void evas_common_scale_clip_a8_span                    (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);

EAPI void evas_common_scale_hsva_span                       (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);
EAPI void evas_common_scale_hsva_a8_span                    (DATA32 *src, DATA8 *mask, int src_len, DATA32 mul_col, DATA32 *dst, int dst_len, int dir);


#endif /* _EVAS_SCALE_SPAN_H */
