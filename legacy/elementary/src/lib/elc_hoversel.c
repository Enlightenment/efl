#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Hoversel
 *
 * A hoversel is a button that pops up a list of items (automatically
 * choosing the direction to display) that have a lable and/or an icon to
 * select from. It is a convenience widget to avoid the need to do all the
 * piecing together yourself. It is intended for a small numbr of items in
 * the hoversel menu (no more than 8), though is capable of many more.
 *
 * Signals that you can add callbacks for are:
 *
 * clicked  - the user clicked the hoversel button and popped up the sel
 *
 * selected - an item in the hoversel list is selected. event_info is the item
 * selected - Elm_Hoversel_Item
 *
 * dismissed - the hover is dismissed
 */
typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *btn, *hover;
   Evas_Object *hover_parent;
   Eina_List *items;
};

struct _Elm_Hoversel_Item
{
   Evas_Object *obj;
   const char *label;
   const char *icon_file;
   const char *icon_group;
   Elm_Icon_Type icon_type;
   void (*func) (void *data, Evas_Object *obj, void *event_info);
   void *data;
};

static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   elm_hoversel_hover_end(obj);
   elm_hoversel_hover_parent_set(obj, NULL);
}

static void
_del_hook(Evas_Object *obj)
{
   Elm_Hoversel_Item *it;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_hoversel_hover_end(obj);
   EINA_LIST_FREE(wd->items, it)
     {
	eina_stringshare_del(it->label);
	eina_stringshare_del(it->icon_file);
	eina_stringshare_del(it->icon_group);
	free(it);
     }
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char buf[4096];
   if (!wd) return;
   
   snprintf(buf, sizeof(buf), "hoversel_vertical/%s", elm_widget_style_get(obj));
   elm_button_style_set(wd->btn, buf);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (elm_widget_disabled_get(obj))
     elm_widget_disabled_set(wd->btn, 1);
   else
     elm_widget_disabled_set(wd->btn, 0);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   evas_object_size_hint_min_get(wd->btn, &minw, &minh);
   evas_object_size_hint_max_get(wd->btn, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_hover_clicked(void *data, Evas_Object *obj, void *event_info)
{
   elm_hoversel_hover_end(data);
}

static void
_item_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Hoversel_Item *it = data;
   Evas_Object *obj2 = it->obj;
   elm_hoversel_hover_end(obj2);
   if (it->func) it->func(it->data, obj2, it);
   evas_object_smart_callback_call(obj2, "selected", it);
}

static void
_activate(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Evas_Object *bt, *bx, *ic;
   const Eina_List *l;
   const Elm_Hoversel_Item *it;
   char buf[4096];

   if (elm_widget_disabled_get(obj))
       return;

   wd->hover = elm_hover_add(obj);
   snprintf(buf, sizeof(buf), "hoversel_vertical/%s", elm_widget_style_get(obj));
   elm_hover_style_set(wd->hover, buf);
   evas_object_smart_callback_add(wd->hover, "clicked", _hover_clicked, obj);
   elm_hover_parent_set(wd->hover, wd->hover_parent);
   elm_hover_target_set(wd->hover, wd->btn);

   bx = elm_box_add(wd->hover);
   elm_box_homogenous_set(bx, 1);

   snprintf(buf, sizeof(buf), "hoversel_vertical_entry/%s", elm_widget_style_get(obj));
   EINA_LIST_FOREACH(wd->items, l, it)
     {
	bt = elm_button_add(wd->hover);
	elm_button_style_set(bt, buf);
	elm_button_label_set(bt, it->label);
	if (it->icon_file)
	  {
	     ic = elm_icon_add(obj);
	     elm_icon_scale_set(ic, 0, 1);
	     if (it->icon_type == ELM_ICON_FILE)
	       elm_icon_file_set(ic, it->icon_file, it->icon_group);
	     else if (it->icon_type == ELM_ICON_STANDARD)
	       elm_icon_standard_set(ic, it->icon_file);
	     elm_button_icon_set(bt, ic);
	     evas_object_show(ic);
	  }
	evas_object_size_hint_weight_set(bt, 1.0, 0.0);
	evas_object_size_hint_align_set(bt, -1.0, -1.0);
	elm_box_pack_end(bx, bt);
	evas_object_smart_callback_add(bt, "clicked", _item_clicked, it);
	evas_object_show(bt);
     }

   elm_hover_content_set
     (wd->hover,
      elm_hover_best_content_location_get(wd->hover, ELM_HOVER_AXIS_VERTICAL),
      bx);
   evas_object_show(bx);

   evas_object_show(wd->hover);
   evas_object_smart_callback_call(obj, "clicked", NULL);
}

static void
_button_clicked(void *data, Evas_Object *obj, void *event_info)
{
  _activate(data);
}

static void
_parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->hover_parent = NULL;
}

