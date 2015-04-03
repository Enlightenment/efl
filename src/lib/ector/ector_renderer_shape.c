#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

typedef struct _Ector_Renderer_Shape_Data Ector_Renderer_Shape_Data;
struct _Ector_Renderer_Shape_Data
{
   Ector_Renderer *fill;
   struct {
      Ector_Renderer *fill;
      Ector_Renderer *marker;

      double scale;
      double width;
      double centered;

      struct {
         int r, g, b, a;
      } color;

      Efl_Geometry_Dash *dash;
      unsigned int dash_length;

      Efl_Geometry_Cap cap;
      Efl_Geometry_Cap join;
   } stroke;
};

void
_ector_renderer_shape_fill_set(Eo *obj EINA_UNUSED,
                               Ector_Renderer_Shape_Data *pd,
                               const Ector_Renderer *r)
{
   _ector_renderer_replace(&pd->fill, r);
}

const Ector_Renderer *
_ector_renderer_shape_fill_get(Eo *obj EINA_UNUSED,
                               Ector_Renderer_Shape_Data *pd)
{
   return pd->fill;
}

void
_ector_renderer_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                                      Ector_Renderer_Shape_Data *pd,
                                      const Ector_Renderer *r)
{
   _ector_renderer_replace(&pd->stroke.fill, r);
}

const Ector_Renderer *
_ector_renderer_shape_stroke_fill_get(Eo *obj EINA_UNUSED,
                                      Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.fill;
}

void
_ector_renderer_shape_stroke_marker_set(Eo *obj EINA_UNUSED,
                                        Ector_Renderer_Shape_Data *pd,
                                        const Ector_Renderer *r)
{
   _ector_renderer_replace(&pd->stroke.marker, r);
}

const Ector_Renderer *
_ector_renderer_shape_stroke_marker_get(Eo *obj EINA_UNUSED,
                                        Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.marker;
}

void
_ector_renderer_shape_efl_geometry_shape_stroke_scale_set(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Shape_Data *pd,
                                                          double s)
{
   pd->stroke.scale = s;
}

double
_ector_renderer_shape_efl_geometry_shape_stroke_scale_get(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.scale;
}

void
_ector_renderer_shape_efl_geometry_shape_stroke_color_set(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Shape_Data *pd,
                                                          int r, int g, int b, int a)
{
   pd->stroke.color.r = r;
   pd->stroke.color.g = g;
   pd->stroke.color.b = b;
   pd->stroke.color.a = a;
}


void
_ector_renderer_shape_efl_geometry_shape_stroke_color_get(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Shape_Data *pd,
                                                          int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->stroke.color.r;
   if (g) *g = pd->stroke.color.g;
   if (b) *b = pd->stroke.color.b;
   if (a) *a = pd->stroke.color.a;
}

void
_ector_renderer_shape_efl_geometry_shape_stroke_width_set(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Shape_Data *pd,
                                                          double w)
{
   pd->stroke.width = w;
}

double
_ector_renderer_shape_efl_geometry_shape_stroke_width_get(Eo *obj EINA_UNUSED,
                                                          Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.width;
}

void
_ector_renderer_shape_efl_geometry_shape_stroke_location_set(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Shape_Data *pd,
                                                             double centered)
{
   pd->stroke.centered = centered;
}

double
_ector_renderer_shape_efl_geometry_shape_stroke_location_get(Eo *obj EINA_UNUSED,
                                                             Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.centered;
}

void
_ector_renderer_shape_efl_geometry_shape_stroke_dash_set(Eo *obj EINA_UNUSED,
                                                         Ector_Renderer_Shape_Data *pd,
                                                         const Efl_Geometry_Dash *dash,
                                                         unsigned int length)
{
   Efl_Geometry_Dash *tmp;

   if (!dash)
     {
        free(pd->stroke.dash);
        pd->stroke.dash = NULL;
        pd->stroke.dash_length = 0;
        return ;
     }

   tmp = realloc(pd->stroke.dash, length * sizeof (Efl_Geometry_Dash));
   if (!tmp) return ;
   memcpy(tmp, dash, length * sizeof (Efl_Geometry_Dash));

   pd->stroke.dash = tmp;
   pd->stroke.dash_length = length;
}

void
_ector_renderer_shape_efl_geometry_shape_stroke_dash_get(Eo *obj EINA_UNUSED,
                                                         Ector_Renderer_Shape_Data *pd,
                                                         const Efl_Geometry_Dash **dash,
                                                         unsigned int *length)
{
   if (dash) *dash = pd->stroke.dash;
   if (length) *length = pd->stroke.dash_length;
}

void
_ector_renderer_shape_efl_geometry_shape_stroke_cap_set(Eo *obj EINA_UNUSED,
                                                        Ector_Renderer_Shape_Data *pd,
                                                        Efl_Geometry_Cap c)
{
   pd->stroke.cap = c;
}

Efl_Geometry_Cap
_ector_renderer_shape_efl_geometry_shape_stroke_cap_get(Eo *obj EINA_UNUSED,
                                                        Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.cap;
}

void
_ector_renderer_shape_efl_geometry_shape_stroke_join_set(Eo *obj EINA_UNUSED,
                                                         Ector_Renderer_Shape_Data *pd,
                                                         Efl_Geometry_Join j)
{
   pd->stroke.join = j;
}

Efl_Geometry_Join
_ector_renderer_shape_efl_geometry_shape_stroke_join_get(Eo *obj EINA_UNUSED,
                                                         Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.join;
}

Eina_Bool
_ector_renderer_shape_efl_geometry_shape_path_set(Eo *obj,
                                                  Ector_Renderer_Shape_Data *pd,
                                                  const Efl_Geometry_Path_Command *op,
                                                  const double *points)
{
}

Eina_Bool
_ector_renderer_shape_ector_renderer_base_prepare(Eo *obj,
                                                  Ector_Renderer_Shape_Data *pd,
                                                  Ector_Surface *s)
{
}

void
_ector_renderer_shape_eo_base_constructor(Eo *obj,
                                          Ector_Renderer_Shape_Data *pd)
{
}

void
_ector_renderer_shape_eo_base_destructor(Eo *obj, Ector_Renderer_Shape_Data *pd)
{
}

#include "ector_renderer_shape.eo.c"
