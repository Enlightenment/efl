/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_convert_grypal_6.h"

#ifdef BUILD_CONVERT_8_GRAYSCALE_64
void evas_common_convert_rgba_to_8bpp_pal_gray64       (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
    DATA32 *src_ptr;
    DATA8 *dst_ptr;
    int x, y;
    DATA8 Y;

    dst_ptr = dst;
    CONVERT_LOOP_START_ROT_0();

    /* RGB -> YUV conversion */
    Y = ((R_VAL(src_ptr) * 76) + 
         (G_VAL(src_ptr) * 151) + 
         (B_VAL(src_ptr) * 29)) >> 10;
    *dst_ptr = pal[Y];

    CONVERT_LOOP_END_ROT_0();
}
#endif
