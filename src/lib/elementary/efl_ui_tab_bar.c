#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif


#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_tab_bar_private.h"
#include "els_box.h"

#define MY_CLASS EFL_UI_TAB_BAR_CLASS
#define MY_CLASS_NAME "Efl.Ui.Tab_Bar"

static const char PART_NAME_TAB[] = "tab";

static void _tab_select(Efl_Ui_Tab_Bar_Data *sd, Tab_Info *ti);

static Eina_Bool _key_action_select(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"select", _key_action_select},
   {NULL, NULL}
};

static Eina_Bool
_key_action_select(Evas_Object *obj, const char *params EINA_UNUSED)
{
   EFL_UI_TAB_BAR_DATA_GET(obj, sd);

   if (!sd->tab_infos) return EINA_FALSE;

   Tab_Info *ti;
   Eina_List *l, *l_next;
   EINA_LIST_FOREACH_SAFE(sd->tab_infos, l, l_next, ti)
     {
        if (efl_ui_focus_object_focus_get(ti->tab))
          {
             _tab_select(sd, ti);
             return EINA_TRUE;
          }
     }

   return EINA_FALSE;
}

EOLIAN static void
_efl_ui_tab_bar_current_tab_set(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *sd, int index)
{
   Tab_Info *ti;
   ti = eina_list_nth(sd->tab_infos, index);

   _tab_select(sd, ti);
}

EOLIAN static int
_efl_ui_tab_bar_current_tab_get(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *sd)
{
   return sd->cur;
}

EOLIAN static unsigned int
_efl_ui_tab_bar_tab_count(const Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *sd)
{
   return sd->cnt;
}

static void
_tab_icon_update(Tab_Info *ti)
{
   Eo *old_icon =
     efl_content_get(efl_part(ti->tab, "efl.icon"));

   _elm_widget_sub_object_redirect_to_top(ti->tab, old_icon);
   efl_content_unset(efl_part(ti->tab, "efl.icon"));
   efl_content_set(efl_part(ti->tab, "efl.icon"), ti->icon);

   efl_del(old_icon);
}

static void
_tab_icon_set_cb(void *data,
                 Eo *obj,
                 const char *emission,
                 const char *source)
{
   Tab_Info *ti = data;
   _tab_icon_update(ti);

   efl_layout_signal_callback_del(obj, emission, source, ti, _tab_icon_set_cb, NULL);
   efl_layout_signal_emit(ti->tab, "efl,state,icon,reset", "efl");
}

static void
_tab_icon_obj_set(Eo *obj,
                  Tab_Info *ti,
                  Eo *icon_obj,
                  const char *icon_str)
{
   Eo *old_icon;
   const char *s;

   if (icon_str)
     eina_stringshare_replace(&ti->icon_str, icon_str);
   else
     {
        eina_stringshare_del(ti->icon_str);
        ti->icon_str = NULL;
     }

   ti->icon = icon_obj;
   if (icon_obj)
     {
        efl_gfx_entity_visible_set(ti->icon, EINA_TRUE);
        elm_widget_sub_object_add(obj, ti->icon);
     }

   s = elm_layout_data_get(ti->tab, "transition_animation_on");
   if ((s) && (atoi(s)))
     {
        old_icon = efl_content_get
            (efl_part(ti->tab, "efl.icon_new"));
        if (old_icon)
          {
             _elm_widget_sub_object_redirect_to_top(ti->tab, old_icon);
             efl_gfx_entity_visible_set(old_icon, EINA_FALSE);
          }
        efl_content_set
          (efl_part(ti->tab, "efl.icon_new"), ti->icon);
        efl_layout_signal_emit(ti->tab, "efl,state,icon_new,set", "efl");
        efl_layout_signal_callback_add
          (ti->tab, "efl,state,icon_set,done", "efl", ti, _tab_icon_set_cb, NULL);
     }
   else
     _tab_icon_update(ti);
}

static Eina_Bool
_tab_icon_set(Eo *icon_obj,
              const char *type,
              const char *icon)
{
   char icon_str[512];

   if ((!type) || (!*type)) goto end;
   if ((!icon) || (!*icon)) return EINA_FALSE;
   if ((snprintf(icon_str, sizeof(icon_str), "%s%s", type, icon) > 0)
       && (elm_icon_standard_set(icon_obj, icon_str)))
     return EINA_TRUE;
end:
   if (elm_icon_standard_set(icon_obj, icon))
     return EINA_TRUE;

   WRN("couldn't find icon definition for '%s'", icon);
   return EINA_FALSE;
}

static void
_tab_unselect(Efl_Ui_Tab_Bar_Data *sd, Tab_Info *ti)
{
   if ((!ti->tab) || (!ti->selected)) return;

   ti->selected = EINA_FALSE;

   efl_layout_signal_emit(ti->tab, "efl,state,unselected", "efl");
   if (ti->icon)
     elm_widget_signal_emit(ti->icon, "efl,state,unselected", "efl");

   sd->cur = -1;
}

static void
_tab_select(Efl_Ui_Tab_Bar_Data *sd, Tab_Info *ti)
{
   if (!ti->selected)
     {
        Eo *tp;
        tp = efl_parent_get(ti->tab);
        int index;

        _tab_unselect(sd, sd->selected_tab);
        ti->selected = EINA_TRUE;
        sd->selected_tab = ti;

        efl_layout_signal_emit(ti->tab, "efl,state,selected", "efl");
        if (ti->icon)
          elm_widget_signal_emit(ti->icon, "efl,state,selected", "efl");

        index = eina_list_data_idx(sd->tab_infos, ti);

        sd->cur = index;

        efl_event_callback_call(tp, EFL_UI_EVENT_ITEM_SELECTED, NULL);
     }
}

