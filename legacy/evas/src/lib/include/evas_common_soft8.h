#ifndef EVAS_COMMON_SOFT8_H
#define EVAS_COMMON_SOFT8_H

#include "evas_common.h"
#include "evas_private.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GRY_8_BLEND(a, b, alpha)	\
   ((b) + (a) - ((((b) * ((alpha) + 1)) >> 8) & 0xff))

#define GRY_8_BLEND_UNMUL(a, b, alpha)	\
   ((b) + ((((a) - (b)) * ((alpha) + 1)) >> 8))

#if 0
#define GRY_8_FROM_COMPONENTS(r, g, b)	\
     (((307 * (r)) +	\
       (604 * (g)) +	\
       (113 * (b))) >> 10)

#define GRY_8_FROM_RGB(rgb)	\
     GRY_8_FROM_COMPONENTS(R_VAL(rgb), G_VAL(rgb), B_VAL(rgb))
#endif

//Y = R * 0.2126 + G * 0.7152 + B * 0.0722

#define GRY_8_FROM_COMPONENTS(r, g, b)	\
     (((218 * (r)) +	\
       (732 * (g)) +	\
       (74  * (b))) >> 10)

#define GRY_8_FROM_RGB(rgb)	\
     GRY_8_FROM_COMPONENTS(R_VAL(rgb), G_VAL(rgb), B_VAL(rgb))

#define UNROLL2(op...) op op
#define UNROLL4(op...) UNROLL2(op) UNROLL2(op)
#define UNROLL8(op...) UNROLL4(op) UNROLL4(op)
#define UNROLL16(op...) UNROLL8(op) UNROLL8(op)

#if defined(__ARM_ARCH_3M__)
# define __ARM_ARCH__ 40
#endif
#if defined(__ARM_ARCH_4__)
# define __ARM_ARCH__ 40
#endif
#if defined(__ARM_ARCH_4T__)
# define __ARM_ARCH__ 41
#endif

#if defined(__ARM_ARCH_5__)
# define __ARM_ARCH__ 50
#endif
#if defined(__ARM_ARCH_5T__)
# define __ARM_ARCH__ 51
#endif
#if defined(__ARM_ARCH_5E__)
# define __ARM_ARCH__ 52
#endif
#if defined(__ARM_ARCH_5TE__)
# define __ARM_ARCH__ 53
#endif
#if defined(__ARM_ARCH_5TEJ__)
# define __ARM_ARCH__ 54
#endif

#if defined(__ARM_ARCH_6__)
# define __ARM_ARCH__ 60
#endif
#if defined(__ARM_ARCH_6J__)
# define __ARM_ARCH__ 61
#endif
#if defined(__ARM_ARCH_6K__)
# define __ARM_ARCH__ 62
#endif
#if defined(__ARM_ARCH_6Z__)
# define __ARM_ARCH__ 63
#endif
#if defined(__ARM_ARCH_6ZK__)
# define __ARM_ARCH__ 64
#endif
#if defined(__ARM_ARCH_6T2__)
# define __ARM_ARCH__ 65
#endif

#if defined(__ARM_ARCH_7__)
# define __ARM_ARCH__ 70
#endif
#if defined(__ARM_ARCH_7A__)
# define __ARM_ARCH__ 71
#endif
#if defined(__ARM_ARCH_7R__)
# define __ARM_ARCH__ 72
#endif
#if defined(__ARM_ARCH_7M__)
# define __ARM_ARCH__ 73
#endif

#if defined(__ARM_ARCH__) && (__ARM_ARCH__ >= 52)
/* tested on ARMv6 (arm1136j-s), Nokia N800 CPU */
#define pld(addr, off)                                                  \
   __asm__("pld [%[address], %[offset]]"::                              \
           [address] "r" (addr), [offset] "i" (off))
#else
#define pld(addr, off)
#endif /* __ARMEL__ */

static inline unsigned int
_calc_stride(unsigned int w)
{
   unsigned int pad = w % 4;
   if (!pad)  return w;
   else return w + 4 - pad;
}

#define IMG_BYTE_SIZE(stride, height, has_alpha)                       \
   ((stride) * (height) * (!(has_alpha) ? 1 : 2))

typedef struct _Soft8_Image Soft8_Image;
struct _Soft8_Image
{
   Image_Entry    cache_entry;

   RGBA_Image    *source;

   int            stride;     // pixel stride
   DATA8        *pixels;      // 8bpp pixels gry8
   DATA8         *alpha;      // 8bit alpha mask - optional. points into pixels

   void                *extended_info;

   struct
   {
/*      unsigned char  have_alpha  : 1; // 1 if we have halpha */
     unsigned char  free_pixels : 1; // 1 if pixels should be freed
     unsigned char  free_alpha  : 1; // 1 if alpha mask should be freed
   } flags;
};

/**
 * Image (evas_soft8_main.c)
 */
EAPI void                evas_common_soft8_image_init(void);
EAPI void                evas_common_soft8_image_shutdown(void);
EAPI Evas_Cache_Image   *evas_common_soft8_image_cache_get(void);

EAPI void                evas_common_soft8_image_draw(Soft8_Image *src, Soft8_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h, int smooth);
EAPI Soft8_Image       *evas_common_soft8_image_alpha_set(Soft8_Image *im, int have_alpha);

void                     evas_common_soft8_image_draw_unscaled(Soft8_Image *src, Soft8_Image *dst, RGBA_Draw_Context *dc, const Eina_Rectangle sr, const Eina_Rectangle dr, const Eina_Rectangle cr);
void                     evas_common_soft8_image_draw_scaled_sampled(Soft8_Image *src, Soft8_Image *dst, RGBA_Draw_Context *dc, const Eina_Rectangle sr, const Eina_Rectangle dr, const Eina_Rectangle cr);

/* convert/dither functions */
void                     evas_common_soft8_image_convert_from_rgb(Soft8_Image *im, const DATA32 *src);
void                     evas_common_soft8_image_convert_from_rgba(Soft8_Image *im, const DATA32 *src);

/**
 * Rectangle (evas_soft8_rectangle.c)
 */
EAPI void                evas_common_soft8_rectangle_draw(Soft8_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h);

/**
 * Polygon (evas_soft8_polygon.c)
 */
EAPI void                evas_common_soft8_polygon_draw(Soft8_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points, int x, int y);

/**
 * Line (evas_soft8_line.c)
 */
EAPI void                evas_common_soft8_line_draw(Soft8_Image *dst, RGBA_Draw_Context *dc, int x0, int y0, int x1, int y1);

/**
 * Font (evas_soft8_font.c)
 */
EAPI void               *evas_common_soft8_font_glyph_new(void *data, RGBA_Font_Glyph *fg);
EAPI void                evas_common_soft8_font_glyph_free(void *ext_dat);
EAPI void                evas_common_soft8_font_glyph_draw(void *data, void *dest, void *context, RGBA_Font_Glyph *fg, int x, int y);

#ifdef __cplusplus
}
#endif

#endif
