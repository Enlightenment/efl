/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_convert_gry_8.h"

#ifdef USE_DITHER_44
extern const DATA8 _evas_dither_44[4][4];
#endif
#ifdef USE_DITHER_128128
extern const DATA8 _evas_dither_128128[128][128];
#endif

#ifdef BUILD_CONVERT_8_GRY_1
void evas_common_convert_rgba_to_8bpp_gry_256_dith     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA32 gry8;

   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   r = (R_VAL(src_ptr));
   g = (G_VAL(src_ptr));
   b = (B_VAL(src_ptr));

   // Y = 0.299 * R + 0.587 * G + 0.114 * B;
   gry8 = ((r * 19595) + (g * 38469) + (b * 7471)) >> 16;

   *dst_ptr = gry8;

   CONVERT_LOOP_END_ROT_0();
}
#endif

#ifdef BUILD_CONVERT_8_GRY_4
void evas_common_convert_rgba_to_8bpp_gry_64_dith      (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal){}
#endif

#ifdef BUILD_CONVERT_8_GRY_16
void evas_common_convert_rgba_to_8bpp_gry_16_dith      (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA32 gry8;
   DATA8 dith;

   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   dith = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(4);

   r = (R_VAL(src_ptr));
   g = (G_VAL(src_ptr));
   b = (B_VAL(src_ptr));

   // Y = 0.299 * R + 0.587 * G + 0.114 * B;
   gry8 = ((r * 19595) + (g * 38469) + (b * 7471)) >> 16;

   if (((gry8 - gry8 * 255 / 4) >= dith) && (gry8 < 0x07)) gry8++;

   *dst_ptr = gry8;

   CONVERT_LOOP_END_ROT_0();
}
#endif

#ifdef BUILD_CONVERT_8_GRY_64
void evas_common_convert_rgba_to_8bpp_gry_4_dith       (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal){}
#endif

#ifdef BUILD_CONVERT_8_GRY_256
void evas_common_convert_rgba_to_8bpp_gry_1_dith       (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal){}
#endif
