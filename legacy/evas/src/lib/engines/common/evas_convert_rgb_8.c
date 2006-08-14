#include "evas_common.h"

#ifdef USE_DITHER_44
extern const DATA8 _evas_dither_44[4][4];
#endif
#ifdef USE_DITHER_128128
extern const DATA8 _evas_dither_128128[128][128];
#endif

#ifdef BUILD_CONVERT_8_RGB_332
void evas_common_convert_rgba_to_8bpp_rgb_332_dith     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA8 dith, dith2;

   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   dith = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(3);
   dith2 = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(2);
/*   r = (R_VAL(src_ptr)) >> (8 - 3);*/
/*   g = (G_VAL(src_ptr)) >> (8 - 3);*/
/*   b = (B_VAL(src_ptr)) >> (8 - 2);*/
/*   if (((R_VAL(src_ptr) - (r << (8 - 3))) >= dith ) && (r < 0x07)) r++;*/
/*   if (((G_VAL(src_ptr) - (g << (8 - 3))) >= dith ) && (g < 0x07)) g++;*/
/*   if (((B_VAL(src_ptr) - (b << (8 - 2))) >= dith2) && (b < 0x03)) b++;*/
   r = (R_VAL(src_ptr)) * 7 / 255;
   if (((R_VAL(src_ptr) - (r * 255 / 7)) >= dith ) && (r < 0x07)) r++;
   g = (G_VAL(src_ptr)) * 7 / 255;
   if (((G_VAL(src_ptr) - (g * 255 / 7)) >= dith ) && (g < 0x07)) g++;
   b = (B_VAL(src_ptr)) * 3 / 255;
   if (((B_VAL(src_ptr) - (b * 255 / 3)) >= dith2) && (b < 0x03)) b++;

   *dst_ptr = pal[(r << 5) | (g << 2) | (b)];

   CONVERT_LOOP_END_ROT_0();
}
#endif
#ifdef BUILD_CONVERT_8_RGB_666
static DATA8 p_to_6[256];
static DATA8 p_to_6_err[256];

void evas_common_convert_rgba_to_8bpp_rgb_666_dith     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA8 dith;
   static int tables_calcualted = 0;

   if (!tables_calcualted)
     {
	int i;

	tables_calcualted = 1;
	for (i = 0; i < 256; i++)
	  p_to_6[i] = (i * 5) / 255;
	for (i = 0; i < 256; i++)
	  p_to_6_err[i] = ((i * 5) - (p_to_6[i] * 255)) * DM_DIV / 255;
     }
   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   r = p_to_6[(R_VAL(src_ptr))];
   g = p_to_6[(G_VAL(src_ptr))];
   b = p_to_6[(B_VAL(src_ptr))];
   dith = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK];
   if ((p_to_6_err[(R_VAL(src_ptr))] >= dith ) && (r < 5)) r++;
   if ((p_to_6_err[(G_VAL(src_ptr))] >= dith ) && (g < 5)) g++;
   if ((p_to_6_err[(B_VAL(src_ptr))] >= dith ) && (b < 5)) b++;

   *dst_ptr = pal[(r * 36) + (g * 6) + (b)];

   CONVERT_LOOP_END_ROT_0();
}
#endif
#ifdef BUILD_CONVERT_8_RGB_232
void evas_common_convert_rgba_to_8bpp_rgb_232_dith     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA8 dith, dith2;

   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   dith = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(3);
   dith2 = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(2);
/*   r = (R_VAL(src_ptr)) >> (8 - 2);*/
/*   g = (G_VAL(src_ptr)) >> (8 - 3);*/
/*   b = (B_VAL(src_ptr)) >> (8 - 2);*/
/*   if (((R_VAL(src_ptr) - (r << (8 - 2))) >= dith2) && (r < 0x03)) r++;*/
/*   if (((G_VAL(src_ptr) - (g << (8 - 3))) >= dith ) && (g < 0x07)) g++;*/
/*   if (((B_VAL(src_ptr) - (b << (8 - 2))) >= dith2) && (b < 0x03)) b++;*/
   r = (R_VAL(src_ptr)) * 3 / 255;
   if (((R_VAL(src_ptr) - (r * 255 / 3)) >= dith2) && (r < 0x03)) r++;
   g = (G_VAL(src_ptr)) * 7 / 255;
   if (((G_VAL(src_ptr) - (g * 255 / 7)) >= dith ) && (g < 0x07)) g++;
   b = (B_VAL(src_ptr)) * 3 / 255;
   if (((B_VAL(src_ptr) - (b * 255 / 3)) >= dith2) && (b < 0x03)) b++;

   *dst_ptr = pal[(r << 5) | (g << 2) | (b)];

   CONVERT_LOOP_END_ROT_0();
}
#endif
#ifdef BUILD_CONVERT_8_RGB_222
void evas_common_convert_rgba_to_8bpp_rgb_222_dith     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA8 dith;

   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   dith = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(2);
