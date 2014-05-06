#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "elm_widget_gengrid.h"
#include "elm_interface_scrollable.h"

#define MY_PAN_CLASS ELM_OBJ_GENGRID_PAN_CLASS

#define MY_PAN_CLASS_NAME "Elm_Gengrid_Pan"
#define MY_PAN_CLASS_NAME_LEGACY "elm_gengrid_pan"

#define MY_CLASS ELM_OBJ_GENGRID_CLASS

#define MY_CLASS_NAME "Elm_Gengrid"
#define MY_CLASS_NAME_LEGACY "elm_gengrid"

// internally allocated
#define CLASS_ALLOCATED     0x3a70f00f

/* --
 * TODO:
 * Handle non-homogeneous objects too.
 */

#define PRELOAD             1
#define REORDER_EFFECT_TIME 0.5

EAPI const char ELM_GENGRID_SMART_NAME[] = "elm_gengrid";
EAPI const char ELM_GENGRID_PAN_SMART_NAME[] = "elm_gengrid_pan";

#define GG_IT(_it) (_it->item)

#define ELM_PRIV_GENGRID_SIGNALS(cmd) \
   cmd(SIG_ACTIVATED, "activated", "") \
   cmd(SIG_CLICKED_DOUBLE, "clicked,double", "") \
   cmd(SIG_LONGPRESSED, "longpressed", "") \
   cmd(SIG_SELECTED, "selected", "") \
   cmd(SIG_UNSELECTED, "unselected", "") \
   cmd(SIG_REALIZED, "realized", "") \
   cmd(SIG_UNREALIZED, "unrealized", "") \
   cmd(SIG_CHANGED, "changed", "") \
   cmd(SIG_DRAG_START_UP, "drag,start,up", "") \
   cmd(SIG_DRAG_START_DOWN, "drag,start,down", "") \
   cmd(SIG_DRAG_START_LEFT, "drag,start,left", "") \
   cmd(SIG_DRAG_START_RIGHT, "drag,start,right", "") \
   cmd(SIG_DRAG_STOP, "drag,stop", "") \
   cmd(SIG_DRAG, "drag", "") \
   cmd(SIG_SCROLL, "scroll", "") \
   cmd(SIG_SCROLL_ANIM_START, "scroll,anim,start", "") \
   cmd(SIG_SCROLL_ANIM_STOP, "scroll,anim,stop", "") \
   cmd(SIG_SCROLL_DRAG_START, "scroll,drag,start", "") \
   cmd(SIG_SCROLL_DRAG_STOP, "scroll,drag,stop", "") \
   cmd(SIG_SCROLL_PAGE_CHANGE, "scroll,page,changed", "") \
   cmd(SIG_EDGE_TOP, "edge,top", "") \
   cmd(SIG_EDGE_BOTTOM, "edge,bottom", "") \
   cmd(SIG_EDGE_LEFT, "edge,left", "") \
   cmd(SIG_EDGE_RIGHT, "edge,right", "") \
   cmd(SIG_MOVED, "moved", "") \
   cmd(SIG_INDEX_UPDATE, "index,update", "") \
   cmd(SIG_HIGHLIGHTED, "highlighted", "") \
   cmd(SIG_UNHIGHLIGHTED, "unhighlighted", "") \
   cmd(SIG_ITEM_FOCUSED, "item,focused", "") \
   cmd(SIG_ITEM_UNFOCUSED, "item,unfocused", "") \
   cmd(SIG_PRESSED, "pressed", "") \
   cmd(SIG_RELEASED, "released", "") \
   cmd(SIG_ITEM_REORDER_START, "item,reorder,anim,start", "") \
   cmd(SIG_ITEM_REORDER_STOP, "item,reorder,anim,stop", "")

ELM_PRIV_GENGRID_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   ELM_PRIV_GENGRID_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {SIG_ITEM_FOCUSED, ""},
   {SIG_ITEM_UNFOCUSED, ""},

   {NULL, NULL}
};
#undef ELM_PRIV_GENGRID_SIGNALS

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_select(Evas_Object *obj, const char *params);
static Eina_Bool _key_action_escape(Evas_Object *obj, const char *params);
static void _item_position_update(Eina_Inlist *list, int idx);


static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {"select", _key_action_select},
   {"escape", _key_action_escape},
   {NULL, NULL}
};

static void
_item_show_region(void *data)
{
   Elm_Gengrid_Data *sd = data;
   Evas_Coord cvw, cvh, it_xpos = 0, it_ypos = 0, col = 0, row = 0, minx = 0, miny = 0;
   Evas_Coord vw = 0, vh = 0;
   Elm_Gen_Item *it = NULL;
   evas_object_geometry_get(sd->pan_obj, NULL, NULL, &cvw, &cvh);

   if ((cvw != 0) && (cvh != 0))
       {
          int x = 0, y = 0;
          if (sd->show_region)
            it = sd->show_it;
          else if (sd->bring_in)
            it = sd->bring_in_it;

          if (!it) return;

          eo_do(sd->pan_obj, elm_obj_pan_pos_min_get(&minx, &miny));
          if (sd->horizontal && (sd->item_height > 0))
            {
               row = cvh / sd->item_height;
               if (row <= 0) row = 1;
               x = (it->position - 1) / row;
               if (x == 0) y = it->position - 1;
               else if (x > 0) y = (it->position - 1) % x;
               if (x >= 1)
                 it_xpos = ((x - GG_IT(it)->prev_group) * sd->item_width)
                    + (GG_IT(it)->prev_group * sd->group_item_width)
                    + minx;
               else it_xpos = minx;
               miny = miny + ((cvh - (sd->item_height * row))
                    * GG_IT(it)->wsd->align_y);
               it_ypos = y * sd->item_height + miny;
               it->x = x;
               it->y = y;
            }
          else if (sd->item_width > 0)
            {
               col = cvw / sd->item_width;
               if (col <= 0) col = 1;
               y = (it->position - 1) / col;
               if (y == 0) x = it->position - 1;
               else if (y > 0) x = (it->position - 1) % y;
               it_xpos = x * sd->item_width + minx;
               if (y >= 1)
                 it_ypos = ((y - GG_IT(it)->prev_group) * sd->item_height)
                    + (GG_IT(it)->prev_group * sd->group_item_height)
                    + miny;
               else it_ypos = miny;
               minx = minx + ((cvw - (sd->item_width * col))
                    * GG_IT(it)->wsd->align_x);
               it->x = x;
               it->y = y;
            }

          switch (sd->scroll_to_type)
            {
               case ELM_GENGRID_ITEM_SCROLLTO_TOP:
                  eo_do(WIDGET(it), elm_interface_scrollable_content_viewport_geometry_get
                        (NULL, NULL, &vw, &vh));
                  break;
               case ELM_GENGRID_ITEM_SCROLLTO_MIDDLE:
                  eo_do(WIDGET(it), elm_interface_scrollable_content_viewport_geometry_get
                        (NULL, NULL, &vw, &vh));
                  it_xpos = it_xpos - ((vw - sd->item_width) / 2);
                  it_ypos = it_ypos - ((vh - sd->item_height) / 2);
                  break;
               default:
                  vw = sd->item_width;
                  vh = sd->item_height;
                  break;
            }

          if (sd->show_region)
            {
               eo_do(WIDGET(it), elm_interface_scrollable_content_region_show(
                                                     it_xpos, it_ypos, vw, vh));
               sd->show_region = EINA_FALSE;
            }
          if (sd->bring_in)
            {
               eo_do(WIDGET(it), elm_interface_scrollable_region_bring_in(
                                                 it_xpos, it_ypos, vw, vh));
               sd->bring_in = EINA_FALSE;
            }
       }
}

static void
_calc_job(void *data)
{
   ELM_GENGRID_DATA_GET(data, sd);
   Evas_Coord minw = 0, minh = 0, nmax = 0, cvw, cvh;
   Elm_Gen_Item *it, *group_item = NULL;
   int count_group = 0;
   long count = 0;

   sd->items_lost = 0;

   evas_object_geometry_get(sd->pan_obj, NULL, NULL, &cvw, &cvh);

   if ((cvw != 0) || (cvh != 0))
     {
        if ((sd->horizontal) && (sd->item_height > 0))
          nmax = cvh / sd->item_height;
        else if (sd->item_width > 0)
          nmax = cvw / sd->item_width;

        if (nmax < 1)
          nmax = 1;

        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (GG_IT(it)->prev_group != count_group)
               GG_IT(it)->prev_group = count_group;
             if (it->group)
               {
                  count = count % nmax;
                  if (count)
                    sd->items_lost += nmax - count;
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
        count = sd->item_count + sd->items_lost - count_group;
        if (sd->horizontal)
          {
             minw = (ceil(count / (float)nmax) * sd->item_width) +
               (count_group * sd->group_item_width);
             minh = nmax * sd->item_height;
          }
        else
          {
             minw = nmax * sd->item_width;
             minh = (ceil(count / (float)nmax) * sd->item_height) +
               (count_group * sd->group_item_height);
          }

        if ((minw != sd->minw) || (minh != sd->minh))
          {
             sd->minh = minh;
             sd->minw = minw;
             evas_object_smart_callback_call(sd->pan_obj, "changed", NULL);
          }

        sd->nmax = nmax;
        evas_object_smart_changed(sd->pan_obj);

        if (sd->show_region || sd->bring_in)
          _item_show_region(sd);
     }
   sd->calc_job = NULL;
}

EOLIAN static void
_elm_gengrid_pan_eo_base_destructor(Eo *obj, Elm_Gengrid_Pan_Data *psd)
{
   eo_data_unref(psd->wobj, psd->wsd);
   eo_do_super(obj, MY_PAN_CLASS, eo_destructor());
}

EOLIAN static void
_elm_gengrid_pan_evas_smart_move(Eo *obj EINA_UNUSED, Elm_Gengrid_Pan_Data *psd, Evas_Coord _gen_param2 EINA_UNUSED, Evas_Coord _gen_param3 EINA_UNUSED)
{
   ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job, psd->wobj);
}

EOLIAN static void
_elm_gengrid_pan_evas_smart_resize(Eo *obj, Elm_Gengrid_Pan_Data *psd, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job, psd->wobj);
}

static void
_item_unselect(Elm_Gen_Item *it)
{
   Elm_Gen_Item_Type *item = GG_IT(it);
   Elm_Gengrid_Data *sd = item->wsd;

   if ((it->generation < sd->generation) || (!it->highlighted))
     return;

   edje_object_signal_emit(VIEW(it), "elm,state,unselected", "elm");
   evas_object_smart_callback_call(WIDGET(it), SIG_UNHIGHLIGHTED, it);

   evas_object_stack_below(VIEW(it), sd->stack);

   it->highlighted = EINA_FALSE;
   if (it->selected)
     {
        it->selected = EINA_FALSE;
        sd->selected = eina_list_remove(sd->selected, it);
        evas_object_smart_callback_call(WIDGET(it), SIG_UNSELECTED, it);
     }
}

static void
_item_mouse_in_cb(void *data,
                  Evas *evas EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   if (!elm_object_item_disabled_get(data) &&
       (_elm_config->focus_move_policy == ELM_FOCUS_MOVE_POLICY_IN))
     elm_object_item_focus_set(data, EINA_TRUE);
}

static void
_item_mouse_move_cb(void *data,
                    Evas *evas EINA_UNUSED,
                    Evas_Object *obj,
                    void *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord ox, oy, ow, oh, it_scrl_x, it_scrl_y;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        if (!sd->on_hold)
          {
             sd->on_hold = EINA_TRUE;
             if (!sd->was_selected)
               it->unsel_cb(it);
          }
     }

   if ((it->dragging) && (it->down))
     {
        ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG, it);
        return;
     }

   if ((!it->down) || (sd->longpressed))
     {
        ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
        if ((sd->reorder_mode) && (sd->reorder_it))
          {
             evas_object_geometry_get
               (sd->pan_obj, &ox, &oy, &ow, &oh);

             it_scrl_x = ev->cur.canvas.x - sd->reorder_it->dx;
             it_scrl_y = ev->cur.canvas.y - sd->reorder_it->dy;

             if (it_scrl_x < ox) sd->reorder_item_x = ox;
             else if (it_scrl_x + sd->item_width > ox + ow)
               sd->reorder_item_x = ox + ow - sd->item_width;
             else sd->reorder_item_x = it_scrl_x;

             if (it_scrl_y < oy) sd->reorder_item_y = oy;
             else if (it_scrl_y + sd->item_height > oy + oh)
               sd->reorder_item_y = oy + oh - sd->item_height;
             else sd->reorder_item_y = it_scrl_y;

             ecore_job_del(sd->calc_job);
             sd->calc_job = ecore_job_add(_calc_job, sd->obj);
          }
        return;
     }

   if (it->select_mode != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
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

        if (!elm_widget_mirrored_get(WIDGET(it)))
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
        ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
        if (!GG_IT(it)->wsd->was_selected)
          it->unsel_cb(it);

        if (dy < 0)
          {
             if (ady > adx)
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_DRAG_START_UP, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(WIDGET(it), left_drag, it);
               }
          }
        else
          {
             if (ady > adx)
               evas_object_smart_callback_call
                 (WIDGET(it), SIG_DRAG_START_DOWN, it);
             else
               {
                  if (dx < 0)
                    evas_object_smart_callback_call(WIDGET(it), left_drag, it);
                  else
                    evas_object_smart_callback_call
                      (WIDGET(it), right_drag, it);
               }
          }
     }
}

static Eina_Bool
_long_press_cb(void *data)
{
   Elm_Gen_Item *it = data;
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   it->long_timer = NULL;
   if (elm_widget_item_disabled_get(it) || (it->dragging))
     return ECORE_CALLBACK_CANCEL;
   sd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(WIDGET(it), SIG_LONGPRESSED, it);

   if (sd->reorder_mode)
     {
        sd->reorder_it = it;
        evas_object_raise(VIEW(it));
        eo_do(WIDGET(it), elm_interface_scrollable_hold_set(EINA_TRUE));
        eo_do(WIDGET(it), elm_interface_scrollable_bounce_allow_get(
          &(GG_IT(it)->wsd->old_h_bounce),
          &(GG_IT(it)->wsd->old_v_bounce)));

        eo_do(WIDGET(it), elm_interface_scrollable_bounce_allow_set(EINA_FALSE, EINA_FALSE));
        edje_object_signal_emit(VIEW(it), "elm,state,reorder,enabled", "elm");
     }

   return ECORE_CALLBACK_CANCEL;
}

static void
_item_highlight(Elm_Gen_Item *it)
{
   const char *selectraise = NULL;
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)
       || (!sd->highlight) || (it->highlighted) ||
       (it->generation < sd->generation))
     return;

   edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
   evas_object_smart_callback_call(WIDGET(it), SIG_HIGHLIGHTED, it);

   selectraise = edje_object_data_get(VIEW(it), "selectraise");
   if ((selectraise) && (!strcmp(selectraise, "on")))
     evas_object_stack_above(VIEW(it), sd->stack);

   it->highlighted = EINA_TRUE;
}

