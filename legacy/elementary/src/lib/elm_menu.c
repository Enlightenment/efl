/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Menu Menu
 *
 * A menu is a list of items displayed above the window. Each item can
 * have a sub-menu. The menu object can be used to display a menu on right
 * click, in a toolbar, anywhere.
 *
 */

typedef struct _Widget_Data Widget_Data;

struct _Menu_Item
{
   Evas_Object *menu;
   Evas_Object *o;
   Elm_Menu_Item *parent;

   Eina_Bool separator;
   Eina_Bool disabled;

   //if classic item or submenu
   Evas_Object *icon;
   const char *label;

   Evas_Smart_Cb func;
   Evas_Smart_Cb del_cb;
   const void *data;

   //if submenu
   Eina_Bool open;
   Evas_Object *hv, *bx, *location;
   Eina_List *items;
};

struct _Widget_Data
{
   Evas_Object *hv, *bx, *location, *parent;
   Eina_List *items;
   Evas_Coord xloc, yloc;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _submenu_sizing_eval(Elm_Menu_Item *parent);
static void _item_sizing_eval(Elm_Menu_Item *item);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _submenu_hide(Elm_Menu_Item *item);
static void _submenu_open(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _parent_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _menu_hide(void *data, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_RESIZE, _parent_resize, obj);
}

static void
_del_hook(Evas_Object *obj)
{
   Eina_List *l, *ll = NULL;
   Elm_Menu_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   ll = eina_list_append(ll, wd->items);
   EINA_LIST_FOREACH(ll, ll, l)
     {
	EINA_LIST_FREE(l, item)
	  {
	     ll = eina_list_append(ll, item->items);
	     if (item->del_cb) item->del_cb((void*)item->data, item->o, item);
	     if (item->label) eina_stringshare_del(item->label);
	     if (item->icon) evas_object_del(item->icon);
	     if (item->hv) evas_object_del(item->hv);
	     if (item->location) evas_object_del(item->location);
	     free(item);
	  }
     }
   if (wd->hv) evas_object_del(wd->hv);
   if (wd->location) evas_object_del(wd->location);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Eina_List *l, *_l, *ll = NULL;
   Elm_Menu_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   ll = eina_list_append(ll, wd->items);
   EINA_LIST_FOREACH(ll, ll, l)
     {
	EINA_LIST_FOREACH(l, _l, item)
	  {
	     ll = eina_list_append(ll, item->items);
	     if (item->separator)
	       _elm_theme_object_set(obj, item->o, "menu", "separator",
                                     elm_widget_style_get(obj));
	     else if (item->bx)
	       {
		  _elm_theme_object_set(obj, item->o, "menu", "item_with_submenu",
                                        elm_widget_style_get(obj));
		  elm_menu_item_label_set(item, item->label);
		  elm_menu_item_icon_set(item, item->icon);
	       }
	     else
	       {
		  _elm_theme_object_set(obj, item->o, "menu", "item",
                                        elm_widget_style_get(obj));
		  elm_menu_item_label_set(item, item->label);
		  elm_menu_item_icon_set(item, item->icon);
	       }
	     if (item->disabled)
	       edje_object_signal_emit(item->o, "elm,state,disabled", "elm");
	     else
	       edje_object_signal_emit(item->o, "elm,state,enabled", "elm");
	     edje_object_message_signal_process(item->o);
	     edje_object_scale_set(item->o, elm_widget_scale_get(obj) *
                                   _elm_config->scale);
	  }
     }
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Menu_Item *item;
   Evas_Coord x_p, y_p, w_p, h_p, x2, y2, w2, h2, bx, by, bw, bh;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->parent)) return;
   EINA_LIST_FOREACH(wd->items,l,item) _item_sizing_eval(item);
   evas_object_geometry_get(wd->location, &x_p, &y_p, &w_p, &h_p);
   evas_object_geometry_get(wd->parent, &x2, &y2, &w2, &h2);
   evas_object_geometry_get(wd->bx, &bx, &by, &bw, &bh);

   x_p = wd->xloc;
   y_p = wd->yloc;

   if (x_p+bw > x2+w2) x_p -= x_p+bw - (x2+w2);
   if (x_p < x2) x_p += x2 - x_p;

   if (y_p+h_p+bh > y2+h2) y_p -= y_p+h_p+bh - (y2+h2);
   if (y_p < y2) y_p += y2 - y_p;

   evas_object_move(wd->location, x_p, y_p);
   evas_object_resize(wd->location, bw, h_p);
   evas_object_size_hint_min_set(wd->location, bw, h_p);
   evas_object_size_hint_max_set(wd->location, bw, h_p);
   elm_hover_target_set(wd->hv, wd->location);

   EINA_LIST_FOREACH(wd->items,l,item)
     {
	if (item->open) _submenu_sizing_eval(item);
     }
}

