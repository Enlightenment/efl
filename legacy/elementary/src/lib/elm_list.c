#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_list.h"
#include "elm_interface_scrollable.h"

#define MY_CLASS ELM_OBJ_LIST_CLASS

#define MY_CLASS_NAME "Elm_List"
#define MY_CLASS_NAME_LEGACY "elm_list"

#define ELM_LIST_SWIPE_TIME 0.4

static const char SIG_ACTIVATED[] = "activated";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_SELECTED[] = "selected";
static const char SIG_UNSELECTED[] = "unselected";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_SWIPE[] = "swipe";
static const char SIG_HIGHLIGHTED[] = "highlighted";
static const char SIG_UNHIGHLIGHTED[] = "unhighlighted";
static const char SIG_ITEM_FOCUSED[] = "item,focused";
static const char SIG_ITEM_UNFOCUSED[] = "item,unfocused";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_ACTIVATED, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_SELECTED, ""},
   {SIG_UNSELECTED, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_EDGE_TOP, ""},
   {SIG_EDGE_BOTTOM, ""},
   {SIG_EDGE_LEFT, ""},
   {SIG_EDGE_RIGHT, ""},
   {SIG_SWIPE, ""},
   {SIG_HIGHLIGHTED, ""},
   {SIG_UNHIGHLIGHTED, ""},
   {SIG_ITEM_FOCUSED, ""},
   {SIG_ITEM_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void _size_hints_changed_cb(void *, Evas *, Evas_Object *, void *);
static void _mouse_up_cb(void *, Evas *, Evas_Object *, void *);
static void _mouse_down_cb(void *, Evas *, Evas_Object *, void *);
static void _mouse_move_cb(void *, Evas *, Evas_Object *, void *);
static void _mouse_in_cb(void *, Evas *, Evas_Object *, void *);
static void _items_fix(Evas_Object *);

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_select(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_escape(Evas_Object *obj, const char *params);


static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"select", _key_action_select},
   {"escape", _key_action_escape},
   {NULL, NULL}
};

static inline void
_elm_list_item_free(Elm_List_Item *it)
{
   ELM_LIST_DATA_GET(WIDGET(it), sd);

   if (sd->focused_item == (Elm_Object_Item *)it)
     sd->focused_item = NULL;
   if (sd->last_focused_item == (Elm_Object_Item *)it)
     sd->last_focused_item = NULL;
   if (sd->prev_focused_item == (Elm_Object_Item *)it)
     sd->prev_focused_item = NULL;
   if (sd->last_selected_item == (Elm_Object_Item *)it)
     sd->last_selected_item = NULL;


   evas_object_event_callback_del_full
     (VIEW(it), EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, it);
   evas_object_event_callback_del_full
     (VIEW(it), EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, it);
   evas_object_event_callback_del_full
     (VIEW(it), EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, it);
   evas_object_event_callback_del_full
     (VIEW(it), EVAS_CALLBACK_MOUSE_IN, _mouse_in_cb, it);

   if (it->icon)
     evas_object_event_callback_del_full
       (it->icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
       _size_hints_changed_cb, WIDGET(it));

   if (it->end)
     evas_object_event_callback_del_full
       (it->end, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
       _size_hints_changed_cb, WIDGET(it));

   ELM_SAFE_FREE(it->label, eina_stringshare_del);
   ELM_SAFE_FREE(it->swipe_timer, ecore_timer_del);
   ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
   ELM_SAFE_FREE(it->icon, evas_object_del);
   ELM_SAFE_FREE(it->end, evas_object_del);
}

