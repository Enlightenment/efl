#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "elm_widget_gengrid.h"

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

static const Evas_Smart_Interface *_smart_interfaces[] =
{
   (Evas_Smart_Interface *)&ELM_SCROLLABLE_IFACE, NULL
};

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
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_MOVED[] = "moved";
static const char SIG_INDEX_UPDATE[] = "index,update";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
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
   {SIG_EDGE_TOP, ""},
   {SIG_EDGE_BOTTOM, ""},
   {SIG_EDGE_LEFT, ""},
   {SIG_EDGE_RIGHT, ""},
   {SIG_MOVED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_IFACE_NEW
  (ELM_GENGRID_SMART_NAME, _elm_gengrid, Elm_Gengrid_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks,
  _smart_interfaces);

ELM_INTERNAL_SMART_SUBCLASS_NEW
  (ELM_GENGRID_PAN_SMART_NAME, _elm_gengrid_pan, Elm_Gengrid_Pan_Smart_Class,
  Elm_Pan_Smart_Class, elm_pan_smart_class_get, NULL);

static void
_calc_job(void *data)
{
   Elm_Gengrid_Smart_Data *sd = data;
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

        EINA_INLIST_FOREACH (sd->items, it)
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
     }
   sd->calc_job = NULL;
}

static void
_elm_gengrid_pan_smart_add(Evas_Object *obj)
{
   /* here just to allocate our extended data */
   EVAS_SMART_DATA_ALLOC(obj, Elm_Gengrid_Pan_Smart_Data);

   ELM_PAN_CLASS(_elm_gengrid_pan_parent_sc)->base.add(obj);
}

static void
_elm_gengrid_pan_smart_move(Evas_Object *obj,
                            Evas_Coord x __UNUSED__,
                            Evas_Coord y __UNUSED__)
{
   ELM_GENGRID_PAN_DATA_GET(obj, psd);

   if (psd->wsd->calc_job) ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job, psd->wsd);
}

static void
_elm_gengrid_pan_smart_resize(Evas_Object *obj,
                              Evas_Coord w,
                              Evas_Coord h)
{
   Evas_Coord ow, oh;

   ELM_GENGRID_PAN_DATA_GET(obj, psd);

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   if (psd->wsd->calc_job) ecore_job_del(psd->wsd->calc_job);
   psd->wsd->calc_job = ecore_job_add(_calc_job, psd->wsd);
}

static void
_item_unselect(Elm_Gen_Item *it)
{
   Elm_Gen_Item_Type *item = GG_IT(it);

   if ((it->generation < item->wsd->generation) || (!it->highlighted))
     return;

   edje_object_signal_emit(VIEW(it), "elm,state,unselected", "elm");
   it->highlighted = EINA_FALSE;
   if (it->selected)
     {
        it->selected = EINA_FALSE;
        item->wsd->selected = eina_list_remove(item->wsd->selected, it);
        evas_object_smart_callback_call(WIDGET(it), SIG_UNSELECTED, it);
     }
}

