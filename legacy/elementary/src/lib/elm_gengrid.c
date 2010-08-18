#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Gengrid Gengrid
 *
 * This widget aims to position objects in a grid layout while actually
 * building only the visible ones, using the same idea as genlist: the user
 * define a class for each item, specifying functions that will be called at
 * object creation and deletion.
 *
 * Signals that you can add callbacks for are:
 *
 * clicked - The user has double-clicked a item. The event_info parameter is
 * the Gengrid item that was double-clicked.
 *
 * selected - The user has made an item selected. The event_info parameter is
 * the Gengrid item that was selected.
 *
 * unselected - The user has made an item unselected. The event_info parameter
 * is the Gengrid item that was unselected.
 *
 * realized - This is called when the item in the Gengrid is created as a real
 * evas object. event_info is the Gengrid item that was created. The object may be
 * deleted at any time, so it is up to the caller to not use the object pointer
 * from elm_gengrid_item_object_get() in a way where it may point to
 * freed objects.
 *
 * drag,start,up - Called when the item in the Gengrid has been dragged (not
 * scrolled) up.
 *
 * drag,start,down - Called when the item in the Gengrid has been dragged (not
 * scrolled) down.
 *
 * drag,start,left - Called when the item in the Gengrid has been dragged (not
 * scrolled) left.
 *
 * drag,start,right - Called when the item in the Gengrid has been dragged (not
 * scrolled) right.
 *
 * drag,stop - Called when the item in the Gengrid has stopped being dragged.
 *
 * drag - Called when the item in the Gengrid is being dragged.
 *
 * scroll - called when the content has been scrolled (moved).
 *
 * scroll,drag,start - called when dragging the content has started.
 *
 * scroll,drag,stop - called when dragging the content has stopped.
 *
 *
 * A item in the Gengrid can have 0 or more text labels (they can be regular text
 * or textblock - that's up to the style to determine), 0 or more icons (which
 * are simply objects swallowed into the Gengrid item) and 0 or more boolean states
 * that can be used for check, radio or other indicators by the edje theme style.
 * A item may be one of several styles (Elementary provides 1 by default -
 * "default", but this can be extended by system or application custom
 * themes/overlays/extensions).
 *
 * In order to implement the ability to add and delete items on the fly, Gengrid
 * implements a class/callback system where the application provides a structure
 * with information about that type of item (Gengrid may contain multiple different
 * items with different classes, states and styles). Gengrid will call the functions
 * in this struct (methods) when a item is "realized" (that is created
 * dynamically while scrolling). All objects will simply be deleted when no
 * longer needed with evas_object_del(). The Elm_GenGrid_Item_Class structure
 * contains the following members:
 *
 * item_style - This is a constant string and simply defines the name of the
 * item style. It must be specified and the default should be "default".
 *
 * func.label_get - This function is called when an actual item object is
 * created. The data parameter is the one passed to elm_gengrid_item_append()
 * and related item creation functions. The obj parameter is the Gengrid object and
 * the part parameter is the string name of the text part in the edje design that
 * is listed as one of the possible labels that can be set. This function must
 * return a strdup'()ed string as the caller will free() it when done.
 *
 * func.icon_get - This function is called when an actual item object is
 * created. The data parameter is the one passed to elm_gengrid_item_append()
 * and related item creation functions. The obj parameter is the Gengrid object and
 * the part parameter is the string name of the icon part in the edje design that
 * is listed as one of the possible icons that can be set. This must return NULL
 * for no object or a valid object. The object will be deleted by Gengrid on
 * shutdown or when the item is unrealized.
 *
 * func.state_get - This function is called when an actual item object is
 * created. The data parameter is the one passed to elm_gengrid_item_append()
 * and related item creation functions. The obj parameter is the Gengrid object and
 * the part parameter is the string name of th state part in the edje design that
 * is listed as one of the possible states that can be set. Return 0 for false
 * and 1 for true. Gengrid will emit a signal to the edje object with
 * "elm,state,XXX,active" "elm" when true (the default is false), where XXX is
 * the name of the part.
 *
 * func.del - This is called when elm_gengrid_item_del() is called on a
 * item or elm_gengrid_clear() is called on the Gengrid. This is intended for
 * use when actual Gengrid items are deleted, so any backing data attached to the
 * item (e.g. its data parameter on creation) can be deleted.
 *
 * If the application wants multiple items to be able to be selected,
 * elm_gengrid_multi_select_set() can enable this. If the Gengrid is
 * single-selection only (the default), then elm_gengrid_select_item_get()
 * will return the selected item, if any, or NULL if none is selected. If the
 * Gengrid is multi-select then elm_gengrid_selected_items_get() will return a
 * list (that is only valid as long as no items are modified (added, deleted,
 * selected or unselected).
 *
 * If a item changes (state of boolean changes, label or icons change), then use
 * elm_gengrid_item_update() to have Gengrid update the item with the new
 * state. Gengrid will re-realize the item thus call the functions in the
 * _Elm_Gengrid_Item_Class for that item.
 *
 * To programmatically (un)select a item use elm_gengrid_item_selected_set().
 * To get its selected state use elm_gengrid_item_selected_get(). To make a
 * item disabled (unable to be selected and appear differently) use
 * elm_gengrid_item_disable_set() to set this and
 * elm_gengrid_item_disable_get() to get the disabled state.
 *
 * Cells will only call their selection func and callback when first becoming
 * selected. Any further clicks will do nothing, unless you enable always
 * select with elm_gengrid_always_select_mode_set(). This means event if
 * selected, every click will make the selected callbacks be called.
 * elm_gengrid_no_select_mode_set() will turn off the ability to select
 * items entirely and they will neither appear selected nor call selected
 * callback function.
 *
 * Remember that you can create new styles and add your own theme augmentation
 * per application with elm_theme_extension_add(). If you absolutely must have a
 * specific style that overrides any theme the user or system sets up you can use
 * elm_theme_overlay_add() to add such a file.
 *
 * --
 * TODO:
 *  * Handle non-homogeneous objects too.
 */

typedef struct _Widget_Data Widget_Data;
typedef struct _Pan Pan;

#define PRELOAD 1

struct _Elm_Gengrid_Item
{
   Evas_Object *base, *spacer;
   const Elm_Gengrid_Item_Class *gic;
   Ecore_Timer *long_timer;
   Widget_Data *wd;
   Eina_List *labels, *icons, *states, *icon_objs;
   const void *data;
   struct
     {
	Evas_Smart_Cb func;
	const void *data;
     } func;

   Evas_Coord x, y, dx, dy;
   int relcount;

   Eina_Bool want_unrealize : 1;
   Eina_Bool realized : 1;
   Eina_Bool dragging : 1;
   Eina_Bool down : 1;
   Eina_Bool delete_me : 1;
   Eina_Bool display_only : 1;
   Eina_Bool disabled : 1;
   Eina_Bool selected : 1;
   Eina_Bool hilighted : 1;
   Eina_Bool walking : 1;
};

struct _Widget_Data
{
   Evas_Object *self, *scr;
   Evas_Object *pan_smart;
   Pan *pan;
   Eina_List *items;
   Ecore_Job *calc_job;
   Eina_List *selected;
   double align_x, align_y;

   Evas_Coord pan_x, pan_y;
   Evas_Coord item_width, item_height;	/* Each item size */
   Evas_Coord minw, minh;		/* Total obj size */
   unsigned int nmax;

   Eina_Bool horizontal : 1;
   Eina_Bool on_hold : 1;
   Eina_Bool longpressed : 1;
   Eina_Bool multi : 1;
   Eina_Bool no_select : 1;
   Eina_Bool wasselected : 1;
   Eina_Bool always_select : 1;
};

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data *wd;
};