static Eina_Bool
_item_multi_select_up(Elm_List_Data *sd)
{
   Elm_Object_Item *prev;

   if (!sd->selected) return EINA_FALSE;
   if (!sd->multi) return EINA_FALSE;

   prev = elm_list_item_prev(sd->last_selected_item);
   while (prev)
     {
        if (!elm_object_item_disabled_get(prev)) break;
        prev = elm_list_item_prev(prev);
     }
   if (!prev) return EINA_TRUE;

   if (elm_list_item_selected_get(prev))
     {
        elm_list_item_selected_set(sd->last_selected_item, EINA_FALSE);
        sd->last_selected_item = prev;
     }
   else
     {
        elm_list_item_selected_set(prev, EINA_TRUE);
     }
   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_down(Elm_List_Data *sd)
{
   Elm_Object_Item *next;

   if (!sd->selected) return EINA_FALSE;
   if (!sd->multi) return EINA_FALSE;

   next = elm_list_item_next(sd->last_selected_item);
   while (next)
     {
        if (!elm_object_item_disabled_get(next)) break;
        next = elm_list_item_next(next);
     }
   if (!next) return EINA_TRUE;

   if (elm_list_item_selected_get(next))
     {
        elm_list_item_selected_set(sd->last_selected_item, EINA_FALSE);
        sd->last_selected_item = next;
     }
   else
     {
        elm_list_item_selected_set(next, EINA_TRUE);
     }
   return EINA_TRUE;
}

static Eina_Bool
_all_items_unselect(Elm_List_Data *sd)
{
   if (!sd->selected) return EINA_FALSE;

   while (sd->selected)
     elm_list_item_selected_set
       ((Elm_Object_Item *)sd->selected->data, EINA_FALSE);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_up(Elm_List_Data *sd)
{
   Elm_Object_Item *prev;

   if (!sd->selected)
     prev = eina_list_data_get(eina_list_last(sd->items));
   else
     prev = elm_list_item_prev(sd->last_selected_item);

   while (prev)
     {
        if (!elm_object_item_disabled_get(prev)) break;
        prev = elm_list_item_prev(prev);
     }

   if (!prev) return EINA_FALSE;

   _all_items_unselect(sd);

   elm_list_item_selected_set(prev, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_down(Elm_List_Data *sd)
{
   Elm_Object_Item *next;

   if (!sd->selected)
     next = eina_list_data_get(sd->items);
   else
     next = elm_list_item_next(sd->last_selected_item);

   while (next)
     {
        if (!elm_object_item_disabled_get(next)) break;
        next = elm_list_item_next(next);
     }

   if (!next) return EINA_FALSE;

   _all_items_unselect(sd);

   elm_list_item_selected_set(next, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_elm_list_item_content_focus_set(Elm_List_Item *it, Elm_Focus_Direction dir,
                                 Eina_Bool h_mode)
{
   if (!it) return EINA_FALSE;
   ELM_LIST_DATA_GET(WIDGET(it), sd);

   if (!sd->focus_on_selection_enabled) return EINA_FALSE;
   if ((h_mode && (dir != ELM_FOCUS_UP) && (dir != ELM_FOCUS_DOWN)) ||
       (!h_mode && (dir != ELM_FOCUS_LEFT) && (dir != ELM_FOCUS_RIGHT)))
     return EINA_FALSE;

   int focus_objs = 0;
   Evas_Object *focus_chain[2];
   Evas_Object *focused = NULL;
   int idx;

   if (it->icon && elm_object_focus_allow_get(it->icon))
     focus_chain[focus_objs++] = it->icon;
   if (it->end && elm_object_focus_allow_get(it->end))
     focus_chain[focus_objs++] = it->end;

   if (!focus_objs)
     return EINA_FALSE;

   for (idx = 0; idx < focus_objs; idx++)
     {
        if (elm_object_focus_get(focus_chain[idx]))
          {
             focused = focus_chain[idx];
             break;
          }
     }

   if (!focused)
     {
        elm_object_focus_set(focus_chain[0], EINA_TRUE);
        return EINA_TRUE;
     }

   if (dir != ELM_FOCUS_PREVIOUS)
     {
        Evas_Object *nextfocus;
        if (elm_widget_focus_next_get(focused, dir, &nextfocus))
          {
             elm_object_focus_set(nextfocus, EINA_TRUE);
             return EINA_TRUE;
          }

        idx += ((dir == ELM_FOCUS_UP) || (dir == ELM_FOCUS_LEFT)) ? -1 : 1;
        if (idx < 0) idx = focus_objs - 1;
        if (idx >= focus_objs) idx = 0;
        focused = focus_chain[idx];
     }

   elm_object_focus_set(focused, EINA_TRUE);
   return EINA_TRUE;
}

static Elm_List_Item *
_next_item_get(Elm_List_Data *sd, Elm_List_Item *cur_it, Elm_Focus_Direction dir)
{
   Eina_List *list = NULL;
   Elm_List_Item *it = NULL;

   list = eina_list_data_find_list(sd->items, cur_it);
   if (!list) return it;
   if ((!sd->h_mode && (dir == ELM_FOCUS_UP)) ||
       ((sd->h_mode) && (dir == ELM_FOCUS_LEFT)))
     it = eina_list_data_get(eina_list_prev(list));
   else if (((!sd->h_mode) && (dir == ELM_FOCUS_DOWN)) ||
            ((sd->h_mode) && (dir == ELM_FOCUS_RIGHT)))
     it = eina_list_data_get(eina_list_next(list));

   return it;
}

static Eina_Bool
_item_focused_next(Evas_Object *obj, Elm_Focus_Direction dir)
{
   ELM_LIST_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);
   Elm_List_Item *it = NULL;

   sd->prev_focused_item = sd->focused_item;
   if (sd->focused_item)
     it = _next_item_get(sd, (Elm_List_Item *)sd->focused_item, dir);

   while (it &&
          elm_object_item_disabled_get((Elm_Object_Item *)it))
     {
        it = _next_item_get(sd, it, dir);
     }

   if (it)
     {
        elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
_elm_list_elm_widget_event_direction(Evas_Object *obj, Elm_Focus_Direction dir, Eina_Bool multi)
{
   Elm_List_Item *it = NULL;
   ELM_LIST_DATA_GET(obj, sd);
   Eina_Bool ret = EINA_FALSE;
   Evas_Coord v = 0;
   Evas_Coord min = 0;
   Eina_Bool focus_only = EINA_FALSE;

   // check if the content can get the focus by direction key
   it = (Elm_List_Item *)elm_object_focused_item_get(obj);
   if (_elm_list_item_content_focus_set(it, dir, sd->h_mode))
     return EINA_TRUE;

   if ((sd->h_mode && (dir != ELM_FOCUS_LEFT) && (dir != ELM_FOCUS_RIGHT)) ||
       (!sd->h_mode && (dir != ELM_FOCUS_UP) && (dir != ELM_FOCUS_DOWN)))
     return EINA_FALSE;

   // get content size and viewport size
   if ((dir == ELM_FOCUS_LEFT) || (dir == ELM_FOCUS_RIGHT))
     {
        eo_do(obj,
              elm_interface_scrollable_content_viewport_geometry_get
              (NULL, NULL, &v, NULL),
              elm_interface_scrollable_content_size_get(&min, NULL));
     }
   else
     {
        eo_do(obj,
              elm_interface_scrollable_content_viewport_geometry_get
              (NULL, NULL, NULL, &v),
              elm_interface_scrollable_content_size_get(NULL, &min));
     }

   // move focus or selection according to the configuration
   focus_only = _elm_config->item_select_on_focus_disable && elm_widget_focus_highlight_enabled_get(obj);
   if (focus_only)
     ret = _item_focused_next(obj, dir);
   else
     {
        if (multi)
          {
             if ((dir == ELM_FOCUS_LEFT) || (dir == ELM_FOCUS_UP))
               ret = _item_multi_select_up(sd);
             else
               ret = _item_multi_select_down(sd);
          }
        else
          {
             if ((dir == ELM_FOCUS_LEFT) || (dir == ELM_FOCUS_UP))
               ret = _item_single_select_up(sd);
             else
               ret = _item_single_select_down(sd);
          }
     }
   if (ret)
     return EINA_TRUE;

   // handle item loop feature
   if (sd->item_loop_enable)
     {
        if (min > v)
          {
             if (dir == ELM_FOCUS_LEFT)
               elm_layout_signal_emit(obj, "elm,action,looping,left", "elm");
             else if (dir == ELM_FOCUS_RIGHT)
               elm_layout_signal_emit(obj, "elm,action,looping,right", "elm");
             else if (dir == ELM_FOCUS_UP)
               elm_layout_signal_emit(obj, "elm,action,looping,up", "elm");
             else if (dir == ELM_FOCUS_DOWN)
               elm_layout_signal_emit(obj, "elm,action,looping,down", "elm");
          }
        else
          {
             if ((dir == ELM_FOCUS_LEFT) || (dir == ELM_FOCUS_UP))
               it = (Elm_List_Item *)elm_list_last_item_get(obj);
             else
               it = (Elm_List_Item *)elm_list_first_item_get(obj);

             if (focus_only)
               elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);
             else
               elm_list_item_selected_set((Elm_Object_Item *)it, EINA_TRUE);
          }
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params)
{
   ELM_LIST_DATA_GET(obj, sd);
   const char *dir = params;

   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;
   Elm_List_Item *it = NULL;

   eo_do(obj,
         elm_interface_scrollable_content_pos_get(&x, &y),
         elm_interface_scrollable_step_size_get(&step_x, &step_y),
         elm_interface_scrollable_page_size_get(&page_x, &page_y),
         elm_interface_scrollable_content_viewport_geometry_get
         (NULL, NULL, &v_w, &v_h));

   /* TODO: fix logic for horizontal mode */
   if (!strcmp(dir, "left"))
     {
        if (_elm_list_elm_widget_event_direction(obj, ELM_FOCUS_LEFT, EINA_FALSE))
          return EINA_TRUE;
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "left_multi"))
     {
        if (_elm_list_elm_widget_event_direction(obj, ELM_FOCUS_LEFT, EINA_TRUE))
          return EINA_TRUE;
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "right"))
     {
        if (_elm_list_elm_widget_event_direction(obj, ELM_FOCUS_RIGHT, EINA_FALSE))
          return EINA_TRUE;
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "right_multi"))
     {
        if (_elm_list_elm_widget_event_direction(obj, ELM_FOCUS_RIGHT, EINA_TRUE))
          return EINA_TRUE;
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "up"))
     {
        if (_elm_list_elm_widget_event_direction(obj, ELM_FOCUS_UP, EINA_FALSE))
          return EINA_TRUE;
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "up_multi"))
     {
        if (_elm_list_elm_widget_event_direction(obj, ELM_FOCUS_UP, EINA_TRUE))
          return EINA_TRUE;
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "down"))
     {
        if (_elm_list_elm_widget_event_direction(obj, ELM_FOCUS_DOWN, EINA_FALSE))
          return EINA_TRUE;
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "down_multi"))
     {
        if (_elm_list_elm_widget_event_direction(obj, ELM_FOCUS_DOWN, EINA_TRUE))
          return EINA_TRUE;
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "first"))
     {
        it = eina_list_data_get(sd->items);
        if (it)
          {
             elm_list_item_selected_set((Elm_Object_Item *)it, EINA_TRUE);
             return EINA_TRUE;
          }
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "last"))
     {
        it = eina_list_data_get(eina_list_last(sd->items));
        if (it)
          {
             elm_list_item_selected_set((Elm_Object_Item *)it, EINA_TRUE);
             return EINA_TRUE;
          }
        else
          return EINA_FALSE;
     }
   else if (!strcmp(dir, "prior"))
     {
        if (sd->h_mode)
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
   else if (!strcmp(dir, "next"))
     {
        if (sd->h_mode)
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
   else return EINA_FALSE;

   eo_do(obj, elm_interface_scrollable_content_pos_set(x, y, EINA_TRUE));
   return EINA_TRUE;
}

static Eina_Bool _key_action_select(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_LIST_DATA_GET(obj, sd);
   Elm_List_Item *it = NULL;

   if (!_elm_config->item_select_on_focus_disable &&
       (!sd->multi) && (sd->selected))
     it = (Elm_List_Item *)elm_list_selected_item_get(obj);
   else
     it = (Elm_List_Item *)elm_object_focused_item_get(obj);
   elm_list_item_selected_set((Elm_Object_Item *)it, EINA_TRUE);
   if (it) evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);

   return EINA_TRUE;
}

