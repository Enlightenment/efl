#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "els_scroller.h"

/* --
 * TODO:
 * Handle non-homogeneous objects too.
 */

 typedef struct _Widget_Data Widget_Data;
 typedef struct _Pan         Pan;

#define PRELOAD 1
#define REORDER_EFFECT_TIME 0.5

 struct _Elm_Gengrid_Item
{
   ELM_WIDGET_ITEM;
   EINA_INLIST;
   Evas_Object                  *spacer;
   const Elm_Gengrid_Item_Class *itc;
   Elm_Gengrid_Item             *parent;
   Ecore_Timer                  *long_timer;
   Ecore_Animator               *item_moving_effect_timer;
   Widget_Data                  *wd;
   Eina_List                    *labels, *icons, *states, *icon_objs;
   struct
     {
        Evas_Smart_Cb func;
        const void   *data;
     } func;

   Evas_Coord   x, y, gx, gy, dx, dy, ox, oy, tx, ty, rx, ry;
   unsigned int moving_effect_start_time;
   int          relcount;
   int          walking;
   int          prev_group;

   struct
     {
        const void                 *data;
        Elm_Tooltip_Item_Content_Cb content_cb;
        Evas_Smart_Cb               del_cb;
        const char                 *style;
        Eina_Bool                   free_size : 1;
     } tooltip;

   const char *mouse_cursor;

   Eina_Bool   is_group : 1;
   Eina_Bool   want_unrealize : 1;
   Eina_Bool   group_realized : 1;
   Eina_Bool   realized : 1;
   Eina_Bool   dragging : 1;
   Eina_Bool   down : 1;
   Eina_Bool   delete_me : 1;
   Eina_Bool   display_only : 1;
   Eina_Bool   disabled : 1;
   Eina_Bool   selected : 1;
   Eina_Bool   highlighted : 1;
   Eina_Bool   moving : 1;
};

struct _Widget_Data
{
   Eina_Inlist_Sorted_State *state;
   Evas_Object      *self, *scr;
   Evas_Object      *pan_smart;
   Pan              *pan;
   Eina_Inlist      *items;
   Eina_List        *group_items;
   Ecore_Job        *calc_job;
   Eina_List        *selected;
   Elm_Gengrid_Item *last_selected_item, *reorder_item;
   double            align_x, align_y;

   Evas_Coord        pan_x, pan_y, old_pan_x, old_pan_y;
   Evas_Coord        item_width, item_height; /* Each it size */
   Evas_Coord        group_item_width, group_item_height; /* Each group it size */
   Evas_Coord        minw, minh; /* Total obj size */
   Evas_Coord        reorder_item_x, reorder_item_y;
   unsigned int      nmax;
   long              count;
   long              items_lost;
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

#define ELM_GENGRID_ITEM_FROM_INLIST(it) \
   ((it) ? EINA_INLIST_CONTAINER_GET(it, Elm_Gengrid_Item) : NULL)

struct _Pan
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Widget_Data                   *wd;
};

static const char *widtype = NULL;
static void      _item_highlight(Elm_Gengrid_Item *it);
static void      _item_unrealize(Elm_Gengrid_Item *it);
static void      _item_select(Elm_Gengrid_Item *it);
static void      _item_unselect(Elm_Gengrid_Item *it);
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