static void
_action_click_cb(void *data,
                 Eo *obj EINA_UNUSED,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   Eo *tb;
   Tab_Info *ti;
   ti = data;
   tb = efl_parent_get(ti->tab);
   EFL_UI_TAB_BAR_DATA_GET(tb, sd);

   _tab_select(sd, ti);
}

static Tab_Info *
_tab_add(Eo *obj, const char *label, const char *icon)
{
   Eo *tab, *icon_obj;
   Tab_Info *ti;
   Eina_Error theme_apply;

   ti = calloc(1, sizeof(*ti));

   ti->tab = NULL;
   ti->label = eina_stringshare_add(label);

   tab = efl_add(EFL_UI_LAYOUT_CLASS, obj,
                 efl_gfx_hint_align_set(efl_added, EVAS_HINT_FILL, EVAS_HINT_FILL));
   /* FIXME: This is for tab sizing issue.
    * Recently, the size_hint_fill API has been added,
    * but currently tab_bar is not available because it uses evas_object_box.
    * This should be removed after the box in tab_bar has been replaced by efl.ui.box */

   icon_obj = elm_icon_add(tab);

   if (_tab_icon_set(icon_obj, "toolbar/", icon))
     {
        ti->icon = icon_obj;
        ti->icon_str = eina_stringshare_add(icon);
     }
   else
     {
        ti->icon = NULL;
        ti->icon_str = NULL;
        efl_del(icon_obj);
     }

   theme_apply = elm_widget_element_update(obj, tab, PART_NAME_TAB);

   if (theme_apply == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   efl_layout_signal_callback_add
     (tab, "efl,action,click", "efl", ti,_action_click_cb, NULL);

   if (ti->icon)
     efl_content_set(efl_part(tab, "efl.icon"), ti->icon);

   if (ti->label)
     efl_text_set(efl_part(tab, "efl.text"), ti->label);

   efl_ui_widget_focus_allow_set(tab, EINA_TRUE);

   ti->tab = tab;

   return ti;
}

EOLIAN static void
_efl_ui_tab_bar_tab_add(Eo *obj, Efl_Ui_Tab_Bar_Data *sd,
                        int index, const char *label, const char *icon)
{
   Tab_Info *ti;
   ti = _tab_add(obj, label, icon);

   efl_parent_set(ti->tab, obj);
   efl_ui_widget_sub_object_add(obj, ti->tab);

   if (sd->cnt > index)
     {
        Tab_Info *existing_ti;
        existing_ti = eina_list_nth(sd->tab_infos, index);

        sd->tab_infos = eina_list_prepend_relative(sd->tab_infos, ti, existing_ti);
        evas_object_box_insert_before(sd->bx, ti->tab, existing_ti->tab);

        if (sd->cur >= index) sd->cur ++;
     }
   else
     {
        sd->tab_infos = eina_list_append(sd->tab_infos, ti);
        evas_object_box_append(sd->bx, ti->tab);
     }

   sd->cnt ++;
}

EOLIAN static void
_efl_ui_tab_bar_tab_remove(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *sd, int index)
{
   if ((sd->cnt > 0) && (sd->cnt > index) && (index >= 0))
     {
        Tab_Info *ti;
        ti = eina_list_nth(sd->tab_infos, index);

        _tab_unselect(sd, ti);
        evas_object_box_remove(sd->bx, ti->tab);
        efl_del(ti->tab);

        sd->tab_infos = eina_list_remove(sd->tab_infos, ti);
        sd->cnt --;
     }
}

EOLIAN static void
_efl_ui_tab_bar_tab_label_set(Eo *obj EINA_UNUSED, Efl_Ui_Tab_Bar_Data *sd, int index, const char *label)
{
   Tab_Info *ti;
   ti = eina_list_nth(sd->tab_infos, index);
   eina_stringshare_replace(&ti->label, label);

   efl_text_set(efl_part(ti->tab, "efl.text"), ti->label);
}

EOLIAN static void
_efl_ui_tab_bar_tab_icon_set(Eo *obj, Efl_Ui_Tab_Bar_Data *sd, int index, const char *icon)
{
   Eo *icon_obj;
   Tab_Info *ti;
   ti = eina_list_nth(sd->tab_infos, index);

   if ((icon) && (ti->icon_str) && (!strcmp(icon, ti->icon_str))) return;

   icon_obj = elm_icon_add(obj);
   if (!icon_obj) return;
   if (_tab_icon_set(icon_obj, "toolbar/", icon))
     _tab_icon_obj_set(obj, ti, icon_obj, icon);
   else
     {
        _tab_icon_obj_set(obj, ti, NULL, NULL);
        efl_del(icon_obj);
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

   horizontal = efl_ui_dir_is_horizontal(sd->dir, EINA_TRUE);

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

   sd->dir = EFL_UI_DIR_HORIZONTAL;
   sd->bx = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_box_align_set(sd->bx, 0.5, 0.5);
   evas_object_box_layout_set(sd->bx, _layout, obj, NULL);

   efl_ui_widget_focus_allow_set(obj, EINA_TRUE);

   efl_content_set(efl_part(obj, "efl.content"), sd->bx);

   sd->cnt = 0;
   sd->cur = -1;

   Tab_Info *ti = calloc(1, sizeof(*ti));
   sd->selected_tab = ti;

   return obj;
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_tab_bar, Efl_Ui_Tab_Bar_Data)

#include "efl_ui_tab_bar.eo.c"