static void
_submenu_sizing_eval(Elm_Menu_Item *parent)
{
   Eina_List *l;
   Elm_Menu_Item *item;
   Evas_Coord x_p, y_p, w_p, h_p, x2, y2, w2, h2, bx, by, bw, bh, px, py, pw, ph;
   Widget_Data *wd = elm_widget_data_get(parent->menu);
   if (!wd) return;
   EINA_LIST_FOREACH(parent->items,l,item) _item_sizing_eval(item);
   evas_object_geometry_get(parent->location, &x_p, &y_p, &w_p, &h_p);
   evas_object_geometry_get(parent->o, &x2, &y2, &w2, &h2);
   evas_object_geometry_get(parent->bx, &bx, &by, &bw, &bh);
   evas_object_geometry_get(wd->parent, &px, &py, &pw, &ph);

   x_p = x2+w2;
   y_p = y2;

   if (x_p + bw > px + pw)
     x_p = x2-bw;

   if (y_p+bh > py+ph)
     y_p -= y_p+bh - (py+ph);
   if (y_p < py)
     y_p += y_p - y_p;

   evas_object_move(parent->location, x_p, y_p);
   evas_object_resize(parent->location, bw, h_p);
   evas_object_size_hint_min_set(parent->location, bw, h_p);
   evas_object_size_hint_max_set(parent->location, bw, h_p);
   elm_hover_target_set(parent->hv, parent->location);

   EINA_LIST_FOREACH(parent->items,l,item)
     {
	if (item->open)
	  _submenu_sizing_eval(item);
     }
}

static void
_item_sizing_eval(Elm_Menu_Item *item)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!item->separator)
     elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(item->o, &minw, &minh, minw, minh);
   if (!item->separator)
     elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(item->o, minw, minh);
   evas_object_size_hint_max_set(item->o, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_menu_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_parent_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_item_move_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Menu_Item *item = data;
   if (item->open) _submenu_sizing_eval(item);
}

static void
_hover_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   _menu_hide(data, obj, event_info);
   evas_object_smart_callback_call(data, "clicked", NULL);
}

static void
_menu_hide(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Eina_List *l;
   Elm_Menu_Item *item2;
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_hide(wd->hv);
   evas_object_hide(data);

   EINA_LIST_FOREACH(wd->items, l, item2)
     {
	if (item2->open) _submenu_hide(item2);
     }
}

static void
_submenu_hide(Elm_Menu_Item *item)
{
   Eina_List *l;
   Elm_Menu_Item *item2;
   evas_object_hide(item->hv);
   item->open = EINA_FALSE;
   EINA_LIST_FOREACH(item->items, l, item2)
     {
	if (item2->open) _submenu_hide(item2);
     }
}

static void
_menu_item_select(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Elm_Menu_Item *it = data;
   if (it->items)
     {
	if (!it->open) _submenu_open(it, NULL, NULL, NULL);
	else _submenu_hide(it);
     }
   else
     _menu_hide(it->menu, NULL, NULL);

   if (it->func) it->func((void *)(it->data), it->menu, it);
}

static void
_menu_item_activate(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Eina_List *l;
   Elm_Menu_Item *item2;
   Elm_Menu_Item *item = data;
   if (item->parent)
     {
	EINA_LIST_FOREACH(item->parent->items, l, item2)
	  {
	     if ((item2->open) && (item2 != item)) _submenu_hide(item2);
	  }
     }
   else
     {
	Widget_Data *wd = elm_widget_data_get(item->menu);
	EINA_LIST_FOREACH(wd->items, l, item2)
	  {
	     if ((item2->open) && (item2 != item)) _submenu_hide(item2);
	  }
     }
}

static void
_submenu_open(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Elm_Menu_Item *it = data;
   it->open = EINA_TRUE;
   evas_object_show(it->hv);
   _sizing_eval(it->menu);
}

