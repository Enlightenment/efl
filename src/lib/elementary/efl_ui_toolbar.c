#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FOCUS_COMPOSITION_PROTECTED
#define EFL_ACCESS_PROTECTED
#define EFL_ACCESS_SELECTION_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define ELM_WIDGET_ITEM_PROTECTED
#define EFL_UI_TRANSLATABLE_PROTECTED
#define EFL_UI_FOCUS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_toolbar_private.h"
#include "efl_ui_toolbar_item.eo.h"

#define MY_CLASS EFL_UI_TOOLBAR_CLASS
#define MY_CLASS_NAME "Efl.Ui.Toolbar"

#define EFL_UI_TOOLBAR_ITEM_FROM_INLIST(item) \
  ((item) ? EINA_INLIST_CONTAINER_GET(item, Efl_Ui_Toolbar_Item_Data) : NULL)

static const char SIG_CLICKED[] = "clicked";
static const char SIG_SELECTED[] = "selected";
static const char SIG_UNSELECTED[] = "unselected";
static const char SIG_ITEM_FOCUSED[] = "item,focused";
static const char SIG_ITEM_UNFOCUSED[] = "item,unfocused";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_SELECTED, ""},
   {SIG_UNSELECTED, ""},
   {SIG_ITEM_FOCUSED, ""},
   {SIG_ITEM_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static Eina_Bool _key_action_select(Evas_Object *obj, const char *params);
static void _sizing_eval(Evas_Object *obj);

static const Elm_Action key_actions[] = {
   {"select", _key_action_select},
   {NULL, NULL}
};

static void _item_select(Efl_Ui_Toolbar_Item_Data *it);

static void
_item_unselect(Efl_Ui_Toolbar_Item_Data *item)
{
   if ((!item) || (!item->selected)) return;

   EFL_UI_TOOLBAR_DATA_GET(WIDGET(item), sd);

   item->selected = EINA_FALSE;
   sd->selected_item = NULL;
   elm_layout_signal_emit(VIEW(item), "elm,state,unselected", "elm");
   if (item->icon)
     elm_widget_signal_emit(item->icon, "elm,state,unselected", "elm");
   efl_event_callback_legacy_call(WIDGET(item), EFL_UI_EVENT_UNSELECTED, EO_OBJ(item));
   if (_elm_config->atspi_mode)
    efl_access_state_changed_signal_emit(EO_OBJ(item), EFL_ACCESS_STATE_SELECTED, EINA_FALSE);
}

static void
_item_mirrored_set(Evas_Object *obj EINA_UNUSED,
                   Efl_Ui_Toolbar_Item_Data *it,
                   Eina_Bool mirrored)
{
   efl_ui_mirrored_set(VIEW(it), mirrored);
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool mirrored)
{
   Efl_Ui_Toolbar_Item_Data *it;

   EFL_UI_TOOLBAR_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->items, it)
     _item_mirrored_set(obj, it, mirrored);
}