static const char *widtype = NULL;
static void _item_hilight(Elm_Gengrid_Item *item);
static void _item_unrealize(Elm_Gengrid_Item *item);
static void _item_select(Elm_Gengrid_Item *item);
static void _item_unselect(Elm_Gengrid_Item *item);

static Evas_Smart_Class _pan_sc = EVAS_SMART_CLASS_INIT_VERSION;

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_object_theme_set(obj, wd->scr, "gengrid", "base",
                                       elm_widget_style_get(obj));
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_gengrid_clear(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
   free(wd);
}

static void
_mouse_move(void *data, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info)
{
   Elm_Gengrid_Item *item = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
	if (!item->wd->on_hold) 
	  {
	     item->wd->on_hold = EINA_TRUE;
             if (!item->wd->wasselected)
               _item_unselect(item);
	  }
     }
   if ((item->dragging) && (item->down))
     {
	if (item->long_timer)
	  {
	     ecore_timer_del(item->long_timer);
	     item->long_timer = NULL;
	  }
	evas_object_smart_callback_call(item->wd->self, "drag", item);
	return;
     }
   if ((!item->down) || (item->wd->longpressed))
     {
	if (item->long_timer)
	  {
	     ecore_timer_del(item->long_timer);
	     item->long_timer = NULL;
	  }
	return;
     }
   if (!item->display_only)
     elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - item->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   dy = y - item->dy;
   ady = dy;
   if (ady < 0) ady = -dy;
   minw /= 2;
   minh /= 2;
   if ((adx > minw) || (ady > minh))
     {
	item->dragging = 1;
	if (item->long_timer)
	  {
	     ecore_timer_del(item->long_timer);
	     item->long_timer = NULL;
	  }
	if (!item->wd->wasselected)
	  _item_unselect(item);
	if (dy < 0)
	  {
	     if (ady > adx)
	       evas_object_smart_callback_call(item->wd->self, "drag,start,up",
					       item);
	     else
	       {
		  if (dx < 0)
		    evas_object_smart_callback_call(item->wd->self,
						    "drag,start,left", item);
	       }
	  }
	else
	  {
	     if (ady > adx)
	       evas_object_smart_callback_call(item->wd->self,
					       "drag,start,down", item);
	     else
	       {
		  if (dx < 0)
		    evas_object_smart_callback_call(item->wd->self,
						    "drag,start,left", item);
		  else
		    evas_object_smart_callback_call(item->wd->self,
						    "drag,start,right", item);
	       }
	  }
     }
}

