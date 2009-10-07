/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Menu menu
 *
 * A menu is a list of items display on top of the window. It can be used to
 * display a menu on right click, in a toolbar ...
 *
 * This widget use a hover and each item is a button with the style "menu"
 */

typedef struct _Widget_Data Widget_Data;

struct _Menu_Item
{
   Evas_Object *parent;
   Evas_Object *o;

   Evas_Object *icon;
   const char *label;

   void (*func) (void *data, Evas_Object *obj, void *event_info);
   const void *data;
};

struct _Widget_Data
{
   Evas_Object *hv, *bx, *location, *parent;
   Eina_List *menu;
   Evas_Coord xloc, yloc;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if(!wd) return;
   //_elm_theme_set(wd->base, "menu", "base", "default");
   //edje_object_scale_set(wd->base, elm_widget_scale_get(obj) *
   //                      _elm_config->scale);
   _sizing_eval(obj);
}

   static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord x_p,y_p,w_p,h_p,x2,y2,w2,h2,bx,by,bw,bh;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd || !wd->parent) return;
   evas_object_geometry_get(wd->location, &x_p, &y_p, &w_p, &h_p);
   evas_object_geometry_get(wd->parent, &x2, &y2, &w2, &h2);
   evas_object_geometry_get(wd->bx, &bx, &by, &bw, &bh);

   x_p = wd->xloc;
   y_p = wd->yloc;

   //printf("%d %d %d %d\n%d %d %d %d\n\n", x_p, y_p, bw, bh,x2, y2, w2, h2);
   if(x_p+bw > x2+w2)
     x_p -= x_p+bw - (x2+w2);
   if(x_p < x2)
     x_p += x2 - x_p;

   if(y_p+h_p+bh > y2+h2)
     y_p -= y_p+h_p+bh - (y2+h2);
   if(y_p < y2)
     y_p += y2 - y_p;

   //printf("%d %d %d %d\n\n\n", x_p, y_p, bw, bh);
   evas_object_move(wd->location, x_p, y_p);
   evas_object_resize(wd->location, bw, h_p);
   evas_object_size_hint_min_set(wd->location, bw, h_p);
   evas_object_size_hint_max_set(wd->location, bw, h_p);
   elm_hover_target_set(wd->hv, wd->location);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   _sizing_eval(data);
}

static void
_menu_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   _sizing_eval(data);
}

static void
_parent_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);

   _sizing_eval(data);
}

static void
_menu_hide(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_hide(wd->hv);
   evas_object_hide(data);

   evas_object_smart_callback_call(data, "clicked", event_info);
}

static void
_menu_item_select(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Menu_Item *it = data;
    if (it->func) it->func((void *)(it->data), it->o, it);

    _menu_hide(it->parent, NULL, NULL);
}

static void
_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_show(wd->hv);
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
   elm_widget_type_set(obj, "menu");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->location = elm_icon_add(obj);
   wd->parent = parent;

   wd->hv = elm_hover_add(obj);
   elm_hover_parent_set(wd->hv, parent);
   elm_hover_target_set(wd->hv, wd->location);
   elm_hover_style_set(wd->hv, "menu");
   evas_object_smart_callback_add(wd->hv, "clicked", _menu_hide, obj);

   wd->bx = elm_box_add(obj);
   evas_object_event_callback_add(wd->bx, EVAS_CALLBACK_RESIZE, _menu_resize, obj);
   evas_object_size_hint_weight_set(wd->bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(wd->bx);
   elm_hover_content_set(wd->hv, elm_hover_best_content_location_get(wd->hv, ELM_HOVER_AXIS_VERTICAL), wd->bx);

   evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_RESIZE, _parent_resize, obj);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _show, obj);

   _sizing_eval(obj);
   return obj;
}

/*
 * Set the parent
 *
 * @param obj The menu object.
 * @param parent The new parent.
 */
EAPI void		
elm_menu_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_hover_parent_set(wd->hv, parent);
   wd->parent = parent;
   _sizing_eval(obj);
}

/*
 * Move the menu to a new position
 * 
 * @param obj The menu object.
 * @param x The new position.
 * @param y The new position.
 */
EAPI void 
elm_menu_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->xloc = x;
   wd->yloc = y;
   _sizing_eval(obj);
}

/*
 * Add an item at the end
 *
 * @param obj The menu object.
 * @param icon A icon display on the item. The icon will be destryed by the
 * menu.
 * @param label The label of the item.
 * @param func Function called when the user select the item.
 * @param data Data sent by the callback.
 * #return Returns the new item.
 */
EAPI Elm_Menu_Item *
elm_menu_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Evas_Object *bt;
   Elm_Menu_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   item = calloc(1, sizeof(Elm_Menu_Item));
   if(!item) return NULL;
   if(label)
     item->label = eina_stringshare_add(label);
   item->icon = icon;
   item->func = func;
   item->data = data;
   item->parent = obj;

   bt = elm_button_add(obj);
   item->o = bt;
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_style_set(bt, "menu");
   elm_button_label_set(bt, label);
   elm_button_icon_set(bt, icon);
   evas_object_smart_callback_add(bt, "clicked", _menu_item_select, item);
   elm_box_pack_end(wd->bx, bt);
   evas_object_show(bt);


   wd->menu = eina_list_append(wd->menu, item);

   _sizing_eval(obj);
   return item;
}