static const char SIG_ACTIVATED[] = "activated";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_LONGPRESSED[] = "longpressed";
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
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_SCROLL_EDGE_TOP[] = "scroll,edge,top"; // TODO : remove this
static const char SIG_SCROLL_EDGE_BOTTOM[] = "scroll,edge,bottom"; // TODO : remove this
static const char SIG_SCROLL_EDGE_LEFT[] = "scroll,edge,left"; // TODO : remove this
static const char SIG_SCROLL_EDGE_RIGHT[] = "scroll,edge,right"; // TODO : remove this
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_MOVED[] = "moved";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_ACTIVATED, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_LONGPRESSED, ""},
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
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {SIG_SCROLL_EDGE_TOP, ""},
   {SIG_SCROLL_EDGE_BOTTOM, ""},
   {SIG_SCROLL_EDGE_LEFT, ""},
   {SIG_SCROLL_EDGE_RIGHT, ""},
   {SIG_EDGE_TOP, ""},
   {SIG_EDGE_BOTTOM, ""},
   {SIG_EDGE_LEFT, ""},
   {SIG_EDGE_RIGHT, ""},
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

   Elm_Gengrid_Item *it = NULL;
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
        it = elm_gengrid_first_item_get(obj);
        elm_gengrid_item_bring_in(it);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "End")) || (!strcmp(ev->keyname, "KP_End")))
     {
        it = elm_gengrid_last_item_get(obj);
        elm_gengrid_item_bring_in(it);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Prior")) || (!strcmp(ev->keyname, "KP_Prior")))
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
        it = elm_gengrid_selected_item_get(obj);
        evas_object_smart_callback_call(it->wd->self, SIG_ACTIVATED, it);
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
   while (wd->selected)
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
   Elm_Gengrid_Item *it;
   if (!wd) return;
   elm_smart_scroller_mirrored_set(wd->scr, rtl);
   if (!wd->items) return;
   it = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);

   while (it)
     {
        edje_object_mirrored_set(VIEW(it), rtl);
        elm_gengrid_item_update(it);
        it = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
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
_signal_callback_add_hook(Evas_Object *obj,
                          const char  *emission,
                          const char  *source,
                          Edje_Signal_Cb func_cb,
                          void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_add(elm_smart_scroller_edje_object_get(wd->scr),
                                   emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj,
                          const char  *emission,
                          const char  *source,
                          Edje_Signal_Cb func_cb,
                          void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_del_full(elm_smart_scroller_edje_object_get(wd->scr),
                                        emission, source, func_cb, data);
}

static void
_mouse_move(void        *data,
            Evas *evas   __UNUSED__,
            Evas_Object *obj,
            void        *event_info)
{
   Elm_Gengrid_Item *it = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;
   Evas_Coord ox, oy, ow, oh, it_scrl_x, it_scrl_y;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        if (!it->wd->on_hold)
          {
             it->wd->on_hold = EINA_TRUE;
             if (!it->wd->wasselected)
               _item_unselect(it);
          }
     }
   if ((it->dragging) && (it->down))
     {
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        evas_object_smart_callback_call(it->wd->self, SIG_DRAG, it);
        return;
     }
   if ((!it->down) || (it->wd->longpressed))
     {
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if ((it->wd->reorder_mode) && (it->wd->reorder_item))
          {
             evas_object_geometry_get(it->wd->pan_smart, &ox, &oy, &ow, &oh);

             it_scrl_x = ev->cur.canvas.x - it->wd->reorder_item->dx;
             it_scrl_y = ev->cur.canvas.y - it->wd->reorder_item->dy;

             if (it_scrl_x < ox) it->wd->reorder_item_x = ox;
             else if (it_scrl_x + it->wd->item_width > ox + ow)
               it->wd->reorder_item_x = ox + ow - it->wd->item_width;
             else it->wd->reorder_item_x = it_scrl_x;

             if (it_scrl_y < oy) it->wd->reorder_item_y = oy;
             else if (it_scrl_y + it->wd->item_height > oy + oh)
               it->wd->reorder_item_y = oy + oh - it->wd->item_height;
             else it->wd->reorder_item_y = it_scrl_y;

             if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
             it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
          }
        return;
     }
   if (!it->display_only)
     elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - it->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   dy = y - it->dy;
   ady = dy;
   if (ady < 0) ady = -dy;
   minw /= 2;
   minh /= 2;
   if ((adx > minw) || (ady > minh))
     {
        const char *left_drag, *right_drag;
        if (!elm_widget_mirrored_get(it->wd->self))
          {
             left_drag = SIG_DRAG_START_LEFT;
             right_drag = SIG_DRAG_START_RIGHT;
          }
        else
          {
             left_drag = SIG_DRAG_START_RIGHT;
             right_drag = SIG_DRAG_START_LEFT;
          }

        it->dragging = 1;
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if (!it->wd->wasselected)
          _item_unselect(it);
        if (dy < 0)
          {
             if (ady > adx)
               evas_object_smart_callback_call(it->wd->self, SIG_DRAG_START_UP,
                                               it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(it->wd->self,
                                                    left_drag, it);
               }
          }
        else
          {
             if (ady > adx)
               evas_object_smart_callback_call(it->wd->self,
                                               SIG_DRAG_START_DOWN, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(it->wd->self,
                                                    left_drag, it);
                  else
                    evas_object_smart_callback_call(it->wd->self,
                                                    right_drag, it);
               }
          }
     }
}

static Eina_Bool
_long_press(void *data)
{
   Elm_Gengrid_Item *it = data;

   it->long_timer = NULL;
   if ((it->disabled) || (it->dragging)) return ECORE_CALLBACK_CANCEL;
   it->wd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(it->wd->self, SIG_LONGPRESSED, it);
   if (it->wd->reorder_mode)
     {
        it->wd->reorder_item = it;
        evas_object_raise(VIEW(it));
        elm_smart_scroller_hold_set(it->wd->scr, EINA_TRUE);
        elm_smart_scroller_bounce_allow_set(it->wd->scr, EINA_FALSE, EINA_FALSE);
        edje_object_signal_emit(VIEW(it), "elm,state,reorder,enabled", "elm");
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_down(void        *data,
            Evas *evas   __UNUSED__,
            Evas_Object *obj,
            void        *event_info)
{
   Elm_Gengrid_Item *it = data;
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y;

   if (ev->button != 1) return;
   it->down = 1;
   it->dragging = 0;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   it->dx = ev->canvas.x - x;
   it->dy = ev->canvas.y - y;
   it->wd->longpressed = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = EINA_TRUE;
   else it->wd->on_hold = EINA_FALSE;
   it->wd->wasselected = it->selected;
   _item_highlight(it);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     {
        evas_object_smart_callback_call(it->wd->self, SIG_CLICKED_DOUBLE, it);
        evas_object_smart_callback_call(it->wd->self, SIG_ACTIVATED, it);
     }
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->realized)
     it->long_timer = ecore_timer_add(_elm_config->longpress_timeout,
                                        _long_press, it);
   else
     it->long_timer = NULL;
}

static void
_mouse_up(void            *data,
          Evas *evas       __UNUSED__,
          Evas_Object *obj __UNUSED__,
          void            *event_info)
{
   Elm_Gengrid_Item *it = data;
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool dragged = EINA_FALSE;

   if (ev->button != 1) return;
   it->down = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) it->wd->on_hold = EINA_TRUE;
   else it->wd->on_hold = EINA_FALSE;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->dragging)
     {
        it->dragging = EINA_FALSE;
        evas_object_smart_callback_call(it->wd->self, SIG_DRAG_STOP, it);
        dragged = EINA_TRUE;
     }
   if (it->wd->on_hold)
     {
        it->wd->longpressed = EINA_FALSE;
        it->wd->on_hold = EINA_FALSE;
        return;
     }
   if ((it->wd->reorder_mode) && (it->wd->reorder_item))
     {
        evas_object_smart_callback_call(it->wd->self, SIG_MOVED, it->wd->reorder_item);
        it->wd->reorder_item = NULL;
        it->wd->move_effect_enabled = EINA_FALSE;
        if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
          it->wd->calc_job = ecore_job_add(_calc_job, it->wd);

        elm_smart_scroller_hold_set(it->wd->scr, EINA_FALSE);
        elm_smart_scroller_bounce_allow_set(it->wd->scr, it->wd->h_bounce, it->wd->v_bounce);
        edje_object_signal_emit(VIEW(it), "elm,state,reorder,disabled", "elm");
     }
   if (it->wd->longpressed)
     {
        it->wd->longpressed = EINA_FALSE;
        if (!it->wd->wasselected) _item_unselect(it);
        it->wd->wasselected = EINA_FALSE;
        return;
     }
   if (dragged)
     {
        if (it->want_unrealize) _item_unrealize(it);
     }
   if ((it->disabled) || (dragged)) return;
   if (it->wd->multi)
     {
        if (!it->selected)
          {
             _item_highlight(it);
             _item_select(it);
          }
        else _item_unselect(it);
     }
   else
     {
        if (!it->selected)
          {
             while (it->wd->selected)
               _item_unselect(it->wd->selected->data);
          }
        else
          {
             const Eina_List *l, *l_next;
             Elm_Gengrid_Item *item2;

             EINA_LIST_FOREACH_SAFE(it->wd->selected, l, l_next, item2)
                if (item2 != it) _item_unselect(item2);
          }
        _item_highlight(it);
        _item_select(it);
     }
}

static void
_item_highlight(Elm_Gengrid_Item *it)
{
   if ((it->wd->no_select) || (it->delete_me) || (it->highlighted)) return;
   edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
   it->highlighted = EINA_TRUE;
}

