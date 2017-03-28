#include "evas_image_private.h"
#include "efl_canvas_snapshot.eo.h"

#define MY_CLASS EFL_CANVAS_SNAPSHOT_CLASS

EOLIAN static Eo *
_efl_canvas_snapshot_efl_object_constructor(Eo *eo_obj, void *pd EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj;

   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   if (!eo_obj) return NULL;

   efl_canvas_object_pass_events_set(eo_obj, EINA_TRUE);

   obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   EINA_COW_STATE_WRITE_BEGIN(obj, sw, cur)
     sw->snapshot = EINA_TRUE;
   EINA_COW_STATE_WRITE_END(obj, sw, cur);

   return eo_obj;
}

EOLIAN static void
_efl_canvas_snapshot_efl_gfx_buffer_buffer_update_add(Eo *eo_obj, void *_pd EINA_UNUSED,
                                                      int x EINA_UNUSED, int y EINA_UNUSED,
                                                      int w EINA_UNUSED, int h EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj;

   // FIXME: Prevent obscures in the region x,y,w,h
   obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

#include "efl_canvas_snapshot.eo.c"
