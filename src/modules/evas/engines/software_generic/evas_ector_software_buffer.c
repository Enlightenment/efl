#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <software/Ector_Software.h>
#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_ector_buffer.eo.h"
#include "evas_ector_software_buffer.eo.h"

#define MY_CLASS EVAS_ECTOR_SOFTWARE_BUFFER_CLASS

typedef struct {
   Ector_Software_Buffer_Base_Data *base;
   Evas *evas;
   RGBA_Image *image;
} Evas_Ector_Software_Buffer_Data;

// Note: Don't use ENFN, ENDT here because the GL engine may also use SW buffers
// eg. in the filters.

EOLIAN static void
_evas_ector_software_buffer_evas_ector_buffer_engine_image_set(Eo *obj, Evas_Ector_Software_Buffer_Data *pd,
                                                               Evas *evas, void *image)
{
   RGBA_Image *im = image;

   EINA_SAFETY_ON_NULL_RETURN(image);
   if (eo_finalized_get(obj))
     {
        CRI("engine_image must be set at construction time only");
        return;
     }

   if (!im->image.data)
     {
        CRI("image has no pixels yet");
        return;
     }

   pd->evas = eo_xref(evas, obj);
   evas_cache_image_ref(&im->cache_entry);
   pd->image = im;

   ector_buffer_pixels_set(obj, im->image.data, im->cache_entry.w, im->cache_entry.h, 0, (Efl_Gfx_Colorspace) im->cache_entry.space, EINA_TRUE, 0, 0, 0, 0);
}

EOLIAN static void
_evas_ector_software_buffer_evas_ector_buffer_engine_image_get(Eo *obj EINA_UNUSED,
                                                               Evas_Ector_Software_Buffer_Data *pd,
                                                               Evas **evas, void **image)
{
   Evas_Public_Data *e = eo_data_scope_get(pd->evas, EVAS_CANVAS_CLASS);

   if (evas) *evas = pd->evas;
   if (e->engine.func->gl_surface_read_pixels)
     {
        ERR("Invalid: requesting engine_image from a GL image from a simple SW buffer!");
        if (image) *image = NULL;
        return;
     }

   if (image) *image = pd->image;
}

EOLIAN static Eo *
_evas_ector_software_buffer_eo_base_constructor(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   obj = eo_constructor(eo_super(obj, MY_CLASS));
   pd->base = eo_data_xref(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN, obj);
   return obj;
}

EOLIAN static Eo *
_evas_ector_software_buffer_eo_base_finalize(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->base, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->image, NULL);
   pd->base->generic->immutable = EINA_TRUE;
   return eo_finalize(eo_super(obj, MY_CLASS));
}

EOLIAN static void
_evas_ector_software_buffer_eo_base_destructor(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   eo_data_xunref(obj, pd->base, obj);
   evas_cache_image_drop(&pd->image->cache_entry);
   eo_xunref(pd->evas, obj);
   eo_destructor(eo_super(obj, MY_CLASS));
}

#include "evas_ector_buffer.eo.c"
#include "evas_ector_software_buffer.eo.c"
