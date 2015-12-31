#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define ECTOR_GL_BUFFER_BASE_PROTECTED

#include <gl/Ector_GL.h>
#include "gl/ector_gl_private.h"
#include "evas_common_private.h"
#include "../gl_common/evas_gl_common.h"
#include "evas_private.h"
#include "ector_buffer.h"

#include "evas_ector_buffer.eo.h"
#include "evas_ector_gl_buffer.eo.h"
#include "evas_ector_gl_image_buffer.eo.h"

#define MY_CLASS EVAS_ECTOR_GL_IMAGE_BUFFER_CLASS

typedef struct
{
   Ector_GL_Buffer_Base_Data *base;
   Evas *evas;
   Evas_GL_Image *image;
} Evas_Ector_GL_Image_Buffer_Data;

#define ENFN e->engine.func
#define ENDT e->engine.data.output

EOLIAN static void
_evas_ector_gl_image_buffer_evas_ector_buffer_engine_image_set(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd,
                                                               Evas *evas, void *image)
{
   Evas_Public_Data *e = eo_data_scope_get(evas, EVAS_CANVAS_CLASS);
   Evas_GL_Image *im = image;
   int l = 0, r = 0, t = 0, b = 0;

   if (pd->base->generic->immutable)
     {
        CRI("Can't set image after finalize");
        return;
     }

   pd->evas = eo_xref(evas, obj);
   EINA_SAFETY_ON_NULL_RETURN(im);

   if (im->tex && im->tex->pt)
     {
        if (im->im)
          {
             l = im->im->cache_entry.borders.l;
             r = im->im->cache_entry.borders.r;
             t = im->im->cache_entry.borders.t;
             b = im->im->cache_entry.borders.b;
          }
        else
          {
             // always 1 pixel border, except FBO
             if (!im->tex->pt->fb)
               l = r = t = b = 1;
          }

        pd->image = ENFN->image_ref(ENDT, im);
        eo_do(obj, ector_gl_buffer_base_attach(im->tex->pt->texture,
                                               im->tex->pt->fb,
                                               (Efl_Gfx_Colorspace) evas_gl_common_gl_format_to_colorspace(im->tex->pt->format),
                                               im->tex->w, im->tex->h,
                                               im->tex->x, im->tex->y,
                                               im->tex->pt->w, im->tex->pt->h,
                                               l, r, t, b));
     }
   else
     {
        // FIXME: This might be required to support texture upload here
        ERR("Image has no attached texture! Unsupported!");
        pd->image = NULL;
     }
}

EOLIAN static void
_evas_ector_gl_image_buffer_evas_ector_buffer_engine_image_get(Eo *obj EINA_UNUSED,
                                                               Evas_Ector_GL_Image_Buffer_Data *pd,
                                                               Evas **evas, void **image)
{
   if (evas) *evas = pd->evas;
   if (image) *image = pd->image;
}

EOLIAN static Eo_Base *
_evas_ector_gl_image_buffer_eo_base_constructor(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd)
{
   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   pd->base = eo_data_ref(obj, ECTOR_GL_BUFFER_BASE_MIXIN);
   pd->base->generic = eo_data_ref(obj, ECTOR_GENERIC_BUFFER_MIXIN);
   pd->base->generic->eo = obj;
   return obj;
}

EOLIAN static Eo_Base *
_evas_ector_gl_image_buffer_eo_base_finalize(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->base, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->image, NULL);
   pd->base->generic->immutable = EINA_TRUE;
   return eo_do_super_ret(obj, MY_CLASS, obj, eo_finalize());
}

EOLIAN static void
_evas_ector_gl_image_buffer_eo_base_destructor(Eo *obj, Evas_Ector_GL_Image_Buffer_Data *pd)
{
   Evas_Public_Data *e = eo_data_scope_get(pd->evas, EVAS_CANVAS_CLASS);

   eo_data_unref(obj, pd->base->generic);
   eo_data_unref(obj, pd->base);
   ENFN->image_free(ENDT, pd->image);
   eo_xunref(pd->evas, obj);
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "evas_ector_gl_image_buffer.eo.c"