static void
_item_realize(Elm_Gengrid_Item *it)
{
   char buf[1024];
   char style[1024];

   if ((it->realized) || (it->delete_me)) return;
   VIEW(it) = edje_object_add(evas_object_evas_get(it->wd->self));
   edje_object_scale_set(VIEW(it), elm_widget_scale_get(it->wd->self) *
                         _elm_config->scale);
   edje_object_mirrored_set(VIEW(it), elm_widget_mirrored_get(WIDGET(it)));
   evas_object_smart_member_add(VIEW(it), it->wd->pan_smart);
   elm_widget_sub_object_add(it->wd->self, VIEW(it));
   snprintf(style, sizeof(style), "it/%s",
            it->itc->item_style ? it->itc->item_style : "default");
   _elm_theme_object_set(it->wd->self, VIEW(it), "gengrid", style,
                         elm_widget_style_get(it->wd->self));
   it->spacer =
      evas_object_rectangle_add(evas_object_evas_get(it->wd->self));
   evas_object_color_set(it->spacer, 0, 0, 0, 0);
   elm_widget_sub_object_add(it->wd->self, it->spacer);
   evas_object_size_hint_min_set(it->spacer, 2 * _elm_config->scale, 1);
   edje_object_part_swallow(VIEW(it), "elm.swallow.pad", it->spacer);

   if (it->itc->func.label_get)
     {
        const Eina_List *l;
        const char *key;

        it->labels =
           elm_widget_stringlist_get(edje_object_data_get(VIEW(it),
                                                          "labels"));
        EINA_LIST_FOREACH(it->labels, l, key)
          {
             char *s = it->itc->func.label_get
                ((void *)it->base.data, it->wd->self, l->data);
             if (s)
               {
                  edje_object_part_text_set(VIEW(it), l->data, s);
                  free(s);
               }
          }
     }

   if (it->itc->func.icon_get)
     {
        const Eina_List *l;
        const char *key;

        it->icons =
           elm_widget_stringlist_get(edje_object_data_get(VIEW(it),
                                                          "icons"));
        EINA_LIST_FOREACH(it->icons, l, key)
          {
             Evas_Object *ic = it->itc->func.icon_get
                ((void *)it->base.data, it->wd->self, l->data);
             if (ic)
               {
                  it->icon_objs = eina_list_append(it->icon_objs, ic);
                  edje_object_part_swallow(VIEW(it), key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(it->wd->self, ic);
               }
          }
     }

   if (it->itc->func.state_get)
     {
        const Eina_List *l;
        const char *key;

        it->states =
           elm_widget_stringlist_get(edje_object_data_get(VIEW(it),
                                                          "states"));
        EINA_LIST_FOREACH(it->states, l, key)
          {
             Eina_Bool on = it->itc->func.state_get
                ((void *)it->base.data, it->wd->self, l->data);
             if (on)
               {
                  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
                  edje_object_signal_emit(VIEW(it), buf, "elm");
               }
          }
     }

   if (it->is_group)
     {
        if ((!it->wd->group_item_width) && (!it->wd->group_item_height))
          {
             edje_object_size_min_restricted_calc(VIEW(it),
                                                  &it->wd->group_item_width,
                                                  &it->wd->group_item_height,
                                                  it->wd->group_item_width,
                                                  it->wd->group_item_height);
          }
     }
   else
     {
        if ((!it->wd->item_width) && (!it->wd->item_height))
          {
             edje_object_size_min_restricted_calc(VIEW(it),
                                                  &it->wd->item_width,
                                                  &it->wd->item_height,
                                                  it->wd->item_width,
                                                  it->wd->item_height);
             elm_coords_finger_size_adjust(1, &it->wd->item_width,
                                           1, &it->wd->item_height);
          }

        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_DOWN,
                                       _mouse_down, it);
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_UP,
                                       _mouse_up, it);
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_MOVE,
                                       _mouse_move, it);

        if (it->selected)
          edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
        if (it->disabled)
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
     }
   evas_object_show(VIEW(it));

   if (it->tooltip.content_cb)
     {
        elm_widget_item_tooltip_content_cb_set(it,
                                               it->tooltip.content_cb,
                                               it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, it->tooltip.style);
        elm_widget_item_tooltip_size_restrict_disable(it, it->tooltip.free_size);
     }

   if (it->mouse_cursor)
     elm_widget_item_cursor_set(it, it->mouse_cursor);

   it->realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;
}

static void
_item_unrealize(Elm_Gengrid_Item *it)
{
   Evas_Object *icon;

   if (!it->realized) return;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   evas_object_del(VIEW(it));
   VIEW(it) = NULL;
   evas_object_del(it->spacer);
   it->spacer = NULL;
   elm_widget_stringlist_free(it->labels);
   it->labels = NULL;
   elm_widget_stringlist_free(it->icons);
   it->icons = NULL;
   elm_widget_stringlist_free(it->states);
   it->states = NULL;

   EINA_LIST_FREE(it->icon_objs, icon)
      evas_object_del(icon);

   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;
}