static void
_item_mouse_down_cb(void *data,
                    Evas *evas EINA_UNUSED,
                    Evas_Object *obj,
                    void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Elm_Gen_Item *it = data;
   Evas_Coord x, y;
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   if (ev->button != 1) return;

   it->down = 1;
   sd->mouse_down = EINA_TRUE;
   it->dragging = 0;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   it->dx = ev->canvas.x - x;
   it->dy = ev->canvas.y - y;
   sd->longpressed = EINA_FALSE;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   if (sd->on_hold) return;

   sd->was_selected = it->selected;
   it->highlight_cb(it);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     {
        evas_object_smart_callback_call(WIDGET(it), SIG_CLICKED_DOUBLE, it);
        evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
     }

   evas_object_smart_callback_call(WIDGET(it), SIG_PRESSED, it);
   ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
   if (it->realized)
     it->long_timer = ecore_timer_add
         (_elm_config->longpress_timeout, _long_press_cb, it);
   else
     it->long_timer = NULL;
}

static void
_elm_gengrid_item_unrealize(Elm_Gen_Item *it,
                            Eina_Bool calc)
{
   Evas_Object *content;

   if (!it->realized) return;
   if (GG_IT(it)->wsd->reorder_it == it) return;

   evas_event_freeze(evas_object_evas_get(WIDGET(it)));
   if (!calc)
     evas_object_smart_callback_call(WIDGET(it), SIG_UNREALIZED, it);
   ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
   ELM_SAFE_FREE(it->texts, elm_widget_stringlist_free);
   ELM_SAFE_FREE(it->contents, elm_widget_stringlist_free);
   ELM_SAFE_FREE(it->states, elm_widget_stringlist_free);

   EINA_LIST_FREE(it->content_objs, content)
     evas_object_del(content);

   elm_widget_item_track_cancel(it);

   it->unrealize_cb(it);

   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;

   evas_event_thaw(evas_object_evas_get(WIDGET(it)));
   evas_event_thaw_eval(evas_object_evas_get(WIDGET(it)));
}

static void
_item_mouse_up_cb(void *data,
                  Evas *evas EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool dragged = EINA_FALSE;
   Elm_Gengrid_Data *sd;
   Elm_Gen_Item *it = data;

   if (ev->button != 1) return;

   sd = GG_IT(it)->wsd;

   it->down = EINA_FALSE;
   sd->mouse_down = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   evas_object_smart_callback_call(WIDGET(it), SIG_RELEASED, it);
   ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
   if (it->dragging)
     {
        it->dragging = EINA_FALSE;
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG_STOP, it);
        dragged = EINA_TRUE;
     }
   if (sd->on_hold)
     {
        sd->longpressed = EINA_FALSE;
        sd->on_hold = EINA_FALSE;
        return;
     }
   if ((sd->reorder_mode) &&
       (sd->reorder_it))
     {
        evas_object_smart_callback_call
          (WIDGET(it), SIG_MOVED, sd->reorder_it);
        sd->reorder_it = NULL;
        sd->move_effect_enabled = EINA_FALSE;
        ecore_job_del(sd->calc_job);
        sd->calc_job =
          ecore_job_add(_calc_job, sd->obj);

        eo_do(WIDGET(it), elm_interface_scrollable_hold_set(EINA_FALSE));
        eo_do(WIDGET(it), elm_interface_scrollable_bounce_allow_set(
          GG_IT(it)->wsd->old_h_bounce,
          GG_IT(it)->wsd->old_v_bounce));

        edje_object_signal_emit(VIEW(it), "elm,state,reorder,disabled", "elm");
     }
   if (sd->longpressed)
     {
        sd->longpressed = EINA_FALSE;
        if (!sd->was_selected) it->unsel_cb(it);
        sd->was_selected = EINA_FALSE;
        return;
     }
   if (dragged)
     {
        if (it->want_unrealize)
          _elm_gengrid_item_unrealize(it, EINA_FALSE);
     }

   if (elm_widget_item_disabled_get(it) || (dragged)) return;

   if (sd->focused_item != (Elm_Object_Item *)it)
     elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);

   if (sd->multi &&
       ((sd->multi_select_mode != ELM_OBJECT_MULTI_SELECT_MODE_WITH_CONTROL) ||
        (evas_key_modifier_is_set(ev->modifiers, "Control"))))
     {
        if (!it->selected)
          {
             it->highlight_cb(it);
             it->sel_cb(it);
          }
        else it->unsel_cb(it);
     }
   else
     {
        if (!it->selected)
          {
             while (sd->selected)
               it->unsel_cb(sd->selected->data);
          }
        else
          {
             const Eina_List *l, *l_next;
             Elm_Gen_Item *item2;

             EINA_LIST_FOREACH_SAFE(sd->selected, l, l_next, item2)
               if (item2 != it) it->unsel_cb(item2);
          }
        it->highlight_cb(it);
        it->sel_cb(it);
     }
}

static void
_elm_gengrid_item_index_update(Elm_Gen_Item *it)
{
   if (it->position_update)
     {
        evas_object_smart_callback_call(WIDGET(it), SIG_INDEX_UPDATE, it);
        it->position_update = EINA_FALSE;
     }
}

static void
_item_unrealize_cb(Elm_Gen_Item *it)
{
   ELM_SAFE_FREE(VIEW(it), evas_object_del);
   ELM_SAFE_FREE(it->spacer, evas_object_del);
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)data;
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, NULL);

   if (it->itc->func.text_get)
     {
        const Eina_List *l;
        const char *key;

        if (!(it->texts)) it->texts =
          elm_widget_stringlist_get(edje_object_data_get(VIEW(it), "texts"));

        EINA_LIST_FOREACH(it->texts, l, key)
          {
             char *s = it->itc->func.text_get
                ((void *)it->base.data, WIDGET(it), key);
             return s;
          }
     }

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)data;
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, NULL);

   if (it->base.disabled)
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_on_highlight_cb(void *data)
{
   Evas_Coord x, y, w, h;
   Evas_Coord sx, sy, sw, sh;
   Elm_Gen_Item *it = (Elm_Gen_Item *)data;
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   evas_object_geometry_get(it->base.view, &x, &y, &w, &h);

   evas_object_geometry_get(it->base.widget, &sx, &sy, &sw, &sh);
   if ((x < sx) || (y < sy) || ((x + w) > (sx + sw)) || ((y + h) > (sy + sh)))
     elm_gengrid_item_bring_in((Elm_Object_Item *)it,
                               ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static void
_access_widget_item_register(Elm_Gen_Item *it)
{
   Elm_Access_Info *ai;

   _elm_access_widget_item_register((Elm_Widget_Item *)it);

   ai = _elm_access_info_get(it->base.access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Gengrid Item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);
   _elm_access_on_highlight_hook_set(ai, _access_on_highlight_cb, it);
}

static void
_item_realize(Elm_Gen_Item *it)
{
   char buf[1024];
   char style[1024];
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   if ((it->realized) ||
       (it->generation < sd->generation))
     return;

   VIEW(it) = edje_object_add(evas_object_evas_get(WIDGET(it)));
   edje_object_scale_set
     (VIEW(it), elm_widget_scale_get(WIDGET(it)) * elm_config_scale_get());
   edje_object_mirrored_set(VIEW(it), elm_widget_mirrored_get(WIDGET(it)));
   evas_object_smart_member_add(VIEW(it), sd->pan_obj);
   elm_widget_sub_object_add(WIDGET(it), VIEW(it));
   snprintf(style, sizeof(style), "item/%s",
            it->itc->item_style ? it->itc->item_style : "default");
   if (!elm_widget_theme_object_set(WIDGET(it), VIEW(it), "gengrid", style,
                                    elm_widget_style_get(WIDGET(it))))
     {
        ERR("%s is not a valid gengrid item style. "
            "Automatically falls back into default style.",
            it->itc->item_style);
        elm_widget_theme_object_set
           (WIDGET(it), VIEW(it), "gengrid", "item/default", "default");
     }
   evas_object_stack_below(VIEW(it), sd->stack);

   if (edje_object_part_exists(VIEW(it), "elm.swallow.pad"))
     {
        it->spacer =
           evas_object_rectangle_add(evas_object_evas_get(WIDGET(it)));
        evas_object_color_set(it->spacer, 0, 0, 0, 0);
        elm_widget_sub_object_add(WIDGET(it), it->spacer);
        evas_object_size_hint_min_set(it->spacer, 2 * elm_config_scale_get(), 1);
        edje_object_part_swallow(VIEW(it), "elm.swallow.pad", it->spacer);
     }

   /* access */
   if (_elm_config->access_mode) _access_widget_item_register(it);

   if (it->itc->func.text_get)
     {
        const Eina_List *l;
        const char *key;

        it->texts =
          elm_widget_stringlist_get(edje_object_data_get(VIEW(it), "texts"));
        EINA_LIST_FOREACH(it->texts, l, key)
          {
             char *s = it->itc->func.text_get
                 ((void *)it->base.data, WIDGET(it), key);
             if (s)
               {
                  edje_object_part_text_escaped_set(VIEW(it), key, s);
                  free(s);
               }
          }
     }

   if (it->itc->func.content_get)
     {
        const Eina_List *l;
        const char *key;
        Evas_Object *ic = NULL;

        it->contents = elm_widget_stringlist_get
            (edje_object_data_get(VIEW(it), "contents"));
        EINA_LIST_FOREACH(it->contents, l, key)
          {
             ic = it->itc->func.content_get((void *)it->base.data, WIDGET(it), key);
             if (ic)
               {
                  it->content_objs = eina_list_append(it->content_objs, ic);
                  edje_object_part_swallow(VIEW(it), key, ic);
                  evas_object_show(ic);
                  elm_widget_sub_object_add(WIDGET(it), ic);
               }
          }
     }

   if (it->itc->func.state_get)
     {
        const Eina_List *l;
        const char *key;

        it->states =
          elm_widget_stringlist_get(edje_object_data_get(VIEW(it), "states"));
        EINA_LIST_FOREACH(it->states, l, key)
          {
             Eina_Bool on = it->itc->func.state_get
                 ((void *)it->base.data, WIDGET(it), l->data);
             if (on)
               {
                  snprintf(buf, sizeof(buf), "elm,state,%s,active", key);
                  edje_object_signal_emit(VIEW(it), buf, "elm");
               }
          }
     }

   if (it->group)
     {
        if ((!sd->group_item_width)
            && (!sd->group_item_height))
          {
             edje_object_size_min_restricted_calc
               (VIEW(it), &sd->group_item_width,
               &sd->group_item_height,
               sd->group_item_width,
               sd->group_item_height);
          }
     }
   else
     {
        if ((!sd->item_width)
            && (!sd->item_height))
          {
             edje_object_size_min_restricted_calc
               (VIEW(it), &sd->item_width,
               &sd->item_height,
               sd->item_width,
               sd->item_height);
             elm_coords_finger_size_adjust
               (1, &sd->item_width, 1,
               &sd->item_height);
          }

        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_MOUSE_DOWN, _item_mouse_down_cb, it);
        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_MOUSE_UP, _item_mouse_up_cb, it);
        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_MOUSE_MOVE, _item_mouse_move_cb, it);
        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_MOUSE_IN, _item_mouse_in_cb, it);

        _elm_gengrid_item_index_update(it);

        if (it->selected)
          edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
        if (elm_widget_item_disabled_get(it))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
     }
   edje_object_message_signal_process(VIEW(it));
   evas_object_show(VIEW(it));

   if (it->tooltip.content_cb)
     {
        elm_widget_item_tooltip_content_cb_set
          (it, it->tooltip.content_cb, it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, it->tooltip.style);
        elm_widget_item_tooltip_window_mode_set(it, it->tooltip.free_size);
     }

   if (it->mouse_cursor)
     elm_widget_item_cursor_set(it, it->mouse_cursor);

   _elm_widget_item_highlight_in_theme(WIDGET(it), (Elm_Object_Item *)it);

   it->realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;
}

static Eina_Bool
_reorder_item_move_animator_cb(void *data)
{
   Elm_Gen_Item *it = data;
   Evas_Coord dx, dy;
   double tt, t;
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   tt = REORDER_EFFECT_TIME;
   t = ((0.0 > (t = ecore_loop_time_get() -
                  GG_IT(it)->moving_effect_start_time)) ? 0.0 : t);
   dx = ((GG_IT(it)->tx - GG_IT(it)->ox) / 10)
     * elm_config_scale_get();
   dy = ((GG_IT(it)->ty - GG_IT(it)->oy) / 10)
     * elm_config_scale_get();

   if (t <= tt)
     {
        GG_IT(it)->rx += (1 * sin((t / tt) * (M_PI / 2)) * dx);
        GG_IT(it)->ry += (1 * sin((t / tt) * (M_PI / 2)) * dy);
     }
   else
     {
        GG_IT(it)->rx += dx;
        GG_IT(it)->ry += dy;
     }

   if ((((dx > 0) && (GG_IT(it)->rx >= GG_IT(it)->tx)) ||
        ((dx <= 0) && (GG_IT(it)->rx <= GG_IT(it)->tx)))
       && (((dy > 0) && (GG_IT(it)->ry >= GG_IT(it)->ty)) ||
           ((dy <= 0) && (GG_IT(it)->ry <= GG_IT(it)->ty))))
     {
        evas_object_move(VIEW(it), GG_IT(it)->tx, GG_IT(it)->ty);
        if (it->group)
          {
             Evas_Coord vw, vh;

             evas_object_geometry_get
               (sd->pan_obj, NULL, NULL, &vw, &vh);
             if (sd->horizontal)
               evas_object_resize
                 (VIEW(it), sd->group_item_width, vh);
             else
               evas_object_resize
                 (VIEW(it), vw, sd->group_item_height);
          }
        else
          evas_object_resize(VIEW(it), sd->item_width, sd->item_height);
        GG_IT(it)->moving = EINA_FALSE;
        GG_IT(it)->item_reorder_move_animator = NULL;

        return ECORE_CALLBACK_CANCEL;
     }

   evas_object_move(VIEW(it), GG_IT(it)->rx, GG_IT(it)->ry);
   if (it->group)
     {
        Evas_Coord vw, vh;

        evas_object_geometry_get(sd->pan_obj, NULL, NULL, &vw, &vh);
        if (sd->horizontal)
          evas_object_resize(VIEW(it), sd->group_item_width, vh);
        else
          evas_object_resize(VIEW(it), vw, sd->group_item_height);
     }
   else
     evas_object_resize(VIEW(it), sd->item_width, sd->item_height);

   return ECORE_CALLBACK_RENEW;
}

