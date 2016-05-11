#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

static void
_ector_renderer_shape_fill_set(Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Shape_Data *pd,
                                       const Ector_Renderer *r)
{
   _eo_refplace(&pd->fill, r);
}

static const Ector_Renderer *
_ector_renderer_shape_fill_get(Eo *obj EINA_UNUSED,
                                       Ector_Renderer_Shape_Data *pd)
{
   return pd->fill;
}

static void
_ector_renderer_shape_stroke_fill_set(Eo *obj EINA_UNUSED,
                                              Ector_Renderer_Shape_Data *pd,
                                              const Ector_Renderer *r)
{
   _eo_refplace(&pd->stroke.fill, r);
}

static const Ector_Renderer *
_ector_renderer_shape_stroke_fill_get(Eo *obj EINA_UNUSED,
                                              Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.fill;
}

static void
_ector_renderer_shape_stroke_marker_set(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Shape_Data *pd,
                                                const Ector_Renderer *r)
{
   _eo_refplace(&pd->stroke.marker, r);
}

static const Ector_Renderer *
_ector_renderer_shape_stroke_marker_get(Eo *obj EINA_UNUSED,
                                                Ector_Renderer_Shape_Data *pd)
{
   return pd->stroke.marker;
}

#include "ector_renderer_shape.eo.c"