static Eina_Bool
_reorder_item_moving_effect_timer_cb(void *data)
{
   Elm_Gengrid_Item *it = data;
   double time, t;
   Evas_Coord dx, dy;

   time = REORDER_EFFECT_TIME;
   t = ((0.0 > (t = ecore_loop_time_get()-it->moving_effect_start_time)) ? 0.0 : t);
   dx = ((it->tx - it->ox) / 10) * _elm_config->scale;
   dy = ((it->ty - it->oy) / 10) * _elm_config->scale;

   if (t <= time)
     {
        it->rx += (1 * sin((t / time) * (M_PI / 2)) * dx);
        it->ry += (1 * sin((t / time) * (M_PI / 2)) * dy);
     }
   else
     {
        it->rx += dx;
        it->ry += dy;
     }

   if ((((dx > 0) && (it->rx >= it->tx)) || ((dx <= 0) && (it->rx <= it->tx))) &&
       (((dy > 0) && (it->ry >= it->ty)) || ((dy <= 0) && (it->ry <= it->ty))))
     {
        evas_object_move(VIEW(it), it->tx, it->ty);
        if (it->is_group)
          {
             Evas_Coord vw, vh;
             evas_object_geometry_get(it->wd->pan_smart, NULL, NULL, &vw, &vh);
             if (it->wd->horizontal)
               evas_object_resize(VIEW(it), it->wd->group_item_width, vh);
             else
               evas_object_resize(VIEW(it), vw, it->wd->group_item_height);
          }
        else
          evas_object_resize(VIEW(it), it->wd->item_width, it->wd->item_height);
        it->moving = EINA_FALSE;
        it->item_moving_effect_timer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   evas_object_move(VIEW(it), it->rx, it->ry);
   if (it->is_group)
     {
        Evas_Coord vw, vh;
        evas_object_geometry_get(it->wd->pan_smart, NULL, NULL, &vw, &vh);
        if (it->wd->horizontal)
          evas_object_resize(VIEW(it), it->wd->group_item_width, vh);
        else
          evas_object_resize(VIEW(it), vw, it->wd->group_item_height);
     }
   else
     evas_object_resize(VIEW(it), it->wd->item_width, it->wd->item_height);

   return ECORE_CALLBACK_RENEW;
}

static void
_group_item_place(Pan *sd)
{
   Evas_Coord iw, ih, vw, vh;
   Eina_List *l;
   Eina_Bool was_realized;
   Elm_Gengrid_Item *it;
   evas_object_geometry_get(sd->wd->pan_smart, NULL, NULL, &vw, &vh);
   if (sd->wd->horizontal)
     {
        iw = sd->wd->group_item_width;
        ih = vh;
     }
   else
     {
        iw = vw;
        ih = sd->wd->group_item_height;
     }
   EINA_LIST_FOREACH(sd->wd->group_items, l, it)
     {
        was_realized = it->realized;
        if (it->group_realized)
          {
             _item_realize(it);
             if (!was_realized)
               evas_object_smart_callback_call(it->wd->self, SIG_REALIZED, it);
             evas_object_move(VIEW(it), it->gx, it->gy);
             evas_object_resize(VIEW(it), iw, ih);
             evas_object_raise(VIEW(it));
          }
        else
          {
             _item_unrealize(it);
             if (was_realized)
               evas_object_smart_callback_call(it->wd->self, SIG_UNREALIZED, it);
          }
     }
}


static void
_item_place(Elm_Gengrid_Item *it,
            Evas_Coord        cx,
            Evas_Coord        cy)
{
   Evas_Coord x, y, ox, oy, cvx, cvy, cvw, cvh, iw, ih, ww;
   Evas_Coord tch, tcw, alignw = 0, alignh = 0, vw, vh;
   Eina_Bool reorder_item_move_forward = EINA_FALSE;
   long items_count;
   it->x = cx;
   it->y = cy;
   evas_object_geometry_get(it->wd->pan_smart, &ox, &oy, &vw, &vh);

   /* Preload rows/columns at each side of the Gengrid */
   cvx = ox - PRELOAD * it->wd->item_width;
   cvy = oy - PRELOAD * it->wd->item_height;
   cvw = vw + 2 * PRELOAD * it->wd->item_width;
   cvh = vh + 2 * PRELOAD * it->wd->item_height;

   alignh = 0;
   alignw = 0;

   items_count = it->wd->count - eina_list_count(it->wd->group_items) + it->wd->items_lost;
   if (it->wd->horizontal)
     {
        int columns, items_visible = 0, items_row;

        if (it->wd->item_height > 0)
          items_visible = vh / it->wd->item_height;
        if (items_visible < 1)
          items_visible = 1;

        columns = items_count / items_visible;
        if (items_count % items_visible)
          columns++;

        tcw = (it->wd->item_width * columns) + (it->wd->group_item_width * eina_list_count(it->wd->group_items));
        alignw = (vw - tcw) * it->wd->align_x;

        items_row = items_visible;
        if (items_row > it->wd->count)
          items_row = it->wd->count;
        tch = items_row * it->wd->item_height;
        alignh = (vh - tch) * it->wd->align_y;
     }
   else
     {
        int rows, items_visible = 0, items_col;

        if (it->wd->item_width > 0)
          items_visible = vw / it->wd->item_width;
        if (items_visible < 1)
          items_visible = 1;

        rows = items_count / items_visible;
        if (items_count % items_visible)
          rows++;

        tch = (it->wd->item_height * rows) + (it->wd->group_item_height * eina_list_count(it->wd->group_items));
        alignh = (vh - tch) * it->wd->align_y;

        items_col = items_visible;
        if (items_col > it->wd->count)
          items_col = it->wd->count;
        tcw = items_col * it->wd->item_width;
        alignw = (vw - tcw) * it->wd->align_x;
     }

   if (it->is_group)
     {
        if (it->wd->horizontal)
          {
             x = (((cx - it->prev_group) * it->wd->item_width) + (it->prev_group * it->wd->group_item_width)) - it->wd->pan_x + ox + alignw;
             y = 0;
             iw = it->wd->group_item_width;
             ih = vh;
          }
        else
          {
             x = 0;
             y = (((cy - it->prev_group) * it->wd->item_height) + (it->prev_group * it->wd->group_item_height)) - it->wd->pan_y + oy + alignh;
             iw = vw;
             ih = it->wd->group_item_height;
          }
        it->gx = x;
        it->gy = y;
     }
   else
     {
        if (it->wd->horizontal)
          {
             x = (((cx - it->prev_group) * it->wd->item_width) + (it->prev_group * it->wd->group_item_width)) - it->wd->pan_x + ox + alignw;
             y = (cy * it->wd->item_height) - it->wd->pan_y + oy + alignh;
          }
        else
          {
             x = (cx * it->wd->item_width) - it->wd->pan_x + ox + alignw;
             y = (((cy - it->prev_group) * it->wd->item_height) + (it->prev_group * it->wd->group_item_height)) - it->wd->pan_y + oy + alignh;
          }
        if (elm_widget_mirrored_get(it->wd->self))
          {  /* Switch items side and componsate for pan_x when in RTL mode */
             evas_object_geometry_get(it->wd->self, NULL, NULL, &ww, NULL);
             x = ww - x - it->wd->item_width - it->wd->pan_x - it->wd->pan_x;
          }
        iw = it->wd->item_width;
        ih = it->wd->item_height;
     }

   Eina_Bool was_realized = it->realized;
   if (ELM_RECTS_INTERSECT(x, y, iw, ih, cvx, cvy, cvw, cvh))
     {
        _item_realize(it);
        if (!was_realized)
          evas_object_smart_callback_call(it->wd->self, SIG_REALIZED, it);
        if (it->parent)
          {
             if (it->wd->horizontal)
               {
                  if (it->parent->gx < 0)
                    {
                       it->parent->gx = x + it->wd->item_width - it->wd->group_item_width;
                       if (it->parent->gx > 0)
                         it->parent->gx = 0;
                    }
               }
             else
               {
                  if (it->parent->gy < 0)
                    {
                       it->parent->gy = y + it->wd->item_height - it->wd->group_item_height;
                       if (it->parent->gy > 0)
                         it->parent->gy = 0;
                    }
                  it->parent->group_realized = EINA_TRUE;
               }
          }
        if (it->wd->reorder_mode)
          {
             if (it->wd->reorder_item)
               {
                  if (it->moving) return;

                  if (!it->wd->move_effect_enabled)
                    {
                       it->ox = x;
                       it->oy = y;
                    }
                  if (it->wd->reorder_item == it)
                    {
                       evas_object_move(VIEW(it),
                                        it->wd->reorder_item_x, it->wd->reorder_item_y);
                       evas_object_resize(VIEW(it), iw, ih);
                       return;
                    }
                  else
                    {
                       if (it->wd->move_effect_enabled)
                         {
                            if ((it->ox != x) || (it->oy != y))
                              {
                                 if (((it->wd->old_pan_x == it->wd->pan_x) && (it->wd->old_pan_y == it->wd->pan_y)) ||
                                     ((it->wd->old_pan_x != it->wd->pan_x) && !(it->ox - it->wd->pan_x + it->wd->old_pan_x == x)) ||
                                     ((it->wd->old_pan_y != it->wd->pan_y) && !(it->oy - it->wd->pan_y + it->wd->old_pan_y == y)))
                                   {
                                      it->tx = x;
                                      it->ty = y;
                                      it->rx = it->ox;
                                      it->ry = it->oy;
                                      it->moving = EINA_TRUE;
                                      it->moving_effect_start_time = ecore_loop_time_get();
                                      it->item_moving_effect_timer = ecore_animator_add(_reorder_item_moving_effect_timer_cb, it);
                                      return;
                                   }
                              }
                         }

                       /* need fix here */
                       Evas_Coord nx, ny, nw, nh;
                       if (it->is_group)
                         {
                            if (it->wd->horizontal)
                              {
                                 nx = x + (it->wd->group_item_width / 2);
                                 ny = y;
                                 nw = 1;
                                 nh = vh;
                              }
                            else
                              {
                                 nx = x;
                                 ny = y + (it->wd->group_item_height / 2);
                                 nw = vw;
                                 nh = 1;
                              }
                         }
                       else
                         {
                            nx = x + (it->wd->item_width / 2);
                            ny = y + (it->wd->item_height / 2);
                            nw = 1;
                            nh = 1;
                         }

                       if ( ELM_RECTS_INTERSECT(it->wd->reorder_item_x, it->wd->reorder_item_y,
                                                it->wd->item_width, it->wd->item_height,
                                                nx, ny, nw, nh))
                         {
                            if (it->wd->horizontal)
                              {
                                 if ((it->wd->nmax * it->wd->reorder_item->x + it->wd->reorder_item->y) >
                                     (it->wd->nmax * it->x + it->y))
                                   reorder_item_move_forward = EINA_TRUE;
                              }
                            else
                              {
                                 if ((it->wd->nmax * it->wd->reorder_item->y + it->wd->reorder_item->x) >
                                     (it->wd->nmax * it->y + it->x))
                                   reorder_item_move_forward = EINA_TRUE;
                              }

                            it->wd->items = eina_inlist_remove(it->wd->items,
                                                                 EINA_INLIST_GET(it->wd->reorder_item));
                            if (reorder_item_move_forward)
                              it->wd->items = eina_inlist_prepend_relative(it->wd->items,
                                                                             EINA_INLIST_GET(it->wd->reorder_item),
                                                                             EINA_INLIST_GET(it));
                            else
                              it->wd->items = eina_inlist_append_relative(it->wd->items,
                                                                            EINA_INLIST_GET(it->wd->reorder_item),
                                                                            EINA_INLIST_GET(it));

                            it->wd->reorder_item_changed = EINA_TRUE;
                            it->wd->move_effect_enabled = EINA_TRUE;
                            if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
                              it->wd->calc_job = ecore_job_add(_calc_job, it->wd);

                            return;
                         }
                    }
               }
             else if (it->item_moving_effect_timer)
               {
                  ecore_animator_del(it->item_moving_effect_timer);
                  it->item_moving_effect_timer = NULL;
                  it->moving = EINA_FALSE;
               }
          }
        if (!it->is_group)
          {
             evas_object_move(VIEW(it), x, y);
             evas_object_resize(VIEW(it), iw, ih);
          }
        else
          it->group_realized = EINA_TRUE;
     }
   else
     {
        if (!it->is_group)
          {
             _item_unrealize(it);
             if (was_realized)
               evas_object_smart_callback_call(it->wd->self, SIG_UNREALIZED, it);
          }
        else
          it->group_realized = EINA_FALSE;
     }
}

static const char *
_item_label_hook(Elm_Gengrid_Item *it, const char *part)
{
   if (!it->itc->func.label_get) return NULL;
   return edje_object_part_text_get(VIEW(it), part);
}

static Elm_Gengrid_Item *
_item_create(Widget_Data                  *wd,
             const Elm_Gengrid_Item_Class *itc,
             const void                   *data,
             Evas_Smart_Cb                 func,
             const void                   *func_data)
{
   Elm_Gengrid_Item *it;

   it = elm_widget_item_new(wd->self, Elm_Gengrid_Item);
   if (!it) return NULL;
   wd->count++;
   it->wd = wd;
   it->itc = itc;
   it->base.data = data;
   it->func.func = func;
   it->func.data = func_data;
   it->mouse_cursor = NULL;
   it->is_group = !strcmp(it->itc->item_style, "group_index");
   elm_widget_item_text_get_hook_set(it, _item_label_hook);

   return it;
}

static void
_item_del(Elm_Gengrid_Item *it)
{
   elm_widget_item_pre_notify_del(it);
   if (it->selected)
     it->wd->selected = eina_list_remove(it->wd->selected, it);
   if (it->realized) _item_unrealize(it);
   if ((!it->delete_me) && (it->itc->func.del))
     it->itc->func.del((void *)it->base.data, it->wd->self);
   it->delete_me = EINA_TRUE;
   it->wd->items = eina_inlist_remove(it->wd->items, EINA_INLIST_GET(it));
   if (it->long_timer) ecore_timer_del(it->long_timer);
   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->wd->walking -= it->walking;
   it->wd->count--;
   if (it->is_group)
     it->wd->group_items = eina_list_remove(it->wd->group_items, it);
   if (it->wd->calc_job) ecore_job_del(it->wd->calc_job);
   it->wd->calc_job = ecore_job_add(_calc_job, it->wd);
   elm_widget_item_del(it);
}

static void
_item_select(Elm_Gengrid_Item *it)
{
   if ((it->wd->no_select) || (it->delete_me)) return;
   if (it->selected)
     {
        if (it->wd->always_select) goto call;
        return;
     }
   it->selected = EINA_TRUE;
   it->wd->selected = eina_list_append(it->wd->selected, it);
call:
   it->walking++;
   it->wd->walking++;
   if (it->func.func)
     it->func.func((void *)it->func.data, it->wd->self, it);
   if (!it->delete_me)
     evas_object_smart_callback_call(it->wd->self, SIG_SELECTED, it);
   it->walking--;
   it->wd->walking--;
   it->wd->last_selected_item = it;
   if ((it->wd->clear_me) && (!it->wd->walking))
     elm_gengrid_clear(WIDGET(it));
   else
     {
        if ((!it->walking) && (it->delete_me))
          if (!it->relcount) _item_del(it);
     }
}

static void
_item_unselect(Elm_Gengrid_Item *it)
{
   if ((it->delete_me) || (!it->highlighted)) return;
   edje_object_signal_emit(VIEW(it), "elm,state,unselected", "elm");
   it->highlighted = EINA_FALSE;
   if (it->selected)
     {
        it->selected = EINA_FALSE;
        it->wd->selected = eina_list_remove(it->wd->selected, it);
        evas_object_smart_callback_call(it->wd->self, SIG_UNSELECTED, it);
     }
}

static void
_calc_job(void *data)
{
   Widget_Data *wd = data;
   Evas_Coord minw = 0, minh = 0, nmax = 0, cvw, cvh;
   Elm_Gengrid_Item *it, *group_item = NULL;
   int count_group = 0;
   long count = 0;
   wd->items_lost = 0;

   evas_object_geometry_get(wd->pan_smart, NULL, NULL, &cvw, &cvh);
   if ((cvw != 0) || (cvh != 0))
     {
        if ((wd->horizontal) && (wd->item_height > 0))
          nmax = cvh / wd->item_height;
        else if (wd->item_width > 0)
          nmax = cvw / wd->item_width;

        if (nmax < 1)
          nmax = 1;

        EINA_INLIST_FOREACH(wd->items, it)
          {
             if (it->prev_group != count_group)
               it->prev_group = count_group;
             if (it->is_group)
               {
                  count = count % nmax;
                  if (count)
                    wd->items_lost += nmax - count;
                  //printf("%d items and I lost %d\n", count, wd->items_lost);
                  count_group++;
                  if (count) count = 0;
                  group_item = it;
               }
             else
               {
                  if (it->parent != group_item)
                    it->parent = group_item;
                  count++;
               }
          }
        count = wd->count + wd->items_lost - count_group;
        if (wd->horizontal)
          {
             minw = (ceil(count / (float)nmax) * wd->item_width) + (count_group * wd->group_item_width);
             minh = nmax * wd->item_height;
          }
        else
          {
             minw = nmax * wd->item_width;
             minh = (ceil(count / (float)nmax) * wd->item_height) + (count_group * wd->group_item_height);
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
   Elm_Gengrid_Item *it;

   if (!sd) return;
   if (!sd->wd->nmax) return;

   sd->wd->reorder_item_changed = EINA_FALSE;

   EINA_INLIST_FOREACH(sd->wd->items, it)
     {
        if (it->is_group)
          {
             if (sd->wd->horizontal)
               {
                  if (cy)
                    {
                       cx++;
                       cy = 0;
                    }
               }
             else
               {
                  if (cx)
                    {
                       cx = 0;
                       cy++;
                    }
               }
          }
        _item_place(it, cx, cy);
        if (sd->wd->reorder_item_changed) return;
        if (it->is_group)
          {
             if (sd->wd->horizontal)
               {
                  cx++;
                  cy = 0;
               }
             else
               {
                  cx = 0;
                  cy++;
               }
          }
        else
          {
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
   _group_item_place(sd);


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
_scr_anim_start(void        *data,
                Evas_Object *obj __UNUSED__,
                void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scr_anim_stop(void        *data,
                Evas_Object *obj __UNUSED__,
                void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_STOP, NULL);
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
_edge_left(void        *data,
           Evas_Object *scr __UNUSED__,
           void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_EDGE_LEFT, NULL);
   evas_object_smart_callback_call(data, SIG_EDGE_LEFT, NULL);
}

static void
_edge_right(void        *data,
            Evas_Object *scr __UNUSED__,
            void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_EDGE_RIGHT, NULL);
   evas_object_smart_callback_call(data, SIG_EDGE_RIGHT, NULL);
}

static void
_edge_top(void        *data,
          Evas_Object *scr __UNUSED__,
          void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_EDGE_TOP, NULL);
   evas_object_smart_callback_call(data, SIG_EDGE_TOP, NULL);
}

static void
_edge_bottom(void        *data,
             Evas_Object *scr __UNUSED__,
             void        *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_SCROLL_EDGE_BOTTOM, NULL);
   evas_object_smart_callback_call(data, SIG_EDGE_BOTTOM, NULL);
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
   const Elm_Gengrid_Item *it = data;
   const Elm_Gengrid_Item *item1 = data1;

   return _elm_gengrid_item_compare_data_cb(it->base.data, item1->base.data);
}

static int
_elm_gengrid_item_compare(const void *data, const void *data1)
{
   Elm_Gengrid_Item *it, *item1;
   it = ELM_GENGRID_ITEM_FROM_INLIST(data);
   item1 = ELM_GENGRID_ITEM_FROM_INLIST(data1);
   return _elm_gengrid_item_compare_cb(it, item1);
}

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
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_event_hook_set(obj, _event_hook);

   wd->scr = elm_smart_scroller_add(e);
   elm_smart_scroller_widget_set(wd->scr, obj);
   elm_smart_scroller_object_theme_set(obj, wd->scr, "gengrid", "base",
                                       "default");
   elm_widget_resize_object_set(obj, wd->scr);

   evas_object_smart_callback_add(wd->scr, "animate,start", _scr_anim_start, obj);
   evas_object_smart_callback_add(wd->scr, "animate,stop", _scr_anim_stop, obj);
   evas_object_smart_callback_add(wd->scr, "drag,start", _scr_drag_start, obj);
   evas_object_smart_callback_add(wd->scr, "drag,stop", _scr_drag_stop, obj);
   evas_object_smart_callback_add(wd->scr, "edge,left", _edge_left, obj);
   evas_object_smart_callback_add(wd->scr, "edge,right", _edge_right, obj);
   evas_object_smart_callback_add(wd->scr, "edge,top", _edge_top, obj);
   evas_object_smart_callback_add(wd->scr, "edge,bottom", _edge_bottom,
                                  obj);
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

EAPI void
elm_gengrid_group_item_size_set(Evas_Object *obj,
                          Evas_Coord   w,
                          Evas_Coord   h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if ((wd->group_item_width == w) && (wd->group_item_height == h)) return;
   wd->group_item_width = w;
   wd->group_item_height = h;
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);
}

EAPI void
elm_gengrid_group_item_size_get(const Evas_Object *obj,
                          Evas_Coord        *w,
                          Evas_Coord        *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (w) *w = wd->group_item_width;
   if (h) *h = wd->group_item_height;
}

EAPI void
elm_gengrid_align_set(Evas_Object *obj,
                      double       align_x,
                      double       align_y)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   double old_h = wd->align_x, old_y = wd->align_y;

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

   if ((old_h != wd->align_x) || (old_y != wd->align_y))
     evas_object_smart_calculate(wd->pan_smart);
}

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

EAPI Elm_Gengrid_Item *
elm_gengrid_item_append(Evas_Object                  *obj,
                        const Elm_Gengrid_Item_Class *itc,
                        const void                   *data,
                        Evas_Smart_Cb                 func,
                        const void                   *func_data)
{
   Elm_Gengrid_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_create(wd, itc, data, func, func_data);
   if (!it) return NULL;
   wd->items = eina_inlist_append(wd->items, EINA_INLIST_GET(it));

   if (it->is_group)
     wd->group_items = eina_list_prepend(wd->group_items, it);

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return it;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_prepend(Evas_Object                  *obj,
                         const Elm_Gengrid_Item_Class *itc,
                         const void                   *data,
                         Evas_Smart_Cb                 func,
                         const void                   *func_data)
{
   Elm_Gengrid_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_create(wd, itc, data, func, func_data);
   if (!it) return NULL;
   wd->items = eina_inlist_prepend(wd->items, EINA_INLIST_GET(it));
   if (it->is_group)
     wd->group_items = eina_list_append(wd->group_items, it);

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return it;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_insert_before(Evas_Object                  *obj,
                               const Elm_Gengrid_Item_Class *itc,
                               const void                   *data,
                               Elm_Gengrid_Item             *relative,
                               Evas_Smart_Cb                 func,
                               const void                   *func_data)
{
   Elm_Gengrid_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(relative, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_create(wd, itc, data, func, func_data);
   if (!it) return NULL;
   wd->items = eina_inlist_prepend_relative
      (wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(relative));
   if (it->is_group)
     wd->group_items = eina_list_append_relative(wd->group_items, it, relative->parent);

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return it;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_insert_after(Evas_Object                  *obj,
                              const Elm_Gengrid_Item_Class *itc,
                              const void                   *data,
                              Elm_Gengrid_Item             *relative,
                              Evas_Smart_Cb                 func,
                              const void                   *func_data)
{
   Elm_Gengrid_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   EINA_SAFETY_ON_NULL_RETURN_VAL(relative, NULL);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_create(wd, itc, data, func, func_data);
   if (!it) return NULL;
   wd->items = eina_inlist_append_relative
      (wd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(relative));
   if (it->is_group)
     wd->group_items = eina_list_prepend_relative(wd->group_items, it, relative->parent);

   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return it;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_direct_sorted_insert(Evas_Object                  *obj,
				      const Elm_Gengrid_Item_Class *itc,
				      const void                   *data,
				      Eina_Compare_Cb               comp,
				      Evas_Smart_Cb                 func,
				      const void                   *func_data)
{
   Elm_Gengrid_Item *it;
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_create(wd, itc, data, func, func_data);
   if (!it) return NULL;

   if (!wd->state)
     wd->state = eina_inlist_sorted_state_new();

   _elm_gengrid_item_compare_cb = comp;
   wd->items = eina_inlist_sorted_state_insert(wd->items, EINA_INLIST_GET(it),
                                         _elm_gengrid_item_compare, wd->state);
   if (wd->calc_job) ecore_job_del(wd->calc_job);
   wd->calc_job = ecore_job_add(_calc_job, wd);

   return it;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_sorted_insert(Evas_Object                  *obj,
                               const Elm_Gengrid_Item_Class *itc,
                               const void                   *data,
                               Eina_Compare_Cb               comp,
                               Evas_Smart_Cb                 func,
                               const void                   *func_data)
{
   _elm_gengrid_item_compare_data_cb = comp;

   return elm_gengrid_item_direct_sorted_insert(obj, itc, data, _elm_gengrid_item_compare_data, func, func_data);
}

EAPI void
elm_gengrid_item_del(Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   if ((it->relcount > 0) || (it->walking > 0))
     {
        it->delete_me = EINA_TRUE;
        elm_widget_item_pre_notify_del(it);
        if (it->selected)
          it->wd->selected = eina_list_remove(it->wd->selected, it);
        if (it->itc->func.del)
          it->itc->func.del((void *)it->base.data, it->wd->self);
        return;
     }

   _item_del(it);
}

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

EAPI Eina_Bool
elm_gengrid_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->horizontal;
}

EAPI void
elm_gengrid_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->state)
     {
        eina_inlist_sorted_state_free(wd->state);
        wd->state = NULL;
     }
   if (wd->calc_job)
     {
        ecore_job_del(wd->calc_job);
        wd->calc_job = NULL;
     }

   if (wd->walking > 0)
     {
        Elm_Gengrid_Item *it;
        wd->clear_me = 1;
        EINA_INLIST_FOREACH(wd->items, it)
           it->delete_me = 1;
        return;
     }
   wd->clear_me = 0;
   while (wd->items)
     {
        Elm_Gengrid_Item *it = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);

        wd->items = eina_inlist_remove(wd->items, wd->items);
        elm_widget_item_pre_notify_del(it);
        if (it->realized) _item_unrealize(it);
        if (it->itc->func.del)
          it->itc->func.del((void *)it->base.data, wd->self);
        if (it->long_timer) ecore_timer_del(it->long_timer);
        elm_widget_item_del(it);
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

EAPI const Evas_Object *
elm_gengrid_item_object_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   return VIEW(it);
}

EAPI void
elm_gengrid_item_update(Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   if (!it->realized) return;
   if (it->want_unrealize) return;
   _item_unrealize(it);
   _item_realize(it);
   _item_place(it, it->x, it->y);
}

EAPI void *
elm_gengrid_item_data_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   return elm_widget_item_data_get(it);
}

EAPI void
elm_gengrid_item_data_set(Elm_Gengrid_Item *it,
                          const void       *data)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   elm_widget_item_data_set(it, data);
}

EAPI const Elm_Gengrid_Item_Class *
elm_gengrid_item_item_class_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   if (it->delete_me) return NULL;
   return it->itc;
}

EAPI void
elm_gengrid_item_item_class_set(Elm_Gengrid_Item *it,
                                const Elm_Gengrid_Item_Class *itc)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   EINA_SAFETY_ON_NULL_RETURN(itc);
   if (it->delete_me) return;
   it->itc = itc;
   elm_gengrid_item_update(it);
}

