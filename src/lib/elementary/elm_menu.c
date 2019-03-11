#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_WIDGET_PROTECTED
#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_SELECTION_PROTECTED
#define ELM_WIDGET_ITEM_PROTECTED
#define EFL_UI_L10N_PROTECTED
#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_menu.h"

#define MY_CLASS ELM_MENU_CLASS

#define MY_CLASS_NAME "Elm_Menu"
#define MY_CLASS_NAME_LEGACY "elm_menu"

#define ELM_PRIV_MENU_SIGNALS(cmd) \
   cmd(SIG_CLICKED, "clicked", "") \
   cmd(SIG_DISMISSED, "dismissed", "")

ELM_PRIV_MENU_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   ELM_PRIV_MENU_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};
#undef ELM_PRIV_MENU_SIGNALS

EOLIAN static void
_elm_menu_efl_ui_l10n_translation_update(Eo *obj EINA_UNUSED, Elm_Menu_Data *sd)
{
   Elm_Menu_Item_Data *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_wdg_item_translate(EO_OBJ(it));
}

static void
_item_del(Elm_Object_Item *eo_item)
{
   Elm_Object_Item *child;
   ELM_MENU_ITEM_DATA_GET(eo_item, item);

   Eina_List *itr, *itr2;
   EINA_LIST_FOREACH_SAFE(item->submenu.items, itr, itr2, child)
     _item_del(child);
   eina_list_free(item->submenu.items);

   efl_del(eo_item);
}

static void
_submenu_hide(Elm_Menu_Item_Data *item)
{
   Eina_List *l;
   Elm_Object_Item *eo_item2;

   evas_object_hide(item->submenu.hv);
   item->submenu.open = EINA_FALSE;

   EINA_LIST_FOREACH(item->submenu.items, l, eo_item2)
     {
        ELM_MENU_ITEM_DATA_GET(eo_item2, item2);
        if (item2->submenu.open) _submenu_hide(item2);
     }
}

static void
_elm_menu_item_elm_widget_item_disable(Eo *eo_item, Elm_Menu_Item_Data *item)
{
   if (elm_object_item_disabled_get((Elm_Object_Item*)eo_item))
     {
        elm_layout_signal_emit(VIEW(item), "elm,state,disabled", "elm");
        if (item->submenu.open) _submenu_hide(item);
     }
   else
     elm_layout_signal_emit(VIEW(item), "elm,state,enabled", "elm");

   if (item->dbus_menu) _elm_dbus_menu_update(item->dbus_menu);
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(item)));
}

static void
_elm_menu_item_elm_widget_item_signal_emit(Eo *eo_item EINA_UNUSED, Elm_Menu_Item_Data *item,
                       const char *emission,
                       const char *source)
{
   elm_layout_signal_emit(VIEW(item), emission, source);
}

static inline void
_parent_geometry_get(Elm_Menu_Data *sd, int *x, int *y, int *w, int *h)
{
   Eina_Rect r;

   r = efl_gfx_entity_geometry_get(sd->parent);
   if (efl_isa(sd->parent, EFL_UI_WIN_CLASS))
     {
        if (sd->menu_bar && efl_canvas_object_is_frame_object_get(sd->obj))
          r.pos = efl_gfx_entity_position_get(sd->obj);
        else
          r.pos = EINA_POSITION2D(0, 0);
     }
   if (x) *x = r.x;
   if (y) *y = r.y;
   if (w) *w = r.w;
   if (h) *h = r.h;
}

static void
_submenu_sizing_eval(Elm_Menu_Item_Data *parent_it)
{
   Eina_List *l;
   Elm_Object_Item *eo_item;
   Evas_Coord x_p, y_p, w_p, h_p, x2, y2, w2,
              h2, bx, by, bw, bh, px, py, pw, ph;
   ELM_MENU_DATA_GET_OR_RETURN(WIDGET(parent_it), sd);

   EINA_LIST_FOREACH(parent_it->submenu.items, l, eo_item)
     {
        ELM_MENU_ITEM_DATA_GET(eo_item, item);
        elm_layout_sizing_eval(VIEW(item));
        if (_elm_config->atspi_mode)
          efl_access_state_changed_signal_emit(eo_item, EFL_ACCESS_STATE_TYPE_SHOWING, EINA_TRUE);
     }


   evas_object_geometry_get
     (parent_it->submenu.location, &x_p, &y_p, &w_p, &h_p);
   evas_object_geometry_get(VIEW(parent_it), &x2, &y2, &w2, &h2);
   evas_object_geometry_get(parent_it->submenu.bx, &bx, &by, &bw, &bh);
   _parent_geometry_get(sd, &px, &py, &pw, &ph);

   if (sd->menu_bar && !parent_it->parent)
     {
        x_p = x2;
        y_p = y2 + h2;
     }
   else
     {
        x_p = x2 + w2;
        y_p = y2;
     }

   /* If it overflows on the right, adjust the x */
   if ((x_p + bw > px + pw) || efl_ui_mirrored_get(WIDGET(parent_it)))
     x_p = x2 - bw;

   /* If it overflows on the left, adjust the x - usually only happens
    * with an RTL interface */
   if (x_p < px)
     x_p = x2 + w2;

   /* If after all the adjustments it still overflows, fix it */
   if (x_p + bw > px + pw)
     x_p = x2 - bw;

   if (y_p + bh > py + ph)
     y_p -= y_p + bh - (py + ph);

   if (sd->menu_bar && (y_p < py))
     y_p = py;

   evas_object_geometry_set(parent_it->submenu.location, x_p, y_p, bw, h_p);
   evas_object_size_hint_min_set(parent_it->submenu.location, bw, h_p);
   evas_object_size_hint_max_set(parent_it->submenu.location, bw, h_p);
   elm_hover_target_set(parent_it->submenu.hv, parent_it->submenu.location);

   EINA_LIST_FOREACH(parent_it->submenu.items, l, eo_item)
     {
        ELM_MENU_ITEM_DATA_GET(eo_item, item);
        if (item->submenu.open) _submenu_sizing_eval(item);
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Object_Item *eo_item;
   Evas_Coord x_p, y_p, w_p, h_p, x2, y2, w2, h2, bw, bh;
   Elm_Widget_Smart_Data *hover;

   ELM_MENU_DATA_GET(obj, sd);

   if (!sd->parent) return;

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_MENU_ITEM_DATA_GET(eo_item, item);
        elm_layout_sizing_eval(VIEW(item));
     }

   evas_object_geometry_get(sd->location, NULL, NULL, &w_p, &h_p);
   _parent_geometry_get(sd, &x2, &y2, &w2, &h2);
   evas_object_geometry_get(sd->bx, NULL, NULL, &bw, &bh);

   x_p = sd->xloc;
   y_p = sd->yloc;

   if (efl_ui_mirrored_get(obj)) x_p -= w_p;

   if (x_p + bw > x2 + w2) x_p -= x_p + bw - (x2 + w2);
   if (x_p < x2) x_p = x2;

   if (y_p + h_p + bh > y2 + h2) y_p -= y_p + h_p + bh - (y2 + h2);
   if (y_p < y2) y_p = y2;

   efl_gfx_entity_geometry_set(sd->location, EINA_RECT(x_p, y_p, bw, h_p));
   evas_object_size_hint_min_set(sd->location, bw, h_p);
   evas_object_size_hint_max_set(sd->location, bw, h_p);
   elm_hover_target_set(sd->hv, sd->location);

   hover = efl_data_scope_get(sd->hv, EFL_UI_WIDGET_CLASS);
   edje_object_part_geometry_get(hover->resize_obj, "bottom", NULL,
                                 NULL, &bw, &bh);
   evas_object_size_hint_min_set(obj, bw, bh);

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_MENU_ITEM_DATA_GET(eo_item, item);
        if (item->submenu.open) _submenu_sizing_eval(item);
     }
}

