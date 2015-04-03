#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <cairo/Ector_Cairo.h>

#include "ector_private.h"
#include "ector_cairo_private.h"

static cairo_pattern_t *(*cairo_pattern_create_linear)(double x0, double y0,
                                                       double x1, double y1) = NULL;
static void (*cairo_set_source)(cairo_t *cr, cairo_pattern_t *source) = NULL;
static void (*cairo_fill)(cairo_t *cr) = NULL;
static void (*cairo_rectangle)(cairo_t *cr,
                               double x, double y,
                               double width, double height) = NULL;
static void (*cairo_pattern_add_color_stop_rgba)(cairo_pattern_t *pattern, double offset,
                                                 double red, double green, double blue, double alpha) = NULL;
static void (*cairo_pattern_destroy)(cairo_pattern_t *pattern) = NULL;

typedef struct _Ector_Renderer_Cairo_Gradient_Linear_Data Ector_Renderer_Cairo_Gradient_Linear_Data;
struct _Ector_Renderer_Cairo_Gradient_Linear_Data
{
   Ector_Cairo_Surface_Data *parent;
   cairo_pattern_t *pat;
};

static Eina_Bool
_ector_renderer_cairo_gradient_linear_ector_renderer_generic_base_prepare(Eo *obj,
                                                                          Ector_Renderer_Cairo_Gradient_Linear_Data *pd)
{
   Ector_Renderer_Generic_Gradient_Linear_Data *gld;
   Ector_Renderer_Generic_Gradient_Data *gd;
   unsigned int i;

   if (pd->pat) return EINA_FALSE;

   gld = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_CLASS);
   gd = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_CLASS);
   if (!gld || !gd) return EINA_FALSE;

   USE(obj, cairo_pattern_create_linear, EINA_FALSE);
   USE(obj, cairo_pattern_add_color_stop_rgba, EINA_FALSE);

   pd->pat = cairo_pattern_create_linear(gld->start.x, gld->start.y,
                                     gld->end.x, gld->end.y);
   for (i = 0; i < gd->colors_count; i++)
     cairo_pattern_add_color_stop_rgba(pd->pat, gd->colors[i].offset,
                                       gd->colors[i].r, gd->colors[i].g,
                                       gd->colors[i].b, gd->colors[i].a);

   if (!pd->parent)
     {
        Eo *parent;

        eo_do(obj, parent = eo_parent_get());
        if (!parent) return EINA_FALSE;
        pd->parent = eo_data_xref(parent, ECTOR_CAIRO_SURFACE_CLASS, obj);
     }

   return EINA_FALSE;
}

static Eina_Bool
_ector_renderer_cairo_gradient_linear_ector_renderer_generic_base_draw(Eo *obj,
                                                                       Ector_Renderer_Cairo_Gradient_Linear_Data *pd,
                                                                       Ector_Rop op, Eina_Array *clips, int x, int y, unsigned int mul_col)
{
   Ector_Renderer_Generic_Gradient_Linear_Data *gld;

   // FIXME: don't ignore clipping !
   gld = eo_data_scope_get(obj, ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_CLASS);
   if (!pd->pat || !gld || CHECK_CAIRO(pd->parent)) return EINA_FALSE;

   USE(obj, cairo_rectangle, EINA_FALSE);

   cairo_rectangle(pd->parent->cairo, gld->start.x - x, gld->start.y - y,
                   gld->end.x - gld->start.x,
                   gld->end.y - gld->start.y);
   eo_do(obj, ector_renderer_cairo_base_fill());
   cairo_fill(pd->parent->cairo);

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_cairo_gradient_linear_ector_renderer_cairo_base_fill(Eo *obj,
                                                                     Ector_Renderer_Cairo_Gradient_Linear_Data *pd)
{
   if (!pd->pat || CHECK_CAIRO(pd->parent)) return EINA_FALSE;

   USE(obj, cairo_set_source, EINA_FALSE);

   cairo_set_source(pd->parent->cairo, pd->pat);

   return EINA_TRUE;
}

void
_ector_renderer_cairo_gradient_linear_eo_base_destructor(Eo *obj,
                                                         Ector_Renderer_Cairo_Gradient_Linear_Data *pd)
{
   Eo *parent;

   USE(obj, cairo_pattern_destroy, );

   if (pd->pat) cairo_pattern_destroy(pd->pat);
   pd->pat = NULL;

   eo_do(obj, parent = eo_parent_get());
   eo_data_xunref(parent, pd->parent, obj);

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS, eo_destructor());
}

void
_ector_renderer_cairo_gradient_linear_efl_gfx_gradient_stop_set(Eo *obj, Ector_Renderer_Cairo_Gradient_Linear_Data *pd, const Efl_Gfx_Gradient_Stop *colors, unsigned int length)
{
   USE(obj, cairo_pattern_destroy, );

   if (pd->pat) cairo_pattern_destroy(pd->pat);
   pd->pat = NULL;

   eo_do_super(obj, ECTOR_RENDERER_CAIRO_GRADIENT_LINEAR_CLASS,
               efl_gfx_gradient_stop_set(colors, length));
}

#include "ector_renderer_cairo_gradient_linear.eo.c"
