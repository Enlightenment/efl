#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

#define MY_CLASS ECTOR_SOFTWARE_SURFACE_CLASS

static Ector_Renderer *
_ector_software_surface_ector_surface_renderer_factory_new(Eo *obj,
                                                                   Ector_Software_Surface_Data *pd EINA_UNUSED,
                                                                   const Efl_Class *type)
{
   if (type == ECTOR_RENDERER_SHAPE_MIXIN)
     return eo_add(ECTOR_RENDERER_SOFTWARE_SHAPE_CLASS, NULL, ector_renderer_surface_set(eo_self, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_LINEAR_MIXIN)
     return eo_add(ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS, NULL, ector_renderer_surface_set(eo_self, obj));
   else if (type == ECTOR_RENDERER_GRADIENT_RADIAL_MIXIN)
     return eo_add(ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS, NULL, ector_renderer_surface_set(eo_self, obj));
   else if (type == ECTOR_RENDERER_BUFFER_MIXIN)
     return eo_add(ECTOR_RENDERER_SOFTWARE_BUFFER_CLASS, NULL, ector_renderer_surface_set(eo_self, obj));
   ERR("Couldn't find class for type: %s\n", eo_class_name_get(type));
   return NULL;
}

static Eo *
_ector_software_surface_efl_object_constructor(Eo *obj, Ector_Software_Surface_Data *pd)
{
   obj = efl_constructor(eo_super(obj, MY_CLASS));
   pd->rasterizer = (Software_Rasterizer *) calloc(1, sizeof(Software_Rasterizer));
   ector_software_rasterizer_init(pd->rasterizer);
   pd->rasterizer->fill_data.raster_buffer = eo_data_ref(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN);
   return obj;
}

static void
_ector_software_surface_efl_object_destructor(Eo *obj, Ector_Software_Surface_Data *pd)
{
   ector_software_rasterizer_done(pd->rasterizer);
   eo_data_unref(obj, pd->rasterizer->fill_data.raster_buffer);
   free(pd->rasterizer);
   pd->rasterizer = NULL;
   efl_destructor(eo_super(obj, ECTOR_SOFTWARE_SURFACE_CLASS));
}

static void
_ector_software_surface_ector_surface_reference_point_set(Eo *obj EINA_UNUSED,
                                                                  Ector_Software_Surface_Data *pd,
                                                                  int x, int y)
{
   pd->x = x;
   pd->y = y;
}

#include "ector_software_surface.eo.c"
#include "ector_renderer_software.eo.c"