EOLIAN static Eina_Error
_elm_menu_efl_ui_widget_theme_apply(Eo *obj, Elm_Menu_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   Eina_List *l, *_l, *_ll, *ll = NULL;
   Elm_Object_Item *eo_item;
   const char *s;
   char style[1024];

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   if (sd->menu_bar)
      snprintf(style, sizeof(style), "main_menu/%s", elm_widget_style_get(obj));
   else
      snprintf(style, sizeof(style), "menu/%s", elm_widget_style_get(obj));
   elm_object_style_set(sd->hv, style);

   ll = eina_list_append(ll, sd->items);
   EINA_LIST_FOREACH(ll, _ll, l)
     {
        EINA_LIST_FOREACH(l, _l, eo_item)
          {
             ELM_MENU_ITEM_DATA_GET(eo_item, item);
             ll = eina_list_append(ll, item->submenu.items);
             if (item->separator)
               {
                  if (!elm_layout_theme_set(VIEW(item), "menu", "separator",
                                            elm_widget_style_get(obj)))
                    CRI("Failed to set layout!");
               }
             else if (item->submenu.bx)
               {
                  if (sd->menu_bar && !item->parent) s = "main_menu_submenu";
                  else s = "item_with_submenu";

                  if (!elm_layout_theme_set(VIEW(item), "menu", s,
                                            elm_widget_style_get(obj)))
                    CRI("Failed to set layout!");
                  snprintf(style, sizeof(style), "menu/%s", elm_widget_style_get(WIDGET(item)));
                  elm_object_style_set(item->submenu.hv, style);

                  elm_object_item_text_set(eo_item,
                                           item->label);
                  if (item->icon_str)
                    elm_menu_item_icon_name_set(eo_item,
                                                item->icon_str);
               }
             else
               {
                  if (!elm_layout_theme_set(VIEW(item), "menu", "item",
                                            elm_widget_style_get(obj)))
                    CRI("Failed to set layout!");

                  elm_object_item_text_set(eo_item,
                                           item->label);
                  if (item->icon_str)
                    elm_menu_item_icon_name_set(eo_item,
                                                item->icon_str);
               }
             elm_wdg_item_disable(eo_item);
             /* SEOZ
             edje_object_scale_set
               (VIEW(item), efl_gfx_entity_scale_get(obj) *
               elm_config_scale_get());
               */
          }
     }

   eina_list_free(ll); //fixme: test

   _sizing_eval(obj);

   return int_ret;
}

EOLIAN static void
_elm_menu_item_elm_widget_item_part_text_set(Eo *eo_item EINA_UNUSED,
                                             Elm_Menu_Item_Data *item,
                                             const char *part,
                                             const char *label)
{
   if (part && strcmp(part, "default")) return;

   eina_stringshare_replace(&item->label, label);

   if (label)
     elm_layout_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
   else
     elm_layout_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");

   edje_object_message_signal_process(elm_layout_edje_get(VIEW(item)));
   elm_layout_text_set(VIEW(item), "elm.text", label);

   _sizing_eval(WIDGET(item));
}

EOLIAN static const char *
_elm_menu_item_elm_widget_item_part_text_get(const Eo *eo_item EINA_UNUSED,
                                             Elm_Menu_Item_Data *it,
                                             const char *part)
{
   if (part && strcmp(part, "default")) return NULL;

   return it->label;
}

EOLIAN static void
_elm_menu_item_elm_widget_item_part_content_set(Eo *eo_item EINA_UNUSED,
                                                Elm_Menu_Item_Data *item,
                                                const char *part,
                                                Evas_Object *content)
{
   if (part && strcmp(part, "default")) return;

   if (content == item->content) return;

   evas_object_del(item->content);
   item->content = content;
   if (item->content)
     elm_layout_content_set(VIEW(item), "elm.swallow.content", item->content);

   _sizing_eval(WIDGET(item));
}

