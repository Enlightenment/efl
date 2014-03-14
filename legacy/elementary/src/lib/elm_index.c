#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "els_box.h"
#include "elm_widget_layout.h"
#include "elm_widget_index.h"

EAPI Eo_Op ELM_OBJ_INDEX_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_INDEX_CLASS

#define MY_CLASS_NAME "Elm_Index"
#define MY_CLASS_NAME_LEGACY "elm_index"

#define INDEX_DELAY_CHANGE_TIME 0.2

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const char SIG_SELECTED[] = "selected";
static const char SIG_LEVEL_UP[] = "level,up";
static const char SIG_LEVEL_DOWN[] = "level,down";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_DELAY_CHANGED, ""},
   {SIG_SELECTED, ""},
   {SIG_LEVEL_UP, ""},
   {SIG_LEVEL_DOWN, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void
_item_free(Elm_Index_Item *it)
{
   ELM_INDEX_DATA_GET(WIDGET(it), sd);

   sd->items = eina_list_remove(sd->items, it);

   if (it->omitted)
     it->omitted = eina_list_free(it->omitted);

   ELM_SAFE_FREE(it->letter, eina_stringshare_del);
}

static void
_box_custom_layout(Evas_Object *o,
                   Evas_Object_Box_Data *priv,
                   void *data)
{
   Elm_Index_Smart_Data *sd = data;

   _els_box_layout(o, priv, sd->horizontal, EINA_TRUE, EINA_FALSE);
}

static void
_index_box_clear(Evas_Object *obj,
                 int level)
{
   Eina_List *l;
   Elm_Index_Item *it;

   ELM_INDEX_DATA_GET(obj, sd);

   if (!sd->level_active[level]) return;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->level != level) continue;
        ELM_SAFE_FREE(VIEW(it), evas_object_del);
     }

   sd->level_active[level] = EINA_FALSE;
}

static void
_access_index_register(Evas_Object *obj)
{
   Evas_Object *ao;
   elm_widget_can_focus_set(obj, EINA_TRUE);
   ao = _elm_access_edje_object_part_object_register
              (obj, elm_layout_edje_get(obj), "access");
   _elm_access_text_set
     (_elm_access_info_get(ao), ELM_ACCESS_TYPE, E_("Index"));
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   const char *txt = NULL;

   Elm_Index_Item *it = (Elm_Index_Item *)data;
   ELM_INDEX_ITEM_CHECK_OR_RETURN(it, NULL);

   txt = elm_widget_access_info_get(obj);
   if (!txt) txt = it->letter;
   if (txt) return strdup(txt);

   return NULL;
}

static void
_access_widget_item_register(Elm_Index_Item *it)
{
   Elm_Access_Info *ai;

   _elm_access_widget_item_register((Elm_Widget_Item *)it);

   ai = _elm_access_info_get(it->base.access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Index Item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
}

static void
_omit_calc(void *data, int num_of_items, int max_num_of_items)
{
   Elm_Index_Smart_Data *sd = data;
   int max_group_num, num_of_extra_items, i, g, size, sum, *group_pos, *omit_info;
   Elm_Index_Omit *o;

   if ((max_num_of_items < 3) || (num_of_items <= max_num_of_items)) return;

   max_group_num = (max_num_of_items - 1) / 2;
   num_of_extra_items = num_of_items - max_num_of_items;

   group_pos = (int *)malloc(sizeof(int) * max_group_num);
   omit_info = (int *)malloc(sizeof(int) * max_num_of_items);

   if (num_of_extra_items >= max_group_num)
     {
        g = 1;
        for (i = 0; i < max_group_num; i++)
          {
             group_pos[i] = g;
             g += 2;
          }
     }
   else
     {
        size = max_num_of_items / (num_of_extra_items + 1);
        g = size;
        for (i = 0; i < num_of_extra_items; i++)
          {
             group_pos[i] = g;
             g += size;
          }
     }
   for (i = 0; i < max_num_of_items; i++)
     omit_info[i] = 1;
   for (i = 0; i < num_of_extra_items; i++)
     omit_info[group_pos[i % max_group_num]]++;

   sum = 0;
   for (i = 0; i < max_num_of_items; i++)
     {
        if (omit_info[i] > 1)
          {
             o = (Elm_Index_Omit *)malloc(sizeof(Elm_Index_Omit));
             o->offset = sum;
             o->count = omit_info[i];
             sd->omit = eina_list_append(sd->omit, o);
          }
        sum += omit_info[i];
     }

   free(group_pos);
   free(omit_info);
}

// FIXME: always have index filled
static void
_index_box_auto_fill(Evas_Object *obj,
                     int level)
{
   int i = 0, max_num_of_items = 0, num_of_items = 0, g = 0, skip = 0;
   Eina_List *l;
   Eina_Bool rtl;
   Elm_Index_Item *it, *head = NULL;
   Evas_Coord mw, mh, ih;
   Evas_Object *o;
   Elm_Index_Omit *om;
   const char *style = elm_widget_style_get(obj);

   ELM_INDEX_DATA_GET(obj, sd);

   if (sd->level_active[level]) return;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_geometry_get(wd->resize_obj, NULL, NULL, NULL, &ih);

   rtl = elm_widget_mirrored_get(obj);

   EINA_LIST_FREE(sd->omit, om)
     free(om);

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->omitted)
          it->omitted = eina_list_free(it->omitted);
        if (it->head) it->head = NULL;
     }

   if (sd->omit_enabled)
     {
        o = edje_object_add(evas_object_evas_get(obj));
        elm_widget_theme_object_set
           (obj, o, "index", "item/vertical", style);

        edje_object_size_min_restricted_calc(o, NULL, &mh, 0, 0);

        evas_object_del(o);

        EINA_LIST_FOREACH(sd->items, l, it)
           if (it->level == level) num_of_items++;

        if (mh != 0)
          max_num_of_items = ih / mh;

        _omit_calc(sd, num_of_items, max_num_of_items);
     }

   om = eina_list_nth(sd->omit, g);
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        const char *stacking;

        if (it->level != level) continue;

        if ((om) && (i == om->offset))
          {
             skip = om->count;
             skip--;
             head = it;
             it->head = head;
             head->omitted = eina_list_append(head->omitted, it);
             om = eina_list_nth(sd->omit, ++g);
          }
        else if (skip > 0)
          {
             skip--;
             i++;
             if (head)
               {
                  it->head = head;
                  head->omitted = eina_list_append(head->omitted, it);
                  // if it is selected but omitted, send signal to it's head
                  if (it->selected)
                    edje_object_signal_emit(VIEW(it->head), "elm,state,active", "elm");
               }
             continue;
          }

        o = edje_object_add(evas_object_evas_get(obj));
        VIEW(it) = o;
        edje_object_mirrored_set(VIEW(it), rtl);

        if (sd->horizontal)
          {
             if (i & 0x1)
               elm_widget_theme_object_set
                 (obj, o, "index", "item_odd/horizontal", style);
             else
               elm_widget_theme_object_set
                 (obj, o, "index", "item/horizontal", style);
          }
        else
          {
             if (i & 0x1)
               elm_widget_theme_object_set
                 (obj, o, "index", "item_odd/vertical", style);
             else
               elm_widget_theme_object_set
                 (obj, o, "index", "item/vertical", style);
          }

        if (skip > 0)
          edje_object_part_text_escaped_set(o, "elm.text", "*");
        else
          edje_object_part_text_escaped_set(o, "elm.text", it->letter);
        edje_object_size_min_restricted_calc(o, &mw, &mh, 0, 0);
        evas_object_size_hint_min_set(o, mw, mh);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_widget_sub_object_add(obj, o);
        evas_object_box_append(sd->bx[level], o);
        stacking = edje_object_data_get(o, "stacking");

        if (it->selected)
          edje_object_signal_emit(o, "elm,state,active", "elm");

        if (stacking)
          {
             if (!strcmp(stacking, "below")) evas_object_lower(o);
             else if (!strcmp(stacking, "above"))
               evas_object_raise(o);
          }

        evas_object_show(o);

        i++;

        // ACCESS
        if ((it->level == 0) &&
            (_elm_config->access_mode == ELM_ACCESS_MODE_ON))
          _access_widget_item_register(it);
     }

   evas_object_smart_calculate(sd->bx[level]);
   sd->level_active[level] = EINA_TRUE;
}

