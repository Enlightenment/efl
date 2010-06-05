/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Toolbar Toolbar
 *
 * A toolbar is a widget that displays a list of buttons inside
 * a box.  It is scrollable, and only one item can be selected at a time.
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr, *bx;
   Eina_List *items;
   int icon_size;
   Eina_Bool scrollable : 1;
   Evas_Object *menu_parent;
   Eina_Bool homogeneous : 1;
   double align;
};

struct _Elm_Toolbar_Item
{
   Evas_Object *obj;
   Evas_Object *base;
   const char *label;
   Evas_Object *icon;
   Evas_Smart_Cb func;
   Evas_Smart_Cb del_cb;
   const void *data;
   Eina_Bool selected : 1;
   Eina_Bool disabled : 1;
   Eina_Bool separator : 1;
   Eina_Bool menu;
   Evas_Object *o_menu;
};

static const char *widtype = NULL;
static void _item_show(Elm_Toolbar_Item *it);
static void _item_select(Elm_Toolbar_Item *it);
static void _item_disable(Elm_Toolbar_Item *it, Eina_Bool disabled);
static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _menu_move_resize(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _menu_hide(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _layout(Evas_Object *o, Evas_Object_Box_Data *priv, void *data);

static void
_item_show(Elm_Toolbar_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Coord x, y, w, h, bx, by;

   if (!wd) return;
   evas_object_geometry_get(wd->bx, &bx, &by, NULL, NULL);
   evas_object_geometry_get(it->base, &x, &y, &w, &h);
   elm_smart_scroller_child_region_show(wd->scr, x - bx, y - by, w, h);
}

static void
_item_select(Elm_Toolbar_Item *it)
{
   Elm_Toolbar_Item *it2;
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Object *obj2;
   const Eina_List *l;

   if (!wd) return;
   if ((it->selected) || (it->disabled) || (it->separator)) return;
   EINA_LIST_FOREACH(wd->items, l, it2)
     {
	if (it2->selected)
	  {
	     it2->selected = EINA_FALSE;
	     edje_object_signal_emit(it2->base, "elm,state,unselected", "elm");
	     break;
	  }
     }
   it->selected = EINA_TRUE;
   edje_object_signal_emit(it->base, "elm,state,selected", "elm");
   _item_show(it);
   obj2 = it->obj;
   if(it->menu)
     {
        evas_object_show(it->o_menu);
        evas_object_event_callback_add(it->base, EVAS_CALLBACK_RESIZE,
                                       _menu_move_resize, it);
        evas_object_event_callback_add(it->base, EVAS_CALLBACK_MOVE,
                                       _menu_move_resize, it);

        _menu_move_resize(it, NULL, NULL, NULL);
     }
   if (it->func) it->func((void *)(it->data), it->obj, it);
   evas_object_smart_callback_call(obj2, "clicked", it);
}

static void
_menu_hide(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Toolbar_Item *it = data;
   elm_toolbar_item_unselect_all(it->obj);
}

static void
_menu_del(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   // avoid hide being emitted during object deletion
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_HIDE, _menu_hide, data);
}

static void
_menu_move_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
    Elm_Toolbar_Item *it = data;
    Evas_Coord x,y,w,h;
    Widget_Data *wd = elm_widget_data_get(it->obj);

    if ((!wd) || (!wd->menu_parent)) return;
    evas_object_geometry_get(it->base, &x, &y, &w, &h);
    elm_menu_move(it->o_menu, x, y+h);
}

