#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

static unsigned int _software_count = 0;

typedef struct _Ector_Renderer_Software_Base_Data Ector_Renderer_Software_Base_Data;
struct _Ector_Renderer_Software_Base_Data
{
};

static Ector_Renderer *
_ector_software_surface_ector_generic_surface_renderer_factory_new(Eo *obj,
                                                                   Ector_Software_Surface_Data *pd EINA_UNUSED,
                                                                   const Eo_Class *type)
{
   if (type == ECTOR_RENDERER_GENERIC_SHAPE_MIXIN)
     return eo_add(ECTOR_RENDERER_SOFTWARE_SHAPE_CLASS, obj);
   else if (type == ECTOR_RENDERER_GENERIC_GRADIENT_LINEAR_MIXIN)
     return eo_add(ECTOR_RENDERER_SOFTWARE_GRADIENT_LINEAR_CLASS, obj);
   else if (type == ECTOR_RENDERER_GENERIC_GRADIENT_RADIAL_MIXIN)
     return eo_add(ECTOR_RENDERER_SOFTWARE_GRADIENT_RADIAL_CLASS, obj);
   ERR("Couldn't find class for type: %s\n", eo_class_name_get(type));
   return NULL;
}

static void
_ector_software_surface_context_set(Eo *obj EINA_UNUSED,
                                    Ector_Software_Surface_Data *pd,
                                    Software_Rasterizer *ctx)
{
   pd->software = ctx;
}

static Software_Rasterizer *
_ector_software_surface_context_get(Eo *obj EINA_UNUSED,
                                    Ector_Software_Surface_Data *pd)
{
   return pd->software;
}

void
_ector_software_surface_surface_set(Eo *obj EINA_UNUSED,
                                    Ector_Software_Surface_Data *pd,
                                    void *pixels, unsigned int width, unsigned int height)
{
   pd->software->fillData.raster_buffer.buffer = pixels;
   pd->software->fillData.raster_buffer.width = width;
   pd->software->fillData.raster_buffer.height = height;
}

void
_ector_software_surface_surface_get(Eo *obj EINA_UNUSED,
                                    Ector_Software_Surface_Data *pd,
                                    void **pixels, unsigned int *width, unsigned int *height)
{
   *pixels = pd->software->fillData.raster_buffer.buffer;
   *width = pd->software->fillData.raster_buffer.width;
   *height = pd->software->fillData.raster_buffer.height;
}

static void
_ector_software_surface_eo_base_constructor(Eo *obj,
                                            Ector_Software_Surface_Data *pd EINA_UNUSED)
{
  eo_do_super(obj, ECTOR_SOFTWARE_SURFACE_CLASS, eo_constructor());
  if(_software_count == 0)
    {
       pd->software = (Software_Rasterizer *) calloc(1, sizeof(Software_Rasterizer));
       ector_software_rasterizer_init(pd->software);
    }
  _software_count++;
}

static void
_ector_software_surface_eo_base_destructor(Eo *obj EINA_UNUSED,
                                           Ector_Software_Surface_Data *pd EINA_UNUSED)
{
   --_software_count;
   if (_software_count > 0) return;
   ector_software_rasterizer_done(pd->software);
   free(pd->software);
   pd->software = NULL;
   eo_do_super(obj, ECTOR_SOFTWARE_SURFACE_CLASS, eo_destructor());
}

static void
_ector_software_surface_ector_generic_surface_reference_point_set(Eo *obj EINA_UNUSED,
                                                                  Ector_Software_Surface_Data *pd,
                                                                  int x, int y)
{
   pd->x = x;
   pd->y = y;
}

#include "ector_software_surface.eo.c"
#include "ector_renderer_software_base.eo.c"
