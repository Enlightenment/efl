#include "evas_common.h"

#ifdef BUILD_CONVERT_24_RGB_888
void evas_common_convert_rgba_to_24bpp_rgb_888         (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal){}
#endif
#ifdef BUILD_CONVERT_24_BGR_888
void evas_common_convert_rgba_to_24bpp_bgr_888         (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal){}
#endif
