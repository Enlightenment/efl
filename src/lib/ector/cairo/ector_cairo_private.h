#ifndef ECTOR_CAIRO_PRIVATE_H_
# define ECTOR_CAIRO_PRIVATE_H_

typedef void cairo_pattern_t;

typedef struct {
   double xx; double yx;
   double xy; double yy;
   double x0; double y0;
} cairo_matrix_t;

typedef struct _Ector_Cairo_Surface_Data Ector_Cairo_Surface_Data;
typedef struct _Ector_Renderer_Cairo_Base_Data Ector_Renderer_Cairo_Base_Data;

struct _Ector_Cairo_Surface_Data
{
   cairo_t *cairo;
   struct {
      double x, y;
   } current;

   Eina_Bool internal : 1;
};

struct _Ector_Renderer_Cairo_Base_Data
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
_ector_cairo_symbol_get(Eo *obj, const char *name)
{
   Eo *parent;
   void *sym;

   eo_do(obj, parent = eo_parent_get());
   if (!parent) return NULL;

   eo_do(parent, sym = ector_cairo_surface_symbol_get(name));
   return sym;
}

#endif