EAPI void
elm_gengrid_item_pos_get(const Elm_Gengrid_Item *it,
                         unsigned int           *x,
                         unsigned int           *y)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   if (x) *x = it->x;
   if (y) *y = it->y;
}

EAPI void
elm_gengrid_multi_select_set(Evas_Object *obj,
                             Eina_Bool    multi)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->multi = multi;
}

EAPI Eina_Bool
elm_gengrid_multi_select_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->multi;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_selected_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (wd->selected) return wd->selected->data;
   return NULL;
}

EAPI const Eina_List *
elm_gengrid_selected_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->selected;
}

EAPI void
elm_gengrid_item_selected_set(Elm_Gengrid_Item *it,
                              Eina_Bool         selected)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   Widget_Data *wd = it->wd;
   if (!wd) return;
   if (it->delete_me) return;
   selected = !!selected;
   if (it->selected == selected) return;

   if (selected)
     {
        if (!wd->multi)
          {
             while (wd->selected)
               _item_unselect(wd->selected->data);
          }
        _item_highlight(it);
        _item_select(it);
     }
   else
     _item_unselect(it);
}

EAPI Eina_Bool
elm_gengrid_item_selected_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, EINA_FALSE);
   return it->selected;
}

EAPI void
elm_gengrid_item_disabled_set(Elm_Gengrid_Item *it,
                              Eina_Bool         disabled)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   if (it->disabled == disabled) return;
   if (it->delete_me) return;
   it->disabled = !!disabled;
   if (it->realized)
     {
        if (it->disabled)
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");
     }
}