static void
_efl_ui_toolbar_item_focused(Elm_Object_Item *eo_it)
{
   EFL_UI_TOOLBAR_ITEM_DATA_GET(eo_it, it);
   Evas_Object *obj = WIDGET(it);
   EFL_UI_TOOLBAR_DATA_GET(obj, sd);
   const char *focus_raise;

   if ((!sd) || (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       (eo_it == sd->focused_item))
     return;

   sd->focused_item = eo_it;

   if (elm_widget_focus_highlight_enabled_get(obj))
     {
        elm_layout_signal_emit
           (VIEW(it), "elm,state,focused", "elm");
     }
   elm_layout_signal_emit
      (VIEW(it), "elm,highlight,on", "elm");
   focus_raise = elm_layout_data_get(VIEW(it), "focusraise");
   if ((focus_raise) && (!strcmp(focus_raise, "on")))
     evas_object_raise(VIEW(it));
   efl_event_callback_legacy_call
     (obj, EFL_UI_TOOLBAR_EVENT_ITEM_FOCUSED, EO_OBJ(it));
   if (_elm_config->atspi_mode)
     efl_access_state_changed_signal_emit(EO_OBJ(it), EFL_ACCESS_STATE_FOCUSED, EINA_TRUE);
}

static void
_efl_ui_toolbar_item_unfocused(Elm_Object_Item *eo_it)
{
   EFL_UI_TOOLBAR_ITEM_DATA_GET(eo_it, it);
   Evas_Object *obj = WIDGET(it);
   EFL_UI_TOOLBAR_DATA_GET(obj, sd);

   if ((!sd) || !sd->focused_item ||
       (eo_it != sd->focused_item))
     return;
   if (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
     return;
   if (elm_widget_focus_highlight_enabled_get(obj))
     {
        EFL_UI_TOOLBAR_ITEM_DATA_GET(sd->focused_item, focus_it);
        elm_layout_signal_emit
           (VIEW(focus_it), "elm,state,unfocused", "elm");
     }
   elm_layout_signal_emit
      (VIEW(it), "elm,highlight,off", "elm");
   sd->focused_item = NULL;
   efl_event_callback_legacy_call
     (obj, EFL_UI_TOOLBAR_EVENT_ITEM_UNFOCUSED, eo_it);
   if (_elm_config->atspi_mode)
     efl_access_state_changed_signal_emit(eo_it, EFL_ACCESS_STATE_FOCUSED, EINA_TRUE);
}

EOLIAN static Eina_Bool
_efl_ui_toolbar_elm_widget_on_focus_update(Eo *obj, Efl_Ui_Toolbar_Data *sd, Elm_Object_Item *item EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Object_Item *eo_it = NULL;

   int_ret = efl_ui_widget_on_focus_update(efl_super(obj, MY_CLASS), NULL);
   if (!int_ret) return EINA_FALSE;
   if (!sd->items) return EINA_FALSE;

   if (elm_widget_focus_get(obj))
     {
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);

        if (sd->last_focused_item)
          eo_it = sd->last_focused_item;
        else if (_elm_config->first_item_focus_on_first_focus_in)
          {
             eo_it = efl_ui_menu_first_item_get(obj);
          }
        if (eo_it) _efl_ui_toolbar_item_focused(eo_it);
     }
   else
     {
        sd->last_focused_item = sd->focused_item;
        if (sd->focused_item)
          _efl_ui_toolbar_item_unfocused(sd->focused_item);
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_toolbar_item_elm_widget_item_focus_set(Eo *eo_it, Efl_Ui_Toolbar_Item_Data *it, Eina_Bool focused)
{
   EFL_UI_TOOLBAR_ITEM_CHECK(it);
   Evas_Object *obj = WIDGET(it);
   EFL_UI_TOOLBAR_DATA_GET(obj, sd);

   if (focused)
     {
        sd->last_focused_item = eo_it;
        if (!elm_object_focus_get(obj))
          elm_object_focus_set(obj, EINA_TRUE);

        if (!elm_object_focus_get(obj))
          return;

        if (eo_it != sd->focused_item)
          {
             if (sd->focused_item)
               _efl_ui_toolbar_item_unfocused(sd->focused_item);
             _efl_ui_toolbar_item_focused(eo_it);
          }

     }
   else
     {
        if (!elm_object_focus_get(obj))
          return;
        if (eo_it)
          _efl_ui_toolbar_item_unfocused(eo_it);
     }

   evas_object_focus_set(VIEW(it), focused);

   _elm_widget_item_highlight_in_theme(obj, EO_OBJ(it));
   _elm_widget_highlight_in_theme_update(obj);
   _elm_widget_focus_highlight_start(obj);
}

EOLIAN static Eina_Bool
_efl_ui_toolbar_item_elm_widget_item_focus_get(Eo *eo_it, Efl_Ui_Toolbar_Item_Data *it)
{
   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Evas_Object *obj = WIDGET(it);
   EFL_UI_TOOLBAR_CHECK(obj) EINA_FALSE;
   EFL_UI_TOOLBAR_DATA_GET(obj, sd);

   if (eo_it == sd->focused_item)
     return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
_key_action_select(Evas_Object *obj, const char *params EINA_UNUSED)
{
   EFL_UI_TOOLBAR_DATA_GET(obj, sd);

   if (!sd->items) return EINA_FALSE;
   if (sd->focused_item)
     {
        EFL_UI_TOOLBAR_ITEM_DATA_GET(sd->focused_item, focus_it);
        _item_select(focus_it);
     }

   return EINA_TRUE;
}

static void
_resizing_eval_item(Efl_Ui_Toolbar_Item_Data *it)
{
   Evas_Coord mw = -1, mh = -1;

   edje_object_size_min_restricted_calc(elm_layout_edje_get(VIEW(it)), &mw, &mh, mw, mh);
   if (!it->object)
     elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   evas_object_size_hint_min_set(VIEW(it), mw, mh);
   evas_object_size_hint_max_set(VIEW(it), -1, -1);
}

EOLIAN static void
_efl_ui_toolbar_item_elm_widget_item_disabled_set(Eo *eo_id, Efl_Ui_Toolbar_Item_Data *sd, Eina_Bool disabled)
{
   elm_wdg_item_disabled_set(efl_super(eo_id, EFL_UI_TOOLBAR_ITEM_CLASS), disabled);
   efl_ui_focus_composition_dirty(WIDGET(sd));
}

EOLIAN static void
_efl_ui_toolbar_item_elm_widget_item_disable(Eo *eo_toolbar, Efl_Ui_Toolbar_Item_Data *toolbar_it)
{
   const char* emission;

   if (elm_wdg_item_disabled_get(eo_toolbar))
     emission = "elm,state,disabled";
   else
     emission = "elm,state,enabled";

   elm_layout_signal_emit(VIEW(toolbar_it), emission, "elm");
   if (toolbar_it->icon)
     elm_widget_signal_emit(toolbar_it->icon, emission, "elm");
}

EOLIAN static void
_efl_ui_toolbar_item_elm_widget_item_signal_emit(Eo *eo_toolbar_it EINA_UNUSED,
                                              Efl_Ui_Toolbar_Item_Data *toolbar_it,
                                              const char *emission,
                                              const char *source)
{
   elm_layout_signal_emit(VIEW(toolbar_it), emission, source);
}

static Eina_Bool
_item_icon_set(Evas_Object *icon_obj,
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
_item_select(Efl_Ui_Toolbar_Item_Data *it)
{
   Evas_Object *obj;
   Eina_Bool sel;

   EFL_UI_TOOLBAR_DATA_GET(WIDGET(it), sd);

   if (elm_wdg_item_disabled_get(EO_OBJ(it)) || (it->object))
     return;

   sel = it->selected;

   if ((sd->select_mode != ELM_OBJECT_SELECT_MODE_NONE) &&
       (sd->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     {
        if (sel)
          {
             if (sd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS)
               _item_unselect(it);
          }
        else
          {
             Elm_Object_Item *eo_it2 =
                efl_ui_menu_selected_item_get(WIDGET(it));
             EFL_UI_TOOLBAR_ITEM_DATA_GET(eo_it2, it2);
             _item_unselect(it2);

             it->selected = EINA_TRUE;
             sd->selected_item = EO_OBJ(it);

             elm_layout_signal_emit(VIEW(it), "elm,state,selected", "elm");
             if (it->icon)
               elm_widget_signal_emit(it->icon, "elm,state,selected", "elm");
          }
     }

   obj = WIDGET(it);

   if ((!sel) || (sd->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS))
     {
        if (it->func) it->func((void *)(WIDGET_ITEM_DATA_GET(EO_OBJ(it))), WIDGET(it), EO_OBJ(it));
     }

   efl_event_callback_legacy_call(obj, EFL_UI_EVENT_SELECTED, EO_OBJ(it));
   if (_elm_config->atspi_mode)
    efl_access_state_changed_signal_emit(EO_OBJ(it), EFL_ACCESS_STATE_SELECTED, EINA_TRUE);
}

/* Send order signals when item is added/deleted.
 * If the given item is on deletion, item_on_deletion should be EINA_TRUE. */
static void
_efl_ui_toolbar_item_order_signal_emit(Efl_Ui_Toolbar_Data *sd,
                                    Efl_Ui_Toolbar_Item_Data *it,
                                    Eina_List *prev_list,
                                    Eina_Bool item_on_deletion)
{
   Efl_Ui_Toolbar_Item_Data *first_it = NULL, *last_it = NULL;
   Evas_Object *first_it_view = NULL, *last_it_view = NULL;
   Efl_Ui_Toolbar_Item_Data *prev_first_it = NULL, *prev_last_it = NULL;
   Evas_Object *prev_first_it_view = NULL, *prev_last_it_view = NULL;
   Eina_List *list = NULL;
   Eina_Iterator *itr;
   Evas_Object *sobj;

   itr = efl_content_iterate(sd->bx);
   EINA_ITERATOR_FOREACH(itr, sobj)
      list = eina_list_append(list, sobj);

   if (!list) return;

   if (prev_list)
     {
        prev_first_it_view = eina_list_data_get(prev_list);
        prev_last_it_view = eina_list_data_get(eina_list_last(prev_list));
        prev_first_it = evas_object_data_get(prev_first_it_view, "item");
        prev_last_it = evas_object_data_get(prev_last_it_view, "item");
     }

   first_it_view = eina_list_data_get(list);
   last_it_view = eina_list_data_get(eina_list_last(list));
   first_it = evas_object_data_get(first_it_view, "item");
   last_it = evas_object_data_get(last_it_view, "item");

   if (prev_first_it)
     {
        if ((prev_first_it != first_it) && (prev_first_it != last_it))
          elm_layout_signal_emit(VIEW(prev_first_it), "elm,order,default,item", "elm");
        else if (prev_first_it == last_it)
          elm_layout_signal_emit(VIEW(prev_first_it), "elm,order,last,item", "elm");
     }

   if (prev_last_it)
     {
        if ((prev_last_it != last_it) && (prev_last_it != first_it))
          elm_layout_signal_emit(VIEW(prev_last_it), "elm,order,default,item", "elm");
        else if (prev_last_it == first_it)
          elm_layout_signal_emit(VIEW(prev_last_it), "elm,order,first,item", "elm");
     }

   if (it)
     {
        if (!item_on_deletion)
          {
             if (first_it == last_it)
               {
                  elm_layout_signal_emit(VIEW(it), "elm,order,first,item", "elm");
                  elm_layout_signal_emit(VIEW(it), "elm,order,last,item", "elm");
               }
             else if (it == first_it)
               {
                  elm_layout_signal_emit(VIEW(it), "elm,order,first,item", "elm");
               }
             else if (it == last_it)
               {
                  elm_layout_signal_emit(VIEW(it), "elm,order,last,item", "elm");
               }
          }
        else if (first_it != last_it)
          {
             if (it == first_it)
               {
                  Eina_List *next_l = eina_list_next(list);
                  first_it_view = eina_list_data_get(next_l);
                  first_it = evas_object_data_get(first_it_view, "item");

                  elm_layout_signal_emit(first_it_view, "elm,order,first,item", "elm");
               }
             else if (it == last_it)
               {
                  Eina_List *prev_l = eina_list_prev(eina_list_last(list));
                  last_it_view = eina_list_data_get(prev_l);
                  last_it = evas_object_data_get(last_it_view, "item");

                  elm_layout_signal_emit(last_it_view, "elm,order,last,item", "elm");
               }
          }
     }

   eina_list_free(list);
}

static void
_item_del(Efl_Ui_Toolbar_Item_Data *it)
{
   EFL_UI_TOOLBAR_DATA_GET(WIDGET(it), sd);

   _item_unselect(it);

   _efl_ui_toolbar_item_order_signal_emit(sd, it, NULL, EINA_TRUE);

   eina_stringshare_del(it->label);
   if (it->label)
     elm_layout_signal_emit(VIEW(it), "elm,state,text,hidden", "elm");
   eina_stringshare_del(it->icon_str);

   if (it->icon)
     {
        elm_layout_signal_emit(VIEW(it), "elm,state,icon,hidden", "elm");
        evas_object_del(it->icon);
     }

   if (sd->focused_item == EO_OBJ(it))
     sd->focused_item = NULL;
   if (sd->last_focused_item == EO_OBJ(it))
     sd->last_focused_item = NULL;

   evas_object_del(it->object);
}

static void
_item_theme_hook(Evas_Object *obj,
                 Efl_Ui_Toolbar_Item_Data *it,
                 double scale)
{
   Evas_Coord mw = -1, mh = -1;
   Evas_Object *view = VIEW(it);
   const char *style;

   EFL_UI_TOOLBAR_DATA_GET(obj, sd);

   style = elm_widget_style_get(obj);

   _item_mirrored_set(obj, it, efl_ui_mirrored_get(obj));
   edje_object_scale_set(elm_layout_edje_get(view), scale);

   if (!it->object)
     {
        if (!elm_layout_theme_set(view, "toolbar", "item", style))
          CRI("Failed to set layout!");
        if (it->selected)
          {
             elm_layout_signal_emit(view, "elm,state,selected", "elm");
             if (it->icon)
               elm_widget_signal_emit(it->icon, "elm,state,selected", "elm");
          }
        if (elm_wdg_item_disabled_get(EO_OBJ(it)))
          {
             elm_layout_signal_emit(view, "elm,state,disabled", "elm");
             if (it->icon)
               elm_widget_signal_emit(it->icon, "elm,state,disabled", "elm");
          }
        if (it->icon)
          {
             elm_layout_content_set(view, "elm.swallow.icon", it->icon);
             elm_layout_signal_emit
               (view, "elm,state,icon,visible", "elm");
          }
        if (it->label)
          {
             elm_layout_text_set(view, "elm.text", it->label);
             elm_layout_signal_emit(view, "elm,state,text,visible", "elm");
          }
     }
   else
     {
        if (!elm_layout_theme_set(view, "toolbar", "object", style))
          CRI("Failed to set layout!");
        elm_layout_content_set(view, "elm.swallow.object", it->object);
     }

   //FIXME: after signal rule is fixed, this should be considered again
   elm_layout_signal_emit(view, "elm,state,shrink,default", "elm");

   if (!efl_ui_dir_is_horizontal(sd->dir, EINA_TRUE))
     elm_layout_signal_emit(view, "elm,orient,vertical", "elm");
   else
     elm_layout_signal_emit(view, "elm,orient,horizontal", "elm");

    edje_object_message_signal_process(elm_layout_edje_get(view));
    if (!it->object)
      elm_coords_finger_size_adjust(1, &mw, 1, &mh);

    if (!efl_ui_dir_is_horizontal(sd->dir, EINA_TRUE))
      {
         evas_object_size_hint_weight_set(view, EVAS_HINT_EXPAND, -1.0);
         evas_object_size_hint_align_set
            (view, EVAS_HINT_FILL, EVAS_HINT_FILL);
      }
    else
      {
         evas_object_size_hint_weight_set(VIEW(it), -1.0, EVAS_HINT_EXPAND);
         evas_object_size_hint_align_set
            (view, EVAS_HINT_FILL, EVAS_HINT_FILL);
      }

    _resizing_eval_item(it);
    evas_object_smart_need_recalculate_set(obj, EINA_TRUE);
}

static void
_inform_item_number(Evas_Object *obj)
{
   EFL_UI_TOOLBAR_DATA_GET(obj, sd);
   Efl_Ui_Toolbar_Item_Data *it;
   char buf[sizeof("elm,number,item,") + 4];
   static int scount = 0;
   int count = 0;
   Evas_Coord mw, mh;

   EINA_INLIST_FOREACH(sd->items, it)
     {
        count++;
     }
   if (scount != count)
     {
        scount = count;
        if (snprintf(buf, sizeof(buf), "elm,number,item,%d", count) >= (int)sizeof(buf))
          ERR("Too many items to fit signal buffer (%d)", count);

        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (!it->object)
               {
                  elm_layout_signal_emit(VIEW(it), buf, "elm");
                  edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));

                  mw = mh = -1;
                  elm_coords_finger_size_adjust(1, &mw, 1, &mh);

                  edje_object_size_min_restricted_calc(elm_layout_edje_get(VIEW(it)), &mw, &mh, mw, mh);
                  evas_object_size_hint_min_set(VIEW(it), mw, mh);
               }
          }
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, minw_bx = -1, minh_bx = -1;
   Evas_Coord w, h;

   EFL_UI_TOOLBAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   if (w < minw) w = minw;
   if (h < minh) h = minh;

   evas_object_resize(wd->resize_obj, w, h);
   evas_object_size_hint_combined_min_get(sd->bx, &minw_bx, &minh_bx);
   evas_object_size_hint_min_set(obj, minw_bx, minh_bx);

   _inform_item_number(obj);
}

static void
_efl_ui_toolbar_highlight_in_theme(Evas_Object *obj)
{
   const char *fh;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   fh = edje_object_data_get
       (wd->resize_obj, "focus_highlight");
   if ((fh) && (!strcmp(fh, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
}

EOLIAN static Efl_Ui_Theme_Apply
_efl_ui_toolbar_elm_widget_theme_apply(Eo *obj, Efl_Ui_Toolbar_Data *sd)
{
   Efl_Ui_Toolbar_Item_Data *it;
   double scale = 0;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_FAILED);

   if (sd->delete_me) return EFL_UI_THEME_APPLY_SUCCESS;

   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

   elm_widget_theme_object_set
     (obj, wd->resize_obj, "toolbar", "base",
     elm_widget_style_get(obj));
   if (!efl_ui_dir_is_horizontal(sd->dir, EINA_TRUE))
     edje_object_signal_emit(wd->resize_obj, "elm,orient,vertical", "elm");
   else
     edje_object_signal_emit(wd->resize_obj, "elm,orient,horizontal", "elm");

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   scale = (efl_ui_scale_get(obj) * elm_config_scale_get());
   EINA_INLIST_FOREACH(sd->items, it)
     _item_theme_hook(obj, it, scale);

   _efl_ui_toolbar_highlight_in_theme(obj);
   evas_object_smart_need_recalculate_set(obj, EINA_TRUE);

   return int_ret;
}

static void
_efl_ui_toolbar_item_label_update(Efl_Ui_Toolbar_Item_Data *item)
{
   elm_layout_text_set(VIEW(item), "elm.text", item->label);
   if (item->label)
     elm_layout_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
   else
     elm_layout_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");
}

static void
_efl_ui_toolbar_item_label_set_cb(void *data,
                               Evas_Object *obj,
                               const char *emission,
                               const char *source)
{
   Efl_Ui_Toolbar_Item_Data *item = data;

   _efl_ui_toolbar_item_label_update(item);
   elm_layout_signal_callback_del
     (obj, emission, source, _efl_ui_toolbar_item_label_set_cb);
   elm_layout_signal_emit(VIEW(item), "elm,state,label,reset", "elm");
}

static void
_item_label_set(Efl_Ui_Toolbar_Item_Data *item,
                const char *label,
                const char *sig)
{
   const char *s;

   if ((label) && (item->label) && (!strcmp(label, item->label))) return;

   eina_stringshare_replace(&item->label, label);
   s = elm_layout_data_get(VIEW(item), "transition_animation_on");
   if ((s) && (atoi(s)))
     {
        elm_layout_text_set
          (VIEW(item), "elm.text_new", item->label);
        elm_layout_signal_emit(VIEW(item), sig, "elm");
        elm_layout_signal_callback_add
          (VIEW(item), "elm,state,label_set,done", "elm",
          _efl_ui_toolbar_item_label_set_cb, item);
     }
   else
     _efl_ui_toolbar_item_label_update(item);

   _resizing_eval_item(item);
}

EOLIAN static void
_efl_ui_toolbar_item_elm_widget_item_part_text_set(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *item,
                    const char *part,
                    const char *label)
{
   char buf[256];

   if ((!part) || (!strcmp(part, "default")) ||
       (!strcmp(part, "elm.text")))
     {
        _item_label_set(item, label, "elm,state,label_set");
     }
   else
     {
        if (label)
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,visible", part);
             elm_layout_signal_emit(VIEW(item), buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,hidden", part);
             elm_layout_signal_emit(VIEW(item), buf, "elm");
          }
        elm_layout_text_set(VIEW(item), part, label);
     }
}

EOLIAN static const char *
_efl_ui_toolbar_item_elm_widget_item_part_text_get(Eo *eo_it EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *it,
                    const char *part)
{
   char buf[256];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), "elm.text");
   else
     snprintf(buf, sizeof(buf), "%s", part);

   return elm_layout_text_get(VIEW(it), buf);
}

EOLIAN static void
_efl_ui_toolbar_item_elm_widget_item_part_content_set(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *item,
                       const char *part,
                       Evas_Object *content)
{
   Evas_Object *obj = WIDGET(item);
   double scale;

   if (part && strcmp(part, "object") && strcmp(part, "elm.swallow.object"))
     {
        efl_content_set(efl_part(VIEW(item), part), content);
        return;
     }
   if (item->object == content) return;

   evas_object_del(item->object);

   item->object = content;
   if (item->object)
     elm_widget_sub_object_add(obj, item->object);

   scale = (efl_ui_scale_get(obj) * elm_config_scale_get());
   _item_theme_hook(obj, item, scale);
}

EOLIAN static Evas_Object *
_efl_ui_toolbar_item_elm_widget_item_part_content_get(Eo *eo_it EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *it,
                       const char *part)
{
   if (part && strcmp(part, "object") && strcmp(part, "elm.swallow.object"))
     {
        return efl_content_get(efl_part(VIEW(it), part));
     }
   return it->object;
}

EOLIAN static Evas_Object *
_efl_ui_toolbar_item_elm_widget_item_part_content_unset(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *item,
                         const char *part)
{
   Evas_Object *obj = WIDGET(item);
   Evas_Object *o;
   double scale;

   if (part && strcmp(part, "object") && strcmp(part, "elm.swallow.object"))
     {
        return efl_content_unset(efl_part(VIEW(item), part));
     }

   elm_layout_content_unset(VIEW(item), "elm.swallow.object");
   _elm_widget_sub_object_redirect_to_top(obj, item->object);
   o = item->object;
   item->object = NULL;
   scale = (efl_ui_scale_get(obj) * elm_config_scale_get());
   _item_theme_hook(obj, item, scale);

   return o;
}

EOLIAN static void
_efl_ui_toolbar_efl_ui_translatable_translation_update(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   Efl_Ui_Toolbar_Item_Data *it;

   EINA_INLIST_FOREACH(sd->items, it)
     elm_wdg_item_translate(EO_OBJ(it));

   efl_ui_translatable_translation_update(efl_super(obj, MY_CLASS));
}

static void
_action_click_cb(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   Efl_Ui_Toolbar_Item_Data *it = data;

   if ((_elm_config->access_mode == ELM_ACCESS_MODE_OFF) ||
       (_elm_access_2nd_click_timeout(VIEW(it))))
     {
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
          _elm_access_say(E_("Selected"));
        _item_select(it);
     }
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Efl_Ui_Toolbar_Item_Data *it = (Efl_Ui_Toolbar_Item_Data *)data;
   const char *txt = (it->base)->access_info;

   if (!txt) txt = it->label;
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Efl_Ui_Toolbar_Item_Data *it = (Efl_Ui_Toolbar_Item_Data *)data;

   if (elm_wdg_item_disabled_get(EO_OBJ(it)))
     return strdup(E_("State: Disabled"));
   else if (it->selected)
     return strdup(E_("State: Selected"));

   return NULL;
}

EOLIAN static void
_efl_ui_toolbar_item_efl_object_destructor(Eo *eo_item, Efl_Ui_Toolbar_Item_Data *item)
{
   Efl_Ui_Toolbar_Item_Data *next = NULL;
   Evas_Object *obj;

   EFL_UI_TOOLBAR_DATA_GET(WIDGET(item), sd);

   obj = WIDGET(item);


   if (!sd->delete_me)
     next = EFL_UI_TOOLBAR_ITEM_FROM_INLIST(EINA_INLIST_GET(item)->next);
   sd->items = eina_inlist_remove(sd->items, EINA_INLIST_GET(item));
   sd->item_count--;
   if (!sd->delete_me)
     {
        if (!next) next = EFL_UI_TOOLBAR_ITEM_FROM_INLIST(sd->items);
        if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS) &&
            item->selected && next) _item_select(next);
     }

   efl_ui_focus_composition_dirty(WIDGET(item));

   _item_del(item);
   efl_ui_widget_theme_apply(obj);

   efl_destructor(efl_super(eo_item, EFL_UI_TOOLBAR_ITEM_CLASS));
}

static void
_access_activate_cb(void *data EINA_UNUSED,
                    Evas_Object *part_obj EINA_UNUSED,
                    Elm_Object_Item *item)
{
   EFL_UI_TOOLBAR_ITEM_DATA_GET(item, it);
   EFL_UI_TOOLBAR_DATA_GET(WIDGET(it), sd);

   if (elm_wdg_item_disabled_get(item)) return;

   if (it->selected && (sd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS))
     {
        _elm_access_say(E_("Unselected"));
        _item_unselect(it);
     }
   else
     {
        _elm_access_say(E_("Selected"));
        _item_select(it);
     }
}

static void
_access_widget_item_register(Efl_Ui_Toolbar_Item_Data *it)
{
   Elm_Access_Info *ai;
   _elm_access_widget_item_register(it->base);
   ai = _elm_access_info_get(it->base->access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Toolbar Item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);
   _elm_access_activate_callback_set(ai, _access_activate_cb, NULL);
}

EOLIAN static Eina_Rect
_efl_ui_toolbar_item_efl_ui_focus_object_focus_geometry_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *pd)
{
   Eina_Rect rect;

   evas_object_geometry_get(VIEW(pd), &rect.x, &rect.y, &rect.w, &rect.h);

   return rect;
}

EOLIAN static Eo *
_efl_ui_toolbar_item_efl_object_constructor(Eo *eo_it, Efl_Ui_Toolbar_Item_Data *it)
{
   eo_it = efl_constructor(efl_super(eo_it, EFL_UI_TOOLBAR_ITEM_CLASS));
   it->base = efl_data_scope_get(eo_it, ELM_WIDGET_ITEM_CLASS);
   efl_access_role_set(eo_it, EFL_ACCESS_ROLE_MENU_ITEM);

   return eo_it;
}

static Efl_Ui_Toolbar_Item_Data *
_item_new(Evas_Object *obj,
          const char *icon,
          const char *label,
          Evas_Smart_Cb func,
          const void *data)
{
   Evas_Object *icon_obj;

   EFL_UI_TOOLBAR_DATA_GET(obj, sd);

   Eo *eo_it = efl_add(EFL_UI_TOOLBAR_ITEM_CLASS, obj);

   if (!eo_it) return NULL;

   EFL_UI_TOOLBAR_ITEM_DATA_GET(eo_it, it);

   it->label = eina_stringshare_add(label);
   it->func = func;
   it->object = NULL;
   WIDGET_ITEM_DATA_SET(EO_OBJ(it), data);

   VIEW(it) = elm_layout_add(obj);
   elm_widget_tree_unfocusable_set(VIEW(it), EINA_TRUE);
   evas_object_data_set(VIEW(it), "item", it);
   efl_access_type_set(VIEW(it), EFL_ACCESS_TYPE_DISABLED);

   icon_obj = elm_icon_add(VIEW(it));

   if (_elm_config->atspi_mode)
       if (icon_obj) efl_access_parent_set(icon_obj, eo_it);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_widget_item_register(it);

   if (_item_icon_set(icon_obj, "toolbar/", icon))
     {
        it->icon = icon_obj;
        it->icon_str = eina_stringshare_add(icon);
     }
   else
     {
        it->icon = NULL;
        it->icon_str = NULL;
        evas_object_del(icon_obj);
     }

   if (!elm_layout_theme_set
       (VIEW(it), "toolbar", "item", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   elm_layout_signal_callback_add
     (VIEW(it), "elm,action,click", "elm", _action_click_cb, it);

   if (it->icon)
     {
        elm_layout_content_set(VIEW(it), "elm.swallow.icon", it->icon);
        elm_layout_signal_emit(VIEW(it), "elm,state,icon,visible", "elm");
        elm_layout_signal_emit(VIEW(it), "elm,icon,visible", "elm");
        evas_object_show(it->icon);
     }
   else
     {
        elm_layout_signal_emit(VIEW(it), "elm,state,icon,hidden", "elm");
        elm_layout_signal_emit(VIEW(it), "elm,icon,hidden", "elm");
     }

   if (it->label)
     {
        elm_layout_text_set(VIEW(it), "elm.text", it->label);
        elm_layout_signal_emit(VIEW(it), "elm,state,text,visible", "elm");
        elm_layout_signal_emit(VIEW(it), "elm,text,visible", "elm");
     }
   else
     {
        elm_layout_signal_emit(VIEW(it), "elm,state,text,hidden", "elm");
        elm_layout_signal_emit(VIEW(it), "elm,text,hidden", "elm");
     }

   edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));

   if ((!sd->items) && (sd->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS))
     _item_select(it);

   if (_elm_config->atspi_mode)
        efl_access_added(eo_it);

   return it;
}

static void
_efl_ui_toolbar_item_icon_update(Efl_Ui_Toolbar_Item_Data *item)
{
   Evas_Object *old_icon =
     elm_layout_content_get(VIEW(item), "elm.swallow.icon");

   _elm_widget_sub_object_redirect_to_top(WIDGET(item), old_icon);
   elm_layout_content_unset(VIEW(item), "elm.swallow.icon");
   elm_layout_content_set(VIEW(item), "elm.swallow.icon", item->icon);
   if (item->icon)
       elm_layout_signal_emit(VIEW(item), "elm,state,icon,visible", "elm");
   else
       elm_layout_signal_emit(VIEW(item), "elm,state,icon,hidden", "elm");

   evas_object_del(old_icon);
}

static void
_efl_ui_toolbar_item_icon_set_cb(void *data,
                              Evas_Object *obj,
                              const char *emission,
                              const char *source)
{
   Efl_Ui_Toolbar_Item_Data *item = data;
   _efl_ui_toolbar_item_icon_update(item);
   elm_layout_signal_callback_del
     (obj, emission, source, _efl_ui_toolbar_item_icon_set_cb);
   elm_layout_signal_emit(VIEW(item), "elm,state,icon,reset", "elm");
}

static void
_efl_ui_toolbar_item_icon_obj_set(Evas_Object *obj,
                               Efl_Ui_Toolbar_Item_Data *item,
                               Evas_Object *icon_obj,
                               const char *icon_str,
                               const char *sig)
{
   Evas_Object *old_icon;
   const char *s;

   if (icon_str)
     eina_stringshare_replace(&item->icon_str, icon_str);
   else
     {
        eina_stringshare_del(item->icon_str);
        item->icon_str = NULL;
     }
   item->icon = icon_obj;

   if (icon_obj)
     {
        evas_object_show(item->icon);
        elm_widget_sub_object_add(obj, item->icon);
     }
   s = elm_layout_data_get(VIEW(item), "transition_animation_on");
   if ((s) && (atoi(s)))
     {
        old_icon = elm_layout_content_get
            (VIEW(item), "elm.swallow.icon_new");
        if (old_icon)
          {
             _elm_widget_sub_object_redirect_to_top(WIDGET(item), old_icon);
             evas_object_hide(old_icon);
          }
        elm_layout_content_set
          (VIEW(item), "elm.swallow.icon_new", item->icon);
        elm_layout_signal_emit(VIEW(item), sig, "elm");
        elm_layout_signal_callback_add
          (VIEW(item), "elm,state,icon_set,done", "elm",
          _efl_ui_toolbar_item_icon_set_cb, item);
     }
   else
     _efl_ui_toolbar_item_icon_update(item);
   _resizing_eval_item(item);
}

EOLIAN static void
_efl_ui_toolbar_efl_object_destructor(Eo *obj, Efl_Ui_Toolbar_Data *sd)
{
   Efl_Ui_Toolbar_Item_Data *it, *next;

   sd->delete_me = EINA_TRUE;

   it = EFL_UI_TOOLBAR_ITEM_FROM_INLIST(sd->items);
   while (it)
     {
        next = EFL_UI_TOOLBAR_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        elm_wdg_item_del(EO_OBJ(it));
        it = next;
     }
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_toolbar_efl_gfx_position_set(Eo *obj, Efl_Ui_Toolbar_Data *sd EINA_UNUSED, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), pos);
}

EOLIAN static void
_efl_ui_toolbar_efl_gfx_size_set(Eo *obj, Efl_Ui_Toolbar_Data *sd EINA_UNUSED, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), sz);
}

