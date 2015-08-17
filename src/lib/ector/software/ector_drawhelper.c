#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ector.h>
#include "ector_drawhelper_private.h"

/*
  s = source pixel
  d = destination pixel
  ca = const_alpha
  sia = source inverse alpha
  cia = const inverse alpha

*/

/*
  result = s + d * sia  
  dest = (s + d * sia) * ca + d * cia
       = s * ca + d * (sia * ca + cia)
       = s * ca + d * (1 - sa*ca)
*/
void
comp_func_solid_source_over(uint *dest, int length, uint color, uint const_alpha)
{
   int ialpha, i;
   if (const_alpha != 255)
     color = BYTE_MUL(color, const_alpha);
   ialpha = Alpha(~color);
   for (i = 0; i < length; ++i)
     dest[i] = color + BYTE_MUL(dest[i], ialpha);
}


static void
comp_func_source_over(uint *dest, const uint *src, int length, uint color, uint const_alpha)
{
   int i;
   uint s, sc, sia;
   if (const_alpha != 255)
     color = BYTE_MUL(color, const_alpha);

   if (color == 0xffffffff) // No color multiplier
     {
        for (i = 0; i < length; ++i)
          {
             s = src[i];
             if (s >= 0xff000000)
               dest[i] = s;
             else if (s != 0)
               {
                  sia = Alpha(~s);
                  dest[i] = s + BYTE_MUL(dest[i], sia);
               }
          }
     }
   else
     {
        for (i = 0; i < length; ++i)
          {
             s = src[i];
             sc = ECTOR_MUL4_SYM(color, s);
             sia = Alpha(~sc);
             dest[i] = sc + BYTE_MUL(dest[i], sia);
          }
     }
}

/*
  result = s
  dest = s * ca + d * cia
*/
static void
comp_func_solid_source(uint *dest, int length, uint color, uint const_alpha)
{
   int ialpha, i;
   if (const_alpha == 255) _ector_memfill(dest, length, color);
   else
     {
        ialpha = 255 - const_alpha;
        color = BYTE_MUL(color, const_alpha);
        for (i = 0; i < length; ++i)
          dest[i] = color + BYTE_MUL(dest[i], ialpha);
     }
}

static void
comp_func_source(uint *dest, const uint *src, int length, uint color, uint const_alpha)
{
   int i, ialpha;
   uint src_color;
   if (color == 0xffffffff) // No color multiplier
     {
        if (const_alpha == 255)
          memcpy(dest, src, length * sizeof(uint));
        else
         {
            ialpha = 255 - const_alpha;
            for (i = 0; i < length; ++i)
              dest[i] = INTERPOLATE_PIXEL_256(src[i], const_alpha, dest[i], ialpha);
         }
     }
   else
     {
        if (const_alpha == 255)
          {
             for (i = 0; i < length; ++i)
               dest[i] = ECTOR_MUL4_SYM(src[i], color);
          }
        else
          {
            ialpha = 255 - const_alpha;
            for (i = 0; i < length; ++i)
              {
                 src_color = ECTOR_MUL4_SYM(src[i], color);
                 dest[i] = INTERPOLATE_PIXEL_256(src_color, const_alpha, dest[i], ialpha);
            }
          }
     }
}

RGBA_Comp_Func_Solid func_for_mode_solid[ECTOR_ROP_LAST] = {
        comp_func_solid_source_over,
        comp_func_solid_source
};

RGBA_Comp_Func func_for_mode[ECTOR_ROP_LAST] = {
        comp_func_source_over,
        comp_func_source
};

RGBA_Comp_Func_Solid
ector_comp_func_solid_span_get(Ector_Rop op, uint color)
{
   if ((color & 0xff000000) == 0xff000000)
     {
        if (op == ECTOR_ROP_BLEND) op = ECTOR_ROP_COPY;
     }

   return func_for_mode_solid[op];
}

RGBA_Comp_Func ector_comp_func_span_get(Ector_Rop op, uint color, Eina_Bool src_alpha)
{
   if (((color & 0xff000000) == 0xff000000) && !src_alpha)
     {
        if (op == ECTOR_ROP_BLEND) op = ECTOR_ROP_COPY;
     }
   return func_for_mode[op];
}

extern void init_drawhelper_gradient();
extern void init_draw_helper_sse2();
extern void init_draw_helper_neon();

void init_draw_helper()
{
   init_drawhelper_gradient();
   init_draw_helper_sse2();
   init_draw_helper_neon();
}
