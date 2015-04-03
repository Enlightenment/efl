#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>

#include "ector_private.h"

typedef struct _Ector_Renderer_Cairo_Gradient_Radial_Data Ector_Renderer_Cairo_Gradient_Radial_Data;
struct _Ector_Renderer_Cairo_Gradient_Radial_Data
{
};

static Eina_Bool
_ector_renderer_cairo_gradient_radial_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_Cairo_Gradient_Radial_Data *pd, Ector_Surface *s)
{
}

static Eina_Bool
_ector_renderer_cairo_gradient_radial_ector_renderer_generic_base_draw(Eo *obj, Ector_Renderer_Cairo_Gradient_Radial_Data *pd, Ector_Surface *s, Ector_Rop op, Eina_Array *clips, int x, int y)
{
}

static Eina_Bool
_ector_renderer_cairo_gradient_radial_ector_renderer_generic_base_done(Eo *obj, Ector_Renderer_Cairo_Gradient_Radial_Data *pd)
{
}

#include "ector_renderer_cairo_gradient_radial.eo.c"
