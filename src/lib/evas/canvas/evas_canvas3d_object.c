#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_CANVAS3D_OBJECT_CLASS

EOLIAN static Eo *
_evas_canvas3d_object_eo_base_constructor(Eo *obj, Evas_Canvas3D_Object_Data *pd)
{
   Eo *e = NULL;
   obj = eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
   eo_do(obj, e = eo_parent_get());
   pd->evas = e;
   pd->type = EVAS_CANVAS3D_OBJECT_TYPE_INVALID;
   memset(&pd->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_CANVAS3D_STATE_MAX);

   return obj;
}

EOLIAN static Evas *
 _evas_canvas3d_object_evas_common_interface_evas_get(Eo *obj EINA_UNUSED, Evas_Canvas3D_Object_Data *pd)
{
   return pd->evas;
}

EOLIAN static void
_evas_canvas3d_object_type_set(Eo *obj EINA_UNUSED, Evas_Canvas3D_Object_Data *pd, Evas_Canvas3D_Object_Type type)
{
   pd->type = type;
}

EOLIAN static Evas_Canvas3D_Object_Type
_evas_canvas3d_object_type_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Object_Data *pd)
{
   return pd->type;
}

EOLIAN static Eina_Bool
_evas_canvas3d_object_dirty_get(const Eo *obj EINA_UNUSED, Evas_Canvas3D_Object_Data *pd, Evas_Canvas3D_State state)
{
   return pd->dirty[state];
}

EOLIAN static void
_evas_canvas3d_object_change(Eo *obj, Evas_Canvas3D_Object_Data *pd, Evas_Canvas3D_State state, Evas_Canvas3D_Object *ref)
{
   /* Skip already dirty properties. */
   if (pd->dirty[state])
     return;

   pd->dirty[state] = EINA_TRUE;
   pd->dirty[EVAS_CANVAS3D_STATE_ANY] = EINA_TRUE;

   eo_do(obj, evas_canvas3d_object_change_notify(state, ref));
}

EOLIAN static void
_evas_canvas3d_object_update(Eo *obj, Evas_Canvas3D_Object_Data *pd)
{
   if (!pd->dirty[EVAS_CANVAS3D_STATE_ANY])
     return;

   eo_do(obj, evas_canvas3d_object_update_notify());

   memset(&pd->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_CANVAS3D_STATE_MAX);
}

EOLIAN static Eina_Bool
_evas_canvas3d_object_eo_base_event_callback_priority_add(Eo *obj,
                                                    Evas_Canvas3D_Object_Data *pd EINA_UNUSED,
                                                    const Eo_Event_Description *desc,
                                                    Eo_Callback_Priority priority,
                                                    Eo_Event_Cb func,
                                                    const void *user_data)
{
   Eina_Bool r = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, r = eo_event_callback_priority_add(desc, priority, func, user_data));
   eo_do(obj, evas_canvas3d_object_callback_register(desc->name, user_data));

   return r;
}

EOLIAN static Eina_Bool
_evas_canvas3d_object_eo_base_event_callback_del(Eo *obj, Evas_Canvas3D_Object_Data *pd EINA_UNUSED,
                                           const Eo_Event_Description *desc,
                                           Eo_Event_Cb func,
                                           const void *user_data)
{
   Eina_Bool r = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, r = eo_event_callback_del(desc, func, user_data));
   if (r) eo_do(obj, evas_canvas3d_object_callback_unregister(desc->name));
   return r;
}

#include "canvas/evas_canvas3d_object.eo.c"
