#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_focus_layer.eo.h"

#define MY_CLASS EFL_UI_FOCUS_LAYER_MIXIN

typedef struct {
   Efl_Ui_Focus_Manager *registered_manager;
   Eina_Bool registered;
   Efl_Ui_Focus_Manager *manager;
} Efl_Ui_Focus_Layer_Data;

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_focus_layer_elm_widget_focus_manager_create(Eo *obj, Efl_Ui_Focus_Layer_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   return efl_add(EFL_UI_FOCUS_MANAGER_ROOT_FOCUS_CLASS, obj, efl_ui_focus_manager_root_set(efl_added, root));
}

EOLIAN static void
_efl_ui_focus_layer_efl_gfx_visible_set(Eo *obj, Efl_Ui_Focus_Layer_Data *pd, Eina_Bool v)
{
   efl_gfx_visible_set(efl_super(obj, MY_CLASS), v);

   if (v && !pd->registered)
     {
        pd->registered_manager = elm_widget_top_get(obj);

        efl_ui_focus_manager_redirect_set(pd->registered_manager, obj);
        efl_ui_focus_manager_focus_set(pd->manager, obj);
        pd->registered = EINA_TRUE;
     }
   else if (!v && pd->registered)
     {
        efl_ui_focus_manager_redirect_set(pd->registered_manager, NULL);
        pd->registered = EINA_FALSE;
     }
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_layer_efl_ui_focus_manager_move(Eo *obj, Efl_Ui_Focus_Layer_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Eo *ret = efl_ui_focus_manager_move(pd->manager, direction);

   if (ret)
     return ret;

   if ((direction == EFL_UI_FOCUS_DIRECTION_PREV) || (direction == EFL_UI_FOCUS_DIRECTION_NEXT))
     efl_ui_focus_manager_focus_set(pd->manager, obj);

   return efl_ui_focus_manager_focus_get(obj);
}

EOLIAN static void
_efl_ui_focus_layer_efl_object_destructor(Eo *obj, Efl_Ui_Focus_Layer_Data *pd)
{
   if (pd->registered)
     {
        efl_ui_focus_manager_redirect_set(pd->registered_manager, NULL);
        pd->registered = EINA_FALSE;
     }
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_focus_layer_efl_ui_focus_user_manager_get(Eo *obj, Efl_Ui_Focus_Layer_Data *pd)
{
   return elm_widget_top_get(obj);
}

EOLIAN static Eina_Bool
_efl_ui_focus_layer_elm_widget_focus_state_apply(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Layer_Data *pd EINA_UNUSED, Elm_Widget_Focus_State current_state EINA_UNUSED, Elm_Widget_Focus_State *configured_state EINA_UNUSED, Elm_Widget *redirect EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Efl_Object*
_efl_ui_focus_layer_efl_object_constructor(Eo *obj, Efl_Ui_Focus_Layer_Data *pd)
{
   pd->manager = elm_obj_widget_focus_manager_create(obj, obj);

   efl_composite_attach(obj, pd->manager);

   return efl_constructor(efl_super(obj, MY_CLASS));
}

#include "efl_ui_focus_layer.eo.c"
