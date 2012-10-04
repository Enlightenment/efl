#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_menu.h"

EAPI const char ELM_MENU_SMART_NAME[] = "elm_menu";

static const char SIG_CLICKED[] = "clicked";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_MENU_SMART_NAME, _elm_menu, Elm_Menu_Smart_Class,
   Elm_Widget_Smart_Class, elm_widget_smart_class_get, _smart_callbacks);

static void
_item_del(Elm_Menu_Item *item)
{
   Elm_Menu_Item *child;

   EINA_LIST_FREE (item->submenu.items, child)
     _item_del(child);

   if (item->label) eina_stringshare_del(item->label);
   if (item->submenu.hv) evas_object_del(item->submenu.hv);
   if (item->submenu.location) evas_object_del(item->submenu.location);
   if (item->icon_str) eina_stringshare_del(item->icon_str);

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
        edje_object_signal_emit(VIEW(item), "elm,state,disabled", "elm");
        if (item->submenu.open) _submenu_hide(item);
     }
   else
     edje_object_signal_emit(VIEW(item), "elm,state,enabled", "elm");

   edje_object_message_signal_process(VIEW(item));
}

static void
_item_sizing_eval(Elm_Menu_Item *item)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   if (!item->separator) elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(VIEW(item), &minw, &minh, minw, minh);

   if (!item->separator) elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(VIEW(item), minw, minh);
   evas_object_size_hint_max_set(VIEW(item), maxw, maxh);
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
     _item_sizing_eval(item);

   evas_object_geometry_get
     (parent_it->submenu.location, &x_p, &y_p, &w_p, &h_p);
   evas_object_geometry_get(VIEW(parent_it), &x2, &y2, &w2, &h2);
   evas_object_geometry_get(parent_it->submenu.bx, &bx, &by, &bw, &bh);
   evas_object_geometry_get(sd->parent, &px, &py, &pw, &ph);

   x_p = x2 + w2;
   y_p = y2;

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

   ELM_MENU_DATA_GET(obj, sd);

   if (!sd->parent) return;

   EINA_LIST_FOREACH(sd->items, l, item)
     _item_sizing_eval(item);

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

   EINA_LIST_FOREACH(sd->items, l, item)
     if (item->submenu.open) _submenu_sizing_eval(item);
}