static void
_elm_index_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord minw = 0, minh = 0;
   Elm_Index_Item *it;

   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Index_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   ELM_LAYOUT_DATA_GET(obj, ld);

   _index_box_clear(obj, 0);
   _index_box_clear(obj, 1);

   if (sd->horizontal)
     eina_stringshare_replace(&ld->group, "base/horizontal");
   else
     eina_stringshare_replace(&ld->group, "base/vertical");

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->event_rect[0], minw, minh);

   if (edje_object_part_exists
         (wd->resize_obj, "elm.swallow.index.1"))
     {
        if (!sd->bx[1])
          {
             sd->bx[1] = evas_object_box_add(evas_object_evas_get(obj));
             evas_object_box_layout_set
               (sd->bx[1], _box_custom_layout, sd, NULL);
             elm_widget_sub_object_add(obj, sd->bx[1]);
          }
        elm_layout_content_set(obj, "elm.swallow.index.1", sd->bx[1]);
     }
   else
     ELM_SAFE_FREE(sd->bx[1], evas_object_del);

   if (edje_object_part_exists
         (wd->resize_obj, "elm.swallow.event.1"))
     {
        if (!sd->event_rect[1])
          {
             sd->event_rect[1] =
               evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_color_set(sd->event_rect[1], 0, 0, 0, 0);
             elm_widget_sub_object_add(obj, sd->event_rect[1]);
          }
        elm_layout_content_set(obj, "elm.swallow.event.1", sd->event_rect[1]);
        evas_object_size_hint_min_set(sd->event_rect[1], minw, minh);
     }
   else
     ELM_SAFE_FREE(sd->event_rect[1], evas_object_del);

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);
   _index_box_auto_fill(obj, 0);

   if (sd->autohide_disabled)
     {
        if (sd->level == 1) _index_box_auto_fill(obj, 1);
        elm_layout_signal_emit(obj, "elm,state,active", "elm");
     }
   else elm_layout_signal_emit(obj, "elm,state,inactive", "elm");

   it = (Elm_Index_Item *)elm_index_selected_item_get(obj, sd->level);
   if (it)
     {
        if (it->head)
          edje_object_signal_emit(VIEW(it->head), "elm,state,active", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,active", "elm");
     }

   if (ret) *ret = EINA_TRUE;

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_index_register(obj);
}

static void
_elm_index_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   ELM_INDEX_DATA_GET(WIDGET(it), sd);

   _item_free((Elm_Index_Item *)it);
   _index_box_clear(WIDGET(it), sd->level);

   return EINA_TRUE;
}

