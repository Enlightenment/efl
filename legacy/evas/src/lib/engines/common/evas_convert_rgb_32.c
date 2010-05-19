/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_convert_rgb_32.h"

#ifdef BUILD_CONVERT_32_RGB_8888
#ifdef BUILD_CONVERT_32_RGB_ROT0
void
evas_common_convert_rgba_to_32bpp_rgb_8888 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int y;
   Gfx_Func_Copy func;

   dst_ptr = (DATA32 *)dst;
   src_ptr = src;

   func = evas_common_draw_func_copy_get(w, 0);

   for (y = 0; y < h; y++)
     {
	func(src_ptr, dst_ptr, w);
	src_ptr += w + src_jump;
	dst_ptr += w + dst_jump;
     }
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_RGB_8888
#ifdef BUILD_CONVERT_32_RGB_ROT180
void
evas_common_convert_rgba_to_32bpp_rgb_8888_rot_180 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_180();

   *dst_ptr = *src_ptr;

   CONVERT_LOOP_END_ROT_180();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_RGB_8888
#ifdef BUILD_CONVERT_32_RGB_ROT270
void
evas_common_convert_rgba_to_32bpp_rgb_8888_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_270();

   *dst_ptr = *src_ptr;

   CONVERT_LOOP_END_ROT_270();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_RGB_8888
#ifdef BUILD_CONVERT_32_RGB_ROT90
void
evas_common_convert_rgba_to_32bpp_rgb_8888_rot_90 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
#ifndef BUILD_NEONx
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;
   CONVERT_LOOP_START_ROT_90();

   *dst_ptr = *src_ptr;

   CONVERT_LOOP_END_ROT_90();
#else
#define AP	"convert_rgba32_rot_90_"
   asm volatile (
	AP"outer:						\n\t"
		// Set up src
	"	sub		%[s1],		%[src], %[y],lsl #2	\n\t"
	"	sub		%[s2],		%[src2], %[y],lsl #2	\n\t"
	"	add		%[y],	#2			\n\t"
	"	add		%[x],	%[d1], %[w], lsl #2		\n\t"
	AP"inner:						\n\t"
	"	vldm		%[s1], {d0}			\n\t"
	"	vldm		%[s2], {d1}			\n\t"
	"	vtrn.u32	d1,d0				\n\t"
	"	vstm		%[d1]!, {d0}			\n\t"
	"	vstm		%[d2]!, {d1}			\n\t"
	"	add		%[s1],		%[sadv]		\n\t"
	"	add		%[s2],		%[sadv]		\n\t"
	"	cmp		%[x],		%[d1]		\n\t"
	"	bhi		"AP"inner			\n\t"

	"	add		%[d1], %[djump]			\n\t"
	"	add		%[d2], %[djump]			\n\t"
	"	cmp		%[y],	%[h]			\n\t"
	"	blt		"AP"outer			\n\t"


	: // Out
	:	[s1] "r" (src),
		[s2] "r" (src + (h + src_jump) * 4),
		[d1] "r" (dst),
		[d2] "r" ((DATA32*)dst + w + dst_jump),
		[sadv] "r" ((h + 2 * src_jump) * 8),
		[src] "r" ((DATA32*)src + (h - 1)- 1),
		[src2] "r" ((DATA32*)src + (h - 1)- 2),
		[djump] "r" ((w + 2 * dst_jump) * 4),
		[x] "r" (7),
		[y] "r" (0),
		[w] "r" (w),
		[h] "r" (h)
	: "q0", "q1", "memory", "cc"// Clober
   );
#undef AP
#endif
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_RGBX_8888
#ifdef BUILD_CONVERT_32_RGB_ROT0
void
evas_common_convert_rgba_to_32bpp_rgbx_8888 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_0();

//   *dst_ptr = (R_VAL(src_ptr) << 24) | (G_VAL(src_ptr) << 16) | (B_VAL(src_ptr) << 8);
   *dst_ptr = (*src_ptr << 8);

   CONVERT_LOOP_END_ROT_0();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_RGBX_8888
#ifdef BUILD_CONVERT_32_RGB_ROT180
void
evas_common_convert_rgba_to_32bpp_rgbx_8888_rot_180 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_180();

//   *dst_ptr = (R_VAL(src_ptr) << 24) | (G_VAL(src_ptr) << 16) | (B_VAL(src_ptr) << 8);
   *dst_ptr = (*src_ptr << 8);

   CONVERT_LOOP_END_ROT_180();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_RGBX_8888
