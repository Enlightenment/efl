/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_CONVERT_RGB_8_H
#define _EVAS_CONVERT_RGB_8_H


void evas_common_convert_rgba_to_8bpp_rgb_332_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_666_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_232_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_222_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_221_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_121_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);
void evas_common_convert_rgba_to_8bpp_rgb_111_dith             (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal);


#endif /* _EVAS_CONVERT_RGB_8_H */
