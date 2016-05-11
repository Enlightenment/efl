#ifndef ECTOR_CAIRO_PRIVATE_H_
# define ECTOR_CAIRO_PRIVATE_H_

#include "draw.h"

typedef void cairo_pattern_t;

typedef struct {
   double xx; double yx;
   double xy; double yy;
   double x0; double y0;
} cairo_matrix_t;

typedef struct _Ector_Cairo_Surface_Data Ector_Cairo_Surface_Data;
typedef struct _Ector_Renderer_Cairo_Data Ector_Renderer_Cairo_Data;

struct _Ector_Cairo_Surface_Data
{
   cairo_t *cairo;
   struct {
      double x, y;
   } current;
};

struct _Ector_Renderer_Cairo_Data
{
   Ector_Cairo_Surface_Data *parent;
   Ector_Renderer_Generic_Base_Data *generic;

   cairo_matrix_t *m;
};

typedef enum _cairo_extend {
    CAIRO_EXTEND_NONE,
    CAIRO_EXTEND_REPEAT,
    CAIRO_EXTEND_REFLECT,
    CAIRO_EXTEND_PAD
} cairo_extend_t;

static inline cairo_extend_t
_ector_cairo_extent_get(Efl_Gfx_Gradient_Spread s)
{
   switch (s)
     {
       case EFL_GFX_GRADIENT_SPREAD_PAD:
          return CAIRO_EXTEND_PAD;
       case EFL_GFX_GRADIENT_SPREAD_REFLECT:
          return CAIRO_EXTEND_REFLECT;
       case EFL_GFX_GRADIENT_SPREAD_REPEAT:
          return CAIRO_EXTEND_REPEAT;
       default:
          return CAIRO_EXTEND_NONE;
     }
}

#define CHECK_CAIRO(Parent) (!(Parent && Parent->cairo))

#define USE(Obj, Sym, Error)                            \
  if (!Sym) Sym = _ector_cairo_symbol_get(Obj, #Sym);   \
  if (!Sym) return Error;

static inline void *
_ector_cairo_symbol_get(Ector_Renderer_Generic_Base_Data *base,
                        const char *name)
{
   void *sym;

   sym = ector_cairo_surface_symbol_get(base->surface, name);
   return sym;
}

extern void (*cairo_pattern_add_color_stop_rgba)(cairo_pattern_t *pattern, double offset,
                                                 double red, double green, double blue, double alpha);

static inline void
_ector_renderer_cairo_gradient_prepare(cairo_pattern_t *pat,
                                       Ector_Renderer_Generic_Gradient_Data *src,
                                       unsigned int mul_col)
{
   unsigned int i;

   for (i = 0; i < src->colors_count; i++)
     {
        int r,g,b,a;

        r = ((src->colors[i].r * R_VAL(&mul_col)) >> 8);
        g = ((src->colors[i].g * G_VAL(&mul_col)) >> 8);
        b = ((src->colors[i].b * B_VAL(&mul_col)) >> 8);
        a = ((src->colors[i].a * A_VAL(&mul_col)) >> 8);
        ector_color_argb_unpremul(a, &r, &g, &b);
        cairo_pattern_add_color_stop_rgba(pat, src->colors[i].offset, r/255.0, g/255.0, b/255.0, a/255.0);
     }
}

#endif