static void
_item_disable(Elm_Toolbar_Item *it, Eina_Bool disabled)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);

   if (!wd) return;
   if (it->disabled == disabled) return;
   it->disabled = disabled;
   if (it->disabled)
     edje_object_signal_emit(it->base, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(it->base, "elm,state,enabled", "elm");
}

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Toolbar_Item *it;

   if (!wd) return;
   EINA_LIST_FREE(wd->items, it)
     {
	if (it->del_cb) it->del_cb((void *)it->data, it->obj, it);
	eina_stringshare_del(it->label);
	if (it->icon) evas_object_del(it->icon);
	if ((!wd->menu_parent) && (it->o_menu)) evas_object_del(it->o_menu);
	evas_object_del(it->base);
	free(it);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *l;
   Elm_Toolbar_Item *it;
   const char *style = elm_widget_style_get(obj);
   double scale = 0;

   if (!wd) return;
   elm_smart_scroller_object_theme_set(obj, wd->scr, "toolbar", "base", elm_widget_style_get(obj));
   scale = (elm_widget_scale_get(obj) * _elm_config->scale);
//   edje_object_scale_set(wd->scr, scale);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        Evas_Coord mw, mh;

	edje_object_scale_set(it->base, scale);
        if (!it->separator)
          {
             if (it->selected)
               edje_object_signal_emit(it->base, "elm,state,selected", "elm");
             if (it->disabled)
               edje_object_signal_emit(it->base, "elm,state,disabled", "elm");
             _elm_theme_object_set(obj, it->base, "toolbar", "item", style);
             if (it->icon)
               {
                  int ms = 0;

                  ms = ((double)wd->icon_size * _elm_config->scale);
		  evas_object_size_hint_min_set(it->icon, ms, ms);
		  evas_object_size_hint_max_set(it->icon, ms, ms);
                  edje_object_part_swallow(it->base, "elm.swallow.icon",
                                           it->icon);
               }
             edje_object_part_text_set(it->base, "elm.text", it->label);
          }
        else
          _elm_theme_object_set(obj, it->base, "toolbar", "separator", style);

	mw = mh = -1;
	if (!it->separator)
	  elm_coords_finger_size_adjust(1, &mw, 1, &mh);
	edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
	if (!it->separator)
	  elm_coords_finger_size_adjust(1, &mw, 1, &mh);
        evas_object_size_hint_min_set(it->base, mw, mh);
     }
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   Evas_Coord vw = 0, vh = 0;
   Evas_Coord w, h;

   if (!wd) return;
   evas_object_smart_calculate(wd->bx);
   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr),
                             &minw, &minh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;

   evas_object_resize(wd->scr, w, h);

   evas_object_size_hint_min_get(wd->bx, &minw, &minh);
   if (w > minw) minw = w;
   evas_object_resize(wd->bx, minw, minh);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   if (wd->scrollable)
     minw = w - vw;
   else
     minw = minw + (w - vw);
   minh = minh + (h - vh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Coord mw, mh, vw, vh, w, h;
   const Eina_List *l;
   Elm_Toolbar_Item *it;

   if (!wd) return;
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   evas_object_size_hint_min_get(wd->bx, &mw, &mh);
   evas_object_geometry_get(wd->bx, NULL, NULL, &w, &h);
   if (vw >= mw)
     {
	if (w != vw) evas_object_resize(wd->bx, vw, h);
     }
   EINA_LIST_FOREACH(wd->items, l, it)
     {
	if (it->selected)
	  {
	     _item_show(it);
	     break;
	  }
     }
}

static void
_select(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   _item_select(data);
}

static void
_layout(Evas_Object *o, Evas_Object_Box_Data *priv, void *data)
{
   Widget_Data *wd = data;
   if (!wd) return;
   _els_box_layout(o, priv, 1, wd->homogeneous);
}

/**
 * Add a toolbar object to @p parent.
 *
 * @param parent The parent object
 *
 * @return The created object, or NULL on failure
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object *
elm_toolbar_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "toolbar");
   elm_widget_type_set(obj, "toolbar");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, 0);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "toolbar", "base", "default");
   elm_smart_scroller_bounce_allow_set(wd->scr, 1, 0);
   elm_widget_resize_object_set(obj, wd->scr);
   elm_smart_scroller_policy_set(wd->scr,
				 ELM_SMART_SCROLLER_POLICY_AUTO,
				 ELM_SMART_SCROLLER_POLICY_OFF);

   wd->icon_size = 32;
   wd->scrollable = EINA_TRUE;
   wd->homogeneous = EINA_TRUE;
   wd->align = 0.5;

   wd->bx = evas_object_box_add(e);
   evas_object_size_hint_align_set(wd->bx, wd->align, 0.5);
   evas_object_box_layout_set(wd->bx, _layout, wd, NULL);
   elm_widget_sub_object_add(obj, wd->bx);
   elm_smart_scroller_child_set(wd->scr, wd->bx);
   evas_object_show(wd->bx);

   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_RESIZE, _resize, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the icon size (in pixels) for the toolbar.
 *
 * @param obj The toolbar object
 * @param icon_size The icon size in pixels
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_icon_size_set(Evas_Object *obj, int icon_size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (icon_size > 48) return;
   if (wd->icon_size == icon_size) return;
   wd->icon_size = icon_size;
   _theme_hook(obj);
}

/**
 * Get the icon size (in pixels) for the toolbar.
 *
 * @param obj The toolbar object
 * @return The icon size in pixels
 *
 * @ingroup Toolbar
 */
EAPI int
elm_toolbar_icon_size_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->icon_size;
}