static void
_item_place(Elm_Gen_Item *it,
            Evas_Coord cx,
            Evas_Coord cy)
{
   Evas_Coord x, y, ox, oy, cvx, cvy, cvw, cvh, iw, ih, ww;
   Evas_Coord tch, tcw, alignw = 0, alignh = 0, vw, vh;
   Eina_Bool reorder_item_move_forward = EINA_FALSE;
   Elm_Gengrid_Data *wsd;
   Eina_Bool was_realized;
   Elm_Gen_Item_Type *item;
   long items_count;

   item = GG_IT(it);
   wsd = GG_IT(it)->wsd;

   it->x = cx;
   it->y = cy;
   evas_object_geometry_get(wsd->pan_obj, &ox, &oy, &vw, &vh);

   /* Preload rows/columns at each side of the Gengrid */
   cvx = ox - PRELOAD * wsd->item_width;
   cvy = oy - PRELOAD * wsd->item_height;
   cvw = vw + 2 * PRELOAD * wsd->item_width;
   cvh = vh + 2 * PRELOAD * wsd->item_height;

   items_count = wsd->item_count -
     eina_list_count(wsd->group_items) + wsd->items_lost;
   if (wsd->horizontal)
     {
        int columns, items_visible = 0, items_row;

        if (wsd->item_height > 0)
          items_visible = vh / wsd->item_height;
        if (items_visible < 1)
          items_visible = 1;

        columns = items_count / items_visible;
        if (items_count % items_visible)
          columns++;

        tcw = (wsd->item_width * columns) + (wsd->group_item_width *
                                             eina_list_count(wsd->group_items));
        alignw = (vw - tcw) * wsd->align_x;

        items_row = items_visible;
        if ((unsigned int)items_row > wsd->item_count)
          items_row = wsd->item_count;
        if (wsd->filled && (unsigned int)wsd->nmax
            > (unsigned int)wsd->item_count)
          tch = wsd->nmax * wsd->item_height;
        else
          tch = items_row * wsd->item_height;
        alignh = (vh - tch) * wsd->align_y;
     }
   else
     {
        unsigned int rows, items_visible = 0, items_col;

        if (wsd->item_width > 0)
          items_visible = vw / wsd->item_width;
        if (items_visible < 1)
          items_visible = 1;

        rows = items_count / items_visible;
        if (items_count % items_visible)
          rows++;

        tch = (wsd->item_height * rows) + (wsd->group_item_height *
                                           eina_list_count(wsd->group_items));
        alignh = (vh - tch) * wsd->align_y;

        items_col = items_visible;
        if (items_col > wsd->item_count)
          items_col = wsd->item_count;
        if (wsd->filled && (unsigned int)wsd->nmax
            > (unsigned int)wsd->item_count)
          tcw = wsd->nmax * wsd->item_width;
        else
          tcw = items_col * wsd->item_width;
        alignw = (vw - tcw) * wsd->align_x;
     }

   if (it->group)
     {
        if (wsd->horizontal)
          {
             x = (((cx - item->prev_group) * wsd->item_width)
                  + (item->prev_group * wsd->group_item_width)) -
               wsd->pan_x + ox + alignw;
             y = oy;
             iw = wsd->group_item_width;
             ih = vh;
          }
        else
          {
             x = ox;
             y = (((cy - item->prev_group) * wsd->item_height)
                  + (item->prev_group * wsd->group_item_height))
               - wsd->pan_y + oy + alignh;
             iw = vw;
             ih = wsd->group_item_height;
          }
        item->gx = x;
        item->gy = y;
     }
   else
     {
        if (wsd->horizontal)
          {
             x = (((cx - item->prev_group) * wsd->item_width)
                  + (item->prev_group * wsd->group_item_width)) -
               wsd->pan_x + ox + alignw;
             y = (cy * wsd->item_height) - wsd->pan_y + oy + alignh;
          }
        else
          {
             x = (cx * wsd->item_width) - wsd->pan_x + ox + alignw;
             y = (((cy - item->prev_group)
                   * wsd->item_height) + (item->prev_group *
                                          wsd->group_item_height)) -
               wsd->pan_y + oy + alignh;
          }
        if (elm_widget_mirrored_get(WIDGET(it))) /* Switch items side
                                                  * and componsate for
                                                  * pan_x when in RTL
                                                  * mode */
          {
             evas_object_geometry_get(WIDGET(it), NULL, NULL, &ww, NULL);
             x = ww - x - wsd->item_width - wsd->pan_x - wsd->pan_x;
          }
        iw = wsd->item_width;
        ih = wsd->item_height;
     }

   was_realized = it->realized;
   if (ELM_RECTS_INTERSECT(x, y, iw, ih, cvx, cvy, cvw, cvh))
     {
        _item_realize(it);
        if (!was_realized)
          evas_object_smart_callback_call(WIDGET(it), SIG_REALIZED, it);
        if (it->parent)
          {
             if (wsd->horizontal)
               {
                  if (it->parent->item->gx < ox)
                    {
                       it->parent->item->gx = x + wsd->item_width -
                         wsd->group_item_width;
                       if (it->parent->item->gx > ox)
                         it->parent->item->gx = ox;
                    }
                  it->parent->item->group_realized = EINA_TRUE;
               }
             else
               {
                  if (it->parent->item->gy < oy)
                    {
                       it->parent->item->gy = y + wsd->item_height -
                         wsd->group_item_height;
                       if (it->parent->item->gy > oy)
                         it->parent->item->gy = oy;
                    }
                  it->parent->item->group_realized = EINA_TRUE;
               }
          }
        if (wsd->reorder_mode)
          {
             if (wsd->reorder_it)
               {
                  if (item->moving) return;

                  if (!wsd->move_effect_enabled)
                    {
                       item->ox = x;
                       item->oy = y;
                    }
                  if (wsd->reorder_it == it)
                    {
                       evas_object_move(VIEW(it), wsd->reorder_item_x,
                                        wsd->reorder_item_y);
                       evas_object_resize(VIEW(it), iw, ih);
                       return;
                    }
                  else
                    {
                       Evas_Coord nx, ny, nw, nh;

                       if (wsd->move_effect_enabled)
                         {
                            if ((item->ox != x) || (item->oy != y))
                              if (((wsd->old_pan_x == wsd->pan_x)
                                   && (wsd->old_pan_y == wsd->pan_y))
                                  || ((wsd->old_pan_x != wsd->pan_x) &&
                                      !(item->ox - wsd->pan_x
                                        + wsd->old_pan_x == x)) ||
                                  ((wsd->old_pan_y != wsd->pan_y) &&
                                   !(item->oy - wsd->pan_y +
                                     wsd->old_pan_y == y)))
                                {
                                   item->tx = x;
                                   item->ty = y;
                                   item->rx = item->ox;
                                   item->ry = item->oy;
                                   item->moving = EINA_TRUE;
                                   item->moving_effect_start_time =
                                     ecore_loop_time_get();
                                   item->item_reorder_move_animator =
                                     ecore_animator_add
                                       (_reorder_item_move_animator_cb, it);
                                   return;
                                }
                         }

                       /* need fix here */
                       if (it->group)
                         {
                            if (wsd->horizontal)
                              {
                                 nx = x + (wsd->group_item_width / 2);
                                 ny = y;
                                 nw = 1;
                                 nh = vh;
                              }
                            else
                              {
                                 nx = x;
                                 ny = y + (wsd->group_item_height / 2);
                                 nw = vw;
                                 nh = 1;
                              }
                         }
                       else
                         {
                            nx = x + (wsd->item_width / 2);
                            ny = y + (wsd->item_height / 2);
                            nw = 1;
                            nh = 1;
                         }

                       if (ELM_RECTS_INTERSECT
                             (wsd->reorder_item_x, wsd->reorder_item_y,
                             wsd->item_width, wsd->item_height,
                             nx, ny, nw, nh))
                         {
                            if (wsd->horizontal)
                              {
                                 if ((wsd->nmax * wsd->reorder_it->x +
                                      wsd->reorder_it->y) >
                                     (wsd->nmax * it->x + it->y))
                                   reorder_item_move_forward = EINA_TRUE;
                              }
                            else
                              {
                                 if ((wsd->nmax * wsd->reorder_it->y +
                                      wsd->reorder_it->x) >
                                     (wsd->nmax * it->y + it->x))
                                   reorder_item_move_forward = EINA_TRUE;
                              }

                            wsd->items = eina_inlist_remove
                                (wsd->items,
                                EINA_INLIST_GET(wsd->reorder_it));
                            if (reorder_item_move_forward)
                              wsd->items = eina_inlist_prepend_relative
                                  (wsd->items,
                                  EINA_INLIST_GET(wsd->reorder_it),
                                  EINA_INLIST_GET(it));
                            else
                              wsd->items = eina_inlist_append_relative
                                  (wsd->items,
                                  EINA_INLIST_GET(wsd->reorder_it),
                                  EINA_INLIST_GET(it));

                            wsd->reorder_item_changed = EINA_TRUE;
                            wsd->move_effect_enabled = EINA_TRUE;
                            ecore_job_del(wsd->calc_job);
                            wsd->calc_job =
                              ecore_job_add(_calc_job, wsd->obj);

                            return;
                         }
                    }
               }
             else if (item->item_reorder_move_animator)
               {
                  ELM_SAFE_FREE(item->item_reorder_move_animator,
                                ecore_animator_del);
                  item->moving = EINA_FALSE;
               }
          }
        if (!it->group)
          {
             evas_object_move(VIEW(it), x, y);
             evas_object_resize(VIEW(it), iw, ih);
          }
        else
          item->group_realized = EINA_TRUE;
     }
   else
     {
        if (!it->group)
          _elm_gengrid_item_unrealize(it, EINA_FALSE);
        else
          item->group_realized = EINA_FALSE;
     }
}

static void
_group_item_place(Elm_Gengrid_Pan_Data *psd)
{
   Evas_Coord iw, ih, vw, vh;
   Eina_Bool was_realized;
   Elm_Gen_Item *it;
   Eina_List *l;

   evas_object_geometry_get(psd->wsd->pan_obj, NULL, NULL, &vw, &vh);
   if (psd->wsd->horizontal)
     {
        iw = psd->wsd->group_item_width;
        ih = vh;
     }
   else
     {
        iw = vw;
        ih = psd->wsd->group_item_height;
     }
   EINA_LIST_FOREACH(psd->wsd->group_items, l, it)
     {
        was_realized = it->realized;
        if (GG_IT(it)->group_realized)
          {
             _item_realize(it);
             if (!was_realized)
               evas_object_smart_callback_call(WIDGET(it), SIG_REALIZED, it);
             evas_object_move
               (VIEW(it), GG_IT(it)->gx,
               GG_IT(it)->gy);
             evas_object_resize(VIEW(it), iw, ih);
             evas_object_raise(VIEW(it));
          }
        else
          _elm_gengrid_item_unrealize(it, EINA_FALSE);
     }
}

EOLIAN static void
_elm_gengrid_pan_evas_smart_calculate(Eo *obj EINA_UNUSED, Elm_Gengrid_Pan_Data *psd)
{
   Evas_Coord cx = 0, cy = 0;
   Elm_Gen_Item *it;

   Elm_Gengrid_Data *sd = psd->wsd;

   if (!sd->nmax) return;

   sd->reorder_item_changed = EINA_FALSE;

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->group)
          {
             if (sd->horizontal)
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
        if (sd->reorder_item_changed) return;
        if (it->group)
          {
             if (sd->horizontal)
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
             if (sd->horizontal)
               {
                  cy = (cy + 1) % sd->nmax;
                  if (!cy) cx++;
               }
             else
               {
                  cx = (cx + 1) % sd->nmax;
                  if (!cx) cy++;
               }
          }
     }
   _group_item_place(psd);

   if ((sd->reorder_mode) && (sd->reorder_it))
     {
        if (!sd->reorder_item_changed)
          {
             sd->old_pan_x = sd->pan_x;
             sd->old_pan_y = sd->pan_y;
          }
        sd->move_effect_enabled = EINA_FALSE;
     }

   evas_object_smart_callback_call
     (psd->wobj, SIG_CHANGED, NULL);

   if (sd->focused_item)
     _elm_widget_focus_highlight_start(psd->wobj);
}

EOLIAN static void
_elm_gengrid_pan_elm_pan_pos_set(Eo *obj, Elm_Gengrid_Pan_Data *psd, Evas_Coord x, Evas_Coord y)
{
   if ((x == psd->wsd->pan_x) && (y == psd->wsd->pan_y)) return;
   psd->wsd->pan_x = x;
   psd->wsd->pan_y = y;
   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_gengrid_pan_elm_pan_pos_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = psd->wsd->pan_x;
   if (y) *y = psd->wsd->pan_y;
}

EOLIAN static void
_elm_gengrid_pan_elm_pan_content_size_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Pan_Data *psd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = psd->wsd->minw;
   if (h) *h = psd->wsd->minh;
}

EOLIAN static void
_elm_gengrid_pan_elm_pan_pos_max_get(Eo *obj, Elm_Gengrid_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord ow, oh;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if (x)
     *x = (ow < psd->wsd->minw) ? psd->wsd->minw - ow : 0;
   if (y)
     *y = (oh < psd->wsd->minh) ? psd->wsd->minh - oh : 0;
}

EOLIAN static void
_elm_gengrid_pan_elm_pan_pos_min_get(Eo *obj, Elm_Gengrid_Pan_Data *psd, Evas_Coord *x, Evas_Coord *y)
{
   Evas_Coord mx = 0, my = 0;

   eo_do(obj, elm_obj_pan_pos_max_get(&mx, &my));
   if (x)
     *x = -mx * psd->wsd->align_x;
   if (y)
     *y = -my * psd->wsd->align_y;
}

EOLIAN static void
_elm_gengrid_pan_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_PAN_CLASS_NAME_LEGACY, klass);
}

#include "elm_gengrid_pan.eo.c"

static void
_elm_gengrid_item_focused(Elm_Gen_Item *it)
{
   Evas_Object *obj = WIDGET(it);
   ELM_GENGRID_DATA_GET(obj, sd);
   const char *focus_raise;

   if (it->generation < sd->generation)
     return;

   if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       (it == (Elm_Gen_Item *)sd->focused_item) ||
       (elm_widget_item_disabled_get(it)))
     return;

   if (!_elm_config->focus_auto_scroll_bring_in_enable)
     elm_gengrid_item_show
            ((Elm_Object_Item *)it, ELM_GENGRID_ITEM_SCROLLTO_IN);
   else
     elm_gengrid_item_bring_in
            ((Elm_Object_Item *)it, ELM_GENGRID_ITEM_SCROLLTO_IN);
   sd->focused_item = (Elm_Object_Item *)it;

   if (elm_widget_focus_highlight_enabled_get(obj))
     {
        edje_object_signal_emit
           (VIEW(it), "elm,state,focused", "elm");
     }

   focus_raise = edje_object_data_get(VIEW(it), "focusraise");
   if ((focus_raise) && (!strcmp(focus_raise, "on")))
     evas_object_raise(VIEW(it));
   evas_object_smart_callback_call(obj, SIG_ITEM_FOCUSED, it);
}

static void
_elm_gengrid_item_unfocused(Elm_Gen_Item *it)
{
   Evas_Object *obj = WIDGET(it);
   ELM_GENGRID_DATA_GET(obj, sd);

   if (it->generation < sd->generation)
     return;

   if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY))
     return;

   if ((!sd->focused_item) ||
       (it != (Elm_Gen_Item *)sd->focused_item))
     return;

   sd->prev_focused_item = (Elm_Object_Item *)it;

   if (elm_widget_focus_highlight_enabled_get(obj))
     {
        edje_object_signal_emit
           (VIEW(sd->focused_item), "elm,state,unfocused", "elm");
     }

   sd->focused_item = NULL;
   evas_object_smart_callback_call(obj, SIG_ITEM_UNFOCUSED, it);
}