EOLIAN static void
_efl_ui_toolbar_efl_canvas_group_group_member_add(Eo *obj, Efl_Ui_Toolbar_Data *sd EINA_UNUSED, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);
}

static Eina_Bool _efl_ui_toolbar_smart_focus_next_enable = EINA_FALSE;

static void
_access_obj_process(Efl_Ui_Toolbar_Data *sd, Eina_Bool is_access)
{
   Efl_Ui_Toolbar_Item_Data *it;

   EINA_INLIST_FOREACH (sd->items, it)
     {
        if (is_access) _access_widget_item_register(it);
        else _elm_access_widget_item_unregister(it->base);
     }
}

EOLIAN static void
_efl_ui_toolbar_elm_widget_on_access_update(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd, Eina_Bool acs)
{
   _efl_ui_toolbar_smart_focus_next_enable = acs;
   _access_obj_process(sd, _efl_ui_toolbar_smart_focus_next_enable);
}

EOLIAN static void
_efl_ui_toolbar_item_efl_ui_focus_object_focus_set(Eo *obj, Efl_Ui_Toolbar_Item_Data *pd EINA_UNUSED, Eina_Bool focus)
{
   efl_ui_focus_object_focus_set(efl_super(obj, EFL_UI_TOOLBAR_ITEM_CLASS), focus);
   elm_wdg_item_focus_set(obj, focus);
}