/**
 * Add an item to the toolbar.
 *
 * @param obj The toolbar object
 * @param icon The icon object of the item
 * @param label The label of the item
 * @param func The function to call when the item is clicked
 * @param data The data to associate with the item
 *
 * @return The toolbar item, or NULL upon failure
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item *
elm_toolbar_item_add(Evas_Object *obj, Evas_Object *icon, const char *label, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw, mh;
   Elm_Toolbar_Item *it;

   if (!wd) return NULL;
   it = ELM_NEW(Elm_Toolbar_Item);
   if (!it) return NULL;
   wd->items = eina_list_append(wd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon = icon;
   it->func = func;
   it->data = data;
   it->separator = EINA_FALSE;
   it->base = edje_object_add(evas_object_evas_get(obj));
   _elm_theme_object_set(obj, it->base, "toolbar", "item", elm_widget_style_get(obj));
   edje_object_signal_callback_add(it->base, "elm,action,click", "elm",
				   _select, it);
   elm_widget_sub_object_add(obj, it->base);
   if (it->icon)
     {
        int ms = 0;

        ms = ((double)wd->icon_size * _elm_config->scale);
	evas_object_size_hint_min_set(it->icon, ms, ms);
	evas_object_size_hint_max_set(it->icon, ms, ms);
	edje_object_part_swallow(it->base, "elm.swallow.icon", it->icon);
	evas_object_show(it->icon);
	elm_widget_sub_object_add(obj, it->icon);
     }
   edje_object_part_text_set(it->base, "elm.text", it->label);
   mw = mh = -1;
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   edje_object_size_min_restricted_calc(it->base, &mw, &mh, mw, mh);
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   evas_object_size_hint_weight_set(it->base, -1.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(it->base, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(it->base, mw, mh);
   evas_object_box_append(wd->bx, it->base);
   evas_object_show(it->base);
   _sizing_eval(obj);
   return it;
}

/**
 * Get the icon associated with @p item.
 *
 * @param item The toolbar item
 * @return The icon object
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object *
elm_toolbar_item_icon_get(Elm_Toolbar_Item *item)
{
   if (!item) return NULL;
   return item->icon;
}

/**
 * Get the label associated with @p item.
 *
 * @param item The toolbar item
 * @return The label
 *
 * @ingroup Toolbar
 */
EAPI const char *
elm_toolbar_item_label_get(const Elm_Toolbar_Item *item)
{
   if (!item) return NULL;
   return item->label;
}

/**
 * Get the selected state of @p item.
 *
 * @param item The toolbar item
 * @return If true, the item is selected
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool
elm_toolbar_item_selected_get(const Elm_Toolbar_Item *item)
{
   if (!item) return EINA_FALSE;
   return item->selected;
}

/**
 * Set the label associated with @p item.
 *
 * @param item The toolbar item
 * @param label The label of @p item
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_label_set(Elm_Toolbar_Item *item, const char *label)
{
   Evas_Coord mw = -1, mh = -1;

   if (!item) return;
   eina_stringshare_replace(&item->label, label);
   edje_object_part_text_set(item->base, "elm.text", item->label);

   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   edje_object_size_min_restricted_calc(item->base, &mw, &mh, mw, mh);
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   evas_object_size_hint_weight_set(item->base, -1.0, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(item->base, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_min_set(item->base, mw, mh);
}


/**
 * Delete a toolbar item.
 *
 * @param it The toolbar item
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_del(Elm_Toolbar_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   Evas_Object *obj2 = it->obj;

   if ((!wd) || (!it)) return;
   if (it->del_cb) it->del_cb((void *)it->data, it->obj, it);
   wd->items = eina_list_remove(wd->items, it);
   eina_stringshare_del(it->label);
   if (it->icon) evas_object_del(it->icon);
   evas_object_del(it->base);
   free(it);
   _theme_hook(obj2);
}

/**
 * Set the function called when a toolbar item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_del_cb_set(Elm_Toolbar_Item *it, Evas_Smart_Cb func)
{
   it->del_cb = func;
}

/**
 * Select the toolbar item @p item.
 *
 * @param item The toolbar item
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_select(Elm_Toolbar_Item *item)
{
   if (!item) return;
   _item_select(item);
}

/**
 * Get the disabled state of @p item.
 *
 * @param item The toolbar item
 * @return If true, the item is disabled
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool
elm_toolbar_item_disabled_get(Elm_Toolbar_Item *item)
{
   if (!item) return EINA_FALSE;
   return item->disabled;
}

/**
 * Set the disabled state of @p item.
 *
 * @param item The toolbar item
 * @param disabled If true, the item is disabled
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_disabled_set(Elm_Toolbar_Item *item, Eina_Bool disabled)
{
   if (!item) return;
   _item_disable(item, disabled);
}

/**
 * Get the separator state of @p item.
 *
 * @param item The toolbar item
 * @param separator If true, the item is a separator
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_separator_set(Elm_Toolbar_Item *item, Eina_Bool separator)
{
   if (!item) return;
   if (item->separator == separator) return;
   item->separator = separator;
   _theme_hook(item->obj);
}

/**
 * Set the separator state of @p item.
 *
 * @param item The toolbar item
 * @return If true, the item is a separator
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool
elm_toolbar_item_separator_get(Elm_Toolbar_Item *item)
{
   if (!item) return EINA_FALSE;
   return item->separator;
}

/**
 * Set the scrollable state of toolbar @p obj.
 *
 * @param obj The toolbar object
 * @param scrollable If true, the toolbar will be scrollable
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_scrollable_set(Evas_Object *obj, Eina_Bool scrollable)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->scrollable = scrollable;
   _sizing_eval(obj);
}

/**
 * Get the scrollable state of toolbar @p obj.
 *
 * @param obj The toolbar object
 * @return If true, the toolbar is scrollable
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool
elm_toolbar_scrollable_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->scrollable;
}

/**
 * Set the homogenous mode of toolbar @p obj.
 *
 * @param obj The toolbar object
 * @param homogenous If true, the toolbar items will be uniform in size
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_homogenous_set(Evas_Object *obj, Eina_Bool homogenous)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   wd->homogeneous = !!homogenous;
   evas_object_smart_calculate(wd->bx);
}

/**
 * Get the homogenous mode of toolbar @p obj.
 *
 * @param obj The toolbar object
 * @return If true, the toolbar items are uniform in size
 *
 * @ingroup Toolbar
 */