static Eina_Bool
_item_focus_up(Elm_Gengrid_Data *sd)
{
   unsigned int i;
   Elm_Gen_Item *prev;

   if (!sd->focused_item)
     {
        prev = ELM_GEN_ITEM_FROM_INLIST(sd->items->last);
        while ((prev) && (prev->generation < sd->generation))
          prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
        elm_object_item_focus_set((Elm_Object_Item *)prev, EINA_TRUE);
        return EINA_TRUE;
     }
   else
     {
        prev = (Elm_Gen_Item *)elm_gengrid_item_prev_get(sd->focused_item);
        if (!prev) return EINA_FALSE;
        if(prev == (Elm_Gen_Item *)sd->focused_item) return EINA_FALSE;
     }

   for (i = 1; i < sd->nmax; i++)
     {
        Elm_Object_Item *tmp =
          elm_gengrid_item_prev_get((Elm_Object_Item *)prev);
        if (!tmp) return EINA_FALSE;
        prev = (Elm_Gen_Item *)tmp;
     }

   elm_object_item_focus_set((Elm_Object_Item *)prev, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_item_focus_down(Elm_Gengrid_Data *sd)
{
   unsigned int i;
   Elm_Gen_Item *next;

   if (!sd->focused_item)
     {
        next = ELM_GEN_ITEM_FROM_INLIST(sd->items);
        while ((next) && (next->generation < sd->generation))
          next = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);

        elm_object_item_focus_set((Elm_Object_Item *)next, EINA_TRUE);
        return EINA_TRUE;
     }
   else
     {
        next = (Elm_Gen_Item *)elm_gengrid_item_next_get(sd->focused_item);
        if (!next) return EINA_FALSE;
        if (next == (Elm_Gen_Item *)sd->focused_item) return EINA_FALSE;
     }

   for (i = 1; i < sd->nmax; i++)
     {
        Elm_Object_Item *tmp =
          elm_gengrid_item_next_get((Elm_Object_Item *)next);
        if (!tmp) return EINA_FALSE;
        next = (Elm_Gen_Item *)tmp;
     }

   elm_object_item_focus_set((Elm_Object_Item *)next, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_item_focus_left(Elm_Gengrid_Data *sd)
{
   Elm_Gen_Item *prev;

   if (!sd->focused_item)
     {
        prev = ELM_GEN_ITEM_FROM_INLIST(sd->items->last);
        while ((prev) && (prev->generation < sd->generation))
          prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
     }
   else
     {
        prev = (Elm_Gen_Item *)elm_gengrid_item_prev_get(sd->focused_item);
        if (!prev) return EINA_FALSE;
        if (prev == (Elm_Gen_Item *)sd->focused_item) return EINA_FALSE;
     }

   elm_object_item_focus_set((Elm_Object_Item *)prev, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_item_focus_right(Elm_Gengrid_Data *sd)
{
   Elm_Gen_Item *next;

   if (!sd->focused_item)
     {
        next = ELM_GEN_ITEM_FROM_INLIST(sd->items);
        while ((next) && (next->generation < sd->generation))
          next = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
     }
   else
     {
        next = (Elm_Gen_Item *)elm_gengrid_item_next_get(sd->focused_item);
        if (!next) return EINA_FALSE;
        if (next == (Elm_Gen_Item *)sd->focused_item) return EINA_FALSE;
     }

   elm_object_item_focus_set((Elm_Object_Item *)next, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_left(Elm_Gengrid_Data *sd)
{
   Elm_Object_Item *prev;

   if (!sd->selected) return EINA_FALSE;

   prev = elm_gengrid_item_prev_get(sd->last_selected_item);
   if (!prev) return EINA_TRUE;

   if (elm_gengrid_item_selected_get(prev))
     {
        elm_gengrid_item_selected_set(sd->last_selected_item, EINA_FALSE);
        sd->last_selected_item = prev;
        elm_gengrid_item_show
          (sd->last_selected_item, ELM_GENGRID_ITEM_SCROLLTO_IN);
     }
   else
     {
        elm_gengrid_item_selected_set(prev, EINA_TRUE);
        elm_gengrid_item_show(prev, ELM_GENGRID_ITEM_SCROLLTO_IN);
     }

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_right(Elm_Gengrid_Data *sd)
{
   Elm_Object_Item *next;

   if (!sd->selected) return EINA_FALSE;

   next = elm_gengrid_item_next_get(sd->last_selected_item);
   if (!next) return EINA_TRUE;

   if (elm_gengrid_item_selected_get(next))
     {
        elm_gengrid_item_selected_set(sd->last_selected_item, EINA_FALSE);
        sd->last_selected_item = next;
        elm_gengrid_item_show
          (sd->last_selected_item, ELM_GENGRID_ITEM_SCROLLTO_IN);
     }
   else
     {
        elm_gengrid_item_selected_set(next, EINA_TRUE);
        elm_gengrid_item_show(next, ELM_GENGRID_ITEM_SCROLLTO_IN);
     }

   return EINA_TRUE;
}

static Eina_Bool
_item_multi_select_up(Elm_Gengrid_Data *sd)
{
   unsigned int i;
   Eina_Bool r = EINA_TRUE;

   if (!sd->selected) return EINA_FALSE;

   for (i = 0; (r) && (i < sd->nmax); i++)
     r &= _item_multi_select_left(sd);

   return r;
}

static Eina_Bool
_item_multi_select_down(Elm_Gengrid_Data *sd)
{
   unsigned int i;
   Eina_Bool r = EINA_TRUE;

   if (!sd->selected) return EINA_FALSE;

   for (i = 0; (r) && (i < sd->nmax); i++)
     r &= _item_multi_select_right(sd);

   return r;
}

static Eina_Bool
_all_items_deselect(Elm_Gengrid_Data *sd)
{
   if (!sd->selected) return EINA_FALSE;

   while (sd->selected)
     elm_gengrid_item_selected_set
       ((Elm_Object_Item *)sd->selected->data, EINA_FALSE);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_up(Elm_Gengrid_Data *sd)
{
   unsigned int i;
   Elm_Gen_Item *prev;

   if (!sd->selected)
     {
        prev = ELM_GEN_ITEM_FROM_INLIST(sd->items->last);
        while ((prev) && (prev->generation < sd->generation))
          prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
        elm_gengrid_item_selected_set((Elm_Object_Item *)prev, EINA_TRUE);
        return EINA_TRUE;
     }
   else
     prev = (Elm_Gen_Item *)elm_gengrid_item_prev_get(sd->last_selected_item);

   if (!prev) return EINA_FALSE;

   for (i = 1; i < sd->nmax; i++)
     {
        Elm_Object_Item *tmp =
          elm_gengrid_item_prev_get((Elm_Object_Item *)prev);
        if (!tmp) return EINA_FALSE;
        prev = (Elm_Gen_Item *)tmp;
     }

   _all_items_deselect(sd);

   elm_gengrid_item_selected_set((Elm_Object_Item *)prev, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_down(Elm_Gengrid_Data *sd)
{
   unsigned int i;
   Elm_Gen_Item *next;

   if (!sd->selected)
     {
        next = ELM_GEN_ITEM_FROM_INLIST(sd->items);
        while ((next) && (next->generation < sd->generation))
          next = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
        elm_gengrid_item_selected_set((Elm_Object_Item *)next, EINA_TRUE);
        return EINA_TRUE;
     }
   else
     next = (Elm_Gen_Item *)elm_gengrid_item_next_get(sd->last_selected_item);

   if (!next) return EINA_FALSE;

   for (i = 1; i < sd->nmax; i++)
     {
        Elm_Object_Item *tmp =
          elm_gengrid_item_next_get((Elm_Object_Item *)next);
        if (!tmp) return EINA_FALSE;
        next = (Elm_Gen_Item *)tmp;
     }

   _all_items_deselect(sd);

   elm_gengrid_item_selected_set((Elm_Object_Item *)next, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_left(Elm_Gengrid_Data *sd)
{
   Elm_Gen_Item *prev;

   if (!sd->selected)
     {
        prev = ELM_GEN_ITEM_FROM_INLIST(sd->items->last);
        while ((prev) && (prev->generation < sd->generation))
          prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
     }
   else
     prev = (Elm_Gen_Item *)elm_gengrid_item_prev_get(sd->last_selected_item);

   if (!prev) return EINA_FALSE;

   _all_items_deselect(sd);

   elm_gengrid_item_selected_set((Elm_Object_Item *)prev, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_right(Elm_Gengrid_Data *sd)
{
   Elm_Gen_Item *next;

   if (!sd->selected)
     {
        next = ELM_GEN_ITEM_FROM_INLIST(sd->items);
        while ((next) && (next->generation < sd->generation))
          next = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
     }
   else
     next = (Elm_Gen_Item *)elm_gengrid_item_next_get(sd->last_selected_item);

   if (!next) return EINA_FALSE;

   _all_items_deselect(sd);

   elm_gengrid_item_selected_set((Elm_Object_Item *)next, EINA_TRUE);

   return EINA_TRUE;
}

static Eina_Bool
_elm_gengrid_item_edge_check(Elm_Object_Item *it,
                             Elm_Focus_Direction dir)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   ELM_GENGRID_DATA_GET(WIDGET(it), sd);
   Evas_Coord ix = 0, iy = 0; //item's geometry
   Evas_Coord cx = 0, cy = 0; //prev or next item's geometry
   Elm_Object_Item *item = NULL;
   Elm_Gen_Item *tmp = (Elm_Gen_Item *)it;

   evas_object_geometry_get(VIEW(it), &ix, &iy, NULL, NULL);

   if (((sd->horizontal) && (dir == ELM_FOCUS_UP)) ||
       ((!sd->horizontal) && (dir == ELM_FOCUS_LEFT)))
     {
        item = elm_gengrid_item_prev_get(it);
        while (item)
          {
             if (sd->reorder_mode || !elm_object_item_disabled_get(item)) break;
             item = elm_gengrid_item_prev_get(item);
          }
        if (item)
          {
             evas_object_geometry_get(VIEW(item), &cx, &cy, NULL, NULL);
             if (((sd->horizontal) && (ix == cx) && (iy > cy))||
                 ((!sd->horizontal) && (iy == cy) && (ix > cx)))
               return EINA_FALSE;
             else
               return EINA_TRUE;
          }
     }
   else if (((sd->horizontal) && (dir == ELM_FOCUS_DOWN)) ||
            ((!sd->horizontal) && (dir == ELM_FOCUS_RIGHT)))
     {
        item = elm_gengrid_item_next_get(it);
        while (item)
          {
             if (sd->reorder_mode || !elm_object_item_disabled_get(item)) break;
             item = elm_gengrid_item_next_get(item);
          }
        if (item)
          {
             evas_object_geometry_get(VIEW(item), &cx, &cy, NULL, NULL);
             if (((sd->horizontal) && (ix == cx) && (iy < cy)) ||
                 ((!sd->horizontal) && (iy == cy) && (ix < cx)))
               return EINA_FALSE;
             else
               return EINA_TRUE;
          }
     }
   else if (((!sd->horizontal) && (dir == ELM_FOCUS_UP)) ||
            ((sd->horizontal) && (dir == ELM_FOCUS_LEFT)))
     {
        Evas_Coord col, row, cvw, cvh;

        evas_object_geometry_get(sd->pan_obj, NULL, NULL, &cvw, &cvh);
        if (sd->horizontal && sd->item_height > 0)
          {
             row = cvh / sd->item_height;
             if (row <= 0) row = 1;
             if (tmp->position < row)
               return EINA_TRUE;
          }
        else if (sd->item_width > 0)
          {
             col = cvw / sd->item_width;
             if (col <= 0) col = 1;
             if (tmp->position < col)
               return EINA_TRUE;
          }
     }
   else if (((!sd->horizontal) && (dir == ELM_FOCUS_DOWN)) ||
            ((sd->horizontal) && (dir == ELM_FOCUS_RIGHT)))
     {
        Evas_Coord col = 0, row = 0, cvw, cvh;
        int x = 0;

        evas_object_geometry_get(sd->pan_obj, NULL, NULL, &cvw, &cvh);
        if (sd->horizontal && sd->item_height > 0)
          {
             row = cvh / sd->item_height;
             if (row <= 0) row = 1;
             col = sd->item_count / row;
             x = sd->item_count % row;
             if (x == 0)
               {
                  if ((tmp->position <= (row * col)) &&
                      (tmp->position >= (row *(col - 1))))
                    return EINA_TRUE;
               }
             else
               {
                  if ((tmp->position <= ((col * row) + x)) &&
                      (tmp->position >= (col * row)))
                    return EINA_TRUE;
               }
          }
        else if (sd->item_width > 0)
          {
             col = cvw / sd->item_width;
             if (col <= 0) col = 1;
             row = sd->item_count / col;
             x = sd->item_count % col;
             if (x == 0)
               {
                  if ((tmp->position <= (col * row)) &&
                      (tmp->position >= (col *(row - 1))))
                       return EINA_TRUE;
               }
             else
               {
                  if ((tmp->position <= ((col * row) + x)) &&
                      (tmp->position >= (col * row)))
                    return EINA_TRUE;
               }
          }
     }

   return EINA_FALSE;
}

static Elm_Gen_Item *
get_up_item(Elm_Gengrid_Data *sd, Elm_Object_Item *it)
{
   Elm_Gen_Item *prev = NULL;
   unsigned int i;

   prev = (Elm_Gen_Item *)elm_gengrid_item_prev_get(it);
   if (!prev)
     return prev;

   for (i = 1; i < sd->nmax; i++)
     {
        Elm_Object_Item *tmp =
          elm_gengrid_item_prev_get((Elm_Object_Item *)prev);
        if (!tmp) return prev;
        prev = (Elm_Gen_Item *)tmp;
     }

   return prev;
}

static Elm_Gen_Item *
get_down_item(Elm_Gengrid_Data *sd, Elm_Object_Item *it)
{
   Elm_Gen_Item *next = NULL;
   unsigned int i;

   next = (Elm_Gen_Item *)elm_gengrid_item_next_get(it);
   if (!next)
     return next;

   for (i = 1; i < sd->nmax; i++)
     {
        Elm_Object_Item *tmp =
          elm_gengrid_item_next_get((Elm_Object_Item *)next);
        if (!tmp) return next;
        next = (Elm_Gen_Item *)tmp;
     }

   return next;
}

static void
_anim_end(Elm_Gengrid_Data *sd)
{
   Eina_Inlist *tmp = NULL;
   Elm_Gen_Item *it1_prev = NULL, *it2_prev = NULL;

   it1_prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(sd->reorder.it1)->prev);
   it2_prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(sd->reorder.it2)->prev);

   if ((!sd->horizontal && ((sd->reorder.dir == ELM_FOCUS_UP) || (sd->reorder.dir == ELM_FOCUS_DOWN))) ||
       (sd->horizontal && ((sd->reorder.dir == ELM_FOCUS_LEFT) || (sd->reorder.dir == ELM_FOCUS_RIGHT))))
     {
        sd->items = eina_inlist_remove(sd->items, EINA_INLIST_GET(sd->reorder.it1));
        sd->items = eina_inlist_remove(sd->items, EINA_INLIST_GET(sd->reorder.it2));

        if (it1_prev)
          {
             tmp = eina_inlist_find(sd->items, EINA_INLIST_GET(it1_prev));
             sd->items = eina_inlist_append_relative(sd->items, EINA_INLIST_GET(sd->reorder.it2),
                                                     tmp);
          }
        else
          sd->items = eina_inlist_prepend(sd->items, EINA_INLIST_GET(sd->reorder.it2));
        if (it2_prev)
          {
             tmp = eina_inlist_find(sd->items, EINA_INLIST_GET(it2_prev));
             sd->items = eina_inlist_append_relative(sd->items, EINA_INLIST_GET(sd->reorder.it1),
                                                     tmp);
          }
        else
          sd->items = eina_inlist_prepend(sd->items, EINA_INLIST_GET(sd->reorder.it1));
     }
   else if ((!sd->horizontal && (sd->reorder.dir == ELM_FOCUS_LEFT)) ||
            (sd->horizontal && (sd->reorder.dir == ELM_FOCUS_UP)))
     {
        sd->items = eina_inlist_remove(sd->items, EINA_INLIST_GET(sd->reorder.it1));
        sd->items = eina_inlist_prepend_relative(sd->items, EINA_INLIST_GET(sd->reorder.it1),
                                                 EINA_INLIST_GET(sd->reorder.it2));
     }
   else if ((!sd->horizontal && (sd->reorder.dir == ELM_FOCUS_RIGHT)) ||
            (sd->horizontal && (sd->reorder.dir == ELM_FOCUS_DOWN)))
     {
        sd->items = eina_inlist_remove(sd->items, EINA_INLIST_GET(sd->reorder.it1));
        sd->items = eina_inlist_append_relative(sd->items, EINA_INLIST_GET(sd->reorder.it1),
                                                EINA_INLIST_GET(sd->reorder.it2));
     }
   _item_position_update(sd->items, 0);

   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd->obj);
}

static Eina_Bool
_item_move_cb(void *data, double pos)
{
   Elm_Gengrid_Data *sd = data;
   double frame = pos;
   Evas_Coord xx1, yy1, xx2, yy2;
   double dx, dy;

   switch (sd->reorder.tween_mode)
     {
      case ECORE_POS_MAP_LINEAR:
         frame = ecore_animator_pos_map(frame, sd->reorder.tween_mode, 0, 0);
         break;
      case ECORE_POS_MAP_ACCELERATE:
      case ECORE_POS_MAP_DECELERATE:
      case ECORE_POS_MAP_SINUSOIDAL:
         frame = ecore_animator_pos_map(frame, sd->reorder.tween_mode, 1.0, 0);
         break;
      case ECORE_POS_MAP_DIVISOR_INTERP:
      case ECORE_POS_MAP_BOUNCE:
      case ECORE_POS_MAP_SPRING:
         frame = ecore_animator_pos_map(frame, sd->reorder.tween_mode, 1.0, 1.0);
         break;
      default:
         frame = ecore_animator_pos_map(frame, sd->reorder.tween_mode, 0, 0);
         break;
     }

   dx = sd->reorder.x2 - sd->reorder.x1;
   dy = sd->reorder.y2 - sd->reorder.y1;
   xx1 = sd->reorder.x1 + dx * frame;
   yy1 = sd->reorder.y1 + dy * frame;

   xx2 = sd->reorder.x2 - dx * frame;
   yy2 = sd->reorder.y2 - dy * frame;

   evas_object_move(VIEW(sd->reorder.it1), xx1, yy1);
   evas_object_move(VIEW(sd->reorder.it2), xx2, yy2);

   if (pos == 1.0)
     {
        _anim_end(sd);
        elm_gengrid_item_show((Elm_Object_Item *)sd->reorder.it1,
                              ELM_GENGRID_ITEM_SCROLLTO_IN);
        evas_object_smart_callback_call(sd->obj, SIG_ITEM_REORDER_START,
                                        sd->reorder.it1);
        sd->reorder.running = EINA_FALSE;
     }
   _elm_widget_focus_highlight_start(sd->obj);

   return EINA_TRUE;
}

static void
_swap_items(Elm_Gen_Item *it1, Elm_Gen_Item *it2, Elm_Focus_Direction dir)
{
   ELM_GENGRID_DATA_GET(WIDGET(it1), sd);
   Evas_Coord xx1, yy1, xx2, yy2;

   evas_object_geometry_get(VIEW(it1), &xx1, &yy1, NULL, NULL);
   evas_object_geometry_get(VIEW(it2), &xx2, &yy2, NULL, NULL);
   sd->reorder.x1 = xx1;
   sd->reorder.y1 = yy1;
   sd->reorder.x2 = xx2;
   sd->reorder.y2 = yy2;

   sd->reorder.running = EINA_TRUE;
   sd->reorder.dir = dir;
   sd->reorder.it1 = it1;
   sd->reorder.it2 = it2;

   evas_object_raise(VIEW(it2));
   evas_object_stack_above(VIEW(it1), VIEW(it2));
   evas_object_smart_callback_call(sd->obj, SIG_ITEM_REORDER_STOP,
                                   sd->reorder.it1);
   //TODO: Add elm config for time
   ecore_animator_timeline_add(0.3, _item_move_cb, sd);
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   ELM_GENGRID_DATA_GET(obj, sd);
   const char *dir = params;

   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;
   Elm_Object_Item *it = NULL;

   eo_do(obj,
         elm_interface_scrollable_content_pos_get(&x, &y),
         elm_interface_scrollable_step_size_get(&step_x, &step_y),
         elm_interface_scrollable_page_size_get(&page_x, &page_y),
         elm_interface_scrollable_content_viewport_geometry_get
         (NULL, NULL, &v_w, &v_h));

   if (sd->reorder_mode && sd->reorder.running) return EINA_TRUE;

   if (!strcmp(dir, "left"))
     {
        if (sd->reorder_mode)
          {
             Elm_Gen_Item *left;

             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_LEFT))
               return EINA_TRUE;
             if (!sd->horizontal)
               left  = (Elm_Gen_Item *)elm_gengrid_item_prev_get(sd->focused_item);
             else
               left = get_up_item(sd, sd->focused_item);
             if (!left)
               return EINA_TRUE;
             _swap_items((Elm_Gen_Item *)sd->focused_item, left, ELM_FOCUS_LEFT);

             return EINA_TRUE;
          }
        if (sd->horizontal)
          {
             if (!_elm_config->item_select_on_focus_disable)
               {
                  return _item_single_select_up(sd);
               }
             else
               return _item_focus_up(sd);
          }
        else
          {
             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_LEFT))
               return EINA_FALSE;
             if (!_elm_config->item_select_on_focus_disable)
               {
                  return _item_single_select_left(sd);
               }
             else
               return _item_focus_left(sd);
          }
     }
   else if (!strcmp(dir, "left_multi"))
     {
        if (sd->horizontal)
          {
             if (_item_multi_select_up(sd)) return EINA_TRUE;
             else if (_item_single_select_up(sd)) return EINA_TRUE;
             else return EINA_FALSE;
          }
        else
          {
             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_LEFT))
               return EINA_FALSE;
             if (_item_multi_select_left(sd)) return EINA_TRUE;
             else if (_item_single_select_left(sd)) return EINA_TRUE;
             else return EINA_FALSE;
          }
     }
   else if (!strcmp(dir, "right"))
     {
        if (sd->reorder_mode)
          {
             Elm_Gen_Item *right;

             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_RIGHT))
               return EINA_TRUE;
             if (!sd->horizontal)
               right = (Elm_Gen_Item *)elm_gengrid_item_next_get(sd->focused_item);
             else
               right = get_down_item(sd, sd->focused_item);
             if (!right)
               return EINA_TRUE;
             _swap_items((Elm_Gen_Item *)sd->focused_item, right, ELM_FOCUS_RIGHT);

             return EINA_TRUE;
          }
        if (sd->horizontal)
          {
             if (!_elm_config->item_select_on_focus_disable)
               {
                  return _item_single_select_down(sd);
               }
             else
               return _item_focus_down(sd);
          }
        else
          {
             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_RIGHT))
               return EINA_FALSE;
             if (!_elm_config->item_select_on_focus_disable)
               {
                  return _item_single_select_right(sd);
               }
             else
               return _item_focus_right(sd);
          }
     }
   else if (!strcmp(dir, "right_multi"))
     {
        if (sd->horizontal)
          {
             if (_item_multi_select_down(sd)) return EINA_TRUE;
             else if (_item_single_select_down(sd)) return EINA_TRUE;
             else return EINA_FALSE;
          }
        else
          {
             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_RIGHT))
               return EINA_FALSE;
             if (_item_multi_select_right(sd)) return EINA_TRUE;
             else if (_item_single_select_right(sd)) return EINA_TRUE;
             else return EINA_FALSE;
          }
     }
   else if (!strcmp(dir, "up"))
     {
        if (sd->reorder_mode)
          {
             Elm_Gen_Item *up;

             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_UP))
               return EINA_TRUE;
             if (!sd->horizontal)
               up = get_up_item(sd, sd->focused_item);
             else
               up = (Elm_Gen_Item *)elm_gengrid_item_prev_get(sd->focused_item);
             if (!up)
               return EINA_TRUE;
             _swap_items((Elm_Gen_Item *)sd->focused_item, up, ELM_FOCUS_UP);

             return EINA_TRUE;
          }
        if (sd->horizontal)
          {
             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_UP))
               return EINA_FALSE;
             if (!_elm_config->item_select_on_focus_disable)
               {
                  if (_item_single_select_left(sd)) return EINA_TRUE;
                  else return EINA_FALSE;
               }
             else
               return _item_focus_left(sd);
          }
        else
          {
             if (!_elm_config->item_select_on_focus_disable)
               {
                  return _item_single_select_up(sd);
               }
             else
               return _item_focus_up(sd);
          }
     }
   else if (!strcmp(dir, "up_multi"))
     {
        if (sd->horizontal)
          {
             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_UP))
               return EINA_FALSE;
             if (_item_multi_select_left(sd)) return EINA_TRUE;
             else if (_item_single_select_left(sd)) return EINA_TRUE;
             else return EINA_FALSE;
          }
        else
          {
             if (_item_multi_select_up(sd)) return EINA_TRUE;
             else if (_item_single_select_up(sd)) return EINA_TRUE;
             else return EINA_FALSE;
          }
     }
   else if (!strcmp(dir, "down"))
     {
        if (sd->reorder_mode)
          {
             Elm_Gen_Item *down;

             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_DOWN))
               return EINA_TRUE;
             if (!sd->horizontal)
               down = get_down_item(sd, sd->focused_item);
             else
               down = (Elm_Gen_Item *)elm_gengrid_item_next_get((Elm_Object_Item *)sd->focused_item);
             if (!down)
               return EINA_TRUE;
             _swap_items((Elm_Gen_Item *)sd->focused_item, down, ELM_FOCUS_DOWN);

             return EINA_TRUE;
          }
        if (sd->horizontal)
          {
             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_DOWN))
               return EINA_FALSE;
             if (!_elm_config->item_select_on_focus_disable)
               {
                  return _item_single_select_right(sd);
               }
             else
               return _item_focus_right(sd);
          }
        else
          {
             if (!_elm_config->item_select_on_focus_disable)
               {
                  return _item_single_select_down(sd);
               }
             else
               return _item_focus_down(sd);
          }
     }
   else if (!strcmp(dir, "down_multi"))
     {
        if (sd->horizontal)
          {
             if (_elm_gengrid_item_edge_check(sd->focused_item, ELM_FOCUS_DOWN))
               return EINA_FALSE;
             if (_item_multi_select_right(sd)) return EINA_TRUE;
             else if (_item_single_select_right(sd)) return EINA_TRUE;
             else return EINA_FALSE;
          }
        else
          {
             if (_item_multi_select_down(sd)) return EINA_TRUE;
             else if (_item_single_select_down(sd)) return EINA_TRUE;
             else return EINA_FALSE;
          }
     }
   else if (!strcmp(dir, "first"))
     {
        it = elm_gengrid_first_item_get(obj);
        if (!_elm_config->item_select_on_focus_disable)
          elm_gengrid_item_selected_set(it, EINA_TRUE);
        else
          elm_object_item_focus_set(it, EINA_TRUE);
        return EINA_TRUE;
     }
   else if (!strcmp(dir, "last"))
     {
        it = elm_gengrid_last_item_get(obj);
        if (!_elm_config->item_select_on_focus_disable)
          elm_gengrid_item_selected_set(it, EINA_TRUE);
        else
          elm_object_item_focus_set(it, EINA_TRUE);
        return EINA_TRUE;
     }
   else if (!strcmp(dir, "prior"))
     {
        if (sd->horizontal)
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
        if (sd->horizontal)
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

static Eina_Bool
_key_action_select(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_GENGRID_DATA_GET(obj, sd);
   Elm_Object_Item *it = NULL;

   if (!sd->multi)
     {
        it = elm_object_focused_item_get(obj);
        evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_key_action_escape(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_GENGRID_DATA_GET(obj, sd);

   if (!_all_items_deselect(sd)) return EINA_FALSE;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_gengrid_elm_widget_event(Eo *obj, Elm_Gengrid_Data *sd, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   (void) src;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (!sd->items) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

/*
 * This function searches the nearest visible item based on the given item.
 * If the given item is in the gengrid viewport, this returns the given item.
 * Or this searches the realized items and checks the nearest fully visible item
 * according to the given item's position.
 */
static Elm_Object_Item *
_elm_gengrid_nearest_visible_item_get(Evas_Object *obj, Elm_Object_Item *it)
{
   Evas_Coord vx = 0, vy = 0, vw = 0, vh = 0; // gengrid viewport geometry
   Evas_Coord ix = 0, iy = 0, iw = 0, ih = 0; // given item geometry
   Evas_Coord cx = 0, cy = 0, cw = 0, ch = 0; // candidate item geometry
   Eina_List *item_list = NULL, *l = NULL;
   Elm_Object_Item *item = NULL;
   ELM_GENGRID_DATA_GET(obj, sd);
   Eina_Bool search_next = EINA_FALSE;

   if (!it) return NULL;

   evas_object_geometry_get(sd->pan_obj, &vx, &vy, &vw, &vh);
   evas_object_geometry_get(VIEW(it), &ix, &iy, &iw, &ih); // FIXME: check if the item is realized or not

   item_list = elm_gengrid_realized_items_get(obj);

   if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, ix, iy, iw, ih))
     {
        if (!elm_object_item_disabled_get(it))
          return it;
        else
          search_next = EINA_TRUE;
     }

   if ((iy < vy) || search_next)
     {
        EINA_LIST_FOREACH(item_list, l, item)
          {
             evas_object_geometry_get(VIEW(item), &cx, &cy, &cw, &ch);
             if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, cx, cy, cw, ch) &&
                 !elm_object_item_disabled_get(item))
               return item;
          }
     }
   else
     {
        EINA_LIST_REVERSE_FOREACH(item_list, l, item)
          {
             evas_object_geometry_get(VIEW(item), &cx, &cy, &cw, &ch);
             if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, cx, cy, cw, ch) &&
                 !elm_object_item_disabled_get(item))
               return item;
          }
     }
   return it;
}

