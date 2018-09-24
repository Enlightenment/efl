#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FOCUS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef struct {
  Eina_Bool old_focus;
  Eina_Bool ongoing_prepare_call;
  Eina_Bool child_focus;
} Efl_Ui_Focus_Object_Data;

EOLIAN static void
_efl_ui_focus_object_focus_set(Eo *obj, Efl_Ui_Focus_Object_Data *pd, Eina_Bool focus)
{
   if (pd->old_focus == focus) return;

   pd->old_focus = focus;
   efl_ui_focus_object_child_focus_set(efl_ui_focus_object_focus_parent_get(obj), focus);
   efl_event_callback_call(obj, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED , (void*) (uintptr_t) focus);
}

EOLIAN static Eina_Bool
_efl_ui_focus_object_focus_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Object_Data *pd)
{
   return pd->old_focus;
}

EOLIAN static void
_efl_ui_focus_object_prepare_logical(Eo *obj, Efl_Ui_Focus_Object_Data *pd)
{
  if (pd->ongoing_prepare_call) return;

  pd->ongoing_prepare_call = EINA_TRUE;

  efl_ui_focus_object_prepare_logical_none_recursive(obj);

  pd->ongoing_prepare_call = EINA_FALSE;
}

EOLIAN static void
_efl_ui_focus_object_child_focus_set(Eo *obj, Efl_Ui_Focus_Object_Data *pd, Eina_Bool child_focus)
{
   if (child_focus == pd->child_focus) return;

   pd->child_focus = child_focus;
   efl_ui_focus_object_child_focus_set(efl_ui_focus_object_focus_parent_get(obj), pd->child_focus);
}

EOLIAN static Eina_Bool
_efl_ui_focus_object_child_focus_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Object_Data *pd)
{
   return pd->child_focus;
}



#include "efl_ui_focus_object.eo.c"
