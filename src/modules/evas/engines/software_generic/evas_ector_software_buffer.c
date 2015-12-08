#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <software/Ector_Software.h>
#include "evas_common_private.h"
#include "evas_private.h"
#include "draw.h"

#include "evas_ector_buffer.eo.h"
#include "evas_ector_software_buffer.eo.h"

#define MY_CLASS EVAS_ECTOR_SOFTWARE_BUFFER_CLASS

typedef struct {
   Ector_Software_Buffer_Base_Data *base;
   Evas *evas;
   Image_Entry *ie;
} Evas_Ector_Software_Buffer_Data;

#define ENFN e->engine.func
#define ENDT e->engine.data.output

EOLIAN static void
_evas_ector_software_buffer_evas_ector_buffer_engine_image_set(Eo *obj, Evas_Ector_Software_Buffer_Data *pd,
                                                               Evas *evas, void *image)
{
   Evas_Public_Data *e = eo_data_scope_get(evas, EVAS_CANVAS_CLASS);
   Image_Entry *ie = image;
   RGBA_Image *im;
   Eina_Bool b;

   if (eo_do_ret(obj, b, eo_finalized_get()))
     {
        CRI("engine_image must be set at construction time only");
        return;
     }

   im = (RGBA_Image *) ie;
   if (!im->image.data)
     {
        CRI("image has no pixels yet");
        return;
     }

   pd->evas = eo_xref(evas, obj);
   pd->ie = ENFN->image_ref(ENDT, ie);
   if (!pd->ie) return;

   eo_do(obj, ector_buffer_pixels_set(im->image.data,
                                      im->cache_entry.w, im->cache_entry.h, 0,
                                      _evas_to_gfx_render_op(im->cache_entry.space),
                                      EINA_TRUE, 0, 0, 0, 0));
}

EOLIAN static void
_evas_ector_software_buffer_evas_ector_buffer_engine_image_get(Eo *obj EINA_UNUSED,
                                                               Evas_Ector_Software_Buffer_Data *pd,
                                                               Evas **evas, void **image)
{
   if (evas) *evas = pd->evas;
   if (image) *image = pd->ie;
}

EOLIAN static Eo *
_evas_ector_software_buffer_eo_base_constructor(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   pd->base = eo_data_xref(obj, ECTOR_SOFTWARE_BUFFER_BASE_MIXIN, obj);
   return obj;
}

EOLIAN static Eo *
_evas_ector_software_buffer_eo_base_finalize(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   if (!pd->ie)
     {
        CRI("engine_image must be set at construction time only");
        return NULL;
     }
   pd->base->generic->immutable = EINA_TRUE;
   return eo_do_super_ret(obj, MY_CLASS, obj, eo_finalize());
}

EOLIAN static void
_evas_ector_software_buffer_eo_base_destructor(Eo *obj, Evas_Ector_Software_Buffer_Data *pd)
{
   Evas_Public_Data *e = eo_data_scope_get(pd->evas, EVAS_CANVAS_CLASS);

   eo_data_xunref(obj, pd->base, obj);
   ENFN->image_free(ENDT, pd->ie);
   eo_xunref(pd->evas, obj);
   eo_do_super(obj, MY_CLASS, eo_destructor());
}

#include "evas_ector_buffer.eo.c"
#include "evas_ector_software_buffer.eo.c"