static Eina_Bool _key_action_escape(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_LIST_DATA_GET(obj, sd);

   if (!_all_items_unselect(sd)) return EINA_FALSE;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_event(Eo *obj, Elm_List_Data *sd, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (!sd->items) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_translate(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   Elm_List_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_widget_item_translate(it);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_translate());

   return EINA_TRUE;
}

static void
_elm_list_deletions_process(Elm_List_Data *sd)
{
   Elm_List_Item *it;

   sd->walking++; // avoid nested deletion and also _sub_del() items_fix

   EINA_LIST_FREE(sd->to_delete, it)
     {
        sd->items = eina_list_remove_list(sd->items, it->node);

        /* issuing free because of "locking" item del pre hook */
        _elm_list_item_free(it);
        elm_widget_item_free(it);
     }

   sd->walking--;
}

EOLIAN static void
_elm_list_elm_layout_sizing_eval(Eo *obj, Elm_List_Data *sd)
{
   Evas_Coord vw = 0, vh = 0;
   Evas_Coord minw, minh, maxw, maxh, w, h, vmw, vmh;
   double xw, yw;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_size_hint_min_get(sd->box, &minw, &minh);
   evas_object_size_hint_max_get(sd->box, &maxw, &maxh);
   evas_object_size_hint_weight_get(sd->box, &xw, &yw);

   eo_do(obj, elm_interface_scrollable_content_viewport_geometry_get
         (NULL, NULL, &vw, &vh));
   if (xw > 0.0)
     {
        if ((minw > 0) && (vw < minw)) vw = minw;
        else if ((maxw > 0) && (vw > maxw))
          vw = maxw;
     }
   else if (minw > 0)
     vw = minw;
   if (yw > 0.0)
     {
        if ((minh > 0) && (vh < minh)) vh = minh;
        else if ((maxh > 0) && (vh > maxh))
          vh = maxh;
     }
   else if (minh > 0)
     vh = minh;

   evas_object_resize(sd->box, vw, vh);
   w = -1;
   h = -1;

   edje_object_size_min_calc
     (wd->resize_obj, &vmw, &vmh);

   if (sd->scr_minw) w = vmw + minw;
   if (sd->scr_minh) h = vmh + minh;

   evas_object_size_hint_max_get(obj, &maxw, &maxh);
   if ((maxw > 0) && (w > maxw))
     w = maxw;
   if ((maxh > 0) && (h > maxh))
     h = maxh;

   evas_object_size_hint_min_set(obj, w, h);
}

static void
_elm_list_content_min_limit_cb(Evas_Object *obj,
                               Eina_Bool w,
                               Eina_Bool h)
{
   ELM_LIST_DATA_GET(obj, sd);

   if ((sd->mode == ELM_LIST_LIMIT) ||
       (sd->mode == ELM_LIST_EXPAND)) return;
   sd->scr_minw = !!w;
   sd->scr_minh = !!h;

   elm_layout_sizing_eval(obj);
}

static void
_elm_list_mode_set_internal(Evas_Object *obj)
{
   Elm_List_Data *sd = eo_data_scope_get(obj, MY_CLASS);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   if (sd->mode == ELM_LIST_LIMIT)
     {
        if (!sd->h_mode)
          {
             sd->scr_minw = EINA_TRUE;
             sd->scr_minh = EINA_FALSE;
          }
        else
          {
             sd->scr_minw = EINA_FALSE;
             sd->scr_minh = EINA_TRUE;
          }
     }
   else if (sd->mode == ELM_LIST_EXPAND)
     {
        sd->scr_minw = EINA_TRUE;
        sd->scr_minh = EINA_TRUE;
     }
   else
     {
        sd->scr_minw = EINA_FALSE;
        sd->scr_minh = EINA_FALSE;
     }

   elm_layout_sizing_eval(wd->obj);
}

static inline void
_elm_list_walk(Elm_List_Data *sd)
{
   if (sd->walking < 0)
     {
        ERR("ERROR: walking was negative. fixed!\n");
        sd->walking = 0;
     }
   sd->walking++;
}

static inline void
_elm_list_unwalk(Evas_Object *obj, Elm_List_Data *sd)
{
   sd->walking--;
   if (sd->walking < 0)
     {
        ERR("ERROR: walking became negative. fixed!\n");
        sd->walking = 0;
     }

   if (sd->walking)
     return;

   if (sd->to_delete)
     _elm_list_deletions_process(sd);

   if (sd->fix_pending)
     {
        sd->fix_pending = EINA_FALSE;
        _items_fix(obj);
        elm_layout_sizing_eval(obj);
     }
}

static void
_items_fix(Evas_Object *obj)
{
   Evas_Coord minw[2] = { 0, 0 }, minh[2] = { 0, 0 };
   const Eina_List *l;
   Elm_List_Item *it;
   Evas_Coord mw, mh;
   int i, redo = 0;

   const char *style;
   const char *it_odd;
   const char *it_plain;
   const char *it_compress;
   const char *it_compress_odd;

   ELM_LIST_DATA_GET(obj, sd);

   style = elm_widget_style_get(obj);
   it_plain = sd->h_mode ? "h_item" : "item";
   it_odd = sd->h_mode ? "h_item_odd" : "item_odd";
   it_compress = sd->h_mode ? "h_item_compress" : "item_compress";
   it_compress_odd = sd->h_mode ? "h_item_compress_odd" : "item_compress_odd";

   if (sd->walking)
     {
        sd->fix_pending = EINA_TRUE;
        return;
     }

   evas_object_ref(obj);
   _elm_list_walk(sd); // watch out "return" before unwalk!

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->deleted) continue;
        if (it->icon)
          {
             evas_object_size_hint_min_get(it->icon, &mw, &mh);
             if (mw > minw[0]) minw[0] = mw;
             if (mh > minh[0]) minh[0] = mh;
          }
        if (it->end)
          {
             evas_object_size_hint_min_get(it->end, &mw, &mh);
             if (mw > minw[1]) minw[1] = mw;
             if (mh > minh[1]) minh[1] = mh;
          }
     }

   if ((minw[0] != sd->minw[0]) || (minw[1] != sd->minw[1]) ||
       (minh[0] != sd->minh[0]) || (minh[1] != sd->minh[1]))
     {
        sd->minw[0] = minw[0];
        sd->minw[1] = minw[1];
        sd->minh[0] = minh[0];
        sd->minh[1] = minh[1];
        redo = 1;
     }

   i = 0;
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->deleted)
          continue;

        it->even = i & 0x1;
        if ((it->even != it->is_even) || (!it->fixed) || (redo))
          {
             const char *stacking;

             if (it->is_separator)
               elm_widget_theme_object_set
                 (obj, VIEW(it), "separator", sd->h_mode ?
                     "vertical" : "horizontal", style);
             else if (sd->mode == ELM_LIST_COMPRESS)
               {
                  if (it->even)
                    elm_widget_theme_object_set
                      (obj, VIEW(it), "list", it_compress, style);
                  else
                    elm_widget_theme_object_set
                      (obj, VIEW(it), "list", it_compress_odd, style);
               }
             else
               {
                  if (it->even)
                    elm_widget_theme_object_set
                      (obj, VIEW(it), "list", it_plain, style);
                  else
                    elm_widget_theme_object_set
                      (obj, VIEW(it), "list", it_odd, style);
               }
             stacking = edje_object_data_get(VIEW(it), "stacking");
             if (stacking)
               {
                  if (!strcmp(stacking, "below"))
                    evas_object_lower(VIEW(it));
                  else if (!strcmp(stacking, "above"))
                    evas_object_raise(VIEW(it));
               }

             if (!it->is_separator)
               {
                  edje_object_part_text_escaped_set
                     (VIEW(it), "elm.text", it->label);
                  elm_widget_item_part_text_custom_update(it);

                  if ((!it->icon) && (minh[0] > 0))
                    {
                       it->icon = evas_object_rectangle_add
                          (evas_object_evas_get(VIEW(it)));
                       evas_object_color_set(it->icon, 0, 0, 0, 0);
                       it->dummy_icon = EINA_TRUE;
                    }
                  if ((!it->end) && (minh[1] > 0))
                    {
                       it->end = evas_object_rectangle_add
                          (evas_object_evas_get(VIEW(it)));
                       evas_object_color_set(it->end, 0, 0, 0, 0);
                       it->dummy_end = EINA_TRUE;
                    }
                  if (it->icon)
                    {
                       evas_object_size_hint_min_set(it->icon, minw[0], minh[0]);
                       evas_object_size_hint_max_set(it->icon, 99999, 99999);
                       edje_object_part_swallow
                          (VIEW(it), "elm.swallow.icon", it->icon);
                    }
                  if (it->end)
                    {
                       evas_object_size_hint_min_set(it->end, minw[1], minh[1]);
                       evas_object_size_hint_max_set(it->end, 99999, 99999);
                       edje_object_part_swallow
                          (VIEW(it), "elm.swallow.end", it->end);
                    }
                  if (eina_list_count(sd->items) == 1)
                    {
                       edje_object_signal_emit
                           (VIEW(it), "elm,state,list,single", "elm");
                    }
                  else if (l == sd->items) //1st item
                    {
                       edje_object_signal_emit
                           (VIEW(it), "elm,state,list,first", "elm");
                    }
                  else if (l == eina_list_last(sd->items))
                    {
                       edje_object_signal_emit
                           (VIEW(it), "elm,state,list,last", "elm");
                    }
                  else
                   {
                       edje_object_signal_emit
                           (VIEW(it), "elm,state,list,middle", "elm");
                   }
               }
             if (!it->fixed)
               {
                  // this may call up user and it may modify the list item
                  // but we're safe as we're flagged as walking.
                  // just don't process further
                  edje_object_message_signal_process(VIEW(it));
                  if (it->deleted)
                    continue;
                  mw = mh = -1;
                  if (!it->is_separator)
                    elm_coords_finger_size_adjust(1, &mw, 1, &mh);
                  edje_object_size_min_restricted_calc
                    (VIEW(it), &mw, &mh, mw, mh);
                  /*
                  FIXME: this fixed T179 but introduced T286.
                  Temporarily disable these lines until it gets fixed correctly.
                  Evas_Coord ew, eh;
                  edje_object_parts_extends_calc
                    (VIEW(it), NULL, NULL, &ew, &eh);
                  mw = mw > ew ? mw : ew;
                  mh = mh > eh ? mh : eh;
                  */
                  if (!it->is_separator)
                    elm_coords_finger_size_adjust(1, &mw, 1, &mh);
                  evas_object_size_hint_min_set(VIEW(it), mw, mh);
                  evas_object_show(VIEW(it));
               }
             if ((it->selected) || (it->highlighted))
               {
                  const char *select_raise;

                  // this may call up user and it may modify the list item
                  // but we're safe as we're flagged as walking.
                  // just don't process further
                  edje_object_signal_emit
                    (VIEW(it), "elm,state,selected", "elm");
                  if (it->deleted)
                    continue;

                  select_raise = edje_object_data_get(VIEW(it), "selectraise");
                  if ((select_raise) && (!strcmp(select_raise, "on")))
                    evas_object_raise(VIEW(it));
               }
             if (it->base.disabled)
               edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");

             it->fixed = EINA_TRUE;
             it->is_even = it->even;
          }

        if (!it->is_separator)
          i++;
     }

   _elm_list_mode_set_internal(obj);
   _elm_list_unwalk(obj, sd);

   //focus highlight in_theme is set by list item theme.
   _elm_widget_item_highlight_in_theme(
         obj, elm_list_first_item_get(obj));

   evas_object_unref(obj);
}

static void
_size_hints_changed_cb(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   ELM_LIST_DATA_GET(data, sd);
   if (sd->delete_me) return;

   _items_fix(data);
   elm_layout_sizing_eval(data);
}

/* FIXME: take off later. maybe this show region coords belong in the
 * interface (new api functions, set/get)? */