/**
 * Add a new Hoversel object
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Hoversel
 */
EAPI Evas_Object *
elm_hoversel_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_type_set(obj, "hoversel");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);

   wd->btn = elm_button_add(parent);
   elm_button_style_set(wd->btn, "hoversel_vertical");
   elm_widget_resize_object_set(obj, wd->btn);
   evas_object_event_callback_add(wd->btn, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   evas_object_smart_callback_add(wd->btn, "clicked", _button_clicked, obj);
   _sizing_eval(obj);
   return obj;
}

/**
 * Set the Hover parent
 *
 * Sets the hover parent object. See Hover objects for more information.
 *
 * @param obj The hoversel object
 * @param parent The parent to use
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover_parent)
     evas_object_event_callback_del(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del);
   wd->hover_parent = parent;
   if (wd->hover_parent)
     evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
}

/**
 * Set the hoversel button label
 *
 * This sets the label of the button that is always visible (before it is
 * clicked and expanded). Also see elm_button_label_set().
 *
 * @param obj The hoversel object
 * @param label The label text.
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_label_set(Evas_Object *obj, const char *label)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_button_label_set(wd->btn, label);
}

/**
 * Get the hoversel button label
 *
 * @param obj The hoversel object
 * @return The label text.
 *
 * @ingroup Hoversel
 */
EAPI const char*
elm_hoversel_label_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd || !wd->btn) return NULL;

   return elm_button_label_get(wd->btn);
}

/**
 * Set the icon of the hoversel button
 *
 * Sets the icon of the button that is always visible (before it is clicked
 * and expanded). Also see elm_button_icon_set().
 *
 * @param obj The hoversel object
 * @param icon The icon object
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_button_icon_set(wd->btn, icon);
}

/**
 * Get the icon of the hoversel button
 *
 * Get the icon of the button that is always visible (before it is clicked
 * and expanded). Also see elm_button_icon_get().
 *
 * @param obj The hoversel object
 * @return The icon object
 *
 * @ingroup Hoversel
 */
EAPI Evas_Object *
elm_hoversel_icon_get(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd || !wd->btn) return NULL;
   return elm_button_icon_get(wd->btn);
}

/**
 * Trigger the hoversel popup from code
 *
 * This makes the hoversel popup activate with the items added being listed.
 *
 * @param obj The hoversel object
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_hover_begin(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover) return;
   _activate(obj);
}

/**
 * This ends the hoversel popup.
 *
 * This will close the hoversel popup, making it disappear, if it was active.
 *
 * @param obj The hoversel object
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_hover_end(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover)
     {
	evas_object_del(wd->hover);
	wd->hover = NULL;
	evas_object_smart_callback_call(obj, "dismissed", NULL);
     }
}

/**
 * Remove all the items from the given hoversel object.
 *
 * This will remove all the children items from the hoversel. (should not be
 * called while the hoversel is active).
 * 
 * @param obj The hoversel object
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_clear(Evas_Object *obj)
{
   Elm_Hoversel_Item *it;
   Eina_List *l, *ll;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_LIST_FOREACH_SAFE(wd->items, l, ll, it)
      elm_hoversel_item_del(it);
}

/**
 * Add an item to the hoversel button
 *
 * This adds an item to the hoversel to show when it is clicked. Note: if you
 * need to use an icon from an edje file then use elm_hoversel_item_icon_set()
 * right after the this function, and set icon_file to NULL here.
 *
 * @param obj The hoversel object
 * @param label The text abel to use for the item (NULL if not desired)
 * @param icon_file A image file path on disk to use for the icon  or standard
 * icon name(NULL if not desired)
 * @param icon_type The icon type if relevant
 * @param func Convenience function to call when this item is selected
 * @param data Data to pass to the conveience function
 * @return A handle to the item added.
 *
 * @ingroup Hoversel
 */