EOLIAN static Eina_Rect
_efl_ui_toolbar_elm_widget_focus_highlight_geometry_get(Eo *obj, Efl_Ui_Toolbar_Data *sd)
{
   Eina_Rect r = {};

   if (sd->focused_item)
     {
        EFL_UI_TOOLBAR_ITEM_DATA_GET(sd->focused_item, focus_it);

        evas_object_geometry_get(VIEW(focus_it), &r.x, &r.y, &r.w, &r.h);
        elm_widget_focus_highlight_focus_part_geometry_get(VIEW(focus_it), &r.x, &r.y, &r.w, &r.h);
     }
   else
     evas_object_geometry_get(obj, &r.x, &r.y, &r.w, &r.h);

   return r;
}

EOLIAN static Efl_Object *
_efl_ui_toolbar_efl_object_constructor(Eo *obj, Efl_Ui_Toolbar_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_role_set(obj, EFL_ACCESS_ROLE_TOOL_BAR);
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set(obj, "toolbar", "base",
                             elm_object_style_get(obj)))
	   CRI("Failed to set layout!");

   elm_widget_can_focus_set(obj, EINA_TRUE);

   sd->dir = EFL_UI_DIR_HORIZONTAL;
   sd->bx = efl_add(EFL_UI_BOX_CLASS, obj,
                    efl_ui_direction_set(efl_added, sd->dir),
                    efl_pack_align_set(efl_added, -1.0, -1.0));

   elm_layout_content_set(obj, "elm.swallow.content", sd->bx);
   _efl_ui_toolbar_highlight_in_theme(obj);
   _sizing_eval(obj);

   efl_ui_focus_composition_custom_manager_set(obj, obj);
   return obj;
}

