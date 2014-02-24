#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_menu.h"

EAPI Eo_Op ELM_OBJ_MENU_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_MENU_CLASS

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

static void
_elm_menu_smart_translate(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Menu_Smart_Data *sd = _pd;
   Elm_Menu_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_widget_item_translate(it);

   if (ret) *ret = EINA_TRUE;
}

static void
_item_del(Elm_Menu_Item *item)
{
   Elm_Menu_Item *child;

   EINA_LIST_FREE(item->submenu.items, child)
     _item_del(child);

   eina_stringshare_del(item->label);
   evas_object_del(item->submenu.hv);
   evas_object_del(item->submenu.location);
   eina_stringshare_del(item->icon_str);

   elm_widget_item_free(item);
}

static void
_submenu_hide(Elm_Menu_Item *item)
{
   Eina_List *l;
   Elm_Menu_Item *item2;

   evas_object_hide(item->submenu.hv);
   item->submenu.open = EINA_FALSE;

   EINA_LIST_FOREACH(item->submenu.items, l, item2)
     {
        if (item2->submenu.open) _submenu_hide(item2);
     }
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   Elm_Menu_Item *item = (Elm_Menu_Item *)it;

   if (elm_widget_item_disabled_get(item))
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
_submenu_sizing_eval(Elm_Menu_Item *parent_it)
{
   Eina_List *l;
   Elm_Menu_Item *item;
   Evas_Coord x_p, y_p, w_p, h_p, x2, y2, w2,
              h2, bx, by, bw, bh, px, py, pw, ph;
   ELM_MENU_DATA_GET_OR_RETURN(WIDGET(parent_it), sd);

   EINA_LIST_FOREACH(parent_it->submenu.items, l, item)
     elm_layout_sizing_eval(VIEW(item));

   evas_object_geometry_get
     (parent_it->submenu.location, &x_p, &y_p, &w_p, &h_p);
   evas_object_geometry_get(VIEW(parent_it), &x2, &y2, &w2, &h2);
   evas_object_geometry_get(parent_it->submenu.bx, &bx, &by, &bw, &bh);
   evas_object_geometry_get(sd->parent, &px, &py, &pw, &ph);

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
   if ((x_p + bw > px + pw) || elm_widget_mirrored_get(WIDGET(parent_it)))
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

   evas_object_move(parent_it->submenu.location, x_p, y_p);
   evas_object_resize(parent_it->submenu.location, bw, h_p);
   evas_object_size_hint_min_set(parent_it->submenu.location, bw, h_p);
   evas_object_size_hint_max_set(parent_it->submenu.location, bw, h_p);
   elm_hover_target_set(parent_it->submenu.hv, parent_it->submenu.location);

   EINA_LIST_FOREACH(parent_it->submenu.items, l, item)
     {
        if (item->submenu.open) _submenu_sizing_eval(item);
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Menu_Item *item;
   Evas_Coord x_p, y_p, w_p, h_p, x2, y2, w2, h2, bw, bh;
   Elm_Widget_Smart_Data *hover;

   ELM_MENU_DATA_GET(obj, sd);

   if (!sd->parent) return;

   EINA_LIST_FOREACH(sd->items, l, item)
     elm_layout_sizing_eval(VIEW(item));

   evas_object_geometry_get(sd->location, NULL, NULL, &w_p, &h_p);
   evas_object_geometry_get(sd->parent, &x2, &y2, &w2, &h2);
   evas_object_geometry_get(sd->bx, NULL, NULL, &bw, &bh);

   x_p = sd->xloc;
   y_p = sd->yloc;

   if (elm_widget_mirrored_get(obj)) x_p -= w_p;

   if (x_p + bw > x2 + w2) x_p -= x_p + bw - (x2 + w2);
   if (x_p < x2) x_p = x2;

   if (y_p + h_p + bh > y2 + h2) y_p -= y_p + h_p + bh - (y2 + h2);
   if (y_p < y2) y_p = y2;

   evas_object_move(sd->location, x_p, y_p);
   evas_object_resize(sd->location, bw, h_p);
   evas_object_size_hint_min_set(sd->location, bw, h_p);
   evas_object_size_hint_max_set(sd->location, bw, h_p);
   elm_hover_target_set(sd->hv, sd->location);

   hover = eo_data_scope_get(sd->hv, ELM_OBJ_WIDGET_CLASS);
   edje_object_part_geometry_get(hover->resize_obj, "bottom", NULL,
                                 NULL, &bw, &bh);
   evas_object_size_hint_min_set(obj, bw, bh);

   EINA_LIST_FOREACH(sd->items, l, item)
     if (item->submenu.open) _submenu_sizing_eval(item);
}

static void
_elm_menu_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Menu_Smart_Data *sd = _pd;

   Eina_List *l, *_l, *_ll, *ll = NULL;
   Elm_Menu_Item *item;
   const char *s;
   char style[1024];

   eo_do_super(obj, MY_CLASS, elm_wdg_theme_apply(&int_ret));
   if (!int_ret) return;

   if (sd->menu_bar)
      snprintf(style, sizeof(style), "main_menu/%s", elm_widget_style_get(obj));
   else
      snprintf(style, sizeof(style), "menu/%s", elm_widget_style_get(obj));
   elm_object_style_set(sd->hv, style);

   ll = eina_list_append(ll, sd->items);
   EINA_LIST_FOREACH(ll, _ll, l)
     {
        EINA_LIST_FOREACH(l, _l, item)
          {
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

                  elm_object_item_text_set((Elm_Object_Item *)item,
                                           item->label);
                  if (item->icon_str)
                    elm_menu_item_icon_name_set((Elm_Object_Item *)item,
                                                item->icon_str);
               }
             else
               {
                  if (!elm_layout_theme_set(VIEW(item), "menu", "item",
                                            elm_widget_style_get(obj)))
                    CRI("Failed to set layout!");

                  elm_object_item_text_set((Elm_Object_Item *)item,
                                           item->label);
                  if (item->icon_str)
                    elm_menu_item_icon_name_set((Elm_Object_Item *)item,
                                                item->icon_str);
               }
             _item_disable_hook((Elm_Object_Item *)item);
             /* SEOZ
             edje_object_scale_set
               (VIEW(item), elm_widget_scale_get(obj) *
               elm_config_scale_get());
               */
          }
     }

   eina_list_free(ll); //fixme: test

   _sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Menu_Item *item;

   if (part && strcmp(part, "default")) return;

   item = (Elm_Menu_Item *)it;

   eina_stringshare_replace(&item->label, label);

   if (label)
     elm_layout_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
   else
     elm_layout_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");

   edje_object_message_signal_process(elm_layout_edje_get(VIEW(item)));
   elm_layout_text_set(VIEW(item), "elm.text", label);

   _sizing_eval(WIDGET(item));
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "default")) return NULL;

   return ((Elm_Menu_Item *)it)->label;
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Menu_Item *item;

   if (part && strcmp(part, "default")) return;

   item = (Elm_Menu_Item *)it;
   if (content == item->content) return;

   evas_object_del(item->content);
   item->content = content;
   if (item->content)
     elm_layout_content_set(VIEW(item), "elm.swallow.content", item->content);

   _sizing_eval(WIDGET(item));
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   if (part && strcmp(part, "default")) return NULL;

   return ((Elm_Menu_Item *)it)->content;
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
   Elm_Menu_Item *item = data;

   if (item->submenu.open) _submenu_sizing_eval(item);
}