static Elm_Index_Item *
_item_new(Evas_Object *obj,
          const char *letter,
          Evas_Smart_Cb func,
          const void *data)
{
   Elm_Index_Item *it;

   ELM_INDEX_DATA_GET(obj, sd);

   it = elm_widget_item_new(obj, Elm_Index_Item);
   if (!it) return NULL;

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   if (letter) it->letter = eina_stringshare_add(letter);
   it->func = func;
   it->base.data = data;
   it->level = sd->level;

   return it;
}

static Elm_Index_Item *
_item_find(Evas_Object *obj,
           const void *data)
{
   Eina_List *l;
   Elm_Index_Item *it;

   ELM_INDEX_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->items, l, it)
     if (it->base.data == data) return it;

   return NULL;
}

static Eina_Bool
_delay_change_cb(void *data)
{
   Elm_Object_Item *item;

   ELM_INDEX_DATA_GET(data, sd);

   sd->delay = NULL;
   item = elm_index_selected_item_get(data, sd->level);
   if (item) evas_object_smart_callback_call(data, SIG_DELAY_CHANGED, item);

   return ECORE_CALLBACK_CANCEL;
}

static void
_sel_eval(Evas_Object *obj,
          Evas_Coord evx,
          Evas_Coord evy)
{
   Evas_Coord x, y, w, h, bx, by, bw, bh, xx, yy;
   Elm_Index_Item *it, *it_closest, *it_last, *om_closest;
   char *label = NULL, *last = NULL;
   double cdv = 0.5;
   Evas_Coord dist;
   Eina_List *l;
   int i, j, size, dh, dx, dy;

   ELM_INDEX_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   for (i = 0; i <= sd->level; i++)
     {
        it_last = NULL;
        it_closest = NULL;
        om_closest = NULL;
        dist = 0x7fffffff;
        evas_object_geometry_get(sd->bx[i], &bx, &by, &bw, &bh);

        EINA_LIST_FOREACH(sd->items, l, it)
          {
             if (it->level != i) continue;
             if (it->level != sd->level)
               {
                  if (it->selected)
                    {
                       it_closest = it;
                       break;
                    }
                  continue;
               }
             if (it->selected)
               {
                  it_last = it;
                  it->selected = EINA_FALSE;
               }
             if (VIEW(it))
               {
                  evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
                  xx = x + (w / 2);
                  yy = y + (h / 2);
                  x = evx - xx;
                  y = evy - yy;
                  x = (x * x) + (y * y);
                  if ((x < dist) || (!it_closest))
                    {
                       if (sd->horizontal)
                         cdv = (double)(xx - bx) / (double)bw;
                       else
                         cdv = (double)(yy - by) / (double)bh;
                       it_closest = it;
                       dist = x;
                    }
               }
          }
        if ((i == 0) && (sd->level == 0))
          edje_object_part_drag_value_set
            (wd->resize_obj, "elm.dragable.index.1", cdv, cdv);

        if (it_closest && it_closest->omitted)
          {
             it = it_closest;
             size = eina_list_count(it->omitted);
             evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
             dist = 0x7fffffff;
             dh = h / size;
             if (dh == 0)
               WRN("too many index items to omit."); //FIXME
             else
               {
                  for (j = 0; j < size; j++)
                    {
                       xx = x + (w / 2);
                       yy = y + (dh * j) + (dh / 2);
                       dx = evx - xx;
                       dy = evy - yy;
                       dx = (dx * dx) + (dy * dy);
                       if ((dx < dist) || (!om_closest))
                         {
                            om_closest = eina_list_nth(it->omitted, j);
                            dist = dx;
                         }
                    }
               }
          }

        if (om_closest) om_closest->selected = EINA_TRUE;
        else if (it_closest) it_closest->selected = EINA_TRUE;

        if (it_closest != it_last)
          {
             if (it_last)
               {
                  const char *stacking, *selectraise;

                  it = it_last;
                  if (it->head)
                    {
                       if (it->head != it_closest) it = it->head;
                       else it = NULL;
                    }
                  if (it)
                    {
                       edje_object_signal_emit
                          (VIEW(it), "elm,state,inactive", "elm");
                       stacking = edje_object_data_get(VIEW(it), "stacking");
                       selectraise = edje_object_data_get(VIEW(it), "selectraise");
                       if ((selectraise) && (!strcmp(selectraise, "on")))
                         {
                            if ((stacking) && (!strcmp(stacking, "below")))
                              evas_object_lower(VIEW(it));
                         }
                    }
               }
             if (it_closest)
               {
                  const char *selectraise;

                  it = it_closest;

                  if (!((it_last) && (it_last->head) && (it_last->head == it_closest)))
                    {
                       edje_object_signal_emit(VIEW(it), "elm,state,active", "elm");
                       selectraise = edje_object_data_get(VIEW(it), "selectraise");
                       if ((selectraise) && (!strcmp(selectraise, "on")))
                         evas_object_raise(VIEW(it));
                    }

                  // ACCESS
                  if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
                    {
                       char *ret;
                       Eina_Strbuf *buf;
                       buf = eina_strbuf_new();

                       if (om_closest)
                         eina_strbuf_append_printf(buf, "index item %s clicked", om_closest->letter);
                       else
                         eina_strbuf_append_printf(buf, "index item %s clicked", it->letter);
                       ret = eina_strbuf_string_steal(buf);
                       eina_strbuf_free(buf);

                       _elm_access_highlight_set(it->base.access_obj);
                       _elm_access_say(ret);
                    }

                  if (om_closest)
                    evas_object_smart_callback_call
                       (obj, SIG_CHANGED, om_closest);
                  else
                    evas_object_smart_callback_call
                       (obj, SIG_CHANGED, it);
                  ecore_timer_del(sd->delay);
                  sd->delay = ecore_timer_add(sd->delay_change_time,
                                              _delay_change_cb, obj);
               }
          }
        if (it_closest)
          {
             if (om_closest) it = om_closest;
             else it = it_closest;
             if (!last && it->letter) last = strdup(it->letter);
             else
               {
                  if (!label && last) label = strdup(last);
                  else
                    {
                       if (label && last)
                         {
                            label = realloc(label, strlen(label) +
                                            strlen(last) + 1);
                            if (!label) return;
                            strcat(label, last);
                         }
                    }
                  free(last);
                  if (it->letter) last = strdup(it->letter);
               }
          }
     }
   if (!label) label = strdup("");
   if (!last) last = strdup("");

   elm_layout_text_set(obj, "elm.text.body", label);
   elm_layout_text_set(obj, "elm.text", last);

   free(label);
   free(last);
}

