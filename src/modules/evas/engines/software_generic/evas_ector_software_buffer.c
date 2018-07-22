#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <software/Ector_Software.h>
#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_ector_software.h"

#define MY_CLASS EVAS_ECTOR_SOFTWARE_BUFFER_CLASS

typedef struct {
   Ector_Software_Buffer_Base_Data *base;
   RGBA_Image *image;
} Evas_Ector_Software_Buffer_Data;

// Note: Don't use ENFN, ENDT here because the GL engine may also use SW buffers
// eg. in the filters.

EOLIAN static void
_evas_ector_software_buffer_evas_ector_buffer_engine_image_set(Eo *obj,
                                                               Evas_Ector_Software_Buffer_Data *pd,
                                                               void *engine EINA_UNUSED,
                                                               void *image)
{
   RGBA_Image *im = image;

   EINA_SAFETY_ON_NULL_RETURN(image);
   EINA_SAFETY_ON_FALSE_RETURN(!efl_finalized_get(obj));
   EINA_SAFETY_ON_NULL_RETURN(im->image.data);

   evas_cache_image_ref(&im->cache_entry);
   pd->image = im;

   ector_buffer_pixels_set(obj, im->image.data, im->cache_entry.w, im->cache_entry.h, 0, im->cache_entry.space, EINA_TRUE);
}

EOLIAN static void *
_evas_ector_software_buffer_evas_ector_buffer_drawable_image_get(Eo *obj EINA_UNUSED,
                                                                 Evas_Ector_Software_Buffer_Data *pd)
{
   evas_cache_image_ref(&pd->image->cache_entry);
   return pd->image;
}

EOLIAN static void *
_evas_ector_software_buffer_evas_ector_buffer_render_image_get(Eo *obj EINA_UNUSED,
                                                               Evas_Ector_Software_Buffer_Data *pd)
{
   evas_cache_image_ref(&pd->image->cache_entry);
   return pd->image;
}

EOLIAN static Eina_Bool
_evas_ector_software_buffer_evas_ector_buffer_engine_image_release(Eo *obj EINA_UNUSED,
                                                                   Evas_Ector_Software_Buffer_Data *pd,
                                                                   void *image)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->image == image, EINA_FALSE);

   evas_cache_image_drop(&pd->image->cache_entry);
   return EINA_TRUE;
}

EOLIAN static Eo *
_evas_ector_software_buffer_efl_object_constructor(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->base = efl_data_xref(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN, obj);
   return obj;
}

EOLIAN static Eo *
_evas_ector_software_buffer_efl_object_finalize(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->base, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->image, NULL);
   pd->base->generic->immutable = EINA_TRUE;
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_evas_ector_software_buffer_efl_object_destructor(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   efl_data_xunref(obj, pd->base, obj);
   evas_cache_image_drop(&pd->image->cache_entry);
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "evas_ector_buffer.eo.c"
#include "evas_ector_software_buffer.eo.c"
