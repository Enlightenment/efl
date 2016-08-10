#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_CANVAS3D_OBJECT_CLASS

EOLIAN static Eo *
_evas_canvas3d_object_efl_object_constructor(Eo *obj, Evas_Canvas3D_Object_Data *pd)
{
   Eo *e = NULL;
   obj = efl_constructor(eo_super(obj, MY_CLASS));
   e = efl_parent_get(obj);
   pd->evas = e;
   pd->type = EVAS_CANVAS3D_OBJECT_TYPE_INVALID;
   memset(&pd->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_CANVAS3D_STATE_MAX);

   return obj;
}

EOLIAN static Evas *
 _evas_canvas3d_object_efl_object_provider_find(Eo *obj, Evas_Canvas3D_Object_Data *pd, const Efl_Class *klass)
{
   if (klass == EVAS_CANVAS_CLASS)
     return pd->evas;
   return efl_provider_find(eo_super(obj, MY_CLASS), klass);
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

   evas_canvas3d_object_change_notify(obj, state, ref);
}

EOLIAN static void
_evas_canvas3d_object_update(Eo *obj, Evas_Canvas3D_Object_Data *pd)
{
   if (!pd->dirty[EVAS_CANVAS3D_STATE_ANY])
     return;

   evas_canvas3d_object_update_notify(obj);

   memset(&pd->dirty[0], 0x00, sizeof(Eina_Bool) * EVAS_CANVAS3D_STATE_MAX);
}

EOLIAN static Eina_Bool
_evas_canvas3d_object_efl_object_event_callback_priority_add(Eo *obj,
                                                    Evas_Canvas3D_Object_Data *pd EINA_UNUSED,
                                                    const Efl_Event_Description *desc,
                                                    Efl_Callback_Priority priority,
                                                    Efl_Event_Cb func,
                                                    const void *user_data)
{
   Eina_Bool r = EINA_FALSE;

   r = efl_event_callback_priority_add(eo_super(obj, MY_CLASS), desc, priority, func, user_data);
   evas_canvas3d_object_callback_register(obj, desc->name, user_data);

   return r;
}

EOLIAN static Eina_Bool
_evas_canvas3d_object_efl_object_event_callback_del(Eo *obj, Evas_Canvas3D_Object_Data *pd EINA_UNUSED,
                                           const Efl_Event_Description *desc,
                                           Efl_Event_Cb func,
                                           const void *user_data)
{
   Eina_Bool r = EINA_FALSE;
   r = efl_event_callback_del(eo_super(obj, MY_CLASS), desc, func, user_data);
   if (r) evas_canvas3d_object_callback_unregister(obj, desc->name);
   return r;
}

#include "canvas/evas_canvas3d_object.eo.c"