static void
_menu_hide(void *data,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Eina_List *l;
   Elm_Menu_Item *item2;

   ELM_MENU_DATA_GET(data, sd);

   if (!sd->menu_bar)
     {
        evas_object_hide(sd->hv);
        evas_object_hide(data);
     }

   EINA_LIST_FOREACH(sd->items, l, item2)
     {
        if (item2->submenu.open) _submenu_hide(item2);
     }
}

static void
_hover_dismissed_cb(void *data,
                    Evas_Object *obj,
                    void *event_info)
{
   _menu_hide(data, obj, event_info);
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
   evas_object_smart_callback_call(data, SIG_DISMISSED, NULL);
}

static void
_submenu_open_cb(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   Elm_Menu_Item *item = data;

   item->submenu.open = EINA_TRUE;
   evas_object_show(item->submenu.hv);
   _submenu_sizing_eval(item);
}

void
_elm_dbus_menu_item_select_cb(Elm_Object_Item *obj_item)
{
  Elm_Menu_Item *item = (Elm_Menu_Item *)obj_item;

  if (item->func) item->func((void *)(item->base.data), WIDGET(item), item);
}

static void
_menu_item_select_cb(void *data,
                     Evas_Object *obj EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   Elm_Menu_Item *item = data;

   if (item->submenu.items)
     {
        if (!item->submenu.open) _submenu_open_cb(item, NULL, NULL, NULL);
        else _submenu_hide(item);
     }
   else _menu_hide(WIDGET(item), NULL, NULL);

   if (item->func) item->func((void *)(item->base.data), WIDGET(item), item);
}