static void
_on_mouse_down(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *o EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w;

   ELM_INDEX_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   if (ev->button != 1) return;
   sd->down = 1;
   evas_object_geometry_get(wd->resize_obj, &x, &y, &w, NULL);
   sd->dx = ev->canvas.x - x;
   sd->dy = ev->canvas.y - y;
   if (!sd->autohide_disabled)
     {
        _index_box_clear(data, 1);
        elm_layout_signal_emit(data, "elm,state,active", "elm");
     }
   _sel_eval(data, ev->canvas.x, ev->canvas.y);
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.pointer",
     (!elm_object_mirrored_get(data)) ? sd->dx : (sd->dx - w), sd->dy);
   if (sd->items && !sd->indicator_disabled)
     elm_layout_signal_emit(data, "elm,indicator,state,active", "elm");
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *o EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Elm_Object_Item *item;
   Elm_Index_Item *id_item;

   ELM_INDEX_DATA_GET(data, sd);

   if (ev->button != 1) return;
   sd->down = 0;
   item = elm_index_selected_item_get(data, sd->level);
   if (item)
     {
        evas_object_smart_callback_call(data, SIG_SELECTED, item);
        id_item = (Elm_Index_Item *)item;
        if (id_item->func)
          id_item->func((void *)id_item->base.data, WIDGET(id_item), id_item);
     }
   if (!sd->autohide_disabled)
     elm_layout_signal_emit(data, "elm,state,inactive", "elm");

   elm_layout_signal_emit(data, "elm,state,level,0", "elm");
   if (sd->items && !sd->indicator_disabled)
     elm_layout_signal_emit(data, "elm,indicator,state,inactive", "elm");
}

