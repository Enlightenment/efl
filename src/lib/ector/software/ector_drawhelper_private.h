#ifndef ECTOR_DRAWHELPER_PRIVATE_H
#define ECTOR_DRAWHELPER_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef MIN
#define MIN( a, b )  ( (a) < (b) ? (a) : (b) )
#endif

#ifndef MAX
#define MAX( a, b )  ( (a) > (b) ? (a) : (b) )
#endif

#ifndef uint
typedef unsigned int uint;
#endif

inline int Alpha(uint c)
{
   return c>>24;
}



#define ECTOR_ARGB_JOIN(a,r,g,b) \
        (((a) << 24) + ((r) << 16) + ((g) << 8) + (b))

#define ECTOR_MUL4_SYM(x, y) \
 ( ((((((x) >> 16) & 0xff00) * (((y) >> 16) & 0xff00)) + 0xff0000) & 0xff000000) + \
   ((((((x) >> 8) & 0xff00) * (((y) >> 16) & 0xff)) + 0xff00) & 0xff0000) + \
   ((((((x) & 0xff00) * ((y) & 0xff00)) + 0xff0000) >> 16) & 0xff00) + \
   (((((x) & 0xff) * ((y) & 0xff)) + 0xff) >> 8) )

#define BYTE_MUL(c, a) \
 ( (((((c) >> 8) & 0x00ff00ff) * (a)) & 0xff00ff00) + \
   (((((c) & 0x00ff00ff) * (a)) >> 8) & 0x00ff00ff) )

#define LOOP_ALIGNED_U1_A4(DEST, LENGTH, UOP, A4OP) \
   { \
      while((uintptr_t)DEST & 0xF && LENGTH) UOP \
   \
      while(LENGTH) { \
         switch(LENGTH) { \
            case 3: \
            case 2: \
            case 1: \
               UOP \
               break; \
            default: \
               A4OP \
               break; \
         } \
      } \
   }

static inline void
_ector_memfill(uint *dest, int length, uint value)
{
   int n;
   if (!length)
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

typedef void (*RGBA_Comp_Func)(uint *dest, const uint *src, int length, uint mul_col, uint const_alpha);
typedef void (*RGBA_Comp_Func_Solid)(uint *dest, int length, uint color, uint const_alpha);
extern RGBA_Comp_Func_Solid func_for_mode_solid[ECTOR_ROP_LAST];
extern RGBA_Comp_Func func_for_mode[ECTOR_ROP_LAST];

void init_draw_helper();

RGBA_Comp_Func_Solid ector_comp_func_solid_span_get(Ector_Rop op, uint color);
RGBA_Comp_Func ector_comp_func_span_get(Ector_Rop op, uint color, Eina_Bool src_alpha);

#endif