EOLIAN static Evas_Object *
_elm_menu_item_elm_widget_item_part_content_get(const Eo *eo_item EINA_UNUSED,
                                                Elm_Menu_Item_Data *it,
                                                const char *part)
{
   if (part && strcmp(part, "default")) return NULL;

   return it->content;
}

static void
_menu_resize_cb(void *data,
                Evas *e EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
}

static void
_parent_resize_cb(void *data,
                  Evas *e EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
}

static void
_parent_del_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info EINA_UNUSED)
{
   ELM_MENU_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _parent_resize_cb, wd->obj);
   sd->parent = NULL;
}

static void
_item_move_resize_cb(void *data,
                     Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   Elm_Menu_Item_Data *item = data;

   if (item->submenu.open) _submenu_sizing_eval(item);
}

static void
_menu_hide(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Eina_List *l;
   Elm_Object_Item *eo_item2;

   ELM_MENU_DATA_GET(data, sd);

   if (!sd->menu_bar)
     {
        evas_object_hide(sd->hv);
        evas_object_hide(data);
     }

   EINA_LIST_FOREACH(sd->items, l, eo_item2)
     {
        ELM_MENU_ITEM_DATA_GET(eo_item2, item2);
        if (item2->submenu.open) _submenu_hide(item2);
     }
}

static void
_hover_dismissed_cb(void *data, const Efl_Event *event)
{
   _menu_hide(data, event->object, event->info);
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_CLICKED, NULL);
   efl_event_callback_legacy_call(data, ELM_MENU_EVENT_DISMISSED, NULL);
}

static void
_submenu_open_cb(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   Elm_Menu_Item_Data *item = data;

   item->submenu.open = EINA_TRUE;
   evas_object_show(item->submenu.hv);
   _submenu_sizing_eval(item);
}

void
_elm_dbus_menu_item_select_cb(Elm_Object_Item *obj_item)
{
  ELM_MENU_ITEM_DATA_GET(obj_item, item);
  if (item->func) item->func((void *)(WIDGET_ITEM_DATA_GET(EO_OBJ(item))), WIDGET(item), obj_item);
}

static void
_menu_item_select_cb(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   Elm_Menu_Item_Data *item = data;

   if (item->submenu.items)
     {
        if (!item->submenu.open) _submenu_open_cb(item, NULL, NULL, NULL);
        else _submenu_hide(item);
     }
   else _menu_hide(WIDGET(item), NULL, NULL);

   if (item->func) item->func((void *)(WIDGET_ITEM_DATA_GET(EO_OBJ(item))), WIDGET(item), EO_OBJ(item));
}

static void
_menu_item_activate_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   Eina_List *l;
   Elm_Object_Item *eo_item2;
   Elm_Menu_Item_Data *item = data;

   item->selected = 1;
   if (item->parent)
     {
        EINA_LIST_FOREACH(item->parent->submenu.items, l, eo_item2)
          {
             if (eo_item2 != EO_OBJ(item))
               elm_menu_item_selected_set(eo_item2, 0);
          }
        efl_access_object_event_emit(EO_OBJ(item->parent), EFL_ACCESS_SELECTION_EVENT_SELECTION_CHANGED, NULL);
     }
   else
     {
        Eina_Bool was_open = EINA_FALSE;
        ELM_MENU_DATA_GET(WIDGET(item), sd);
        EINA_LIST_FOREACH(sd->items, l, eo_item2)
          {
             if (eo_item2 != EO_OBJ(item))
               {
                  ELM_MENU_ITEM_DATA_GET(eo_item2, item2);
                  was_open |= item2->submenu.open;
                  elm_menu_item_selected_set(eo_item2, 0);
               }
          }
        efl_access_object_event_emit(WIDGET(item), EFL_ACCESS_SELECTION_EVENT_SELECTION_CHANGED, NULL);
        if (sd->menu_bar && was_open)
          _menu_item_select_cb(item, NULL, NULL, NULL);
     }
   if (_elm_config->atspi_mode)
     efl_access_state_changed_signal_emit(EO_OBJ(item), EFL_ACCESS_STATE_TYPE_SELECTED, EINA_TRUE);
}

static void
_menu_item_inactivate_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   Elm_Menu_Item_Data *item = data;

   item->selected = 0;
   if (item->submenu.open) _submenu_hide(item);
   if (_elm_config->atspi_mode)
     efl_access_state_changed_signal_emit(EO_OBJ(item), EFL_ACCESS_STATE_TYPE_SELECTED, EINA_FALSE);
}

static void
_block_menu(void *_sd, const Efl_Event *event EINA_UNUSED)
{
   const Eina_List *l;
   Elm_Object_Item *eo_current;
   Elm_Menu_Data *sd = _sd;
   Eina_List *items = sd->items;
   EINA_LIST_FOREACH(items, l, eo_current)
     {
        ELM_MENU_ITEM_DATA_GET(eo_current, current);
        if (!current->blocked) current->was_enabled = !elm_wdg_item_disabled_get(eo_current);
        current->blocked = EINA_TRUE;
        elm_object_item_disabled_set(eo_current, EINA_TRUE);
     }
}

static void
_unblock_menu(void *_sd, const Efl_Event *event EINA_UNUSED)
{
   const Eina_List *l;
   Elm_Object_Item *eo_current;
   Elm_Menu_Data *sd = _sd;
   Eina_List *items = sd->items;
   EINA_LIST_FOREACH(items, l, eo_current)
     {
        ELM_MENU_ITEM_DATA_GET(eo_current, current);
        elm_object_item_disabled_set(eo_current, !current->was_enabled);
        current->blocked = EINA_FALSE;
     }
}

EOLIAN static void
_elm_menu_efl_gfx_entity_visible_set(Eo *obj EINA_UNUSED, Elm_Menu_Data *sd, Eina_Bool vis)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 0, vis))
     return;

   efl_gfx_entity_visible_set(efl_super(obj, MY_CLASS), vis);
   if (vis) efl_gfx_entity_visible_set(sd->hv, EINA_TRUE);
}