static void
_on_mouse_move(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *o EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, adx, w;
   char buf[1024];

   ELM_INDEX_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   if (!sd->down) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(wd->resize_obj, &x, &y, &w, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - sd->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.pointer",
     (!edje_object_mirrored_get(wd->resize_obj)) ?
     x : (x - w), y);
   if (!sd->horizontal)
     {
        if (adx > minw)
          {
             if (!sd->level)
               {
                  sd->level = 1;
                  snprintf(buf, sizeof(buf), "elm,state,level,%i", sd->level);
                  elm_layout_signal_emit(data, buf, "elm");
                  evas_object_smart_callback_call(data, SIG_LEVEL_UP, NULL);
               }
          }
        else
          {
             if (sd->level == 1)
               {
                  sd->level = 0;
                  snprintf(buf, sizeof(buf), "elm,state,level,%i", sd->level);
                  elm_layout_signal_emit(data, buf, "elm");
                  evas_object_smart_callback_call(data, SIG_LEVEL_DOWN, NULL);
               }
          }
     }
   _sel_eval(data, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_on_mouse_in_access(void *data,
                    Evas *e EINA_UNUSED,
                    Evas_Object *o EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   ELM_INDEX_DATA_GET(data, sd);

   if (sd->down) return;

   if (!sd->autohide_disabled)
     {
        _index_box_clear(data, 1);
        elm_layout_signal_emit(data, "elm,state,active", "elm");
     }
}

static void
_on_mouse_move_access(void *data,
                      Evas *e EINA_UNUSED,
                      Evas_Object *o EINA_UNUSED,
                      void *event_info)
{

   Evas_Event_Mouse_Down *ev = event_info;
   Elm_Index_Item *it, *it_closest;
   Eina_List *l;
   Evas_Coord dist = 0;
   Evas_Coord x, y, w, h, xx, yy;

   ELM_INDEX_DATA_GET(data, sd);

   it_closest = NULL;
   dist = 0x7fffffff;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
        xx = x + (w / 2);
        yy = y + (h / 2);
        x = ev->canvas.x - xx;
        y = ev->canvas.y - yy;
        x = (x * x) + (y * y);
        if ((x < dist) || (!it_closest))
          {
             it_closest = it;
             dist = x;
          }
     }

   if (it_closest)
     _elm_access_highlight_set(it_closest->base.access_obj);
}

static void
_on_mouse_out_access(void *data,
                      Evas *e EINA_UNUSED,
                      Evas_Object *o EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   ELM_INDEX_DATA_GET(data, sd);

   if (!sd->autohide_disabled)
     elm_layout_signal_emit(data, "elm,state,inactive", "elm");
}

static void
_index_resize_cb(void *data EINA_UNUSED,
                 Evas *e EINA_UNUSED,
                 Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   ELM_INDEX_DATA_GET_OR_RETURN(obj, sd);

   if (!sd->omit_enabled) return;

   Elm_Index_Item *it;

   _index_box_clear(obj, 0);
   _index_box_auto_fill(obj, 0);

   it = (Elm_Index_Item *)elm_index_selected_item_get(obj, sd->level);
   if (it)
     {
        if (it->head)
          edje_object_signal_emit(VIEW(it->head), "elm,state,active", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,active", "elm");
     }
}

static void
_elm_index_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Object *o;
   Evas_Coord minw = 0, minh = 0;

   Elm_Index_Smart_Data *priv = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set
       (obj, "index", "base/vertical", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _index_resize_cb, NULL);

   priv->event_rect[0] = o = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(o, 0, 0, 0, 0);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(o, minw, minh);
   elm_layout_content_set(obj, "elm.swallow.event.0", o);

   evas_object_event_callback_add
     (o, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, obj);
   evas_object_event_callback_add
     (o, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up, obj);
   evas_object_event_callback_add
     (o, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, obj);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        evas_object_event_callback_add
          (o, EVAS_CALLBACK_MOUSE_IN, _on_mouse_in_access, obj);
        evas_object_event_callback_add
          (o, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move_access, obj);
        evas_object_event_callback_add
          (o, EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out_access, obj);
     }

   if (edje_object_part_exists
         (wd->resize_obj, "elm.swallow.event.1"))
     {
        o = evas_object_rectangle_add(evas_object_evas_get(obj));
        priv->event_rect[1] = o;
        evas_object_color_set(o, 0, 0, 0, 0);
        evas_object_size_hint_min_set(o, minw, minh);
        elm_layout_content_set(obj, "elm.swallow.event.1", o);
     }

   priv->bx[0] = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_box_layout_set(priv->bx[0], _box_custom_layout, priv, NULL);
   elm_layout_content_set(obj, "elm.swallow.index.0", priv->bx[0]);
   evas_object_show(priv->bx[0]);

   priv->delay_change_time = INDEX_DELAY_CHANGE_TIME;

   if (edje_object_part_exists
         (wd->resize_obj, "elm.swallow.index.1"))
     {
        priv->bx[1] = evas_object_box_add(evas_object_evas_get(obj));
        evas_object_box_layout_set
          (priv->bx[1], _box_custom_layout, priv, NULL);
        elm_widget_sub_object_add(obj, priv->bx[1]);
        elm_layout_content_set(obj, "elm.swallow.index.1", priv->bx[1]);
        evas_object_show(priv->bx[1]);
     }

   elm_layout_sizing_eval(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_index_register(obj);
}

static void
_elm_index_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Index_Omit *o;

   Elm_Index_Smart_Data *sd = _pd;

   while (sd->items)
     elm_widget_item_del(sd->items->data);

   EINA_LIST_FREE(sd->omit, o)
     free(o);

   ecore_timer_del(sd->delay);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static Eina_Bool _elm_index_smart_focus_next_enable = EINA_FALSE;

static void
_elm_index_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = _elm_index_smart_focus_next_enable;
}

static void
_elm_index_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_index_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Eina_List *items = NULL;
   Eina_List *l = NULL;
   Elm_Index_Item *it;
   Evas_Object *ao;
   Evas_Object *po;

   Elm_Index_Smart_Data *sd = _pd;

   if (!sd->autohide_disabled)
     elm_layout_signal_emit((Evas_Object *)obj, "elm,state,active", "elm");

   po = (Evas_Object *)edje_object_part_object_get
              (elm_layout_edje_get(obj), "access");
   ao = evas_object_data_get(po, "_part_access_obj");
   items = eina_list_append(items, ao);

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->level != 0) continue;
        items = eina_list_append(items, it->base.access_obj);
     }

   int_ret = elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);

   // to hide index item, if there is nothing to focus on autohide disable mode
   if ((!sd->autohide_disabled) && (!int_ret))
     elm_layout_signal_emit((Evas_Object *)obj, "elm,state,inactive", "elm");

   if (ret) *ret = int_ret;
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Eina_List *l;
   Elm_Index_Item *it;

   ELM_INDEX_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->level != 0) continue;
        if (is_access) _access_widget_item_register(it);
        else _elm_access_widget_item_unregister((Elm_Widget_Item *)it);
     }

   if (is_access)
     {
        _access_index_register(obj);

        evas_object_event_callback_add
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_IN, _on_mouse_in_access, obj);
        evas_object_event_callback_add
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move_access, obj);
        evas_object_event_callback_add
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out_access, obj);
     }
   else
     {
        // opposition of  _access_index_register();
        elm_widget_can_focus_set(obj, EINA_FALSE);
        _elm_access_edje_object_part_object_unregister
             (obj, elm_layout_edje_get(obj), "access");

        evas_object_event_callback_del_full
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_IN, _on_mouse_in_access, obj);
        evas_object_event_callback_del_full
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move_access, obj);
        evas_object_event_callback_del_full
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out_access, obj);
     }
}

static void
_elm_index_smart_access(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   _elm_index_smart_focus_next_enable = va_arg(*list, int);
   _access_obj_process(obj, _elm_index_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_index_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_index_autohide_disabled_set(Evas_Object *obj,
                                Eina_Bool disabled)
{
   ELM_INDEX_CHECK(obj);
   eo_do(obj, elm_obj_index_autohide_disabled_set(disabled));
}

static void
_autohide_disabled_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool disabled = va_arg(*list, int);
   Elm_Index_Smart_Data *sd = _pd;

   disabled = !!disabled;
   if (sd->autohide_disabled == disabled) return;
   sd->autohide_disabled = disabled;
   sd->level = 0;
   if (sd->autohide_disabled)
     {
        _index_box_clear(obj, 1);
        elm_layout_signal_emit(obj, "elm,state,active", "elm");
     }
   else
     elm_layout_signal_emit(obj, "elm,state,inactive", "elm");

   //FIXME: Should be update indicator based on the indicator visibility
}

EAPI Eina_Bool
elm_index_autohide_disabled_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_index_autohide_disabled_get(&ret));
   return ret;
}

