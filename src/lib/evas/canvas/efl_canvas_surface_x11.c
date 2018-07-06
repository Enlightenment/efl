#include "efl_canvas_surface.h"

#define MY_CLASS EFL_CANVAS_SURFACE_X11_CLASS

typedef struct _Efl_Canvas_Surface_X11_Data
{
   Efl_Canvas_Surface_Data *base;
   Efl_Canvas_Surface_X11_Pixmap px;
} Efl_Canvas_Surface_X11_Data;

EOLIAN static Eo *
_efl_canvas_surface_x11_efl_object_constructor(Eo *eo, Efl_Canvas_Surface_X11_Data *pd)
{
   Evas_Object_Protected_Data *obj;

   eo = efl_constructor(efl_super(eo, MY_CLASS));
   obj = efl_data_scope_get(eo, EFL_CANVAS_OBJECT_CLASS);
   if (!obj) return NULL;

   if (!ENFN->image_native_init(ENC, EVAS_NATIVE_SURFACE_X11))
     {
        ERR("X11 is not supported on this platform");
        return NULL;
     }

   pd->base = efl_data_ref(eo, EFL_CANVAS_SURFACE_MIXIN);
   pd->base->surf.version = EVAS_NATIVE_SURFACE_VERSION;
   pd->base->surf.type = EVAS_NATIVE_SURFACE_X11;
   return eo;
}

EOLIAN static void
_efl_canvas_surface_x11_efl_object_destructor(Eo *eo, Efl_Canvas_Surface_X11_Data *pd)
{
   Evas_Object_Protected_Data *obj;

   obj = efl_data_scope_get(eo, EFL_CANVAS_OBJECT_CLASS);

   ENFN->image_native_shutdown(ENC, EVAS_NATIVE_SURFACE_X11);
   efl_data_unref(eo, pd->base);
   efl_destructor(eo);
}

EOLIAN static Eina_Bool
_efl_canvas_surface_x11_pixmap_set(Eo *eo, Efl_Canvas_Surface_X11_Data *pd EINA_UNUSED, void *visual, unsigned long pixmap)
{
   pd->px.pixmap = pixmap;
   pd->px.visual = visual;
   if (!_evas_image_native_surface_set(eo, &pd->base->surf))
     {
        ERR("failed to set native buffer");
        pd->base->buffer = NULL;
        return EINA_FALSE;
     }
   pd->base->buffer = &pd->px;
   return EINA_TRUE;
}

EOLIAN static void
_efl_canvas_surface_x11_pixmap_get(const Eo *eo EINA_UNUSED, Efl_Canvas_Surface_X11_Data *pd, void **visual, unsigned long *pixmap)
{
   if (pixmap) *pixmap = pd->px.pixmap;
   if (visual) *visual = pd->px.visual;
}

EOLIAN static Eina_Bool
_efl_canvas_surface_x11_efl_canvas_surface_native_buffer_set(Eo *eo, Efl_Canvas_Surface_X11_Data *pd, void *buffer)
{
   Efl_Canvas_Surface_X11_Pixmap *px = buffer;

   if (px)
     {
        pd->px.pixmap = px->pixmap;
        pd->px.visual = px->visual;
        pd->base->buffer = &pd->px;
     }
   else
     {
        pd->px.pixmap = 0L;
        pd->px.visual = NULL;
        pd->base->buffer = NULL;
     }
   return efl_canvas_surface_x11_pixmap_set(eo, pd->px.visual, pd->px.pixmap);
}

#include "efl_canvas_surface_x11.eo.c"
