#include "efl_canvas_surface.h"

#define MY_CLASS EFL_CANVAS_SURFACE_MIXIN

EOLIAN static Eo *
_efl_canvas_surface_efl_object_constructor(Eo *eo, Efl_Canvas_Surface_Data *pd)
{
   eo = efl_constructor(efl_super(eo, MY_CLASS));
   pd->surf.version = EVAS_NATIVE_SURFACE_VERSION;
   return eo;
}

EOLIAN static void *
_efl_canvas_surface_native_buffer_get(const Eo *obj EINA_UNUSED, Efl_Canvas_Surface_Data *pd)
{
   return pd->buffer;
}

#include "efl_canvas_surface.eo.c"
