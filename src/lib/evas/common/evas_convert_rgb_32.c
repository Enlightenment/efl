#include "evas_common_private.h"
#include "evas_convert_rgb_32.h"
#ifdef BUILD_NEON
#include <arm_neon.h>
#endif

void
evas_common_convert_rgba_to_32bpp_rgb_8888 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_rgb_8888_rot_180 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

#ifdef TILE_ROTATE
#ifdef BUILD_NEON
#define ROT90_QUAD_COPY_LOOP(pix_type) \
   if (evas_common_cpu_has_feature(CPU_FEATURE_NEON)) \
   { \
      if((w%4) == 0) \
      { \
        int klght = 4 * src_stride; \
        for(y = 0; y < h; y++) \
        { \
          const pix_type *s = &(src[(h - y - 1)]); \
          pix_type *d = &(dst[(dst_stride * y)]); \
          pix_type *ptr1 = s; \
          pix_type *ptr2 = ptr1 + src_stride; \
          pix_type *ptr3 = ptr2 + src_stride; \
          pix_type *ptr4 = ptr3 + src_stride; \
          for(x = 0; x < w; x+=4) \
          { \
            pix_type s_array[4] = {*ptr1, *ptr2, *ptr3, *ptr4}; \
            vst1q_s32(d, vld1q_s32(s_array)); \
            d += 4; \
            ptr1 += klght; \
            ptr2 += klght; \
            ptr3 += klght; \
            ptr4 += klght; \
          } \
        } \
      } \
      else \
      { \
        for (y = 0; y < h; y++) \
          { \
             const pix_type *s = &(src[(h - y - 1)]); \
             pix_type *d = &(dst[(dst_stride * y)]); \
             for (x = 0; x < w; x++) \
               { \
                  *d++ = *s; \
                  s += src_stride; \
               } \
          } \
      } \
   } \
   else
#define ROT270_QUAD_COPY_LOOP(pix_type) \
   if (evas_common_cpu_has_feature(CPU_FEATURE_NEON)) \
   { \
      if((w%4) == 0) \
      { \
        int klght = 4 * src_stride; \
        for(y = 0; y < h; y++) \
        { \
          const pix_type *s = &(src[(src_stride * (w - 1)) + y]); \
          pix_type *d = &(dst[(dst_stride * y)]); \
          pix_type *ptr1 = s; \
          pix_type *ptr2 = ptr1 + src_stride; \
          pix_type *ptr3 = ptr2 + src_stride; \
          pix_type *ptr4 = ptr3 + src_stride; \
          for(x = 0; x < w; x+=4) \
          { \
            pix_type s_array[4] = {*ptr1, *ptr2, *ptr3, *ptr4}; \
            vst1q_s32(d, vld1q_s32(s_array)); \
            d += 4; \
            ptr1 += klght; \
            ptr2 += klght; \
            ptr3 += klght; \
            ptr4 += klght; \
          } \
        } \
      } \
      else \
      { \
        for (y = 0; y < h; y++) \
          { \
             const pix_type *s = &(src[(src_stride * (w - 1)) + y]); \
             pix_type *d = &(dst[(dst_stride * y)]); \
             for (x = 0; x < w; x++) \
               { \
                  *d++ = *s; \
                  s += src_stride; \
               } \
          } \
      } \
   } \
   else