static void
_item_obj_create(Elm_Menu_Item_Data *item)
{
   VIEW_SET(item, elm_layout_add(WIDGET(item)));
   efl_access_object_access_type_set(VIEW(item), EFL_ACCESS_TYPE_SKIPPED);
   evas_object_size_hint_weight_set
     (VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
   if (!elm_layout_theme_set(VIEW(item), "menu", "item",
                        elm_widget_style_get(WIDGET(item))))
     CRI("Failed to set layout!");
   else
     {
        elm_layout_signal_callback_add(VIEW(item), "elm,action,click", "*",
                                       _menu_item_select_cb, item);
        elm_layout_signal_callback_add(VIEW(item), "elm,action,activate", "*",
                                       _menu_item_activate_cb, item);
        elm_layout_signal_callback_add(VIEW(item), "elm,action,inactivate", "*",
                                       _menu_item_inactivate_cb,
                                       item);
        evas_object_show(VIEW(item));
     }
}

static void
_item_separator_obj_create(Elm_Menu_Item_Data *item)
{
   VIEW_SET(item, elm_layout_add(WIDGET(item)));
   efl_access_object_access_type_set(VIEW(item), EFL_ACCESS_TYPE_SKIPPED);
   evas_object_size_hint_weight_set
     (VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
   if (!elm_layout_theme_set(VIEW(item), "menu", "separator",
                             elm_widget_style_get(WIDGET(item))))
     CRI("Failed to set layout!");
   else
     {
        elm_layout_signal_callback_add
           (VIEW(item), "elm,action,activate", "*", _menu_item_activate_cb, item);
        evas_object_show(VIEW(item));
     }
}

static void
_item_submenu_obj_create(Elm_Menu_Item_Data *item)
{
   ELM_MENU_DATA_GET(WIDGET(item), sd);
   Evas_Object *hv, *bx;
   char style[1024];

   item->submenu.location = elm_icon_add(sd->bx);
   efl_access_object_access_type_set(item->submenu.location, EFL_ACCESS_TYPE_DISABLED);
   item->submenu.hv = hv = elm_hover_add(sd->bx);
   efl_access_object_access_type_set(item->submenu.hv, EFL_ACCESS_TYPE_SKIPPED);
   efl_ui_mirrored_set(hv, EINA_FALSE);
   elm_hover_target_set(hv, item->submenu.location);
   elm_hover_parent_set(hv, sd->parent);

   if (sd->menu_bar && !item->parent)
     {
        snprintf(style, sizeof(style), "main_menu_submenu/%s", elm_widget_style_get(WIDGET(item)));
        elm_object_style_set(hv, style);
        efl_event_callback_add
          (hv, ELM_HOVER_EVENT_DISMISSED, _hover_dismissed_cb, WIDGET(item));
     }
   else
     {
        snprintf(style, sizeof(style), "submenu/%s", elm_widget_style_get(WIDGET(item)));
        elm_object_style_set(hv, style);
     }

   item->submenu.bx = bx = elm_box_add(sd->bx);
   efl_ui_mirrored_set(bx, EINA_FALSE);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bx);
   elm_object_part_content_set
      (hv, elm_hover_best_content_location_get
       (hv, ELM_HOVER_AXIS_VERTICAL), bx);

   if (sd->menu_bar && !item->parent)
     {
        if (!elm_layout_theme_set(VIEW(item), "menu",
                                  "main_menu_submenu",
                                  elm_widget_style_get(WIDGET(item))))
          CRI("Failed to set layout!");
     }
   else
     {
        if (!elm_layout_theme_set(VIEW(item), "menu",
                                  "item_with_submenu",
                                  elm_widget_style_get(WIDGET(item))))
          CRI("Failed to set layout!");
     }

   elm_object_item_text_set(EO_OBJ(item), item->label);

   if (item->icon_str)
     elm_menu_item_icon_name_set(EO_OBJ(item), item->icon_str);

   elm_layout_signal_callback_add(VIEW(item), "elm,action,open", "*",
                                   _submenu_open_cb, item);
   evas_object_event_callback_add
     (VIEW(item), EVAS_CALLBACK_MOVE, _item_move_resize_cb, item);
   evas_object_event_callback_add
     (VIEW(item), EVAS_CALLBACK_RESIZE, _item_move_resize_cb, item);

   evas_object_event_callback_add(bx, EVAS_CALLBACK_RESIZE,
                                  _menu_resize_cb, WIDGET(item));
}

EOLIAN static void
_elm_menu_efl_canvas_group_group_add(Eo *obj, Elm_Menu_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->location = elm_icon_add(obj);
   efl_access_object_access_type_set(priv->location, EFL_ACCESS_TYPE_DISABLED);

   priv->hv = elm_hover_add(obj);
   efl_access_object_access_type_set(priv->hv, EFL_ACCESS_TYPE_SKIPPED);
   efl_ui_mirrored_set(priv->hv, EINA_FALSE);

   elm_object_style_set(priv->hv, "menu/default");
   efl_event_callback_add
     (priv->hv, ELM_HOVER_EVENT_DISMISSED, _hover_dismissed_cb, obj);

   priv->bx = elm_box_add(obj);
   efl_access_object_access_type_set(priv->bx, EFL_ACCESS_TYPE_SKIPPED);
   efl_ui_mirrored_set(priv->bx, EINA_FALSE);
   evas_object_size_hint_weight_set
     (priv->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_event_callback_add
     (priv->bx, EVAS_CALLBACK_RESIZE, _menu_resize_cb, obj);
}

EOLIAN static void
_elm_menu_efl_canvas_group_group_del(Eo *obj, Elm_Menu_Data *sd)
{
   Elm_Object_Item *eo_item;

   _elm_dbus_menu_unregister(obj);

   if (sd->parent)
     {
        evas_object_event_callback_del_full
           (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
        evas_object_event_callback_del_full
           (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, obj);
     }

   EINA_LIST_FREE(sd->items, eo_item)
     _item_del(eo_item);

   evas_object_event_callback_del_full
      (sd->bx, EVAS_CALLBACK_RESIZE, _menu_resize_cb, obj);

   evas_object_del(sd->hv);
   evas_object_del(sd->location);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

void
_elm_menu_menu_bar_hide(Eo *obj)
{
  ELM_MENU_DATA_GET_OR_RETURN(obj, sd);

  evas_object_hide(sd->hv);
  evas_object_hide(obj);
  _menu_hide(obj, NULL, NULL);
}

void
_elm_menu_menu_bar_set(Eo *obj, Eina_Bool menu_bar)
{
   Eina_List *l;
   Elm_Object_Item *eo_item;
   char style[1024];

   ELM_MENU_DATA_GET_OR_RETURN(obj, sd);

   if (menu_bar == sd->menu_bar) return;

   elm_box_horizontal_set(sd->bx, menu_bar);
   elm_box_homogeneous_set(sd->bx, !menu_bar);
   sd->menu_bar = menu_bar;

   if (sd->menu_bar)
     snprintf(style, sizeof(style), "main_menu/%s", elm_widget_style_get(obj));
   else
     snprintf(style, sizeof(style), "menu/%s", elm_widget_style_get(obj));
   elm_object_style_set(sd->hv, style);

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_MENU_ITEM_DATA_GET(eo_item, item);
        if (!item->submenu.bx) continue;

        if (menu_bar)
          {
             efl_event_callback_add
               (item->submenu.hv, EFL_UI_EVENT_CLICKED, _hover_dismissed_cb, WIDGET(item));
             snprintf(style, sizeof(style), "main_menu_submenu//%s", elm_widget_style_get(obj));
             elm_object_style_set(item->submenu.hv, style);
          }
        else
          {
             efl_event_callback_del(item->submenu.hv, EFL_UI_EVENT_CLICKED, _hover_dismissed_cb, WIDGET(item));
             snprintf(style, sizeof(style), "submenu/%s", elm_widget_style_get(obj));
             elm_object_style_set(item->submenu.hv, style);
          }
     }

   efl_ui_widget_theme_apply(obj);
}

EAPI Evas_Object *
elm_menu_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Efl_Ui_Focus_Manager*
_elm_menu_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj EINA_UNUSED, Elm_Menu_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   Efl_Ui_Focus_Manager *manager;

   manager = efl_add(EFL_UI_FOCUS_MANAGER_CALC_CLASS, obj,
     efl_ui_focus_manager_root_set(efl_added, root)
   );

   return manager;
}

EOLIAN static Eo *
_elm_menu_efl_object_constructor(Eo *obj, Elm_Menu_Data *sd)
{
   Eo *parent = NULL;

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   parent = efl_parent_get(obj);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_MENU);

   elm_menu_parent_set(obj, parent);
   elm_hover_target_set(sd->hv, sd->location);
   elm_layout_content_set
     (sd->hv, elm_hover_best_content_location_get
       (sd->hv, ELM_HOVER_AXIS_VERTICAL), sd->bx);

   _sizing_eval(obj);
   efl_event_callback_add
     (obj, ELM_MENU_EVENT_ELM_ACTION_BLOCK_MENU, _block_menu, sd);
   efl_event_callback_add
     (obj, ELM_MENU_EVENT_ELM_ACTION_UNBLOCK_MENU, _unblock_menu, sd);

   sd->obj = obj;
   return obj;
}

EOLIAN static void
_elm_menu_efl_object_destructor(Eo *obj, Elm_Menu_Data *sd)
{
   Eina_List *itr, *itr2;
   Elm_Object_Item *eo_item;
   EINA_LIST_FOREACH_SAFE(sd->items, itr, itr2, eo_item)
     efl_del(eo_item);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EAPI void
elm_menu_parent_set(Evas_Object *obj,
                    Evas_Object *parent)
{
   ELM_MENU_CHECK(obj);
   efl_ui_widget_parent_set(obj, parent);
}

EOLIAN static void
_elm_menu_efl_ui_widget_widget_parent_set(Eo *obj, Elm_Menu_Data *sd, Evas_Object *parent)
{
   Eina_List *l, *_l, *_ll, *ll = NULL;
   Elm_Object_Item *eo_item;

   if (sd->parent == parent) return;
   if (sd->parent)
     {
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, obj);
     }
   sd->parent = parent;
   if (sd->parent)
     {
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, obj);
     }
   elm_hover_parent_set(sd->hv, parent);

   ll = eina_list_append(ll, sd->items);
   EINA_LIST_FOREACH(ll, _ll, l)
     {
        EINA_LIST_FOREACH(l, _l, eo_item)
          {
             ELM_MENU_ITEM_DATA_GET(eo_item, item);
             if (item->submenu.hv)
               {
                  elm_hover_parent_set(item->submenu.hv, parent);
                  ll = eina_list_append(ll, item->submenu.items);
               }
          }
     }

   eina_list_free(ll);

   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_menu_parent_get(const Evas_Object *obj)
{
   ELM_MENU_CHECK(obj) NULL;
   return efl_ui_widget_parent_get(obj);
}

EOLIAN static Evas_Object*
_elm_menu_efl_ui_widget_widget_parent_get(const Eo *obj EINA_UNUSED, Elm_Menu_Data *sd)
{
   return sd->parent;
}

EOLIAN static void
_elm_menu_relative_move(Eo *obj, Elm_Menu_Data *sd, Evas_Coord x, Evas_Coord y)
{
   sd->xloc = x;
   sd->yloc = y;
   _sizing_eval(obj);
}

EOLIAN static void
_elm_menu_open(Eo *obj, Elm_Menu_Data *sd EINA_UNUSED)
{
   evas_object_show(obj);
}

EOLIAN static void
_elm_menu_close(Eo *obj, Elm_Menu_Data *sd)
{
   _menu_hide(obj, sd->hv, NULL);
}

EOLIAN static Evas_Object *
_elm_menu_item_object_get(const Eo *eo_it EINA_UNUSED, Elm_Menu_Item_Data *it)
{
   return VIEW(it);
}

static void
_item_clone(Evas_Object *obj,
            Elm_Object_Item *parent,
            Elm_Object_Item *eo_item)
{
   Elm_Object_Item *new_item;
   Elm_Object_Item *subitem;
   Eina_List *iter;

   ELM_MENU_ITEM_DATA_GET(eo_item, item);
   if (item->separator)
     new_item = elm_menu_item_separator_add(obj, parent);
   else
     new_item = elm_menu_item_add(obj,
                                  parent,
                                  item->icon_str,
                                  item->label,
                                  item->func,
                                  WIDGET_ITEM_DATA_GET(EO_OBJ(item)));

   Eina_Bool disabled;
   disabled = elm_wdg_item_disabled_get(eo_item);
   elm_wdg_item_disabled_set(new_item, disabled);

   EINA_LIST_FOREACH(item->submenu.items, iter, subitem)
     _item_clone(obj, new_item, subitem);
}

void
elm_menu_clone(Evas_Object *from_menu,
               Evas_Object *to_menu,
               Elm_Object_Item *parent)
{
   Eina_List *iter;
   Elm_Object_Item *eo_item;

   ELM_MENU_CHECK(from_menu);
   ELM_MENU_CHECK(to_menu);

   ELM_MENU_DATA_GET_OR_RETURN(from_menu, from_sd);

   EINA_LIST_FOREACH(from_sd->items, iter, eo_item)
     _item_clone(to_menu, parent, eo_item);
}

static void
_elm_menu_item_add_helper(Evas_Object *obj,
                          Elm_Menu_Item_Data *parent,
                          Elm_Menu_Item_Data *subitem,
                          Elm_Menu_Data *sd)
{
   if (parent)
     {
        if (!parent->submenu.bx) _item_submenu_obj_create(parent);
        elm_box_pack_end(parent->submenu.bx, VIEW(subitem));
        parent->submenu.items =
          eina_list_append(parent->submenu.items, EO_OBJ(subitem));
        subitem->idx = eina_list_count(parent->submenu.items) - 1;
     }
   else
     {
        elm_box_pack_end(sd->bx, VIEW(subitem));
        sd->items = eina_list_append(sd->items, EO_OBJ(subitem));
        subitem->idx = eina_list_count(sd->items) - 1;
     }

   _sizing_eval(obj);
}

EOLIAN static void
_elm_menu_item_efl_object_destructor(Eo *eo_item, Elm_Menu_Item_Data *item)
{
   ELM_MENU_DATA_GET(WIDGET(item), sd);

   elm_menu_item_subitems_clear(eo_item);
   eina_stringshare_del(item->label);
   eina_stringshare_del(item->icon_str);
   evas_object_del(item->content);
   evas_object_del(item->submenu.hv);
   evas_object_del(item->submenu.location);

   if (item->parent)
     item->parent->submenu.items =
       eina_list_remove(item->parent->submenu.items, eo_item);
   else
     sd->items = eina_list_remove(sd->items, eo_item);

   if (sd->dbus_menu)
     _elm_dbus_menu_item_delete(sd->dbus_menu, item->dbus_idx);

   efl_destructor(efl_super(eo_item, ELM_MENU_ITEM_CLASS));
}

EOLIAN static Eo *
_elm_menu_item_efl_object_constructor(Eo *eo_item, Elm_Menu_Item_Data *item)
{
   eo_item = efl_constructor(efl_super(eo_item, ELM_MENU_ITEM_CLASS));
   item->base = efl_data_scope_get(eo_item, ELM_WIDGET_ITEM_CLASS);

   return eo_item;
}

EOLIAN static Elm_Object_Item*
_elm_menu_item_add(Eo *obj, Elm_Menu_Data *sd, Elm_Object_Item *parent, const char *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   Elm_Object_Item *eo_item;
   Evas_Object *icon_obj;

   icon_obj = elm_icon_add(obj);
   efl_access_object_access_type_set(icon_obj, EFL_ACCESS_TYPE_DISABLED);
   if (!icon_obj) return NULL;

   eo_item = efl_add(ELM_MENU_ITEM_CLASS, obj);
   if (!eo_item)
     {
        evas_object_del(icon_obj);
        return NULL;
     }

   ELM_MENU_ITEM_DATA_GET(eo_item, it);

   WIDGET_ITEM_DATA_SET(eo_item, data);
   it->func = func;
   it->parent = efl_data_scope_get(parent, ELM_MENU_ITEM_CLASS);
   it->content = icon_obj;

   _item_obj_create(it);
   efl_canvas_object_is_frame_object_set(VIEW(it), efl_canvas_object_is_frame_object_get(obj));
   efl_canvas_object_is_frame_object_set(icon_obj, efl_canvas_object_is_frame_object_get(obj));
   elm_object_item_text_set(eo_item, label);

   elm_layout_content_set(VIEW(it), "elm.swallow.content",
                          it->content);

   if (icon) elm_menu_item_icon_name_set(eo_item, icon);

   _elm_menu_item_add_helper(obj, it->parent, it, sd);

   if (sd->dbus_menu)
   {
     it->dbus_idx = _elm_dbus_menu_item_add(sd->dbus_menu, eo_item);
     it->dbus_menu = sd->dbus_menu;
   }
   if (_elm_config->atspi_mode)
     {
        efl_access_added(eo_item);
        efl_access_children_changed_added_signal_emit(parent ? parent : obj, eo_item);
     }

   return eo_item;
}

EOLIAN static unsigned int
_elm_menu_item_index_get(const Eo *eo_it EINA_UNUSED, Elm_Menu_Item_Data *it)
{
   return it->idx;
}

EOLIAN static void
_elm_menu_item_icon_name_set(Eo *eo_item EINA_UNUSED,
                             Elm_Menu_Item_Data *item,
                             const char *icon)
{
   char icon_tmp[512];

   EINA_SAFETY_ON_NULL_RETURN(icon);

   if ((icon[0] != '\0') &&
       (snprintf(icon_tmp, sizeof(icon_tmp), "menu/%s", icon) > 0) &&
       (elm_icon_standard_set(item->content, icon_tmp) ||
        elm_icon_standard_set(item->content, icon)))
     {
        eina_stringshare_replace(&item->icon_str, icon);
        elm_layout_signal_emit(VIEW(item), "elm,state,icon,visible", "elm");
     }
   else
     elm_layout_signal_emit(VIEW(item), "elm,state,icon,hidden", "elm");

   edje_object_message_signal_process(elm_layout_edje_get(VIEW(item)));
   _sizing_eval(WIDGET(item));
}

EOLIAN static Elm_Object_Item*
_elm_menu_item_separator_add(Eo *obj, Elm_Menu_Data *sd, Elm_Object_Item *eo_p_item)
{
   Elm_Object_Item *eo_subitem;

   ELM_MENU_ITEM_DATA_GET(eo_p_item, p_item);
   /* don't add a separator as the first item */
   if (!sd->items) return NULL;

   /* don't allow adding more than one separator in a row */
   if (eo_p_item)
     {
        if (!p_item->submenu.items) return NULL;
        eo_subitem = eina_list_last(p_item->submenu.items)->data;
     }
   else eo_subitem = eina_list_last(sd->items)->data;

   ELM_MENU_ITEM_DATA_GET(eo_subitem, subitem);
   if (subitem->separator) return NULL;

   eo_subitem = efl_add(ELM_MENU_ITEM_CLASS, obj);
   if (!eo_subitem) return NULL;

   subitem = efl_data_scope_get(eo_subitem, ELM_MENU_ITEM_CLASS);

   subitem->separator = EINA_TRUE;
   _item_separator_obj_create(subitem);
   if (!eo_p_item)
     {
        elm_box_pack_end(sd->bx, VIEW(subitem));
        sd->items = eina_list_append(sd->items, eo_subitem);
     }
   else
     {
        if (!p_item->submenu.bx) _item_submenu_obj_create(p_item);
        elm_box_pack_end(p_item->submenu.bx, VIEW(subitem));
        p_item->submenu.items = eina_list_append
            (p_item->submenu.items, eo_subitem);
     }

   _sizing_eval(obj);

   if (sd->dbus_menu)
     subitem->dbus_idx = _elm_dbus_menu_item_add(sd->dbus_menu,
                                                 eo_subitem);
   return eo_subitem;
}

EOLIAN static const char *
_elm_menu_item_icon_name_get(const Eo *eo_item EINA_UNUSED, Elm_Menu_Item_Data *item)
{
   return item->icon_str;
}

EOLIAN static Eina_Bool
_elm_menu_item_is_separator(const Eo *eo_item EINA_UNUSED, Elm_Menu_Item_Data *item)
{
   return item->separator;
}

EOLIAN static const Eina_List *
_elm_menu_item_subitems_get(const Eo *eo_item EINA_UNUSED, Elm_Menu_Item_Data *item)
{
   return item->submenu.items;
}

EOLIAN static void
_elm_menu_item_subitems_clear(Eo *eo_item EINA_UNUSED, Elm_Menu_Item_Data *it)
{
   Elm_Object_Item *sub_it;
   Eina_List *l, *l_next;

   EINA_LIST_FOREACH_SAFE(it->submenu.items,
                          l, l_next, sub_it)
     efl_del(sub_it);
}

EOLIAN static const Eina_List *
_elm_menu_items_get(const Eo *obj EINA_UNUSED, Elm_Menu_Data *sd)
{
   return sd->items;
}

EOLIAN static void
_elm_menu_item_selected_set(Eo *eo_item EINA_UNUSED,
                            Elm_Menu_Item_Data *item,
                            Eina_Bool selected)
{
   if (selected == item->selected) return;
   item->selected = selected;
   if (selected)
     {
        elm_layout_signal_emit(VIEW(item), "elm,state,selected", "elm");
        _menu_item_activate_cb(item, NULL, NULL, NULL);
     }
   else
     {
        elm_layout_signal_emit(VIEW(item), "elm,state,unselected", "elm");
        _menu_item_inactivate_cb(item, NULL, NULL, NULL);
     }
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(item)));
}