/*   r = (R_VAL(src_ptr)) >> (8 - 2);*/
/*   g = (G_VAL(src_ptr)) >> (8 - 2);*/
/*   b = (B_VAL(src_ptr)) >> (8 - 2);*/
/*   if (((R_VAL(src_ptr) - (r << (8 - 2))) >= dith ) && (r < 0x03)) r++;*/
/*   if (((G_VAL(src_ptr) - (g << (8 - 2))) >= dith ) && (g < 0x03)) g++;*/
/*   if (((B_VAL(src_ptr) - (b << (8 - 2))) >= dith ) && (b < 0x03)) b++;*/
   r = (R_VAL(src_ptr)) * 3 / 255;
   if (((R_VAL(src_ptr) - (r * 255 / 3)) >= dith ) && (r < 0x03)) r++;
   g = (G_VAL(src_ptr)) * 3 / 255;
   if (((G_VAL(src_ptr) - (g * 255 / 3)) >= dith ) && (g < 0x03)) g++;
   b = (B_VAL(src_ptr)) * 3 / 255;
   if (((B_VAL(src_ptr) - (b * 255 / 3)) >= dith ) && (b < 0x03)) b++;

   *dst_ptr = pal[(r << 4) | (g << 2) | (b)];

   CONVERT_LOOP_END_ROT_0();
}
#endif
#ifdef BUILD_CONVERT_8_RGB_221
void evas_common_convert_rgba_to_8bpp_rgb_221_dith     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA8 dith, dith2;

   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   dith = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(2);
   dith2 = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(1);
/*   r = (R_VAL(src_ptr)) >> (8 - 2);*/
/*   g = (G_VAL(src_ptr)) >> (8 - 2);*/
/*   b = (B_VAL(src_ptr)) >> (8 - 1);*/
/*   if (((R_VAL(src_ptr) - (r << (8 - 2))) >= dith ) && (r < 0x03)) r++;*/
/*   if (((G_VAL(src_ptr) - (g << (8 - 2))) >= dith ) && (g < 0x03)) g++;*/
/*   if (((B_VAL(src_ptr) - (b << (8 - 1))) >= dith2) && (b < 0x01)) b++;*/
   r = (R_VAL(src_ptr)) * 3 / 255;
   if (((R_VAL(src_ptr) - (r * 255 / 3)) >= dith ) && (r < 0x03)) r++;
   g = (G_VAL(src_ptr)) * 3 / 255;
   if (((G_VAL(src_ptr) - (g * 255 / 3)) >= dith ) && (g < 0x03)) g++;
   b = (B_VAL(src_ptr)) * 1 / 255;
   if (((B_VAL(src_ptr) - (b * 255 / 1)) >= dith2) && (b < 0x01)) b++;

   *dst_ptr = pal[(r << 3) | (g << 1) | (b)];

   CONVERT_LOOP_END_ROT_0();
}
#endif
#ifdef BUILD_CONVERT_8_RGB_121
void evas_common_convert_rgba_to_8bpp_rgb_121_dith     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA8 dith, dith2;

   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   dith = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(2);
   dith2 = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(1);
/*   r = (R_VAL(src_ptr)) >> (8 - 1);*/
/*   g = (G_VAL(src_ptr)) >> (8 - 2);*/
/*   b = (B_VAL(src_ptr)) >> (8 - 1);*/
/*   if (((R_VAL(src_ptr) - (r << (8 - 1))) >= dith2) && (r < 0x01)) r++;*/
/*   if (((G_VAL(src_ptr) - (g << (8 - 2))) >= dith ) && (g < 0x03)) g++;*/
/*   if (((B_VAL(src_ptr) - (b << (8 - 1))) >= dith2) && (b < 0x01)) b++;*/

   r = (R_VAL(src_ptr)) * 1 / 255;
   if (((R_VAL(src_ptr) - (r * 255 / 1)) >= dith2) && (r < 0x01)) r++;
   g = (G_VAL(src_ptr)) * 3 / 255;
   if (((G_VAL(src_ptr) - (g * 255 / 3)) >= dith ) && (g < 0x03)) g++;
   b = (B_VAL(src_ptr)) * 1 / 255;
   if (((B_VAL(src_ptr) - (b * 255 / 1)) >= dith2) && (b < 0x01)) b++;

   *dst_ptr = pal[(r << 3) | (g << 1) | (b)];

   CONVERT_LOOP_END_ROT_0();
}
#endif
#ifdef BUILD_CONVERT_8_RGB_111
void evas_common_convert_rgba_to_8bpp_rgb_111_dith     (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;
   DATA8 r, g, b;
   DATA8 dith;

   dst_ptr = (DATA8 *)dst;

   CONVERT_LOOP_START_ROT_0();

   dith = DM_TABLE[(x + dith_x) & DM_MSK][(y + dith_y) & DM_MSK] >> DM_SHF(1);
/*   r = (R_VAL(src_ptr)) >> (8 - 1);*/
/*   g = (G_VAL(src_ptr)) >> (8 - 1);*/
/*   b = (B_VAL(src_ptr)) >> (8 - 1);*/
/*   if (((R_VAL(src_ptr) - (r << (8 - 1))) >= dith ) && (r < 0x01)) r++;*/
/*   if (((G_VAL(src_ptr) - (g << (8 - 1))) >= dith ) && (g < 0x01)) g++;*/
/*   if (((B_VAL(src_ptr) - (b << (8 - 1))) >= dith ) && (b < 0x01)) b++;*/

   r = (R_VAL(src_ptr)) * 1 / 255;
   if (((R_VAL(src_ptr) - (r * 255 / 1)) >= dith ) && (r < 0x01)) r++;
   g = (G_VAL(src_ptr)) * 1 / 255;
   if (((G_VAL(src_ptr) - (g * 255 / 1)) >= dith ) && (g < 0x01)) g++;
   b = (B_VAL(src_ptr)) * 1 / 255;
   if (((B_VAL(src_ptr) - (b * 255 / 1)) >= dith ) && (b < 0x01)) b++;

   *dst_ptr = pal[(r << 2) | (g << 1) | (b)];

   CONVERT_LOOP_END_ROT_0();
}
#endif