EOLIAN static Eina_Bool
_efl_ui_toolbar_elm_widget_focus_state_apply(Eo *obj, Efl_Ui_Toolbar_Data *pd EINA_UNUSED, Elm_Widget_Focus_State current_state, Elm_Widget_Focus_State *configured_state, Elm_Widget *redirect EINA_UNUSED)
{
   configured_state->logical = EINA_TRUE;
   return efl_ui_widget_focus_state_apply(efl_super(obj, MY_CLASS), current_state, configured_state, obj);
}

EOLIAN static Elm_Object_Item*
_efl_ui_toolbar_item_append(Eo *obj, Efl_Ui_Toolbar_Data *sd, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   Efl_Ui_Toolbar_Item_Data *it;
   double scale;
   Eina_List *prev_list = NULL;
   Eina_Iterator *itr;
   Evas_Object *sobj;

   it = _item_new(obj, icon, label, func, data);
   if (!it) return NULL;
   scale = (efl_ui_scale_get(obj) * elm_config_scale_get());

   itr = efl_content_iterate(sd->bx);
   EINA_ITERATOR_FOREACH(itr, sobj)
      prev_list = eina_list_append(prev_list, sobj);

   sd->items = eina_inlist_append(sd->items, EINA_INLIST_GET(it));
   efl_pack(sd->bx, VIEW(it));
   evas_object_show(VIEW(it));

   _item_theme_hook(obj, it, scale);
   sd->item_count++;

   _efl_ui_toolbar_item_order_signal_emit(sd, it, prev_list, EINA_FALSE);
   eina_list_free(prev_list);

   return EO_OBJ(it);
}

