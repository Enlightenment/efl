#ifndef EFL_DRAW_H
#define EFL_DRAW_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Efl.h>

typedef void (*RGBA_Comp_Func)(uint *dest, const uint *src, int length, uint mul_col, uint const_alpha);
typedef void (*RGBA_Comp_Func_Solid)(uint *dest, int length, uint color, uint const_alpha);

int efl_draw_init(void);

RGBA_Comp_Func_Solid efl_draw_func_solid_span_get(Efl_Gfx_Render_Op op, uint color);
RGBA_Comp_Func efl_draw_func_span_get(Efl_Gfx_Render_Op op, uint color, Eina_Bool src_alpha);


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

#define DRAW_MUL4_SYM(x, y) \
 ( ((((((x) >> 16) & 0xff00) * (((y) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) + \
   ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff0000) >> 16) & 0xff00) + \
   (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

static inline uint
INTERPOLATE_PIXEL_256(uint x, uint a, uint y, uint b)
{
   uint t = (x & 0xff00ff) * a + (y & 0xff00ff) * b;
   t >>= 8;
   t &= 0xff00ff;
   x = ((x >> 8) & 0xff00ff) * a + ((y >> 8) & 0xff00ff) * b;
   x &= 0xff00ff00;
   x |= t;
   return x;
}

static inline void
draw_memset32(uint *dest, uint value, int length)
{
   int n;

   if (length <= 0)
     return;

   n = (length + 7) / 8;
   switch (length & 0x07)
     {
        case 0: do { *dest++ = value;
        case 7:      *dest++ = value;
        case 6:      *dest++ = value;
        case 5:      *dest++ = value;
        case 4:      *dest++ = value;
        case 3:      *dest++ = value;
        case 2:      *dest++ = value;
        case 1:      *dest++ = value;
        } while (--n > 0);
     }
}

#endif