static void
_menu_item_activate_cb(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   Eina_List *l;
   Elm_Menu_Item *item2;
   Elm_Menu_Item *item = data;

   item->selected = 1;
   if (item->parent)
     {
        EINA_LIST_FOREACH(item->parent->submenu.items, l, item2)
          {
             if (item2 != item)
               elm_menu_item_selected_set((Elm_Object_Item *)item2, 0);
          }
     }
   else
     {
        ELM_MENU_DATA_GET(WIDGET(item), sd);
        EINA_LIST_FOREACH(sd->items, l, item2)
          {
             if (item2 != item)
               elm_menu_item_selected_set((Elm_Object_Item *)item2, 0);
          }
     }
}

static void
_menu_item_inactivate_cb(void *data,
                         Evas_Object *obj EINA_UNUSED,
                         const char *emission EINA_UNUSED,
                         const char *source EINA_UNUSED)
{
   Elm_Menu_Item *item = data;

   item->selected = 0;
   if (item->submenu.open) _submenu_hide(item);
}

static void
_elm_menu_smart_show(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Menu_Smart_Data *sd = _pd;
   evas_object_show(sd->hv);
}

static void
_item_obj_create(Elm_Menu_Item *item)
{
   VIEW(item) = elm_layout_add(WIDGET(item));
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
_item_separator_obj_create(Elm_Menu_Item *item)
{
   VIEW(item) = elm_layout_add(WIDGET(item));
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
_item_submenu_obj_create(Elm_Menu_Item *item)
{
   ELM_MENU_DATA_GET(WIDGET(item), sd);
   Evas_Object *hv, *bx;
   char style[1024];

   item->submenu.location = elm_icon_add(sd->bx);
   item->submenu.hv = hv = elm_hover_add(sd->bx);
   elm_widget_mirrored_set(hv, EINA_FALSE);
   elm_hover_target_set(hv, item->submenu.location);
   elm_hover_parent_set(hv, sd->parent);

   if (sd->menu_bar && !item->parent)
     {
        snprintf(style, sizeof(style), "main_menu_submenu/%s", elm_widget_style_get(WIDGET(item)));
        elm_object_style_set(hv, style);
        evas_object_smart_callback_add(hv, "dismissed",
                                       _hover_dismissed_cb, WIDGET(item));
     }
   else
     {
        snprintf(style, sizeof(style), "submenu/%s", elm_widget_style_get(WIDGET(item)));
        elm_object_style_set(hv, style);
     }

   item->submenu.bx = bx = elm_box_add(sd->bx);
   elm_widget_mirrored_set(bx, EINA_FALSE);
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

   elm_object_item_text_set((Elm_Object_Item *)item, item->label);

   if (item->icon_str)
     elm_menu_item_icon_name_set((Elm_Object_Item *)item, item->icon_str);

   elm_layout_signal_callback_add(VIEW(item), "elm,action,open", "*",
                                   _submenu_open_cb, item);
   evas_object_event_callback_add
     (VIEW(item), EVAS_CALLBACK_MOVE, _item_move_resize_cb, item);
   evas_object_event_callback_add
     (VIEW(item), EVAS_CALLBACK_RESIZE, _item_move_resize_cb, item);

   evas_object_event_callback_add(bx, EVAS_CALLBACK_RESIZE,
                                  _menu_resize_cb, WIDGET(item));
}

static void
_elm_menu_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Menu_Smart_Data *priv = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->location = elm_icon_add(obj);

   priv->hv = elm_hover_add(obj);
   elm_widget_mirrored_set(priv->hv, EINA_FALSE);

   elm_object_style_set(priv->hv, "menu/default");
   evas_object_smart_callback_add(priv->hv, "dismissed",
                                  _hover_dismissed_cb, obj);

   priv->bx = elm_box_add(obj);
   elm_widget_mirrored_set(priv->bx, EINA_FALSE);
   evas_object_size_hint_weight_set
     (priv->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_event_callback_add
     (priv->bx, EVAS_CALLBACK_RESIZE, _menu_resize_cb, obj);
}

static void
_elm_menu_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Menu_Item *item;
   Elm_Menu_Smart_Data *sd = _pd;

   _elm_dbus_menu_unregister(obj);

   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, obj);

   EINA_LIST_FREE(sd->items, item)
     _item_del(item);

   evas_object_event_callback_del_full
      (sd->bx, EVAS_CALLBACK_RESIZE, _menu_resize_cb, obj);

   evas_object_del(sd->hv);
   evas_object_del(sd->location);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
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
   Elm_Menu_Item *item;
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

   EINA_LIST_FOREACH(sd->items, l, item)
     {
        if (!item->submenu.bx) continue;

        if (menu_bar)
          {
             evas_object_smart_callback_add(item->submenu.hv, "clicked",
                                            _hover_dismissed_cb, WIDGET(item));
             snprintf(style, sizeof(style), "main_menu_submenu//%s", elm_widget_style_get(obj));
             elm_object_style_set(item->submenu.hv, style);
          }
        else
          {
             evas_object_smart_callback_del_full(item->submenu.hv, "clicked",
                                                 _hover_dismissed_cb,
                                                 WIDGET(item));
             snprintf(style, sizeof(style), "submenu/%s", elm_widget_style_get(obj));
             elm_object_style_set(item->submenu.hv, style);
          }
     }

   eo_do(obj, elm_wdg_theme_apply(NULL));
}