static void
_autohide_disabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Index_Smart_Data *sd = _pd;
   *ret = sd->autohide_disabled;
}

EAPI void
elm_index_item_level_set(Evas_Object *obj,
                         int level)
{
   ELM_INDEX_CHECK(obj);
   eo_do(obj, elm_obj_index_item_level_set(level));
}

static void
_item_level_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int level = va_arg(*list, int);
   Elm_Index_Smart_Data *sd = _pd;

   if (sd->level == level) return;
   sd->level = level;
}

EAPI int
elm_index_item_level_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_obj_index_item_level_get(&ret));
   return ret;
}

static void
_item_level_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Index_Smart_Data *sd = _pd;
   *ret = sd->level;
}

//FIXME: Should update indicator based on the autohidden status & indicator visibility
EAPI void
elm_index_item_selected_set(Elm_Object_Item *it,
                            Eina_Bool selected)
{
   Elm_Index_Item *it_sel, *it_last, *it_inactive, *it_active;
   Evas_Object *obj = WIDGET(it);

   ELM_INDEX_ITEM_CHECK_OR_RETURN(it);
   ELM_INDEX_DATA_GET(obj, sd);

   selected = !!selected;
   it_sel = (Elm_Index_Item *)it;
   if (it_sel->selected == selected) return;

   if (selected)
     {
        it_last = (Elm_Index_Item *)elm_index_selected_item_get(obj, sd->level);

        if (it_last)
          {
             it_last->selected = EINA_FALSE;
             if (it_last->head)
               it_inactive = it_last->head;
             else
               it_inactive = it_last;

             edje_object_signal_emit(VIEW(it_inactive),
                                     "elm,state,inactive", "elm");
             edje_object_message_signal_process(VIEW(it_inactive));
          }

        it_sel->selected = EINA_TRUE;
        if (it_sel->head)
          it_active = it_sel->head;
        else
          it_active = it_sel;

        edje_object_signal_emit(VIEW(it_active), "elm,state,active", "elm");
        edje_object_message_signal_process(VIEW(it_active));

        evas_object_smart_callback_call
           (obj, SIG_CHANGED, it);
        evas_object_smart_callback_call
           (obj, SIG_SELECTED, it);
        ecore_timer_del(sd->delay);
        sd->delay = ecore_timer_add(sd->delay_change_time,
                                    _delay_change_cb, obj);
     }
   else
     {
        it_sel->selected = EINA_FALSE;
        if (it_sel->head)
          it_inactive = it_sel->head;
        else
          it_inactive = it_sel;

        edje_object_signal_emit(VIEW(it_inactive), "elm,state,inactive", "elm");
        edje_object_message_signal_process(VIEW(it_inactive));

        // for the case in which the selected item is unselected before mouse up
        elm_layout_signal_emit(obj, "elm,indicator,state,inactive", "elm");
     }
}

EAPI Elm_Object_Item *
elm_index_selected_item_get(const Evas_Object *obj,
                            int level)
{
   ELM_INDEX_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_index_selected_item_get(level, &ret));
   return ret;
}

static void
_selected_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int level = va_arg(*list, int);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);

   Eina_List *l;
   Elm_Index_Item *it;

   Elm_Index_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if ((it->selected) && (it->level == level))
          {
             *ret = (Elm_Object_Item *)it;
             return;
          }
     }

   *ret = NULL;
}

EAPI Elm_Object_Item *
elm_index_item_append(Evas_Object *obj,
                      const char *letter,
                      Evas_Smart_Cb func,
                      const void *data)
{
   ELM_INDEX_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_index_item_append(letter, func, data, &ret));
   return ret;
}

static void
_item_append(Eo *obj, void *_pd, va_list *list)
{
   const char *letter = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Index_Item *it;

   Elm_Index_Smart_Data *sd = _pd;

   it = _item_new(obj, letter, func, data);
   if (!it) return;

   sd->items = eina_list_append(sd->items, it);
   _index_box_clear(obj, sd->level);

   *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_prepend(Evas_Object *obj,
                       const char *letter,
                       Evas_Smart_Cb func,
                       const void *data)
{
   ELM_INDEX_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_index_item_prepend(letter, func, data, &ret));
   return ret;
}

static void
_item_prepend(Eo *obj, void *_pd, va_list *list)
{
   const char *letter = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Index_Item *it;

   Elm_Index_Smart_Data *sd = _pd;

   it = _item_new(obj, letter, func, data);
   if (!it) return;

   sd->items = eina_list_prepend(sd->items, it);
   _index_box_clear(obj, sd->level);

   *ret = (Elm_Object_Item *)it;
}

EINA_DEPRECATED EAPI Elm_Object_Item *
elm_index_item_prepend_relative(Evas_Object *obj,
                                const char *letter,
                                const void *item,
                                const Elm_Object_Item *relative)
{
   return elm_index_item_insert_before
            (obj, (Elm_Object_Item *)relative, letter, NULL, item);
}