EOLIAN static Eina_Bool
_elm_menu_item_selected_get(const Eo *eo_item EINA_UNUSED, Elm_Menu_Item_Data *item)
{
   return item->selected;
}

EOLIAN static Elm_Object_Item *
_elm_menu_item_prev_get(const Eo *eo_item, Elm_Menu_Item_Data *item)
{
   if (item->parent)
     {
        Eina_List *l = eina_list_data_find_list
            (item->parent->submenu.items, eo_item);
        l = eina_list_prev(l);
        if (!l) return NULL;
        return l->data;
     }
   else
     {
        ELM_MENU_DATA_GET(WIDGET(item), sd);
        if (!sd || !sd->items) return NULL;
        Eina_List *l = eina_list_data_find_list(sd->items, eo_item);
        l = eina_list_prev(l);
        if (!l) return NULL;
        return l->data;
     }

   return NULL;
}

EOLIAN static Elm_Object_Item *
_elm_menu_item_next_get(const Eo *eo_item, Elm_Menu_Item_Data *item)
{
   if (item->parent)
     {
        Eina_List *l =
          eina_list_data_find_list(item->parent->submenu.items, eo_item);
        l = eina_list_next(l);
        if (!l) return NULL;
        return l->data;
     }
   else
     {
        ELM_MENU_DATA_GET(WIDGET(item), sd);
        if (!sd || !sd->items) return NULL;
        Eina_List *l = eina_list_data_find_list(sd->items, eo_item);
        l = eina_list_next(l);
        if (!l) return NULL;
        return l->data;
     }

   return NULL;
}