static Eina_Bool
_long_press(void *data)
{
   Elm_Gengrid_Item *item = data;

   item->long_timer = NULL;
   if ((item->disabled) || (item->dragging)) return ECORE_CALLBACK_CANCEL;
   item->wd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(item->wd->self, "longpressed", item);
   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down(void *data, Evas *evas __UNUSED__, Evas_Object *obj, void *event_info)
{
   Elm_Gengrid_Item *item = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y;

   if (ev->button != 1) return;
   item->down = 1;
   item->dragging = 0;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   item->dx = ev->canvas.x - x;
   item->dy = ev->canvas.y - y;
   item->wd->longpressed = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) item->wd->on_hold = EINA_TRUE;
   else item->wd->on_hold = EINA_FALSE;
   item->wd->wasselected = item->selected;
   _item_hilight(item);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(item->wd->self, "clicked", item);
   if (item->long_timer) ecore_timer_del(item->long_timer);
   if (item->realized)
     item->long_timer = ecore_timer_add(1.0, _long_press, item);
   else
     item->long_timer = NULL;
}

static void
_mouse_up(void *data, Evas *evas __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Elm_Gengrid_Item *item = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool dragged = EINA_FALSE;

   if (ev->button != 1) return;
   item->down = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) item->wd->on_hold = EINA_TRUE;
   else item->wd->on_hold = EINA_FALSE;
   if (item->long_timer)
     {
	ecore_timer_del(item->long_timer);
	item->long_timer = NULL;
     }
   if (item->dragging)
     {
	item->dragging = EINA_FALSE;
	evas_object_smart_callback_call(item->wd->self, "drag,stop", item);
	dragged = EINA_TRUE;
     }
   if (item->wd->on_hold)
     {
	item->wd->longpressed = EINA_FALSE;
	item->wd->on_hold = EINA_FALSE;
	return;
     }
   if (item->wd->longpressed)
     {
	item->wd->longpressed = EINA_FALSE;
	if (!item->wd->wasselected) _item_unselect(item);
	item->wd->wasselected = EINA_FALSE;
	return;
     }
   if (dragged)
     {
	if (item->want_unrealize) _item_unrealize(item);
     }
   if ((item->disabled) || dragged) return;
   if (item->wd->multi)
     {
	if (!item->selected)
	  {
	     _item_hilight(item);
	     _item_select(item);
	  }
	else _item_unselect(item);
     }
   else
     {
	if (!item->selected)
	  {
	     while (item->wd->selected) 
	       _item_unselect(item->wd->selected->data);
	  }
	else
	  {
	     const Eina_List *l, *l_next;
	     Elm_Gengrid_Item *item2;

	     EINA_LIST_FOREACH_SAFE(item->wd->selected, l, l_next, item2)
	       if (item2 != item) _item_unselect(item2);
	  }
	_item_hilight(item);
	_item_select(item);
     }
}

static void
_item_hilight(Elm_Gengrid_Item *item)
{
   if ((item->wd->no_select) || (item->delete_me) || (item->hilighted)) return;
   edje_object_signal_emit(item->base, "elm,state,selected", "elm");
   item->hilighted = EINA_TRUE;
}