EAPI Evas_Object *
elm_menu_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Menu_Smart_Data *sd = _pd;
   Eo *parent;

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL),
         eo_parent_get(&parent));

   elm_menu_parent_set(obj, parent);
   elm_hover_target_set(sd->hv, sd->location);
   elm_layout_content_set
     (sd->hv, elm_hover_best_content_location_get
       (sd->hv, ELM_HOVER_AXIS_VERTICAL), sd->bx);

   _sizing_eval(obj);
}

EAPI void
elm_menu_parent_set(Evas_Object *obj,
                    Evas_Object *parent)
{
   ELM_MENU_CHECK(obj);
   eo_do(obj, elm_wdg_parent_set(parent));
}

static void
_parent_set(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *parent = va_arg(*list, Evas_Object *);
   Elm_Menu_Smart_Data *sd = _pd;
   Eina_List *l, *_l, *_ll, *ll = NULL;
   Elm_Menu_Item *item;

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
        EINA_LIST_FOREACH(l, _l, item)
          {
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
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_wdg_parent_get(&ret));
   return ret;
}

static void
_parent_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Menu_Smart_Data *sd = _pd;
   *ret = sd->parent;
}

EAPI void
elm_menu_move(Evas_Object *obj,
              Evas_Coord x,
              Evas_Coord y)
{
   ELM_MENU_CHECK(obj);
   eo_do(obj, elm_obj_menu_move(x, y));
}

static void
_move(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Elm_Menu_Smart_Data *sd = _pd;

   sd->xloc = x;
   sd->yloc = y;
   _sizing_eval(obj);
}

EAPI void
elm_menu_close(Evas_Object *obj)
{
   ELM_MENU_CHECK(obj);
   eo_do(obj, elm_obj_menu_close());
}

static void
_menu_close(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Menu_Smart_Data *sd = _pd;
   _menu_hide(obj, sd->hv, NULL);
}

EAPI Evas_Object *
elm_menu_item_object_get(const Elm_Object_Item *it)
{
   return VIEW(((Elm_Menu_Item *)it));
}

static void
_item_clone(Evas_Object *obj,
            Elm_Menu_Item *parent,
            Elm_Menu_Item *item)
{
   Elm_Object_Item *new_item;
   Elm_Menu_Item *subitem;
   Eina_List *iter;

   if (item->separator)
     new_item = elm_menu_item_separator_add(obj, (Elm_Object_Item *)parent);
   else
     new_item = elm_menu_item_add(obj,
                                  (Elm_Object_Item *)parent,
                                  item->icon_str,
                                  item->label,
                                  item->func,
                                  item->base.data);

   elm_object_item_disabled_set
     (new_item, elm_widget_item_disabled_get(item));

   EINA_LIST_FOREACH(item->submenu.items, iter, subitem)
     _item_clone(obj, (Elm_Menu_Item *)new_item, subitem);
}