static Eina_Bool
_elm_menu_smart_theme(Evas_Object *obj)
{
   Eina_List *l, *_l, *_ll, *ll = NULL;
   Elm_Menu_Item *item;

   ELM_MENU_DATA_GET(obj, sd);

   if (!_elm_menu_parent_sc->theme(obj)) return EINA_FALSE;

   ll = eina_list_append(ll, sd->items);
   EINA_LIST_FOREACH(ll, _ll, l)
     {
        EINA_LIST_FOREACH(l, _l, item)
          {
             edje_object_mirrored_set(VIEW(item), elm_widget_mirrored_get(obj));
             ll = eina_list_append(ll, item->submenu.items);
             if (item->separator)
               elm_widget_theme_object_set
                 (obj, VIEW(item), "menu", "separator",
                 elm_widget_style_get(obj));
             else if (item->submenu.bx)
               {
                  elm_widget_theme_object_set
                    (obj, VIEW(item), "menu", "item_with_submenu",
                    elm_widget_style_get(obj));
                  elm_object_item_text_set((Elm_Object_Item *)item,
                                           item->label);
                  elm_menu_item_icon_name_set((Elm_Object_Item *)item,
                                              item->icon_str);
               }
             else
               {
                  elm_widget_theme_object_set
                    (obj, VIEW(item), "menu", "item",
                    elm_widget_style_get(obj));
                  elm_object_item_text_set((Elm_Object_Item *)item,
                                           item->label);
                  elm_menu_item_icon_name_set((Elm_Object_Item *)item,
                                              item->icon_str);
               }
             _item_disable_hook((Elm_Object_Item *)item);
             edje_object_scale_set
               (VIEW(item), elm_widget_scale_get(obj) *
               elm_config_scale_get());
          }
     }

   eina_list_free(ll); //fixme: test

   _sizing_eval(obj);

   return EINA_TRUE;
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
     edje_object_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");

   edje_object_message_signal_process(VIEW(item));
   edje_object_part_text_set(VIEW(item), "elm.text", label);

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

   if (item->content) evas_object_del(item->content);

   item->content = content;
   elm_widget_sub_object_add(WIDGET(item), item->content);
   if (item->content)
     edje_object_part_swallow
       (VIEW(item), "elm.swallow.content", item->content);

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
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_parent_resize_cb(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_parent_del_cb(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Elm_Menu_Smart_Data *sd = data;

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _parent_resize_cb, ELM_WIDGET_DATA(sd)->obj);
   sd->parent = NULL;
}

static void
_item_move_resize_cb(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   Elm_Menu_Item *item = data;

   if (item->submenu.open) _submenu_sizing_eval(item);
}

static void
_menu_hide(void *data,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   Eina_List *l;
   Elm_Menu_Item *item2;

   ELM_MENU_DATA_GET(data, sd);

   evas_object_hide(sd->hv);
   evas_object_hide(data);

   EINA_LIST_FOREACH(sd->items, l, item2)
     {
        if (item2->submenu.open) _submenu_hide(item2);
     }
}

static void
_hover_clicked_cb(void *data,
                  Evas_Object *obj,
                  void *event_info)
{
   _menu_hide(data, obj, event_info);
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_submenu_open_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   Elm_Menu_Item *item = data;

   item->submenu.open = EINA_TRUE;
   evas_object_show(item->submenu.hv);
   _submenu_sizing_eval(item);
}

static void
_menu_item_select_cb(void *data,
                     Evas_Object *obj __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
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
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
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
                         Evas_Object *obj __UNUSED__,
                         const char *emission __UNUSED__,
                         const char *source __UNUSED__)
{
   Elm_Menu_Item *item = data;

   item->selected = 0;
   if (item->submenu.open) _submenu_hide(item);
}

static void
_elm_menu_smart_show(Evas_Object *obj)
{
   ELM_MENU_DATA_GET(obj, sd);

   evas_object_show(sd->hv);
}

static void
_item_obj_create(Elm_Menu_Item *item)
{
   VIEW(item) = edje_object_add(evas_object_evas_get(WIDGET(item)));
   edje_object_mirrored_set(VIEW(item), elm_widget_mirrored_get(WIDGET(item)));
   evas_object_size_hint_weight_set
     (VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_widget_theme_object_set
     (WIDGET(item), VIEW(item), "menu", "item",
     elm_widget_style_get(WIDGET(item)));

   edje_object_signal_callback_add
     (VIEW(item), "elm,action,click", "", _menu_item_select_cb, item);
   edje_object_signal_callback_add
     (VIEW(item), "elm,action,activate", "", _menu_item_activate_cb, item);
   edje_object_signal_callback_add
     (VIEW(item), "elm,action,inactivate", "", _menu_item_inactivate_cb,
     item);
   evas_object_show(VIEW(item));
}

static void
_item_separator_obj_create(Elm_Menu_Item *item)
{
   VIEW(item) = edje_object_add(evas_object_evas_get(WIDGET(item)));
   evas_object_size_hint_weight_set
     (VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_widget_theme_object_set
     (WIDGET(item), VIEW(item), "menu", "separator",
     elm_widget_style_get(WIDGET(item)));
   edje_object_signal_callback_add
     (VIEW(item), "elm,action,activate", "", _menu_item_activate_cb, item);
   evas_object_show(VIEW(item));
}

static void
_item_submenu_obj_create(Elm_Menu_Item *item)
{
   ELM_MENU_DATA_GET(WIDGET(item), sd);

   item->submenu.location = elm_icon_add(sd->bx);
   item->submenu.hv = elm_hover_add(sd->bx);
   elm_widget_mirrored_set(item->submenu.hv, EINA_FALSE);
   elm_hover_target_set(item->submenu.hv, item->submenu.location);
   elm_hover_parent_set(item->submenu.hv, sd->parent);
   elm_object_style_set(item->submenu.hv, "submenu");

   item->submenu.bx = elm_box_add(sd->bx);
   elm_widget_mirrored_set(item->submenu.bx, EINA_FALSE);
   evas_object_size_hint_weight_set
     (item->submenu.bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(item->submenu.bx);
   elm_object_part_content_set
     (item->submenu.hv, elm_hover_best_content_location_get
       (item->submenu.hv, ELM_HOVER_AXIS_VERTICAL), item->submenu.bx);

   edje_object_mirrored_set(VIEW(item), elm_widget_mirrored_get(WIDGET(item)));
   elm_widget_theme_object_set
     (WIDGET(item), VIEW(item), "menu", "item_with_submenu",
     elm_widget_style_get(WIDGET(item)));
   elm_object_item_text_set((Elm_Object_Item *)item, item->label);

   if (item->icon_str)
     elm_menu_item_icon_name_set((Elm_Object_Item *)item, item->icon_str);

   edje_object_signal_callback_add(VIEW(item), "elm,action,open", "",
                                   _submenu_open_cb, item);
   evas_object_event_callback_add
     (VIEW(item), EVAS_CALLBACK_MOVE, _item_move_resize_cb, item);
   evas_object_event_callback_add
     (VIEW(item), EVAS_CALLBACK_RESIZE, _item_move_resize_cb, item);

   evas_object_event_callback_add
     (item->submenu.bx, EVAS_CALLBACK_RESIZE, _menu_resize_cb, WIDGET(item));
}

static void
_elm_menu_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Menu_Smart_Data);

   _elm_menu_parent_sc->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   priv->location = elm_icon_add(obj);

   priv->hv = elm_hover_add(obj);
   elm_widget_mirrored_set(priv->hv, EINA_FALSE);

   elm_object_style_set(priv->hv, "menu");
   evas_object_smart_callback_add(priv->hv, "clicked", _hover_clicked_cb, obj);

   priv->bx = elm_box_add(obj);
   elm_widget_mirrored_set(priv->bx, EINA_FALSE);
   evas_object_size_hint_weight_set
     (priv->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   evas_object_event_callback_add
     (priv->bx, EVAS_CALLBACK_RESIZE, _menu_resize_cb, obj);
}

static void
_elm_menu_smart_del(Evas_Object *obj)
{
   Elm_Menu_Item *item;

   ELM_MENU_DATA_GET(obj, sd);

   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
   evas_object_event_callback_del_full
     (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, sd);

   EINA_LIST_FREE (sd->items, item)
     _item_del(item);

   evas_object_event_callback_del_full
      (sd->bx, EVAS_CALLBACK_RESIZE, _menu_resize_cb, obj);

   if (sd->hv) evas_object_del(sd->hv);
   if (sd->location) evas_object_del(sd->location);

   _elm_menu_parent_sc->base.del(obj); /* handles freeing sd */
}

static void
_elm_menu_smart_parent_set(Evas_Object *obj,
                           Evas_Object *parent)
{
   ELM_MENU_DATA_GET(obj, sd);

   elm_menu_parent_set(obj, parent);
   elm_hover_target_set(sd->hv, sd->location);
   elm_layout_content_set
     (sd->hv, elm_hover_best_content_location_get
       (sd->hv, ELM_HOVER_AXIS_VERTICAL), sd->bx);

   _sizing_eval(obj);
}

static void
_elm_menu_smart_set_user(Elm_Menu_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_menu_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_menu_smart_del;
   ELM_WIDGET_CLASS(sc)->base.show = _elm_menu_smart_show;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_WIDGET_CLASS(sc)->parent_set = _elm_menu_smart_parent_set;
   ELM_WIDGET_CLASS(sc)->theme = _elm_menu_smart_theme;
}

EAPI const Elm_Menu_Smart_Class *
elm_menu_smart_class_get(void)
{
   static Elm_Menu_Smart_Class _sc =
     ELM_MENU_SMART_CLASS_INIT_NAME_VERSION(ELM_MENU_SMART_NAME);
   static const Elm_Menu_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_menu_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_menu_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_menu_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_menu_parent_set(Evas_Object *obj,
                    Evas_Object *parent)
{
   Eina_List *l, *_l, *_ll, *ll = NULL;
   Elm_Menu_Item *item;

   ELM_MENU_CHECK(obj);
   ELM_MENU_DATA_GET(obj, sd);

   if (sd->parent == parent) return;
   if (sd->parent)
     {
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, sd);
     }
   sd->parent = parent;
   if (sd->parent)
     {
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_RESIZE, _parent_resize_cb, obj);
        evas_object_event_callback_add
          (sd->parent, EVAS_CALLBACK_DEL, _parent_del_cb, sd);
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
   ELM_MENU_DATA_GET(obj, sd);

   return sd->parent;
}

EAPI void
elm_menu_move(Evas_Object *obj,
              Evas_Coord x,
              Evas_Coord y)
{
   ELM_MENU_CHECK(obj);
   ELM_MENU_DATA_GET(obj, sd);

   sd->xloc = x;
   sd->yloc = y;
   _sizing_eval(obj);
}

EAPI void
elm_menu_close(Evas_Object *obj)
{
   ELM_MENU_CHECK(obj);
   ELM_MENU_DATA_GET(obj, sd);

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
   Elm_Object_Item *_item;

   EINA_LIST_FREE (item->submenu.items, _item)
     elm_object_item_del(_item);
   if (item->label) eina_stringshare_del(item->label);
   if (item->content) evas_object_del(item->content);
   if (item->submenu.hv) evas_object_del(item->submenu.hv);
   if (item->submenu.location) evas_object_del(item->submenu.location);

   if (item->parent)
     item->parent->submenu.items =
       eina_list_remove(item->parent->submenu.items, item);
   else
     {
        ELM_MENU_DATA_GET(WIDGET(item), sd);
        sd->items = eina_list_remove(sd->items, item);
     }

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
   Elm_Menu_Item *subitem;
   Evas_Object *icon_obj;

   ELM_MENU_CHECK(obj) NULL;
   ELM_MENU_DATA_GET(obj, sd);

   icon_obj = elm_icon_add(obj);
   if (!icon_obj) return NULL;

   subitem = elm_widget_item_new(obj, Elm_Menu_Item);
   if (!subitem)
     {
        evas_object_del(icon_obj);
        return NULL;
     }

   elm_widget_item_del_pre_hook_set(subitem, _item_del_pre_hook);
   elm_widget_item_disable_hook_set(subitem, _item_disable_hook);
   elm_widget_item_text_set_hook_set(subitem, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(subitem, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(subitem, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(subitem, _item_content_get_hook);

   subitem->base.data = data;
   subitem->func = func;
   subitem->parent = (Elm_Menu_Item *)parent;
   subitem->content = icon_obj;

   _item_obj_create(subitem);
   elm_object_item_text_set((Elm_Object_Item *)subitem, label);

   elm_widget_sub_object_add(WIDGET(subitem), subitem->content);
   edje_object_part_swallow
     (VIEW(subitem), "elm.swallow.content", subitem->content);

   if (icon) elm_menu_item_icon_name_set((Elm_Object_Item *)subitem, icon);

   _elm_menu_item_add_helper(obj, (Elm_Menu_Item *)parent, subitem, sd);

   return (Elm_Object_Item *)subitem;
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

   if (!*icon) return;
   if ((item->icon_str) && (!strcmp(item->icon_str, icon))) return;
   if ((snprintf(icon_tmp, sizeof(icon_tmp), "menu/%s", icon) > 0) &&
       (elm_icon_standard_set(item->content, icon_tmp) ||
        elm_icon_standard_set(item->content, icon)))
     {
        eina_stringshare_replace(&item->icon_str, icon);
        edje_object_signal_emit(VIEW(item), "elm,state,icon,visible", "elm");
     }
   else
     edje_object_signal_emit(VIEW(item), "elm,state,icon,hidden", "elm");

   edje_object_message_signal_process(VIEW(item));
   _sizing_eval(WIDGET(item));
}

EAPI Elm_Object_Item *
elm_menu_item_separator_add(Evas_Object *obj,
                            Elm_Object_Item *parent)
{
   Elm_Menu_Item *subitem;
   Elm_Menu_Item *p_item = (Elm_Menu_Item *)parent;

   ELM_MENU_CHECK(obj) NULL;
   ELM_MENU_DATA_GET(obj, sd);

   /* don't add a separator as the first item */
   if (!sd->items) return NULL;

   /* don't allow adding more than one separator in a row */
   if (p_item)
     {
        if (!p_item->submenu.items) return NULL;
        subitem = eina_list_last(p_item->submenu.items)->data;
     }
   else subitem = eina_list_last(sd->items)->data;

   if (subitem->separator) return NULL;

   subitem = elm_widget_item_new(obj, Elm_Menu_Item);
   if (!subitem) return NULL;

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
   return (Elm_Object_Item *)subitem;
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

EAPI const Eina_List *
elm_menu_items_get(const Evas_Object *obj)
{
   ELM_MENU_CHECK(obj) NULL;
   ELM_MENU_DATA_GET(obj, sd);

   return sd->items;
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
        edje_object_signal_emit(VIEW(item), "elm,state,selected", "elm");
        _menu_item_activate_cb(item, NULL, NULL, NULL);
     }
   else
     {
        edje_object_signal_emit(VIEW(item), "elm,state,unselected", "elm");
        _menu_item_inactivate_cb(item, NULL, NULL, NULL);
     }
   edje_object_message_signal_process(VIEW(item));
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
   ELM_MENU_DATA_GET(obj, sd);

   if (sd->items) return sd->items->data;
   return NULL;
}

EAPI Elm_Object_Item *
elm_menu_last_item_get(const Evas_Object *obj)
{
   ELM_MENU_CHECK(obj) NULL;
   ELM_MENU_DATA_GET(obj, sd);

   Eina_List *l = eina_list_last(sd->items);
   if (l) return l->data;

   return NULL;
}

EAPI Elm_Object_Item *
elm_menu_selected_item_get(const Evas_Object *obj)
{
   Eina_List *l;
   Elm_Menu_Item *item;

   ELM_MENU_CHECK(obj) NULL;
   ELM_MENU_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->items, l, item)
     {
        if (item->selected) return (Elm_Object_Item *)item;
     }

   return NULL;
}
