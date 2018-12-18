#include "efl_canvas_surface.h"

#define MY_CLASS EFL_CANVAS_SURFACE_TBM_CLASS

typedef struct _Efl_Canvas_Surface_Tbm_Data
{
} Efl_Canvas_Surface_Tbm_Data;

EOLIAN static Eo *
_efl_canvas_surface_tbm_efl_object_constructor(Eo *eo, Efl_Canvas_Surface_Tbm_Data *pd EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj;
   Efl_Canvas_Surface_Data *sd;

   eo = efl_constructor(efl_super(eo, MY_CLASS));
   obj = efl_data_scope_get(eo, EFL_CANVAS_OBJECT_CLASS);
   if (!obj) return NULL;

   if (!ENFN->image_native_init(ENC, EVAS_NATIVE_SURFACE_TBM))
     {
        ERR("TBM is not supported on this platform");
        return NULL;
     }

   sd = efl_data_scope_get(eo, EFL_CANVAS_SURFACE_CLASS);
   sd->surf.type = EVAS_NATIVE_SURFACE_TBM;
   return eo;
}

EOLIAN static void
_efl_canvas_surface_tbm_efl_object_destructor(Eo *eo, Efl_Canvas_Surface_Tbm_Data *pd EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj;

   obj = efl_data_scope_get(eo, EFL_CANVAS_OBJECT_CLASS);

   ENFN->image_native_shutdown(ENC, EVAS_NATIVE_SURFACE_TBM);
   efl_destructor(eo);
}

EOLIAN static Eina_Bool
_efl_canvas_surface_tbm_efl_canvas_surface_native_buffer_set(Eo *eo, Efl_Canvas_Surface_Tbm_Data *pd EINA_UNUSED, void *buffer)
{
   Efl_Canvas_Surface_Data *sd = efl_data_scope_get(eo, EFL_CANVAS_SURFACE_CLASS);

   sd->surf.data.tbm.buffer = buffer;
   if (!_evas_image_native_surface_set(eo, &sd->surf))
     {
        ERR("failed to set native buffer");
        sd->buffer = NULL;
        return EINA_FALSE;
     }
   sd->buffer = buffer;
   return EINA_TRUE;
}

#include "efl_canvas_surface_tbm.eo.c"