static void
_show_region_hook(void *data EINA_UNUSED,
                  Evas_Object *obj)
{
   Evas_Coord x, y, w, h;

   elm_widget_show_region_get(obj, &x, &y, &w, &h);
   eo_do(obj, elm_interface_scrollable_content_region_set(x, y, w, h));
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_disable(Eo *obj, Elm_List_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_disable());
   if (!int_ret) return EINA_FALSE;

   if (elm_widget_disabled_get(obj))
     {
        elm_widget_scroll_freeze_push(obj);
        elm_widget_scroll_hold_push(obj);
        /* FIXME: if we get to have a way to only un-highlight items
         * in the future, keeping them selected... */
        _all_items_unselect(sd);
     }
   else
     {
        elm_widget_scroll_freeze_pop(obj);
        elm_widget_scroll_hold_pop(obj);
     }

   return EINA_TRUE;
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   Elm_List_Item *it;
   Eina_List *n;

   ELM_LIST_DATA_GET(obj, sd);

   eo_do(obj, elm_interface_scrollable_mirrored_set(rtl));

   EINA_LIST_FOREACH(sd->items, n, it)
     edje_object_mirrored_set(VIEW(it), rtl);
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_theme_apply(Eo *obj, Elm_List_Data *sd)
{
   Elm_List_Item *it;
   Eina_List *n;

   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   EINA_LIST_FOREACH(sd->items, n, it)
     {
        edje_object_scale_set
          (VIEW(it), elm_widget_scale_get(obj) * elm_config_scale_get());
        it->fixed = EINA_FALSE;
     }

   _items_fix(obj);

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_elm_list_item_focused(Elm_List_Item *it)
{
   ELM_LIST_DATA_GET(WIDGET(it), sd);
   Evas_Coord x, y, w, h, sx, sy, sw, sh;
   const char *focus_raise;

   if ((!sd) || (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       (it == (Elm_List_Item *)sd->focused_item))
     return;
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
   evas_object_geometry_get(sd->hit_rect, &sx, &sy, &sw, &sh);
   if ((x < sx) || (y < sy)|| ((x + w) > (sx + sw)) || ((y + h) > (sy + sh)))
     {
        switch (_elm_config->focus_autoscroll_mode)
          {
           case ELM_FOCUS_AUTOSCROLL_MODE_SHOW:
              elm_list_item_show((Elm_Object_Item *)it);
              break;
           case ELM_FOCUS_AUTOSCROLL_MODE_BRING_IN:
              elm_list_item_bring_in((Elm_Object_Item *)it);
              break;
           default:
              break;
          }
     }
   sd->focused_item = (Elm_Object_Item *)it;
   if (elm_widget_focus_highlight_enabled_get(WIDGET(it)))
     {
        edje_object_signal_emit
           (VIEW(it), "elm,state,focused", "elm");
     }
   focus_raise = edje_object_data_get(VIEW(it), "focusraise");
   if ((focus_raise) && (!strcmp(focus_raise, "on")))
     evas_object_raise(VIEW(it));
   evas_object_smart_callback_call
      (WIDGET(it), SIG_ITEM_FOCUSED, it);
}

static void
_elm_list_item_unfocused(Elm_List_Item *it)
{
   Evas_Object *obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   if ((!sd) || (!sd->focused_item) ||
       (it != (Elm_List_Item *)sd->focused_item))
     return;

   sd->prev_focused_item = (Elm_Object_Item *)it;
   if (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
     return;

   if (elm_widget_focus_highlight_enabled_get(obj))
     {
        edje_object_signal_emit
           (VIEW(sd->focused_item), "elm,state,unfocused", "elm");
     }

   sd->focused_item = NULL;
   evas_object_smart_callback_call(obj, SIG_ITEM_UNFOCUSED, it);
}

/*
 * This function searches the nearest visible item based on the given item.
 * If the given item is in the list viewport, this returns the given item.
 * Or this searches other items and checks the nearest fully visible item
 * according to the given item's position.
 */
static Elm_Object_Item *
_elm_list_nearest_visible_item_get(Evas_Object *obj, Elm_Object_Item *it)
{
   Evas_Coord vx = 0, vy = 0, vw = 0, vh = 0; // list viewport geometry
   Evas_Coord ix = 0, iy = 0, iw = 0, ih = 0; // given item geometry
   Evas_Coord cx = 0, cy = 0, cw = 0, ch = 0; // candidate item geometry
   Eina_List *item_list = NULL;
   Elm_Object_Item *item = NULL;
   ELM_LIST_DATA_GET(obj, sd);
   Eina_Bool search_next = EINA_FALSE;

   if (!it) return NULL;

   evas_object_geometry_get(obj, &vx, &vy, &vw, &vh);
   evas_object_geometry_get(VIEW(it), &ix, &iy, &iw, &ih);

   item_list = eina_list_data_find_list(sd->items, it);

   if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, ix, iy, iw, ih))
     {
        if (!elm_object_item_disabled_get(it))
          return it;
        else
          search_next = EINA_TRUE;
     }

   if ((!sd->h_mode && (iy < vy)) ||
       (sd->h_mode && (iw < vw)) ||
       search_next)
     {
        while ((item_list = eina_list_next(item_list)))
          {
             item = eina_list_data_get(item_list);
             evas_object_geometry_get(VIEW(item), &cx, &cy, &cw, &ch);
             if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, cx, cy, cw, ch) &&
                 !elm_object_item_disabled_get(item))
               return item;
          }
     }
   else
     {
        while ((item_list = eina_list_prev(item_list)))
          {
             item = eina_list_data_get(item_list);
             evas_object_geometry_get(VIEW(item), &cx, &cy, &cw, &ch);
             if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, cx, cy, cw, ch) &&
                 !elm_object_item_disabled_get(item))
               return item;
          }
     }
   return it;
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_on_focus(Eo *obj, Elm_List_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Object_Item *it = NULL;
   Eina_Bool is_sel = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_on_focus());
   if (!int_ret) return EINA_FALSE;

   if (elm_widget_focus_get(obj) && sd->selected && !sd->last_selected_item)
     sd->last_selected_item = eina_list_data_get(sd->selected);

   if (!sd->items) return EINA_FALSE;

   if (elm_widget_focus_get(obj) && !sd->mouse_down)
     {
        if (sd->last_focused_item)
          it = sd->last_focused_item;
        else if (sd->last_selected_item)
          it = sd->last_selected_item;
        /* TODO: make it optional on 1.11
        else if (elm_widget_focus_highlight_enabled_get(obj))
          {
             it = eina_list_data_get(sd->items);
             is_sel = EINA_TRUE;
          }
        */

        if (it)
          {
             it = _elm_list_nearest_visible_item_get(obj, it);
             if (it)
               {
                  if (!_elm_config->item_select_on_focus_disable && is_sel)
                    elm_list_item_selected_set(it, EINA_TRUE);
                  else
                    elm_object_item_focus_set(it, EINA_TRUE);
               }
          }
     }
   else
     {
        sd->prev_focused_item = sd->focused_item;
        sd->last_focused_item = sd->focused_item;
        if (sd->focused_item)
          _elm_list_item_unfocused((Elm_List_Item *)sd->focused_item);
     }
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_sub_object_del(Eo *obj, Elm_List_Data *sd, Evas_Object *sobj)
{
   const Eina_List *l;
   Elm_List_Item *it;

   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_sub_object_del(sobj));
   if (!int_ret) return EINA_FALSE;

   if ((sobj == sd->box) || (sobj == obj)) goto end;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if ((sobj == it->icon) || (sobj == it->end))
          {
             if (it->icon == sobj) it->icon = NULL;
             if (it->end == sobj) it->end = NULL;
             evas_object_event_callback_del_full
               (sobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _size_hints_changed_cb,
               obj);
             if (!sd->walking)
               {
                  _items_fix(obj);
                  elm_layout_sizing_eval(obj);
               }
             else
               sd->fix_pending = EINA_TRUE;
             break;
          }
     }

end:
   return EINA_TRUE;
}

static void
_item_highlight(Elm_List_Item *it)
{
   Evas_Object *obj;
   const char *select_raise;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   if ((it->highlighted) || (it->base.disabled) ||
       (sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)) return;

   evas_object_ref(obj);
   _elm_list_walk(sd);

   edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
   evas_object_smart_callback_call(obj, SIG_HIGHLIGHTED, it);
   select_raise = edje_object_data_get(VIEW(it), "selectraise");
   if ((select_raise) && (!strcmp(select_raise, "on")))
     evas_object_raise(VIEW(it));
   it->highlighted = EINA_TRUE;
   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

static void
_item_select(Elm_List_Item *it)
{
   Evas_Object *obj;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   if (it->base.disabled || (sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE))
     return;
   if (it->selected)
     {
        if (sd->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS) goto call;
        return;
     }

   _elm_list_item_content_focus_set(it, ELM_FOCUS_PREVIOUS, sd->h_mode);

   it->selected = EINA_TRUE;
   sd->selected = eina_list_append(sd->selected, it);

call:
   evas_object_ref(obj);
   _elm_list_walk(sd);

   if (it->func) it->func((void *)it->base.data, WIDGET(it), it);
   evas_object_smart_callback_call(obj, SIG_SELECTED, it);
   sd->last_selected_item = (Elm_Object_Item *)it;

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

static void
_item_unhighlight(Elm_List_Item *it)
{
   Evas_Object *obj;
   const char *stacking, *select_raise;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

//   if ((!it->highlighted) || (it->base.disabled) ||
//       (sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)) return;
   if (!it->highlighted) return;

   evas_object_ref(obj);
   _elm_list_walk(sd);

   edje_object_signal_emit(VIEW(it), "elm,state,unselected", "elm");
   evas_object_smart_callback_call(obj, SIG_UNHIGHLIGHTED, it);
   stacking = edje_object_data_get(VIEW(it), "stacking");
   select_raise = edje_object_data_get(VIEW(it), "selectraise");
   if ((select_raise) && (!strcmp(select_raise, "on")))
     {
        if ((stacking) && (!strcmp(stacking, "below")))
          evas_object_lower(VIEW(it));
     }
   it->highlighted = EINA_FALSE;

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

static void
_item_unselect(Elm_List_Item *it)
{
   Evas_Object *obj;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

//   if (it->base.disabled || (sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE))
//     return;

   evas_object_ref(obj);
   _elm_list_walk(sd);

   if (sd->focus_on_selection_enabled)
     {
        if (it->icon) elm_object_focus_set(it->icon, EINA_FALSE);
        if (it->end) elm_object_focus_set(it->end, EINA_FALSE);
     }

   if (it->selected)
     {
        it->selected = EINA_FALSE;
        sd->selected = eina_list_remove(sd->selected, it);
        if (!(it->base.disabled ||
              (sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)))
          evas_object_smart_callback_call(WIDGET(it), SIG_UNSELECTED, it);
     }

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

static Eina_Bool
_swipe_cancel(void *data)
{
   Elm_List_Item *it = data;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it, ECORE_CALLBACK_CANCEL);
   ELM_LIST_DATA_GET(WIDGET(it), sd);

   sd->swipe = EINA_FALSE;
   sd->movements = 0;

   return ECORE_CALLBACK_RENEW;
}

static void
_edge_left_cb(Evas_Object *obj,
              void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_LEFT, NULL);
}

static void
_edge_right_cb(Evas_Object *obj,
               void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_RIGHT, NULL);
}

static void
_edge_top_cb(Evas_Object *obj,
             void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_TOP, NULL);
}