EAPI Eina_Bool
elm_gengrid_item_disabled_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, EINA_FALSE);
   if (it->delete_me) return EINA_FALSE;
   return it->disabled;
}

static Evas_Object *
_elm_gengrid_item_label_create(void        *data,
                               Evas_Object *obj __UNUSED__,
                               Evas_Object *tooltip,
                               void *it   __UNUSED__)
{
   Evas_Object *label = elm_label_add(tooltip);
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

EAPI void
elm_gengrid_item_tooltip_text_set(Elm_Gengrid_Item *it,
                                  const char       *text)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   text = eina_stringshare_add(text);
   elm_gengrid_item_tooltip_content_cb_set(it, _elm_gengrid_item_label_create,
                                           text,
                                           _elm_gengrid_item_label_del_cb);
}

EAPI void
elm_gengrid_item_tooltip_content_cb_set(Elm_Gengrid_Item           *it,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void                 *data,
                                        Evas_Smart_Cb               del_cb)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_GOTO(it, error);

   if ((it->tooltip.content_cb == func) && (it->tooltip.data == data))
     return;

   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data,
                          WIDGET(it), it);
   it->tooltip.content_cb = func;
   it->tooltip.data = data;
   it->tooltip.del_cb = del_cb;
   if (VIEW(it))
     {
        elm_widget_item_tooltip_content_cb_set(it,
                                               it->tooltip.content_cb,
                                               it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, it->tooltip.style);
        elm_widget_item_tooltip_size_restrict_disable(it, it->tooltip.free_size);
     }

   return;