static void
_item_realize(Elm_Gengrid_Item *item)
{
   char buf[1024];
   char style[1024];

   if ((item->realized) || (item->delete_me)) return;
   item->base = edje_object_add(evas_object_evas_get(item->wd->self));
   edje_object_scale_set(item->base, elm_widget_scale_get(item->wd->self) *
			 _elm_config->scale);
   evas_object_smart_member_add(item->base, item->wd->pan_smart);
   elm_widget_sub_object_add(item->wd->self, item->base);
   snprintf(style, sizeof(style), "item/%s",
	    item->gic->item_style ? item->gic->item_style : "default");
   _elm_theme_object_set(item->wd->self, item->base, "gengrid", style,
                         elm_widget_style_get(item->wd->self));
   item->spacer = evas_object_rectangle_add(evas_object_evas_get(item->wd->self));
   evas_object_color_set(item->spacer, 0, 0, 0, 0);
   elm_widget_sub_object_add(item->wd->self, item->spacer);
   evas_object_size_hint_min_set(item->spacer, 2 * _elm_config->scale, 1);
   edje_object_part_swallow(item->base, "elm.swallow.pad", item->spacer);

   if (item->gic->func.label_get)
     {
	const Eina_List *l;
	const char *key;

	item->labels = _elm_stringlist_get(edje_object_data_get(item->base,
							        "labels"));
	EINA_LIST_FOREACH(item->labels, l, key)
	  {
	     char *s = item->gic->func.label_get(item->data, item->wd->self,
						 l->data);
	     if (s)
	       {
		  edje_object_part_text_set(item->base, l->data, s);
		  free(s);
	       }
	  }
     }

   if (item->gic->func.icon_get)
     {
	const Eina_List *l;
	const char *key;

	item->icons = _elm_stringlist_get(edje_object_data_get(item->base,
							       "icons"));
	EINA_LIST_FOREACH(item->icons, l, key)
	  {
	     Evas_Object *ic = item->gic->func.icon_get(item->data,
							item->wd->self,
						        l->data);
	     if (ic)
	       {
		  item->icon_objs = eina_list_append(item->icon_objs, ic);
		  edje_object_part_swallow(item->base, key, ic);
		  evas_object_show(ic);
		  elm_widget_sub_object_add(item->wd->self, ic);
	       }
	  }
     }

   if (item->gic->func.state_get)
     {
	const Eina_List *l;
	const char *key;

	item->states = _elm_stringlist_get(edje_object_data_get(item->base,
							        "states"));
	EINA_LIST_FOREACH(item->states, l, key)
	  {
	     Eina_Bool on = item->gic->func.state_get(item->data,
						      item->wd->self, l->data);
	     if (on)
	       {
		  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
		  edje_object_signal_emit(item->base, buf, "elm");
	       }
	  }
     }

   if ((!item->wd->item_width) && (!item->wd->item_height))
     {
	edje_object_size_min_restricted_calc(item->base,
					     &item->wd->item_width, &item->wd->item_height,
					     item->wd->item_width, item->wd->item_height);
	elm_coords_finger_size_adjust(1, &item->wd->item_width,
				      1, &item->wd->item_height);
     }

   evas_object_event_callback_add(item->base, EVAS_CALLBACK_MOUSE_DOWN,
				  _mouse_down, item);
   evas_object_event_callback_add(item->base, EVAS_CALLBACK_MOUSE_UP,
				  _mouse_up, item);
   evas_object_event_callback_add(item->base, EVAS_CALLBACK_MOUSE_MOVE,
				  _mouse_move, item);

   if (item->selected)
     edje_object_signal_emit(item->base, "elm,state,selected", "elm");
   if (item->disabled)
     edje_object_signal_emit(item->base, "elm,state,disabled", "elm");

   evas_object_show(item->base);
   item->realized = EINA_TRUE;
   item->want_unrealize = EINA_FALSE;
}

static void
_item_unrealize(Elm_Gengrid_Item *item)
{
   Evas_Object *icon;

   if (!item->realized) return;
   if (item->long_timer)
     {
	ecore_timer_del(item->long_timer);
	item->long_timer = NULL;
     }
   evas_object_del(item->base);
   item->base = NULL;
   evas_object_del(item->spacer);
   item->spacer = NULL;
   _elm_stringlist_free(item->labels);
   item->labels = NULL;
   _elm_stringlist_free(item->icons);
   item->icons = NULL;
   _elm_stringlist_free(item->states);

   EINA_LIST_FREE(item->icon_objs, icon)
     evas_object_del(icon);

   item->states = NULL;
   item->realized = EINA_FALSE;
   item->want_unrealize = EINA_FALSE;
}

static void
_item_place(Elm_Gengrid_Item *item, Evas_Coord cx, Evas_Coord cy)
{
   Evas_Coord x, y, ox, oy, cvx, cvy, cvw, cvh;
   Evas_Coord tch, tcw, alignw = 0, alignh = 0, vw, vh;

   item->x = cx;
   item->y = cy;
   evas_object_geometry_get(item->wd->pan_smart, &ox, &oy, &vw, &vh);
   evas_output_viewport_get(evas_object_evas_get(item->wd->self),
			    &cvx, &cvy, &cvw, &cvh);

   /* Preload rows/columns at each side of the Gengrid */
   cvx = ox - PRELOAD * item->wd->item_width;
   cvy = oy - PRELOAD * item->wd->item_height;
   cvw = vw + 2 * PRELOAD * item->wd->item_width;
   cvh = vh + 2 * PRELOAD * item->wd->item_height;

   tch = ((vh/item->wd->item_height)*item->wd->item_height);
   alignh = (vh - tch)*item->wd->align_y;

   tcw = ((vw/item->wd->item_width)*item->wd->item_width);
   alignw = (vw - tcw)*item->wd->align_x;

   if ((item->wd->horizontal) && (item->wd->minw < vw))
     {
        int columns;

        columns = eina_list_count(item->wd->items)/(vh/item->wd->item_height);
        if (eina_list_count(item->wd->items) % (vh/item->wd->item_height))
	  columns++;

        tcw = item->wd->item_width * columns;
	alignw = (vw - tcw)*item->wd->align_x;
     }
   else if ((item->wd->horizontal) && (item->wd->minw > vw))
     alignw = 0;
   if ((!item->wd->horizontal) && (item->wd->minh < vh))
     {
        int rows;

        rows = eina_list_count(item->wd->items)/(vw/item->wd->item_width);
        if (eina_list_count(item->wd->items) % (vw/item->wd->item_width))
	  rows++;

        tch = item->wd->item_height * rows;
        alignh = (vh - tch)*item->wd->align_y;
     }
   else if ((!item->wd->horizontal) && (item->wd->minh > vh))
     alignh = 0;
   x = cx * item->wd->item_width - item->wd->pan_x + ox + alignw;
   y = cy * item->wd->item_height - item->wd->pan_y + oy + alignh;

   if (ELM_RECTS_INTERSECT(x, y, item->wd->item_width, item->wd->item_height,
			   cvx, cvy, cvw, cvh))
     {
	Eina_Bool was_realized = item->realized;

	_item_realize(item);
	if (!was_realized)
	  evas_object_smart_callback_call(item->wd->self, "realized", item);
	evas_object_move(item->base, x, y);
	evas_object_resize(item->base, item->wd->item_width,
			   item->wd->item_height);
     }
   else
     _item_unrealize(item);
}