EOLIAN static Elm_Object_Item*
_elm_menu_first_item_get(const Eo *obj EINA_UNUSED, Elm_Menu_Data *sd)
{
   return (sd->items ? sd->items->data : NULL);
}

EOLIAN static Elm_Object_Item*
_elm_menu_last_item_get(const Eo *obj EINA_UNUSED, Elm_Menu_Data *sd)
{
   Eina_List *l = eina_list_last(sd->items);
   return (l ? l->data : NULL);
}

EOLIAN static Elm_Object_Item*
_elm_menu_selected_item_get(const Eo *obj EINA_UNUSED, Elm_Menu_Data *sd)
{
   Eina_List *l;
   Elm_Object_Item *eo_item;

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_MENU_ITEM_DATA_GET(eo_item, item);
        if (item->selected) return eo_item;
     }

   return NULL;
}

static void
_elm_menu_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eina_List*
_elm_menu_efl_access_object_access_children_get(const Eo *obj, Elm_Menu_Data *sd)
{
   Eina_List *ret;
   ret = efl_access_object_access_children_get(efl_super(obj, ELM_MENU_CLASS));
   return eina_list_merge(eina_list_clone(sd->items), ret);
}

EOLIAN static Eina_List*
_elm_menu_item_efl_access_object_access_children_get(const Eo *obj EINA_UNUSED, Elm_Menu_Item_Data *sd)
{
   return eina_list_clone(sd->submenu.items);
}