EAPI Elm_Hoversel_Item *
elm_hoversel_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, void (*func) (void *data, Evas_Object *obj, void *event_info), const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Elm_Hoversel_Item *it = calloc(1, sizeof(Elm_Hoversel_Item));
   if (!it) return NULL;
   wd->items = eina_list_append(wd->items, it);
   it->obj = obj;
   it->label = eina_stringshare_add(label);
   it->icon_file = eina_stringshare_add(icon_file);
   it->icon_type = icon_type;
   it->func = func;
   it->data = (void *)data;
   return it;
}

/**
 * Delete an item from the hoversel
 *
 * This deletes the item from the hoversel (should not be called while the
 * hoversel is active).
 *
 * @param it The item to delete
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_item_del(Elm_Hoversel_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(it->obj);
   if (!wd) return;
   elm_hoversel_hover_end(it->obj);
   wd->items = eina_list_remove(wd->items, it);
   eina_stringshare_del(it->label);
   eina_stringshare_del(it->icon_file);
   eina_stringshare_del(it->icon_group);
   free(it);
}

/**
 * Get the data pointer passed to the item add function
 *
 * This returns the data pointer supplied with elm_hoversel_item_add() that
 * will be passed to the select function callback.
 *
 * @param it The item to get the data from
 * @return The data pointer set with elm_hoversel_item_add()
 *
 * @ingroup Hoversel
 */
EAPI void *
elm_hoversel_item_data_get(Elm_Hoversel_Item *it)
{
   if (!it) return NULL;
   return it->data;
}

/**
 * Get the text label of an hoversel item
 *
 * This returns the text of the label of the given hoversel item
 *
 * @param it The item to get the label
 * @return The text label of the hoversel item
 *
 * @ingroup Hoversel
 */
EAPI const char *
elm_hoversel_item_label_get(Elm_Hoversel_Item *it)
{
   if (!it) return NULL;
   return it->label;
}

/**
 * Set the icon of the hoversel item
 *
 * This set the icon for the given hoversel item. The icon can be loaded from
 * the standard set, from an image file or from an edje file.
 *
 * @param it The item to set the icon
 * @param icon_file An image file path on disk to use for the icon or standard
 * icon name
 * @param icon_group The edje group to use if @p icon_file is an edje file. Set this
 * to NULL if the icon is not an edje file
 * @param icon_type The icon type
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_item_icon_set(Elm_Hoversel_Item *it, const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type)
{
   if (!it) return;

   if (it->icon_file) eina_stringshare_del(it->icon_file);
   it->icon_file = eina_stringshare_add(icon_file);

   if (it->icon_group) eina_stringshare_del(it->icon_group);
   it->icon_group = eina_stringshare_add(icon_group);

   it->icon_type = icon_type;
}

/**
 * Get the icon object of the hoversel item
 *
 * @param it The item to get the icon from
 * @param icon_file The image file path on disk used for the icon or standard
 * icon name. return
 * @param icon_group The edje group used if @p icon_file is an edje file. NULL
 * if the icon is not an edje file. return
 * @param icon_type The icon type. return
 *
 * @ingroup Hoversel
 */
EAPI void
elm_hoversel_item_icon_get(Elm_Hoversel_Item *it, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type)
{
   if (!it) return;

   if (icon_file) *icon_file = it->icon_file;
   if (icon_group) *icon_group = it->icon_group;
   if (icon_type) *icon_type = it->icon_type;
}