static void
_item_mouse_move_cb(void *data,
                    Evas *evas __UNUSED__,
                    Evas_Object *obj,
                    void *event_info)
{
   Elm_Gen_Item *it = data;
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord ox, oy, ow, oh, it_scrl_x, it_scrl_y;
   Evas_Coord minw = 0, minh = 0, x, y, dx, dy, adx, ady;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     {
        if (!GG_IT(it)->wsd->on_hold)
          {
             GG_IT(it)->wsd->on_hold = EINA_TRUE;
             if (!GG_IT(it)->wsd->was_selected)
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
        evas_object_smart_callback_call(WIDGET(it), SIG_DRAG, it);
        return;
     }

   if ((!it->down) || (GG_IT(it)->wsd->longpressed))
     {
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if ((GG_IT(it)->wsd->reorder_mode) &&
            (GG_IT(it)->wsd->reorder_it))
          {
             evas_object_geometry_get
               (GG_IT(it)->wsd->pan_obj, &ox, &oy, &ow, &oh);

             it_scrl_x = ev->cur.canvas.x -
               GG_IT(it)->wsd->reorder_it->dx;
             it_scrl_y = ev->cur.canvas.y -
               GG_IT(it)->wsd->reorder_it->dy;

             if (it_scrl_x < ox) GG_IT(it)->wsd->reorder_item_x = ox;
             else if (it_scrl_x + GG_IT(it)->wsd->item_width > ox + ow)
               GG_IT(it)->wsd->reorder_item_x =
                 ox + ow - GG_IT(it)->wsd->item_width;
             else GG_IT(it)->wsd->reorder_item_x = it_scrl_x;

             if (it_scrl_y < oy) GG_IT(it)->wsd->reorder_item_y = oy;
             else if (it_scrl_y + GG_IT(it)->wsd->item_height > oy + oh)
               GG_IT(it)->wsd->reorder_item_y =
                 oy + oh - GG_IT(it)->wsd->item_height;
             else GG_IT(it)->wsd->reorder_item_y = it_scrl_y;

             if (GG_IT(it)->wsd->calc_job)
               ecore_job_del(GG_IT(it)->wsd->calc_job);
             GG_IT(it)->wsd->calc_job =
               ecore_job_add(_calc_job, GG_IT(it)->wsd);
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
        if (it->long_timer)
          {
             ecore_timer_del(it->long_timer);
             it->long_timer = NULL;
          }
        if (!GG_IT(it)->wsd->was_selected)
          _item_unselect(it);

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
   Elm_Gengrid_Smart_Data *sd = GG_IT(it)->wsd;

   it->long_timer = NULL;
   if (elm_widget_item_disabled_get(it) || (it->dragging))
     return ECORE_CALLBACK_CANCEL;
   sd->longpressed = EINA_TRUE;
   evas_object_smart_callback_call(WIDGET(it), SIG_LONGPRESSED, it);

   if (sd->reorder_mode)
     {
        sd->reorder_it = it;
        evas_object_raise(VIEW(it));
        sd->s_iface->hold_set(WIDGET(it), EINA_TRUE);
        sd->s_iface->bounce_allow_get
          (WIDGET(it), &(GG_IT(it)->wsd->old_h_bounce),
          &(GG_IT(it)->wsd->old_v_bounce));

        sd->s_iface->bounce_allow_set(WIDGET(it), EINA_FALSE, EINA_FALSE);
        edje_object_signal_emit(VIEW(it), "elm,state,reorder,enabled", "elm");
     }

   return ECORE_CALLBACK_CANCEL;
}

static void
_item_highlight(Elm_Gen_Item *it)
{
   if ((GG_IT(it)->wsd->select_mode == ELM_OBJECT_SELECT_MODE_NONE)
       || (!GG_IT(it)->wsd->highlight) || (it->highlighted) ||
       (it->generation < GG_IT(it)->wsd->generation))
     return;

   edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
   it->highlighted = EINA_TRUE;
}

static void
_item_mouse_down_cb(void *data,
                    Evas *evas __UNUSED__,
                    Evas_Object *obj,
                    void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Elm_Gen_Item *it = data;
   Evas_Coord x, y;

   if (ev->button != 1) return;

   it->down = 1;
   it->dragging = 0;
   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   it->dx = ev->canvas.x - x;
   it->dy = ev->canvas.y - y;
   GG_IT(it)->wsd->longpressed = EINA_FALSE;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     GG_IT(it)->wsd->on_hold = EINA_TRUE;
   else GG_IT(it)->wsd->on_hold = EINA_FALSE;

   if (GG_IT(it)->wsd->on_hold) return;

   GG_IT(it)->wsd->was_selected = it->selected;
   _item_highlight(it);
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     {
        evas_object_smart_callback_call(WIDGET(it), SIG_CLICKED_DOUBLE, it);
        evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
     }
   if (it->long_timer) ecore_timer_del(it->long_timer);
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
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }

   elm_widget_stringlist_free(it->texts);
   it->texts = NULL;
   elm_widget_stringlist_free(it->contents);
   it->contents = NULL;
   elm_widget_stringlist_free(it->states);
   it->states = NULL;
   EINA_LIST_FREE (it->content_objs, content)
     evas_object_del(content);

   it->unrealize_cb(it);

   it->realized = EINA_FALSE;
   it->want_unrealize = EINA_FALSE;
   evas_event_thaw(evas_object_evas_get(WIDGET(it)));
   evas_event_thaw_eval(evas_object_evas_get(WIDGET(it)));
}

static void
_item_mouse_up_cb(void *data,
                  Evas *evas __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Eina_Bool dragged = EINA_FALSE;
   Elm_Gengrid_Smart_Data *sd;
   Elm_Gen_Item *it = data;

   if (ev->button != 1) return;

   sd = GG_IT(it)->wsd;

   it->down = EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     sd->on_hold = EINA_TRUE;
   else sd->on_hold = EINA_FALSE;

   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
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
        if (sd->calc_job)
          ecore_job_del(sd->calc_job);
        sd->calc_job =
          ecore_job_add(_calc_job, sd);

        sd->s_iface->hold_set(WIDGET(it), EINA_FALSE);
        sd->s_iface->bounce_allow_set
          (WIDGET(it), GG_IT(it)->wsd->old_h_bounce,
          GG_IT(it)->wsd->old_v_bounce);

        edje_object_signal_emit(VIEW(it), "elm,state,reorder,disabled", "elm");
     }
   if (sd->longpressed)
     {
        sd->longpressed = EINA_FALSE;
        if (!sd->was_selected) _item_unselect(it);
        sd->was_selected = EINA_FALSE;
        return;
     }
   if (dragged)
     {
        if (it->want_unrealize)
          _elm_gengrid_item_unrealize(it, EINA_FALSE);
     }
   if (elm_widget_item_disabled_get(it) || (dragged)) return;
   if (sd->multi)
     {
        if (!it->selected)
          {
             _item_highlight(it);
             it->sel_cb(it);
          }
        else _item_unselect(it);
     }
   else
     {
        if (!it->selected)
          {
             while (sd->selected)
               _item_unselect(sd->selected->data);
          }
        else
          {
             const Eina_List *l, *l_next;
             Elm_Gen_Item *item2;

             EINA_LIST_FOREACH_SAFE (sd->selected, l, l_next, item2)
               if (item2 != it) _item_unselect(item2);
          }
        _item_highlight(it);
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
   evas_object_del(VIEW(it));
   VIEW(it) = NULL;
   evas_object_del(it->spacer);
   it->spacer = NULL;
}

static char *
_access_info_cb(void *data,
                Evas_Object *obj __UNUSED__,
                Elm_Widget_Item *item __UNUSED__)
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
_access_state_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 Elm_Widget_Item *item __UNUSED__)
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

   ELM_GENGRID_DATA_GET(it->base.widget, sd);

   evas_object_geometry_get(it->base.view, &x, &y, &w, &h);

   evas_object_geometry_get(ELM_WIDGET_DATA(sd)->obj, &sx, &sy, &sw, &sh);
   if ((x < sx) || (y < sy) || ((x + w) > (sx + sw)) || ((y + h) > (sy + sh)))
     elm_gengrid_item_bring_in((Elm_Object_Item *)it,
                               ELM_GENGRID_ITEM_SCROLLTO_IN);
}

static void
_access_widget_item_register(Elm_Gen_Item *it)
{
   Elm_Access_Info *ai;

   _elm_access_widget_item_register((Elm_Widget_Item *)it);

   ai = _elm_access_object_get(it->base.access_obj);

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

   if ((it->realized) ||
       (it->generation < GG_IT(it)->wsd->generation))
     return;

   VIEW(it) = edje_object_add(evas_object_evas_get(WIDGET(it)));
   edje_object_scale_set
     (VIEW(it), elm_widget_scale_get(WIDGET(it)) * elm_config_scale_get());
   edje_object_mirrored_set(VIEW(it), elm_widget_mirrored_get(WIDGET(it)));
   evas_object_smart_member_add(VIEW(it), GG_IT(it)->wsd->pan_obj);
   elm_widget_sub_object_add(WIDGET(it), VIEW(it));
   snprintf(style, sizeof(style), "item/%s",
            it->itc->item_style ? it->itc->item_style : "default");
   elm_widget_theme_object_set(WIDGET(it), VIEW(it), "gengrid", style,
                               elm_widget_style_get(WIDGET(it)));
   it->spacer =
     evas_object_rectangle_add(evas_object_evas_get(WIDGET(it)));
   evas_object_color_set(it->spacer, 0, 0, 0, 0);
   elm_widget_sub_object_add(WIDGET(it), it->spacer);
   evas_object_size_hint_min_set(it->spacer, 2 * elm_config_scale_get(), 1);
   edje_object_part_swallow(VIEW(it), "elm.swallow.pad", it->spacer);

   if (it->itc->func.text_get)
     {
        const Eina_List *l;
        const char *key;

        it->texts =
          elm_widget_stringlist_get(edje_object_data_get(VIEW(it), "texts"));
        EINA_LIST_FOREACH (it->texts, l, key)
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
        EINA_LIST_FOREACH (it->contents, l, key)
          {
             if (it->itc->func.content_get)
               ic = it->itc->func.content_get
                   ((void *)it->base.data, WIDGET(it), key);
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
        EINA_LIST_FOREACH (it->states, l, key)
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
        if ((!GG_IT(it)->wsd->group_item_width)
            && (!GG_IT(it)->wsd->group_item_height))
          {
             edje_object_size_min_restricted_calc
               (VIEW(it), &GG_IT(it)->wsd->group_item_width,
               &GG_IT(it)->wsd->group_item_height,
               GG_IT(it)->wsd->group_item_width,
               GG_IT(it)->wsd->group_item_height);
          }
     }
   else
     {
        if ((!GG_IT(it)->wsd->item_width)
            && (!GG_IT(it)->wsd->item_height))
          {
             edje_object_size_min_restricted_calc
               (VIEW(it), &GG_IT(it)->wsd->item_width,
               &GG_IT(it)->wsd->item_height,
               GG_IT(it)->wsd->item_width,
               GG_IT(it)->wsd->item_height);
             elm_coords_finger_size_adjust
               (1, &GG_IT(it)->wsd->item_width, 1,
               &GG_IT(it)->wsd->item_height);
          }

        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_MOUSE_DOWN, _item_mouse_down_cb, it);
        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_MOUSE_UP, _item_mouse_up_cb, it);
        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_MOUSE_MOVE, _item_mouse_move_cb, it);

        _elm_gengrid_item_index_update(it);

        if (it->selected)
          edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
        if (elm_widget_item_disabled_get(it))
          edje_object_signal_emit(VIEW(it), "elm,state,disabled", "elm");
     }
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

   it->realized = EINA_TRUE;
   it->want_unrealize = EINA_FALSE;

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_widget_item_register(it);
}