EOLIAN static Eo*
_elm_menu_item_efl_object_parent_get(const Eo *obj, Elm_Menu_Item_Data *sd)
{
   if (sd->parent) return EO_OBJ(sd->parent);
   if (sd->base) return WIDGET(sd);
   return efl_parent_get(efl_super(obj, ELM_MENU_ITEM_CLASS));
}

EOLIAN static Efl_Access_Role
_elm_menu_item_efl_access_object_role_get(const Eo *obj EINA_UNUSED, Elm_Menu_Item_Data *sd)
{
   return sd->submenu.items ? EFL_ACCESS_ROLE_MENU : EFL_ACCESS_ROLE_MENU_ITEM;
}

EOLIAN static const char*
_elm_menu_item_efl_access_object_i18n_name_get(const Eo *obj, Elm_Menu_Item_Data *sd)
{
   const char *ret;
   ret = efl_access_object_i18n_name_get(efl_super(obj, ELM_MENU_ITEM_CLASS));
   if (ret) return ret;
   return sd->label;
}

EOLIAN static Efl_Access_State_Set
_elm_menu_item_efl_access_object_state_set_get(const Eo *obj EINA_UNUSED, Elm_Menu_Item_Data *sd)
{
   Efl_Access_State_Set ret;
   ret = efl_access_object_state_set_get(efl_super(obj, ELM_MENU_ITEM_CLASS));

   STATE_TYPE_SET(ret, EFL_ACCESS_STATE_TYPE_SELECTABLE);

   if (sd->selected)
      STATE_TYPE_SET(ret, EFL_ACCESS_STATE_TYPE_SELECTED);

   return ret;
}

