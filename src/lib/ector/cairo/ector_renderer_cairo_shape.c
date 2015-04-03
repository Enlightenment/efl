#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <cairo/Ector_Cairo.h>

#include "ector_private.h"
#include "ector_cairo_private.h"

typedef struct _Ector_Renderer_Cairo_Shape_Data Ector_Renderer_Cairo_Shape_Data;
struct _Ector_Renderer_Cairo_Shape_Data
{
};

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd, Ector_Surface *s)
{
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_generic_base_draw(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd, Ector_Surface *s, Ector_Rop op, Eina_Array *clips, int x, int y)
{
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_generic_base_done(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
}

static Eina_Bool
_ector_renderer_cairo_shape_ector_renderer_cairo_base_fill(Eo *obj, Ector_Renderer_Cairo_Shape_Data *pd)
{
}

#include "ector_renderer_cairo_shape.eo.c"