void
elm_menu_clone(Evas_Object *from_menu,
               Evas_Object *to_menu,
               Elm_Object_Item *parent)
{
   Eina_List *iter;
   Elm_Menu_Item *item;

   ELM_MENU_CHECK(from_menu);
   ELM_MENU_CHECK(to_menu);

   ELM_MENU_DATA_GET_OR_RETURN(from_menu, from_sd);

   EINA_LIST_FOREACH(from_sd->items, iter, item)
     _item_clone(to_menu, (Elm_Menu_Item *)parent, item);
}

static void
_elm_menu_item_add_helper(Evas_Object *obj,
                          Elm_Menu_Item *parent,
                          Elm_Menu_Item *subitem,
                          Elm_Menu_Smart_Data *sd)
{
   if (parent)
     {
        if (!parent->submenu.bx) _item_submenu_obj_create(parent);
        elm_box_pack_end(parent->submenu.bx, VIEW(subitem));
        parent->submenu.items =
          eina_list_append(parent->submenu.items, subitem);
        subitem->idx = eina_list_count(parent->submenu.items) - 1;
     }
   else
     {
        elm_box_pack_end(sd->bx, VIEW(subitem));
        sd->items = eina_list_append(sd->items, subitem);
        subitem->idx = eina_list_count(sd->items) - 1;
     }

   _sizing_eval(obj);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Menu_Item *item = (Elm_Menu_Item *)it;

   ELM_MENU_DATA_GET(WIDGET(item), sd);

   elm_menu_item_subitems_clear(it);
   eina_stringshare_del(item->label);
   evas_object_del(item->content);
   evas_object_del(item->submenu.hv);
   evas_object_del(item->submenu.location);

   if (item->parent)
     item->parent->submenu.items =
       eina_list_remove(item->parent->submenu.items, item);
   else
     sd->items = eina_list_remove(sd->items, item);

   if (sd->dbus_menu)
     _elm_dbus_menu_item_delete(sd->dbus_menu, item->dbus_idx);

   return EINA_TRUE;
}

EAPI Elm_Object_Item *
elm_menu_item_add(Evas_Object *obj,
                  Elm_Object_Item *parent,
                  const char *icon,
                  const char *label,
                  Evas_Smart_Cb func,
                  const void *data)
{
   ELM_MENU_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_menu_item_add(parent, icon, label, func, data, &ret));
   return ret;
}

static void
_item_add(Eo *obj, void *_pd, va_list *list)
{
   Elm_Object_Item *parent = va_arg(*list, Elm_Object_Item *);
   const char *icon = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   if (ret) *ret = NULL;

   Elm_Menu_Item *it;
   Evas_Object *icon_obj;

   Elm_Menu_Smart_Data *sd = _pd;

   icon_obj = elm_icon_add(obj);
   if (!icon_obj) return;

   it = elm_widget_item_new(obj, Elm_Menu_Item);
   if (!it)
     {
        evas_object_del(icon_obj);
        return;
     }

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);

   it->base.data = data;
   it->func = func;
   it->parent = (Elm_Menu_Item *)parent;
   it->content = icon_obj;

   _item_obj_create(it);
   elm_object_item_text_set((Elm_Object_Item *)it, label);

   elm_layout_content_set(VIEW(it), "elm.swallow.content",
                          it->content);

   if (icon) elm_menu_item_icon_name_set((Elm_Object_Item *)it, icon);

   _elm_menu_item_add_helper(obj, (Elm_Menu_Item *)parent, it, sd);

   if (sd->dbus_menu)
   {
     it->dbus_idx = _elm_dbus_menu_item_add(sd->dbus_menu, (Elm_Object_Item *)it);
     it->dbus_menu = sd->dbus_menu;
   }
   if (ret) *ret = (Elm_Object_Item *)it;
}

EAPI unsigned int
elm_menu_item_index_get(const Elm_Object_Item *it)
{
   ELM_MENU_ITEM_CHECK_OR_RETURN(it, 0);

   return ((Elm_Menu_Item *)it)->idx;
}

EAPI void
elm_menu_item_icon_name_set(Elm_Object_Item *it,
                            const char *icon)
{
   char icon_tmp[512];
   Elm_Menu_Item *item = (Elm_Menu_Item *)it;

   ELM_MENU_ITEM_CHECK_OR_RETURN(it);
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

EAPI Elm_Object_Item *
elm_menu_item_separator_add(Evas_Object *obj,
                            Elm_Object_Item *parent)
{
   ELM_MENU_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_menu_item_separator_add(parent, &ret));
   return ret;
}

