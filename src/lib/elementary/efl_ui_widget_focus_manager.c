#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED

#include <Elementary.h>
#include <Elementary_Cursor.h>

#include "elm_priv.h"

#define MY_CLASS EFL_UI_WIDGET_FOCUS_MANAGER_MIXIN

typedef struct
{
   Efl_Ui_Focus_Manager *manager;
} Efl_Ui_Widget_Focus_Manager_Data;

EOLIAN static Efl_Object*
_efl_ui_widget_focus_manager_efl_object_constructor(Eo *obj, Efl_Ui_Widget_Focus_Manager_Data *pd)
{
   Eo *res;

   pd->manager = efl_ui_widget_focus_manager_create(obj, obj);

   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->manager, res);

   efl_ref(pd->manager);
   efl_composite_attach(obj, pd->manager);
   _efl_ui_focus_manager_redirect_events_add(pd->manager, obj);
   res = efl_constructor(efl_super(obj, MY_CLASS));


   return res;
}

EOLIAN static void
_efl_ui_widget_focus_manager_efl_object_destructor(Eo *obj, Efl_Ui_Widget_Focus_Manager_Data *pd)
{
   efl_destructor(efl_super(obj, MY_CLASS));

   _efl_ui_focus_manager_redirect_events_del(pd->manager, obj);
   efl_unref(pd->manager);
   pd->manager = NULL;
}

EOLIAN static Eina_Bool
_efl_ui_widget_focus_manager_efl_ui_widget_focus_state_apply(Eo *obj, Efl_Ui_Widget_Focus_Manager_Data *pd EINA_UNUSED, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect)
{
   Eina_Bool state;

   state = efl_ui_widget_focus_state_apply(efl_super(obj, MY_CLASS), current_state, configured_state, redirect);

   if (!state && !configured_state->manager && current_state.manager)
     efl_ui_focus_manager_reset_history(obj);

   return state;
}

#include "efl_ui_widget_focus_manager.eo.c"
