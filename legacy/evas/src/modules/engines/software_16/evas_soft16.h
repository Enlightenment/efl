#ifndef EVAS_SOFT16_H
#define EVAS_SOFT16_H

#include "Evas.h"
#include "evas_common.h"
#include "evas_private.h"

#define RGB_565_UNPACKED_MASK 0x07e0f81f
#define RGB_565_UNPACK(rgb)                                             \
   (((rgb) | ((rgb) << 16)) & RGB_565_UNPACKED_MASK)
#define RGB_565_PACK(rgb)                                               \
  ((((rgb) & RGB_565_UNPACKED_MASK) |                                   \
   ((rgb) & RGB_565_UNPACKED_MASK) >> 16) & 0xffff)
#define RGB_565_UNPACKED_BLEND(a, b, alpha)                             \
   ((b) + ((((a) - (b)) * (alpha)) >> 5))

#define RGB_565_FROM_COMPONENTS(r, g, b)                                \
  (((((r) >> 3) & 0x1f) << 11) |                                        \
   ((((g) >> 2) & 0x3f) << 5) |                                         \
   (((b) >> 3) & 0x1f))

#define UNROLL2(op...) op op
#define UNROLL4(op...) UNROLL2(op) UNROLL2(op)
#define UNROLL8(op...) UNROLL4(op) UNROLL4(op)
#define UNROLL16(op...) UNROLL8(op) UNROLL8(op)

#if defined(__ARM_ARCH_3M__) || defined(__ARM_ARCH_4__) || \
    defined(__ARM_ARCH_4T__)
# define __ARM_ARCH__ 4
#endif

#if defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5T__) || \
    defined(__ARM_ARCH_5E__) || defined(__ARM_ARCH_5TE__) || \
    defined(__ARM_ARCH_5TEJ__)
# define __ARM_ARCH__ 5
#endif

#if defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || \
    defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || \
    defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__)
# define __ARM_ARCH__ 6
#endif

#if defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || \
    defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__)
# define __ARM_ARCH__ 7
#endif


#if defined(__ARM_ARCH__) && (__ARM_ARCH__ >= 5)
/* tested on ARMv6 (arm1136j-s), Nokia N800 CPU */
#define pld(addr, off)                                                  \
   __asm__("pld [%[address], %[offset]]"::                              \
           [address] "r" (addr), [offset] "i" (off))
#else
#define pld(addr, off)
#endif /* __ARMEL__ */

typedef struct _Soft16_Image Soft16_Image;

struct _Soft16_Image
{
   const char    *file;       // file source - optional
   const char    *key;        // key within file source - optional
   time_t         timestamp;  // file modified timestamp
   time_t         laststat;   // last time this file was statted
   int            w, h;       // width and height in pixels
   int            stride;     // pixel stride - likely a multiple of 2
   DATA16        *pixels;     // 16bpp pixels rgb565
   DATA8         *alpha;      // 8bit alpha mask - optional. points into pixels
   int            references; // refcount
   RGBA_Image    *source_im;  // original source rgba image - if still reffed

   Evas_Image_Load_Opts lo;   // load options

   unsigned char  have_alpha  : 1; // 1 if we have halpha
   unsigned char  free_pixels : 1; // 1 if pixels should be freed
   unsigned char  free_alpha  : 1; // 1 if alpha mask should be freed
};

/**
 * Image (evas_soft16_main.c)
 */
Soft16_Image *soft16_image_new(int w, int h, int stride, int have_alpha, DATA16 *pixels, int copy);
void soft16_image_free(Soft16_Image *im);
Soft16_Image *soft16_image_load(const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo);
void soft16_image_load_data(Soft16_Image *im);
void soft16_image_draw(Soft16_Image *src, Soft16_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h, int smooth);

void soft16_image_draw_unscaled(Soft16_Image *src, Soft16_Image *dst, RGBA_Draw_Context *dc, const Evas_Rectangle sr, const Evas_Rectangle dr, const Evas_Rectangle cr);
void soft16_image_draw_scaled_sampled(Soft16_Image *src, Soft16_Image *dst, RGBA_Draw_Context *dc, const Evas_Rectangle sr, const Evas_Rectangle dr, const Evas_Rectangle cr);


/**
 * Rectangle (evas_soft16_rectangle.c)
 */
void soft16_rectangle_draw(Soft16_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);


/**
 * Font (evas_soft16_font.c)
 */
void *soft16_font_glyph_new(void *data, RGBA_Font_Glyph *fg);
void  soft16_font_glyph_free(void *ext_dat);
void  soft16_font_glyph_draw(Soft16_Image *dst, void *data, RGBA_Draw_Context *dc, RGBA_Font_Glyph *fg, int x, int y);

#endif