EOLIAN static Eina_Bool
_elm_gengrid_elm_widget_on_focus(Eo *obj, Elm_Gengrid_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Object_Item *it = NULL;
   Eina_Bool is_sel = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_on_focus());
   if (!int_ret) return EINA_FALSE;

   if (elm_widget_focus_get(obj) && (sd->selected) &&
       (!sd->last_selected_item))
     sd->last_selected_item = eina_list_data_get(sd->selected);

   if (elm_widget_focus_get(obj) && !sd->mouse_down)
     {
        if (sd->last_focused_item)
          it = sd->last_focused_item;
        else if (sd->last_selected_item)
          it = sd->last_selected_item;
        else if (elm_widget_focus_highlight_enabled_get(obj))
          {
             it = elm_gengrid_first_item_get(obj);
             is_sel = EINA_TRUE;
          }

        if (it)
          {
             it = _elm_gengrid_nearest_visible_item_get(obj, it);
             if (it)
               {
                  if (!_elm_config->item_select_on_focus_disable && is_sel)
                    elm_gengrid_item_selected_set(it, EINA_TRUE);
                  else
                    elm_object_item_focus_set(it, EINA_TRUE);
                  _elm_widget_focus_highlight_start(obj);
               }
          }
     }
   else
     {
        if (sd->focused_item)
          {
             sd->prev_focused_item = sd->focused_item;
             sd->last_focused_item = sd->focused_item;
             _elm_gengrid_item_unfocused((Elm_Gen_Item *)sd->focused_item);
          }
     }

   return EINA_TRUE;
}