static Elm_Gengrid_Item *
_item_create(Widget_Data *wd, const Elm_Gengrid_Item_Class *gic, const void *data, Evas_Smart_Cb func, const void *func_data)
{
   Elm_Gengrid_Item *item;

   item = calloc(1, sizeof(*item));
   if (!item) return NULL;
   item->wd = wd;
   item->gic = gic;
   item->data = data;
   item->func.func = func;
   item->func.data = func_data;
   return item;
}

static void
_item_del(Elm_Gengrid_Item *item)
{
   if (item->selected)
     item->wd->selected = eina_list_remove(item->wd->selected, item);
   if (item->realized) _item_unrealize(item);
   if ((!item->delete_me) && (item->gic->func.del))
     item->gic->func.del(item->data, item->wd->self);
   item->delete_me = EINA_TRUE;
   item->wd->items = eina_list_remove(item->wd->items, item);
   if (item->long_timer) ecore_timer_del(item->long_timer);
   free(item);
}

static void
_item_select(Elm_Gengrid_Item *item)
{
   if ((item->wd->no_select) || (item->delete_me)) return;
   if (item->selected)
     {
	if (item->wd->always_select) goto call;
	return;
     }
   item->selected = EINA_TRUE;
   item->wd->selected = eina_list_append(item->wd->selected, item);
call:
   item->walking++;
   if (item->func.func) 
     item->func.func((void *)item->func.data, item->wd->self, item);
   if (!item->delete_me)
     evas_object_smart_callback_call(item->wd->self, "selected", item);
   item->walking--;
   if ((item->walking == 0) && (item->delete_me))
     if (item->relcount == 0) _item_del(item);
}

static void
_item_unselect(Elm_Gengrid_Item *item)
{
   if ((item->delete_me) || (!item->hilighted)) return;
   edje_object_signal_emit(item->base, "elm,state,unselected", "elm");
   item->hilighted = EINA_FALSE;
   if (item->selected)
     {
	item->selected = EINA_FALSE;
	item->wd->selected = eina_list_remove(item->wd->selected, item);
	evas_object_smart_callback_call(item->wd->self, "unselected", item);
     }
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Evas_Coord minw = 0, minh = 0, nmax = 0, cvw, cvh;
   int count;

   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &cvw, &cvh);
   if ((wd->horizontal) && (wd->item_height))
     nmax = cvh / wd->item_height;
   else if (wd->item_width)
     nmax = cvw / wd->item_width;

   if (nmax)
     {
	count = eina_list_count(wd->items);
	if (wd->horizontal)
	  {
	     minw = ceil(count  / (float)nmax) * wd->item_width;
	     minh = nmax * wd->item_height;
	  }
	else
	  {
	     minw = nmax * wd->item_width;
	     minh = ceil(count / (float)nmax) * wd->item_height;
	  }
     }

   if ((minw != wd->minw) || (minh != wd->minh))
     {
	wd->minh = minh;
	wd->minw = minw;
	evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
     }

   wd->nmax = nmax;
   wd->calc_job = NULL;
   evas_object_smart_changed(wd->pan_smart);
}

static void
_pan_add(Evas_Object *obj)
{
   Pan *sd;
   Evas_Object_Smart_Clipped_Data *cd;

   _pan_sc.add(obj);
   cd = evas_object_smart_data_get(obj);
   sd = ELM_NEW(Pan);
   if (!sd) return;
   sd->__clipped_data = *cd;
   free(cd);
   evas_object_smart_data_set(obj, sd);
}

static void
_pan_del(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);

   if (!sd) return;
   _pan_sc.del(obj);
}