static void
_item_separator_add(Eo *obj, void *_pd, va_list *list)
{
   Elm_Object_Item *parent = va_arg(*list, Elm_Object_Item *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Menu_Item *subitem;
   Elm_Menu_Item *p_item = (Elm_Menu_Item *)parent;

   Elm_Menu_Smart_Data *sd = _pd;

   /* don't add a separator as the first item */
   if (!sd->items) return;

   /* don't allow adding more than one separator in a row */
   if (p_item)
     {
        if (!p_item->submenu.items) return;
        subitem = eina_list_last(p_item->submenu.items)->data;
     }
   else subitem = eina_list_last(sd->items)->data;

   if (subitem->separator) return;

   subitem = elm_widget_item_new(obj, Elm_Menu_Item);
   if (!subitem) return;

   elm_widget_item_del_pre_hook_set(subitem, _item_del_pre_hook);
   elm_widget_item_disable_hook_set(subitem, _item_disable_hook);
   elm_widget_item_text_set_hook_set(subitem, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(subitem, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(subitem, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(subitem, _item_content_get_hook);

   subitem->separator = EINA_TRUE;
   _item_separator_obj_create(subitem);
   if (!p_item)
     {
        elm_box_pack_end(sd->bx, VIEW(subitem));
        sd->items = eina_list_append(sd->items, subitem);
     }
   else
     {
        if (!p_item->submenu.bx) _item_submenu_obj_create(p_item);
        elm_box_pack_end(p_item->submenu.bx, VIEW(subitem));
        p_item->submenu.items = eina_list_append
            (p_item->submenu.items, subitem);
     }

   _sizing_eval(obj);

   if (sd->dbus_menu)
     subitem->dbus_idx = _elm_dbus_menu_item_add(sd->dbus_menu,
                                                 (Elm_Object_Item *)subitem);
   *ret = (Elm_Object_Item *)subitem;
}

EAPI const char *
elm_menu_item_icon_name_get(const Elm_Object_Item *it)
{
   ELM_MENU_ITEM_CHECK_OR_RETURN(it, NULL);

   return ((Elm_Menu_Item *)it)->icon_str;
}

EAPI Eina_Bool
elm_menu_item_is_separator(Elm_Object_Item *it)
{
   ELM_MENU_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_Menu_Item *)it)->separator;
}

EAPI const Eina_List *
elm_menu_item_subitems_get(const Elm_Object_Item *it)
{
   ELM_MENU_ITEM_CHECK_OR_RETURN(it, NULL);

   return ((Elm_Menu_Item *)it)->submenu.items;
}

EAPI void
elm_menu_item_subitems_clear(Elm_Object_Item *it)
{
   Elm_Object_Item *sub_it;
   Eina_List *l, *l_next;

   ELM_MENU_ITEM_CHECK_OR_RETURN(it);
   EINA_LIST_FOREACH_SAFE(((Elm_Menu_Item *)it)->submenu.items,
                          l, l_next, sub_it)
     elm_object_item_del(sub_it);
}

EAPI const Eina_List *
elm_menu_items_get(const Evas_Object *obj)
{
   ELM_MENU_CHECK(obj) NULL;
   const Eina_List *ret = NULL;
   eo_do((Eo *) obj, elm_obj_menu_items_get(&ret));
   return ret;
}

static void
_items_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Elm_Menu_Smart_Data *sd = _pd;
   *ret = sd->items;
}

EAPI void
elm_menu_item_selected_set(Elm_Object_Item *it,
                           Eina_Bool selected)
{
   Elm_Menu_Item *item = (Elm_Menu_Item *)it;

   ELM_MENU_ITEM_CHECK_OR_RETURN(item);

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

EAPI Eina_Bool
elm_menu_item_selected_get(const Elm_Object_Item *it)
{
   ELM_MENU_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_Menu_Item *)it)->selected;
}

EAPI Elm_Object_Item *
elm_menu_item_prev_get(const Elm_Object_Item *it)
{
   Elm_Menu_Item *item = (Elm_Menu_Item *)it;

   ELM_MENU_ITEM_CHECK_OR_RETURN(item, NULL);

   if (item->parent)
     {
        Eina_List *l = eina_list_data_find_list
            (item->parent->submenu.items, item);
        l = eina_list_prev(l);
        if (!l) return NULL;
        return l->data;
     }
   else
     {
        ELM_MENU_DATA_GET(WIDGET(item), sd);
        if (!sd || !sd->items) return NULL;
        Eina_List *l = eina_list_data_find_list(sd->items, item);
        l = eina_list_prev(l);
        if (!l) return NULL;
        return l->data;
     }

   return NULL;
}

