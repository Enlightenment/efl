#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "draw_private.h"

int _draw_log_dom = -1;

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
_comp_func_solid_source_over(uint32_t *dest, int length, uint32_t color, uint32_t const_alpha)
{
   int ialpha, i;

   if (const_alpha != 255)
     color = DRAW_BYTE_MUL(color, const_alpha);
   ialpha = alpha_inverse(color);
   for (i = 0; i < length; ++i)
     dest[i] = color + DRAW_BYTE_MUL(dest[i], ialpha);
}

static void
_comp_func_source_over(uint32_t *dest, const uint32_t *src, int length, uint32_t color, uint32_t const_alpha)
{
   int i;
   uint32_t s, sc, sia;

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
_comp_func_solid_source(uint32_t *dest, int length, uint32_t color, uint32_t const_alpha)
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
_comp_func_source(uint32_t *dest, const uint32_t *src, int length, uint32_t color, uint32_t const_alpha)
{
   int i, ialpha;
   uint32_t src_color;

   if (color == 0xffffffff) // No color multiplier
     {
        if (const_alpha == 255)
          {
             memcpy(dest, src, length * sizeof(uint32_t));
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

/* s = m * color
 * d = d * (1-sa) + s * sa
 */
static void
_comp_func_mask_blend(uint32_t *dest, const uint8_t *mask, int length, uint32_t color)
{
   int k;

   for (k = 0; k < length; k++, dest++, mask++)
     {
        uint32_t c = draw_mul_256((*mask + 1), color);
        int a = 256 - (c >> 24);
        *dest = c + draw_mul_256(a, *dest);
     }
}

/* s = m * color
 * d = s * sa
 */
static void
_comp_func_mask_copy(uint32_t *dest, const uint8_t *mask, int length, uint32_t color)
{
   int k;

   for (k = 0; k < length; k++, dest++, mask++)
     {
        *dest = draw_mul_256(*mask + 1, color);
     }
}

/* w = s * m * c
 * d = d * (1-wa) + w * wa
 */
static void
_comp_func_mix3_blend(uint32_t *dest, const uint32_t *src, const uint32_t *mul, int len, uint32_t color)
{
   int k, a;

   for (k = 0; k < len; k++, dest++, src++, mul++)
     {
        uint32_t c = DRAW_MUL4_SYM(*mul, color);
        c = DRAW_MUL4_SYM(c, *src);
        a = 256 - (c >> 24);
        *dest = c + draw_mul_256(a, *dest);
     }
}

/* d = s * m * c */
static void
_comp_func_mix3_copy(uint32_t *dest, const uint32_t *src, const uint32_t *mul, int len, uint32_t color)
{
   int k;

   for (k = 0; k < len; k++, dest++, src++, mul++)
     {
        uint32_t c = DRAW_MUL4_SYM(*mul, color);
        *dest = DRAW_MUL4_SYM(c, *src);
     }
}

/* w = s * m
 * d = d * (1-wa) + w * wa
 */
static void
_comp_func_mix3_blend_nomul(uint32_t *dest, const uint32_t *src, const uint32_t *mul, int len, uint32_t color EINA_UNUSED)
{
   int k, a;

   for (k = 0; k < len; k++, dest++, src++, mul++)
     {
        uint32_t c = DRAW_MUL4_SYM(*mul, *src);
        a = 256 - (c >> 24);
        *dest = c + draw_mul_256(a, *dest);
     }
}

/* d = s * m */
static void
_comp_func_mix3_copy_nomul(uint32_t *dest, const uint32_t *src, const uint32_t *mul, int len, uint32_t color EINA_UNUSED)
{
   int k;

   for (k = 0; k < len; k++, dest++, src++, mul++)
     {
        *dest = DRAW_MUL4_SYM(*mul, *src);
     }
}

RGBA_Comp_Func_Mask func_for_mode_mask[EFL_GFX_RENDER_OP_LAST] = {
   _comp_func_mask_blend,
   _comp_func_mask_copy
};

RGBA_Comp_Func_Solid func_for_mode_solid[EFL_GFX_RENDER_OP_LAST] = {
  _comp_func_solid_source_over,
  _comp_func_solid_source
};

RGBA_Comp_Func func_for_mode[EFL_GFX_RENDER_OP_LAST] = {
  _comp_func_source_over,
  _comp_func_source
};

Draw_Func_ARGB_Mix3 func_for_mode_argb_mix3[EFL_GFX_RENDER_OP_LAST * 2] = {
   _comp_func_mix3_blend,
   _comp_func_mix3_copy,
   _comp_func_mix3_blend_nomul,
   _comp_func_mix3_copy_nomul
};

RGBA_Comp_Func_Mask
efl_draw_func_mask_span_get(Efl_Gfx_Render_Op op, uint32_t color EINA_UNUSED)
{
   return func_for_mode_mask[op];
}

Draw_Func_ARGB_Mix3
efl_draw_func_argb_mix3_get(Efl_Gfx_Render_Op op, uint32_t color)
{
   if (color == 0xffffffff)
     return func_for_mode_argb_mix3[op + 2];
   else
     return func_for_mode_argb_mix3[op];
}

RGBA_Comp_Func_Solid
efl_draw_func_solid_span_get(Efl_Gfx_Render_Op op, uint32_t color)
{
   if ((color & 0xff000000) == 0xff000000)
     {
        if (op == EFL_GFX_RENDER_OP_BLEND) op = EFL_GFX_RENDER_OP_COPY;
     }

   return func_for_mode_solid[op];
}

RGBA_Comp_Func
efl_draw_func_span_get(Efl_Gfx_Render_Op op, uint32_t color, Eina_Bool src_alpha)
{
   if (((color & 0xff000000) == 0xff000000) && !src_alpha)
     {
        if (op == EFL_GFX_RENDER_OP_BLEND) op = EFL_GFX_RENDER_OP_COPY;
     }

   return func_for_mode[op];
}

static double
_ease_linear(double t)
{
   return t;
}

Eina_Bool
efl_draw_generate_gradient_color_table(Efl_Gfx_Gradient_Stop *gradient_stops, int stop_count, uint32_t *color_table, int size)
{
   int dist, idist, pos = 0, i;
   Eina_Bool alpha = EINA_FALSE;
   Efl_Gfx_Gradient_Stop *curr, *next;
   uint32_t current_color, next_color;
   double delta, t, incr, fpos;

   curr = gradient_stops;
   if (curr->a != 255) alpha = EINA_TRUE;
   current_color = DRAW_ARGB_JOIN(curr->a, curr->r, curr->g, curr->b);
   incr = 1.0 / (double)size;
   fpos = 1.5 * incr;

   color_table[pos++] = current_color;

   while (fpos <= curr->offset)
     {
        color_table[pos] = color_table[pos - 1];
        pos++;
        fpos += incr;
     }

   for (i = 0; i < stop_count - 1; ++i)
     {
        curr = (gradient_stops + i);
        next = (gradient_stops + i + 1);
        delta = 1/(next->offset - curr->offset);
        if (next->a != 255) alpha = EINA_TRUE;
        next_color = DRAW_ARGB_JOIN(next->a, next->r, next->g, next->b);
        while (fpos < next->offset && pos < size)
          {
             t = _ease_linear((fpos - curr->offset) * delta);
             dist = (int)(256 * t);
             idist = 256 - dist;
             color_table[pos] = draw_interpolate_256(current_color, idist, next_color, dist);
             ++pos;
             fpos += incr;
          }
        current_color = next_color;
     }

   for (;pos < size; ++pos)
     color_table[pos] = current_color;

   // Make sure the last color stop is represented at the end of the table
   color_table[size-1] = current_color;
   return alpha;
}


int
efl_draw_init(void)
{
   static int i = 0;
   if (!(i++))
     {
        _draw_log_dom = eina_log_domain_register("efl_draw", EINA_COLOR_ORANGE);
        efl_draw_sse2_init();
        efl_draw_neon_init();
     }
   return i;
}