EAPI Eina_Bool
elm_toolbar_homogenous_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return wd->homogeneous;
}

/**
 * Set the parent object of the toolbar menu
 *
 * @param obj The toolbar object
 * @param parent The parent of the menu object
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_menu_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Eina_List *l;
   Elm_Toolbar_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!parent)) return;
   wd->menu_parent = parent;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->o_menu)
          elm_menu_parent_set(it->o_menu, wd->menu_parent);
     }
}

/**
 * Get the parent object of the toolbar menu
 *
 * @param obj The toolbar object
 * @return The parent of the menu object
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object *
elm_toolbar_menu_parent_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->menu_parent;
}

/**
 * Unselect all of the items in the toolbar.
 *
 * @param obj The toolbar object
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_unselect_all(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Toolbar_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->selected)
          {
             it->selected = EINA_FALSE;
             edje_object_signal_emit(it->base, "elm,state,unselected", "elm");
             break;
          }
     }
}

/**
 * Unselect the specified toolbar item.
 *
 * @param item The toolbar item
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_unselect(Elm_Toolbar_Item *item)
{
   if ((!item) || (!item->selected)) return;
   item->selected = EINA_FALSE;
   edje_object_signal_emit(item->base, "elm,state,unselected", "elm");
}

/**
 * Set the alignment of the items.
 *
 * @param obj The toolbar object
 * @param align The new alignment. (left) 0.0 ... 1.0 (right)
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_align_set(Evas_Object *obj, double align)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->align != align)
     evas_object_size_hint_align_set(wd->bx, align, 0.5);
   wd->align = align;
}

/**
 * Get the alignment of the items.
 *
 * @param obj The toolbar object
 * @return The alignment. (left) 0.0 ... 1.0 (right)
 *
 * @ingroup Toolbar
 */
EAPI double
elm_toolbar_align_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return 0.0;
   return wd->align;
}

/**
 * Set whether the toolbar item opens a menu.
 *
 * @param item The toolbar item
 * @param menu If true, @p item will open a menu when selected
 *
 * @ingroup Toolbar
 */
EAPI void
elm_toolbar_item_menu_set(Elm_Toolbar_Item *item, Eina_Bool menu)
{
   Widget_Data *wd = elm_widget_data_get(item->obj);

   if ((!wd) || (!item)) return;
   if (item->menu == menu) return;
   item->menu = menu;
   if (menu)
     {
	item->o_menu = elm_menu_add(item->base);
	if (wd->menu_parent)
	  elm_menu_parent_set(item->o_menu, wd->menu_parent);
	evas_object_event_callback_add(item->o_menu, EVAS_CALLBACK_HIDE,
                                       _menu_hide, item);
	evas_object_event_callback_add(item->o_menu, EVAS_CALLBACK_DEL,
				       _menu_del, item);
     }
   else if (item->o_menu)
     {
	evas_object_del(item->o_menu);
     }
}


