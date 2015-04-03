#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

static void
_ector_renderer_generic_shape_fill_set(Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Generic_Shape_Data *pd,
                                       const Ector_Renderer *r)
{
   _ector_renderer_replace(&pd->fill, r);
}

static const Ector_Renderer *
_ector_renderer_generic_shape_fill_get(Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Generic_Shape_Data *pd)
{
   return pd->fill;
}

static void
_ector_renderer_generic_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                                              Ector_Renderer_Generic_Shape_Data *pd,
                                              const Ector_Renderer *r)
{
   _ector_renderer_replace(&pd->stroke.fill, r);
}

static const Ector_Renderer *
_ector_renderer_generic_shape_stroke_fill_get(Eo *obj EINA_UNUSED,
                                              Ector_Renderer_Generic_Shape_Data *pd)
{
   return pd->stroke.fill;
}

static void
_ector_renderer_generic_shape_stroke_marker_set(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Generic_Shape_Data *pd,
                                                const Ector_Renderer *r)
{
   _ector_renderer_replace(&pd->stroke.marker, r);
}

static const Ector_Renderer *
_ector_renderer_generic_shape_stroke_marker_get(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Generic_Shape_Data *pd)
{
   return pd->stroke.marker;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_scale_set(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Generic_Shape_Data *pd,
                                                             double s)
{
   pd->stroke.scale = s;
}

static double
_ector_renderer_generic_shape_efl_gfx_shape_stroke_scale_get(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Generic_Shape_Data *pd)
{
   return pd->stroke.scale;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_color_set(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Generic_Shape_Data *pd,
                                                             int r, int g, int b, int a)
{
   pd->stroke.color.r = r;
   pd->stroke.color.g = g;
   pd->stroke.color.b = b;
   pd->stroke.color.a = a;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_color_get(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Generic_Shape_Data *pd,
                                                             int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->stroke.color.r;
   if (g) *g = pd->stroke.color.g;
   if (b) *b = pd->stroke.color.b;
   if (a) *a = pd->stroke.color.a;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_width_set(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Generic_Shape_Data *pd,
                                                             double w)
{
   pd->stroke.width = w;
}

static double
_ector_renderer_generic_shape_efl_gfx_shape_stroke_width_get(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Generic_Shape_Data *pd)
{
   return pd->stroke.width;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_location_set(Eo *obj EINA_UNUSED,
                                                                Ector_Renderer_Generic_Shape_Data *pd,
                                                                double centered)
{
   pd->stroke.centered = centered;
}

static double
_ector_renderer_generic_shape_efl_gfx_shape_stroke_location_get(Eo *obj EINA_UNUSED,
                                                                Ector_Renderer_Generic_Shape_Data *pd)
{
   return pd->stroke.centered;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_dash_set(Eo *obj EINA_UNUSED,
                                                            Ector_Renderer_Generic_Shape_Data *pd,
                                                            const Efl_Gfx_Dash *dash,
                                                            unsigned int length)
{
   Efl_Gfx_Dash *tmp;

   if (!dash)
     {
        free(pd->stroke.dash);
        pd->stroke.dash = NULL;
        pd->stroke.dash_length = 0;
        return ;
     }

   tmp = realloc(pd->stroke.dash, length * sizeof (Efl_Gfx_Dash));
   if (!tmp && length) return ;
   memcpy(tmp, dash, length * sizeof (Efl_Gfx_Dash));

   pd->stroke.dash = tmp;
   pd->stroke.dash_length = length;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_dash_get(Eo *obj EINA_UNUSED,
                                                            Ector_Renderer_Generic_Shape_Data *pd,
                                                            const Efl_Gfx_Dash **dash,
                                                            unsigned int *length)
{
   if (dash) *dash = pd->stroke.dash;
   if (length) *length = pd->stroke.dash_length;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_cap_set(Eo *obj EINA_UNUSED,
                                                           Ector_Renderer_Generic_Shape_Data *pd,
                                                           Efl_Gfx_Cap c)
{
   pd->stroke.cap = c;
}

static Efl_Gfx_Cap
_ector_renderer_generic_shape_efl_gfx_shape_stroke_cap_get(Eo *obj EINA_UNUSED,
                                                           Ector_Renderer_Generic_Shape_Data *pd)
{
   return pd->stroke.cap;
}

static void
_ector_renderer_generic_shape_efl_gfx_shape_stroke_join_set(Eo *obj EINA_UNUSED,
                                                            Ector_Renderer_Generic_Shape_Data *pd,
                                                            Efl_Gfx_Join j)
{
   pd->stroke.join = j;
}

static Efl_Gfx_Join
_ector_renderer_generic_shape_efl_gfx_shape_stroke_join_get(Eo *obj EINA_UNUSED,
                                                            Ector_Renderer_Generic_Shape_Data *pd)
{
   return pd->stroke.join;
}

#include "ector_renderer_generic_shape.eo.c"
