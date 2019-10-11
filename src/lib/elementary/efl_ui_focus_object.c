#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FOCUS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_FOCUS_OBJECT_MIXIN

typedef struct {
  Eina_Bool old_focus;
  Eina_Bool ongoing_prepare_call;
  Eina_Bool child_focus;
  Eina_Bool focus_geom_changed;
} Efl_Ui_Focus_Object_Data;

EOLIAN static void
_efl_ui_focus_object_focus_set(Eo *obj, Efl_Ui_Focus_Object_Data *pd, Eina_Bool focus)
{
   Efl_Ui_Focus_Object *parent;

   if (pd->old_focus == focus) return;

   pd->old_focus = focus;
   parent = efl_ui_focus_object_focus_parent_get(obj);
   if (parent)
     efl_ui_focus_object_child_focus_set(parent, focus);
   efl_event_callback_call(obj, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED , &focus);
}

EOLIAN static Eina_Bool
_efl_ui_focus_object_focus_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Object_Data *pd)
{
   return pd->old_focus;
}

EOLIAN static void
_efl_ui_focus_object_setup_order(Eo *obj, Efl_Ui_Focus_Object_Data *pd)
{
  if (pd->ongoing_prepare_call) return;

  pd->ongoing_prepare_call = EINA_TRUE;

  efl_ui_focus_object_setup_order_non_recursive(obj);

  pd->ongoing_prepare_call = EINA_FALSE;
}

EOLIAN static void
_efl_ui_focus_object_child_focus_set(Eo *obj, Efl_Ui_Focus_Object_Data *pd, Eina_Bool child_focus)
{
   Efl_Ui_Focus_Object *parent;

   if (child_focus == pd->child_focus) return;

   pd->child_focus = child_focus;
   parent = efl_ui_focus_object_focus_parent_get(obj);
   if (parent)
     efl_ui_focus_object_child_focus_set(parent, pd->child_focus);
}

EOLIAN static Eina_Bool
_efl_ui_focus_object_child_focus_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Object_Data *pd)
{
   return pd->child_focus;
}


EOLIAN static Eina_Bool
_efl_ui_focus_object_efl_object_event_callback_priority_add(Eo *obj, Efl_Ui_Focus_Object_Data *pd,
                                        const Efl_Event_Description *desc,
                                        Efl_Callback_Priority priority,
                                        Efl_Event_Cb func,
                                        const void *user_data)
{
  if (desc == EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_GEOMETRY_CHANGED)
    {
       pd->focus_geom_changed = EINA_TRUE;
    }

  return efl_event_callback_priority_add(efl_super(obj, MY_CLASS), desc, priority, func, user_data);
}

EOLIAN static Eina_Bool
_efl_ui_focus_object_efl_object_event_callback_array_priority_add(Eo *obj, Efl_Ui_Focus_Object_Data *pd,
                                              const Efl_Callback_Array_Item *array,
                                              Efl_Callback_Priority priority,
                                              const void *user_data)
{
   for (int i = 0; array[i].desc; ++i)
     {
        if (array[i].desc == EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_GEOMETRY_CHANGED)
          {
             pd->focus_geom_changed = EINA_TRUE;
          }
     }
   return efl_event_callback_array_priority_add(efl_super(obj, MY_CLASS), array, priority, user_data);
}


EOLIAN static Eina_Bool
_efl_ui_focus_object_efl_object_event_callback_call(Eo *obj, Efl_Ui_Focus_Object_Data *pd,
            const Efl_Event_Description *desc,
            void *event_info)
{
   if (desc == EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_GEOMETRY_CHANGED && !pd->focus_geom_changed)
     return EINA_TRUE;
   return efl_event_callback_call(efl_super(obj, MY_CLASS), desc, event_info);
}

#define EFL_UI_FOCUS_OBJECT_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_event_callback_priority_add, _efl_ui_focus_object_efl_object_event_callback_priority_add), \
   EFL_OBJECT_OP_FUNC(efl_event_callback_array_priority_add, _efl_ui_focus_object_efl_object_event_callback_array_priority_add), \
   EFL_OBJECT_OP_FUNC(efl_event_callback_call, _efl_ui_focus_object_efl_object_event_callback_call) \

#include "efl_ui_focus_object.eo.c"