static void
_pan_set(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_pan_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_child_size_get(Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_max_get(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   if (!sd) return;
   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if (x)
     *x = (ow < sd->wd->minw) ? sd->wd->minw - ow : 0;
   if (y)
     *y = (oh < sd->wd->minh) ? sd->wd->minh - oh : 0;
}

static void
_pan_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

static void
_pan_calculate(Evas_Object *obj)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord cx = 0, cy = 0;
   Eina_List *l;
   Elm_Gengrid_Item *item;

   if (!sd) return;
   if (!sd->wd->nmax) return;

   EINA_LIST_FOREACH(sd->wd->items, l, item)
     {
	_item_place(item, cx, cy);
	if (sd->wd->horizontal)
	  {
	     cy = (cy + 1) % sd->wd->nmax;
	     if (!cy) cx++;
	  }
	else
	  {
	     cx = (cx + 1) % sd->wd->nmax;
	     if (!cx) cy++;
	  }
     }
}

static void
_pan_move(Evas_Object *obj, Evas_Coord x __UNUSED__, Evas_Coord y __UNUSED__)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

static void
_hold_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_scr_drag_start(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "scroll,drag,start", NULL);
}

static void
_scr_drag_stop(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "scroll,drag,stop", NULL);
}

static void
_scr_scroll(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, "scroll", NULL);
}

/**
 * Add a new Gengrid object.
 *
 * @param parent The parent object.
 * @return  The new object or NULL if it cannot be created.
 *
 * @see elm_gengrid_item_size_set()
 * @see elm_gengrid_horizontal_set()
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_del()
 * @see elm_gengrid_clear()
 *
 * @ingroup Gengrid
 */
EAPI Evas_Object *
elm_gengrid_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   static Evas_Smart *smart = NULL;

   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   ELM_SET_WIDTYPE(widtype, "gengrid");
   elm_widget_type_set(obj, "gengrid");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "gengrid", "base", "default");
   elm_widget_resize_object_set(obj, wd->scr);

   evas_object_smart_callback_add(wd->scr, "drag,start", _scr_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scr_drag_stop, obj);
   evas_object_smart_callback_add(wd->scr, "scroll", _scr_scroll, obj);

   elm_smart_scroller_bounce_allow_set(wd->scr, 1, 1);

   wd->self = obj;
   wd->align_x = 0.5;
   wd->align_y = 0.5;

   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   if (!smart)
     {
	static Evas_Smart_Class sc;

	evas_object_smart_clipped_smart_set(&_pan_sc);
	sc = _pan_sc;
	sc.name = "elm_gengrid_pan";
	sc.version = EVAS_SMART_CLASS_VERSION;
	sc.add = _pan_add;
	sc.del = _pan_del;
	sc.resize = _pan_resize;
	sc.move = _pan_move;
	sc.calculate = _pan_calculate;
	smart = evas_smart_class_new(&sc);
     }
   if (smart)
     {
	wd->pan_smart = evas_object_smart_add(e, smart);
	wd->pan = evas_object_smart_data_get(wd->pan_smart);
	wd->pan->wd = wd;
     }

   elm_smart_scroller_extern_pan_set(wd->scr, wd->pan_smart,
				     _pan_set, _pan_get,
				     _pan_max_get, _pan_child_size_get);

   return obj;
}

/**
 * Set the size for the item of the Gengrid.
 *
 * @param obj The Gengrid object.
 * @param w The item's width.
 * @param h The item's height;
 *
 * @see elm_gengrid_item_size_get()
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_size_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((wd->item_width == w) && (wd->item_height == h)) return;
   wd->item_width = w;
   wd->item_height = h;
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

/**
 * Get the size of the item of the Gengrid.
 *
 * @param obj The Gengrid object.
 * @param w Pointer to the item's width.
 * @param h Pointer to the item's height.
 *
 * @see elm_gengrid_item_size_get()
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_size_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (w) *w = wd->item_width;
   if (h) *h = wd->item_height;
}

/**
 * Set item's alignment within the scroller.
 *
 * @param obj The Gengrid object.
 * @param align_x The x alignment (0 <= x <= 1).
 * @param align_y The y alignment (0 <= y <= 1).
 *
 * @see elm_gengrid_align_get()
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_align_set(Evas_Object *obj, double align_x, double align_y)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (align_x > 1.0)
     align_x = 1.0;
   else if (align_x < 0.0)
     align_x = 0.0;
   wd->align_x = align_x;

   if (align_y > 1.0)
     align_y = 1.0;
   else if (align_y < 0.0)
     align_y = 0.0;
   wd->align_y = align_y;
}

/**
 * Get the alignenment set for the Gengrid object.
 *
 * @param obj The Gengrid object.
 * @param align_x Pointer to x alignenment.
 * @param align_y Pointer to y alignenment.
 *
 * @see elm_gengrid_align_set()
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_align_get(const Evas_Object *obj, double *align_x, double *align_y)
{
    ELM_CHECK_WIDTYPE(obj, widtype);
    Widget_Data *wd = elm_widget_data_get(obj);
    if (align_x) *align_x = wd->align_x;
    if (align_y) *align_y = wd->align_y;
}

/**
 * Add item to the end of the Gengrid.
 *
 * @param obj The Gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param func Convenience function called when item is selected.
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible.
 *
 * @see elm_gengrid_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_item_append(Evas_Object *obj, const Elm_Gengrid_Item_Class *gic,
			   const void *data, Evas_Smart_Cb func,
			   const void *func_data)
{
   Elm_Gengrid_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = _item_create(wd, gic, data, func, func_data);
   if (!item) return NULL;

   wd->items = eina_list_append(wd->items, item);
   wd->no_select = EINA_FALSE;

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return item;
}

/**
 * Remove a item from the Gengrid.
 *
 * @param item The item to be removed.
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise.
 *
 * @see elm_gengrid_clear() to remove all items of the Gengrid.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_del(Elm_Gengrid_Item *item)
{
   if (!item) return;
   if ((item->relcount > 0) || (item->walking > 0))
     {
	item->delete_me = EINA_TRUE;
	if (item->selected)
	  item->wd->selected = eina_list_remove(item->wd->selected, item);
	if (item->gic->func.del) 
	  item->gic->func.del(item->data, item->wd->self);
	return;
     }

   _item_del(item);

   if (item->wd->calc_job) ecore_job_del(item->wd->calc_job);
   item->wd->calc_job = ecore_job_add(_calc_job, item->wd);
}

/**
 * Set for what direction the Gengrid will expand.
 *
 * @param obj The Gengrid object.
 * @param setting If @c EINA_TRUE the Gengrid will expand horizontally or
 * vertically if @c EINA_FALSE.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_horizontal_set(Evas_Object *obj, Eina_Bool setting)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (setting == wd->horizontal) return;
   wd->horizontal = setting;

   /* Update the items to conform to the new layout */
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