EAPI Elm_Object_Item *
elm_index_item_insert_after(Evas_Object *obj,
                            Elm_Object_Item *after,
                            const char *letter,
                            Evas_Smart_Cb func,
                            const void *data)
{
   ELM_INDEX_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_index_item_insert_after(after, letter, func, data, &ret));
   return ret;
}

static void
_item_insert_after(Eo *obj, void *_pd, va_list *list)
{
   Elm_Object_Item *after = va_arg(*list, Elm_Object_Item *);
   const char *letter = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Index_Item *it;

   Elm_Index_Smart_Data *sd = _pd;

   if (!after)
     {
        *ret = elm_index_item_append(obj, letter, func, data);
        return;
     }

   it = _item_new(obj, letter, func, data);
   if (!it) return;

   sd->items = eina_list_append_relative(sd->items, it, after);
   _index_box_clear(obj, sd->level);

   *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_insert_before(Evas_Object *obj,
                             Elm_Object_Item *before,
                             const char *letter,
                             Evas_Smart_Cb func,
                             const void *data)
{
   ELM_INDEX_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_index_item_insert_before(before, letter, func, data, &ret));
   return ret;
}

static void
_item_insert_before(Eo *obj, void *_pd, va_list *list)
{
   Elm_Object_Item *before = va_arg(*list, Elm_Object_Item *);
   const char *letter = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Index_Item *it;

   Elm_Index_Smart_Data *sd = _pd;

   if (!before)
     {
        *ret = elm_index_item_prepend(obj, letter, func, data);
        return;
     }

   it = _item_new(obj, letter, func, data);
   if (!it) return;

   sd->items = eina_list_prepend_relative(sd->items, it, before);
   _index_box_clear(obj, sd->level);

   *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_sorted_insert(Evas_Object *obj,
                             const char *letter,
                             Evas_Smart_Cb func,
                             const void *data,
                             Eina_Compare_Cb cmp_func,
                             Eina_Compare_Cb cmp_data_func)
{
   ELM_INDEX_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_index_item_sorted_insert(letter, func, data, cmp_func, cmp_data_func, &ret));
   return ret;
}

static void
_item_sorted_insert(Eo *obj, void *_pd, va_list *list)
{
   const char *letter = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Eina_Compare_Cb cmp_func = va_arg(*list, Eina_Compare_Cb);
   Eina_Compare_Cb cmp_data_func = va_arg(*list, Eina_Compare_Cb);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Index_Item *it;
   Eina_List *lnear;
   int cmp;

   Elm_Index_Smart_Data *sd = _pd;

   if (!(sd->items))
     {
        *ret = elm_index_item_append(obj, letter, func, data);
        return;
     }

   it = _item_new(obj, letter, func, data);
   if (!it) return;

   lnear = eina_list_search_sorted_near_list(sd->items, cmp_func, it, &cmp);
   if (cmp < 0)
     sd->items = eina_list_append_relative_list(sd->items, it, lnear);
   else if (cmp > 0)
     sd->items = eina_list_prepend_relative_list(sd->items, it, lnear);
   else
     {
        /* If cmp_data_func is not provided, append a duplicated item */
        if (!cmp_data_func)
          sd->items = eina_list_append_relative_list(sd->items, it, lnear);
        else
          {
             Elm_Index_Item *p_it = eina_list_data_get(lnear);
             if (cmp_data_func(p_it->base.data, it->base.data) >= 0)
               p_it->base.data = it->base.data;
             elm_widget_item_del(it);
             it = NULL;
          }
     }
   _index_box_clear(obj, sd->level);

   if (!it)
     *ret = NULL;
   else
     *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_find(Evas_Object *obj,
                    const void *data)
{
   ELM_INDEX_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_index_item_find(data, &ret));
   return ret;
}

static void
_elm_index_item_find(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = (Elm_Object_Item *)_item_find(obj, data);
}

EAPI void
elm_index_item_clear(Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj);
   eo_do(obj, elm_obj_index_item_clear());
}

static void
_item_clear(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Index_Item *it;
   Eina_List *l, *clear = NULL;

   Elm_Index_Smart_Data *sd = _pd;

   _index_box_clear(obj, sd->level);
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->level != sd->level) continue;
        clear = eina_list_append(clear, it);
     }
   EINA_LIST_FREE(clear, it)
     elm_widget_item_del(it);
}

EAPI void
elm_index_level_go(Evas_Object *obj,
                   int level)
{
   ELM_INDEX_CHECK(obj);
   eo_do(obj, elm_obj_index_level_go(level));
}

static void
_level_go(Eo *obj, void *_pd, va_list *list)
{
   int level = va_arg(*list, int);
   (void) level;
   Elm_Index_Smart_Data *sd = _pd;
   _index_box_clear(obj, 0);
   _index_box_auto_fill(obj, 0);
   if (sd->level == 1)
     {
        _index_box_clear(obj, 1);
        _index_box_auto_fill(obj, 1);
     }
}

EAPI void
elm_index_indicator_disabled_set(Evas_Object *obj,
                                 Eina_Bool disabled)
{
   ELM_INDEX_CHECK(obj);
   eo_do(obj, elm_obj_index_indicator_disabled_set(disabled));
}

static void
_indicator_disabled_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool disabled = va_arg(*list, int);
   Elm_Index_Smart_Data *sd = _pd;

   disabled = !!disabled;
   if (sd->indicator_disabled == disabled) return;
   sd->indicator_disabled = disabled;
   if (!sd->items) return;
   if (disabled)
     elm_layout_signal_emit(obj, "elm,indicator,state,inactive", "elm");
   else
     elm_layout_signal_emit(obj, "elm,indicator,state,active", "elm");
}