static void
_edge_bottom_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_BOTTOM, NULL);
}

static Eina_Bool
_long_press_cb(void *data)
{
   Elm_List_Item *it = data;
   Evas_Object *obj;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it, ECORE_CALLBACK_CANCEL);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   it->long_timer = NULL;
   if (it->base.disabled) goto end;

   sd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(WIDGET(it), SIG_LONGPRESSED, it);

end:
   return ECORE_CALLBACK_CANCEL;
}

static void
_swipe_do(Elm_List_Item *it)
{
   int i, sum = 0;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   ELM_LIST_DATA_GET(WIDGET(it), sd);

   sd->swipe = EINA_FALSE;
   for (i = 0; i < sd->movements; i++)
     {
        sum += sd->history[i].x;
        if (abs(sd->history[0].y - sd->history[i].y) > 10) return;
     }

   sum /= sd->movements;
   if (abs(sum - sd->history[0].x) <= 10) return;

   evas_object_smart_callback_call(WIDGET(it), SIG_SWIPE, it);
}

static void
_mouse_in_cb(void *data,
             Evas *evas EINA_UNUSED,
             Evas_Object *o EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   if (!elm_object_item_disabled_get(data) &&
       (_elm_config->focus_move_policy == ELM_FOCUS_MOVE_POLICY_IN))
     elm_object_item_focus_set(data, EINA_TRUE);
}

static void
_mouse_move_cb(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *o EINA_UNUSED,
               void *event_info)
{
   Evas_Object *obj;
   Elm_List_Item *it = data;
   Evas_Event_Mouse_Move *ev = event_info;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   evas_object_ref(obj);
   _elm_list_walk(sd);

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        if (!sd->on_hold)
          {
             sd->on_hold = EINA_TRUE;
             ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
             if (!sd->was_selected)
               {
                  _item_unhighlight(it);
                  _item_unselect(it);
               }
          }
        if (sd->movements == ELM_LIST_SWIPE_MOVES) sd->swipe = EINA_TRUE;
        else
          {
             sd->history[sd->movements].x = ev->cur.canvas.x;
             sd->history[sd->movements].y = ev->cur.canvas.y;
             if (abs((sd->history[sd->movements].x - sd->history[0].x)) > 40)
               sd->swipe = EINA_TRUE;
             else
               sd->movements++;
          }
     }

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

static void
_mouse_down_cb(void *data,
               Evas *evas EINA_UNUSED,
               Evas_Object *o EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Elm_List_Item *it = data;
   Evas_Object *obj;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   if (sd->on_hold) return;
   sd->mouse_down = EINA_TRUE;
   sd->was_selected = it->selected;

   evas_object_ref(obj);
   _elm_list_walk(sd);

   _item_highlight(it);
   sd->longpressed = EINA_FALSE;
   ecore_timer_del(it->long_timer);
   it->long_timer = ecore_timer_add
       (_elm_config->longpress_timeout, _long_press_cb, it);
   ecore_timer_del(it->swipe_timer);
   it->swipe_timer = ecore_timer_add(ELM_LIST_SWIPE_TIME, _swipe_cancel, it);

   /* Always call the callbacks last - the user may delete our context! */
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     {
        evas_object_smart_callback_call(WIDGET(it), SIG_CLICKED_DOUBLE, it);
        evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
     }
   sd->swipe = EINA_FALSE;
   sd->movements = 0;

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

static void
_mouse_up_cb(void *data,
             Evas *evas EINA_UNUSED,
             Evas_Object *o EINA_UNUSED,
             void *event_info)
{
   Evas_Object *obj;
   Elm_List_Item *it = data;
   Evas_Event_Mouse_Up *ev = event_info;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   sd->mouse_down = EINA_FALSE;
   sd->longpressed = EINA_FALSE;
   ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
   ELM_SAFE_FREE(it->swipe_timer, ecore_timer_del);
   if (sd->on_hold)
     {
        if (sd->swipe) _swipe_do(data);
        sd->on_hold = EINA_FALSE;
        return;
     }
   if (sd->longpressed)
     {
        if (!sd->was_selected)
          {
             _item_unhighlight(it);
             _item_unselect(it);
          }
        sd->was_selected = 0;
        return;
     }

   if (it->base.disabled)
     return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   evas_object_ref(obj);
   _elm_list_walk(sd);

   if (sd->focused_item != (Elm_Object_Item *)it)
        elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);

   if (sd->multi &&
       ((sd->multi_select_mode != ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL) ||
        (evas_key_modifier_is_set(ev->modifiers, "Control"))))
     {
        if (!it->selected)
          {
             _item_highlight(it);
             _item_select(it);
          }
        else
          {
             _item_unhighlight(it);
             _item_unselect(it);
          }
     }
   else
     {
        if (!it->selected)
          {
             while (sd->selected)
               {
                  Elm_List_Item *it2 = sd->selected->data;
                  sd->selected = eina_list_remove_list
                    (sd->selected, sd->selected);
                  _item_unhighlight(it2);
                  _item_unselect(it2);
               }
             _item_highlight(it);
             _item_select(it);
          }
        else
          {
             const Eina_List *l, *l_next;
             Elm_List_Item *it2;

             EINA_LIST_FOREACH_SAFE(sd->selected, l, l_next, it2)
               if (it2 != it)
                 {
                    _item_unhighlight(it2);
                    _item_unselect(it2);
                 }
             _item_highlight(it);
             _item_select(it);
          }
     }

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

static void
_elm_list_looping_left_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Evas_Object *list = data;
   Elm_List_Item *it = (Elm_List_Item *)elm_list_last_item_get(list);
   if (!_elm_config->item_select_on_focus_disable)
     elm_list_item_selected_set((Elm_Object_Item *)it, EINA_TRUE);
   else
     elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);
   elm_layout_signal_emit(list, "elm,action,looping,left,end", "elm");
}

static void
_elm_list_looping_right_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Evas_Object *list = data;
   Elm_List_Item *it = (Elm_List_Item *)elm_list_first_item_get(list);
   if (!_elm_config->item_select_on_focus_disable)
     elm_list_item_selected_set((Elm_Object_Item *)it, EINA_TRUE);
   else
     elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);
   elm_layout_signal_emit(list, "elm,action,looping,right,end", "elm");
}

static void
_elm_list_looping_up_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Evas_Object *list = data;
   Elm_List_Item *it = (Elm_List_Item *)elm_list_last_item_get(list);
   if (!_elm_config->item_select_on_focus_disable)
     elm_list_item_selected_set((Elm_Object_Item *)it, EINA_TRUE);
   else
     elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);
   elm_layout_signal_emit(list, "elm,action,looping,up,end", "elm");
}

static void
_elm_list_looping_down_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          const char *emission EINA_UNUSED,
                          const char *source EINA_UNUSED)
{
   Evas_Object *list = data;
   Elm_List_Item *it = (Elm_List_Item *)elm_list_first_item_get(list);
   if (!_elm_config->item_select_on_focus_disable)
     elm_list_item_selected_set((Elm_Object_Item *)it, EINA_TRUE);
   else
     elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);
   elm_layout_signal_emit(list, "elm,action,looping,down,end", "elm");
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   Elm_List_Item *item = (Elm_List_Item *)it;

   _item_unhighlight(item);
   _item_unselect(item);
   if (item->base.disabled)
     edje_object_signal_emit(VIEW(item), "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(VIEW(item), "elm,state,enabled", "elm");
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_List_Item *item = (Elm_List_Item *)it;
   Evas_Object **icon_p = NULL;
   Eina_Bool dummy = EINA_FALSE;

   if ((!part) || (!strcmp(part, "start")))
     {
        icon_p = &(item->icon);
        dummy = item->dummy_icon;
        if (!content) item->dummy_icon = EINA_TRUE;
        else item->dummy_icon = EINA_FALSE;
     }
   else if (!strcmp(part, "end"))
     {
        icon_p = &(item->end);
        dummy = item->dummy_end;
        if (!content) item->dummy_end = EINA_TRUE;
        else item->dummy_end = EINA_FALSE;
     }
   else
     return;

   if (content == *icon_p) return;
   if ((dummy) && (!content)) return;
   if (dummy) evas_object_del(*icon_p);
   if (!content)
     {
        content =
          evas_object_rectangle_add(evas_object_evas_get(WIDGET(item)));
        evas_object_color_set(content, 0, 0, 0, 0);
     }
   evas_object_del(*icon_p);
   *icon_p = content;

   if (VIEW(item))
     {
        if ((!part) || !strcmp(part, "start"))
          edje_object_part_swallow(VIEW(item), "elm.swallow.icon", content);
        else
          edje_object_part_swallow(VIEW(item), "elm.swallow.end", content);
     }
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   Elm_List_Item *item = (Elm_List_Item *)it;

   if ((!part) || (!strcmp(part, "start")))
     {
        if (item->dummy_icon) return NULL;
        return item->icon;
     }
   else if (!strcmp(part, "end"))
     {
        if (item->dummy_end) return NULL;
        return item->end;
     }

   return NULL;
}