EOLIAN static Elm_Object_Item*
_efl_ui_toolbar_item_prepend(Eo *obj, Efl_Ui_Toolbar_Data *sd, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   Efl_Ui_Toolbar_Item_Data *it;
   double scale;
   Eina_List *prev_list = NULL;
   Eina_Iterator *itr;
   Evas_Object *sobj;

   it = _item_new(obj, icon, label, func, data);
   if (!it) return NULL;
   scale = (efl_ui_scale_get(obj) * elm_config_scale_get());

   itr = efl_content_iterate(sd->bx);
   EINA_ITERATOR_FOREACH(itr, sobj)
      prev_list = eina_list_append(prev_list, sobj);

   sd->items = eina_inlist_prepend(sd->items, EINA_INLIST_GET(it));
   efl_pack_begin(sd->bx, VIEW(it));
   evas_object_show(VIEW(it));
   _item_theme_hook(obj, it, scale);
   sd->item_count++;

   _efl_ui_toolbar_item_order_signal_emit(sd, it, prev_list, EINA_FALSE);
   eina_list_free(prev_list);

   return EO_OBJ(it);
}

EOLIAN static Elm_Object_Item*
_efl_ui_toolbar_item_insert_before(Eo *obj, Efl_Ui_Toolbar_Data *sd, Elm_Object_Item *eo_before, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   Efl_Ui_Toolbar_Item_Data *it;
   double scale;
   Eina_List *prev_list = NULL;
   Eina_Iterator *itr;
   Evas_Object *sobj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(eo_before, NULL);
   EFL_UI_TOOLBAR_ITEM_DATA_GET(eo_before, _before);
   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(_before,  NULL);

   it = _item_new(obj, icon, label, func, data);
   if (!it) return NULL;
   scale = (efl_ui_scale_get(obj) * elm_config_scale_get());

   itr = efl_content_iterate(sd->bx);
   EINA_ITERATOR_FOREACH(itr, sobj)
      prev_list = eina_list_append(prev_list, sobj);

   sd->items = eina_inlist_prepend_relative
       (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(_before));

   efl_pack_before(sd->bx, VIEW(it), VIEW(_before));
   _item_theme_hook(obj, it, scale);
   sd->item_count++;

   _efl_ui_toolbar_item_order_signal_emit(sd, it, prev_list, EINA_FALSE);
   eina_list_free(prev_list);

   return EO_OBJ(it);
}

