#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

/**
 * @defgroup Gengrid Gengrid
 *
 * This widget aims to position objects in a grid layout while
 * actually building only the visible ones, using the same idea as
 * genlist: the user define a class for each item, specifying
 * functions that will be called at object creation and deletion.
 *
 * A item in the Gengrid can have 0 or more text labels (they can be
 * regular text or textblock - that's up to the style to determine), 0
 * or more icons (which are simply objects swallowed into the Gengrid
 * item) and 0 or more boolean states that can be used for check,
 * radio or other indicators by the edje theme style.  A item may be
 * one of several styles (Elementary provides 1 by default -
 * "default", but this can be extended by system or application custom
 * themes/overlays/extensions).
 *
 * In order to implement the ability to add and delete items on the
 * fly, Gengrid implements a class/callback system where the
 * application provides a structure with information about that type
 * of item (Gengrid may contain multiple different items with
 * different classes, states and styles). Gengrid will call the
 * functions in this struct (methods) when an item is "realized" (that
 * is created dynamically while scrolling). All objects will simply be
 * deleted when no longer needed with evas_object_del(). The
 * Elm_GenGrid_Item_Class structure contains the following members:
 *
 * item_style - This is a constant string and simply defines the name
 * of the item style. It must be specified and the default should be
 * "default".
 *
 * func.label_get - This function is called when an actual item object
 * is created. The data parameter is the one passed to
 * elm_gengrid_item_append() and related item creation functions. The
 * obj parameter is the Gengrid object and the part parameter is the
 * string name of the text part in the edje design that is listed as
 * one of the possible labels that can be set. This function must
 * return a strdup'()ed string as the caller will free() it when done.
 *
 * func.icon_get - This function is called when an actual item object
 * is created. The data parameter is the one passed to
 * elm_gengrid_item_append() and related item creation functions. The
 * obj parameter is the Gengrid object and the part parameter is the
 * string name of the icon part in the edje design that is listed as
 * one of the possible icons that can be set. This must return NULL
 * for no object or a valid object. The object will be deleted by
 * Gengrid on shutdown or when the item is unrealized.
 *
 * func.state_get - This function is called when an actual item object
 * is created. The data parameter is the one passed to
 * elm_gengrid_item_append() and related item creation functions. The
 * obj parameter is the Gengrid object and the part parameter is the
 * string name of th state part in the edje design that is listed as
 * one of the possible states that can be set. Return 0 for false and
 * 1 for true. Gengrid will emit a signal to the edje object with
 * "elm,state,XXX,active" "elm" when true (the default is false),
 * where XXX is the name of the part.
 *
 * func.del - This is called when elm_gengrid_item_del() is called on
 * an item or elm_gengrid_clear() is called on the Gengrid. This is
 * intended for use when actual Gengrid items are deleted, so any
 * backing data attached to the item (e.g. its data parameter on
 * creation) can be deleted.
 *
 * If the application wants multiple items to be able to be selected,
 * elm_gengrid_multi_select_set() can enable this. If the Gengrid is
 * single-selection only (the default), then
 * elm_gengrid_select_item_get() will return the selected item, if
 * any, or NULL if none is selected. If the Gengrid is multi-select
 * then elm_gengrid_selected_items_get() will return a list (that is
 * only valid as long as no items are modified (added, deleted,
 * selected or unselected).
 *
 * If an item changes (state of boolean changes, label or icons
 * change), then use elm_gengrid_item_update() to have Gengrid update
 * the item with the new state. Gengrid will re-realize the item thus
 * call the functions in the _Elm_Gengrid_Item_Class for that item.
 *
 * To programmatically (un)select an item use
 * elm_gengrid_item_selected_set().  To get its selected state use
 * elm_gengrid_item_selected_get(). To make an item disabled (unable to
 * be selected and appear differently) use
 * elm_gengrid_item_disabled_set() to set this and
 * elm_gengrid_item_disabled_get() to get the disabled state.
 *
 * Cells will only call their selection func and callback when first
 * becoming selected. Any further clicks will do nothing, unless you
 * enable always select with
 * elm_gengrid_always_select_mode_set(). This means event if selected,
 * every click will make the selected callbacks be called.
 * elm_gengrid_no_select_mode_set() will turn off the ability to
 * select items entirely and they will neither appear selected nor
 * call selected callback function.
 *
 * Remember that you can create new styles and add your own theme
 * augmentation per application with elm_theme_extension_add(). If you
 * absolutely must have a specific style that overrides any theme the
 * user or system sets up you can use elm_theme_overlay_add() to add
 * such a file.
 *
 * Signals that you can add callbacks for are:
 *
 * "clicked,double" - The user has double-clicked or pressed enter on
 *                    an item. The event_infoparameter is the Gengrid item
 *                    that was double-clicked.
 * "selected" - The user has made an item selected. The event_info
 *              parameter is the Gengrid item that was selected.
 * "unselected" - The user has made an item unselected. The event_info
 *                parameter is the Gengrid item that was unselected.
 * "realized" - This is called when the item in the Gengrid is created
 *              as a real evas object. event_info is the Gengrid item that was
 *              created. The object may be deleted at any time, so it is up to
 *              the caller to not use the object pointer from
 *              elm_gengrid_item_object_get() in a way where it may point to
 *              freed objects.
 * "unrealized" - This is called when the real evas object for this item
 *                is deleted. event_info is the Gengrid item that was created.
 * "changed" - Called when an item is added, removed, resized or moved
 *             and when gengrid is resized or horizontal property changes.
 * "drag,start,up" - Called when the item in the Gengrid has been
 *                   dragged (not scrolled) up.
 * "drag,start,down" - Called when the item in the Gengrid has been
 *                     dragged (not scrolled) down.
 * "drag,start,left" - Called when the item in the Gengrid has been
 *                     dragged (not scrolled) left.
 * "drag,start,right" - Called when the item in the Gengrid has been
 *                      dragged (not scrolled) right.
 * "drag,stop" - Called when the item in the Gengrid has stopped being
 *               dragged.
 * "drag" - Called when the item in the Gengrid is being dragged.
 * "scroll" - called when the content has been scrolled (moved).
 * "scroll,drag,start" - called when dragging the content has started.
 * "scroll,drag,stop" - called when dragging the content has stopped.
 *
 * --
 * TODO:
 * Handle non-homogeneous objects too.
 */

 typedef struct _Widget_Data Widget_Data;
 typedef struct _Pan         Pan;

#define PRELOAD 1
#define REORDER_EFFECT_TIME 0.5

 struct _Elm_Gengrid_Item
{
   Elm_Widget_Item               base;
   EINA_INLIST;
   Evas_Object                  *spacer;
   const Elm_Gengrid_Item_Class *gic;
   Ecore_Timer                  *long_timer;
   Ecore_Animator               *item_moving_effect_timer;
   Widget_Data                  *wd;
   Eina_List                    *labels, *icons, *states, *icon_objs;
   struct
     {
        Evas_Smart_Cb func;
        const void   *data;
     } func;

   Evas_Coord   x, y, dx, dy, ox, oy, tx, ty, rx, ry;
   unsigned int moving_effect_start_time;
   int          relcount;
   int          walking;

   struct
     {
        const void                 *data;
        Elm_Tooltip_Item_Content_Cb content_cb;
        Evas_Smart_Cb               del_cb;
        const char                 *style;
     } tooltip;

   const char *mouse_cursor;

   Eina_Bool   want_unrealize : 1;
   Eina_Bool   realized : 1;
   Eina_Bool   dragging : 1;
   Eina_Bool   down : 1;
   Eina_Bool   delete_me : 1;
   Eina_Bool   display_only : 1;
   Eina_Bool   disabled : 1;
   Eina_Bool   selected : 1;
   Eina_Bool   hilighted : 1;
   Eina_Bool   moving : 1;
};

struct _Widget_Data
{
   Evas_Object      *self, *scr;
   Evas_Object      *pan_smart;
   Pan              *pan;
   Eina_Inlist      *items;
   Ecore_Job        *calc_job;
   Eina_List        *selected;
   Elm_Gengrid_Item *last_selected_item, *reorder_item;
   double            align_x, align_y;

   Evas_Coord        pan_x, pan_y, old_pan_x, old_pan_y;
   Evas_Coord        item_width, item_height; /* Each item size */
   Evas_Coord        minw, minh; /* Total obj size */
   Evas_Coord        reorder_item_x, reorder_item_y;
   unsigned int      nmax;
   long              count;
   int               walking;

   Eina_Bool         horizontal : 1;
   Eina_Bool         on_hold : 1;
   Eina_Bool         longpressed : 1;
   Eina_Bool         multi : 1;
   Eina_Bool         no_select : 1;
   Eina_Bool         wasselected : 1;
   Eina_Bool         always_select : 1;
   Eina_Bool         clear_me : 1;
   Eina_Bool         h_bounce : 1;
   Eina_Bool         v_bounce : 1;
   Eina_Bool         reorder_mode : 1;
   Eina_Bool         reorder_item_changed : 1;
   Eina_Bool         move_effect_enabled : 1;
};

#define ELM_GENGRID_ITEM_FROM_INLIST(item) \
   ((item) ? EINA_INLIST_CONTAINER_GET(item, Elm_Gengrid_Item) : NULL)

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data                   *wd;
};