static Evas_Object *
_item_content_unset_hook(const Elm_Object_Item *it,
                         const char *part)
{
   Elm_List_Item *item = (Elm_List_Item *)it;

   if ((!part) || (!strcmp(part, "start")))
     {
        Evas_Object *obj = item->icon;
        _item_content_set_hook((Elm_Object_Item *)it, part, NULL);
        return obj;
     }
   else if (!strcmp(part, "end"))
     {
        Evas_Object *obj = item->end;
        _item_content_set_hook((Elm_Object_Item *)it, part, NULL);
        return obj;
     }

   return NULL;
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *text)
{
   Elm_List_Item *list_it = (Elm_List_Item *)it;

   if (part && strcmp(part, "default"))
     {
        elm_widget_item_part_text_custom_set(list_it, part, text);
        edje_object_part_text_escaped_set(VIEW(list_it), part, text);
        return;
     }
   if (!eina_stringshare_replace(&list_it->label, text)) return;
   if (VIEW(list_it))
     edje_object_part_text_escaped_set(VIEW(list_it), "elm.text", text);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "default"))
     return elm_widget_item_part_text_custom_get(it, part);
   return ((Elm_List_Item *)it)->label;
}

/* FIXME: this _item_del_pre_hook is never been called at all!
 To fix this,
 1. it->walking concept should be adopted.
 2. elm_widget_item_del() should be called instead of the combination of
 _elm_list_item_free() + elm_widget_item_free()
 */
static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Evas_Object *obj = WIDGET(it);
   Elm_List_Item *item = (Elm_List_Item *)it;

   ELM_LIST_DATA_GET(obj, sd);

   if (item->selected)
     {
        _item_unhighlight(item);
        _item_unselect(item);
     }

   if (sd->walking > 0)
     {
        if (item->deleted) return EINA_FALSE;
        item->deleted = EINA_TRUE;
        sd->to_delete = eina_list_append(sd->to_delete, item);
        return EINA_FALSE;
     }

   sd->items = eina_list_remove_list(sd->items, item->node);

   evas_object_ref(obj);
   _elm_list_walk(sd);

   _elm_list_item_free(item);

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);

   return EINA_TRUE;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   edje_object_signal_emit(VIEW(it), emission, source);
}

static void
_item_focus_set_hook(Elm_Object_Item *it, Eina_Bool focused)
{
   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   Evas_Object *obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   if (focused)
     {
        if (!elm_object_focus_get(obj))
          elm_object_focus_set(obj, EINA_TRUE);
        if (it != sd->focused_item)
          {
             if (sd->focused_item)
               _elm_list_item_unfocused((Elm_List_Item *)sd->focused_item);
             _elm_list_item_focused((Elm_List_Item *)it);
          }
     }
   else
     {
        if (it)
          _elm_list_item_unfocused((Elm_List_Item *)it);
     }
   _elm_widget_focus_highlight_start(obj);
}

static Eina_Bool
_item_focus_get_hook(Elm_Object_Item *it)
{
   ELM_LIST_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Evas_Object *obj = WIDGET(it);
   ELM_LIST_CHECK(obj) EINA_FALSE;
   ELM_LIST_DATA_GET(obj, sd);

   if (it == sd->focused_item)
     return EINA_TRUE;
   return EINA_FALSE;
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_List_Item *it = (Elm_List_Item *)data;
   const char *txt = NULL;
   if (!it) return NULL;

   if (!txt) txt = it->label;
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_List_Item *it = (Elm_List_Item *)data;
   if (!it) return NULL;

   if (it->base.disabled)
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_on_highlight_cb(void *data)
{
   Elm_Object_Item *it = (Elm_Object_Item *)data;
   if (!it) return;

   elm_list_item_bring_in(it);
}

static void
_access_activate_cb(void *data EINA_UNUSED,
                    Evas_Object *part_obj EINA_UNUSED,
                    Elm_Object_Item *item)
{
   Elm_List_Item *it;
   Evas_Object *obj;

   it = (Elm_List_Item *)item;
   ELM_LIST_ITEM_CHECK_OR_RETURN(it);

   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   evas_object_ref(obj);
   _elm_list_walk(sd);

   if (sd->multi)
     {
        if (!it->selected)
          {
             _item_highlight(it);
             _item_select(it);
          }
        else
          {
             _item_unhighlight(it);
             _item_unselect(it);
          }
     }
   else
     {
        if (!it->selected)
          {
             while (sd->selected)
               {
                  _item_unhighlight(sd->selected->data);
                  _item_unselect(sd->selected->data);
               }
             _item_highlight(it);
             _item_select(it);
          }
        else
          {
             const Eina_List *l, *l_next;
             Elm_List_Item *it2;

             EINA_LIST_FOREACH_SAFE(sd->selected, l, l_next, it2)
               if (it2 != it)
                 {
                    _item_unhighlight(it2);
                    _item_unselect(it2);
                 }
             _item_highlight(it);
             _item_select(it);
          }
     }

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

static void
_access_widget_item_register(Elm_List_Item *it, Eina_Bool is_access)
{
   Elm_Access_Info *ai;

   if (!is_access) _elm_access_widget_item_unregister((Elm_Widget_Item *)it);
   else
     {
        _elm_access_widget_item_register((Elm_Widget_Item *)it);

        ai = _elm_access_info_get(it->base.access_obj);

        _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
        _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);
        _elm_access_on_highlight_hook_set(ai, _access_on_highlight_cb, it);
        _elm_access_activate_callback_set(ai, _access_activate_cb, it);
     }
}

static Elm_List_Item *
_item_new(Evas_Object *obj,
          const char *label,
          Evas_Object *icon,
          Evas_Object *end,
          Evas_Smart_Cb func,
          const void *data)
{
   Elm_List_Item *it;

   it = elm_widget_item_new(obj, Elm_List_Item);
   it->label = eina_stringshare_add(label);
   it->icon = icon;
   it->end = end;
   it->func = func;
   it->base.data = data;

   VIEW(it) = edje_object_add(evas_object_evas_get(obj));

   /* access */
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_widget_item_register(it, EINA_TRUE);

   edje_object_mirrored_set(VIEW(it), elm_widget_mirrored_get(obj));
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_DOWN, _mouse_down_cb, it);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_UP, _mouse_up_cb, it);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_MOVE, _mouse_move_cb, it);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_IN, _mouse_in_cb, it);
   evas_object_size_hint_weight_set
     (VIEW(it), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(VIEW(it), EVAS_HINT_FILL, EVAS_HINT_FILL);
   edje_object_mirrored_set(VIEW(it), elm_widget_mirrored_get(obj));

   if (it->icon)
     {
        elm_widget_sub_object_add(obj, it->icon);
        evas_object_event_callback_add
          (it->icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _size_hints_changed_cb,
          obj);
     }
   if (it->end)
     {
        elm_widget_sub_object_add(obj, it->end);
        evas_object_event_callback_add
          (it->end, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _size_hints_changed_cb,
          obj);
     }

   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);
   elm_widget_item_focus_set_hook_set(it, _item_focus_set_hook);
   elm_widget_item_focus_get_hook_set(it, _item_focus_get_hook);

   return it;
}

