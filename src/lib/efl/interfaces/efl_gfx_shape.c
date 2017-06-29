#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>
#include <float.h>
#include <ctype.h>

#include <Efl.h>

typedef struct _Efl_Gfx_Shape_Data
{
   Efl_Gfx_Shape_Public public;
   Efl_Gfx_Fill_Rule fill_rule;
} Efl_Gfx_Shape_Data;

static inline double
interpolate(double from, double to, double pos_map)
{
   return (from * (1.0 - pos_map)) + (to * pos_map);
}

static inline int
interpolatei(int from, int to, double pos_map)
{
   return (from * (1.0 - pos_map)) + (to * pos_map);
}

typedef struct _Efl_Gfx_Property Efl_Gfx_Property;
struct _Efl_Gfx_Property
{
   double scale;
   double w;
   double centered;

   Efl_Gfx_Cap c;
   Efl_Gfx_Join j;

   const Efl_Gfx_Dash *dash;
   unsigned int dash_length;

   int r, g, b, a;
   int fr, fg, fb, fa;
};

static inline void
_efl_gfx_property_get(const Eo *obj, Efl_Gfx_Property *property)
{
   property->scale = efl_gfx_shape_stroke_scale_get(obj);
   efl_gfx_shape_stroke_color_get(obj, &property->r, &property->g,
                                  &property->b, &property->a);
   efl_gfx_color_get(obj, &property->fr, &property->fg,
                     &property->fb, &property->fa);
   property->w = efl_gfx_shape_stroke_width_get(obj);
   property->centered = efl_gfx_shape_stroke_location_get(obj);
   efl_gfx_shape_stroke_dash_get(obj, &property->dash, &property->dash_length);
   property->c = efl_gfx_shape_stroke_cap_get(obj);
   property->j = efl_gfx_shape_stroke_join_get(obj);
}

static Eina_Bool
_efl_gfx_shape_interpolate(Eo *obj, Efl_Gfx_Shape_Data *pd,
                           const Eo *from, const Eo *to, double pos_map)
{
   Efl_Gfx_Shape_Data *from_pd, *to_pd;
   Efl_Gfx_Property property_from, property_to;
   Efl_Gfx_Dash *dash = NULL;
   unsigned int i;

   from_pd = efl_data_scope_get(from, EFL_GFX_SHAPE_MIXIN);
   to_pd = efl_data_scope_get(to, EFL_GFX_SHAPE_MIXIN);
   if (!efl_isa(from, EFL_GFX_SHAPE_MIXIN) || !efl_isa(to, EFL_GFX_SHAPE_MIXIN))
     return EINA_FALSE;
   if ((pd == from_pd) || (pd == to_pd)) return EINA_FALSE;

   _efl_gfx_property_get(from, &property_from);
   _efl_gfx_property_get(to, &property_to);

   if (property_from.dash_length != property_to.dash_length) return EINA_FALSE;

   if (property_to.dash_length)
     {
        dash = malloc(sizeof (Efl_Gfx_Dash) * property_to.dash_length);
        if (!dash) return EINA_FALSE;

        for (i = 0; i < property_to.dash_length; i++)
          {
             dash[i].length = interpolate(property_from.dash[i].length,
                                          property_to.dash[i].length, pos_map);
             dash[i].gap = interpolate(property_from.dash[i].gap,
                                       property_to.dash[i].gap, pos_map);
          }
     }

   efl_gfx_shape_stroke_scale_set(obj, interpolate(property_from.scale,
                                                   property_to.scale, pos_map));
   efl_gfx_shape_stroke_color_set(obj,
                                  interpolatei(property_from.r,
                                               property_to.r, pos_map),
                                  interpolatei(property_from.g,
                                               property_to.g, pos_map),
                                  interpolatei(property_from.b,
                                               property_to.b, pos_map),
                                  interpolatei(property_from.a,
                                               property_to.a, pos_map));
   efl_gfx_color_set(obj,
                     interpolatei(property_from.fr, property_to.fr, pos_map),
                     interpolatei(property_from.fg, property_to.fg, pos_map),
                     interpolatei(property_from.fb, property_to.fb, pos_map),
                     interpolatei(property_from.fa, property_to.fa, pos_map));

   efl_gfx_shape_stroke_width_set(obj, interpolate(property_from.w,
                                                   property_to.w, pos_map));
   efl_gfx_shape_stroke_location_set(obj, interpolate(property_from.centered,
                                                      property_to.centered,
                                                      pos_map));
   efl_gfx_shape_stroke_dash_set(obj, dash, property_to.dash_length);
   efl_gfx_shape_stroke_cap_set(obj, (pos_map < 0.5) ?
                                property_from.c : property_to.c);
   efl_gfx_shape_stroke_join_set(obj, (pos_map < 0.5) ?
                                 property_from.j : property_to.j);

   return efl_gfx_path_interpolate(obj, from, to, pos_map);
}

static void
_efl_gfx_shape_stroke_scale_set(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                                double s)
{
   pd->public.stroke.scale = s;
}

static double
_efl_gfx_shape_stroke_scale_get(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd)
{
   return pd->public.stroke.scale;
}