static Eina_Bool _elm_gengrid_smart_focus_next_enable = EINA_FALSE;

EOLIAN static Eina_Bool
_elm_gengrid_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *_pd EINA_UNUSED)
{
   return _elm_gengrid_smart_focus_next_enable;
}

EOLIAN static Eina_Bool
_elm_gengrid_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *_pd EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_gengrid_elm_widget_focus_next(Eo *obj, Elm_Gengrid_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
{
   Eina_List *items = NULL;
   Elm_Gen_Item *it;

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->realized)
          items = eina_list_append(items, it->base.access_obj);
     }

   return elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   Elm_Gen_Item *it;

   ELM_GENGRID_DATA_GET(obj, sd);

   eo_do_super(obj, MY_CLASS, elm_interface_scrollable_mirrored_set(rtl));

   if (!sd->items) return;
   it = ELM_GEN_ITEM_FROM_INLIST(sd->items);

   while (it)
     {
        edje_object_mirrored_set(VIEW(it), rtl);
        elm_gengrid_item_update((Elm_Object_Item *)it);
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
     }
}

EOLIAN static Eina_Bool
_elm_gengrid_elm_widget_theme_apply(Eo *obj, Elm_Gengrid_Data *sd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;
   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   return EINA_TRUE;
}

static void
_item_position_update(Eina_Inlist *list,
                      int idx)
{
   Elm_Gen_Item *it;

   EINA_INLIST_FOREACH(list, it)
     {
        it->position = idx++;
        it->position_update = EINA_TRUE;
     }
}

static void
_elm_gengrid_item_del_not_serious(Elm_Gen_Item *it)
{
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   elm_widget_item_pre_notify_del(it);
   it->generation = sd->generation - 1; /* This means that the item is deleted */

   if (it->walking > 0) return;

   if (it->selected)
     sd->selected = eina_list_remove(sd->selected, it);
   if (sd->last_selected_item == (Elm_Object_Item *)it)
     sd->last_selected_item = NULL;
   if (sd->focused_item == (Elm_Object_Item *)it)
     sd->focused_item = NULL;
   if (sd->last_focused_item == (Elm_Object_Item *)it)
     sd->last_focused_item = NULL;
   if (sd->prev_focused_item == (Elm_Object_Item *)it)
     sd->prev_focused_item = NULL;

   if (it->itc->func.del)
     it->itc->func.del((void *)it->base.data, WIDGET(it));
}

static void
_elm_gengrid_item_del_serious(Elm_Gen_Item *it)
{
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   _elm_gengrid_item_del_not_serious(it);
   sd->items = eina_inlist_remove(sd->items, EINA_INLIST_GET(it));
   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   sd->walking -= it->walking;
   ELM_SAFE_FREE(it->long_timer, ecore_timer_del);
   if (it->group)
     sd->group_items = eina_list_remove(sd->group_items, it);

   ELM_SAFE_FREE(sd->state, eina_inlist_sorted_state_free);
   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(sd->calc_cb, sd->obj);

   sd->item_count--;

   ELM_SAFE_FREE(it->item, free);
}

static void
_item_del(Elm_Gen_Item *it)
{
   Evas_Object *obj = WIDGET(it);

   evas_event_freeze(evas_object_evas_get(obj));
   GG_IT(it)->wsd->selected = eina_list_remove(GG_IT(it)->wsd->selected, it);
   if (it->realized) _elm_gengrid_item_unrealize(it, EINA_FALSE);
   _elm_gengrid_item_del_serious(it);
   elm_gengrid_item_class_unref((Elm_Gengrid_Item_Class *)it->itc);
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_STOP, NULL);
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

static void
_scroll_page_change_cb(Evas_Object *obj,
                     void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_PAGE_CHANGE, NULL);
}

static void
_scroll_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL, NULL);
}

static int
_elm_gengrid_item_compare(const void *data,
                          const void *data1)
{
   Elm_Gen_Item *it, *item1;
   Eina_Compare_Cb cb = NULL;
   ptrdiff_t d;

   it = ELM_GEN_ITEM_FROM_INLIST(data);
   item1 = ELM_GEN_ITEM_FROM_INLIST(data1);
   if (it && GG_IT(it)->wsd->item_compare_cb)
     cb = GG_IT(it)->wsd->item_compare_cb;
   else if (item1 && GG_IT(item1)->wsd->item_compare_cb)
     cb = GG_IT(item1)->wsd->item_compare_cb;
   if (cb && it && item1) return cb(it, item1);
   d = (char *)data - (char *)data1;
   if (d < 0) return -1;
   if (!d) return 0;
   return 1;
}

static void
_item_disable_hook(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   if (it->generation < GG_IT(it)->wsd->generation) return;

   if (it->realized)
     {
        if (elm_widget_item_disabled_get(it))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,enabled", "elm");
     }
}

static void
_item_del_pre_hook(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   if (it->walking > 0)
     {
        _elm_gengrid_item_del_not_serious(it);
        return;
     }

   _item_del(it);
}

static Evas_Object *
_item_content_get_hook(Elm_Gen_Item *it,
                       const char *part)
{
   return edje_object_part_swallow_get(VIEW(it), part);
}

static const char *
_item_text_get_hook(Elm_Gen_Item *it,
                    const char *part)
{
   if (!it->itc->func.text_get) return NULL;
   return edje_object_part_text_get(VIEW(it), part);
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
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);
   Evas_Object *obj = WIDGET(it);
   ELM_GENGRID_DATA_GET(obj, sd);

   if (focused)
     {
        if (!elm_object_focus_get(obj))
          elm_object_focus_set(obj, EINA_TRUE);

        if (it != sd->focused_item)
          {
             if (sd->focused_item)
               _elm_gengrid_item_unfocused((Elm_Gen_Item *)sd->focused_item);
             _elm_gengrid_item_focused((Elm_Gen_Item *)it);
             _elm_widget_focus_highlight_start(obj);
          }
     }
   else
     _elm_gengrid_item_unfocused((Elm_Gen_Item *)it);
}

static Eina_Bool
_item_focus_get_hook(Elm_Object_Item *it)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Evas_Object *obj = WIDGET(it);
   ELM_GENGRID_DATA_GET(obj, sd);

   if (it == sd->focused_item)
     return EINA_TRUE;

   return EINA_FALSE;
}

static void
_internal_elm_gengrid_clear(Evas_Object *obj,
                   Eina_Bool standby)
{
   Eina_Inlist *next, *l;

   ELM_GENGRID_DATA_GET(obj, sd);
   if (!sd->items) return;

   if (!standby) sd->generation++;

   ELM_SAFE_FREE(sd->state, eina_inlist_sorted_state_free);

   if (sd->walking > 0)
     {
        sd->clear_me = EINA_TRUE;
        return;
     }
   evas_event_freeze(evas_object_evas_get(obj));
   for (l = sd->items, next = l ? l->next : NULL;
        l;
        l = next, next = next ? next->next : NULL)
     {
        Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(l);

        if (it->generation < sd->generation)
          {
             Elm_Gen_Item *itn = NULL;

             if (next) itn = ELM_GEN_ITEM_FROM_INLIST(next);
             if (itn) itn->walking++;  /* prevent early death of subitem */
             it->del_cb(it);
             elm_widget_item_free(it);
             if (itn) itn->walking--;
          }
     }
   sd->clear_me = EINA_FALSE;
   sd->pan_changed = EINA_TRUE;
   ELM_SAFE_FREE(sd->calc_job, ecore_job_del);
   sd->selected = eina_list_free(sd->selected);
   if (sd->clear_cb) sd->clear_cb(sd);
   sd->pan_x = 0;
   sd->pan_y = 0;
   sd->minw = 0;
   sd->minh = 0;

   if (sd->pan_obj)
     {
        evas_object_size_hint_min_set(sd->pan_obj, sd->minw, sd->minh);
        evas_object_smart_callback_call(sd->pan_obj, "changed", NULL);
     }
   eo_do(obj, elm_interface_scrollable_content_region_show(0, 0, 0, 0));
   evas_event_thaw(evas_object_evas_get(obj));
   evas_event_thaw_eval(evas_object_evas_get(obj));
}

static void
_item_select(Elm_Gen_Item *it)
{
   Evas_Object *obj = WIDGET(it);
   Elm_Gengrid_Data *sd = GG_IT(it)->wsd;

   if ((it->generation < sd->generation) || (it->decorate_it_set) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (sd->select_mode == ELM_OBJECT_SELECT_MODE_NONE))
     return;
   if (!it->selected)
     {
        it->selected = EINA_TRUE;
        sd->selected = eina_list_append(sd->selected, it);
     }
   else if (sd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS)
     return;

   evas_object_ref(obj);
   it->walking++;
   sd->walking++;
   if (it->func.func) it->func.func((void *)it->func.data, WIDGET(it), it);
   if (it->generation == sd->generation)
     {
        evas_object_smart_callback_call(WIDGET(it), SIG_SELECTED, it);
        elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);
     }

   it->walking--;
   sd->walking--;
   if ((sd->clear_me) && (!sd->walking))
     _internal_elm_gengrid_clear(WIDGET(it), EINA_TRUE);
   else
     {
        if ((!it->walking) && (it->generation < sd->generation))
          {
             it->del_cb(it);
             elm_widget_item_free(it);
          }
        else
          sd->last_selected_item = (Elm_Object_Item *)it;
     }
   evas_object_unref(obj);
}

static Elm_Gen_Item *
_elm_gengrid_item_new(Elm_Gengrid_Data *sd,
                      const Elm_Gen_Item_Class *itc,
                      const void *data,
                      Evas_Smart_Cb func,
                      const void *func_data)
{
   Elm_Gen_Item *it;

   if (!itc) return NULL;

   it = elm_widget_item_new(sd->obj, Elm_Gen_Item);
   if (!it) return NULL;

   it->generation = sd->generation;
   it->itc = itc;
   elm_gengrid_item_class_ref((Elm_Gengrid_Item_Class *)itc);

   it->base.data = data;
   it->parent = NULL;
   it->func.func = func;
   it->func.data = func_data;

   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);
   elm_widget_item_focus_set_hook_set(it, _item_focus_set_hook);
   elm_widget_item_focus_get_hook_set(it, _item_focus_get_hook);


   it->del_cb = (Ecore_Cb)_item_del;
   it->highlight_cb = (Ecore_Cb)_item_highlight;
   it->sel_cb = (Ecore_Cb)_item_select;
   it->unsel_cb = (Ecore_Cb)_item_unselect;
   it->unrealize_cb = (Ecore_Cb)_item_unrealize_cb;

   GG_IT(it) = ELM_NEW(Elm_Gen_Item_Type);
   GG_IT(it)->wsd = sd;

   it->group = it->itc->item_style &&
     (!strcmp(it->itc->item_style, "group_index"));
   sd->item_count++;

  return it;
}

/* common layout sizing won't apply here */
EOLIAN static void
_elm_gengrid_elm_layout_sizing_eval(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *_pd EINA_UNUSED)
{
   return;  /* no-op */
}

EOLIAN static void
_elm_gengrid_evas_smart_add(Eo *obj, Elm_Gengrid_Data *priv)
{
   Eina_Bool bounce = _elm_config->thumbscroll_bounce_enable;
   Elm_Gengrid_Pan_Data *pan_data;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->calc_cb = (Ecore_Cb)_calc_job;

   priv->generation = 1;

   if (!elm_layout_theme_set(obj, "gengrid", "base",
                             elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   eo_do(obj, elm_interface_scrollable_objects_set(wd->resize_obj, priv->hit_rect));

   priv->old_h_bounce = bounce;
   priv->old_v_bounce = bounce;

   eo_do(obj, elm_interface_scrollable_bounce_allow_set(bounce, bounce));

   eo_do(obj,
         elm_interface_scrollable_animate_start_cb_set
         (_scroll_animate_start_cb),
         elm_interface_scrollable_animate_stop_cb_set(_scroll_animate_stop_cb),
         elm_interface_scrollable_drag_start_cb_set(_scroll_drag_start_cb),
         elm_interface_scrollable_drag_stop_cb_set(_scroll_drag_stop_cb),
         elm_interface_scrollable_edge_left_cb_set(_edge_left_cb),
         elm_interface_scrollable_edge_right_cb_set(_edge_right_cb),
         elm_interface_scrollable_edge_top_cb_set(_edge_top_cb),
         elm_interface_scrollable_edge_bottom_cb_set(_edge_bottom_cb),
         elm_interface_scrollable_scroll_cb_set(_scroll_cb),
         elm_interface_scrollable_page_change_cb_set(_scroll_page_change_cb));

   priv->align_x = 0.5;
   priv->align_y = 0.5;
   priv->highlight = EINA_TRUE;

   priv->pan_obj = eo_add(MY_PAN_CLASS, evas_object_evas_get(obj));
   pan_data = eo_data_scope_get(priv->pan_obj, MY_PAN_CLASS);
   eo_data_ref(obj, NULL);
   pan_data->wobj = obj;
   pan_data->wsd = priv;

   priv->stack = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->stack, priv->pan_obj);
   evas_object_raise(priv->stack);

   eo_do(obj, elm_interface_scrollable_extern_pan_set(priv->pan_obj));
}

EOLIAN static void
_elm_gengrid_evas_smart_del(Eo *obj, Elm_Gengrid_Data *sd)
{
   elm_gengrid_clear(obj);
   eo_unref(sd->pan_obj);
   ELM_SAFE_FREE(sd->pan_obj, evas_object_del);
   ELM_SAFE_FREE(sd->stack, evas_object_del);

   ecore_job_del(sd->calc_job);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EOLIAN static void
_elm_gengrid_evas_smart_move(Eo *obj, Elm_Gengrid_Data *sd, Evas_Coord x, Evas_Coord y)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   evas_object_move(sd->hit_rect, x, y);
}

EOLIAN static void
_elm_gengrid_evas_smart_resize(Eo *obj, Elm_Gengrid_Data *sd, Evas_Coord w, Evas_Coord h)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   evas_object_resize(sd->hit_rect, w, h);
}