#else
#define ROT90_QUAD_COPY_LOOP(pix_type)
#define ROT270_QUAD_COPY_LOOP(pix_type)
#endif
#define FAST_SIMPLE_ROTATE(suffix, pix_type) \
   static void \
   blt_rotated_90_trivial_##suffix(pix_type * restrict dst, \
                                   int             dst_stride, \
                                   const pix_type * restrict src, \
                                   int             src_stride, \
                                   int             w, \
                                   int             h) \
   { \
      int x, y; \
      ROT90_QUAD_COPY_LOOP(pix_type) \
      { \
        for (y = 0; y < h; y++) \
          { \
             const pix_type *s = &(src[(h - y - 1)]); \
             pix_type *d = &(dst[(dst_stride * y)]); \
             for (x = 0; x < w; x++) \
               { \
                  *d++ = *s; \
                  s += src_stride; \
               } \
          } \
      } \
   } \
   static void \
   blt_rotated_270_trivial_##suffix(pix_type * restrict dst, \
                                    int             dst_stride, \
                                    const pix_type * restrict src, \
                                    int             src_stride, \
                                    int             w, \
                                    int             h) \
   { \
      int x, y; \
      ROT270_QUAD_COPY_LOOP(pix_type) \
      { \
        for(y = 0; y < h; y++) \
        { \
           const pix_type *s = &(src[(src_stride * (w - 1)) + y]); \
           pix_type *d = &(dst[(dst_stride * y)]); \
           for (x = 0; x < w; x++) \
           { \
              *d++ = *s; \
              s -= src_stride; \
           } \
        } \
      } \
   } \
   static void \
   blt_rotated_90_##suffix(pix_type * restrict dst, \
                           int             dst_stride, \
                           const pix_type * restrict src, \
                           int             src_stride, \
                           int             w, \
                           int             h) \
   { \
      int x, leading_pixels = 0, trailing_pixels = 0; \
      const int TILE_SIZE = TILE_CACHE_LINE_SIZE / sizeof(pix_type); \
      if ((uintptr_t)dst & (TILE_CACHE_LINE_SIZE - 1)) \
        { \
           leading_pixels = TILE_SIZE - \
             (((uintptr_t)dst & (TILE_CACHE_LINE_SIZE - 1)) / sizeof(pix_type)); \
           if (leading_pixels > w) \
             leading_pixels = w; \
           blt_rotated_90_trivial_##suffix(dst, \
                                           dst_stride, \
                                           src, \
                                           src_stride, \
                                           leading_pixels, \
                                           h); \
           dst += leading_pixels; \
           src += leading_pixels * src_stride; \
           w -= leading_pixels; \
        } \
      if ((uintptr_t)(dst + w) & (TILE_CACHE_LINE_SIZE - 1)) \
        { \
           trailing_pixels = (((uintptr_t)(dst + w) & \
                               (TILE_CACHE_LINE_SIZE - 1)) / sizeof(pix_type)); \
           if (trailing_pixels > w) \
             trailing_pixels = w; \
           w -= trailing_pixels; \
        } \
      for (x = 0; x < w; x += TILE_SIZE) \
        { \
           blt_rotated_90_trivial_##suffix(dst + x, \
                                           dst_stride, \
                                           &(src[(src_stride * x)]), \
                                           src_stride, \
                                           TILE_SIZE, \
                                           h); \
        } \
      if (trailing_pixels) \
        blt_rotated_90_trivial_##suffix(dst + w, \
                                        dst_stride, \
                                        &(src[(w * src_stride)]), \
                                        src_stride, \
                                        trailing_pixels, \
                                        h); \
   } \
   static void \
   blt_rotated_270_##suffix(pix_type * restrict dst, \
                            int             dst_stride, \
                            const pix_type * restrict src, \
                            int             src_stride, \
                            int             w, \
                            int             h) \
   { \
      int x, leading_pixels = 0, trailing_pixels = 0; \
      const int TILE_SIZE = TILE_CACHE_LINE_SIZE / sizeof(pix_type); \
      if ((uintptr_t)dst & (TILE_CACHE_LINE_SIZE - 1)) \
        { \
           leading_pixels = TILE_SIZE - \
             (((uintptr_t)dst & (TILE_CACHE_LINE_SIZE - 1)) / sizeof(pix_type)); \
           if (leading_pixels > w) \
             leading_pixels = w; \
           blt_rotated_270_trivial_##suffix(dst, \
                                            dst_stride, \
                                            &(src[(src_stride * (w - leading_pixels))]), \
                                            src_stride, \
                                            leading_pixels, \
                                            h); \
           dst += leading_pixels; \
           w -= leading_pixels; \
        } \
      if ((uintptr_t)(dst + w) & (TILE_CACHE_LINE_SIZE - 1)) \
        { \
           trailing_pixels = (((uintptr_t)(dst + w) & \
                               (TILE_CACHE_LINE_SIZE - 1)) / sizeof(pix_type)); \
           if (trailing_pixels > w) \
             trailing_pixels = w; \
           w -= trailing_pixels; \
           src += trailing_pixels * src_stride; \
        } \
      for (x = 0; x < w; x += TILE_SIZE) \
        { \
           blt_rotated_270_trivial_##suffix(dst + x, \
                                            dst_stride, \
                                            &(src[(src_stride * (w - x - TILE_SIZE))]), \
                                            src_stride, \
                                            TILE_SIZE, \
                                            h); \
        } \
      if (trailing_pixels) \
        blt_rotated_270_trivial_##suffix(dst + w, \
                                         dst_stride, \
                                         src - (trailing_pixels * src_stride), \
                                         src_stride, \
                                         trailing_pixels, \
                                         h); \
   }

FAST_SIMPLE_ROTATE(8888, DATA32)
#endif

