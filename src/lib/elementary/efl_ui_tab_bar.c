#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_tab_bar_private.h"
#include "els_box.h"

#define MY_CLASS EFL_UI_TAB_BAR_CLASS
#define MY_CLASS_NAME "Efl.Ui.Tab_Bar"


EOLIAN static Efl_Ui_Selectable*
_efl_ui_tab_bar_efl_ui_single_selectable_last_selected_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd)
{
   return pd->selected;
}

EOLIAN static unsigned int
_efl_ui_tab_bar_tab_count(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *sd)
{
   return sd->cnt;
}

EOLIAN static void
_efl_ui_tab_bar_efl_ui_single_selectable_fallback_selection_set(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd, Efl_Ui_Selectable *fallback)
{
   pd->fallback_selection = fallback;
   if (!pd->selected)
     efl_ui_selectable_selected_set(pd->fallback_selection, EINA_TRUE);
}

EOLIAN static Efl_Ui_Selectable*
_efl_ui_tab_bar_efl_ui_single_selectable_fallback_selection_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *pd)
{
   return pd->fallback_selection;
}

static void _remove_item(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Ui_Item *item);

static void
_selelction_change_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Tab_Bar_Data *pd = efl_data_scope_safe_get(data, MY_CLASS);

   if (!efl_ui_selectable_selected_get(ev->object))
     {
        if (pd->selected == ev->object)
          {
             pd->selected = NULL;
          }
        //checkout if we want to do fallback handling
        if (!pd->in_value_change)
          {
             if (!pd->selected && pd->fallback_selection)
               efl_ui_selectable_selected_set(pd->fallback_selection, EINA_TRUE);
          }
     }
   else
     {
        pd->in_value_change = EINA_TRUE;
        if (pd->selected)
          efl_ui_selectable_selected_set(pd->selected, EINA_FALSE);
        pd->in_value_change = EINA_FALSE;
        EINA_SAFETY_ON_FALSE_RETURN(!pd->selected);
        pd->selected = ev->object;
        efl_event_callback_call(data, EFL_UI_EVENT_ITEM_SELECTED, NULL);
     }
   if (!pd->in_value_change)
     {
        efl_event_callback_call(data, EFL_UI_SINGLE_SELECTABLE_EVENT_SELECTION_CHANGED, NULL);
     }
}

static void
_invalidate_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Tab_Bar_Data *pd = efl_data_scope_safe_get(data, MY_CLASS);

   EINA_SAFETY_ON_NULL_RETURN(pd);
   _remove_item(data, pd, ev->object);
}

EFL_CALLBACKS_ARRAY_DEFINE(item_listeners,
  {EFL_UI_EVENT_SELECTED_CHANGED, _selelction_change_cb},
  {EFL_EVENT_INVALIDATE, _invalidate_cb},
)

static void
_remove_item(Eo *obj, Efl_Ui_Tab_Bar_Data *pd, Efl_Ui_Item *item)
{
   pd->tab_infos = eina_list_remove(pd->tab_infos, item);

   if (pd->selected == item)
     pd->selected = NULL;

   efl_event_callback_array_del(item, item_listeners(), obj);
}

EOLIAN static void
_efl_ui_tab_bar_tab_add(Eo *obj, Efl_Ui_Tab_Bar_Data *sd,
                        int index, Efl_Ui_Tab_Bar_Default_Item *tab)
{
   efl_ui_item_container_set(tab, obj);
   efl_ui_widget_sub_object_add(obj, tab);
   efl_event_callback_array_add(tab, item_listeners(), obj);
   efl_gfx_hint_align_set(tab, EVAS_HINT_FILL, EVAS_HINT_FILL);
   if (sd->cnt > index)
     {
        Efl_Ui_Item *existing_ti;
        existing_ti = eina_list_nth(sd->tab_infos, index);

        sd->tab_infos = eina_list_prepend_relative(sd->tab_infos, tab, existing_ti);
        evas_object_box_insert_before(sd->bx, tab, existing_ti);
     }
   else
     {
        sd->tab_infos = eina_list_append(sd->tab_infos, tab);
        evas_object_box_append(sd->bx, tab);
     }

   sd->cnt ++;
}

EOLIAN static void
_efl_ui_tab_bar_tab_remove(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *sd, int index)
{
   if ((sd->cnt > 0) && (sd->cnt > index) && (index >= 0))
     {
        Efl_Ui_Item *ti;
        ti = eina_list_nth(sd->tab_infos, index);

        efl_del(ti);
     }
}

static void
_layout(Evas_Object *o,
        Evas_Object_Box_Data *priv,
        void *data)
{
   Evas_Object *obj = (Evas_Object *)data;
   Eina_Bool horizontal;

   EFL_UI_TAB_BAR_DATA_GET(obj, sd);

   horizontal = efl_ui_layout_orientation_is_horizontal(sd->dir, EINA_TRUE);

   _els_box_layout
     (o, priv, horizontal, EINA_TRUE, efl_ui_mirrored_get(obj));
}

EOLIAN static void
_efl_ui_tab_bar_efl_object_destructor(Eo *obj, Efl_Ui_Tab_Bar_Data *sd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Object *
_efl_ui_tab_bar_efl_object_constructor(Eo *obj, Efl_Ui_Tab_Bar_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "tab_bar");

   obj = efl_constructor(efl_super(obj, MY_CLASS));

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   sd->dir = EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL;
   sd->bx = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_box_align_set(sd->bx, 0.5, 0.5);
   evas_object_box_layout_set(sd->bx, _layout, obj, NULL);

   efl_ui_widget_focus_allow_set(obj, EINA_TRUE);

   efl_content_set(efl_part(obj, "efl.content"), sd->bx);

   sd->cnt = 0;

   return obj;
}

#include "efl_ui_tab_bar.eo.c"