static void
_show(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   evas_object_show(wd->hv);
}

static void
_item_obj_create(Elm_Menu_Item *item)
{
   Widget_Data *wd = elm_widget_data_get(item->menu);
   if (!wd) return;
   item->o = edje_object_add(evas_object_evas_get(wd->bx));
   evas_object_size_hint_weight_set(item->o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(item->o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   _elm_theme_object_set(item->menu, item->o, "menu", "item",  elm_widget_style_get(item->menu));
   edje_object_signal_callback_add(item->o, "elm,action,click", "",
                                   _menu_item_select, item);
   edje_object_signal_callback_add(item->o, "elm,action,activate", "",
                                   _menu_item_activate, item);
   evas_object_show(item->o);
}

static void
_item_separator_obj_create(Elm_Menu_Item *item)
{
   Widget_Data *wd = elm_widget_data_get(item->menu);
   if (!wd) return;
   item->o = edje_object_add(evas_object_evas_get(wd->bx));
   evas_object_size_hint_weight_set(item->o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(item->o, EVAS_HINT_FILL, EVAS_HINT_FILL);
   _elm_theme_object_set(item->menu, item->o, "menu", "separator",  elm_widget_style_get(item->menu));
   edje_object_signal_callback_add(item->o, "elm,action,activate", "",
                                   _menu_item_activate, item);
   evas_object_show(item->o);
}

static void
_item_submenu_obj_create(Elm_Menu_Item *item)
{
   Widget_Data *wd = elm_widget_data_get(item->menu);
   if (!wd) return;
   item->location = elm_icon_add(wd->bx);
   item->hv = elm_hover_add(wd->bx);
   elm_hover_target_set(item->hv, item->location);
   elm_hover_parent_set(item->hv, wd->parent);
   elm_object_style_set(item->hv, "submenu");

   item->bx = elm_box_add(wd->bx);
   evas_object_size_hint_weight_set(item->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(item->bx);
   elm_hover_content_set(item->hv, elm_hover_best_content_location_get(item->hv, ELM_HOVER_AXIS_VERTICAL), item->bx);

   _elm_theme_object_set(item->menu, item->o, "menu", "item_with_submenu",  elm_widget_style_get(item->menu));
   elm_menu_item_label_set(item, item->label);
   elm_menu_item_icon_set(item, item->icon);

   edje_object_signal_callback_add(item->o, "elm,action,open", "",
                                   _submenu_open, item);
   evas_object_event_callback_add(item->o, EVAS_CALLBACK_MOVE, _item_move_resize, item);
   evas_object_event_callback_add(item->o, EVAS_CALLBACK_RESIZE, _item_move_resize, item);

   evas_object_event_callback_add(item->bx, EVAS_CALLBACK_RESIZE, _menu_resize, item->menu);
}

/**
 * Add a new menu to the parent
 *
 * @param parent The parent object.
 * @return The new object or NULL if it cannot be created.
 *
 * @ingroup Menu
 */
EAPI Evas_Object *
elm_menu_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "menu");
   elm_widget_type_set(obj, "menu");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->location = elm_icon_add(obj);
   wd->parent = parent;

   wd->hv = elm_hover_add(obj);
   elm_hover_parent_set(wd->hv, parent);
   elm_hover_target_set(wd->hv, wd->location);
   elm_object_style_set(wd->hv, "menu");
   evas_object_smart_callback_add(wd->hv, "clicked", _hover_clicked_cb, obj);

   wd->bx = elm_box_add(obj);
   evas_object_size_hint_weight_set(wd->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(wd->bx);
   elm_hover_content_set(wd->hv, elm_hover_best_content_location_get(wd->hv, ELM_HOVER_AXIS_VERTICAL), wd->bx);

   evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_RESIZE, _parent_resize, obj);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _show, obj);

   evas_object_event_callback_add(wd->bx, EVAS_CALLBACK_RESIZE, _menu_resize, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the parent
 *
 * @param obj The menu object.
 * @param parent The new parent.
 * 
 * @ingroup Menu
 */
EAPI void
elm_menu_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Eina_List *l, *_l, *ll = NULL;
   Elm_Menu_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_hover_parent_set(wd->hv, parent);
   wd->parent = parent;

   ll = eina_list_append(ll, wd->items);
   EINA_LIST_FOREACH(ll, ll, l)
     {
	EINA_LIST_FOREACH(l, _l, item)
	  {
	     if (item->hv)
	       {
		  elm_hover_parent_set(item->hv, parent);
		  ll = eina_list_append(ll, item->items);
	       }
	  }
     }
   _sizing_eval(obj);
}

/**
 * Move the menu to a new position
 *
 * @param obj The menu object.
 * @param x The new position.
 * @param y The new position.
 * 
 * @ingroup Menu
 */
EAPI void
elm_menu_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->xloc = x;
   wd->yloc = y;
   _sizing_eval(obj);
}

/**
 * Get the Evas_Object of an Elm_Menu_Item
 * 
 * @param it The menu item object.
 * 
 * @ingroup Menu
 */
EAPI Evas_Object *
elm_menu_object_get(const Elm_Menu_Item *it)
{
   return it->o;
}

/**
 * Add an item at the end
 *
 * @param obj The menu object.
 * @param icon A icon display on the item. The icon will be destryed by the menu.
 * @param label The label of the item.
 * @param func Function called when the user select the item.
 * @param data Data sent by the callback.
 * @return Returns the new item.
 * 
 * @ingroup Menu
 */
EAPI Elm_Menu_Item *
elm_menu_item_add(Evas_Object *obj, Elm_Menu_Item *parent, Evas_Object *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   Elm_Menu_Item *subitem;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   subitem = ELM_NEW(Elm_Menu_Item);
   if (!subitem) return NULL;
   subitem->func = func;
   subitem->data = data;
   subitem->menu = obj;
   subitem->parent = parent;

   _item_obj_create(subitem);
   elm_menu_item_label_set(subitem, label);
   elm_menu_item_icon_set(subitem, icon);

   if (parent)
     {
	if (!parent->bx) _item_submenu_obj_create(parent);
	elm_box_pack_end(parent->bx, subitem->o);
	parent->items = eina_list_append(parent->items, subitem);
     }
   else
     {
	elm_box_pack_end(wd->bx, subitem->o);
	wd->items = eina_list_append(wd->items, subitem);
     }

   _sizing_eval(obj);
   return subitem;
}

/**
 * Set the label of a menu item
 *
 * @param it The menu item object.
 * @param label The label to set for @p item
 *
 * @ingroup Menu
 */
EAPI void
elm_menu_item_label_set(Elm_Menu_Item *item, const char *label)
{
   eina_stringshare_replace(&item->label, label);

   if (label)
     edje_object_signal_emit(item->o, "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(item->o, "elm,state,text,hidden", "elm");

   edje_object_message_signal_process(item->o);
   edje_object_part_text_set(item->o, "elm.text", label);
   _sizing_eval(item->menu);
}

/**
 * Get the label of a menu item
 *
 * @param it The menu item object.
 * @return The label of @p item
 *
 * @ingroup Menu
 */
EAPI const char *
elm_menu_item_label_get(Elm_Menu_Item *item)
{
   if (!item) return NULL;
   return item->label;
}

/**
 * Set the icon of a menu item
 *
 * @param it The menu item object.
 * @param icon The icon object to set for @p item
 *
 * @ingroup Menu
 */
EAPI void
elm_menu_item_icon_set(Elm_Menu_Item *item, Evas_Object *icon)
{
   if ((item->icon != icon) && (item->icon))
     elm_widget_sub_object_del(item->menu, item->icon);
   if ((icon) && (item->icon != icon))
     {
	item->icon = icon;
	elm_widget_sub_object_add(item->menu, icon);
	evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, item->menu);
	edje_object_part_swallow(item->o, "elm.swallow.content", icon);
	edje_object_signal_emit(item->o, "elm,state,icon,visible", "elm");
	edje_object_message_signal_process(item->o);
	_sizing_eval(item->menu);
     }
   else
     item->icon = icon;
}


/**
 * Set the disabled state of @p item.
 *
 * @param it The menu item object.
 * @param disabled The enabled/disabled state of the item
 *
 * @ingroup Menu
 */
EAPI void
elm_menu_item_disabled_set(Elm_Menu_Item *item, Eina_Bool disabled)
{
   if (disabled == item->disabled) return;
   item->disabled = disabled;
   if (disabled)
     {
        edje_object_signal_emit(item->o, "elm,state,disabled", "elm");
        if (item->open) _submenu_hide(item);
     }
   else
     edje_object_signal_emit(item->o, "elm,state,enabled", "elm");
   edje_object_message_signal_process(item->o);
}

/**
 * Get the disabled state of @p item.
 * 
 * @param it The menu item object.
 * @return The enabled/disabled state of the item
 * 
 * @ingroup Menu
 */
EAPI Eina_Bool
elm_menu_item_disabled_get(Elm_Menu_Item *item)
{
   if (!item) return 0;
   return item->disabled;
}

/**
 * Add a separator item to menu @p obj under @p parent.
 *
 * @param obj The menu object
 * @param parent The item to add the separator under
 *
 * @return The created item or NULL on failure
 *
 * @ingroup Menu
 */
EAPI Elm_Menu_Item *
elm_menu_item_separator_add(Evas_Object *obj, Elm_Menu_Item *parent)
{
   Elm_Menu_Item *subitem;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   subitem = ELM_NEW(Elm_Menu_Item);
   if (!subitem) return NULL;
   subitem->menu = obj;
   subitem->separator = 1;
   _item_separator_obj_create(subitem);
   if (!parent)
     {
	elm_box_pack_end(wd->bx, subitem->o);
	wd->items = eina_list_append(wd->items, subitem);
     }
   else
     {
	if (!parent->bx) _item_submenu_obj_create(parent);
	elm_box_pack_end(parent->bx, subitem->o);
	parent->items = eina_list_append(parent->items, subitem);
     }
   _sizing_eval(obj);
   return subitem;
}


/**
 * Deletes an item from the menu.
 *
 * @param item The item to delete
 *
 * @ingroup Menu
 */

/**
 * Get the icon of a menu item
 * 
 * @param it The menu item object.
 * @return The icon object of @p item or NULL
 * 
 * @ingroup Menu
 */
EAPI Evas_Object *
elm_menu_item_icon_get(Elm_Menu_Item *item)
{
   if (!item) return NULL;
   return item->icon;
}

/**
 * Returns whether @p item is a separator.
 *
 * @param item The item to check
 * @return If true, @p item is a separator
 *
 * @ingroup Menu
 */
EAPI Eina_Bool
elm_menu_item_is_separator(Elm_Menu_Item *item)
{
   if (!item) return 0;
   return item->separator;
}

EAPI void
elm_menu_item_del(Elm_Menu_Item *item)
{
   Elm_Menu_Item *_item;

   if (!item) return;
   if (item->del_cb) item->del_cb((void*)item->data, item->o, item);

   EINA_LIST_FREE(item->items, _item) elm_menu_item_del(_item);
   if (item->label) eina_stringshare_del(item->label);
   if (item->icon) evas_object_del(item->icon);
   if (item->hv) evas_object_del(item->hv);
   if (item->location) evas_object_del(item->location);
   if (item->o) evas_object_del(item->o);

   if (item->parent)
     item->parent->items = eina_list_remove(item->parent->items, item);
   else
     {
	Widget_Data *wd = elm_widget_data_get(item->menu);
	wd->items = eina_list_remove(wd->items, item);
     }

   free(item);
}

/**
 * Set the function called when a menu item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * @ingroup Menu
 */
EAPI void
elm_menu_item_del_cb_set(Elm_Menu_Item *it, Evas_Smart_Cb func)
{
   if (!it) return;
   it->del_cb = func;
}

/**
 * Returns the data associated with menu item @p it.
 *
 * @param it The item
 * @return The data associated with @p it
 *
 * @ingroup Menu
 */
EAPI void *
elm_menu_item_data_get(const Elm_Menu_Item *it)
{
   if (!it) return NULL;
   return (void *)it->data;
}

/**
 * Sets the data to be associated with menu item @p it.
 *
 * @param item The item
 * @param data The data to be associated with @p item
 *
 * @ingroup Menu
 */
EAPI void
elm_menu_item_data_set(Elm_Menu_Item *item, const void *data)
{
   if (!item) return;
   item->data = data;
}

/**
 * Returns a list of @p item's subitems.
 *
 * @param it The item
 * @return An Eina_List* of @p item's subitems
 *
 * @ingroup Menu
 */
EAPI const Eina_List *
elm_menu_item_subitems_get(Elm_Menu_Item *item)
{
   if (!item) return NULL;
   return item->items;
}
