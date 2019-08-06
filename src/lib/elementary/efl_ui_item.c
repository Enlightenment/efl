#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#define EFL_UI_WIDGET_PART_BG_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_item_private.h"
//part
#include "efl_ui_item_part_text.eo.h"
#include "efl_ui_item_part_icon.eo.h"
#include "efl_ui_item_part_extra.eo.h"
#include "efl_ui_item_part_content.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS      EFL_UI_ITEM_CLASS
#define MY_CLASS_PFX  efl_ui_item

#define MY_CLASS_NAME "Efl.Ui.Item"

/* Efl.Part */
EOLIAN static void
_efl_ui_item_part_text_efl_text_text_set(Eo *obj, void *pd EINA_UNUSED, const char *text)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   efl_text_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), text);

   efl_canvas_group_change(wd->obj);
}

EOLIAN static const char*
_efl_ui_item_part_text_efl_text_text_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_text_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

EOLIAN static void
_efl_ui_item_part_text_efl_text_markup_markup_set(Eo *obj, void *pd EINA_UNUSED, const char *markup)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   efl_text_markup_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), markup);

   efl_canvas_group_change(obj);
}

EOLIAN static const char*
_efl_ui_item_part_text_efl_text_markup_markup_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_text_markup_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

EOLIAN static const char *
_efl_ui_item_part_text_efl_ui_l10n_l10n_text_get(const Eo *obj, void *_pd EINA_UNUSED, const char **domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return elm_widget_part_translatable_text_get(pd->obj, pd->part, domain);
}

EOLIAN static void
_efl_ui_item_part_text_efl_ui_l10n_l10n_text_set(Eo *obj, void *_pd EINA_UNUSED, const char *text, const char *domain)
{
   Elm_Part_Data *pd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   elm_widget_part_translatable_text_set(pd->obj, pd->part, text, domain);
}
#include "efl_ui_item_part_text.eo.c"

/* Efl.Ui.List_Default_Item_Part_Icon */

Eina_Bool
_efl_ui_item_part_icon_efl_content_content_set(Eo *obj, void *pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), content);
}

Efl_Gfx_Entity *
_efl_ui_item_part_icon_efl_content_content_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

Efl_Gfx_Entity *
_efl_ui_item_part_icon_efl_content_content_unset(Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_unset(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}
#include "efl_ui_item_part_icon.eo.c"

Eina_Bool
_efl_ui_item_part_extra_efl_content_content_set(Eo *obj, void *pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), content);
}

Efl_Gfx_Entity *
_efl_ui_item_part_extra_efl_content_content_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

Efl_Gfx_Entity *
_efl_ui_item_part_extra_efl_content_content_unset(Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_unset(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}
#include "efl_ui_item_part_extra.eo.c"

Eina_Bool
_efl_ui_item_part_content_efl_content_content_set(Eo *obj, void *pd EINA_UNUSED, Efl_Gfx_Entity *content)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_set(efl_part(efl_super(wd->obj, MY_CLASS), wd->part), content);
}

Efl_Gfx_Entity *
_efl_ui_item_part_content_efl_content_content_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_get(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}

Efl_Gfx_Entity *
_efl_ui_item_part_content_efl_content_content_unset(Eo *obj, void *pd EINA_UNUSED)
{
   Elm_Part_Data *wd = efl_data_scope_get(obj, EFL_UI_WIDGET_PART_CLASS);
   return efl_content_unset(efl_part(efl_super(wd->obj, MY_CLASS), wd->part));
}
#include "efl_ui_item_part_content.eo.c"
/* Efl.Part */

static void
_item_select(Eo *obj, Efl_Ui_Item_Data *pd)
{
   Efl_Ui_Select_Mode m;

   if (!pd->parent)
     return;
   m = efl_ui_select_mode_get(pd->parent);
   if (m == EFL_UI_SELECT_MODE_NONE || (pd->selected && m != EFL_UI_SELECT_MODE_SINGLE_ALWAYS))
     return;

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
   m = efl_ui_select_mode_get(efl_ui_item_container_get(obj));

   if ((m != EFL_UI_SELECT_MODE_SINGLE_ALWAYS) && (pd->selected))
     _item_unselect(obj, pd);
   else if (m != EFL_UI_SELECT_MODE_NONE)
     _item_select(obj, pd);
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

   efl_ui_action_connector_bind_clickable_to_object(wd->resize_obj, obj);
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
   return efl_pack_index_get(pd->parent, obj);
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
   pd->parent = container;
}

EOLIAN static Efl_Ui_Widget*
_efl_ui_item_container_get(const Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->parent;
}

#include "efl_ui_item.eo.c"
#include "efl_ui_selectable.eo.c"
#include "efl_ui_multi_selectable.eo.c"
#include "efl_ui_single_selectable.eo.c"