static void
_resize_cb(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static Eina_Bool _elm_list_smart_focus_next_enable = EINA_FALSE;

EOLIAN static Eina_Bool
_elm_list_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_List_Data *sd EINA_UNUSED)
{
   return _elm_list_smart_focus_next_enable;
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_List_Data *sd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_focus_next(Eo *obj, Elm_List_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
{
   Eina_List *items = NULL;
   Eina_List *elist = NULL;
   Elm_List_Item *it;

   if (_elm_config->access_mode != ELM_ACCESS_MODE_ON) return EINA_FALSE;

   EINA_LIST_FOREACH(sd->items, elist, it)
     {
        items = eina_list_append(items, it->base.access_obj);
        if (it->icon) items = eina_list_append(items, it->icon);
        if (it->end) items = eina_list_append(items, it->end);
     }

   return elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

EOLIAN static void
_elm_list_evas_smart_add(Eo *obj, Elm_List_Data *priv)
{
   Evas_Coord minw, minh;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->mode = ELM_LIST_SCROLL;

   if (!elm_layout_theme_set(obj, "list", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_data_set(priv->hit_rect, "_elm_leaveme", obj);
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   eo_do(obj,
         elm_interface_scrollable_edge_left_cb_set(_edge_left_cb),
         elm_interface_scrollable_edge_right_cb_set(_edge_right_cb),
         elm_interface_scrollable_edge_top_cb_set(_edge_top_cb),
         elm_interface_scrollable_edge_bottom_cb_set(_edge_bottom_cb),
         elm_interface_scrollable_content_min_limit_cb_set(_elm_list_content_min_limit_cb),
         elm_interface_scrollable_objects_set(wd->resize_obj, priv->hit_rect));

   /* the scrollable interface may set this */
   evas_object_event_callback_add
     (wd->resize_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _size_hints_changed_cb, obj);

   edje_object_size_min_calc
     (wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize_cb, obj);

   eo_do(obj, elm_interface_scrollable_bounce_allow_set(EINA_FALSE, _elm_config->thumbscroll_bounce_enable));

   priv->box = elm_box_add(obj);
   evas_object_size_hint_weight_set(priv->box, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(priv->box, EVAS_HINT_FILL, 0.0);

   /* FIXME: change this ugly code path later */
   elm_widget_on_show_region_hook_set(priv->box, _show_region_hook, obj);
   elm_widget_sub_object_add(obj, priv->box);

   eo_do(obj, elm_interface_scrollable_content_set(priv->box));
   evas_object_event_callback_add
     (priv->box, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _size_hints_changed_cb, obj);

   edje_object_signal_callback_add(wd->resize_obj, "elm,looping,left,done", "elm", _elm_list_looping_left_cb, obj);
   edje_object_signal_callback_add(wd->resize_obj, "elm,looping,right,done", "elm", _elm_list_looping_right_cb, obj);
   edje_object_signal_callback_add(wd->resize_obj, "elm,looping,up,done", "elm", _elm_list_looping_up_cb, obj);
   edje_object_signal_callback_add(wd->resize_obj, "elm,looping,down,done", "elm", _elm_list_looping_down_cb, obj);
}

EOLIAN static void
_elm_list_evas_smart_del(Eo *obj, Elm_List_Data *sd)
{
   const Eina_List *l;
   Elm_List_Item *it;

   if (sd->walking)
     ERR("ERROR: list deleted while walking.\n");

   sd->delete_me = EINA_TRUE;
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->icon)
          evas_object_event_callback_del
            (it->icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
            _size_hints_changed_cb);
        if (it->end)
          evas_object_event_callback_del
            (it->end, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
            _size_hints_changed_cb);
     }

   evas_object_event_callback_del
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _size_hints_changed_cb);
   evas_object_event_callback_del
     (sd->box, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _size_hints_changed_cb);

   _elm_list_walk(sd);

   EINA_LIST_FREE(sd->items, it)
     {
        /* issuing free because of "locking" item del pre hook */
        _elm_list_item_free(it);
        elm_widget_item_free(it);
     }

   _elm_list_unwalk(obj, sd);

   if (sd->to_delete)
     ERR("ERROR: leaking nodes!\n");

   sd->selected = eina_list_free(sd->selected);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_list_evas_smart_move(Eo *obj, Elm_List_Data *sd, Evas_Coord x, Evas_Coord y)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   evas_object_move(sd->hit_rect, x, y);
}

EOLIAN static void
_elm_list_evas_smart_resize(Eo *obj, Elm_List_Data *sd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   evas_object_resize(sd->hit_rect, w, h);
}

EOLIAN static void
_elm_list_evas_smart_member_add(Eo *obj, Elm_List_Data *sd, Evas_Object *member)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_add(member));

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EOLIAN static void
_elm_list_elm_widget_access(Eo *obj EINA_UNUSED, Elm_List_Data *sd, Eina_Bool acs)
{
   Eina_List *elist = NULL;
   Elm_List_Item *it;
   _elm_list_smart_focus_next_enable = acs;

   EINA_LIST_FOREACH(sd->items, elist, it)
     _access_widget_item_register(it, _elm_list_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_list_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_list_eo_base_constructor(Eo *obj, Elm_List_Data *sd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elm_list_go(Eo *obj, Elm_List_Data *sd EINA_UNUSED)
{
   _items_fix(obj);
}

EOLIAN static void
_elm_list_multi_select_set(Eo *obj EINA_UNUSED, Elm_List_Data *sd, Eina_Bool multi)
{
   sd->multi = multi;
}

EOLIAN static Eina_Bool
_elm_list_multi_select_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->multi;
}

EOLIAN static void
_elm_list_multi_select_mode_set(Eo *obj EINA_UNUSED, Elm_List_Data *sd, Elm_Object_Multi_Select_Mode mode)
{
   if (mode >= ELM_OBJECT_MULTI_SELECT_MODE_MAX)
     return;

   if (sd->multi_select_mode != mode)
     sd->multi_select_mode = mode;
}

EOLIAN static Elm_Object_Multi_Select_Mode
_elm_list_multi_select_mode_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->multi_select_mode;
}

EOLIAN static void
_elm_list_mode_set(Eo *obj, Elm_List_Data *sd, Elm_List_Mode mode)
{
   if (sd->mode == mode)
     return;

   sd->mode = mode;

   _elm_list_mode_set_internal(obj);
}

EOLIAN static Elm_List_Mode
_elm_list_mode_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->mode;
}

EOLIAN static void
_elm_list_horizontal_set(Eo *obj, Elm_List_Data *sd, Eina_Bool horizontal)
{
   Eina_Bool bounce = _elm_config->thumbscroll_bounce_enable;

   horizontal = !!horizontal;

   if (sd->h_mode == horizontal)
     return;

   sd->h_mode = horizontal;
   elm_box_horizontal_set(sd->box, horizontal);

   if (horizontal)
     {
        evas_object_size_hint_weight_set(sd->box, 0.0, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(sd->box, 0.0, EVAS_HINT_FILL);
        eo_do(obj, elm_interface_scrollable_bounce_allow_set(bounce, EINA_FALSE));
     }
   else
     {
        evas_object_size_hint_weight_set(sd->box, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(sd->box, EVAS_HINT_FILL, 0.0);
        eo_do(obj, elm_interface_scrollable_bounce_allow_set(EINA_FALSE, bounce));
     }

   _elm_list_mode_set_internal(obj);
}

EOLIAN static Eina_Bool
_elm_list_horizontal_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->h_mode;
}

EOLIAN static void
_elm_list_select_mode_set(Eo *obj EINA_UNUSED, Elm_List_Data *sd, Elm_Object_Select_Mode mode)
{
   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;

   if (sd->select_mode != mode)
     sd->select_mode = mode;
}

EOLIAN static Elm_Object_Select_Mode
_elm_list_select_mode_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->select_mode;
}

EAPI void
elm_list_bounce_set(Evas_Object *obj,
                    Eina_Bool h_bounce,
                    Eina_Bool v_bounce)
{
   ELM_LIST_CHECK(obj);

   eo_do(obj, elm_interface_scrollable_bounce_allow_set(h_bounce, v_bounce));
}

EAPI void
elm_list_bounce_get(const Evas_Object *obj,
                    Eina_Bool *h_bounce,
                    Eina_Bool *v_bounce)
{
   ELM_LIST_CHECK(obj);

   eo_do((Eo *) obj, elm_interface_scrollable_bounce_allow_get(h_bounce, v_bounce));
}

EAPI void
elm_list_scroller_policy_set(Evas_Object *obj,
                             Elm_Scroller_Policy policy_h,
                             Elm_Scroller_Policy policy_v)
{
   ELM_LIST_CHECK(obj);
   eo_do(obj, elm_interface_scrollable_policy_set(policy_h, policy_v));
}

EOLIAN static void
_elm_list_elm_interface_scrollable_policy_set(Eo *obj, Elm_List_Data *sd EINA_UNUSED, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   eo_do_super(obj, MY_CLASS, elm_interface_scrollable_policy_set(policy_h, policy_v));
}

EAPI void
elm_list_scroller_policy_get(const Evas_Object *obj,
                             Elm_Scroller_Policy *policy_h,
                             Elm_Scroller_Policy *policy_v)
{
   ELM_LIST_CHECK(obj);
   eo_do((Eo *) obj, elm_interface_scrollable_policy_get(policy_h, policy_v));
}

EOLIAN static void
_elm_list_clear(Eo *obj, Elm_List_Data *sd)
{
   Elm_List_Item *it;

   if (!sd->items) return;

   sd->selected = eina_list_free(sd->selected);

   if (sd->walking > 0)
     {
        Eina_List *n;

        EINA_LIST_FOREACH(sd->items, n, it)
          {
             if (it->deleted) continue;
             it->deleted = EINA_TRUE;
             sd->to_delete = eina_list_append(sd->to_delete, it);
          }
        return;
     }

   evas_object_ref(obj);

   _elm_list_walk(sd);

   EINA_LIST_FREE(sd->items, it)
     {
        /* issuing free because of "locking" item del pre hook */
        _elm_list_item_free(it);
        elm_widget_item_free(it);
     }

   _elm_list_unwalk(obj, sd);

   _items_fix(obj);
   elm_layout_sizing_eval(obj);

   evas_object_unref(obj);
}

EOLIAN static const Eina_List*
_elm_list_items_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->items;
}

EOLIAN static Elm_Object_Item*
_elm_list_selected_item_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   if (sd->selected) return (Elm_Object_Item *)sd->selected->data;
   else return NULL;
}

EOLIAN static const Eina_List*
_elm_list_selected_items_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->selected;
}