static const char *widtype = NULL;
static void      _item_hilight(Elm_Gengrid_Item *item);
static void      _item_unrealize(Elm_Gengrid_Item *item);
static void      _item_select(Elm_Gengrid_Item *item);
static void      _item_unselect(Elm_Gengrid_Item *item);
static void      _calc_job(void *data);
static void      _on_focus_hook(void        *data,
                                Evas_Object *obj);
static Eina_Bool _item_multi_select_up(Widget_Data *wd);
static Eina_Bool _item_multi_select_down(Widget_Data *wd);
static Eina_Bool _item_multi_select_left(Widget_Data *wd);
static Eina_Bool _item_multi_select_right(Widget_Data *wd);
static Eina_Bool _item_single_select_up(Widget_Data *wd);
static Eina_Bool _item_single_select_down(Widget_Data *wd);
static Eina_Bool _item_single_select_left(Widget_Data *wd);
static Eina_Bool _item_single_select_right(Widget_Data *wd);
static Eina_Bool _event_hook(Evas_Object       *obj,
                             Evas_Object       *src,
                             Evas_Callback_Type type,
                             void              *event_info);
static Eina_Bool _deselect_all_items(Widget_Data *wd);

static Evas_Smart_Class _pan_sc = EVAS_SMART_CLASS_INIT_VERSION;
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);

static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_SELECTED[] = "selected";
static const char SIG_UNSELECTED[] = "unselected";
static const char SIG_REALIZED[] = "realized";
static const char SIG_UNREALIZED[] = "unrealized";
static const char SIG_CHANGED[] = "changed";
static const char SIG_DRAG_START_UP[] = "drag,start,up";
static const char SIG_DRAG_START_DOWN[] = "drag,start,down";
static const char SIG_DRAG_START_LEFT[] = "drag,start,left";
static const char SIG_DRAG_START_RIGHT[] = "drag,start,right";
static const char SIG_DRAG_STOP[] = "drag,stop";
static const char SIG_DRAG[] = "drag";
static const char SIG_SCROLL[] = "scroll";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_MOVED[] = "moved";

static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_CLICKED_DOUBLE, ""},
       {SIG_SELECTED, ""},
       {SIG_UNSELECTED, ""},
       {SIG_REALIZED, ""},
       {SIG_UNREALIZED, ""},
       {SIG_CHANGED, ""},
       {SIG_DRAG_START_UP, ""},
       {SIG_DRAG_START_DOWN, ""},
       {SIG_DRAG_START_LEFT, ""},
       {SIG_DRAG_START_RIGHT, ""},
       {SIG_DRAG_STOP, ""},
       {SIG_DRAG, ""},
       {SIG_SCROLL, ""},
       {SIG_SCROLL_DRAG_START, ""},
       {SIG_SCROLL_DRAG_STOP, ""},
       {SIG_MOVED, ""},
       {NULL, NULL}
};

static Eina_Compare_Cb _elm_gengrid_item_compare_cb;
static Eina_Compare_Cb _elm_gengrid_item_compare_data_cb;

