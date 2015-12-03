#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Ector.h>
#include "draw_private.h"

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

static void
_comp_func_solid_source_over(uint *dest, int length, uint color, uint const_alpha)
{
   int ialpha, i;

   if (const_alpha != 255)
     color = DRAW_BYTE_MUL(color, const_alpha);
   ialpha = alpha_inverse(color);
   for (i = 0; i < length; ++i)
     dest[i] = color + DRAW_BYTE_MUL(dest[i], ialpha);
}

static void
_comp_func_source_over(uint *dest, const uint *src, int length, uint color, uint const_alpha)
{
   int i;
   uint s, sc, sia;

   if (const_alpha != 255)
     color = DRAW_BYTE_MUL(color, const_alpha);

   if (color == 0xffffffff) // No color multiplier
     {
        for (i = 0; i < length; ++i)
          {
             s = src[i];
             if (s >= 0xff000000)
               dest[i] = s;
             else if (s != 0)
               {
                  sia = alpha_inverse(s);
                  dest[i] = s + DRAW_BYTE_MUL(dest[i], sia);
               }
          }
     }
   else
     {
        for (i = 0; i < length; ++i)
          {
             s = src[i];
             sc = DRAW_MUL4_SYM(color, s);
             sia = alpha_inverse(sc);
             dest[i] = sc + DRAW_BYTE_MUL(dest[i], sia);
          }
     }
}

/*
  result = s
  dest = s * ca + d * cia
*/
static void
_comp_func_solid_source(uint *dest, int length, uint color, uint const_alpha)
{
   int ialpha, i;

   if (const_alpha == 255)
     {
        draw_memset32(dest, color, length);
     }
   else
     {
        ialpha = 255 - const_alpha;
        color = DRAW_BYTE_MUL(color, const_alpha);
        for (i = 0; i < length; ++i)
          dest[i] = color + DRAW_BYTE_MUL(dest[i], ialpha);
     }
}

static void
_comp_func_source(uint *dest, const uint *src, int length, uint color, uint const_alpha)
{
   int i, ialpha;
   uint src_color;

   if (color == 0xffffffff) // No color multiplier
     {
        if (const_alpha == 255)
          {
             memcpy(dest, src, length * sizeof(uint));
          }
        else
          {
             ialpha = 255 - const_alpha;
             for (i = 0; i < length; ++i)
               dest[i] = draw_interpolate_256(src[i], const_alpha, dest[i], ialpha);
          }
     }
   else
     {
        if (const_alpha == 255)
          {
             for (i = 0; i < length; ++i)
               dest[i] = DRAW_MUL4_SYM(src[i], color);
          }
        else
          {
             ialpha = 255 - const_alpha;
             for (i = 0; i < length; ++i)
               {
                  src_color = DRAW_MUL4_SYM(src[i], color);
                  dest[i] = draw_interpolate_256(src_color, const_alpha, dest[i], ialpha);
               }
          }
     }
}

RGBA_Comp_Func_Solid func_for_mode_solid[EFL_GFX_RENDER_OP_LAST] = {
  _comp_func_solid_source_over,
  _comp_func_solid_source
};

RGBA_Comp_Func func_for_mode[EFL_GFX_RENDER_OP_LAST] = {
  _comp_func_source_over,
  _comp_func_source
};

RGBA_Comp_Func_Solid
efl_draw_func_solid_span_get(Efl_Gfx_Render_Op op, uint color)
{
   if ((color & 0xff000000) == 0xff000000)
     {
        if (op == EFL_GFX_RENDER_OP_BLEND) op = EFL_GFX_RENDER_OP_COPY;
     }

   return func_for_mode_solid[op];
}

RGBA_Comp_Func
efl_draw_func_span_get(Efl_Gfx_Render_Op op, uint color, Eina_Bool src_alpha)
{
   if (((color & 0xff000000) == 0xff000000) && !src_alpha)
     {
        if (op == EFL_GFX_RENDER_OP_BLEND) op = EFL_GFX_RENDER_OP_COPY;
     }

   return func_for_mode[op];
}

int
efl_draw_init()
{
   static int i = 0;
   if (!(i++))
     {
        efl_draw_sse2_init();
        efl_draw_neon_init();
     }
   return i;
}
