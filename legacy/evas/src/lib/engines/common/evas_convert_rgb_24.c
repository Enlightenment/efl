/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_convert_rgb_24.h"

#ifdef BUILD_CONVERT_24_RGB_888
void
evas_common_convert_rgba_to_24bpp_rgb_888(DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;

   dst_ptr = (DATA8 *)dst;
   src_ptr = (DATA32 *)src;

   for (y = 0; y < h; y++)
     {
	for (x = 0; x < w; x++)
	  {
	     dst_ptr[0] = R_VAL(src_ptr);
	     dst_ptr[1] = G_VAL(src_ptr);
	     dst_ptr[2] = B_VAL(src_ptr);
	     src_ptr++;
	     dst_ptr+=3;
	  }
	src_ptr += src_jump;
	dst_ptr += (dst_jump * 3);
     }
   return;
   pal = 0;
   dith_x = 0;
   dith_y = 0;
}
#endif
#ifdef BUILD_CONVERT_24_BGR_888
void
evas_common_convert_rgba_to_24bpp_bgr_888(DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x, int dith_y, DATA8 *pal)
{
   DATA32 *src_ptr;
   DATA8 *dst_ptr;
   int x, y;

   dst_ptr = (DATA8 *)dst;
   src_ptr = (DATA32 *)src;

   for (y = 0; y < h; y++)
     {
	for (x = 0; x < w; x++)
	  {
	     dst_ptr[2] = R_VAL(src_ptr);
	     dst_ptr[1] = G_VAL(src_ptr);
	     dst_ptr[0] = B_VAL(src_ptr);
	     src_ptr++;
	     dst_ptr+=3;
	  }
	src_ptr += src_jump;
	dst_ptr += (dst_jump * 3);
     }
   return;
   pal = 0;
   dith_x = 0;
   dith_y = 0;
}
#endif