#ifdef BUILD_CONVERT_32_RGB_ROT270
void
evas_common_convert_rgba_to_32bpp_rgbx_8888_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_270();

//   *dst_ptr = (R_VAL(src_ptr) << 24) | (G_VAL(src_ptr) << 16) | (B_VAL(src_ptr) << 8);
   *dst_ptr = (*src_ptr << 8);

   CONVERT_LOOP_END_ROT_270();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_RGBX_8888
#ifdef BUILD_CONVERT_32_RGB_ROT90
void
evas_common_convert_rgba_to_32bpp_rgbx_8888_rot_90 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_90();

//   *dst_ptr = (R_VAL(src_ptr) << 24) | (G_VAL(src_ptr) << 16) | (B_VAL(src_ptr) << 8);
   *dst_ptr = (*src_ptr << 8);

   CONVERT_LOOP_END_ROT_90();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_BGR_8888
#ifdef BUILD_CONVERT_32_RGB_ROT0
void
evas_common_convert_rgba_to_32bpp_bgr_8888 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_0();

   *dst_ptr = (B_VAL(src_ptr) << 16) | (G_VAL(src_ptr) << 8) | (R_VAL(src_ptr));

   CONVERT_LOOP_END_ROT_0();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_BGR_8888
#ifdef BUILD_CONVERT_32_RGB_ROT180
void
evas_common_convert_rgba_to_32bpp_bgr_8888_rot_180 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_180();

   *dst_ptr = (B_VAL(src_ptr) << 16) | (G_VAL(src_ptr) << 8) | (R_VAL(src_ptr));

   CONVERT_LOOP_END_ROT_180();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_BGR_8888
#ifdef BUILD_CONVERT_32_RGB_ROT270
void
evas_common_convert_rgba_to_32bpp_bgr_8888_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_270();

   *dst_ptr = (B_VAL(src_ptr) << 16) | (G_VAL(src_ptr) << 8) | (R_VAL(src_ptr));

   CONVERT_LOOP_END_ROT_270();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_BGR_8888
#ifdef BUILD_CONVERT_32_RGB_ROT90
void
evas_common_convert_rgba_to_32bpp_bgr_8888_rot_90 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_90();

   *dst_ptr = (B_VAL(src_ptr) << 16) | (G_VAL(src_ptr) << 8) | (R_VAL(src_ptr));

   CONVERT_LOOP_END_ROT_90();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_BGRX_8888
#ifdef BUILD_CONVERT_32_RGB_ROT0
void
evas_common_convert_rgba_to_32bpp_bgrx_8888 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_0();

   *dst_ptr = (B_VAL(src_ptr) << 24) | (G_VAL(src_ptr) << 16) | (R_VAL(src_ptr) << 8);

   CONVERT_LOOP_END_ROT_0();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_BGRX_8888
#ifdef BUILD_CONVERT_32_RGB_ROT180
void
evas_common_convert_rgba_to_32bpp_bgrx_8888_rot_180 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_180();

   *dst_ptr = (B_VAL(src_ptr) << 24) | (G_VAL(src_ptr) << 16) | (R_VAL(src_ptr) << 8);

   CONVERT_LOOP_END_ROT_180();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_BGRX_8888
#ifdef BUILD_CONVERT_32_RGB_ROT270
void
evas_common_convert_rgba_to_32bpp_bgrx_8888_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_270();

   *dst_ptr = (B_VAL(src_ptr) << 24) | (G_VAL(src_ptr) << 16) | (R_VAL(src_ptr) << 8);

   CONVERT_LOOP_END_ROT_270();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_BGRX_8888
#ifdef BUILD_CONVERT_32_RGB_ROT90
void
evas_common_convert_rgba_to_32bpp_bgrx_8888_rot_90 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_90();

   *dst_ptr = (B_VAL(src_ptr) << 24) | (G_VAL(src_ptr) << 16) | (R_VAL(src_ptr) << 8);

   CONVERT_LOOP_END_ROT_90();
   return;
}
#endif
#endif

#ifdef BUILD_CONVERT_32_RGB_666
#ifdef BUILD_CONVERT_32_RGB_ROT0
void
evas_common_convert_rgba_to_32bpp_rgb_666(DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x __UNUSED__, int dith_y __UNUSED__, DATA8 *pal __UNUSED__)
{
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_0();

   *dst_ptr = 
     (((R_VAL(src_ptr) << 12) | (B_VAL(src_ptr) >> 2)) & 0x03f03f) |
     ((G_VAL(src_ptr) << 4) & 0x000fc0);

   CONVERT_LOOP_END_ROT_0();
   return;
}
#endif
#endif

