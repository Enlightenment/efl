#ifndef _EVAS_CONVERT_YUV_H
#define _EVAS_CONVERT_YUV_H

EVAS_API void evas_common_convert_yuv_422p_709_rgba     (DATA8 **src, DATA8 *dst, int w, int h);

EVAS_API void evas_common_convert_yuv_422p_601_rgba     (DATA8 **src, DATA8 *dst, int w, int h);
EVAS_API void evas_common_convert_yuv_422_601_rgba      (DATA8 **src, DATA8 *dst, int w, int h);
EVAS_API void evas_common_convert_yuv_420_601_rgba      (DATA8 **src, DATA8 *dst, int w, int h);
EVAS_API void evas_common_convert_yuv_420T_601_rgba     (DATA8 **src, DATA8 *dst, int w, int h);

#endif /* _EVAS_CONVERT_YUV_H */