EAPI Elm_Object_Item *
elm_menu_item_next_get(const Elm_Object_Item *it)
{
   Elm_Menu_Item *item = (Elm_Menu_Item *)it;

   ELM_MENU_ITEM_CHECK_OR_RETURN(item, NULL);

   if (item->parent)
     {
        Eina_List *l =
          eina_list_data_find_list(item->parent->submenu.items, item);
        l = eina_list_next(l);
        if (!l) return NULL;
        return l->data;
     }
   else
     {
        ELM_MENU_DATA_GET(WIDGET(item), sd);
        if (!sd || !sd->items) return NULL;
        Eina_List *l = eina_list_data_find_list(sd->items, item);
        l = eina_list_next(l);
        if (!l) return NULL;
        return l->data;
     }

   return NULL;
}

EAPI Elm_Object_Item *
elm_menu_first_item_get(const Evas_Object *obj)
{
   ELM_MENU_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_menu_first_item_get(&ret));
   return ret;
}

static void
_first_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Menu_Smart_Data *sd = _pd;
   *ret = (sd->items ? sd->items->data : NULL);
}

EAPI Elm_Object_Item *
elm_menu_last_item_get(const Evas_Object *obj)
{
   ELM_MENU_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_menu_last_item_get(&ret));
   return ret;
}

static void
_last_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Menu_Smart_Data *sd = _pd;

   Eina_List *l = eina_list_last(sd->items);
   *ret = (l ? l->data : NULL);
}

EAPI Elm_Object_Item *
elm_menu_selected_item_get(const Evas_Object *obj)
{
   ELM_MENU_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_menu_selected_item_get(&ret));
   return ret;
}

static void
_selected_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;
   Eina_List *l;
   Elm_Menu_Item *item;

   Elm_Menu_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH(sd->items, l, item)
     {
        if (item->selected)
          {
             *ret = (Elm_Object_Item *)item;
             return;
          }
     }
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_menu_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_menu_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _elm_menu_smart_show),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME_APPLY), _elm_menu_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TRANSLATE), _elm_menu_smart_translate),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT_SET), _parent_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT_GET), _parent_get),

        EO_OP_FUNC(ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_MOVE), _move),
        EO_OP_FUNC(ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_CLOSE), _menu_close),
        EO_OP_FUNC(ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_ITEM_ADD), _item_add),
        EO_OP_FUNC(ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_ITEM_SEPARATOR_ADD), _item_separator_add),
        EO_OP_FUNC(ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_ITEMS_GET), _items_get),
        EO_OP_FUNC(ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_FIRST_ITEM_GET), _first_item_get),
        EO_OP_FUNC(ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_LAST_ITEM_GET), _last_item_get),
        EO_OP_FUNC(ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_SELECTED_ITEM_GET), _selected_item_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_MENU_SUB_ID_MOVE, "Move the menu to a new position."),
     EO_OP_DESCRIPTION(ELM_OBJ_MENU_SUB_ID_CLOSE, "Close a opened menu."),
     EO_OP_DESCRIPTION(ELM_OBJ_MENU_SUB_ID_ITEM_ADD, "Add an item at the end of the given menu widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_MENU_SUB_ID_ITEM_SEPARATOR_ADD, "Add a separator item to menu obj under parent."),
     EO_OP_DESCRIPTION(ELM_OBJ_MENU_SUB_ID_ITEMS_GET, "Returns a list of item's items."),
     EO_OP_DESCRIPTION(ELM_OBJ_MENU_SUB_ID_FIRST_ITEM_GET, "Get the first item in the menu."),
     EO_OP_DESCRIPTION(ELM_OBJ_MENU_SUB_ID_LAST_ITEM_GET, "Get the last item in the menu."),
     EO_OP_DESCRIPTION(ELM_OBJ_MENU_SUB_ID_SELECTED_ITEM_GET, "Get the selected item in the menu."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_MENU_BASE_ID, op_desc, ELM_OBJ_MENU_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Menu_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_menu_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, EVAS_SMART_CLICKABLE_INTERFACE, NULL);