static Eina_Bool
_reorder_item_move_animator_cb(void *data)
{
   Elm_Gen_Item *it = data;
   Evas_Coord dx, dy;
   double tt, t;

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
               (GG_IT(it)->wsd->pan_obj, NULL, NULL, &vw, &vh);
             if (GG_IT(it)->wsd->horizontal)
               evas_object_resize
                 (VIEW(it), GG_IT(it)->wsd->group_item_width, vh);
             else
               evas_object_resize
                 (VIEW(it), vw, GG_IT(it)->wsd->group_item_height);
          }
        else
          evas_object_resize(VIEW(it), GG_IT(it)->wsd->item_width,
                             GG_IT(it)->wsd->item_height);
        GG_IT(it)->moving = EINA_FALSE;
        GG_IT(it)->item_reorder_move_animator = NULL;

        return ECORE_CALLBACK_CANCEL;
     }

   evas_object_move(VIEW(it), GG_IT(it)->rx, GG_IT(it)->ry);
   if (it->group)
     {
        Evas_Coord vw, vh;

        evas_object_geometry_get
          (GG_IT(it)->wsd->pan_obj, NULL, NULL, &vw, &vh);
        if (GG_IT(it)->wsd->horizontal)
          evas_object_resize
            (VIEW(it), GG_IT(it)->wsd->group_item_width, vh);
        else
          evas_object_resize
            (VIEW(it), vw, GG_IT(it)->wsd->group_item_height);
     }
   else
     evas_object_resize
       (VIEW(it), GG_IT(it)->wsd->item_width,
       GG_IT(it)->wsd->item_height);

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
   Elm_Gengrid_Smart_Data *wsd;
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

   alignh = 0;
   alignw = 0;

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
                            if (wsd->calc_job)
                              ecore_job_del(wsd->calc_job);
                            wsd->calc_job =
                              ecore_job_add(_calc_job, WIDGET(it));

                            return;
                         }
                    }
               }
             else if (item->item_reorder_move_animator)
               {
                  ecore_animator_del
                    (item->item_reorder_move_animator);
                  item->item_reorder_move_animator = NULL;
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
_group_item_place(Elm_Gengrid_Pan_Smart_Data *psd)
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
   EINA_LIST_FOREACH (psd->wsd->group_items, l, it)
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

static void
_elm_gengrid_pan_smart_calculate(Evas_Object *obj)
{
   Evas_Coord cx = 0, cy = 0;
   Elm_Gen_Item *it;

   ELM_GENGRID_PAN_DATA_GET(obj, psd);

   if (!psd->wsd->nmax) return;

   psd->wsd->reorder_item_changed = EINA_FALSE;

   EINA_INLIST_FOREACH (psd->wsd->items, it)
     {
        if (it->group)
          {
             if (psd->wsd->horizontal)
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
        if (psd->wsd->reorder_item_changed) return;
        if (it->group)
          {
             if (psd->wsd->horizontal)
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
             if (psd->wsd->horizontal)
               {
                  cy = (cy + 1) % psd->wsd->nmax;
                  if (!cy) cx++;
               }
             else
               {
                  cx = (cx + 1) % psd->wsd->nmax;
                  if (!cx) cy++;
               }
          }
     }
   _group_item_place(psd);

   if ((psd->wsd->reorder_mode) && (psd->wsd->reorder_it))
     {
        if (!psd->wsd->reorder_item_changed)
          {
             psd->wsd->old_pan_x = psd->wsd->pan_x;
             psd->wsd->old_pan_y = psd->wsd->pan_y;
          }
        psd->wsd->move_effect_enabled = EINA_FALSE;
     }
   evas_object_smart_callback_call
     (ELM_WIDGET_DATA(psd->wsd)->obj, SIG_CHANGED, NULL);
}

static void
_elm_gengrid_pan_smart_pos_set(Evas_Object *obj,
                               Evas_Coord x,
                               Evas_Coord y)
{
   ELM_GENGRID_PAN_DATA_GET(obj, psd);

   if ((x == psd->wsd->pan_x) && (y == psd->wsd->pan_y)) return;
   psd->wsd->pan_x = x;
   psd->wsd->pan_y = y;
   evas_object_smart_changed(obj);
}

static void
_elm_gengrid_pan_smart_pos_get(const Evas_Object *obj,
                               Evas_Coord *x,
                               Evas_Coord *y)
{
   ELM_GENGRID_PAN_DATA_GET(obj, psd);

   if (x) *x = psd->wsd->pan_x;
   if (y) *y = psd->wsd->pan_y;
}

static void
_elm_gengrid_pan_smart_content_size_get(const Evas_Object *obj,
                                        Evas_Coord *w,
                                        Evas_Coord *h)
{
   ELM_GENGRID_PAN_DATA_GET(obj, psd);

   if (w) *w = psd->wsd->minw;
   if (h) *h = psd->wsd->minh;
}

static void
_elm_gengrid_pan_smart_pos_max_get(const Evas_Object *obj,
                                   Evas_Coord *x,
                                   Evas_Coord *y)
{
   Evas_Coord ow, oh;

   ELM_GENGRID_PAN_DATA_GET(obj, psd);

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   if (x)
     *x = (ow < psd->wsd->minw) ? psd->wsd->minw - ow : 0;
   if (y)
     *y = (oh < psd->wsd->minh) ? psd->wsd->minh - oh : 0;
}

static void
_elm_gengrid_pan_smart_pos_min_get(const Evas_Object *obj,
                                   Evas_Coord *x,
                                   Evas_Coord *y)
{
   Evas_Coord mx = 0, my = 0;

   ELM_GENGRID_PAN_DATA_GET(obj, psd);

   _elm_gengrid_pan_smart_pos_max_get(obj, &mx, &my);
   if (x)
     *x = -mx * psd->wsd->align_x;
   if (y)
     *y = -my * psd->wsd->align_y;
}

static void
_elm_gengrid_pan_smart_set_user(Elm_Gengrid_Pan_Smart_Class *sc)
{
   ELM_PAN_CLASS(sc)->base.add = _elm_gengrid_pan_smart_add;
   ELM_PAN_CLASS(sc)->base.move = _elm_gengrid_pan_smart_move;
   ELM_PAN_CLASS(sc)->base.resize = _elm_gengrid_pan_smart_resize;
   ELM_PAN_CLASS(sc)->base.calculate = _elm_gengrid_pan_smart_calculate;

   ELM_PAN_CLASS(sc)->pos_set = _elm_gengrid_pan_smart_pos_set;
   ELM_PAN_CLASS(sc)->pos_get = _elm_gengrid_pan_smart_pos_get;
   ELM_PAN_CLASS(sc)->pos_max_get = _elm_gengrid_pan_smart_pos_max_get;
   ELM_PAN_CLASS(sc)->pos_min_get = _elm_gengrid_pan_smart_pos_min_get;
   ELM_PAN_CLASS(sc)->content_size_get =
     _elm_gengrid_pan_smart_content_size_get;
}

static Eina_Bool
_item_multi_select_left(Elm_Gengrid_Smart_Data *sd)
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
_item_multi_select_right(Elm_Gengrid_Smart_Data *sd)
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
_item_multi_select_up(Elm_Gengrid_Smart_Data *sd)
{
   unsigned int i;
   Eina_Bool r = EINA_TRUE;

   if (!sd->selected) return EINA_FALSE;

   for (i = 0; (r) && (i < sd->nmax); i++)
     r &= _item_multi_select_left(sd);

   return r;
}

static Eina_Bool
_item_multi_select_down(Elm_Gengrid_Smart_Data *sd)
{
   unsigned int i;
   Eina_Bool r = EINA_TRUE;

   if (!sd->selected) return EINA_FALSE;

   for (i = 0; (r) && (i < sd->nmax); i++)
     r &= _item_multi_select_right(sd);

   return r;
}

static Eina_Bool
_all_items_deselect(Elm_Gengrid_Smart_Data *sd)
{
   if (!sd->selected) return EINA_FALSE;

   while (sd->selected)
     elm_gengrid_item_selected_set
       ((Elm_Object_Item *)sd->selected->data, EINA_FALSE);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_up(Elm_Gengrid_Smart_Data *sd)
{
   unsigned int i;
   Elm_Gen_Item *prev;

   if (!sd->selected)
     {
        prev = ELM_GEN_ITEM_FROM_INLIST(sd->items->last);
        while ((prev) && (prev->generation < sd->generation))
          prev = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(prev)->prev);
        elm_gengrid_item_selected_set((Elm_Object_Item *)prev, EINA_TRUE);
        elm_gengrid_item_show
          ((Elm_Object_Item *)prev, ELM_GENGRID_ITEM_SCROLLTO_IN);
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
   elm_gengrid_item_show
     ((Elm_Object_Item *)prev, ELM_GENGRID_ITEM_SCROLLTO_IN);
   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_down(Elm_Gengrid_Smart_Data *sd)
{
   unsigned int i;
   Elm_Gen_Item *next;

   if (!sd->selected)
     {
        next = ELM_GEN_ITEM_FROM_INLIST(sd->items);
        while ((next) && (next->generation < sd->generation))
          next = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(next)->next);
        elm_gengrid_item_selected_set((Elm_Object_Item *)next, EINA_TRUE);
        elm_gengrid_item_show
          ((Elm_Object_Item *)next, ELM_GENGRID_ITEM_SCROLLTO_IN);
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
   elm_gengrid_item_show
     ((Elm_Object_Item *)next, ELM_GENGRID_ITEM_SCROLLTO_IN);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_left(Elm_Gengrid_Smart_Data *sd)
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
   elm_gengrid_item_show
     ((Elm_Object_Item *)prev, ELM_GENGRID_ITEM_SCROLLTO_IN);

   return EINA_TRUE;
}

static Eina_Bool
_item_single_select_right(Elm_Gengrid_Smart_Data *sd)
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
   elm_gengrid_item_show
     ((Elm_Object_Item *)next, ELM_GENGRID_ITEM_SCROLLTO_IN);

   return EINA_TRUE;
}

static Eina_Bool
_elm_gengrid_smart_event(Evas_Object *obj,
                         Evas_Object *src __UNUSED__,
                         Evas_Callback_Type type,
                         void *event_info)
{
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;
   Elm_Object_Item *it = NULL;
   Evas_Event_Key_Down *ev = event_info;

   ELM_GENGRID_DATA_GET(obj, sd);

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (!sd->items) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   sd->s_iface->content_pos_get(obj, &x, &y);
   sd->s_iface->step_size_get(obj, &step_x, &step_y);
   sd->s_iface->page_size_get(obj, &page_x, &page_y);
   sd->s_iface->content_viewport_size_get(obj, &v_w, &v_h);

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        if ((sd->horizontal) &&
            (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
              (_item_multi_select_up(sd)))
             || (_item_single_select_up(sd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if ((!sd->horizontal) &&
                 (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
                   (_item_multi_select_left(sd)))
                  || (_item_single_select_left(sd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        if ((sd->horizontal) &&
            (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
              (_item_multi_select_down(sd)))
             || (_item_single_select_down(sd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if ((!sd->horizontal) &&
                 (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
                   (_item_multi_select_right(sd)))
                  || (_item_single_select_right(sd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        if ((sd->horizontal) &&
            (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
              (_item_multi_select_left(sd)))
             || (_item_single_select_left(sd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if ((!sd->horizontal) &&
                 (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
                   (_item_multi_select_up(sd)))
                  || (_item_single_select_up(sd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        if ((sd->horizontal) &&
            (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
              (_item_multi_select_right(sd)))
             || (_item_single_select_right(sd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else if ((!sd->horizontal) &&
                 (((evas_key_modifier_is_set(ev->modifiers, "Shift")) &&
                   (_item_multi_select_down(sd)))
                  || (_item_single_select_down(sd))))
          {
             ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
             return EINA_TRUE;
          }
        else
          y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Home")) ||
            ((!strcmp(ev->keyname, "KP_Home")) && (!ev->string)))
     {
        it = elm_gengrid_first_item_get(obj);
        elm_gengrid_item_bring_in(it, ELM_GENGRID_ITEM_SCROLLTO_IN);
        elm_gengrid_item_selected_set(it, EINA_TRUE);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "End")) ||
            ((!strcmp(ev->keyname, "KP_End")) && (!ev->string)))
     {
        it = elm_gengrid_last_item_get(obj);
        elm_gengrid_item_bring_in(it, ELM_GENGRID_ITEM_SCROLLTO_IN);
        elm_gengrid_item_selected_set(it, EINA_TRUE);
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if ((!strcmp(ev->keyname, "Prior")) ||
            ((!strcmp(ev->keyname, "KP_Prior")) && (!ev->string)))
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
   else if ((!strcmp(ev->keyname, "Next")) ||
            ((!strcmp(ev->keyname, "KP_Next")) && (!ev->string)))
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
   else if (!strcmp(ev->keyname, "Escape"))
     {
        if (!_all_items_deselect(sd)) return EINA_FALSE;
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        return EINA_TRUE;
     }
   else if (((!strcmp(ev->keyname, "Return")) ||
             (!strcmp(ev->keyname, "KP_Enter")) ||
             (!strcmp(ev->keyname, "space")))
            && (!sd->multi) && (sd->selected))
     {
        it = elm_gengrid_selected_item_get(obj);
        evas_object_smart_callback_call(WIDGET(it), SIG_ACTIVATED, it);
     }
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   sd->s_iface->content_pos_set(obj, x, y);
   return EINA_TRUE;
}

static Eina_Bool
_elm_gengrid_smart_on_focus(Evas_Object *obj)
{
   ELM_GENGRID_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_gengrid_parent_sc)->on_focus(obj))
     return EINA_FALSE;

   if (elm_widget_focus_get(obj) && (sd->selected) &&
       (!sd->last_selected_item))
     sd->last_selected_item = eina_list_data_get(sd->selected);

   return EINA_TRUE;
}

static Eina_Bool
_elm_gengrid_smart_focus_next(const Evas_Object *obj,
                           Elm_Focus_Direction dir,
                           Evas_Object **next)
{
   Eina_List *items = NULL;
   Elm_Gen_Item *it;

   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   ELM_GENGRID_DATA_GET(obj, sd);

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

   sd->s_iface->mirrored_set(obj, rtl);

   if (!sd->items) return;
   it = ELM_GEN_ITEM_FROM_INLIST(sd->items);

   while (it)
     {
        edje_object_mirrored_set(VIEW(it), rtl);
        elm_gengrid_item_update((Elm_Object_Item *)it);
        it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
     }
}

static Eina_Bool
_elm_gengrid_smart_theme(Evas_Object *obj)
{
   if (!ELM_WIDGET_CLASS(_elm_gengrid_parent_sc)->theme(obj))
     return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   return EINA_TRUE;
}

static void
_item_position_update(Eina_Inlist *list,
                      int idx)
{
   Elm_Gen_Item *it;

   EINA_INLIST_FOREACH (list, it)
     {
        it->position = idx++;
        it->position_update = EINA_TRUE;
     }
}

static void
_elm_gengrid_item_del_not_serious(Elm_Gen_Item *it)
{
   elm_widget_item_pre_notify_del(it);
   it->generation = GG_IT(it)->wsd->generation - 1; /* This means that the
                                                     * item is deleted */

   if ((it->relcount > 0) || (it->walking > 0)) return;

   if (it->selected)
     GG_IT(it)->wsd->selected =
       eina_list_remove(GG_IT(it)->wsd->selected, it);

   if (it->itc->func.del)
     it->itc->func.del((void *)it->base.data, WIDGET(it));
}

static void
_elm_gengrid_item_del_serious(Elm_Gen_Item *it)
{
   _elm_gengrid_item_del_not_serious(it);
   GG_IT(it)->wsd->items = eina_inlist_remove
       (GG_IT(it)->wsd->items, EINA_INLIST_GET(it));
   if (it->tooltip.del_cb)
     it->tooltip.del_cb((void *)it->tooltip.data, WIDGET(it), it);
   GG_IT(it)->wsd->walking -= it->walking;
   if (it->long_timer)
     {
        ecore_timer_del(it->long_timer);
        it->long_timer = NULL;
     }
   if (it->group)
     GG_IT(it)->wsd->group_items =
       eina_list_remove(GG_IT(it)->wsd->group_items, it);

   if (GG_IT(it)->wsd->state)
     {
        eina_inlist_sorted_state_free(GG_IT(it)->wsd->state);
        GG_IT(it)->wsd->state = NULL;
     }
   if (GG_IT(it)->wsd->calc_job) ecore_job_del(GG_IT(it)->wsd->calc_job);
   GG_IT(it)->wsd->calc_job =
     ecore_job_add(GG_IT(it)->wsd->calc_cb, GG_IT(it)->wsd);

   if (GG_IT(it)->wsd->last_selected_item == (Elm_Object_Item *)it)
     GG_IT(it)->wsd->last_selected_item = NULL;
   GG_IT(it)->wsd->item_count--;

   free(it->item);
   it->item = NULL;
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
                         void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_edge_left_cb(Evas_Object *obj,
              void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_LEFT, NULL);
}

static void
_edge_right_cb(Evas_Object *obj,
               void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_RIGHT, NULL);
}

static void
_edge_top_cb(Evas_Object *obj,
             void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_TOP, NULL);
}

static void
_edge_bottom_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_BOTTOM, NULL);
}

static void
_scroll_cb(Evas_Object *obj,
           void *data __UNUSED__)
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

   if ((it->relcount > 0) || (it->walking > 0))
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
_elm_gengrid_clear(Evas_Object *obj,
                   Eina_Bool standby)
{
   Eina_Inlist *next, *l;

   ELM_GENGRID_DATA_GET(obj, sd);

   if (!standby) sd->generation++;

   if (sd->state)
     {
        eina_inlist_sorted_state_free(sd->state);
        sd->state = NULL;
     }

   if (sd->walking > 0)
     {
        sd->clear_me = EINA_TRUE;
        return;
     }
   evas_event_freeze(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
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
   if (sd->calc_job)
     {
        ecore_job_del(sd->calc_job);
        sd->calc_job = NULL;
     }
   if (sd->selected) sd->selected = eina_list_free(sd->selected);
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
   sd->s_iface->content_region_show(obj, 0, 0, 0, 0);
   evas_event_thaw(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
   evas_event_thaw_eval(evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
}

static void
_item_select(Elm_Gen_Item *it)
{
   Evas_Object *obj = WIDGET(it);

   if ((it->generation < GG_IT(it)->wsd->generation) || (it->decorate_it_set) ||
       (it->select_mode == ELM_OBJECT_SELECT_MODE_NONE) ||
       (GG_IT(it)->wsd->select_mode == ELM_OBJECT_SELECT_MODE_NONE))
     return;
   if (!it->selected)
     {
        it->selected = EINA_TRUE;
        GG_IT(it)->wsd->selected =
          eina_list_append(GG_IT(it)->wsd->selected, it);
     }
   else if (GG_IT(it)->wsd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS)
     return;

   evas_object_ref(obj);
   it->walking++;
   GG_IT(it)->wsd->walking++;
   if (it->func.func) it->func.func((void *)it->func.data, WIDGET(it), it);
   if (it->generation == GG_IT(it)->wsd->generation)
     evas_object_smart_callback_call(WIDGET(it), SIG_SELECTED, it);
   it->walking--;
   GG_IT(it)->wsd->walking--;
   if ((GG_IT(it)->wsd->clear_me) && (!GG_IT(it)->wsd->walking))
     _elm_gengrid_clear(WIDGET(it), EINA_TRUE);
   else
     {
        if ((!it->walking) && (it->generation < GG_IT(it)->wsd->generation))
          {
             if (!it->relcount)
               {
                  it->del_cb(it);
                  elm_widget_item_free(it);
               }
          }
        else
          GG_IT(it)->wsd->last_selected_item = (Elm_Object_Item *)it;
     }
   evas_object_unref(obj);
}

static Elm_Gen_Item *
_elm_gengrid_item_new(Elm_Gengrid_Smart_Data *sd,
                      const Elm_Gen_Item_Class *itc,
                      const void *data,
                      Evas_Smart_Cb func,
                      const void *func_data)
{
   Elm_Gen_Item *it;

   if (!itc) return NULL;

   it = elm_widget_item_new(ELM_WIDGET_DATA(sd)->obj, Elm_Gen_Item);
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

   /* TEMPORARY */
   it->sel_cb = (Ecore_Cb)_item_select;

   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_gengrid_item_class_ref((Elm_Gengrid_Item_Class *)itc);
   it->item = ELM_NEW(Elm_Gen_Item_Type);

   GG_IT(it)->wsd = sd;

   sd->item_count++;
   it->group = it->itc->item_style &&
     (!strcmp(it->itc->item_style, "group_index"));

   it->del_cb = (Ecore_Cb)_item_del;
   it->highlight_cb = (Ecore_Cb)_item_highlight;
   it->unsel_cb = (Ecore_Cb)_item_unselect;
   it->unrealize_cb = (Ecore_Cb)_item_unrealize_cb;

   return it;
}

/* common layout sizing won't apply here */
static void
_elm_gengrid_smart_sizing_eval(Evas_Object *obj __UNUSED__)
{
   return;  /* no-op */
}

static void
_elm_gengrid_smart_add(Evas_Object *obj)
{
   Eina_Bool bounce = _elm_config->thumbscroll_bounce_enable;
   Elm_Gengrid_Pan_Smart_Data *pan_data;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Gengrid_Smart_Data);

   ELM_WIDGET_CLASS(_elm_gengrid_parent_sc)->base.add(obj);

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

   elm_layout_theme_set(obj, "gengrid", "base", elm_widget_style_get(obj));

   /* interface's add() routine issued AFTER the object's smart_add() */
   priv->s_iface = evas_object_smart_interface_get
       (obj, ELM_SCROLLABLE_IFACE_NAME);

   priv->s_iface->objects_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, priv->hit_rect);

   priv->old_h_bounce = bounce;
   priv->old_v_bounce = bounce;
   priv->s_iface->bounce_allow_set(obj, bounce, bounce);

   priv->s_iface->animate_start_cb_set(obj, _scroll_animate_start_cb);
   priv->s_iface->animate_stop_cb_set(obj, _scroll_animate_stop_cb);
   priv->s_iface->drag_start_cb_set(obj, _scroll_drag_start_cb);
   priv->s_iface->drag_stop_cb_set(obj, _scroll_drag_stop_cb);
   priv->s_iface->edge_left_cb_set(obj, _edge_left_cb);
   priv->s_iface->edge_right_cb_set(obj, _edge_right_cb);
   priv->s_iface->edge_top_cb_set(obj, _edge_top_cb);
   priv->s_iface->edge_bottom_cb_set(obj, _edge_bottom_cb);
   priv->s_iface->scroll_cb_set(obj, _scroll_cb);

   priv->align_x = 0.5;
   priv->align_y = 0.5;
   priv->highlight = EINA_TRUE;

   priv->pan_obj = evas_object_smart_add
       (evas_object_evas_get(obj), _elm_gengrid_pan_smart_class_new());
   pan_data = evas_object_smart_data_get(priv->pan_obj);
   pan_data->wsd = priv;

   priv->s_iface->extern_pan_set(obj, priv->pan_obj);
}

static void
_elm_gengrid_smart_del(Evas_Object *obj)
{
   ELM_GENGRID_DATA_GET(obj, sd);

   elm_gengrid_clear(obj);
   evas_object_del(sd->pan_obj);
   sd->pan_obj = NULL;

   if (sd->calc_job) ecore_job_del(sd->calc_job);

   ELM_WIDGET_CLASS(_elm_gengrid_parent_sc)->base.del(obj);
}

static void
_elm_gengrid_smart_move(Evas_Object *obj,
                        Evas_Coord x,
                        Evas_Coord y)
{
   ELM_GENGRID_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_gengrid_parent_sc)->base.move(obj, x, y);

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_gengrid_smart_resize(Evas_Object *obj,
                          Evas_Coord w,
                          Evas_Coord h)
{
   ELM_GENGRID_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_gengrid_parent_sc)->base.resize(obj, w, h);

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_gengrid_smart_member_add(Evas_Object *obj,
                              Evas_Object *member)
{
   ELM_GENGRID_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_gengrid_parent_sc)->base.member_add(obj, member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_access_obj_process(Elm_Gengrid_Smart_Data * sd, Eina_Bool is_access)
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

static void
_access_hook(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);
   
   if (is_access)
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next =
     _elm_gengrid_smart_focus_next;
   else
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next = NULL;
   
   _access_obj_process(sd, is_access);
}

static void
_elm_gengrid_smart_set_user(Elm_Gengrid_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_gengrid_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_gengrid_smart_del;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_gengrid_smart_move;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_gengrid_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.member_add = _elm_gengrid_smart_member_add;

   ELM_WIDGET_CLASS(sc)->on_focus = _elm_gengrid_smart_on_focus;
   ELM_WIDGET_CLASS(sc)->theme = _elm_gengrid_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_gengrid_smart_event;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_gengrid_smart_sizing_eval;

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     ELM_WIDGET_CLASS(sc)->focus_next = _elm_gengrid_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _access_hook;
}

EAPI const Elm_Gengrid_Smart_Class *
elm_gengrid_smart_class_get(void)
{
   static Elm_Gengrid_Smart_Class _sc =
     ELM_GENGRID_SMART_CLASS_INIT_NAME_VERSION(ELM_GENGRID_SMART_NAME);
   static const Elm_Gengrid_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_gengrid_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_gengrid_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_gengrid_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_gengrid_item_size_set(Evas_Object *obj,
                          Evas_Coord w,
                          Evas_Coord h)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   if ((sd->item_width == w) && (sd->item_height == h)) return;
   sd->item_width = w;
   sd->item_height = h;
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);
}

EAPI void
elm_gengrid_item_size_get(const Evas_Object *obj,
                          Evas_Coord *w,
                          Evas_Coord *h)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   if (w) *w = sd->item_width;
   if (h) *h = sd->item_height;
}

EAPI void
elm_gengrid_group_item_size_set(Evas_Object *obj,
                                Evas_Coord w,
                                Evas_Coord h)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   if ((sd->group_item_width == w) && (sd->group_item_height == h)) return;
   sd->group_item_width = w;
   sd->group_item_height = h;
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);
}

EAPI void
elm_gengrid_group_item_size_get(const Evas_Object *obj,
                                Evas_Coord *w,
                                Evas_Coord *h)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   if (w) *w = sd->group_item_width;
   if (h) *h = sd->group_item_height;
}

EAPI void
elm_gengrid_align_set(Evas_Object *obj,
                      double align_x,
                      double align_y)
{
   double old_h, old_y;

   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

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

EAPI void
elm_gengrid_align_get(const Evas_Object *obj,
                      double *align_x,
                      double *align_y)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   if (align_x) *align_x = sd->align_x;
   if (align_y) *align_y = sd->align_y;
}

EAPI Elm_Object_Item *
elm_gengrid_item_append(Evas_Object *obj,
                        const Elm_Gengrid_Item_Class *itc,
                        const void *data,
                        Evas_Smart_Cb func,
                        const void *func_data)
{
   Elm_Gen_Item *it;

   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET(obj, sd);

   it = _elm_gengrid_item_new(sd, itc, data, func, func_data);
   if (!it) return NULL;

   sd->items = eina_inlist_append(sd->items, EINA_INLIST_GET(it));
   it->position = eina_inlist_count(sd->items);
   it->position_update = EINA_TRUE;

   if (it->group)
     sd->group_items = eina_list_prepend(sd->group_items, it);

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_gengrid_item_prepend(Evas_Object *obj,
                         const Elm_Gengrid_Item_Class *itc,
                         const void *data,
                         Evas_Smart_Cb func,
                         const void *func_data)
{
   Elm_Gen_Item *it;

   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET(obj, sd);

   it = _elm_gengrid_item_new(sd, itc, data, func, func_data);
   if (!it) return NULL;

   sd->items = eina_inlist_prepend(sd->items, EINA_INLIST_GET(it));
   _item_position_update(sd->items, 0);

   if (it->group)
     sd->group_items = eina_list_append(sd->group_items, it);

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_gengrid_item_insert_before(Evas_Object *obj,
                               const Elm_Gengrid_Item_Class *itc,
                               const void *data,
                               Elm_Object_Item *relative,
                               Evas_Smart_Cb func,
                               const void *func_data)
{
   Elm_Gen_Item *it;
   Eina_Inlist *tmp;

   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(relative, NULL);
   ELM_GENGRID_DATA_GET(obj, sd);

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

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_gengrid_item_insert_after(Evas_Object *obj,
                              const Elm_Gengrid_Item_Class *itc,
                              const void *data,
                              Elm_Object_Item *relative,
                              Evas_Smart_Cb func,
                              const void *func_data)
{
   Elm_Gen_Item *it;
   Eina_Inlist *tmp;

   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_ITEM_CHECK_OR_RETURN(relative, NULL);
   ELM_GENGRID_DATA_GET(obj, sd);

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

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_gengrid_item_sorted_insert(Evas_Object *obj,
                               const Elm_Gengrid_Item_Class *itc,
                               const void *data,
                               Eina_Compare_Cb comp,
                               Evas_Smart_Cb func,
                               const void *func_data)
{
   Elm_Gen_Item *it;

   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET(obj, sd);

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

   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);

   return (Elm_Object_Item *)it;
}

EAPI void
elm_gengrid_horizontal_set(Evas_Object *obj,
                           Eina_Bool horizontal)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   horizontal = !!horizontal;
   if (horizontal == sd->horizontal) return;
   sd->horizontal = horizontal;

   /* Update the items to conform to the new layout */
   if (sd->calc_job) ecore_job_del(sd->calc_job);
   sd->calc_job = ecore_job_add(_calc_job, sd);
}

EAPI Eina_Bool
elm_gengrid_horizontal_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   ELM_GENGRID_DATA_GET(obj, sd);

   return sd->horizontal;
}

EAPI void
elm_gengrid_clear(Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj);

   _elm_gengrid_clear(obj, EINA_FALSE);
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

   _elm_gengrid_item_unrealize(it, EINA_FALSE);
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

   return (Elm_Gengrid_Item_Class *)elm_gengrid_item_item_class_get(it);
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

EAPI void
elm_gengrid_multi_select_set(Evas_Object *obj,
                             Eina_Bool multi)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->multi = !!multi;
}

EAPI Eina_Bool
elm_gengrid_multi_select_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   ELM_GENGRID_DATA_GET(obj, sd);

   return sd->multi;
}

EAPI Elm_Object_Item *
elm_gengrid_selected_item_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET(obj, sd);

   if (sd->selected) return sd->selected->data;
   return NULL;
}

EAPI const Eina_List *
elm_gengrid_selected_items_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET(obj, sd);

   return sd->selected;
}

EAPI void
elm_gengrid_item_selected_set(Elm_Object_Item *item,
                              Eina_Bool selected)
{
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);

   if ((it->generation < GG_IT(it)->wsd->generation) ||
       elm_widget_item_disabled_get(it))
     return;
   selected = !!selected;
   if (it->selected == selected) return;

   if (selected)
     {
        if (!GG_IT(it)->wsd->multi)
          {
             while (GG_IT(it)->wsd->selected)
               {
                  if (it->unhighlight_cb)
                    it->unhighlight_cb(GG_IT(it)->wsd->selected->data);
                  it->unsel_cb(GG_IT(it)->wsd->selected->data);
               }
          }
        it->highlight_cb(it);
        _item_select(it);
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

EAPI Eina_List *
elm_gengrid_realized_items_get(const Evas_Object *obj)
{
   Eina_List *list = NULL;
   Elm_Gen_Item *it;

   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH (sd->items, it)
     {
        if (it->realized) list = eina_list_append(list, (Elm_Object_Item *)it);
     }
   return list;
}

EAPI void
elm_gengrid_realized_items_update(Evas_Object *obj)
{
   Eina_List *list, *l;
   Elm_Object_Item *it;

   ELM_GENGRID_CHECK(obj);

   list = elm_gengrid_realized_items_get(obj);
   EINA_LIST_FOREACH (list, l, it)
     elm_gengrid_item_update(it);
}

static Evas_Object *
_elm_gengrid_item_label_create(void *data,
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
_elm_gengrid_item_label_del_cb(void *data,
                               Evas_Object *obj __UNUSED__,
                               void *event_info __UNUSED__)
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

   eina_stringshare_del(it->mouse_cursor);
   it->mouse_cursor = NULL;
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
elm_gengrid_reorder_mode_set(Evas_Object *obj,
                             Eina_Bool reorder_mode)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->reorder_mode = !!reorder_mode;
}

EAPI Eina_Bool
elm_gengrid_reorder_mode_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   ELM_GENGRID_DATA_GET(obj, sd);

   return sd->reorder_mode;
}

EAPI void
elm_gengrid_bounce_set(Evas_Object *obj,
                       Eina_Bool h_bounce,
                       Eina_Bool v_bounce)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->bounce_allow_set(obj, !!h_bounce, !!v_bounce);
}

EAPI void
elm_gengrid_bounce_get(const Evas_Object *obj,
                       Eina_Bool *h_bounce,
                       Eina_Bool *v_bounce)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->bounce_allow_get(obj, h_bounce, v_bounce);
}

EAPI void
elm_gengrid_page_relative_set(Evas_Object *obj,
                              double h_pagerel,
                              double v_pagerel)
{
   Evas_Coord pagesize_h;
   Evas_Coord pagesize_v;

   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->paging_get(obj, NULL, NULL, &pagesize_h, &pagesize_v);
   sd->s_iface->paging_set
     (obj, h_pagerel, v_pagerel, pagesize_h, pagesize_v);
}

EAPI void
elm_gengrid_page_relative_get(const Evas_Object *obj,
                              double *h_pagerel,
                              double *v_pagerel)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->paging_get(obj, h_pagerel, v_pagerel, NULL, NULL);
}

EAPI void
elm_gengrid_page_size_set(Evas_Object *obj,
                          Evas_Coord h_pagesize,
                          Evas_Coord v_pagesize)
{
   double pagerel_h;
   double pagerel_v;

   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->paging_get(obj, &pagerel_h, &pagerel_v, NULL, NULL);
   sd->s_iface->paging_set
     (obj, pagerel_h, pagerel_v, h_pagesize, v_pagesize);
}

EAPI void
elm_gengrid_current_page_get(const Evas_Object *obj,
                             int *h_pagenumber,
                             int *v_pagenumber)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->current_page_get(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gengrid_last_page_get(const Evas_Object *obj,
                          int *h_pagenumber,
                          int *v_pagenumber)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->last_page_get(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gengrid_page_show(const Evas_Object *obj,
                      int h_pagenumber,
                      int v_pagenumber)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->page_show((Evas_Object *)obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gengrid_page_bring_in(const Evas_Object *obj,
                          int h_pagenumber,
                          int v_pagenumber)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->page_bring_in((Evas_Object *)obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_gengrid_scroller_policy_set(Evas_Object *obj,
                                Elm_Scroller_Policy policy_h,
                                Elm_Scroller_Policy policy_v)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   sd->s_iface->policy_set(obj, policy_h, policy_v);
}

EAPI void
elm_gengrid_scroller_policy_get(const Evas_Object *obj,
                                Elm_Scroller_Policy *policy_h,
                                Elm_Scroller_Policy *policy_v)
{
   Elm_Scroller_Policy s_policy_h, s_policy_v;

   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->s_iface->policy_get(obj, &s_policy_h, &s_policy_v);
   if (policy_h) *policy_h = (Elm_Scroller_Policy)s_policy_h;
   if (policy_v) *policy_v = (Elm_Scroller_Policy)s_policy_v;
}

EAPI Elm_Object_Item *
elm_gengrid_first_item_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

   if (!sd->items) return NULL;

   Elm_Gen_Item *it = ELM_GEN_ITEM_FROM_INLIST(sd->items);

   while ((it) && (it->generation < sd->generation))
     it = ELM_GEN_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_gengrid_last_item_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) NULL;
   ELM_GENGRID_DATA_GET_OR_RETURN_VAL(obj, sd, NULL);

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
   Elm_Gengrid_Smart_Data *sd;
   Evas_Coord minx = 0, miny = 0;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);
   sd = GG_IT(it)->wsd;

   if ((it->generation < sd->generation)) return;
   _elm_gengrid_pan_smart_pos_min_get(sd->pan_obj, &minx, &miny);

   if (type == ELM_GENGRID_ITEM_SCROLLTO_IN)
     {
        //TODO : type based handling like gengrid
     }

   if (sd->horizontal)
     sd->s_iface->content_region_show
       (WIDGET(it), ((it->x - GG_IT(it)->prev_group) * sd->item_width)
       + (GG_IT(it)->prev_group * sd->group_item_width) + minx,
       it->y * sd->item_height + miny, sd->item_width, sd->item_height);
   else
     sd->s_iface->content_region_show
       (WIDGET(it), it->x * sd->item_width + minx,
       ((it->y - GG_IT(it)->prev_group) * sd->item_height) +
       (GG_IT(it)->prev_group * sd->group_item_height) + miny,
       sd->item_width, sd->item_height);
}

EAPI void
elm_gengrid_item_bring_in(Elm_Object_Item *item,
                          Elm_Gengrid_Item_Scrollto_Type type)
{
   Elm_Gengrid_Smart_Data *sd;
   Evas_Coord minx = 0, miny = 0;
   Elm_Gen_Item *it = (Elm_Gen_Item *)item;

   ELM_GENGRID_ITEM_CHECK_OR_RETURN(it);
   sd = GG_IT(it)->wsd;

   if (it->generation < sd->generation) return;

   _elm_gengrid_pan_smart_pos_min_get(sd->pan_obj, &minx, &miny);

   if (type == ELM_GENGRID_ITEM_SCROLLTO_IN)
     {
        //TODO : type based handling like gengrid
     }

   if (sd->horizontal)
     sd->s_iface->region_bring_in
       (WIDGET(it), ((it->x - GG_IT(it)->prev_group) * sd->item_width)
       + (GG_IT(it)->prev_group * sd->group_item_width) + minx,
       it->y * sd->item_height + miny, sd->item_width, sd->item_height);
   else
     sd->s_iface->region_bring_in
       (WIDGET(it), it->x * sd->item_width + minx,
       ((it->y - GG_IT(it)->prev_group) * sd->item_height)
       + (GG_IT(it)->prev_group * sd->group_item_height)
       + miny, sd->item_width, sd->item_height);
}

EAPI void
elm_gengrid_filled_set(Evas_Object *obj,
                       Eina_Bool fill)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   fill = !!fill;
   if (sd->filled != fill)
     sd->filled = fill;
}

EAPI Eina_Bool
elm_gengrid_filled_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   ELM_GENGRID_DATA_GET(obj, sd);

   return sd->filled;
}

EAPI unsigned int
elm_gengrid_items_count(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) 0;
   ELM_GENGRID_DATA_GET(obj, sd);

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

EAPI void
elm_gengrid_select_mode_set(Evas_Object *obj,
                            Elm_Object_Select_Mode mode)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;

   if (sd->select_mode != mode)
     sd->select_mode = mode;
}

EAPI Elm_Object_Select_Mode
elm_gengrid_select_mode_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) ELM_OBJECT_SELECT_MODE_MAX;
   ELM_GENGRID_DATA_GET(obj, sd);

   return sd->select_mode;
}

EAPI void
elm_gengrid_highlight_mode_set(Evas_Object *obj,
                               Eina_Bool highlight)
{
   ELM_GENGRID_CHECK(obj);
   ELM_GENGRID_DATA_GET(obj, sd);

   sd->highlight = !!highlight;
}

EAPI Eina_Bool
elm_gengrid_highlight_mode_get(const Evas_Object *obj)
{
   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   ELM_GENGRID_DATA_GET(obj, sd);

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
   if (!it) return;

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

   if (!it) return ELM_OBJECT_SELECT_MODE_MAX;
   return it->select_mode;
}
