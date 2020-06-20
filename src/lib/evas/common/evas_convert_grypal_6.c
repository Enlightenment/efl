#include "evas_common_private.h"
#include "evas_convert_grypal_6.h"

void evas_common_convert_rgba_to_8bpp_pal_gray64(DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal)
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