EOLIAN static Elm_Object_Item*
_efl_ui_toolbar_item_insert_after(Eo *obj, Efl_Ui_Toolbar_Data *sd, Elm_Object_Item *eo_after, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   Efl_Ui_Toolbar_Item_Data *it;
   double scale;
   Eina_List *prev_list = NULL;
   Eina_Iterator *itr;
   Evas_Object *sobj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(eo_after, NULL);
   EFL_UI_TOOLBAR_ITEM_DATA_GET(eo_after, _after);
   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(_after, NULL);

   it = _item_new(obj, icon, label, func, data);
   if (!it) return NULL;
   scale = (efl_ui_scale_get(obj) * elm_config_scale_get());

   itr = efl_content_iterate(sd->bx);
   EINA_ITERATOR_FOREACH(itr, sobj)
      prev_list = eina_list_append(prev_list, sobj);

   sd->items = eina_inlist_append_relative
       (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(_after));

   efl_pack_after(sd->bx, VIEW(it), VIEW(_after));
   _item_theme_hook(obj, it, scale);
   sd->item_count++;

   _efl_ui_toolbar_item_order_signal_emit(sd, it, prev_list, EINA_FALSE);
   eina_list_free(prev_list);

   return EO_OBJ(it);
}

EOLIAN static Elm_Object_Item*
_efl_ui_toolbar_efl_ui_menu_first_item_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   if (!sd->items) return NULL;
   Efl_Ui_Toolbar_Item_Data *it = EFL_UI_TOOLBAR_ITEM_FROM_INLIST(sd->items);
   if (it) return EO_OBJ(it);
   return NULL;
}

EOLIAN static Elm_Object_Item*
_efl_ui_toolbar_efl_ui_menu_last_item_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   if (!sd->items) return NULL;

   Efl_Ui_Toolbar_Item_Data *it = EFL_UI_TOOLBAR_ITEM_FROM_INLIST(sd->items->last);
   if (it) return EO_OBJ(it);
   return NULL;
}

EOLIAN static Eina_Iterator*
_efl_ui_toolbar_efl_ui_menu_items_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   return eina_inlist_iterator_new(sd->items);
}

EOLIAN static Elm_Object_Item *
_efl_ui_toolbar_item_efl_ui_item_next_get(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *it)
{
   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   Efl_Ui_Toolbar_Item_Data *ret_it = EFL_UI_TOOLBAR_ITEM_FROM_INLIST(
            EINA_INLIST_GET(it)->next);
   if (ret_it) return EO_OBJ(ret_it);
   else return NULL;
}

EOLIAN static Elm_Object_Item *
_efl_ui_toolbar_item_efl_ui_item_prev_get(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *it)
{
   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   Efl_Ui_Toolbar_Item_Data *ret_it = EFL_UI_TOOLBAR_ITEM_FROM_INLIST(
            EINA_INLIST_GET(it)->prev);
   if (ret_it) return EO_OBJ(ret_it);
   else return NULL;
}

EOLIAN static void
_efl_ui_toolbar_item_efl_ui_item_selected_set(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *item, Eina_Bool selected)
{
   if (item->selected == selected) return;
   if (selected) _item_select(item);
   else _item_unselect(item);
}

EOLIAN static Eina_Bool
_efl_ui_toolbar_item_efl_ui_item_selected_get(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *item)
{
   return item->selected;
}

EOLIAN static Elm_Object_Item*
_efl_ui_toolbar_efl_ui_menu_selected_item_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   return sd->selected_item;
}

EOLIAN static void
_efl_ui_toolbar_item_icon_set(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *item,
                          const char *icon)
{
   Evas_Object *obj;
   Evas_Object *icon_obj;

   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(item);

   obj = WIDGET(item);
   if ((icon) && (item->icon_str) && (!strcmp(icon, item->icon_str))) return;

   icon_obj = elm_icon_add(obj);
   if (!icon_obj) return;
   if (_item_icon_set(icon_obj, "toolbar/", icon))
     _efl_ui_toolbar_item_icon_obj_set
       (obj, item, icon_obj, icon, "elm,state,icon_set");
   else
     {
        _efl_ui_toolbar_item_icon_obj_set
          (obj, item, NULL, NULL, "elm,state,icon_set");
        evas_object_del(icon_obj);
     }
}

EOLIAN static const char *
_efl_ui_toolbar_item_icon_get(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *it)
{
   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   return it->icon_str;
}

EOLIAN static Evas_Object *
_efl_ui_toolbar_item_icon_object_get(Eo *eo_it EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *it)
{
   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   return it->icon;
}

EOLIAN static Eina_Bool
_efl_ui_toolbar_item_icon_memfile_set(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *item,
                                  const void *img,
                                  size_t size,
                                  const char *format,
                                  const char *key)
{
   Evas_Object *icon_obj;
   Evas_Object *obj;
   Eina_Bool ret;

   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);

   obj = WIDGET(item);

   if (img && size)
     {
        icon_obj = elm_icon_add(obj);
        evas_object_repeat_events_set(icon_obj, EINA_TRUE);
        ret = elm_image_memfile_set(icon_obj, img, size, format, key);
        if (!ret)
          {
             evas_object_del(icon_obj);
             return EINA_FALSE;
          }
        _efl_ui_toolbar_item_icon_obj_set
          (obj, item, icon_obj, NULL, "elm,state,icon_set");
     }
   else
     _efl_ui_toolbar_item_icon_obj_set
       (obj, item, NULL, NULL, "elm,state,icon_set");
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_toolbar_item_icon_file_set(Eo *eo_item EINA_UNUSED, Efl_Ui_Toolbar_Item_Data *item,
                                  const char *file,
                                  const char *key)
{
   Evas_Object *icon_obj;
   Evas_Object *obj;
   Eina_Bool ret;

   EFL_UI_TOOLBAR_ITEM_CHECK_OR_RETURN(item, EINA_FALSE);

   obj = WIDGET(item);

   if (file)
     {
        icon_obj = elm_icon_add(obj);
        evas_object_repeat_events_set(icon_obj, EINA_TRUE);
        ret = elm_image_file_set(icon_obj, file, key);
        if (!ret)
          {
             evas_object_del(icon_obj);
             return EINA_FALSE;
          }
        _efl_ui_toolbar_item_icon_obj_set
          (obj, item, icon_obj, NULL, "elm,state,icon_set");
     }
   else
     _efl_ui_toolbar_item_icon_obj_set
       (obj, item, NULL, NULL, "elm,state,icon_set");
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_toolbar_efl_ui_direction_direction_set(Eo *obj, Efl_Ui_Toolbar_Data *sd, Efl_Ui_Dir dir)
{
   // Adjust direction to be either horizontal or vertical.
   if (efl_ui_dir_is_horizontal(dir, EINA_TRUE))
     dir = EFL_UI_DIR_HORIZONTAL;
   else
     dir = EFL_UI_DIR_VERTICAL;

   if (sd->dir == dir) return;

   sd->dir = dir;
   efl_ui_direction_set(sd->bx, sd->dir);

   _sizing_eval(obj);
}

EOLIAN static Efl_Ui_Dir
_efl_ui_toolbar_efl_ui_direction_direction_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   return sd->dir;
}