/**
 * Clear the Gengrid
 *
 * This clears all items in the Gengrid, leaving it empty.
 *
 * @param obj The Gengrid object.
 *
 * @see elm_gengrid_item_del() to remove just one item.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_clear(Evas_Object *obj)
{
   Eina_List *l, *l_next;
   Elm_Gengrid_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->calc_job)
     {
	ecore_job_del(wd->calc_job);
	wd->calc_job = NULL;
     }

   EINA_LIST_FOREACH_SAFE(wd->items, l, l_next, item)
     {
	if (item->realized) _item_unrealize(item);
	if (item->gic->func.del) item->gic->func.del(item->data, wd->self);
	if (item->long_timer) ecore_timer_del(item->long_timer);
	free(item);
	wd->items = eina_list_remove_list(wd->items, l);
     }

   if (wd->selected)
     {
	eina_list_free(wd->selected);
	wd->selected = NULL;
     }

   wd->pan_x = 0;
   wd->pan_y = 0;
   wd->minw = 0;
   wd->minh = 0;
   evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
   evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
}

/**
 * Get the real evas object of the Gengrid item
 *
 * This returns the actual evas object used for the specified Gengrid item.
 * This may be NULL as it may not be created, and may be deleted at any time
 * by Gengrid. Do not modify this object (move, resize, show, hide etc.) as
 * Gengrid is controlling it. This function is for querying, emitting custom
 * signals or hooking lower level callbacks for events. Do not delete this
 * object under any circumstances.
 *
 * @param item The Gengrid item.
 * @return the evas object associated to this item.
 *
 * @see elm_gengrid_item_data_get()
 *
 * @ingroup Gengrid
 */
EAPI const Evas_Object *
elm_gengrid_item_object_get(Elm_Gengrid_Item *item)
{
   if (!item) return NULL;
   return item->base;
}

/**
 * Returns the data associated to a item
 *
 * This returns the data value passed on the elm_gengrid_item_append() and
 * related item addition calls.
 *
 * @param item The Gengrid item.
 * @return the data associated to this item.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_object_get()
 *
 * @ingroup Gengrid
 */
EAPI void *
elm_gengrid_item_data_get(Elm_Gengrid_Item *item)
{
   if (!item) return NULL;
   return (void *)item->data;
}

