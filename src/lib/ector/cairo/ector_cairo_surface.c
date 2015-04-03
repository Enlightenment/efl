#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include <Ector.h>
#include <cairo/Ector_Cairo.h>

#include "ector_private.h"

typedef struct _Ector_Cairo_Surface_Data Ector_Cairo_Surface_Data;
struct _Ector_Cairo_Surface_Data
{
};

void *
_ector_cairo_surface_symbol_get(Eo *obj, Ector_Cairo_Surface_Data *pd, char *name)
{
}

Ector_Renderer *
_ector_cairo_surface_ector_generic_surface_renderer_factory_new(Eo *obj, Ector_Cairo_Surface_Data *pd, const Eo_Class *type)
{
}

Eina_Bool
_ector_cairo_surface_context_set(Eo *obj, Ector_Cairo_Surface_Data *pd, cairo_t *ctx)
{
}

Eo *
_ector_cairo_surface_eo_base_finalize(Eo *obj, Ector_Cairo_Surface_Data *pd)
{
}

#include "ector_cairo_surface.eo.c"