EOLIAN static int
_elm_menu_item_efl_access_selection_selected_children_count_get(const Eo *obj EINA_UNUSED, Elm_Menu_Item_Data *sd)
{
   int ret = 0;
   Elm_Object_Item *sobj = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->submenu.items, l, sobj)
     {
        if (!sobj) continue;
        ELM_MENU_ITEM_DATA_GET(sobj, item);
        if (item && item->selected) ret++;
     }

   return ret;
}

EOLIAN static Eo*
_elm_menu_item_efl_access_selection_selected_child_get(const Eo *obj EINA_UNUSED, Elm_Menu_Item_Data *sd, int child)
{
   int seq = 0;
   Elm_Object_Item *sobj = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->submenu.items, l, sobj)
     {
        if (!sobj) continue;
        ELM_MENU_ITEM_DATA_GET(sobj, item);

        if (item && item->selected)
          {
             if (child == seq++)
               break;
          }
     }

   return sobj;
}

EOLIAN static int
_elm_menu_efl_access_selection_selected_children_count_get(const Eo *obj EINA_UNUSED, Elm_Menu_Data *sd)
{
   Elm_Object_Item *sobj = NULL;
   Eina_List *l;
   int ret = 0;

   EINA_LIST_FOREACH(sd->items, l, sobj)
     {
        ELM_MENU_ITEM_DATA_GET(sobj, item);
        if (item && item->selected) ret++;
     }

   return ret;
}

EOLIAN static Eo*
_elm_menu_efl_access_selection_selected_child_get(const Eo *obj EINA_UNUSED, Elm_Menu_Data *sd, int child)
{
   int seq = 0;
   Elm_Object_Item *sobj = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, sobj)
     {
        if (!sobj) continue;
        ELM_MENU_ITEM_DATA_GET(sobj, item);

        if (item && item->selected)
          {
             if (child == seq++)
               break;
          }
     }

   return sobj;

}

EOLIAN static Efl_Object*
_elm_menu_efl_object_provider_find(const Eo *obj, Elm_Menu_Data *pd, const Efl_Object *klass)
{
   if (!pd->parent)
     return efl_provider_find(efl_super(obj, MY_CLASS), klass);

   return efl_provider_find(pd->parent, klass);
}



/* Internal EO APIs and hidden overrides */

#define ELM_MENU_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_menu)

#include "elm_menu_item_eo.c"
#include "elm_menu_eo.c"