EAPI Eina_Bool
elm_index_indicator_disabled_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_index_indicator_disabled_get(&ret));
   return ret;
}

static void
_indicator_disabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Index_Smart_Data *sd = _pd;
   *ret = sd->indicator_disabled;
}

EAPI const char *
elm_index_item_letter_get(const Elm_Object_Item *it)
{
   ELM_INDEX_ITEM_CHECK_OR_RETURN(it, NULL);

   return ((Elm_Index_Item *)it)->letter;
}

EAPI void
elm_index_horizontal_set(Evas_Object *obj,
                         Eina_Bool horizontal)
{
   ELM_INDEX_CHECK(obj);
   eo_do(obj, elm_obj_index_horizontal_set(horizontal));
}

static void
_horizontal_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool horizontal = va_arg(*list, int);
   Elm_Index_Smart_Data *sd = _pd;

   horizontal = !!horizontal;
   if (horizontal == sd->horizontal) return;

   sd->horizontal = horizontal;
   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

EAPI Eina_Bool
elm_index_horizontal_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_index_horizontal_get(&ret));
   return ret;
}

static void
_horizontal_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Index_Smart_Data *sd = _pd;
   *ret = sd->horizontal;
}

EAPI void
elm_index_delay_change_time_set(Evas_Object *obj, double delay_change_time)
{
   ELM_INDEX_CHECK(obj);
   eo_do(obj, elm_obj_index_delay_change_time_set(delay_change_time));
}

static void
_delay_change_time_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double dtime = va_arg(*list, double);
   Elm_Index_Smart_Data *sd = _pd;
   sd->delay_change_time = dtime;
}

EAPI double
elm_index_delay_change_time_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *)obj, elm_obj_index_delay_change_time_get(&ret));
   return ret;
}

static void
_delay_change_time_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Index_Smart_Data *sd = _pd;
   *ret = sd->delay_change_time;
}

EAPI void
elm_index_omit_enabled_set(Evas_Object *obj,
                           Eina_Bool enabled)
{
   ELM_INDEX_CHECK(obj);
   eo_do(obj, elm_obj_index_omit_enabled_set(enabled));
}

static void
_omit_enabled_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool enabled = va_arg(*list, int);
   Elm_Index_Smart_Data *sd = _pd;

   if (sd->horizontal) return;

   enabled = !!enabled;
   if (sd->omit_enabled == enabled) return;
   sd->omit_enabled = enabled;

   _index_box_clear(obj, 0);
   _index_box_auto_fill(obj, 0);
   if (sd->level == 1)
     {
        _index_box_clear(obj, 1);
        _index_box_auto_fill(obj, 1);
     }
}

EAPI Eina_Bool
elm_index_omit_enabled_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_index_omit_enabled_get(&ret));
   return ret;
}

static void
_omit_enabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Index_Smart_Data *sd = _pd;
   *ret = sd->omit_enabled;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_index_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_index_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_index_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_index_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_index_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACCESS), _elm_index_smart_access),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_index_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_index_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_SET), _autohide_disabled_set),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_GET), _autohide_disabled_get),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_SET), _item_level_set),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_GET), _item_level_get),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_SELECTED_ITEM_GET), _selected_item_get),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_APPEND), _item_append),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_PREPEND), _item_prepend),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_AFTER), _item_insert_after),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_BEFORE), _item_insert_before),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_SORTED_INSERT), _item_sorted_insert),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_FIND), _elm_index_item_find),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_ITEM_CLEAR), _item_clear),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_LEVEL_GO), _level_go),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_SET), _indicator_disabled_set),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_GET), _indicator_disabled_get),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_SET), _horizontal_set),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_GET), _horizontal_get),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_SET), _delay_change_time_set),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_GET), _delay_change_time_get),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_OMIT_ENABLED_SET), _omit_enabled_set),
        EO_OP_FUNC(ELM_OBJ_INDEX_ID(ELM_OBJ_INDEX_SUB_ID_OMIT_ENABLED_GET), _omit_enabled_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_SET, "Enable or disable auto hiding feature for a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_AUTOHIDE_DISABLED_GET, "Get whether auto hiding feature is enabled or not for a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_SET, "Set the items level for a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_LEVEL_GET, "Get the items level set for a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_SELECTED_ITEM_GET, "Returns the last selected item, for a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_APPEND, "Append a new item on a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_PREPEND, "Prepend a new item on a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_AFTER, "Insert a new item into the index object after item after."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_INSERT_BEFORE, "Insert a new item into the index object before item before."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_SORTED_INSERT, "Insert a new item into the given index widget, using cmp_func."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_FIND, "Find a given index widget's item, <b>using item data</b>."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_ITEM_CLEAR, "Removes all items from a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_LEVEL_GO, "Go to a given items level on a index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_SET, "Set the indicator as to be disabled."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_INDICATOR_DISABLED_GET, "Get the value of indicator's disabled status."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_SET, "Enable or disable horizontal mode on the index object."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_HORIZONTAL_GET, "Get a value whether horizontal mode is enabled or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_SET, "Set a delay change time value for index object."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_DELAY_CHANGE_TIME_GET, "Get a delay change time value for index object."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_OMIT_ENABLED_SET, "Enable or disable omit feature for a given index widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_INDEX_SUB_ID_OMIT_ENABLED_GET, "Get whether omit feature is enabled or not for a given index widget."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_INDEX_BASE_ID, op_desc, ELM_OBJ_INDEX_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Index_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_index_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