/**
 * Get the item's coordinates.
 *
 * This returns the logical position of the item whithin the Gengrid.
 *
 * @param item The Gengrid item.
 * @param x The x-axis coordinate pointer.
 * @param y The y-axis coordinate pointer.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_pos_get(const Elm_Gengrid_Item *item, unsigned int *x, unsigned int *y)
{
   if (!item) return;
   if (x) *x = item->x;
   if (y) *y = item->y;
}

/**
 * Enable or disable multi-select in the Gengrid.
 *
 * This enables (EINA_TRUE) or disables (EINA_FALSE) multi-select in the Gengrid.
 * This allows more than 1 item to be selected.
 *
 * @param obj The Gengrid object.
 * @param multi Multi-select enabled/disabled
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_multi_select_set(Evas_Object *obj, Eina_Bool multi)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->multi = multi;
}

/**
 * Get if multi-select in Gengrid is enabled or disabled
 *
 * @param obj The Gengrid object
 * @return Multi-select enable/disable
 * (EINA_TRUE = enabled / EINA_FALSE = disabled)
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_multi_select_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->multi;
}

/**
 * Get the selected item in the Gengrid
 *
 * This gets the selected item in the Gengrid (if multi-select is enabled only
 * the first item in the list is selected - which is not very useful, so see
 * elm_gengrid_selected_items_get() for when multi-select is used).
 *
 * If no item is selected, NULL is returned.
 *
 * @param obj The Gengrid object.
 * @return The selected item, or NULL if none.
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_selected_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (wd->selected) return wd->selected->data;
   return NULL;
}

/**
 * Get a list of selected items in the Gengrid.
 *
 * This returns a list of the selected items. This list pointer is only valid
 * so long as no items are selected or unselected (or unselected implictly by
 * deletion). The list contains Elm_Gengrid_Item pointers.
 *
 * @param obj The Gengrid object.
 * @return The list of selected items, or NULL if none are selected.
 *
 * @ingroup Gengrid
 */
EAPI const Eina_List *
elm_gengrid_selected_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->selected;
}

/**
 * Set the selected state of a item.
 *
 * This sets the selected state of a item. If multi-select is not enabled and
 * selected is EINA_TRUE, previously selected items are unselected.
 *
 * @param item The item
 * @param selected The selected state.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_selected_set(Elm_Gengrid_Item *item, Eina_Bool selected)
{
   Widget_Data *wd = elm_widget_data_get(item->wd->self);
   if (!wd) return;
   if ((!item) || (item->delete_me)) return;
   selected = !!selected;
   if (item->selected == selected) return;

   if (selected)
     {
	if (!wd->multi)
	  {
	     while (wd->selected)
	       _item_unselect(wd->selected->data);
	  }
	_item_hilight(item);
	_item_select(item);
     }
   else
     _item_unselect(item);
}

/**
 * Get the selected state of a item.
 *
 * This gets the selected state of a item (1 selected, 0 not selected).
 *
 * @param item The item
 * @return The selected state
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_item_selected_get(const Elm_Gengrid_Item *item)
{
   if (!item) return EINA_FALSE;
   return item->selected;
}

/**
 * Sets the disabled state of a item.
 *
 * A disabled item cannot be selected or unselected. It will also change
 * appearance to disabled. This sets the disabled state (1 disabled, 0 not
 * disabled).
 *
 * @param item The item
 * @param disabled The disabled state
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_disabled_set(Elm_Gengrid_Item *item, Eina_Bool disabled)
{
   if (!item) return;
   if (item->disabled == disabled) return;
   if (item->delete_me) return;
   item->disabled = disabled;
   if (item->realized)
     {
	if (item->disabled)
	  edje_object_signal_emit(item->base, "elm,state,disabled", "elm");
	else
	  edje_object_signal_emit(item->base, "elm,state,enabled", "elm");
     }
}

/**
 * Get the disabled state of a item.
 *
 * This gets the disabled state of the given item.
 *
 * @param item The item
 * @return The disabled state
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_item_disabled_get(const Elm_Gengrid_Item *item)
{
   if (!item) return EINA_FALSE;
   if (item->delete_me) return EINA_FALSE;
   return item->disabled;
}

/**
 * Set the always select mode.
 *
 * Cells will only call their selection func and callback when first becoming
 * selected. Any further clicks will do nothing, unless you enable always select
 * with elm_gengrid_always_select_mode_set(). This means even if selected,
 * every click will make the selected callbacks be called.
 *
 * @param obj The Gengrid object
 * @param always_select The always select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->always_select = always_select;
}

/**
 * Get the always select mode.
 *
 * @param obj The Gengrid object.
 * @return The always select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_always_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->always_select;
}

/**
 * Set no select mode.
 *
 * This will turn off the ability to select items entirely and they will
 * neither appear selected nor call selected callback functions.
 *
 * @param obj The Gengrid object
 * @param no_select The no select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->no_select = no_select;
}

/**
 * Gets no select mode.
 *
 * @param obj The Gengrid object
 * @return The no select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_no_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->no_select;
}

/**
 * Set bounce mode.
 *
 * This will enable or disable the scroller bounce mode for the Gengrid. See
 * elm_scroller_bounce_set() for details.
 *
 * @param obj The Gengrid object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_set(wd->scr, h_bounce, v_bounce);
}

/**
 * Get the bounce mode
 *
 * @param obj The Gengrid object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_get(wd->scr, h_bounce, v_bounce);
}

/**
 * Get all items in the Gengrid.
 *
 * This returns a list of the Gengrid items. The list contains
 * Elm_Gengrid_Item pointers.
 *
 * @param obj The Gengrid object.
 * @return The list of items, or NULL if none.
 *
 * @ingroup Gengrid
 */
EAPI const Eina_List *
elm_gengrid_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->items;
}
