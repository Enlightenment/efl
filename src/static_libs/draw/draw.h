#ifndef EFL_DRAW_H
#define EFL_DRAW_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include "efl/interfaces/efl_gfx_types.eot.h"

/* FIXME: naming convention */
/* FIXME: mul_col & const_alpha are redondant */
typedef void (*RGBA_Comp_Func)       (uint32_t *dest, const uint32_t *src, int length, uint32_t mul_col, uint32_t const_alpha);
typedef void (*RGBA_Comp_Func_Solid) (uint32_t *dest, int length, uint32_t color, uint32_t const_alpha);
typedef void (*RGBA_Comp_Func_Mask)  (uint32_t *dest, const uint8_t *mask, int length, uint32_t color);
typedef void (*Draw_Func_ARGB_Mix3)  (uint32_t *dest, const uint32_t *src, const uint32_t *mul, int len, uint32_t color);
typedef void (*Draw_Func_Alpha)      (uint8_t *dest, const uint8_t *src, int len);
typedef Eina_Bool (*Cspace_Convert_Func) (void *dst, const void *src, int w, int h, int src_stride, int dst_stride, Eina_Bool has_alpha, Efl_Gfx_Colorspace srccs, Efl_Gfx_Colorspace dstcs);

int efl_draw_init(void);

RGBA_Comp_Func       efl_draw_func_span_get         (Efl_Gfx_Render_Op op, uint32_t color, Eina_Bool src_alpha);
RGBA_Comp_Func_Solid efl_draw_func_solid_span_get   (Efl_Gfx_Render_Op op, uint32_t color);
RGBA_Comp_Func_Mask  efl_draw_func_mask_span_get    (Efl_Gfx_Render_Op op, uint32_t color);
Draw_Func_ARGB_Mix3  efl_draw_func_argb_mix3_get    (Efl_Gfx_Render_Op op, uint32_t color);
Draw_Func_Alpha      efl_draw_alpha_func_get        (Efl_Gfx_Render_Op op, Eina_Bool has_mask);
Cspace_Convert_Func  efl_draw_convert_func_get      (Efl_Gfx_Colorspace origcs, Efl_Gfx_Colorspace dstcs, Eina_Bool *region_can);

int efl_draw_argb_premul(uint32_t *data, unsigned int len);
void efl_draw_argb_unpremul(uint32_t *data, unsigned int len);

Eina_Bool efl_draw_generate_gradient_color_table(Efl_Gfx_Gradient_Stop *gradient_stops, int stop_count, uint32_t *color_table, int size);


/* common sw draw helpers */

#ifndef MIN
#define MIN( a, b )  ( (a) < (b) ? (a) : (b) )
#endif

#ifndef MAX
#define MAX( a, b )  ( (a) > (b) ? (a) : (b) )
#endif

#ifndef A_VAL
#ifndef WORDS_BIGENDIAN
/* x86 */
#define A_VAL(p) (((uint8_t *)(p))[3])
#define R_VAL(p) (((uint8_t *)(p))[2])
#define G_VAL(p) (((uint8_t *)(p))[1])
#define B_VAL(p) (((uint8_t *)(p))[0])
#define AR_VAL(p) ((uint16_t *)(p)[1])
#define GB_VAL(p) ((uint16_t *)(p)[0])
#else
/* ppc */
#define A_VAL(p) (((uint8_t *)(p))[0])
#define R_VAL(p) (((uint8_t *)(p))[1])
#define G_VAL(p) (((uint8_t *)(p))[2])
#define B_VAL(p) (((uint8_t *)(p))[3])
#define AR_VAL(p) ((uint16_t *)(p)[0])
#define GB_VAL(p) ((uint16_t *)(p)[1])
#endif
#endif

#define DRAW_ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

/* argb multiply */
#define DRAW_MUL4_SYM(x, y) \
 ( ((((((x) >> 16) & 0xff00) * (((y) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) + \
   ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff0000) >> 16) & 0xff00) + \
   (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

/* alpha from 1 to 256 */
static inline uint32_t
draw_mul_256(int a, uint32_t c)
{
   return (((((c) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) |
         (((((c) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff);
}

static inline uint32_t
draw_interpolate_256(uint32_t x, int a, uint32_t y, int b)
{
   uint32_t t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
   t >>= 8;
   t &= 0xff00ff;
   x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
   x &= 0xff00ff00;
   x |= t;
   return x;
}

static inline void
draw_memset32(uint32_t *dest, uint32_t value, int length)
{
   int n;

   if (length <= 0)
     return;

   // Cute hack to align future memcopy operation
   // and do unroll the loop a bit. Not sure it is
   // the most efficient, but will do for now.
   n = (length + 7) / 8;
   switch (length & 0x07)
     {
        case 0: do { *dest++ = value;
           EINA_FALLTHROUGH;
        case 7:      *dest++ = value;
           EINA_FALLTHROUGH;
        case 6:      *dest++ = value;
           EINA_FALLTHROUGH;
        case 5:      *dest++ = value;
           EINA_FALLTHROUGH;
        case 4:      *dest++ = value;
           EINA_FALLTHROUGH;
        case 3:      *dest++ = value;
           EINA_FALLTHROUGH;
        case 2:      *dest++ = value;
           EINA_FALLTHROUGH;
        case 1:      *dest++ = value;
        } while (--n > 0);
     }
}

#endif