void
evas_common_convert_rgba_to_32bpp_rgb_8888_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
{
#ifdef TILE_ROTATE
   blt_rotated_270_8888((DATA8 *)dst,  dst_jump+w, (const DATA8 *)src, src_jump+h,  w, h) ;
#else
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;
   
   dst_ptr = (DATA32 *)dst;

   CONVERT_LOOP_START_ROT_270();

   *dst_ptr = *src_ptr;

   CONVERT_LOOP_END_ROT_270();
#endif
   return;
}

void
evas_common_convert_rgba_to_32bpp_rgb_8888_rot_90 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
{
# ifndef BUILD_NEON
#  ifdef TILE_ROTATE
   blt_rotated_90_8888((DATA8 *)dst,  dst_jump+w, (const DATA8 *)src, src_jump+h, w, h) ;
#  else
   DATA32 *src_ptr;
   DATA32 *dst_ptr;
   int x, y;
   
   dst_ptr = (DATA32 *)dst;
   CONVERT_LOOP_START_ROT_90();

   *dst_ptr = *src_ptr;

   CONVERT_LOOP_END_ROT_90();
#  endif
   
# else
   
#  ifdef TILE_ROTATE
   blt_rotated_90_8888((DATA8 *)dst,  dst_jump+w, (const DATA8 *)src, src_jump+h, w, h) ;
#  else
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
     }
   else
     {
#   define AP  "convert_rgba32_rot_90_"
        asm volatile (
        ".fpu neon                      \n\t"
        "   mov     %[s1],  %[src]          \n\t"
        "   add     %[s1],  %[s1],  %[h],lsl #2 \n\t"
        "   sub     %[s1],  #8          \n\t"

        "   mov     %[s2],  %[src]          \n\t"
        "   add     %[s2],  %[s2],  %[h], lsl #3    \n\t"
        "   add     %[s2],  %[s2],  %[sjmp], lsr #1 \n\t"
        "   sub     %[s2],  #8          \n\t"

        "   mov     %[d1],  %[dst]          \n\t"

        "   add     %[d2],  %[d1], %[djmp]      \n\t"
        "   add     %[d2],  %[d2],  %[w], lsl #2    \n\t"

        "   mov     %[sadv], %[h], lsl #3       \n\t"
        "   add     %[sadv], %[sadv], %[sjmp], lsl #1\n\t"

        "   mov     %[y],   #0          \n\t"
        "   mov     %[x],   #0          \n\t"
        AP"loop:                        \n\t"
        "   vld1.u32    d0, [%[s1]]         \n\t"
        "   vld1.u32    d1, [%[s2]]         \n\t"
        "   add     %[x],   #2          \n\t"
        "   add     %[s1],  %[sadv]         \n\t"
        "   add     %[s2],  %[sadv]         \n\t"
        "   vtrn.u32    d0, d1          \n\t"
        "   cmp     %[x],   %[w]            \n\t"
        "   vst1.u32    d1, [%[d1]]!        \n\t"
        "   vst1.u32    d0, [%[d2]]!        \n\t"
        "   blt     "AP"loop            \n\t"

        "   mov     %[x],   #0          \n\t"
        "   add     %[d1],  %[djmp]         \n\t"
        "   add     %[d1],  %[d1],  %[w], lsl #2    \n\t"
        "   add     %[d2],  %[djmp]         \n\t"
        "   add     %[d2],  %[d2],  %[w], lsl #2    \n\t"

        "   mov     %[s1],  %[src]          \n\t"
        "   add     %[s1],  %[s1],  %[h], lsl #2    \n\t"
        "   sub     %[s1],  %[s1],  %[y], lsl #2    \n\t"
        "   sub     %[s1],  #16         \n\t"

        "   add     %[s2],  %[s1], %[h], lsl #2 \n\t"
        "   add     %[s2],  %[s2],  %[sjmp], lsl #2 \n\t"

        "   add     %[y],   #2          \n\t"

        "   cmp     %[y],   %[h]            \n\t"
        "   blt     "AP"loop            \n\t"

    : // Out
    :   [s1] "r" (1),
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
#   undef AP
#  endif
# endif
   return;
}

void
evas_common_convert_rgba_to_32bpp_rgbx_8888 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_rgbx_8888_rot_180 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_rgbx_8888_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_rgbx_8888_rot_90 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_bgr_8888 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_bgr_8888_rot_180 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_bgr_8888_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_bgr_8888_rot_90 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_bgrx_8888 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_bgrx_8888_rot_180 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_bgrx_8888_rot_270 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_bgrx_8888_rot_90 (DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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

void
evas_common_convert_rgba_to_32bpp_rgb_666(DATA32 *src, DATA8 *dst, int src_jump, int dst_jump, int w, int h, int dith_x EINA_UNUSED, int dith_y EINA_UNUSED, DATA8 *pal EINA_UNUSED)
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