EOLIAN static void
_elm_gengrid_evas_smart_member_add(Eo *obj, Elm_Gengrid_Data *sd, Evas_Object *member)
{
   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_add(member));

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_access_obj_process(Elm_Gengrid_Data * sd, Eina_Bool is_access)
{
   Elm_Gen_Item *it;

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (!it->realized) continue;
        if (is_access) _access_widget_item_register(it);
        else
          _elm_access_widget_item_unregister((Elm_Widget_Item *)it);

     }
}

EOLIAN static void
_elm_gengrid_elm_widget_access(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Eina_Bool acs)
{
   _elm_gengrid_smart_focus_next_enable = acs;
   _access_obj_process(sd, _elm_gengrid_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_gengrid_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_gengrid_eo_base_constructor(Eo *obj, Elm_Gengrid_Data *sd)
{
   sd->obj = obj;

   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks));
}

EOLIAN static void
_elm_gengrid_item_size_set(Eo *obj, Elm_Gengrid_Data *sd, Evas_Coord w, Evas_Coord h)
{
   if ((sd->item_width == w) && (sd->item_height == h)) return;
   sd->item_width = w;
   sd->item_height = h;
   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, obj);
}

EOLIAN static void
_elm_gengrid_item_size_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = sd->item_width;
   if (h) *h = sd->item_height;
}

EOLIAN static void
_elm_gengrid_group_item_size_set(Eo *obj, Elm_Gengrid_Data *sd, Evas_Coord w, Evas_Coord h)
{
   if ((sd->group_item_width == w) && (sd->group_item_height == h)) return;
   sd->group_item_width = w;
   sd->group_item_height = h;
   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, obj);
}

EOLIAN static void
_elm_gengrid_group_item_size_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Evas_Coord *w, Evas_Coord *h)
{
   if (w) *w = sd->group_item_width;
   if (h) *h = sd->group_item_height;
}

EOLIAN static void
_elm_gengrid_align_set(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, double align_x, double align_y)
{
   double old_h, old_y;

   old_h = sd->align_x;
   old_y = sd->align_y;

   if (align_x > 1.0)
     align_x = 1.0;
   else if (align_x < 0.0)
     align_x = 0.0;
   sd->align_x = align_x;

   if (align_y > 1.0)
     align_y = 1.0;
   else if (align_y < 0.0)
     align_y = 0.0;
   sd->align_y = align_y;

   if ((old_h != sd->align_x) || (old_y != sd->align_y))
     evas_object_smart_calculate(sd->pan_obj);
}

EOLIAN static void
_elm_gengrid_align_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, double *align_x, double *align_y)
{
   if (align_x) *align_x = sd->align_x;
   if (align_y) *align_y = sd->align_y;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_item_append(Eo *obj, Elm_Gengrid_Data *sd, const Elm_Gengrid_Item_Class *itc, const void *data, Evas_Smart_Cb func, const void *func_data)
{
   Elm_Gen_Item *it;

   it = _elm_gengrid_item_new(sd, itc, data, func, func_data);
   if (!it) return NULL;

   sd->items = eina_inlist_append(sd->items, EINA_INLIST_GET(it));
   it->position = sd->item_count;
   it->position_update = EINA_TRUE;

   if (it->group)
     sd->group_items = eina_list_prepend(sd->group_items, it);

   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, obj);

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_item_prepend(Eo *obj, Elm_Gengrid_Data *sd, const Elm_Gengrid_Item_Class *itc, const void *data, Evas_Smart_Cb func, const void *func_data)
{
   Elm_Gen_Item *it;

   it = _elm_gengrid_item_new(sd, itc, data, func, func_data);
   if (!it) return NULL;

   sd->items = eina_inlist_prepend(sd->items, EINA_INLIST_GET(it));
   _item_position_update(sd->items, 0);

   if (it->group)
     sd->group_items = eina_list_append(sd->group_items, it);

   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, obj);

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_item_insert_before(Eo *obj, Elm_Gengrid_Data *sd, const Elm_Gengrid_Item_Class *itc, const void *data, Elm_Object_Item *relative, Evas_Smart_Cb func, const void *func_data)
{
   Elm_Gen_Item *it;
   Eina_Inlist *tmp;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(relative, NULL);

   it = _elm_gengrid_item_new(sd, itc, data, func, func_data);
   if (!it) return NULL;
   sd->items = eina_inlist_prepend_relative
       (sd->items, EINA_INLIST_GET(it),
       EINA_INLIST_GET((Elm_Gen_Item *)relative));
   tmp = eina_inlist_find(sd->items, EINA_INLIST_GET(it));
   _item_position_update(tmp, ((Elm_Gen_Item *)relative)->position);

   if (it->group)
     sd->group_items = eina_list_append_relative
         (sd->group_items, it, ((Elm_Gen_Item *)relative)->parent);

   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, obj);

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_item_insert_after(Eo *obj, Elm_Gengrid_Data *sd, const Elm_Gengrid_Item_Class *itc, const void *data, Elm_Object_Item *relative, Evas_Smart_Cb func, const void *func_data)
{
   Elm_Gen_Item *it;
   Eina_Inlist *tmp;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(relative, NULL);

   it = _elm_gengrid_item_new(sd, itc, data, func, func_data);
   if (!it) return NULL;
   sd->items = eina_inlist_append_relative
       (sd->items, EINA_INLIST_GET(it),
       EINA_INLIST_GET((Elm_Gen_Item *)relative));
   tmp = eina_inlist_find(sd->items, EINA_INLIST_GET(it));
   _item_position_update(tmp, ((Elm_Gen_Item *)relative)->position + 1);

   if (it->group)
     sd->group_items = eina_list_prepend_relative
         (sd->group_items, it, ((Elm_Gen_Item *)relative)->parent);

   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, obj);

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_item_sorted_insert(Eo *obj, Elm_Gengrid_Data *sd, const Elm_Gengrid_Item_Class *itc, const void *data, Eina_Compare_Cb comp, Evas_Smart_Cb func, const void *func_data)
{
   Elm_Gen_Item *it;

   it = _elm_gengrid_item_new(sd, itc, data, func, func_data);
   if (!it) return NULL;

   if (!sd->state)
     {
        sd->state = eina_inlist_sorted_state_new();
        eina_inlist_sorted_state_init(sd->state, sd->items);
     }

   sd->item_compare_cb = comp;
   sd->items = eina_inlist_sorted_state_insert
       (sd->items, EINA_INLIST_GET(it), _elm_gengrid_item_compare, sd->state);
   _item_position_update(sd->items, 0);

   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, obj);

   return (Elm_Object_Item *)it;
}

EOLIAN static void
_elm_gengrid_horizontal_set(Eo *obj, Elm_Gengrid_Data *sd, Eina_Bool horizontal)
{
   horizontal = !!horizontal;
   if (horizontal == sd->horizontal) return;
   sd->horizontal = horizontal;

   /* Update the items to conform to the new layout */
   ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, obj);
}

EOLIAN static Eina_Bool
_elm_gengrid_horizontal_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->horizontal;
}

EOLIAN static void
_elm_gengrid_clear(Eo *obj, Elm_Gengrid_Data *_pd EINA_UNUSED)
{
   _internal_elm_gengrid_clear(obj, EINA_FALSE);
}

EINA_DEPRECATED EAPI const Evas_Object *
elm_gengrid_item_object_get(const Elm_Object_Item *it)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, NULL);

   return VIEW(it);
}

EAPI void
elm_gengrid_item_update(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   if (!it->realized) return;
   if (it->want_unrealize) return;

   _elm_gengrid_item_unrealize(it, EINA_TRUE);
   _item_realize(it);
   _item_place(it, it->x, it->y);
}

EAPI const Elm_Gengrid_Item_Class *
elm_gengrid_item_item_class_get(const Elm_Object_Item *it)
{
   Elm_Gen_Item *item = (Elm_Gen_Item *)it;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, NULL);

   if (item->generation < GG_IT(item)->wsd->generation) return NULL;
   return item->itc;
}

EAPI void
elm_gengrid_item_item_class_update(Elm_Object_Item *item,
                                   const Elm_Gengrid_Item_Class *itc)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);
   EINA_SAFETY_ON_NULL_RETURN(itc);

   if (it->generation < GG_IT(it)->wsd->generation) return;
   it->itc = itc;
   elm_gengrid_item_update(item);
}

EAPI void
elm_gengrid_item_pos_get(const Elm_Object_Item *it,
                         unsigned int *x,
                         unsigned int *y)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   if (x) *x = ((Elm_Gen_Item *)it)->x;
   if (y) *y = ((Elm_Gen_Item *)it)->y;
}

EOLIAN static void
_elm_gengrid_multi_select_set(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Eina_Bool multi)
{
   sd->multi = !!multi;
}

EOLIAN static Eina_Bool
_elm_gengrid_multi_select_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->multi;
}

EOLIAN static void
_elm_gengrid_multi_select_mode_set(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Elm_Object_Multi_Select_Mode mode)
{
   if (mode >= ELM_OBJECT_MULTI_SELECT_MODE_MAX)
     return;

   if (sd->multi_select_mode != mode)
     sd->multi_select_mode = mode;
}

EOLIAN static Elm_Object_Multi_Select_Mode
_elm_gengrid_multi_select_mode_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->multi_select_mode;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_selected_item_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   if (sd->selected) return sd->selected->data;
   return NULL;
}

EOLIAN static const Eina_List*
_elm_gengrid_selected_items_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->selected;
}

EAPI void
elm_gengrid_item_selected_set(Elm_Object_Item *item,
                              Eina_Bool selected)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   Elm_Gengrid_Data *sd;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   sd = GG_IT(it)->wsd;
   if ((it->generation < sd->generation) || elm_widget_item_disabled_get(it))
     return;
   selected = !!selected;
   if (it->selected == selected) return;

   if (selected)
     {
        if (!sd->multi)
          {
             while (sd->selected)
               {
                  if (it->unhighlight_cb)
                    it->unhighlight_cb(sd->selected->data);
                  it->unsel_cb(sd->selected->data);
               }
          }
        it->highlight_cb(it);
        it->sel_cb(it);
        return;
     }
   if (it->unhighlight_cb) it->unhighlight_cb(it);
   it->unsel_cb(it);
}

EAPI Eina_Bool
elm_gengrid_item_selected_get(const Elm_Object_Item *it)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_Gen_Item *)it)->selected;
}

EOLIAN static Eina_List*
_elm_gengrid_realized_items_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   Elm_Gen_Item *it;

   Eina_List *ret = NULL;

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->realized) ret = eina_list_append(ret, (Elm_Object_Item *)it);
     }

   return ret;
}

EOLIAN static void
_elm_gengrid_realized_items_update(Eo *obj, Elm_Gengrid_Data *_pd EINA_UNUSED)
{
   Eina_List *list, *l;
   Elm_Object_Item *it;

   list = elm_gengrid_realized_items_get(obj);
   EINA_LIST_FOREACH(list, l, it)
     elm_gengrid_item_update(it);
}

static Evas_Object *
_elm_gengrid_item_label_create(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               Evas_Object *tooltip,
                               void *it   EINA_UNUSED)
{
   Evas_Object *label = elm_label_add(tooltip);
   if (!label)
     return NULL;
   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);
   return label;
}

static void
_elm_gengrid_item_label_del_cb(void *data,
                               Evas_Object *obj EINA_UNUSED,
                               void *event_info EINA_UNUSED)
{
   eina_stringshare_del(data);
}

EAPI void
elm_gengrid_item_tooltip_text_set(Elm_Object_Item *it,
                                  const char *text)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   text = eina_stringshare_add(text);
   elm_gengrid_item_tooltip_content_cb_set
     (it, _elm_gengrid_item_label_create, text,
     _elm_gengrid_item_label_del_cb);
}

EAPI void
elm_gengrid_item_tooltip_content_cb_set(Elm_Object_Item *item,
                                        Elm_Tooltip_Item_Content_Cb func,
                                        const void *data,
                                        Evas_Smart_Cb del_cb)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_GOTO(it, error);

   if ((it->tooltip.content_cb == func) && (it->tooltip.data == data))
     return;

   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->tooltip.content_cb = func;
   it->tooltip.data = data;
   it->tooltip.del_cb = del_cb;
   if (VIEW(it))
     {
        elm_widget_item_tooltip_content_cb_set
          (it, it->tooltip.content_cb, it->tooltip.data, NULL);
        elm_widget_item_tooltip_style_set(it, it->tooltip.style);
        elm_widget_item_tooltip_window_mode_set(it, it->tooltip.free_size);
     }

   return;

error:
   if (del_cb) del_cb((void *)data, NULL, NULL);
}

EAPI void
elm_gengrid_item_tooltip_unset(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   if ((VIEW(it)) && (it->tooltip.content_cb))
     elm_widget_item_tooltip_unset(it);

   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   it->tooltip.del_cb = NULL;
   it->tooltip.content_cb = NULL;
   it->tooltip.data = NULL;
   it->tooltip.free_size = EINA_FALSE;
   if (it->tooltip.style)
     elm_gengrid_item_tooltip_style_set(item, NULL);
}

EAPI void
elm_gengrid_item_tooltip_style_set(Elm_Object_Item *it,
                                   const char *style)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   eina_stringshare_replace(&((Elm_Gen_Item *)it)->tooltip.style, style);
   if (VIEW(it)) elm_widget_item_tooltip_style_set(it, style);
}

EAPI const char *
elm_gengrid_item_tooltip_style_get(const Elm_Object_Item *it)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, NULL);

   return ((Elm_Gen_Item *)it)->tooltip.style;
}

EAPI Eina_Bool
elm_gengrid_item_tooltip_window_mode_set(Elm_Object_Item *it,
                                         Eina_Bool disable)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   ((Elm_Gen_Item *)it)->tooltip.free_size = disable;
   if (VIEW(it)) return elm_widget_item_tooltip_window_mode_set(it, disable);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_gengrid_item_tooltip_window_mode_get(const Elm_Object_Item *it)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_Gen_Item *)it)->tooltip.free_size;
}

EAPI void
elm_gengrid_item_cursor_set(Elm_Object_Item *it,
                            const char *cursor)
{
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   eina_stringshare_replace(&((Elm_Gen_Item *)it)->mouse_cursor, cursor);
   if (VIEW(it)) elm_widget_item_cursor_set(it, cursor);
}

EAPI const char *
elm_gengrid_item_cursor_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_get(it);
}

EAPI void
elm_gengrid_item_cursor_unset(Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);
   if (!it->mouse_cursor)
     return;

   if (VIEW(it))
     elm_widget_item_cursor_unset(it);

   ELM_SAFE_FREE(it->mouse_cursor, eina_stringshare_del);
}

EAPI void
elm_gengrid_item_cursor_style_set(Elm_Object_Item *it,
                                  const char *style)
{
   elm_widget_item_cursor_style_set(it, style);
}

EAPI const char *
elm_gengrid_item_cursor_style_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_style_get(it);
}

EAPI void
elm_gengrid_item_cursor_engine_only_set(Elm_Object_Item *it,
                                        Eina_Bool engine_only)
{
   elm_widget_item_cursor_engine_only_set(it, engine_only);
}

EAPI Eina_Bool
elm_gengrid_item_cursor_engine_only_get(const Elm_Object_Item *it)
{
   return elm_widget_item_cursor_engine_only_get(it);
}