static Eina_Bool
_event_hook(Evas_Object       *obj,
            Evas_Object *src   __UNUSED__,
            Evas_Callback_Type type,
            void              *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   if (!wd->items) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   Elm_Gengrid_Item *item = NULL;
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   elm_smart_scroller_child_pos_get(wd->scr, &x, &y);
   elm_smart_scroller_step_size_get(wd->scr, &step_x, &step_y);
   elm_smart_scroller_page_size_get(wd->scr, &page_x, &page_y);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &v_w, &v_h);

   if ((!strcmp(ev->keyname, "Left")) || (!strcmp(ev->keyname, "KP_Left")))
     {
        if ((wd->horizontal) &&
            (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
              (_item_multi_select_up(wd)))
             || (_item_single_select_up(wd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if ((!wd->horizontal) &&
                 (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
                   (_item_multi_select_left(wd)))
                  || (_item_single_select_left(wd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) || (!strcmp(ev->keyname, "KP_Right")))
     {
        if ((wd->horizontal) &&
            (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
              (_item_multi_select_down(wd)))
             || (_item_single_select_down(wd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if ((!wd->horizontal) &&
                 (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
                   (_item_multi_select_right(wd)))
                  || (_item_single_select_right(wd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) || (!strcmp(ev->keyname, "KP_Up")))
     {
        if ((wd->horizontal) &&
            (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
              (_item_multi_select_left(wd)))
             || (_item_single_select_left(wd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if ((!wd->horizontal) &&
                 (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
                   (_item_multi_select_up(wd)))
                  || (_item_single_select_up(wd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) || (!strcmp(ev->keyname, "KP_Down")))
     {
        if ((wd->horizontal) &&
            (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
              (_item_multi_select_right(wd)))
             || (_item_single_select_right(wd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if ((!wd->horizontal) &&
                 (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
                   (_item_multi_select_down(wd)))
                  || (_item_single_select_down(wd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Home")) || (!strcmp(ev->keyname, "KP_Home")))
     {
        item = elm_gengrid_first_item_get(obj);
        elm_gengrid_item_bring_in(item);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "End")) || (!strcmp(ev->keyname, "KP_End")))
     {
        item = elm_gengrid_last_item_get(obj);
        elm_gengrid_item_bring_in(item);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Prior")) ||
            (!strcmp(ev->keyname, "KP_Prior")))
     {
        if (wd->horizontal)
          {
             if (page_x < 0)
               x -= -(page_x * v_w) / 100;
             else
               x -= page_x;
          }
        else
          {
             if (page_y < 0)
               y -= -(page_y * v_h) / 100;
             else
               y -= page_y;
          }
     }
   else if ((!strcmp(ev->keyname, "Next")) || (!strcmp(ev->keyname, "KP_Next")))
     {
        if (wd->horizontal)
          {
             if (page_x < 0)
               x += -(page_x * v_w) / 100;
             else
               x += page_x;
          }
        else
          {
             if (page_y < 0)
               y += -(page_y * v_h) / 100;
             else
               y += page_y;
          }
     }
   else if (!strcmp(ev->keyname, "Escape"))
     {
        if (!_deselect_all_items(wd)) return EINA_FALSE;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Return")) ||
            (!strcmp(ev->keyname, "KP_Enter")) ||
            (!strcmp(ev->keyname, "space")))
     {
        item = elm_gengrid_selected_item_get(obj);
        evas_object_smart_callback_call(item->wd->self, SIG_CLICKED_DOUBLE, item);
        evas_object_smart_callback_call(item->wd->self, "clicked", item); // will be removed
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   elm_smart_scroller_child_pos_set(wd->scr, x, y);
   return EINA_TRUE;
}

static Eina_Bool
_deselect_all_items(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;
   while(wd->selected)
     elm_gengrid_item_selected_set(wd->selected->data, EINA_FALSE);

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_left(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;

   Elm_Gengrid_Item *prev = elm_gengrid_item_prev_get(wd->last_selected_item);
   if (!prev) return EINA_TRUE;
   if (elm_gengrid_item_selected_get(prev))
     {
        elm_gengrid_item_selected_set(wd->last_selected_item, EINA_FALSE);
        wd->last_selected_item = prev;
        elm_gengrid_item_show(wd->last_selected_item);
     }
   else
     {
        elm_gengrid_item_selected_set(prev, EINA_TRUE);
        elm_gengrid_item_show(prev);
     }

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_right(Widget_Data *wd)
{
   if (!wd->selected) return EINA_FALSE;

   Elm_Gengrid_Item *next = elm_gengrid_item_next_get(wd->last_selected_item);
   if (!next) return EINA_TRUE;
   if (elm_gengrid_item_selected_get(next))
     {
        elm_gengrid_item_selected_set(wd->last_selected_item, EINA_FALSE);
        wd->last_selected_item = next;
        elm_gengrid_item_show(wd->last_selected_item);
     }
   else
     {
        elm_gengrid_item_selected_set(next, EINA_TRUE);
        elm_gengrid_item_show(next);
     }

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_up(Widget_Data *wd)
{
   unsigned int i;
   Eina_Bool r = EINA_TRUE;

   if (!wd->selected) return EINA_FALSE;

   for (i = 0; (r) && (i < wd->nmax); i++)
     r &= _item_multi_select_left(wd);

   return r;
}

static Eina_Bool
_item_multi_select_down(Widget_Data *wd)
{
   unsigned int i;
   Eina_Bool r = EINA_TRUE;

   if (!wd->selected) return EINA_FALSE;

   for (i = 0; (r) && (i < wd->nmax); i++)
     r &= _item_multi_select_right(wd);

   return r;
}

static Eina_Bool
_item_single_select_up(Widget_Data *wd)
{
   unsigned int i;

   Elm_Gengrid_Item *prev;

   if (!wd->selected)
     {
        prev = ELM_GENGRID_ITEM_FROM_INLIST(wd->items->last);
        while ((prev) && (prev->delete_me))
          prev = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
        elm_gengrid_item_selected_set(prev, EINA_TRUE);
        elm_gengrid_item_show(prev);
        return EINA_TRUE;
     }
   else prev = elm_gengrid_item_prev_get(wd->last_selected_item);

   if (!prev) return EINA_FALSE;

   for (i = 1; i < wd->nmax; i++)
     {
        Elm_Gengrid_Item *tmp = elm_gengrid_item_prev_get(prev);
        if (!tmp) return EINA_FALSE;
        prev = tmp;
     }

   _deselect_all_items(wd);

   elm_gengrid_item_selected_set(prev, EINA_TRUE);
   elm_gengrid_item_show(prev);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_down(Widget_Data *wd)
{
   unsigned int i;

   Elm_Gengrid_Item *next;

   if (!wd->selected)
     {
        next = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);
        while ((next) && (next->delete_me))
          next = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
        elm_gengrid_item_selected_set(next, EINA_TRUE);
        elm_gengrid_item_show(next);
        return EINA_TRUE;
     }
   else next = elm_gengrid_item_next_get(wd->last_selected_item);

   if (!next) return EINA_FALSE;

   for (i = 1; i < wd->nmax; i++)
     {
        Elm_Gengrid_Item *tmp = elm_gengrid_item_next_get(next);
        if (!tmp) return EINA_FALSE;
        next = tmp;
     }

   _deselect_all_items(wd);

   elm_gengrid_item_selected_set(next, EINA_TRUE);
   elm_gengrid_item_show(next);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_left(Widget_Data *wd)
{
   Elm_Gengrid_Item *prev;
   if (!wd->selected)
     {
        prev = ELM_GENGRID_ITEM_FROM_INLIST(wd->items->last);
        while ((prev) && (prev->delete_me))
          prev = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
     }
   else prev = elm_gengrid_item_prev_get(wd->last_selected_item);

   if (!prev) return EINA_FALSE;

   _deselect_all_items(wd);

   elm_gengrid_item_selected_set(prev, EINA_TRUE);
   elm_gengrid_item_show(prev);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_right(Widget_Data *wd)
{
   Elm_Gengrid_Item *next;
   if (!wd->selected)
     {
        next = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);
        while ((next) && (next->delete_me))
          next = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
     }
   else next = elm_gengrid_item_next_get(wd->last_selected_item);

   if (!next) return EINA_FALSE;

   _deselect_all_items(wd);

   elm_gengrid_item_selected_set(next, EINA_TRUE);
   elm_gengrid_item_show(next);
   return EINA_TRUE;
}

static void
_on_focus_hook(void *data   __UNUSED__,
               Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->self, "elm,action,focus", "elm");
        evas_object_focus_set(wd->self, EINA_TRUE);
        if ((wd->selected) && (!wd->last_selected_item))
          wd->last_selected_item = eina_list_data_get(wd->selected);
     }
   else
     {
        edje_object_signal_emit(wd->self, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->self, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Gengrid_Item *item;
   if (!wd) return;
   elm_smart_scroller_mirrored_set(wd->scr, rtl);
   if (!wd->items) return;
   item = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);

   while (item)
     {
        edje_object_mirrored_set(item->base.view, rtl);
        elm_gengrid_item_update(item);
        item = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(item)->next);
     }
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   elm_smart_scroller_object_theme_set(obj, wd->scr, "gengrid", "base",
                                       elm_widget_style_get(obj));
}

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_gengrid_clear(obj);
   evas_object_del(wd->pan_smart);
   wd->pan_smart = NULL;
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_signal_emit_hook(Evas_Object *obj,
                  const char  *emission,
                  const char  *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(elm_smart_scroller_edje_object_get(wd->scr),
                           emission, source);
}

static void
_mouse_move(void        *data,
            Evas *evas   __UNUSED__,
            Evas_Object *obj,
            void        *event_info)
{
   Elm_Gengrid_Item *item = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;
   Evas_Coord ox, oy, ow, oh, it_scrl_x, it_scrl_y;

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
        evas_object_smart_callback_call(item->wd->self, SIG_DRAG, item);
        return;
     }
   if ((!item->down) || (item->wd->longpressed))
     {
        if (item->long_timer)
          {
             ecore_timer_del(item->long_timer);
             item->long_timer = NULL;
          }
        if ((item->wd->reorder_mode) && (item->wd->reorder_item))
          {
             evas_object_geometry_get(item->wd->pan_smart, &ox, &oy, &ow, &oh);

             it_scrl_x = ev->cur.canvas.x - item->wd->reorder_item->dx;
             it_scrl_y = ev->cur.canvas.y - item->wd->reorder_item->dy;

             if (it_scrl_x < ox) item->wd->reorder_item_x = ox;
             else if (it_scrl_x + item->wd->item_width > ox + ow)
               item->wd->reorder_item_x = ox + ow - item->wd->item_width;
             else item->wd->reorder_item_x = it_scrl_x;

             if (it_scrl_y < oy) item->wd->reorder_item_y = oy;
             else if (it_scrl_y + item->wd->item_height > oy + oh)
               item->wd->reorder_item_y = oy + oh - item->wd->item_height;
             else item->wd->reorder_item_y = it_scrl_y;

             if (item->wd->calc_job) ecore_job_del(item->wd->calc_job);
             item->wd->calc_job = ecore_job_add(_calc_job, item->wd);
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
        const char *left_drag, *right_drag;
        if (!elm_widget_mirrored_get(item->wd->self))
          {
             left_drag = SIG_DRAG_START_LEFT;
             right_drag = SIG_DRAG_START_RIGHT;
          }
        else
          {
             left_drag = SIG_DRAG_START_RIGHT;
             right_drag = SIG_DRAG_START_LEFT;
          }

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
               evas_object_smart_callback_call(item->wd->self, SIG_DRAG_START_UP,
                                               item);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(item->wd->self,
                                                    left_drag, item);
               }
          }
        else
          {
             if (ady > adx)
               evas_object_smart_callback_call(item->wd->self,
                                               SIG_DRAG_START_DOWN, item);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(item->wd->self,
                                                    left_drag, item);
                  else
                    evas_object_smart_callback_call(item->wd->self,
                                                    right_drag, item);
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
   if (item->wd->reorder_mode)
     {
        item->wd->reorder_item = item;
        evas_object_raise(item->base.view);
        elm_smart_scroller_hold_set(item->wd->scr, EINA_TRUE);
        elm_smart_scroller_bounce_allow_set(item->wd->scr, EINA_FALSE, EINA_FALSE);
        edje_object_signal_emit(item->base.view, "elm,state,reorder,enabled", "elm");
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down(void        *data,
            Evas *evas   __UNUSED__,
            Evas_Object *obj,
            void        *event_info)
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
     {
        evas_object_smart_callback_call(item->wd->self, SIG_CLICKED_DOUBLE, item);
        evas_object_smart_callback_call(item->wd->self, "clicked", item); // will be removed
     }
   if (item->long_timer) ecore_timer_del(item->long_timer);
   if (item->realized)
     item->long_timer = ecore_timer_add(_elm_config->longpress_timeout,
                                        _long_press, item);
   else
     item->long_timer = NULL;
}

static void
_mouse_up(void            *data,
          Evas *evas       __UNUSED__,
          Evas_Object *obj __UNUSED__,
          void            *event_info)
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
        evas_object_smart_callback_call(item->wd->self, SIG_DRAG_STOP, item);
        dragged = EINA_TRUE;
     }
   if (item->wd->on_hold)
     {
        item->wd->longpressed = EINA_FALSE;
        item->wd->on_hold = EINA_FALSE;
        return;
     }
   if ((item->wd->reorder_mode) && (item->wd->reorder_item))
     {
        if (item->wd->calc_job) ecore_job_del(item->wd->calc_job);
          item->wd->calc_job = ecore_job_add(_calc_job, item->wd);

        evas_object_smart_callback_call(item->wd->self, SIG_MOVED, item->wd->reorder_item);
        item->wd->reorder_item = NULL;
        elm_smart_scroller_hold_set(item->wd->scr, EINA_FALSE);
        elm_smart_scroller_bounce_allow_set(item->wd->scr, item->wd->h_bounce, item->wd->v_bounce);
        edje_object_signal_emit(item->base.view, "elm,state,reorder,disabled", "elm");
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
   if ((item->disabled) || (dragged)) return;
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
   edje_object_signal_emit(item->base.view, "elm,state,selected", "elm");
   item->hilighted = EINA_TRUE;
}

static void
_item_realize(Elm_Gengrid_Item *item)
{
   char buf[1024];
   char style[1024];

   if ((item->realized) || (item->delete_me)) return;
   item->base.view = edje_object_add(evas_object_evas_get(item->wd->self));
   edje_object_scale_set(item->base.view, elm_widget_scale_get(item->wd->self) *
                         _elm_config->scale);
   edje_object_mirrored_set(item->base.view, elm_widget_mirrored_get(item->base.widget));
   evas_object_smart_member_add(item->base.view, item->wd->pan_smart);
   elm_widget_sub_object_add(item->wd->self, item->base.view);
   snprintf(style, sizeof(style), "item/%s",
            item->gic->item_style ? item->gic->item_style : "default");
   _elm_theme_object_set(item->wd->self, item->base.view, "gengrid", style,
                         elm_widget_style_get(item->wd->self));
   item->spacer =
      evas_object_rectangle_add(evas_object_evas_get(item->wd->self));
   evas_object_color_set(item->spacer, 0, 0, 0, 0);
   elm_widget_sub_object_add(item->wd->self, item->spacer);
   evas_object_size_hint_min_set(item->spacer, 2 * _elm_config->scale, 1);
   edje_object_part_swallow(item->base.view, "elm.swallow.pad", item->spacer);

   if (item->gic->func.label_get)
     {
        const Eina_List *l;
        const char *key;

        item->labels =
           elm_widget_stringlist_get(edje_object_data_get(item->base.view,
                                                          "labels"));
        EINA_LIST_FOREACH(item->labels, l, key)
          {
             char *s = item->gic->func.label_get
                ((void *)item->base.data, item->wd->self, l->data);
             if (s)
               {
                  edje_object_part_text_set(item->base.view, l->data, s);
                  free(s);
               }
          }
     }

   if (item->gic->func.icon_get)
     {
        const Eina_List *l;
        const char *key;

        item->icons =
           elm_widget_stringlist_get(edje_object_data_get(item->base.view,
                                                          "icons"));
        EINA_LIST_FOREACH(item->icons, l, key)
          {
             Evas_Object *ic = item->gic->func.icon_get
                ((void *)item->base.data, item->wd->self, l->data);
             if (ic)
               {
                  item->icon_objs = eina_list_append(item->icon_objs, ic);
                  edje_object_part_swallow(item->base.view, key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(item->wd->self, ic);
               }
          }
     }

   if (item->gic->func.state_get)
     {
        const Eina_List *l;
        const char *key;

        item->states =
           elm_widget_stringlist_get(edje_object_data_get(item->base.view,
                                                          "states"));
        EINA_LIST_FOREACH(item->states, l, key)
          {
             Eina_Bool on = item->gic->func.state_get
                ((void *)item->base.data, item->wd->self, l->data);
             if (on)
               {
                  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
                  edje_object_signal_emit(item->base.view, buf, "elm");
               }
          }
     }

   if ((!item->wd->item_width) && (!item->wd->item_height))
     {
        edje_object_size_min_restricted_calc(item->base.view,
                                             &item->wd->item_width,
                                             &item->wd->item_height,
                                             item->wd->item_width,
                                             item->wd->item_height);
        elm_coords_finger_size_adjust(1, &item->wd->item_width,
                                      1, &item->wd->item_height);
     }

   evas_object_event_callback_add(item->base.view, EVAS_CALLBACK_MOUSE_DOWN,
                                  _mouse_down, item);
   evas_object_event_callback_add(item->base.view, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, item);
   evas_object_event_callback_add(item->base.view, EVAS_CALLBACK_MOUSE_MOVE,
                                  _mouse_move, item);

   if (item->selected)
     edje_object_signal_emit(item->base.view, "elm,state,selected", "elm");
   if (item->disabled)
     edje_object_signal_emit(item->base.view, "elm,state,disabled", "elm");

   evas_object_show(item->base.view);

   if (item->tooltip.content_cb)
     {
        elm_widget_item_tooltip_content_cb_set(item,
                                               item->tooltip.content_cb,
                                               item->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(item, item->tooltip.style);
     }

   if (item->mouse_cursor)
     elm_widget_item_cursor_set(item, item->mouse_cursor);

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
   evas_object_del(item->base.view);
   item->base.view = NULL;
   evas_object_del(item->spacer);
   item->spacer = NULL;
   elm_widget_stringlist_free(item->labels);
   item->labels = NULL;
   elm_widget_stringlist_free(item->icons);
   item->icons = NULL;
   elm_widget_stringlist_free(item->states);
   item->states = NULL;

   EINA_LIST_FREE(item->icon_objs, icon)
      evas_object_del(icon);

   item->realized = EINA_FALSE;
   item->want_unrealize = EINA_FALSE;
}

static Eina_Bool
_reorder_item_moving_effect_timer_cb(void *data)
{
   Elm_Gengrid_Item *item = data;
   double time, t;
   Evas_Coord dx, dy;

   time = REORDER_EFFECT_TIME;
   t = ((0.0 > (t = ecore_loop_time_get()-item->moving_effect_start_time)) ? 0.0 : t);
   dx = ((item->tx - item->ox) / 10) * _elm_config->scale;
   dy = ((item->ty - item->oy) / 10) * _elm_config->scale;

   if (t <= time)
     {
        item->rx += (1 * sin((t / time) * (M_PI / 2)) * dx);
        item->ry += (1 * sin((t / time) * (M_PI / 2)) * dy);
     }
   else
     {
        item->rx += dx;
        item->ry += dy;
     }

   if ((((dx > 0) && (item->rx >= item->tx)) || ((dx <= 0) && (item->rx <= item->tx))) &&
       (((dy > 0) && (item->ry >= item->ty)) || ((dy <= 0) && (item->ry <= item->ty))))
     {
        evas_object_move(item->base.view, item->tx, item->ty);
        evas_object_resize(item->base.view, item->wd->item_width, item->wd->item_height);
        item->moving = EINA_FALSE;
        item->item_moving_effect_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   evas_object_move(item->base.view, item->rx, item->ry);
   evas_object_resize(item->base.view, item->wd->item_width, item->wd->item_height);

   return ECORE_CALLBACK_RENEW;
}

static void
_item_place(Elm_Gengrid_Item *item,
            Evas_Coord        cx,
            Evas_Coord        cy)
{
   Evas_Coord x, y, ox, oy, cvx, cvy, cvw, cvh;
   Evas_Coord tch, tcw, alignw = 0, alignh = 0, vw, vh;
   Eina_Bool reorder_item_move_forward = EINA_FALSE;
   item->x = cx;
   item->y = cy;
   evas_object_geometry_get(item->wd->pan_smart, &ox, &oy, &vw, &vh);

   /* Preload rows/columns at each side of the Gengrid */
   cvx = ox - PRELOAD * item->wd->item_width;
   cvy = oy - PRELOAD * item->wd->item_height;
   cvw = vw + 2 * PRELOAD * item->wd->item_width;
   cvh = vh + 2 * PRELOAD * item->wd->item_height;

   alignh = 0;
   alignw = 0;

   if (item->wd->horizontal)
     {
        int columns, items_visible = 0, items_row;

        if (item->wd->item_height > 0)
          items_visible = vh / item->wd->item_height;
        if (items_visible < 1)
          items_visible = 1;

        columns = item->wd->count / items_visible;
        if (item->wd->count % items_visible)
          columns++;

        tcw = item->wd->item_width * columns;
        alignw = (vw - tcw) * item->wd->align_x;

        items_row = items_visible;
        if (items_row > item->wd->count)
          items_row = item->wd->count;
        tch = items_row * item->wd->item_height;
        alignh = (vh - tch) * item->wd->align_y;
     }
   else
     {
        int rows, items_visible = 0, items_col;

        if (item->wd->item_width > 0)
          items_visible = vw / item->wd->item_width;
        if (items_visible < 1)
          items_visible = 1;

        rows = item->wd->count / items_visible;
        if (item->wd->count % items_visible)
          rows++;

        tch = item->wd->item_height * rows;
        alignh = (vh - tch) * item->wd->align_y;

        items_col = items_visible;
        if (items_col > item->wd->count)
          items_col = item->wd->count;
        tcw = items_col * item->wd->item_width;
        alignw = (vw - tcw) * item->wd->align_x;
     }

   x = cx * item->wd->item_width - item->wd->pan_x + ox + alignw;
   if (elm_widget_mirrored_get(item->wd->self))
     {  /* Switch items side and componsate for pan_x when in RTL mode */
        Evas_Coord ww;
        evas_object_geometry_get(item->wd->self, NULL, NULL, &ww, NULL);
        x = ww - x - item->wd->item_width - item->wd->pan_x - item->wd->pan_x;
     }

   y = cy * item->wd->item_height - item->wd->pan_y + oy + alignh;

   Eina_Bool was_realized = item->realized;
   if (ELM_RECTS_INTERSECT(x, y, item->wd->item_width, item->wd->item_height,
                           cvx, cvy, cvw, cvh))
     {
        _item_realize(item);
        if (!was_realized)
          evas_object_smart_callback_call(item->wd->self, SIG_REALIZED, item);
        if ((item->wd->reorder_mode) && (item->wd->reorder_item))
          {
             if (item->moving) return;

             if (!item->wd->move_effect_enabled)
               {
                  item->ox = x;
                  item->oy = y;
               }
             if (item->wd->reorder_item == item)
               {
                  evas_object_move(item->base.view,
                                   item->wd->reorder_item_x, item->wd->reorder_item_y);
                  evas_object_resize(item->base.view,
                                     item->wd->item_width, item->wd->item_height);
                  return;
               }
             else
               {
                  if (item->wd->move_effect_enabled)
                    {
                       if ((item->ox != x) || (item->oy != y))
                         {
                            if (((item->wd->old_pan_x == item->wd->pan_x) && (item->wd->old_pan_y == item->wd->pan_y)) ||
                                ((item->wd->old_pan_x != item->wd->pan_x) && !(item->ox - item->wd->pan_x + item->wd->old_pan_x == x)) ||
                                ((item->wd->old_pan_y != item->wd->pan_y) && !(item->oy - item->wd->pan_y + item->wd->old_pan_y == y)))
                              {
                                 item->tx = x;
                                 item->ty = y;
                                 item->rx = item->ox;
                                 item->ry = item->oy;
                                 item->moving = EINA_TRUE;
                                 item->moving_effect_start_time = ecore_loop_time_get();
                                 item->item_moving_effect_timer = ecore_animator_add(_reorder_item_moving_effect_timer_cb, item);
                                 return;
                              }
                         }
                    }

                  if (ELM_RECTS_INTERSECT(item->wd->reorder_item_x, item->wd->reorder_item_y,
                                          item->wd->item_width, item->wd->item_height,
                                          x+(item->wd->item_width/2), y+(item->wd->item_height/2),
                                          1, 1))
                    {
                       if (item->wd->horizontal)
                         {
                            if ((item->wd->nmax * item->wd->reorder_item->x + item->wd->reorder_item->y) >
                                (item->wd->nmax * item->x + item->y))
                              reorder_item_move_forward = EINA_TRUE;
                         }
                       else
                         {
                            if ((item->wd->nmax * item->wd->reorder_item->y + item->wd->reorder_item->x) >
                                (item->wd->nmax * item->y + item->x))
                              reorder_item_move_forward = EINA_TRUE;
                         }

                       item->wd->items = eina_inlist_remove(item->wd->items,
                                                            EINA_INLIST_GET(item->wd->reorder_item));
                       if (reorder_item_move_forward)
                         item->wd->items = eina_inlist_prepend_relative(item->wd->items,
                                                                        EINA_INLIST_GET(item->wd->reorder_item),
                                                                        EINA_INLIST_GET(item));
                       else
                         item->wd->items = eina_inlist_append_relative(item->wd->items,
                                                                       EINA_INLIST_GET(item->wd->reorder_item),
                                                                       EINA_INLIST_GET(item));

                       item->wd->reorder_item_changed = EINA_TRUE;
                       item->wd->move_effect_enabled = EINA_TRUE;
                       if (item->wd->calc_job) ecore_job_del(item->wd->calc_job);
                         item->wd->calc_job = ecore_job_add(_calc_job, item->wd);

                       return;
                    }
               }
          }
        evas_object_move(item->base.view, x, y);
        evas_object_resize(item->base.view, item->wd->item_width,
                           item->wd->item_height);
     }
   else
     {
        _item_unrealize(item);
        if (was_realized)
          evas_object_smart_callback_call(item->wd->self, SIG_UNREALIZED, item);
     }
}

static Elm_Gengrid_Item *
_item_create(Widget_Data                  *wd,
             const Elm_Gengrid_Item_Class *gic,
             const void                   *data,
             Evas_Smart_Cb                 func,
             const void                   *func_data)
{
   Elm_Gengrid_Item *item;

   item = elm_widget_item_new(wd->self, Elm_Gengrid_Item);
   if (!item) return NULL;
   wd->count++;
   item->wd = wd;
   item->gic = gic;
   item->base.data = data;
   item->func.func = func;
   item->func.data = func_data;
   item->mouse_cursor = NULL;
   return item;
}

static void
_item_del(Elm_Gengrid_Item *item)
{
   elm_widget_item_pre_notify_del(item);
   if (item->selected)
     item->wd->selected = eina_list_remove(item->wd->selected, item);
   if (item->realized) _item_unrealize(item);
   if ((!item->delete_me) && (item->gic->func.del))
     item->gic->func.del((void *)item->base.data, item->wd->self);
   item->delete_me = EINA_TRUE;
   item->wd->items = eina_inlist_remove(item->wd->items, EINA_INLIST_GET(item));
   if (item->long_timer) ecore_timer_del(item->long_timer);
   if (item->tooltip.del_cb)
     item->tooltip.del_cb((void *)item->tooltip.data, item->base.widget, item);
   item->wd->walking -= item->walking;
   item->wd->count--;
   if (item->wd->calc_job) ecore_job_del(item->wd->calc_job);
   item->wd->calc_job = ecore_job_add(_calc_job, item->wd);
   elm_widget_item_del(item);
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
   item->wd->walking++;
   if (item->func.func)
     item->func.func((void *)item->func.data, item->wd->self, item);
   if (!item->delete_me)
     evas_object_smart_callback_call(item->wd->self, SIG_SELECTED, item);
   item->walking--;
   item->wd->walking--;
   item->wd->last_selected_item = item;
   if ((item->wd->clear_me) && (!item->wd->walking))
     elm_gengrid_clear(item->base.widget);
   else
     {
        if ((!item->walking) && (item->delete_me))
          if (!item->relcount) _item_del(item);
     }
}

static void
_item_unselect(Elm_Gengrid_Item *item)
{
   if ((item->delete_me) || (!item->hilighted)) return;
   edje_object_signal_emit(item->base.view, "elm,state,unselected", "elm");
   item->hilighted = EINA_FALSE;
   if (item->selected)
     {
        item->selected = EINA_FALSE;
        item->wd->selected = eina_list_remove(item->wd->selected, item);
        evas_object_smart_callback_call(item->wd->self, SIG_UNSELECTED, item);
     }
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Evas_Coord minw = 0, minh = 0, nmax = 0, cvw, cvh;
   int count;

   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &cvw, &cvh);
   if ((cvw != 0) || (cvh != 0))
     {
        if ((wd->horizontal) && (wd->item_height > 0))
          nmax = cvh / wd->item_height;
        else if (wd->item_width > 0)
          nmax = cvw / wd->item_width;

        if (nmax < 1)
          nmax = 1;

        count = wd->count;
        if (wd->horizontal)
          {
             minw = ceil(count / (float)nmax) * wd->item_width;
             minh = nmax * wd->item_height;
          }
        else
          {
             minw = nmax * wd->item_width;
             minh = ceil(count / (float)nmax) * wd->item_height;
          }

        if ((minw != wd->minw) || (minh != wd->minh))
          {
             wd->minh = minh;
             wd->minw = minw;
             evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
          }

        wd->nmax = nmax;
        evas_object_smart_changed(wd->pan_smart);
     }
   wd->calc_job = NULL;
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
_pan_set(Evas_Object *obj,
         Evas_Coord   x,
         Evas_Coord   y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if ((x == sd->wd->pan_x) && (y == sd->wd->pan_y)) return;
   sd->wd->pan_x = x;
   sd->wd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_pan_get(Evas_Object *obj,
         Evas_Coord  *x,
         Evas_Coord  *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (x) *x = sd->wd->pan_x;
   if (y) *y = sd->wd->pan_y;
}

static void
_pan_child_size_get(Evas_Object *obj,
                    Evas_Coord  *w,
                    Evas_Coord  *h)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (w) *w = sd->wd->minw;
   if (h) *h = sd->wd->minh;
}

static void
_pan_max_get(Evas_Object *obj,
             Evas_Coord  *x,
             Evas_Coord  *y)
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
_pan_min_get(Evas_Object *obj,
             Evas_Coord  *x,
             Evas_Coord  *y)
{
   Pan *sd = evas_object_smart_data_get(obj);
   Evas_Coord mx, my;

   if (!sd) return;
   _pan_max_get(obj, &mx, &my);
   if (x)
     *x = -mx * sd->wd->align_x;
   if (y)
     *y = -my * sd->wd->align_y;
}

static void
_pan_resize(Evas_Object *obj,
            Evas_Coord   w,
            Evas_Coord   h)
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
   Elm_Gengrid_Item *item;

   if (!sd) return;
   if (!sd->wd->nmax) return;

   sd->wd->reorder_item_changed = EINA_FALSE;

   EINA_INLIST_FOREACH(sd->wd->items, item)
     {
        _item_place(item, cx, cy);
        if (sd->wd->reorder_item_changed) return;
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

   if ((sd->wd->reorder_mode) && (sd->wd->reorder_item))
     {
        if (!sd->wd->reorder_item_changed)
          {
             sd->wd->old_pan_x = sd->wd->pan_x;
             sd->wd->old_pan_y = sd->wd->pan_y;
          }
        sd->wd->move_effect_enabled = EINA_FALSE;
     }
   evas_object_smart_callback_call(sd->wd->self, SIG_CHANGED, NULL);
}

static void
_pan_move(Evas_Object *obj,
          Evas_Coord x __UNUSED__,
          Evas_Coord y __UNUSED__)
{
   Pan *sd = evas_object_smart_data_get(obj);
   if (!sd) return;
   if (sd->wd->calc_job) ecore_job_del(sd->wd->calc_job);
   sd->wd->calc_job = ecore_job_add(_calc_job, sd->wd);
}

static void
_hold_on(void *data       __UNUSED__,
         Evas_Object     *obj,
         void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 1);
}

static void
_hold_off(void *data       __UNUSED__,
          Evas_Object     *obj,
          void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_hold_set(wd->scr, 0);
}

static void
_freeze_on(void *data       __UNUSED__,
           Evas_Object     *obj,
           void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 1);
}

static void
_freeze_off(void *data       __UNUSED__,
            Evas_Object     *obj,
            void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_freeze_set(wd->scr, 0);
}

static void
_scr_drag_start(void            *data,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scr_drag_stop(void            *data,
               Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_scr_scroll(void            *data,
            Evas_Object *obj __UNUSED__,
            void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL, NULL);
}

static int
_elm_gengrid_item_compare_data(const void *data, const void *data1)
{
   const Elm_Gengrid_Item *item = data;
   const Elm_Gengrid_Item *item1 = data1;

   return _elm_gengrid_item_compare_data_cb(item->base.data, item1->base.data);
}

static int
_elm_gengrid_item_compare(const void *data, const void *data1)
{
   Elm_Gengrid_Item *item, *item1;
   item = ELM_GENGRID_ITEM_FROM_INLIST(data);
   item1 = ELM_GENGRID_ITEM_FROM_INLIST(data1);
   return _elm_gengrid_item_compare_cb(item, item1);
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
   Eina_Bool bounce = _elm_config->thumbscroll_bounce_enable;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "gengrid");
   elm_widget_type_set(obj, "gengrid");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "gengrid", "base",
                                       "default");
   elm_widget_resize_object_set(obj, wd->scr);

   evas_object_smart_callback_add(wd->scr, "drag,start", _scr_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scr_drag_stop, obj);
   evas_object_smart_callback_add(wd->scr, "scroll", _scr_scroll, obj);

   elm_smart_scroller_bounce_allow_set(wd->scr, bounce, bounce);

   wd->self = obj;
   wd->align_x = 0.5;
   wd->align_y = 0.5;
   wd->h_bounce = bounce;
   wd->v_bounce = bounce;
   wd->no_select = EINA_FALSE;

   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

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
                                     _pan_set, _pan_get, _pan_max_get,
                                     _pan_min_get, _pan_child_size_get);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
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
elm_gengrid_item_size_set(Evas_Object *obj,
                          Evas_Coord   w,
                          Evas_Coord   h)
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
elm_gengrid_item_size_get(const Evas_Object *obj,
                          Evas_Coord        *w,
                          Evas_Coord        *h)
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
elm_gengrid_align_set(Evas_Object *obj,
                      double       align_x,
                      double       align_y)
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
elm_gengrid_align_get(const Evas_Object *obj,
                      double            *align_x,
                      double            *align_y)
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
 * @see elm_gengrid_item_prepend()
 * @see elm_gengrid_item_insert_before()
 * @see elm_gengrid_item_insert_after()
 * @see elm_gengrid_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_item_append(Evas_Object                  *obj,
                        const Elm_Gengrid_Item_Class *gic,
                        const void                   *data,
                        Evas_Smart_Cb                 func,
                        const void                   *func_data)
{
   Elm_Gengrid_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = _item_create(wd, gic, data, func, func_data);
   if (!item) return NULL;
   wd->items = eina_inlist_append(wd->items, EINA_INLIST_GET(item));

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return item;
}

/**
 * Add item at start of the Gengrid.
 *
 * This adds an item to the beginning of the grid.
 *
 * @param obj The Gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param func Convenience function called when item is selected.
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_insert_before()
 * @see elm_gengrid_item_insert_after()
 * @see elm_gengrid_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_item_prepend(Evas_Object                  *obj,
                         const Elm_Gengrid_Item_Class *gic,
                         const void                   *data,
                         Evas_Smart_Cb                 func,
                         const void                   *func_data)
{
   Elm_Gengrid_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = _item_create(wd, gic, data, func, func_data);
   if (!item) return NULL;
   wd->items = eina_inlist_prepend(wd->items, EINA_INLIST_GET(item));

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return item;
}

/**
 * Insert and item before another in the Gengrid.
 *
 * This inserts an item before another in the grid.
 *
 * @param obj The Gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param relative The item to which insert before.
 * @param func Convenience function called when item is selected.
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_prepend()
 * @see elm_gengrid_item_insert_after()
 * @see elm_gengrid_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_item_insert_before(Evas_Object                  *obj,
                               const Elm_Gengrid_Item_Class *gic,
                               const void                   *data,
                               Elm_Gengrid_Item             *relative,
                               Evas_Smart_Cb                 func,
                               const void                   *func_data)
{
   Elm_Gengrid_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(relative, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = _item_create(wd, gic, data, func, func_data);
   if (!item) return NULL;
   wd->items = eina_inlist_prepend_relative
      (wd->items, EINA_INLIST_GET(item), EINA_INLIST_GET(relative));

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return item;
}

/**
 * Insert and item after another in the Gengrid.
 *
 * This inserts an item after another in the grid.
 *
 * @param obj The Gengrid object.
 * @param gic The item class for the item.
 * @param data The item data.
 * @param relative The item to which insert after.
 * @param func Convenience function called when item is selected.
 * @param func_data Data passed to @p func above.
 * @return A handle to the item added or NULL if not possible.
 *
 * @see elm_gengrid_item_append()
 * @see elm_gengrid_item_prepend()
 * @see elm_gengrid_item_insert_before()
 * @see elm_gengrid_item_del()
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_item_insert_after(Evas_Object                  *obj,
                              const Elm_Gengrid_Item_Class *gic,
                              const void                   *data,
                              Elm_Gengrid_Item             *relative,
                              Evas_Smart_Cb                 func,
                              const void                   *func_data)
{
   Elm_Gengrid_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(relative, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = _item_create(wd, gic, data, func, func_data);
   if (!item) return NULL;
   wd->items = eina_inlist_append_relative
      (wd->items, EINA_INLIST_GET(item), EINA_INLIST_GET(relative));

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return item;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_direct_sorted_insert(Evas_Object                  *obj,
				      const Elm_Gengrid_Item_Class *gic,
				      const void                   *data,
				      Eina_Compare_Cb               comp,
				      Evas_Smart_Cb                 func,
				      const void                   *func_data)
{
   Elm_Gengrid_Item *item;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = _item_create(wd, gic, data, func, func_data);
   if (!item) return NULL;

   _elm_gengrid_item_compare_cb = comp;
   wd->items = eina_inlist_sorted_insert(wd->items, EINA_INLIST_GET(item),
                                         _elm_gengrid_item_compare);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return item;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_sorted_insert(Evas_Object                  *obj,
                               const Elm_Gengrid_Item_Class *gic,
                               const void                   *data,
                               Eina_Compare_Cb               comp,
                               Evas_Smart_Cb                 func,
                               const void                   *func_data)
{
   _elm_gengrid_item_compare_data_cb = comp;

   return elm_gengrid_item_direct_sorted_insert(obj, gic, data, _elm_gengrid_item_compare_data, func, func_data);
}

/**
 * Remove an item from the Gengrid.
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
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   if ((item->relcount > 0) || (item->walking > 0))
     {
        item->delete_me = EINA_TRUE;
        elm_widget_item_pre_notify_del(item);
        if (item->selected)
          item->wd->selected = eina_list_remove(item->wd->selected, item);
        if (item->gic->func.del)
          item->gic->func.del((void *)item->base.data, item->wd->self);
        return;
     }

   _item_del(item);
}

/**
 * Set for what direction the Gengrid will expand.
 *
 * @param obj The Gengrid object.
 * @param setting If @c EINA_TRUE the Gengrid will expand horizontally
 * or vertically if @c EINA_FALSE.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_horizontal_set(Evas_Object *obj,
                           Eina_Bool    setting)
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
 * Get for what direction the Gengrid is expanded.
 *
 * @param obj The Gengrid object.
 * @return If the Gengrid is expanded horizontally return @c EINA_TRUE
 * else @c EINA_FALSE.
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->horizontal;
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
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->calc_job)
     {
        ecore_job_del(wd->calc_job);
        wd->calc_job = NULL;
     }

   if (wd->walking > 0)
     {
        Elm_Gengrid_Item *item;
        wd->clear_me = 1;
        EINA_INLIST_FOREACH(wd->items, item)
           item->delete_me = 1;
        return;
     }
   wd->clear_me = 0;
   while (wd->items)
     {
        Elm_Gengrid_Item *item = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);

        wd->items = eina_inlist_remove(wd->items, wd->items);
        elm_widget_item_pre_notify_del(item);
        if (item->realized) _item_unrealize(item);
        if (item->gic->func.del)
          item->gic->func.del((void *)item->base.data, wd->self);
        if (item->long_timer) ecore_timer_del(item->long_timer);
        elm_widget_item_del(item);
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
   wd->count = 0;
   evas_object_size_hint_min_set(wd->pan_smart, wd->minw, wd->minh);
   evas_object_smart_callback_call(wd->pan_smart, "changed", NULL);
}

/**
 * Get the real evas object of the Gengrid item
 *
 * This returns the actual evas object used for the specified Gengrid
 * item.  This may be NULL as it may not be created, and may be
 * deleted at any time by Gengrid. Do not modify this object (move,
 * resize, show, hide etc.) as Gengrid is controlling it. This
 * function is for querying, emitting custom signals or hooking lower
 * level callbacks for events. Do not delete this object under any
 * circumstances.
 *
 * @param item The Gengrid item.
 * @return the evas object associated to this item.
 *
 * @see elm_gengrid_item_data_get()
 *
 * @ingroup Gengrid
 */
EAPI const Evas_Object *
elm_gengrid_item_object_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   return item->base.view;
}

/**
 * Update the contents of an item
 *
 * This updates an item by calling all the item class functions again
 * to get the icons, labels and states. Use this when the original
 * item data has changed and the changes are desired to be reflected.
 *
 * @param item The item
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_update(Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   if (!item->realized) return;
   if (item->want_unrealize) return;
   _item_unrealize(item);
   _item_realize(item);
   _item_place(item, item->x, item->y);
}

/**
 * Returns the data associated to an item
 *
 * This returns the data value passed on the elm_gengrid_item_append()
 * and related item addition calls.
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
elm_gengrid_item_data_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   return elm_widget_item_data_get(item);
}

/**
 * Set the datan item from the gengrid item
 *
 * This set the data value passed on the elm_gengrid_item_append() and
 * related item addition calls. This function will also call
 * elm_gengrid_item_update() so the item will be updated to reflect
 * the new data.
 *
 * @param item The item
 * @param data The new data pointer to set
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_data_set(Elm_Gengrid_Item *item,
                          const void       *data)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   elm_widget_item_data_set(item, data);
   elm_gengrid_item_update(item);
}

EAPI const Elm_Gengrid_Item_Class *
elm_gengrid_item_item_class_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   if (item->delete_me) return NULL;
   return item->gic;
}

EAPI void
elm_gengrid_item_item_class_set(Elm_Gengrid_Item *item,
                                const Elm_Gengrid_Item_Class *gic)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   EINA_SAFETY_ON_NULL_RETURN(gic);
   if (item->delete_me) return;
   item->gic = gic;
   elm_gengrid_item_update(item);
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
elm_gengrid_item_pos_get(const Elm_Gengrid_Item *item,
                         unsigned int           *x,
                         unsigned int           *y)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   if (x) *x = item->x;
   if (y) *y = item->y;
}

/**
 * Enable or disable multi-select in the Gengrid.
 *
 * This enables (EINA_TRUE) or disables (EINA_FALSE) multi-select in
 * the Gengrid.  This allows more than 1 item to be selected.
 *
 * @param obj The Gengrid object.
 * @param multi Multi-select enabled/disabled
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_multi_select_set(Evas_Object *obj,
                             Eina_Bool    multi)
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
 * This gets the selected item in the Gengrid (if multi-select is
 * enabled only the first item in the list is selected - which is not
 * very useful, so see elm_gengrid_selected_items_get() for when
 * multi-select is used).
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
 * This returns a list of the selected items. This list pointer is
 * only valid so long as no items are selected or unselected (or
 * unselected implictly by deletion). The list contains
 * Elm_Gengrid_Item pointers.
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
 * Set the selected state of an item.
 *
 * This sets the selected state of an item. If multi-select is not
 * enabled and selected is EINA_TRUE, previously selected items are
 * unselected.
 *
 * @param item The item
 * @param selected The selected state.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_selected_set(Elm_Gengrid_Item *item,
                              Eina_Bool         selected)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   Widget_Data *wd = item->wd;
   if (!wd) return;
   if (item->delete_me) return;
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
 * Get the selected state of an item.
 *
 * This gets the selected state of an item (1 selected, 0 not selected).
 *
 * @param item The item
 * @return The selected state
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_item_selected_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, EINA_FALSE);
   return item->selected;
}

/**
 * Sets the disabled state of an item.
 *
 * A disabled item cannot be selected or unselected. It will also
 * change appearance to disabled. This sets the disabled state (1
 * disabled, 0 not disabled).
 *
 * @param item The item
 * @param disabled The disabled state
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_disabled_set(Elm_Gengrid_Item *item,
                              Eina_Bool         disabled)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   if (item->disabled == disabled) return;
   if (item->delete_me) return;
   item->disabled = !!disabled;
   if (item->realized)
     {
        if (item->disabled)
          edje_object_signal_emit(item->base.view, "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(item->base.view, "elm,state,enabled", "elm");
     }
}

/**
 * Get the disabled state of an item.
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
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, EINA_FALSE);
   if (item->delete_me) return EINA_FALSE;
   return item->disabled;
}

static Evas_Object *
_elm_gengrid_item_label_create(void        *data,
                               Evas_Object *obj,
                               void *item   __UNUSED__)
{
   Evas_Object *label = elm_label_add(obj);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);
   return label;
}

static void
_elm_gengrid_item_label_del_cb(void            *data,
                               Evas_Object *obj __UNUSED__,
                               void *event_info __UNUSED__)
{
   eina_stringshare_del(data);
}

/**
 * Set the text to be shown in the gengrid item.
 *
 * @param item Target item
 * @param text The text to set in the content
 *
 * Setup the text as tooltip to object. The item can have only one
 * tooltip, so any previous tooltip data is removed.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_tooltip_text_set(Elm_Gengrid_Item *item,
                                  const char       *text)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   text = eina_stringshare_add(text);
   elm_gengrid_item_tooltip_content_cb_set(item, _elm_gengrid_item_label_create,
                                           text,
                                           _elm_gengrid_item_label_del_cb);
}

/**
 * Set the content to be shown in the tooltip item
 *
 * Setup the tooltip to item. The item can have only one tooltip, so
 * any previous tooltip data is removed. @p func(with @p data) will be
 * called every time that need show the tooltip and it should return a
 * valid Evas_Object. This object is then managed fully by tooltip
 * system and is deleted when the tooltip is gone.
 *
 * @param item the gengrid item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @func, the tooltip is unset with
 *        elm_gengrid_item_tooltip_unset() or the owner @an item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_tooltip_content_cb_set(Elm_Gengrid_Item           *item,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void                 *data,
                                        Evas_Smart_Cb               del_cb)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_GOTO(item, error);

   if ((item->tooltip.content_cb == func) && (item->tooltip.data == data))
     return;

   if (item->tooltip.del_cb)
     item->tooltip.del_cb((void *)item->tooltip.data,
                          item->base.widget, item);
   item->tooltip.content_cb = func;
   item->tooltip.data = data;
   item->tooltip.del_cb = del_cb;
   if (item->base.view)
     {
        elm_widget_item_tooltip_content_cb_set(item,
                                               item->tooltip.content_cb,
                                               item->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(item, item->tooltip.style);
     }

   return;

error:
   if (del_cb) del_cb((void *)data, NULL, NULL);
}

/**
 * Unset tooltip from item
 *
 * @param item gengrid item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_gengrid_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @see elm_gengrid_item_tooltip_content_cb_set()
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_tooltip_unset(Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   if ((item->base.view) && (item->tooltip.content_cb))
     elm_widget_item_tooltip_unset(item);

   if (item->tooltip.del_cb)
     item->tooltip.del_cb((void *)item->tooltip.data, item->base.widget, item);
   item->tooltip.del_cb = NULL;
   item->tooltip.content_cb = NULL;
   item->tooltip.data = NULL;
   if (item->tooltip.style)
     elm_gengrid_item_tooltip_style_set(item, NULL);
}

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_gengrid_item_tooltip_content_cb_set() or
 *       elm_gengrid_item_tooltip_text_set()
 *
 * @param item gengrid item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_tooltip_style_set(Elm_Gengrid_Item *item,
                                   const char       *style)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   eina_stringshare_replace(&item->tooltip.style, style);
   if (item->base.view) elm_widget_item_tooltip_style_set(item, style);
}

/**
 * Get the style for this item tooltip.
 *
 * @param item gengrid item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @ingroup Gengrid
 */
EAPI const char *
elm_gengrid_item_tooltip_style_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   return item->tooltip.style;
}

/**
 * Set the cursor to be shown when mouse is over the gengrid item
 *
 * @param item Target item
 * @param cursor the cursor name to be used.
 *
 * @see elm_object_cursor_set()
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_cursor_set(Elm_Gengrid_Item *item,
                            const char       *cursor)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   eina_stringshare_replace(&item->mouse_cursor, cursor);
   if (item->base.view) elm_widget_item_cursor_set(item, cursor);
}

/**
 * Get the cursor to be shown when mouse is over the gengrid item
 *
 * @param item gengrid item with cursor already set.
 * @return the cursor name.
 *
 * @ingroup Gengrid
 */
EAPI const char *
elm_gengrid_item_cursor_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   return elm_widget_item_cursor_get(item);
}

/**
 * Unset the cursor to be shown when mouse is over the gengrid item
 *
 * @param item Target item
 *
 * @see elm_object_cursor_unset()
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_cursor_unset(Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   if (!item->mouse_cursor)
     return;

   if (item->base.view)
     elm_widget_item_cursor_unset(item);

   eina_stringshare_del(item->mouse_cursor);
   item->mouse_cursor = NULL;
}

/**
 * Sets a different style for this item cursor.
 *
 * @note before you set a style you should define a cursor with
 *       elm_gengrid_item_cursor_set()
 *
 * @param item gengrid item with cursor already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_cursor_style_set(Elm_Gengrid_Item *item,
                                  const char       *style)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   elm_widget_item_cursor_style_set(item, style);
}

/**
 * Get the style for this item cursor.
 *
 * @param item gengrid item with cursor already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a cursor set, then NULL is returned.
 *
 * @ingroup Gengrid
 */
EAPI const char *
elm_gengrid_item_cursor_style_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   return elm_widget_item_cursor_style_get(item);
}

/**
 * Set if the cursor set should be searched on the theme or should use
 * the provided by the engine, only.
 *
 * @note before you set if should look on theme you should define a
 * cursor with elm_object_cursor_set(). By default it will only look
 * for cursors provided by the engine.
 *
 * @param item widget item with cursor already set.
 * @param engine_only boolean to define it cursors should be looked
 * only between the provided by the engine or searched on widget's
 * theme as well.
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_cursor_engine_only_set(Elm_Gengrid_Item *item,
                                        Eina_Bool         engine_only)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   elm_widget_item_cursor_engine_only_set(item, engine_only);
}

/**
 * Get the cursor engine only usage for this item cursor.
 *
 * @param item widget item with cursor already set.
 * @return engine_only boolean to define it cursors should be looked
 * only between the provided by the engine or searched on widget's
 * theme as well. If the object does not have a cursor set, then
 * EINA_FALSE is returned.
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_item_cursor_engine_only_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, EINA_FALSE);
   return elm_widget_item_cursor_engine_only_get(item);
}

/**
 * Set the reorder mode
 *
 * @param obj The Gengrid object
 * @param reorder_mode The reorder mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_reorder_mode_set(Evas_Object *obj,
                             Eina_Bool    reorder_mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->reorder_mode = reorder_mode;
}

/**
 * Get the reorder mode
 *
 * @param obj The Gengrid object
 * @return The reorder mode
 * (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Gengrid
 */
EAPI Eina_Bool
elm_gengrid_reorder_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->reorder_mode;
}

/**
 * Set the always select mode.
 *
 * Cells will only call their selection func and callback when first
 * becoming selected. Any further clicks will do nothing, unless you
 * enable always select with
 * elm_gengrid_always_select_mode_set(). This means even if selected,
 * every click will make the selected callbacks be called.
 *
 * @param obj The Gengrid object
 * @param always_select The always select mode (EINA_TRUE = on,
 * EINA_FALSE = off)
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_always_select_mode_set(Evas_Object *obj,
                                   Eina_Bool    always_select)
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
 * This will turn off the ability to select items entirely and they
 * will neither appear selected nor call selected callback functions.
 *
 * @param obj The Gengrid object
 * @param no_select The no select mode (EINA_TRUE = on, EINA_FALSE = off)
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_no_select_mode_set(Evas_Object *obj,
                               Eina_Bool    no_select)
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
 * This will enable or disable the scroller bounce mode for the
 * Gengrid. See elm_scroller_bounce_set() for details.
 *
 * @param obj The Gengrid object
 * @param h_bounce Allow bounce horizontally
 * @param v_bounce Allow bounce vertically
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_bounce_set(Evas_Object *obj,
                       Eina_Bool    h_bounce,
                       Eina_Bool    v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_bounce_allow_set(wd->scr, h_bounce, v_bounce);
   wd->h_bounce = h_bounce;
   wd->v_bounce = v_bounce;
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
elm_gengrid_bounce_get(const Evas_Object *obj,
                       Eina_Bool         *h_bounce,
                       Eina_Bool         *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   *h_bounce = wd->h_bounce;
   *v_bounce = wd->v_bounce;
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

/**
 * Set gengrid scroll page size relative to viewport size.
 *
 * The gengrid scroller is capable of limiting scrolling by the user
 * to "pages" That is to jump by and only show a "whole page" at a
 * time as if the continuous area of the scroller content is split
 * into page sized pieces.  This sets the size of a page relative to
 * the viewport of the scroller. 1.0 is "1 viewport" is size
 * (horizontally or vertically). 0.0 turns it off in that axis. This
 * is mutually exclusive with page size (see
 * elm_gengrid_page_size_set() for more information). Likewise 0.5 is
 * "half a viewport". Sane usable valus are normally between 0.0 and
 * 1.0 including 1.0. If you only want 1 axis to be page "limited",
 * use 0.0 for the other axis.
 *
 * @param obj The gengrid object
 * @param h_pagerel The horizontal page relative size
 * @param v_pagerel The vertical page relative size
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_page_relative_set(Evas_Object *obj,
                              double       h_pagerel,
                              double       v_pagerel)
{
   Evas_Coord pagesize_h;
   Evas_Coord pagesize_v;

   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, NULL, NULL, &pagesize_h, &pagesize_v);
   elm_smart_scroller_paging_set(wd->scr, h_pagerel, v_pagerel, pagesize_h,
                                 pagesize_v);
}

/*
 * Get gengrid scroll page size relative to viewport size.
 *
 * The gengrid scroller is capable of limiting scrolling by the user
 * to "pages" That is to jump by and only show a "whole page" at a
 * time as if the continuous area of the scroller content is split
 * into page sized pieces.  This sets the size of a page relative to
 * the viewport of the scroller. 1.0 is "1 viewport" is size
 * (horizontally or vertically). 0.0 turns it off in that axis. This
 * is mutually exclusive with page size (see
 * elm_gengrid_page_size_set() for more information). Likewise 0.5 is
 * "half a viewport". Sane usable valus are normally between 0.0 and
 * 1.0 including 1.0. If you only want 1 axis to be page "limited",
 * use 0.0 for the other axis.
 *
 * @param obj The gengrid object
 * @param h_pagerel The horizontal page relative size
 * @param v_pagerel The vertical page relative size
 *
 @ingroup Gengrid
 */
EAPI void
elm_gengrid_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, h_pagerel, v_pagerel, NULL, NULL);
}

/**
 * Set gengrid scroll page size.
 *
 * See also elm_gengrid_page_relative_set(). This, instead of a page
 * size being relative to the viewport, sets it to an absolute fixed
 * value, with 0 turning it off for that axis.
 *
 * @param obj The gengrid object
 * @param h_pagesize The horizontal page size
 * @param v_pagesize The vertical page size
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_page_size_set(Evas_Object *obj,
                          Evas_Coord   h_pagesize,
                          Evas_Coord   v_pagesize)
{
   double pagerel_h;
   double pagerel_v;

   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_smart_scroller_paging_get(wd->scr, &pagerel_h, &pagerel_v, NULL, NULL);
   elm_smart_scroller_paging_set(wd->scr, pagerel_h, pagerel_v, h_pagesize,
                                 v_pagesize);
}

/**
 * Get the first item in the gengrid
 *
 * This returns the first item in the list.
 *
 * @param obj The gengrid object
 * @return The first item, or NULL if none
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_first_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Gengrid_Item *item = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);
   while ((item) && (item->delete_me))
     item = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(item)->next);
   return item;
}

/**
 * Get the last item in the gengrid
 *
 * This returns the last item in the list.
 *
 * @return The last item, or NULL if none
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_last_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Gengrid_Item *item = ELM_GENGRID_ITEM_FROM_INLIST(wd->items->last);
   while ((item) && (item->delete_me))
     item = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(item)->prev);
   return item;
}

/**
 * Get the next item in the gengrid
 *
 * This returns the item after the item @p item.
 *
 * @param item The item
 * @return The item after @p item, or NULL if none
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_item_next_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   while (item)
     {
        item = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(item)->next);
        if ((item) && (!item->delete_me)) break;
     }
   return (Elm_Gengrid_Item *)item;
}

/**
 * Get the previous item in the gengrid
 *
 * This returns the item before the item @p item.
 *
 * @param item The item
 * @return The item before @p item, or NULL if none
 *
 * @ingroup Gengrid
 */
EAPI Elm_Gengrid_Item *
elm_gengrid_item_prev_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   while (item)
     {
        item = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(item)->prev);
        if ((item) && (!item->delete_me)) break;
     }
   return (Elm_Gengrid_Item *)item;
}

/**
 * Get the gengrid object from an item
 *
 * This returns the gengrid object itself that an item belongs to.
 *
 * @param item The item
 * @return The gengrid object
 *
 * @ingroup Gengrid
 */
EAPI Evas_Object *
elm_gengrid_item_gengrid_get(const Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   return item->base.widget;
}

/**
 * Show the given item
 *
 * This causes gengrid to jump to the given item @p item and show it
 * (by scrolling), if it is not fully visible.
 *
 * @param item The item
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_show(Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   Widget_Data *wd = elm_widget_data_get(item->wd->self);
   Evas_Coord minx = 0, miny = 0;

   if (!wd) return;
   if ((!item) || (item->delete_me)) return;
   _pan_min_get(wd->pan_smart, &minx, &miny);

   elm_smart_scroller_child_region_show(item->wd->scr,
                                        item->x * wd->item_width + minx,
                                        item->y * wd->item_height + miny,
                                        item->wd->item_width,
                                        item->wd->item_height);
}

/**
 * Bring in the given item
 *
 * This causes gengrig to jump to the given item @p item and show it
 * (by scrolling), if it is not fully visible. This may use animation
 * to do so and take a period of time
 *
 * @param item The item
 *
 * @ingroup Gengrid
 */
EAPI void
elm_gengrid_item_bring_in(Elm_Gengrid_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   if (item->delete_me) return;

   Evas_Coord minx = 0, miny = 0;
   Widget_Data *wd = elm_widget_data_get(item->wd->self);
   if (!wd) return;
   _pan_min_get(wd->pan_smart, &minx, &miny);

   elm_smart_scroller_region_bring_in(item->wd->scr,
                                      item->x * wd->item_width + minx,
                                      item->y * wd->item_height + miny,
                                      item->wd->item_width,
                                      item->wd->item_height);
}