error:
   if (del_cb) del_cb((void *)data, NULL, NULL);
}

EAPI void
elm_gengrid_item_tooltip_unset(Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   if ((VIEW(it)) && (it->tooltip.content_cb))
     elm_widget_item_tooltip_unset(it);

   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->tooltip.del_cb = NULL;
   it->tooltip.content_cb = NULL;
   it->tooltip.data = NULL;
   it->tooltip.free_size = EINA_FALSE;
   if (it->tooltip.style)
     elm_gengrid_item_tooltip_style_set(it, NULL);
}

EAPI void
elm_gengrid_item_tooltip_style_set(Elm_Gengrid_Item *it,
                                   const char       *style)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   eina_stringshare_replace(&it->tooltip.style, style);
   if (VIEW(it)) elm_widget_item_tooltip_style_set(it, style);
}

EAPI const char *
elm_gengrid_item_tooltip_style_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   return it->tooltip.style;
}

EAPI Eina_Bool
elm_gengrid_item_tooltip_size_restrict_disable(Elm_Gengrid_Item *it, Eina_Bool disable)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, EINA_FALSE);
   it->tooltip.free_size = disable;
   if (VIEW(it)) return elm_widget_item_tooltip_size_restrict_disable(it, disable);
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_gengrid_item_tooltip_size_restrict_disabled_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, EINA_FALSE);
   return it->tooltip.free_size;
}