/**
 * Get whether the toolbar item opens a menu.
 *
 * @param item The toolbar item
 * @return If true, @p item opens a menu when selected
 *
 * @ingroup Toolbar
 */
EAPI Evas_Object *
elm_toolbar_item_menu_get(Elm_Toolbar_Item *item)
{
   if (!item) return NULL;
   Widget_Data *wd = elm_widget_data_get(item->obj);
   if (!wd) return NULL;
   elm_toolbar_item_menu_set(item, 1);
   return item->o_menu;
}

/**
 * Return a list of all toolbar items.
 *
 * @param obj The toolbar object
 *
 * @return An Eina_List* of the toolbar items in @p obj
 *
 * @ingroup Toolbar
 */
EAPI Eina_List *
elm_toolbar_item_get_all(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->items;
}

/**
 * Return the first toolbar item in the list of toolbar items.
 *
 * @param obj The toolbar object
 *
 * @return The first toolbar item, or NULL on failure
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item *
elm_toolbar_item_get_first(Evas_Object *obj)
{
   Elm_Toolbar_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!(it = eina_list_data_get(wd->items))) return NULL;

   return it;
}

/**
 * Return the last toolbar item in the list of toolbar items.
 *
 * @param obj The toolbar object
 *
 * @return The last toolbar item, or NULL on failure
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item *
elm_toolbar_item_get_last(Evas_Object *obj)
{
   Eina_List *last;
   Elm_Toolbar_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!(last = eina_list_last(wd->items))) return NULL;
   it = eina_list_data_get(last);

   return it;
}

/**
 * Return the next toolbar item (relative to the currently selected
 * toolbar item) in the list of toolbar items.
 *
 * @param obj The toolbar object
 *
 * @return The next toolbar item, or NULL on failure
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item *
elm_toolbar_item_get_next(Evas_Object *obj)
{
   Eina_List *l, *l2;
   Elm_Toolbar_Item *it, *next;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   EINA_LIST_FOREACH_SAFE(wd->items, l, l2, it)
     {
        if (it->selected)
          {
	     if (!(next = eina_list_data_get(l2))) return NULL;
	     return next;
          }
     }
   return NULL;
}

/**
 * Selects the next non-disabled, non-separator toolbar item in the list
 * of toolbar items.
 *
 * @param obj The toolbar object
 *
 * @return The newly selected toolbar item, or NULL on failure
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item *
elm_toolbar_item_select_next(Evas_Object *obj)
{
   Eina_List *l, *l2;
   Elm_Toolbar_Item *it, *next;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->selected)
          {
             EINA_LIST_FOREACH(l, l2, next)
               if ((!next->disabled) && (next->separator))
                 {
                    _item_select(next);
                    return next;
                 }
          }
     }
  return NULL;
}

/**
 * Selects the first non-disabled, non-separator toolbar item in the list
 * of toolbar items.
 *
 * @param obj The toolbar object
 *
 * @return The newly selected toolbar item, or NULL on failure
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item *
elm_toolbar_item_select_first(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Toolbar_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if ((!it->disabled) && (!it->separator))
          {
             _item_select(it);
             return it;
          }
     }

   return NULL;
}

/**
 * Selects the last non-disabled, non-separator toolbar item in the list
 * of toolbar items.
 *
 * @param obj The toolbar object
 *
 * @return The newly selected toolbar item, or NULL on failure
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item *
elm_toolbar_item_select_last(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Toolbar_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   EINA_LIST_REVERSE_FOREACH(wd->items, l, it)
     {
        if ((!it->disabled) && (!it->separator))
          {
             _item_select(it);
             return it;
          }
     }
 
   return NULL;
}

/**
 * Returns a pointer to a toolbar item by its label
 *
 * @param obj The toolbar object
 * @param label The label of the item to find
 *
 * @return The pointer to the toolbar item matching @p label
 * Returns NULL on failure.
 *
 * @ingroup Toolbar
 */
EAPI Elm_Toolbar_Item *
elm_toolbar_item_find_by_label(Evas_Object *obj, const char *label)
{
   Eina_List *l;
   Elm_Toolbar_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   
   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (!strcmp(it->label, label)) return it;
     }

   return NULL;
}
