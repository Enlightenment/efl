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
#ifndef BUILD_NEON
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;

   dst_ptr = (DATA32 *)dst;
   CONVERT_LOOP_START_ROT_90();

   *dst_ptr = *src_ptr;

   CONVERT_LOOP_END_ROT_90();
#else
   if ((w & 1) || (h & 1))
     {
	/* Rarely (if ever) if ever: so slow path is fine */
	 DATA32 *src_ptr;
	 DATA32 *dst_ptr;
	 int x, y;

	 dst_ptr = (DATA32 *)dst;
	 CONVERT_LOOP_START_ROT_90();

	 *dst_ptr = *src_ptr;

	 CONVERT_LOOP_END_ROT_90();
   } else {
#define AP	"convert_rgba32_rot_90_"
	asm volatile (
	".fpu neon						\n\t"
	"	mov		%[s1],	%[src]			\n\t"
	"	add		%[s1],	%[s1],	%[h],lsl #2	\n\t"
	"	sub		%[s1],	#8			\n\t"

	"	mov		%[s2],	%[src]			\n\t"
	"	add		%[s2],	%[s2],	%[h], lsl #3	\n\t"
	"	add		%[s2],	%[s2],	%[sjmp], lsr #1	\n\t"
	"	sub		%[s2],  #8			\n\t"

	"	mov		%[d1],	%[dst]			\n\t"

	"	add		%[d2],	%[d1], %[djmp]		\n\t"
	"	add		%[d2],	%[d2],	%[w], lsl #2	\n\t"

	"	mov		%[sadv], %[h], lsl #3		\n\t"
	"	add		%[sadv], %[sadv], %[sjmp], lsl #1\n\t"

	"	mov		%[y],	#0			\n\t"
	"	mov		%[x],	#0			\n\t"
	AP"loop:						\n\t"
	"	vld1.u32	d0,	[%[s1]]			\n\t"
	"	vld1.u32	d1,	[%[s2]]			\n\t"
	"	add		%[x],	#2			\n\t"
	"	add		%[s1],  %[sadv]			\n\t"
	"	add		%[s2],  %[sadv]			\n\t"
	"	vtrn.u32	d0,	d1			\n\t"
	"	cmp		%[x],	%[w]			\n\t"
	"	vst1.u32	d1,	[%[d1]]!		\n\t"
	"	vst1.u32	d0,	[%[d2]]!		\n\t"
	"	blt		"AP"loop			\n\t"

	"	mov		%[x],	#0			\n\t"
	"	add		%[d1],  %[djmp]			\n\t"
	"	add		%[d1],	%[d1],	%[w], lsl #2	\n\t"
	"	add		%[d2],  %[djmp]			\n\t"
	"	add		%[d2],	%[d2],	%[w], lsl #2	\n\t"

	"	mov		%[s1],	%[src]			\n\t"
	"	add		%[s1],	%[s1],	%[h], lsl #2	\n\t"
	"	sub		%[s1],	%[s1],	%[y], lsl #2	\n\t"
	"	sub		%[s1],  #16			\n\t"

	"	add		%[s2],	%[s1], %[h], lsl #2	\n\t"
	"	add		%[s2],	%[s2],	%[sjmp], lsl #2	\n\t"

	"	add		%[y],	#2			\n\t"

	"	cmp		%[y],	%[h]			\n\t"
	"	blt		"AP"loop			\n\t"

	: // Out
	:	[s1] "r" (1),
		[s2] "r" (11),
		[d1] "r" (2),
		[d2] "r" (12),
		[src] "r" (src),
		[dst] "r" (dst),
		[x] "r" (3),
		[y] "r" (4),
		[w] "r" (w),
		[h] "r" (h),
		[sadv] "r" (5),
		[sjmp] "r" (src_jump * 4),
		[djmp] "r" (dst_jump * 4 * 2)
	: "d0", "d1", "memory", "cc"// Clober


	);
   }
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