EAPI void
elm_gengrid_item_cursor_set(Elm_Gengrid_Item *it,
                            const char       *cursor)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   eina_stringshare_replace(&it->mouse_cursor, cursor);
   if (VIEW(it)) elm_widget_item_cursor_set(it, cursor);
}

EAPI const char *
elm_gengrid_item_cursor_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   return elm_widget_item_cursor_get(it);
}

EAPI void
elm_gengrid_item_cursor_unset(Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   if (!it->mouse_cursor)
     return;

   if (VIEW(it))
     elm_widget_item_cursor_unset(it);

   eina_stringshare_del(it->mouse_cursor);
   it->mouse_cursor = NULL;
}

EAPI void
elm_gengrid_item_cursor_style_set(Elm_Gengrid_Item *it,
                                  const char       *style)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   elm_widget_item_cursor_style_set(it, style);
}

EAPI const char *
elm_gengrid_item_cursor_style_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   return elm_widget_item_cursor_style_get(it);
}

EAPI void
elm_gengrid_item_cursor_engine_only_set(Elm_Gengrid_Item *it,
                                        Eina_Bool         engine_only)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   elm_widget_item_cursor_engine_only_set(it, engine_only);
}

EAPI Eina_Bool
elm_gengrid_item_cursor_engine_only_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, EINA_FALSE);
   return elm_widget_item_cursor_engine_only_get(it);
}

EAPI void
elm_gengrid_reorder_mode_set(Evas_Object *obj,
                             Eina_Bool    reorder_mode)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->reorder_mode = reorder_mode;
}

EAPI Eina_Bool
elm_gengrid_reorder_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->reorder_mode;
}

EAPI void
elm_gengrid_always_select_mode_set(Evas_Object *obj,
                                   Eina_Bool    always_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->always_select = always_select;
}

EAPI Eina_Bool
elm_gengrid_always_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->always_select;
}

EAPI void
elm_gengrid_no_select_mode_set(Evas_Object *obj,
                               Eina_Bool    no_select)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->no_select = no_select;
}

EAPI Eina_Bool
elm_gengrid_no_select_mode_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->no_select;
}

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

EAPI void
elm_gengrid_bounce_get(const Evas_Object *obj,
                       Eina_Bool         *h_bounce,
                       Eina_Bool         *v_bounce)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (h_bounce) *h_bounce = wd->h_bounce;
   if (v_bounce) *v_bounce = wd->v_bounce;
}

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

EAPI void
elm_gengrid_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_smart_scroller_paging_get(wd->scr, h_pagerel, v_pagerel, NULL, NULL);
}

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

EAPI void
elm_gengrid_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_current_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gengrid_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_last_page_get(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gengrid_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_page_show(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gengrid_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->scr)
     elm_smart_scroller_page_bring_in(wd->scr, h_pagenumber, v_pagenumber);
}

EAPI Elm_Gengrid_Item *
elm_gengrid_first_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Gengrid_Item *it = ELM_GENGRID_ITEM_FROM_INLIST(wd->items);
   while ((it) && (it->delete_me))
     it = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
   return it;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_last_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->items) return NULL;
   Elm_Gengrid_Item *it = ELM_GENGRID_ITEM_FROM_INLIST(wd->items->last);
   while ((it) && (it->delete_me))
     it = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
   return it;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_next_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   while (it)
     {
        it = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        if ((it) && (!it->delete_me)) break;
     }
   return (Elm_Gengrid_Item *)it;
}

EAPI Elm_Gengrid_Item *
elm_gengrid_item_prev_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   while (it)
     {
        it = ELM_GENGRID_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
        if ((it) && (!it->delete_me)) break;
     }
   return (Elm_Gengrid_Item *)it;
}

EAPI Evas_Object *
elm_gengrid_item_gengrid_get(const Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, NULL);
   return WIDGET(it);
}

EAPI void
elm_gengrid_item_show(Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   Widget_Data *wd = elm_widget_data_get(it->wd->self);
   Evas_Coord minx = 0, miny = 0;

   if (!wd) return;
   if ((!it) || (it->delete_me)) return;
   _pan_min_get(wd->pan_smart, &minx, &miny);

   if (wd->horizontal)
     elm_smart_scroller_region_bring_in(it->wd->scr,
                                        ((it->x - it->prev_group) * wd->item_width) + (it->prev_group * it->wd->group_item_width) + minx,
                                        it->y * wd->item_height + miny,
                                        it->wd->item_width,
                                        it->wd->item_height);
   else
     elm_smart_scroller_region_bring_in(it->wd->scr,
                                        it->x * wd->item_width + minx,
                                        ((it->y - it->prev_group)* wd->item_height) + (it->prev_group * it->wd->group_item_height) + miny,
                                        it->wd->item_width,
                                        it->wd->item_height);
}

EAPI void
elm_gengrid_item_bring_in(Elm_Gengrid_Item *it)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it);
   if (it->delete_me) return;

   Evas_Coord minx = 0, miny = 0;
   Widget_Data *wd = elm_widget_data_get(it->wd->self);
   if (!wd) return;
   _pan_min_get(wd->pan_smart, &minx, &miny);

   if (wd->horizontal)
     elm_smart_scroller_region_bring_in(it->wd->scr,
                                        ((it->x - it->prev_group) * wd->item_width) + (it->prev_group * it->wd->group_item_width) + minx,
                                        it->y * wd->item_height + miny,
                                        it->wd->item_width,
                                        it->wd->item_height);
   else
     elm_smart_scroller_region_bring_in(it->wd->scr,
                                        it->x * wd->item_width + minx,
                                        ((it->y - it->prev_group)* wd->item_height) + (it->prev_group * it->wd->group_item_height) + miny,
                                        it->wd->item_width,
                                        it->wd->item_height);
}