static void
_efl_gfx_shape_stroke_color_set(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                                int r, int g, int b, int a)
{
   pd->public.stroke.color.r = r;
   pd->public.stroke.color.g = g;
   pd->public.stroke.color.b = b;
   pd->public.stroke.color.a = a;
}

static void
_efl_gfx_shape_stroke_color_get(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                                int *r, int *g, int *b, int *a)
{
   if (r) *r = pd->public.stroke.color.r;
   if (g) *g = pd->public.stroke.color.g;
   if (b) *b = pd->public.stroke.color.b;
   if (a) *a = pd->public.stroke.color.a;
}

static void
_efl_gfx_shape_stroke_width_set(Eo *obj, Efl_Gfx_Shape_Data *pd, double w)
{
   pd->public.stroke.width = w;
   efl_event_callback_legacy_call(obj, EFL_GFX_PATH_EVENT_CHANGED, NULL);
   efl_event_callback_legacy_call(obj, EFL_GFX_EVENT_CHANGED, NULL);
}

static double
_efl_gfx_shape_stroke_width_get(Eo *obj EINA_UNUSED,
                                Efl_Gfx_Shape_Data *pd)
{
   return pd->public.stroke.width;
}

static void
_efl_gfx_shape_stroke_location_set(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                                   double centered)
{
   pd->public.stroke.centered = centered;
}

static double
_efl_gfx_shape_stroke_location_get(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd)
{
   return pd->public.stroke.centered;
}

static void
_efl_gfx_shape_stroke_dash_set(Eo *obj EINA_UNUSED, Efl_Gfx_Shape_Data *pd,
                               const Efl_Gfx_Dash *dash, unsigned int length)
{
   Efl_Gfx_Dash *tmp;

   if (!dash)
     {
        free(pd->public.stroke.dash);
        pd->public.stroke.dash = NULL;
        pd->public.stroke.dash_length = 0;
        return;
     }

   tmp = realloc(pd->public.stroke.dash, length * sizeof (Efl_Gfx_Dash));
   if (!tmp && length) return;
   memcpy(tmp, dash, length * sizeof (Efl_Gfx_Dash));

   pd->public.stroke.dash = tmp;
   pd->public.stroke.dash_length = length;
}

static void
_efl_gfx_shape_stroke_dash_get(Eo *obj EINA_UNUSED,
                               Efl_Gfx_Shape_Data *pd,
                               const Efl_Gfx_Dash **dash, unsigned int *length)
{
   if (dash) *dash = pd->public.stroke.dash;
   if (length) *length = pd->public.stroke.dash_length;
}

static void
_efl_gfx_shape_stroke_cap_set(Eo *obj EINA_UNUSED,
                              Efl_Gfx_Shape_Data *pd,
                              Efl_Gfx_Cap c)
{
   pd->public.stroke.cap = c;
}

static Efl_Gfx_Cap
_efl_gfx_shape_stroke_cap_get(Eo *obj EINA_UNUSED,
                              Efl_Gfx_Shape_Data *pd)
{
   return pd->public.stroke.cap;
}

static void
_efl_gfx_shape_stroke_join_set(Eo *obj EINA_UNUSED,
                               Efl_Gfx_Shape_Data *pd,
                               Efl_Gfx_Join j)
{
   pd->public.stroke.join = j;
}

static Efl_Gfx_Join
_efl_gfx_shape_stroke_join_get(Eo *obj EINA_UNUSED,
                               Efl_Gfx_Shape_Data *pd)
{
   return pd->public.stroke.join;
}

static void
_efl_gfx_shape_fill_rule_set(Eo *obj EINA_UNUSED,
                             Efl_Gfx_Shape_Data *pd,
                             Efl_Gfx_Fill_Rule fill_rule)
{
   pd->fill_rule = fill_rule;
}

static Efl_Gfx_Fill_Rule
_efl_gfx_shape_fill_rule_get(Eo *obj EINA_UNUSED,
                             Efl_Gfx_Shape_Data *pd)
{
   return pd->fill_rule;
}

static void
_efl_gfx_shape_dup(Eo *obj, Efl_Gfx_Shape_Data *pd,
                                const Eo *dup_from)
{
   Efl_Gfx_Shape_Data *from;

   if (obj == dup_from) return;

   from = efl_data_scope_get(dup_from, EFL_GFX_SHAPE_MIXIN);
   if (!from) return;

   pd->public.stroke.scale = from->public.stroke.scale;
   pd->public.stroke.width = from->public.stroke.width;
   pd->public.stroke.centered = from->public.stroke.centered;
   pd->public.stroke.cap = from->public.stroke.cap;
   pd->public.stroke.join = from->public.stroke.join;
   pd->public.stroke.color.r = from->public.stroke.color.r;
   pd->public.stroke.color.g = from->public.stroke.color.g;
   pd->public.stroke.color.b = from->public.stroke.color.b;
   pd->public.stroke.color.a = from->public.stroke.color.a;
   pd->fill_rule = from->fill_rule;

   _efl_gfx_shape_stroke_dash_set(obj, pd, from->public.stroke.dash,
                                  from->public.stroke.dash_length);

   efl_gfx_path_dup(obj, dup_from);
}

#include "interfaces/efl_gfx_shape.eo.c"
