#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_item_private.h"

#define MY_CLASS      EFL_UI_ITEM_CLASS
#define MY_CLASS_PFX  efl_ui_item

#define MY_CLASS_NAME "Efl.Ui.Item"

static Eina_Bool _key_action_select(Evas_Object *obj, const char *params EINA_UNUSED);

static const Elm_Action key_actions[] = {
   {"select", _key_action_select},
   {NULL, NULL}
};

static Eina_Bool
_key_action_select(Evas_Object *obj, const char *params EINA_UNUSED)
{
   efl_ui_selectable_selected_set(obj, EINA_TRUE);
   return EINA_FALSE;
}

static Efl_Ui_Select_Mode
_fetch_state(Eo *obj)
{
   if (efl_isa(obj, EFL_UI_MULTI_SELECTABLE_INTERFACE))
     return efl_ui_multi_selectable_select_mode_get(obj);
   if (efl_isa(obj, EFL_UI_SINGLE_SELECTABLE_INTERFACE))
     return EFL_UI_SELECT_MODE_SINGLE;
   ERR("Uncaught state %s", efl_debug_name_get(obj));
   return EFL_UI_SELECT_MODE_NONE;
}

static void
_item_select(Eo *obj, Efl_Ui_Item_Data *pd)
{
   Efl_Ui_Select_Mode m;

   if (pd->container)
     {
        m = _fetch_state(pd->container);
        if (m == EFL_UI_SELECT_MODE_NONE)
          return;
     }
   else
     {
        if (pd->selected)
          return;
     }

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Eina_Bool tmp = pd->selected = EINA_TRUE;
   edje_object_signal_emit(wd->resize_obj, "efl,state,selected", "efl");
   efl_event_callback_call(obj, EFL_UI_EVENT_SELECTED_CHANGED, &tmp);
}

static void
_item_unselect(Eo *obj, Efl_Ui_Item_Data *pd)
{
   if (!pd->selected) return;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Eina_Bool tmp = pd->selected =EINA_FALSE;
   edje_object_signal_emit(wd->resize_obj, "efl,state,unselected", "efl");
   efl_event_callback_call(obj, EFL_UI_EVENT_SELECTED_CHANGED, &tmp);
}

/* Mouse Controls */
static void
_item_pressed(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Item *obj = data;
   if (efl_ui_widget_disabled_get(obj)) return;

   efl_layout_signal_emit(obj, "efl,state,pressed", "efl");
}

static void
_item_unpressed(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Efl_Ui_Item *obj = data;
   Efl_Ui_Select_Mode m;
   EFL_UI_ITEM_DATA_GET_OR_RETURN(obj, pd);

   if (efl_ui_widget_disabled_get(obj)) return;
   if (!efl_ui_item_container_get(obj)) return;

   efl_layout_signal_emit(obj, "efl,state,unpressed", "efl");
   m = _fetch_state(pd->container);

   if (pd->selected)
     {
        if (efl_ui_selectable_allow_manual_deselection_get(pd->container))
          efl_ui_selectable_selected_set(obj, EINA_FALSE);
     }
   else if (m != EFL_UI_SELECT_MODE_NONE)
     efl_ui_selectable_selected_set(obj, EINA_TRUE);
}

EFL_CALLBACKS_ARRAY_DEFINE(self_listening,
  {EFL_INPUT_EVENT_PRESSED, _item_pressed},
  {EFL_INPUT_EVENT_UNPRESSED, _item_unpressed},
)

/* Mouse Controls ends */

EOLIAN static Eo *
_efl_ui_item_efl_object_constructor(Eo *obj, Efl_Ui_Item_Data *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_ui_layout_finger_size_multiplier_set(obj, 0, 0);

   efl_event_callback_array_add(obj, self_listening(), obj);

   return obj;
}

EOLIAN static Efl_Object *
_efl_ui_item_efl_object_finalize(Eo *obj, Efl_Ui_Item_Data *pd EINA_UNUSED)
{
   Eo *eo;
   eo = efl_finalize(efl_super(obj, MY_CLASS));
   ELM_WIDGET_DATA_GET_OR_RETURN(eo, wd, eo);

   /* Support Item Focus Feature */
   elm_widget_can_focus_set(obj, EINA_TRUE);

   if (efl_ui_layout_theme_version_get(obj) == 123)
     efl_ui_action_connector_bind_clickable_to_object(wd->resize_obj, obj);
   else
     efl_ui_action_connector_bind_clickable_to_theme(wd->resize_obj, obj);
   return eo;
}

EOLIAN static void
_efl_ui_item_efl_object_destructor(Eo *obj, Efl_Ui_Item_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static int
_efl_ui_item_index_get(const Eo *obj, Efl_Ui_Item_Data *pd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->container, -1);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(pd->container, EFL_PACK_LINEAR_INTERFACE), -1);
   return efl_pack_index_get(pd->container, obj);
}

EOLIAN static void
_efl_ui_item_efl_ui_selectable_selected_set(Eo *obj, Efl_Ui_Item_Data *pd, Eina_Bool select)
{
   Eina_Bool selected = !!select;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (wd->disabled) return;

   if (selected) _item_select(obj, pd);
   else _item_unselect(obj, pd);
}

EOLIAN static Eina_Bool
_efl_ui_item_efl_ui_selectable_selected_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->selected;
}

EOLIAN static void
_efl_ui_item_container_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd, Efl_Ui_Widget *container)
{
   pd->container = container;
   if (!pd->container)
     {
        pd->parent = NULL;
     }
}

EOLIAN static Efl_Ui_Widget*
_efl_ui_item_container_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->container;
}

EOLIAN static void
_efl_ui_item_item_parent_set(Eo *obj, Efl_Ui_Item_Data *pd, Efl_Ui_Item *parent)
{
   if (pd->parent)
     {
        ERR("Parent is already set on object %p", obj);
        return;
     }
   if (efl_invalidated_get(obj) || efl_invalidating_get(obj))
     {
        ERR("Parent cannot be set during invalidate");
        return;
     }
   if (pd->container)
     {
        ERR("Parent must be set before adding the object to the container");
        return;
     }
   pd->parent = parent;
}


EOLIAN static Efl_Ui_Item*
_efl_ui_item_item_parent_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->parent;
}

EOLIAN static void
_efl_ui_item_calc_locked_set(Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd, Eina_Bool locked)
{
   pd->locked = !!locked;
}

EOLIAN static Eina_Bool
_efl_ui_item_calc_locked_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->locked;
}

EOLIAN static void
_efl_ui_item_efl_canvas_group_group_need_recalculate_set(Eo *obj, Efl_Ui_Item_Data *pd EINA_UNUSED, Eina_Bool value)
{
   // Prevent recalc when the item are stored in the cache
   // As due to async behavior, we can still have text updated from future that just finished after
   // we have left the releasing stage of factories. This is the simplest way to prevent those later
   // update.
   if (pd->locked) return;
   efl_canvas_group_need_recalculate_set(efl_super(obj, EFL_UI_ITEM_CLASS), value);
}

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_item, Efl_Ui_Item_Data)

#include "efl_ui_item.eo.c"
#include "efl_ui_selectable.eo.c"
#include "efl_ui_multi_selectable.eo.c"
#include "efl_ui_multi_selectable_object_range.eo.c"
#include "efl_ui_single_selectable.eo.c"
#include "efl_ui_item_clickable.eo.c"

