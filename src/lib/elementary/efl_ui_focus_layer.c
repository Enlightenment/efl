#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FOCUS_LAYER_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_FOCUS_LAYER_MIXIN

typedef struct {
   Efl_Ui_Focus_Manager *registered_manager;
   Efl_Ui_Focus_Manager *manager;
   Eina_Bool cycle;
   Eina_Bool enable_on_visible;
} Efl_Ui_Focus_Layer_Data;

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_focus_layer_efl_ui_widget_focus_manager_create(Eo *obj, Efl_Ui_Focus_Layer_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   return efl_add(EFL_UI_FOCUS_MANAGER_ROOT_FOCUS_CLASS, obj, efl_ui_focus_manager_root_set(efl_added, root));
}

EOLIAN static void
_efl_ui_focus_layer_efl_gfx_visible_set(Eo *obj, Efl_Ui_Focus_Layer_Data *pd, Eina_Bool v)
{
   efl_gfx_visible_set(efl_super(obj, MY_CLASS), v);

   if (pd->enable_on_visible)
     {
        efl_ui_focus_layer_enable_set(obj, v);
     }
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_layer_efl_ui_focus_manager_move(Eo *obj, Efl_Ui_Focus_Layer_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Eo *ret = efl_ui_focus_manager_move(pd->manager, direction);

   if (ret)
     return ret;

   //ret is NULL here, if we do not want to cycle return NULL, which will result in obj being unset
   if (!pd->cycle)
     return NULL;

   if ((direction == EFL_UI_FOCUS_DIRECTION_PREVIOUS ) || (direction == EFL_UI_FOCUS_DIRECTION_NEXT))
     efl_ui_focus_manager_focus_set(pd->manager, obj);

   return efl_ui_focus_manager_focus_get(obj);
}

EOLIAN static void
_efl_ui_focus_layer_efl_object_destructor(Eo *obj, Efl_Ui_Focus_Layer_Data *pd EINA_UNUSED)
{
   efl_ui_focus_layer_enable_set(obj, EINA_FALSE);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_focus_layer_efl_ui_focus_object_focus_manager_get(Eo *obj, Efl_Ui_Focus_Layer_Data *pd EINA_UNUSED)
{
   return elm_widget_top_get(obj);
}

EOLIAN static Eina_Bool
_efl_ui_focus_layer_efl_ui_widget_focus_state_apply(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Layer_Data *pd EINA_UNUSED, Efl_Ui_Widget_Focus_State current_state EINA_UNUSED, Efl_Ui_Widget_Focus_State *configured_state EINA_UNUSED, Efl_Ui_Widget *redirect EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Efl_Object*
_efl_ui_focus_layer_efl_object_constructor(Eo *obj, Efl_Ui_Focus_Layer_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   pd->manager = efl_ui_widget_focus_manager_create(obj, obj);
   efl_composite_attach(obj, pd->manager);
   pd->enable_on_visible = EINA_TRUE;
   pd->cycle = EINA_TRUE;
   return obj;
}

EOLIAN static void
_efl_ui_focus_layer_enable_set(Eo *obj, Efl_Ui_Focus_Layer_Data *pd, Eina_Bool v)
{
   if (v)
     {
        pd->registered_manager = elm_widget_top_get(obj);

        efl_ui_focus_manager_calc_register_logical(pd->registered_manager, obj, efl_ui_focus_manager_root_get(pd->registered_manager), obj);
        efl_ui_focus_manager_focus_set(pd->manager, obj);
     }
   else
     {
        if (efl_ui_focus_manager_redirect_get(pd->registered_manager) == obj)
          efl_ui_focus_manager_redirect_set(pd->registered_manager, NULL);

        efl_ui_focus_manager_calc_unregister(pd->registered_manager, obj);
        pd->registered_manager = NULL;
     }
}

EOLIAN static Eina_Bool
_efl_ui_focus_layer_enable_get(Eo *obj, Efl_Ui_Focus_Layer_Data *pd)
{
   if (!pd->registered_manager) return EINA_FALSE;
   return (efl_ui_focus_manager_redirect_get(pd->registered_manager) == obj);
}

EOLIAN static void
_efl_ui_focus_layer_behaviour_set(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Layer_Data *pd, Eina_Bool enable_on_visible, Eina_Bool cycle)
{
   pd->enable_on_visible = enable_on_visible;
   pd->cycle = cycle;
}

EOLIAN static void
_efl_ui_focus_layer_behaviour_get(Eo *obj EINA_UNUSED, Efl_Ui_Focus_Layer_Data *pd, Eina_Bool *enable_on_visible, Eina_Bool *cycle)
{
   *cycle = pd->cycle;
   *enable_on_visible = pd->enable_on_visible;
}

#include "efl_ui_focus_layer.eo.c"
