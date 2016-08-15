#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_common_private.h"
#include "evas_gl_private.h"

#include <software/Ector_Software.h>
#include <gl/Ector_GL.h>
#include "Evas_Engine_GL_Generic.h"

#include "evas_ector_buffer.eo.h"
#include "evas_ector_gl_rgbaimage_buffer.eo.h"
#include "../software_generic/evas_ector_software_buffer.eo.h"

#define MY_CLASS EVAS_ECTOR_GL_RGBAIMAGE_BUFFER_CLASS

typedef struct {
   Ector_Software_Buffer_Base_Data *base;
   Evas *evas;
   RGBA_Image *image;
   Evas_GL_Image *glim;
} Evas_Ector_GL_RGBAImage_Buffer_Data;

// GL engine stuff, do not use with RGBA_Image / Image_Entry
#define ENFN e->engine.func
#define ENDT e->engine.data.output

EOLIAN static void
_evas_ector_gl_rgbaimage_buffer_evas_ector_buffer_engine_image_set(Eo *obj, Evas_Ector_GL_RGBAImage_Buffer_Data *pd,
                                                                   Evas *evas, void *image)
{
   RGBA_Image *im = image;

   EINA_SAFETY_ON_NULL_RETURN(image);
   if (efl_finalized_get(obj))
     {
        CRI("engine_image must be set at construction time only");
        return;
     }

   if (!im->image.data)
     {
        CRI("image has no pixels yet");
        return;
     }

   pd->evas = efl_xref(evas, obj);
   evas_cache_image_ref(&im->cache_entry);
   pd->image = im;

   ector_buffer_pixels_set(obj, im->image.data, im->cache_entry.w, im->cache_entry.h, 0, (Efl_Gfx_Colorspace) im->cache_entry.space, EINA_TRUE, 0, 0, 0, 0);
}

EOLIAN static void
_evas_ector_gl_rgbaimage_buffer_evas_ector_buffer_engine_image_get(Eo *obj EINA_UNUSED,
                                                                   Evas_Ector_GL_RGBAImage_Buffer_Data *pd,
                                                                   Evas **evas, void **image)
{
   Evas_Public_Data *e = efl_data_scope_get(pd->evas, EVAS_CANVAS_CLASS);
   Render_Engine_GL_Generic *re = e->engine.data.output;
   int err = EVAS_LOAD_ERROR_NONE;
   Evas_Engine_GL_Context *gc;

   if (evas) *evas = pd->evas;
   if (image) *image = NULL;
   if (pd->glim)
     goto end;

   gc = re->window_gl_context_get(re->software.ob);
#ifdef EVAS_CSERVE2
   if (evas_cache2_image_cached(&pd->image->cache_entry))
     evas_cache2_image_ref(&pd->image->cache_entry);
   else
#endif
   evas_cache_image_ref(&pd->image->cache_entry);
   pd->glim = evas_gl_common_image_new_from_rgbaimage(gc, pd->image, NULL, &err);
   if ((err != EVAS_LOAD_ERROR_NONE) || !pd->glim)
     {
        ERR("Failed to create GL image! error %d", err);
        return;
     }

end:
   if (image) *image = pd->glim;
}

EOLIAN static Eo *
_evas_ector_gl_rgbaimage_buffer_efl_object_constructor(Eo *obj, Evas_Ector_GL_RGBAImage_Buffer_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->base = efl_data_xref(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN, obj);
   return obj;
}

EOLIAN static Eo *
_evas_ector_gl_rgbaimage_buffer_efl_object_finalize(Eo *obj, Evas_Ector_GL_RGBAImage_Buffer_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->base, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->image, NULL);
   pd->base->generic->immutable = EINA_TRUE;
   return efl_finalize(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_evas_ector_gl_rgbaimage_buffer_efl_object_destructor(Eo *obj, Evas_Ector_GL_RGBAImage_Buffer_Data *pd)
{
   Evas_Public_Data *e = efl_data_scope_get(pd->evas, EVAS_CANVAS_CLASS);

   efl_data_xunref(obj, pd->base, obj);
   ENFN->image_free(ENDT, pd->glim);
   evas_cache_image_drop(&pd->image->cache_entry);
   efl_xunref(pd->evas, obj);
   efl_destructor(efl_super(obj, MY_CLASS));
}

#include "evas_ector_gl_rgbaimage_buffer.eo.c"