EOLIAN static unsigned int
_efl_ui_toolbar_items_count(const Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   return sd->item_count;
}

EOLIAN static void
_efl_ui_toolbar_select_mode_set(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd, Elm_Object_Select_Mode mode)
{
   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;

   if (sd->select_mode == mode) return;
   sd->select_mode = mode;

   if ((mode == ELM_OBJECT_SELECT_MODE_ALWAYS) &&
       (sd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS) &&
       sd->items)
     _item_select(EFL_UI_TOOLBAR_ITEM_FROM_INLIST(sd->items));

}

EOLIAN static Elm_Object_Select_Mode
_efl_ui_toolbar_select_mode_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   return sd->select_mode;
}

EOLIAN static const char*
_efl_ui_toolbar_item_efl_access_name_get(Eo *eo_item, Efl_Ui_Toolbar_Item_Data *item)
{
   const char *ret;
   ret = efl_access_name_get(efl_super(eo_item, EFL_UI_TOOLBAR_ITEM_CLASS));
   if (ret) return ret;
   return _elm_widget_item_accessible_plain_name_get(eo_item, item->label);
}

EOLIAN static Efl_Access_State_Set
_efl_ui_toolbar_item_efl_access_state_set_get(Eo *eo_it, Efl_Ui_Toolbar_Item_Data *item EINA_UNUSED)
{
   Efl_Access_State_Set ret;
   Eina_Bool sel;

   ret = efl_access_state_set_get(efl_super(eo_it, EFL_UI_TOOLBAR_ITEM_CLASS));

   sel = efl_ui_item_selected_get(eo_it);

   STATE_TYPE_SET(ret, EFL_ACCESS_STATE_SELECTABLE);

   if (sel)
      STATE_TYPE_SET(ret, EFL_ACCESS_STATE_SELECTED);

   return ret;
}

EOLIAN static Elm_Object_Item *
_efl_ui_toolbar_elm_widget_focused_item_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   return sd->focused_item;
}

EOLIAN static void
_efl_ui_toolbar_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}

EOLIAN static const Efl_Access_Action_Data*
_efl_ui_toolbar_efl_access_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "select", "select", NULL, _key_action_select},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

EOLIAN static Eina_List*
_efl_ui_toolbar_efl_access_children_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *sd)
{
   Eina_List *ret = NULL, *ret2 = NULL;
   Efl_Ui_Toolbar_Item_Data *it;
   ret2 = efl_access_children_get(efl_super(obj, EFL_UI_TOOLBAR_CLASS));

   EINA_INLIST_FOREACH(sd->items, it)
      ret = eina_list_append(ret, EO_OBJ(it));

   return eina_list_merge(ret, ret2);
}

EOLIAN static Efl_Access_State_Set
_efl_ui_toolbar_efl_access_state_set_get(Eo *obj, Efl_Ui_Toolbar_Data *sd EINA_UNUSED)
{
   return efl_access_state_set_get(efl_super(obj, EFL_UI_TOOLBAR_CLASS));
}

EOLIAN int
_efl_ui_toolbar_efl_access_selection_selected_children_count_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd)
{
   return pd->selected_item ? 1 : 0;
}

EOLIAN Eo*
_efl_ui_toolbar_efl_access_selection_selected_child_get(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd, int child_idx)
{
   if (child_idx != 0)
     return NULL;

   return pd->selected_item;
}

EOLIAN Eina_Bool
_efl_ui_toolbar_efl_access_selection_child_select(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd, int child_index)
{
   Efl_Ui_Toolbar_Item_Data *item;
   EINA_INLIST_FOREACH(pd->items, item)
     {
        if (child_index-- == 0)
          {
             efl_ui_item_selected_set(EO_OBJ(item), EINA_TRUE);
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_toolbar_efl_access_selection_selected_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd, int child_index)
{
   if (child_index != 0)
     return EINA_FALSE;

   if (!pd->selected_item)
     return EINA_FALSE;

   efl_ui_item_selected_set(pd->selected_item, EINA_FALSE);

   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_toolbar_efl_access_selection_is_child_selected(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd, int child_index)
{
   Efl_Ui_Toolbar_Item_Data *item;

   EINA_INLIST_FOREACH(pd->items, item)
     {
        if (child_index-- == 0)
          {
             return efl_ui_item_selected_get(EO_OBJ(item));
          }
     }
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_toolbar_efl_access_selection_all_children_select(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_efl_ui_toolbar_efl_access_selection_clear(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd)
{
   if (pd->selected_item)
     efl_ui_item_selected_set(pd->selected_item, EINA_FALSE);
   return EINA_TRUE;
}

EOLIAN Eina_Bool
_efl_ui_toolbar_efl_access_selection_child_deselect(Eo *obj EINA_UNUSED, Efl_Ui_Toolbar_Data *pd, int child_index)
{
   Efl_Ui_Toolbar_Item_Data *item;
   if (pd->select_mode != ELM_OBJECT_SELECT_MODE_NONE)
     {
        EINA_INLIST_FOREACH(pd->items, item)
          {
             if (child_index-- == 0)
               {
                  efl_ui_item_selected_set(EO_OBJ(item), EINA_FALSE);
                  return EINA_TRUE;
               }
          }
     }
   return EINA_FALSE;
}

EOLIAN void
_efl_ui_toolbar_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Toolbar_Data *pd EINA_UNUSED)
{
   _sizing_eval(obj);
}

static Eina_Bool
_part_of_chain(Efl_Ui_Toolbar_Item_Data *pd)
{
   Eina_Bool want = EINA_TRUE;
   if (elm_wdg_item_disabled_get(pd->base->eo_obj))
     want = EINA_FALSE;

   if (!evas_object_visible_get(VIEW(pd)))
     want = EINA_FALSE;

   return want;
}

EOLIAN static void
_efl_ui_toolbar_efl_ui_focus_composition_prepare(Eo *obj, Efl_Ui_Toolbar_Data *pd)
{
   Efl_Ui_Toolbar_Item_Data *it;
   Eina_List *order = NULL;

   EINA_INLIST_FOREACH(pd->items, it)
     {
       if (_part_of_chain(it))
          order = eina_list_append(order, EO_OBJ(it));
     }

   efl_ui_focus_composition_elements_set(obj, order);
}

/* Standard widget overrides */
ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_toolbar, Efl_Ui_Toolbar_Data)

#include "efl_ui_toolbar.eo.c"
#include "efl_ui_toolbar_item.eo.c"