EAPI void
elm_gengrid_reorder_mode_start(Evas_Object *obj, Ecore_Pos_Map tween_mode)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->reorder_mode = EINA_TRUE;
   sd->reorder.tween_mode = tween_mode;
}

EAPI void
elm_gengrid_reorder_mode_stop(Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->reorder_mode = EINA_FALSE;
   sd->reorder.tween_mode = -1;
}

EOLIAN static void
_elm_gengrid_reorder_mode_set(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Eina_Bool reorder_mode)
{
   sd->reorder_mode = !!reorder_mode;
}

EOLIAN static Eina_Bool
_elm_gengrid_reorder_mode_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->reorder_mode;
}

EAPI void
elm_gengrid_bounce_set(Evas_Object *obj,
                       Eina_Bool h_bounce,
                       Eina_Bool v_bounce)
{
   ELM_GENGRID_CHECK(obj);
   eo_do(obj, elm_interface_scrollable_bounce_allow_set(h_bounce, v_bounce));
}

EOLIAN static void
_elm_gengrid_elm_interface_scrollable_bounce_allow_set(Eo *obj, Elm_Gengrid_Data *sd EINA_UNUSED, Eina_Bool h_bounce, Eina_Bool v_bounce)
{
   eo_do_super(obj, MY_CLASS, elm_interface_scrollable_bounce_allow_set(!!h_bounce, !!v_bounce));
}

EAPI void
elm_gengrid_bounce_get(const Evas_Object *obj,
                       Eina_Bool *h_bounce,
                       Eina_Bool *v_bounce)
{
   ELM_GENGRID_CHECK(obj);
   eo_do((Eo *) obj, elm_interface_scrollable_bounce_allow_get(h_bounce, v_bounce));
}

EOLIAN static void
_elm_gengrid_page_relative_set(Eo *obj, Elm_Gengrid_Data *_pd EINA_UNUSED, double h_pagerel, double v_pagerel)
{
   Evas_Coord pagesize_h;
   Evas_Coord pagesize_v;

   eo_do((Eo *) obj, elm_interface_scrollable_paging_get(NULL, NULL, &pagesize_h, &pagesize_v));
   eo_do((Eo *) obj, elm_interface_scrollable_paging_set
     (h_pagerel, v_pagerel, pagesize_h, pagesize_v));
}

EOLIAN static void
_elm_gengrid_page_relative_get(Eo *obj, Elm_Gengrid_Data *_pd EINA_UNUSED, double *h_pagerel, double *v_pagerel)
{
   eo_do((Eo *) obj, elm_interface_scrollable_paging_get(h_pagerel, v_pagerel, NULL, NULL));
}

EOLIAN static void
_elm_gengrid_page_size_set(Eo *obj, Elm_Gengrid_Data *_pd EINA_UNUSED, Evas_Coord h_pagesize, Evas_Coord v_pagesize)
{
   double pagerel_h;
   double pagerel_v;

   eo_do((Eo *) obj, elm_interface_scrollable_paging_get(&pagerel_h, &pagerel_v, NULL, NULL));
   eo_do((Eo *) obj, elm_interface_scrollable_paging_set
     (pagerel_h, pagerel_v, h_pagesize, v_pagesize));
}

EAPI void
elm_gengrid_current_page_get(const Evas_Object *obj,
                             int *h_pagenumber,
                             int *v_pagenumber)
{
   ELM_GENGRID_CHECK(obj);
   eo_do((Eo *) obj, elm_interface_scrollable_current_page_get(h_pagenumber, v_pagenumber));
}

EAPI void
elm_gengrid_last_page_get(const Evas_Object *obj,
                          int *h_pagenumber,
                          int *v_pagenumber)
{
   ELM_GENGRID_CHECK(obj);
   eo_do((Eo *) obj, elm_interface_scrollable_last_page_get(h_pagenumber, v_pagenumber));
}

EAPI void
elm_gengrid_page_show(const Evas_Object *obj,
                      int h_pagenumber,
                      int v_pagenumber)
{
   ELM_GENGRID_CHECK(obj);
   eo_do((Eo *) obj, elm_interface_scrollable_page_show(h_pagenumber, v_pagenumber));
}

EAPI void
elm_gengrid_page_bring_in(const Evas_Object *obj,
                          int h_pagenumber,
                          int v_pagenumber)
{
   ELM_GENGRID_CHECK(obj);
   eo_do((Eo *) obj, elm_interface_scrollable_page_bring_in(h_pagenumber, v_pagenumber));
}

EAPI void
elm_gengrid_scroller_policy_set(Evas_Object *obj,
                                Elm_Scroller_Policy policy_h,
                                Elm_Scroller_Policy policy_v)
{
   ELM_GENGRID_CHECK(obj);
   eo_do(obj, elm_interface_scrollable_policy_set(policy_h, policy_v));
}

EOLIAN static void
_elm_gengrid_elm_interface_scrollable_policy_set(Eo *obj, Elm_Gengrid_Data *sd EINA_UNUSED, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   eo_do_super(obj, MY_CLASS, elm_interface_scrollable_policy_set(policy_h, policy_v));
}

EAPI void
elm_gengrid_scroller_policy_get(const Evas_Object *obj,
                                Elm_Scroller_Policy *policy_h,
                                Elm_Scroller_Policy *policy_v)
{
   ELM_GENGRID_CHECK(obj);
   eo_do((Eo *) obj, elm_interface_scrollable_policy_get(policy_h, policy_v));
}

EOLIAN static void
_elm_gengrid_elm_interface_scrollable_policy_get(Eo *obj, Elm_Gengrid_Data *sd EINA_UNUSED, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v)
{
   Elm_Scroller_Policy s_policy_h, s_policy_v;

   eo_do_super((Eo *)obj, MY_CLASS, elm_interface_scrollable_policy_get(&s_policy_h, &s_policy_v));
   if (policy_h) *policy_h = (Elm_Scroller_Policy)s_policy_h;
   if (policy_v) *policy_v = (Elm_Scroller_Policy)s_policy_v;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_first_item_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   if (!sd->items) return NULL;

   Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(sd->items);

   while ((it) && (it->generation < sd->generation))
     it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);

   return (Elm_Object_Item *)it;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_at_xy_item_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Evas_Coord x, Evas_Coord y, int *xposret, int *yposret)
{
   Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(sd->items);

   Evas_Coord l = 0, r = 0, t = 0, b = 0; /* left, right, top, bottom */
   Eina_Bool init = EINA_TRUE;

   while ((it) && (it->generation < sd->generation))
     it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);

   if (it)
     do
       {
          Evas_Coord itx, ity;
          Evas_Coord itw, ith;
          evas_object_geometry_get(VIEW(it), &itx, &ity, &itw, &ith);

          /* Record leftmost, rightmost, top, bottom cords to set posret */
          if ((itw > 0) && (ith > 0) && (itx >= 0) && (ity >= 0))
            {  /* A scroller, ignore items in negative cords,or not rendered */
               if (init)
                 {
                    l = itx;
                    r = itx + itw;
                    t = ity;
                    b = ity + ith;
                    init = EINA_FALSE;
                 }
               else
                 {
                    if (itx < l)
                      l = itx;
                    if ((itx + itw) > r)
                      r = itx + itw;
                    if (ity < t)
                      t = ity;
                    if ((ity + ith) > b)
                      b = ity + ith;
                 }
            }

          if (ELM_RECTS_INTERSECT
                (itx, ity, itw, ith, x, y, 1, 1))
            {
               if (yposret)
                 {
                    if (y <= (ity + (ith / 4))) *yposret = -1;
                    else if (y >= (ity + ith - (ith / 4)))
                      *yposret = 1;
                    else *yposret = 0;
                 }

               if (xposret)
                 {
                    if (x <= (itx + (itw / 4))) *xposret = -1;
                    else if (x >= (itx + itw - (itw / 4)))
                      *xposret = 1;
                    else *xposret = 0;
                 }

               return (Elm_Object_Item *) it;
            }

       } while ((it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next)));

   /* No item found, tell the user if hit left/right/top/bottom of items */
   if (xposret)
     {
        *xposret = 0;
        if (x < l)
          *xposret = (-1);
        else if (x > r)
          *xposret = (1);
     }

   if (yposret)
     {
        *yposret = 0;
        if (y < t)
          *yposret = (-1);
        else if (y > b)
          *yposret = (1);
     }

   return NULL;
}

EOLIAN static Elm_Object_Item*
_elm_gengrid_last_item_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   if (!sd->items) return NULL;

   Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(sd->items->last);
   while ((it) && (it->generation < sd->generation))
     it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_gengrid_item_next_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, NULL);

   while (it)
     {
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        if ((it) && (it->generation == GG_IT(it)->wsd->generation)) break;
     }

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_gengrid_item_prev_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, NULL);

   while (it)
     {
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
        if ((it) && (it->generation == GG_IT(it)->wsd->generation)) break;
     }

   return (Elm_Object_Item *)it;
}

EAPI void
elm_gengrid_item_show(Elm_Object_Item *item,
                      Elm_Gengrid_Item_Scrollto_Type type)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   Elm_Gengrid_Data *sd;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);
   sd = GG_IT(it)->wsd;

   if ((it->generation < sd->generation)) return;

   sd->show_region = EINA_TRUE;
   sd->show_it = it;
   sd->scroll_to_type = type;

   _item_show_region(sd);
}

EAPI void
elm_gengrid_item_bring_in(Elm_Object_Item *item,
                          Elm_Gengrid_Item_Scrollto_Type type)
{
   Elm_Gengrid_Data *sd;
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);
   sd = GG_IT(it)->wsd;

   if (it->generation < sd->generation) return;

   sd->bring_in = EINA_TRUE;
   sd->bring_in_it = it;
   sd->scroll_to_type = type;

   _item_show_region(sd);
}

EOLIAN static void
_elm_gengrid_filled_set(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Eina_Bool fill)
{
   fill = !!fill;
   if (sd->filled != fill)
     sd->filled = fill;
}

EOLIAN static Eina_Bool
_elm_gengrid_filled_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->filled;
}

EOLIAN static unsigned int
_elm_gengrid_items_count(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->item_count;
}

EAPI Elm_Gengrid_Item_Class *
elm_gengrid_item_class_new(void)
{
   Elm_Gengrid_Item_Class *itc;

   itc = calloc(1, sizeof(Elm_Gengrid_Item_Class));
   if (!itc)
     return NULL;
   itc->version = CLASS_ALLOCATED;
   itc->refcount = 1;
   itc->delete_me = EINA_FALSE;

   return itc;
}

EAPI void
elm_gengrid_item_class_free(Elm_Gengrid_Item_Class *itc)
{
   if (!itc || (itc->version != CLASS_ALLOCATED)) return;

   if (!itc->delete_me) itc->delete_me = EINA_TRUE;
   if (itc->refcount > 0) elm_gengrid_item_class_unref(itc);
   else
     {
        itc->version = 0;
        free(itc);
     }
}

EAPI void
elm_gengrid_item_class_ref(Elm_Gengrid_Item_Class *itc)
{
   if (itc && (itc->version == CLASS_ALLOCATED))
     {
        itc->refcount++;
        if (itc->refcount == 0) itc->refcount--;
     }
}

EAPI void
elm_gengrid_item_class_unref(Elm_Gengrid_Item_Class *itc)
{
   if (itc && (itc->version == CLASS_ALLOCATED))
     {
        if (itc->refcount > 0) itc->refcount--;
        if (itc->delete_me && (!itc->refcount))
          elm_gengrid_item_class_free(itc);
     }
}

EOLIAN static void
_elm_gengrid_select_mode_set(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Elm_Object_Select_Mode mode)
{
   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;

   if (sd->select_mode != mode)
     sd->select_mode = mode;
}

EOLIAN static Elm_Object_Select_Mode
_elm_gengrid_select_mode_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->select_mode;
}

EOLIAN static void
_elm_gengrid_highlight_mode_set(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd, Eina_Bool highlight)
{
   sd->highlight = !!highlight;
}

EOLIAN static Eina_Bool
_elm_gengrid_highlight_mode_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->highlight;
}

EAPI int
elm_gengrid_item_index_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, -1);

   return it->position;
}

EAPI void
elm_gengrid_item_select_mode_set(Elm_Object_Item *item,
                                 Elm_Object_Select_Mode mode)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   if (it->generation < GG_IT(it)->wsd->generation) return;
   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;

   if (it->select_mode != mode)
     it->select_mode = mode;

   if (it->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
     elm_gengrid_item_update(item);
}

EAPI Elm_Object_Select_Mode
elm_gengrid_item_select_mode_get(const Elm_Object_Item *item)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it, ELM_OBJECT_SELECT_MODE_MAX);

   return it->select_mode;
}

EAPI Elm_Object_Item *
elm_gengrid_nth_item_get(const Evas_Object *obj, unsigned int nth)
{
   Elm_Gen_Item *it = NULL;
   Eina_Accessor *a;
   void *data;

   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET(obj, sd);

   if (!sd->items) return NULL;

   a = eina_inlist_accessor_new(sd->items);
   if (!a) return NULL;
   if (eina_accessor_data_get(a, nth, &data))
     it = ELM_GEN_ITEM_FROM_INLIST(data);
   eina_accessor_free(a);
   return (Elm_Object_Item *)it;
}

EOLIAN static void
_elm_gengrid_elm_widget_focus_highlight_geometry_get(Eo *obj, Elm_Gengrid_Data *sd, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Coord ox, oy, oh, ow, item_x = 0, item_y = 0, item_w = 0, item_h = 0;

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);

   if (sd->focused_item)
     {
        evas_object_geometry_get(VIEW(sd->focused_item), &item_x, &item_y, &item_w, &item_h);
        elm_widget_focus_highlight_focus_part_geometry_get(VIEW(sd->focused_item), &item_x, &item_y, &item_w, &item_h);
     }

   *x = item_x;
   *y = item_y;
   *w = item_w;
   *h = item_h;

   if (sd->horizontal)
     {
        if (item_x < ox)
          {
             *x = ox;
          }
        else if (item_x > (ox + ow - item_w))
          {
             *x = ox + ow - item_w;
          }
     }
   else
     {
        if (item_y < oy)
          {
             *y = oy;
          }
        else if (item_y > (oy + oh - item_h))
          {
             *y = oy + oh - item_h;
          }
     }
}

EOLIAN static Elm_Object_Item *
_elm_gengrid_elm_widget_focused_item_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->focused_item;
}

EOLIAN static void
_elm_gengrid_wheel_disabled_set(Eo *obj, Elm_Gengrid_Data *sd, Eina_Bool disabled)
{
   disabled = !!disabled;
   if (sd->wheel_disabled != disabled)
     eo_do(obj,
           elm_interface_scrollable_wheel_disabled_set(disabled));

   sd->wheel_disabled = disabled;
}

EOLIAN static Eina_Bool
_elm_gengrid_wheel_disabled_get(Eo *obj EINA_UNUSED, Elm_Gengrid_Data *sd)
{
   return sd->wheel_disabled;
}

EOLIAN static void
_elm_gengrid_class_constructor(Eo_Class *klass)
{
   if (_elm_config->access_mode)
      _elm_gengrid_smart_focus_next_enable = EINA_TRUE;

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_gengrid.eo.c"
