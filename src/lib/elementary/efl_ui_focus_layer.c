#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED
#define EFL_UI_FOCUS_LAYER_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#define MY_CLASS EFL_UI_FOCUS_LAYER_MIXIN

typedef struct {
   Efl_Ui_Focus_Object *old_focus;
   Efl_Ui_Focus_Manager *registered_manager;
   Efl_Ui_Focus_Manager *manager;
   Eina_Bool cycle;
   Eina_Bool enable_on_visible;
} Efl_Ui_Focus_Layer_Data;

EOLIAN static Efl_Ui_Focus_Manager*
_efl_ui_focus_layer_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj, Efl_Ui_Focus_Layer_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   pd->manager = efl_add(EFL_UI_FOCUS_MANAGER_ROOT_FOCUS_CLASS, obj, efl_ui_focus_manager_root_set(efl_added, root));
   return pd->manager;
}

EOLIAN static void
_efl_ui_focus_layer_efl_gfx_entity_visible_set(Eo *obj, Efl_Ui_Focus_Layer_Data *pd, Eina_Bool v)
{
   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), v);

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
_efl_ui_focus_layer_efl_ui_focus_object_focus_manager_get(const Eo *obj, Efl_Ui_Focus_Layer_Data *pd EINA_UNUSED)
{
   if (pd->registered_manager)
     return elm_widget_top_get(obj);
   else
     return NULL;
}

EOLIAN static Efl_Ui_Focus_Object*
_efl_ui_focus_layer_efl_ui_focus_object_focus_parent_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Layer_Data *pd)
{
   if (pd->registered_manager)
     return efl_ui_focus_manager_root_get(pd->registered_manager);
   else
     return NULL;
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
   pd->enable_on_visible = EINA_TRUE;
   pd->cycle = EINA_TRUE;
   return obj;
}

static void
_publish_state_change(Eo *obj, Efl_Ui_Focus_Manager *omanager, Efl_Ui_Focus_Object *oobj)
{
   efl_event_callback_call(obj, EFL_UI_FOCUS_OBJECT_EVENT_MANAGER_CHANGED, omanager);
   efl_event_callback_call(obj, EFL_UI_FOCUS_OBJECT_EVENT_LOGICAL_CHANGED, oobj);
}

EOLIAN static void
_efl_ui_focus_layer_enable_set(Eo *obj, Efl_Ui_Focus_Layer_Data *pd, Eina_Bool v)
{
   if (!elm_object_tree_focus_allow_get(obj))
     v = EINA_FALSE;
   if (v)
     {
        Efl_Ui_Focus_Manager *manager;

        pd->registered_manager = elm_widget_top_get(obj);
        manager = efl_ui_focus_util_active_manager(EFL_UI_FOCUS_UTIL_CLASS, pd->registered_manager);

        efl_ui_focus_manager_calc_register_logical(pd->registered_manager, obj, efl_ui_focus_manager_root_get(pd->registered_manager), obj);
        _publish_state_change(obj, NULL, NULL);

        pd->old_focus = efl_ui_focus_manager_focus_get(manager);
        efl_ui_focus_manager_focus_set(pd->manager, obj);

     }
   else
     {
        Eina_Bool fallback = EINA_TRUE;

        Eo *omanager = pd->registered_manager, *oobj = efl_ui_focus_manager_root_get(omanager);

        if (!pd->registered_manager) return;

        //restore old focus
        if (pd->old_focus)
          {
             Efl_Ui_Focus_Manager *manager;

             manager = efl_ui_focus_object_focus_manager_get(pd->old_focus);
             if (manager)
               {
                  efl_ui_focus_manager_focus_set(manager, pd->old_focus);
                  fallback = EINA_FALSE;
               }
          }

        pd->old_focus = NULL;

        if (fallback && efl_ui_focus_manager_redirect_get(pd->registered_manager) == obj)
          efl_ui_focus_manager_redirect_set(pd->registered_manager, NULL);

        efl_ui_focus_manager_calc_unregister(pd->registered_manager, obj);
        pd->registered_manager = NULL;
        _publish_state_change(obj, omanager, oobj);
     }
}

EOLIAN static Eina_Bool
_efl_ui_focus_layer_enable_get(const Eo *obj, Efl_Ui_Focus_Layer_Data *pd)
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
_efl_ui_focus_layer_behaviour_get(const Eo *obj EINA_UNUSED, Efl_Ui_Focus_Layer_Data *pd, Eina_Bool *enable_on_visible, Eina_Bool *cycle)
{
   *cycle = pd->cycle;
   *enable_on_visible = pd->enable_on_visible;
}

#include "efl_ui_focus_layer.eo.c"