EOLIAN static Elm_Object_Item*
_elm_list_item_append(Eo *obj, Elm_List_Data *sd, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   Elm_List_Item *it;

   it = _item_new(obj, label, icon, end, func, data);

   sd->items = eina_list_append(sd->items, it);
   it->node = eina_list_last(sd->items);
   elm_box_pack_end(sd->box, VIEW(it));

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_list_item_prepend(Eo *obj, Elm_List_Data *sd, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   Elm_List_Item *it;

   it = _item_new(obj, label, icon, end, func, data);

   sd->items = eina_list_prepend(sd->items, it);
   it->node = sd->items;
   elm_box_pack_start(sd->box, VIEW(it));

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_list_item_insert_before(Eo *obj, Elm_List_Data *sd, Elm_Object_Item *before, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   Elm_List_Item *it, *before_it;

   ELM_LIST_ITEM_CHECK_OR_RETURN(before, NULL);

   before_it = (Elm_List_Item *)before;
   if (!before_it->node) return NULL;

   it = _item_new(obj, label, icon, end, func, data);
   sd->items = eina_list_prepend_relative_list(sd->items, it, before_it->node);
   it->node = before_it->node->prev;
   elm_box_pack_before(sd->box, VIEW(it), VIEW(before_it));

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_list_item_insert_after(Eo *obj, Elm_List_Data *sd, Elm_Object_Item *after, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data)
{
   Elm_List_Item *it, *after_it;

   ELM_LIST_ITEM_CHECK_OR_RETURN(after, NULL);

   after_it = (Elm_List_Item *)after;
   if (!after_it->node) return NULL;

   it = _item_new(obj, label, icon, end, func, data);
   sd->items = eina_list_append_relative_list(sd->items, it, after_it->node);
   it->node = after_it->node->next;
   elm_box_pack_after(sd->box, VIEW(it), VIEW(after_it));

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_list_item_sorted_insert(Eo *obj, Elm_List_Data *sd, const char *label, Evas_Object *icon, Evas_Object *end, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func)
{
   Eina_List *l;
   Elm_List_Item *it;

   it = _item_new(obj, label, icon, end, func, data);

   sd->items = eina_list_sorted_insert(sd->items, cmp_func, it);
   l = eina_list_data_find_list(sd->items, it);
   l = eina_list_next(l);
   if (!l)
     {
        it->node = eina_list_last(sd->items);
        elm_box_pack_end(sd->box, VIEW(it));
     }
   else
     {
        Elm_List_Item *before = eina_list_data_get(l);

        it->node = before->node->prev;
        elm_box_pack_before(sd->box, VIEW(it), VIEW(before));
     }

   return (Elm_Object_Item *)it;
}

EAPI void
elm_list_item_separator_set(Elm_Object_Item *it,
                            Eina_Bool setting)
{
   ELM_LIST_ITEM_CHECK_OR_RETURN(it);

   ((Elm_List_Item *)it)->is_separator = !!setting;
}

EAPI Eina_Bool
elm_list_item_separator_get(const Elm_Object_Item *it)
{
   ELM_LIST_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_List_Item *)it)->is_separator;
}

EAPI void
elm_list_item_selected_set(Elm_Object_Item *it,
                           Eina_Bool selected)
{
   Elm_List_Item *item = (Elm_List_Item *)it;
   Evas_Object *obj;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   obj = WIDGET(it);
   ELM_LIST_DATA_GET(obj, sd);

   selected = !!selected;
   if (item->selected == selected) return;

   evas_object_ref(obj);
   _elm_list_walk(sd);

   if (selected)
     {
        if (!sd->multi)
          {
             while (sd->selected)
               {
                  _item_unhighlight(sd->selected->data);
                  _item_unselect(sd->selected->data);
               }
          }
        _item_highlight(item);
        _item_select(item);
        elm_object_item_focus_set((Elm_Object_Item *)item, EINA_TRUE);
     }
   else
     {
        _item_unhighlight(item);
        _item_unselect(item);
     }

   _elm_list_unwalk(obj, sd);
   evas_object_unref(obj);
}

EAPI Eina_Bool
elm_list_item_selected_get(const Elm_Object_Item *it)
{
   ELM_LIST_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_List_Item *)it)->selected;
}

EAPI void
elm_list_item_show(Elm_Object_Item *it)
{
   Evas_Coord bx, by, bw, bh;
   Evas_Coord x, y, w, h;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   ELM_LIST_DATA_GET(WIDGET(it), sd);

   evas_smart_objects_calculate(evas_object_evas_get(sd->box));
   evas_object_geometry_get(sd->box, &bx, &by, &bw, &bh);
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
   x -= bx;
   y -= by;

   eo_do(WIDGET(it), elm_interface_scrollable_content_region_show(x, y, w, h));
}

EAPI void
elm_list_item_bring_in(Elm_Object_Item *it)
{
   Evas_Coord bx, by, bw, bh;
   Evas_Coord x, y, w, h;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it);
   ELM_LIST_DATA_GET(WIDGET(it), sd);

   evas_smart_objects_calculate(evas_object_evas_get(sd->box));
   evas_object_geometry_get(sd->box, &bx, &by, &bw, &bh);
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
   x -= bx;
   y -= by;

   eo_do(WIDGET(it), elm_interface_scrollable_region_bring_in(x, y, w, h));
}

EAPI Evas_Object *
elm_list_item_object_get(const Elm_Object_Item *it)
{
   ELM_LIST_ITEM_CHECK_OR_RETURN(it, NULL);

   return VIEW(it);
}

EAPI Elm_Object_Item *
elm_list_item_prev(const Elm_Object_Item *it)
{
   Elm_List_Item *item = (Elm_List_Item *)it;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it, NULL);

   if (item->node->prev) return item->node->prev->data;
   else return NULL;
}

EAPI Elm_Object_Item *
elm_list_item_next(const Elm_Object_Item *it)
{
   Elm_List_Item *item = (Elm_List_Item *)it;

   ELM_LIST_ITEM_CHECK_OR_RETURN(it, NULL);

   if (item->node->next) return item->node->next->data;
   else return NULL;
}

EOLIAN static Elm_Object_Item*
_elm_list_first_item_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   if (!sd->items) return NULL;
   else return eina_list_data_get(sd->items);
}

EOLIAN static Elm_Object_Item*
_elm_list_last_item_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   if (!sd->items) return NULL;
   else return eina_list_data_get(eina_list_last(sd->items));
}

EOLIAN static Elm_Object_Item*
_elm_list_at_xy_item_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd, Evas_Coord x, Evas_Coord y, int *posret)
{
   Eina_List *l;
   Elm_List_Item *it;
   Evas_Coord lasty;
   evas_object_geometry_get(sd->hit_rect, &lasty, NULL, NULL, NULL);

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        Evas_Coord itx, ity;
        Evas_Object *vit = VIEW(it);
        Evas_Coord vx, vy, vw, vh;
        evas_object_geometry_get(vit, &vx, &vy, &vw, &vh);

        itx = vx;
        ity = vy;
        if (ELM_RECTS_INTERSECT
              (itx, ity, vw, vh, x, y, 1, 1))
          {
             if (posret)
               {
                  if (y <= (ity + (vh / 4))) *posret = -1;
                  else if (y >= (ity + vh - (vh / 4)))
                    *posret = 1;
                  else *posret = 0;
               }

             return (Elm_Object_Item *) it;
          }

        lasty = ity + vh;
     }

   if (posret)
     {
        if (y > lasty) *posret = 1;
        else *posret = -1;
     }

   return NULL;
}

EOLIAN static void
_elm_list_focus_on_selection_set(Eo *obj EINA_UNUSED, Elm_List_Data *sd, Eina_Bool enabled)
{
   sd->focus_on_selection_enabled = !!enabled;
}

EOLIAN static Eina_Bool
_elm_list_focus_on_selection_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->focus_on_selection_enabled;
}

static void
_elm_list_item_coordinates_adjust(Elm_List_Item *it,
                                  Evas_Coord *x,
                                  Evas_Coord *y,
                                  Evas_Coord *w,
                                  Evas_Coord *h)
{
   ELM_LIST_DATA_GET(WIDGET(it), sd);

   Evas_Coord ix, iy, iw, ih, vx, vy, vw, vh;

   evas_object_geometry_get(sd->hit_rect, &vx, &vy, &vw, &vh);
   evas_object_geometry_get(VIEW(it), &ix, &iy, &iw, &ih);
   *x = ix;
   *y = iy;
   *w = iw;
   *h = ih;
   if (!sd->h_mode)
     {
        //TODO: Enhance it later. declare a macro in elm_macros.h
        if ((ix < vx) || (ix + iw) > (vx + vw) || (iy + ih) > (vy + vh))
          *y = iy - ih;
        else if (iy < vy)
          *y = iy + ih;
     }
   else
     {
        //TODO: Enhance it later. declare a macro in elm_macros.h
        if ((iy < vy) || (ix + iw) > (vx + vw) || (iy + ih) > (vy + vh))
          *x = ix - iw;
        else if (ix < vx)
          *x = ix + iw;
     }
}

EOLIAN static void
_elm_list_elm_widget_focus_highlight_geometry_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (sd->focused_item)
     {
        _elm_list_item_coordinates_adjust((Elm_List_Item *)sd->focused_item, x, y, w, h);
        elm_widget_focus_highlight_focus_part_geometry_get(VIEW(sd->focused_item), x, y, w, h);
     }
}

EOLIAN static Elm_Object_Item*
_elm_list_elm_widget_focused_item_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->focused_item;
}

EOLIAN static void
_elm_list_elm_widget_item_loop_enabled_set(Eo *obj EINA_UNUSED, Elm_List_Data *sd, Eina_Bool enable)
{
   if (sd->item_loop_enable == enable) return;
   sd->item_loop_enable = !!enable;
}

EOLIAN static Eina_Bool
_elm_list_elm_widget_item_loop_enabled_get(Eo *obj EINA_UNUSED, Elm_List_Data *sd)
{
   return sd->item_loop_enable;
}

static void
_elm_list_class_constructor(Eo_Class *klass)
{
      if (_elm_config->access_mode)
        _elm_list_smart_focus_next_enable = EINA_TRUE;

      evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
#include "elm_list.eo.c"
