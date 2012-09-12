#include <Elementary.h>
#include "elm_priv.h"
#include "els_box.h"
#include "elm_widget_index.h"

EAPI const char ELM_INDEX_SMART_NAME[] = "elm_index";

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
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_INDEX_SMART_NAME, _elm_index, Elm_Index_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_item_free(Elm_Index_Item *it)
{
   ELM_INDEX_DATA_GET(WIDGET(it), sd);

   sd->items = eina_list_remove(sd->items, it);
   if (it->letter) 
     {
        eina_stringshare_del(it->letter);
        it->letter = NULL;
     }
}

static void
_box_custom_layout(Evas_Object *o,
                   Evas_Object_Box_Data *priv,
                   void *data)
{
   Elm_Index_Smart_Data *sd = data;

   _els_box_layout(o, priv, sd->horizontal, 1, 0);
}

static void
_index_box_clear(Evas_Object *obj,
                 Evas_Object *box __UNUSED__,
                 int level)
{
   Eina_List *l;
   Elm_Index_Item *it;

   ELM_INDEX_DATA_GET(obj, sd);

   if (!sd->level_active[level]) return;

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if (!VIEW(it)) continue;
        if (it->level != level) continue;

        evas_object_del(VIEW(it));
        VIEW(it) = NULL;
     }

   sd->level_active[level] = 0;
}

static char *
_access_info_cb(void *data,
                Evas_Object *obj __UNUSED__,
                Elm_Widget_Item *item __UNUSED__)
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

   ai = _elm_access_object_get(it->base.access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Index Item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
}

// FIXME: always have index filled
static void
_index_box_auto_fill(Evas_Object *obj,
                     Evas_Object *box,
                     int level)
{
   int i = 0;
   Eina_List *l;
   Eina_Bool rtl;
   Elm_Index_Item *it;
   Evas_Coord mw, mh, w, h;

   ELM_INDEX_DATA_GET(obj, sd);

   if (sd->level_active[level]) return;

   rtl = elm_widget_mirrored_get(obj);
   evas_object_geometry_get(box, NULL, NULL, &w, &h);

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        Evas_Object *o;
        const char *stacking;

        if (it->level != level) continue;

        o = edje_object_add(evas_object_evas_get(obj));
        VIEW(it) = o;
        edje_object_mirrored_set(VIEW(it), rtl);

        if (sd->horizontal)
          {
             if (i & 0x1)
               elm_widget_theme_object_set
                 (obj, o, "index", "item_odd/horizontal",
                 elm_widget_style_get(obj));
             else
               elm_widget_theme_object_set
                 (obj, o, "index", "item/horizontal",
                 elm_widget_style_get(obj));
          }
        else
          {
             if (i & 0x1)
               elm_widget_theme_object_set
                 (obj, o, "index", "item_odd/vertical",
                 elm_widget_style_get(obj));
             else
               elm_widget_theme_object_set
                 (obj, o, "index", "item/vertical",
                 elm_widget_style_get(obj));
          }

        edje_object_part_text_escaped_set(o, "elm.text", it->letter);
        edje_object_size_min_restricted_calc(o, &mw, &mh, 0, 0);
        evas_object_size_hint_min_set(o, mw, mh);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_widget_sub_object_add(obj, o);
        evas_object_box_append(box, o);
        stacking = edje_object_data_get(o, "stacking");

        if (stacking)
          {
             if (!strcmp(stacking, "below")) evas_object_lower(o);
             else if (!strcmp(stacking, "above"))
               evas_object_raise(o);
          }

        evas_object_show(o);

        i++;
        evas_object_smart_calculate(box); // force a calc so we know the size
        evas_object_size_hint_min_get(box, &mw, &mh);
        if (mh > h)
          {
             _index_box_clear(obj, box, level);
             if (i > 0)
               {
                  // FIXME: only i objects fit! try again. overflows right now
               }
          }

        // ACCESS
        if ((it->level == 0) &&
            (_elm_config->access_mode == ELM_ACCESS_MODE_ON))
          _access_widget_item_register(it);
     }

   evas_object_smart_calculate(box);
   sd->level_active[level] = 1;
}

static Eina_Bool
_elm_index_smart_theme(Evas_Object *obj)
{
   Evas_Coord minw = 0, minh = 0;

   ELM_INDEX_DATA_GET(obj, sd);

   _index_box_clear(obj, sd->bx[0], 0);
   _index_box_clear(obj, sd->bx[1], 1);

   if (sd->horizontal)
     eina_stringshare_replace(&ELM_LAYOUT_DATA(sd)->group, "base/horizontal");
   else
     eina_stringshare_replace(&ELM_LAYOUT_DATA(sd)->group, "base/vertical");

   if (!ELM_WIDGET_CLASS(_elm_index_parent_sc)->theme(obj)) return EINA_FALSE;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->event[0], minw, minh);

   if (edje_object_part_exists
         (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.index.1"))
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
   else if (sd->bx[1])
     {
        evas_object_del(sd->bx[1]);
        sd->bx[1] = NULL;
     }
   if (edje_object_part_exists
         (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.event.1"))
     {
        if (!sd->event[1])
          {
             sd->event[1] =
               evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_color_set(sd->event[1], 0, 0, 0, 0);
             elm_widget_sub_object_add(obj, sd->event[1]);
          }
        elm_layout_content_set(obj, "elm.swallow.event.1", sd->event[1]);
        evas_object_size_hint_min_set(sd->event[1], minw, minh);
     }
   else if (sd->event[1])
     {
        evas_object_del(sd->event[1]);
        sd->event[1] = NULL;
     }
   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);

   elm_layout_sizing_eval(obj);
   _index_box_auto_fill(obj, sd->bx[0], 0);

   if (sd->autohide_disabled)
     {
        if (sd->level == 1) _index_box_auto_fill(obj, sd->bx[1], 1);
        elm_layout_signal_emit(obj, "elm,state,active", "elm");
     }
   else elm_layout_signal_emit(obj, "elm,state,inactive", "elm");

   return EINA_TRUE;
}

static void
_elm_index_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   ELM_INDEX_DATA_GET(obj, sd);

   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   ELM_INDEX_DATA_GET(WIDGET(it), sd);

   _item_free((Elm_Index_Item *)it);
   _index_box_clear(WIDGET(it), sd->bx[sd->level], sd->level);

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

   EINA_LIST_FOREACH (sd->items, l, it)
     if (it->base.data == data) return it;

   return NULL;
}

static Eina_Bool
_delay_change(void *data)
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
   Elm_Index_Item *it, *it_closest, *it_last;
   char *label = NULL, *last = NULL;
   double cdv = 0.5;
   Evas_Coord dist;
   Eina_List *l;
   int i;

   ELM_INDEX_DATA_GET(obj, sd);

   for (i = 0; i <= sd->level; i++)
     {
        it_last = NULL;
        it_closest = NULL;
        dist = 0x7fffffff;
        evas_object_geometry_get(sd->bx[i], &bx, &by, &bw, &bh);

        EINA_LIST_FOREACH (sd->items, l, it)
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
                  it->selected = 0;
               }
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
        if ((i == 0) && (sd->level == 0))
          edje_object_part_drag_value_set
            (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.index.1", cdv, cdv);
        if (it_closest) it_closest->selected = 1;
        if (it_closest != it_last)
          {
             if (it_last)
               {
                  const char *stacking, *selectraise;

                  it = it_last;
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
             if (it_closest)
               {
                  const char *selectraise;

                  it = it_closest;
                  edje_object_signal_emit(VIEW(it), "elm,state,active", "elm");

                  // ACCESS
                  if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
                    {
                       char *ret;
                       Eina_Strbuf *buf;
                       buf = eina_strbuf_new();

                       eina_strbuf_append_printf(buf, "index item %s clicked", it->letter);
                       ret = eina_strbuf_string_steal(buf);
                       eina_strbuf_free(buf);

                       _elm_access_highlight_set(it->base.access_obj);
                       _elm_access_say(ret);
                    }

                  selectraise = edje_object_data_get(VIEW(it), "selectraise");
                  if ((selectraise) && (!strcmp(selectraise, "on")))
                    evas_object_raise(VIEW(it));
                  evas_object_smart_callback_call
                    (obj, SIG_CHANGED, (void *)it);
                  if (sd->delay) ecore_timer_del(sd->delay);
                  sd->delay = ecore_timer_add(0.2, _delay_change, obj);
               }
          }
        if (it_closest)
          {
             it = it_closest;
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
_on_mouse_wheel(void *data __UNUSED__,
                Evas *e __UNUSED__,
                Evas_Object *o __UNUSED__,
                void *event_info __UNUSED__)
{
}

static void
_on_mouse_down(void *data,
               Evas *e __UNUSED__,
               Evas_Object *o __UNUSED__,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w;

   ELM_INDEX_DATA_GET(data, sd);

   if (ev->button != 1) return;
   sd->down = 1;
   evas_object_geometry_get(ELM_WIDGET_DATA(sd)->resize_obj, &x, &y, &w, NULL);
   sd->dx = ev->canvas.x - x;
   sd->dy = ev->canvas.y - y;
   if (!sd->autohide_disabled)
     {
        _index_box_clear(data, sd->bx[1], 1);
        _index_box_auto_fill(data, sd->bx[0], 0);
        elm_layout_signal_emit(data, "elm,state,active", "elm");
     }
   _sel_eval(data, ev->canvas.x, ev->canvas.y);
   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.pointer",
     (!elm_object_mirrored_get(data)) ? sd->dx : (sd->dx - w), sd->dy);
   if (sd->items && !sd->indicator_disabled)
     elm_layout_signal_emit(data, "elm,indicator,state,active", "elm");
}

static void
_on_mouse_up(void *data,
             Evas *e __UNUSED__,
             Evas_Object *o __UNUSED__,
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
               Evas *e __UNUSED__,
               Evas_Object *o __UNUSED__,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, adx, w;
   char buf[1024];

   ELM_INDEX_DATA_GET(data, sd);

   if (!sd->down) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(ELM_WIDGET_DATA(sd)->resize_obj, &x, &y, &w, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - sd->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.dragable.pointer",
     (!edje_object_mirrored_get(ELM_WIDGET_DATA(sd)->resize_obj)) ?
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
                    Evas *e __UNUSED__,
                    Evas_Object *o __UNUSED__,
                    void *event_info)
{
   ELM_INDEX_DATA_GET(data, sd);

   if (sd->down) return;

   if (!sd->autohide_disabled)
     {
        _index_box_clear(data, sd->bx[1], 1);
        _index_box_auto_fill(data, sd->bx[0], 0);
        elm_layout_signal_emit(data, "elm,state,active", "elm");
     }
}

static void
_on_mouse_move_access(void *data,
                      Evas *e __UNUSED__,
                      Evas_Object *o __UNUSED__,
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

   EINA_LIST_FOREACH (sd->items, l, it)
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
                      Evas *e __UNUSED__,
                      Evas_Object *o __UNUSED__,
                      void *event_info __UNUSED__)
{
   ELM_INDEX_DATA_GET(data, sd);

   if (!sd->autohide_disabled)
     elm_layout_signal_emit(data, "elm,state,inactive", "elm");
}

static void
_access_index_register(Evas_Object *obj)
{
   Evas_Object *ao;
   elm_widget_can_focus_set(obj, EINA_TRUE);
   ao = _elm_access_edje_object_part_object_register
              (obj, elm_layout_edje_get(obj), "access");
   _elm_access_text_set
     (_elm_access_object_get(ao), ELM_ACCESS_TYPE, E_("Index"));
}

static void
_elm_index_smart_add(Evas_Object *obj)
{
   Evas_Object *o;
   Evas_Coord minw, minh;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Index_Smart_Data);

   ELM_WIDGET_CLASS(_elm_index_parent_sc)->base.add(obj);

   priv->indicator_disabled = EINA_FALSE;
   priv->horizontal = EINA_FALSE;
   priv->autohide_disabled = EINA_FALSE;

   elm_layout_theme_set
     (obj, "index", "base/vertical", elm_widget_style_get(obj));

   o = evas_object_rectangle_add(evas_object_evas_get(obj));
   priv->event[0] = o;
   evas_object_color_set(o, 0, 0, 0, 0);
   minw = minh = 0;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(o, minw, minh);
   elm_layout_content_set(obj, "elm.swallow.event.0", o);
   elm_widget_sub_object_add(obj, o);

   evas_object_event_callback_add
     (o, EVAS_CALLBACK_MOUSE_WHEEL, _on_mouse_wheel, obj);
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
         (ELM_WIDGET_DATA(priv)->resize_obj, "elm.swallow.event.1"))
     {
        o = evas_object_rectangle_add(evas_object_evas_get(obj));
        priv->event[1] = o;
        evas_object_color_set(o, 0, 0, 0, 0);
        evas_object_size_hint_min_set(o, minw, minh);
        elm_layout_content_set(obj, "elm.swallow.event.1", o);
        elm_widget_sub_object_add(obj, o);
     }

   priv->bx[0] = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_box_layout_set(priv->bx[0], _box_custom_layout, priv, NULL);
   elm_widget_sub_object_add(obj, priv->bx[0]);
   elm_layout_content_set(obj, "elm.swallow.index.0", priv->bx[0]);
   evas_object_show(priv->bx[0]);

   if (edje_object_part_exists
         (ELM_WIDGET_DATA(priv)->resize_obj, "elm.swallow.index.1"))
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
_elm_index_smart_del(Evas_Object *obj)
{
   Elm_Index_Item *it;

   ELM_INDEX_DATA_GET(obj, sd);

   while (sd->items)
     {
        it = sd->items->data;
        _item_free(it);
        elm_widget_item_del(it);
     }

   if (sd->delay) ecore_timer_del(sd->delay);

   ELM_WIDGET_CLASS(_elm_index_parent_sc)->base.del(obj);
}

static Eina_Bool
_elm_index_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   Eina_List *items = NULL;
   Eina_List *l = NULL;
   Elm_Index_Item *it;
   Evas_Object *ao;
   Evas_Object *po;

   ELM_INDEX_CHECK(obj) EINA_FALSE;
   ELM_INDEX_DATA_GET(obj, sd);

   if (!sd->autohide_disabled)
     elm_layout_signal_emit((Evas_Object *)obj, "elm,state,active", "elm");

   po = (Evas_Object *)edje_object_part_object_get
              (elm_layout_edje_get(obj), "access");
   ao = evas_object_data_get(po, "_part_access_obj");
   items = eina_list_append(items, ao);

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if (it->level != 0) continue;
        items = eina_list_append(items, it->base.access_obj);
     }

   Eina_Bool ret;
   ret = elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);

   // to hide index item, if there is nothing to focus on autohide disalbe mode
   if ((!sd->autohide_disabled) && (!ret))
     elm_layout_signal_emit((Evas_Object *)obj, "elm,state,inactive", "elm");

   return ret;
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Eina_List *l;
   Elm_Index_Item *it;

   ELM_INDEX_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if (it->level != 0) continue;
        if (is_access) _access_widget_item_register(it);
        else _elm_access_widget_item_unregister((Elm_Widget_Item *)it);
     }

   if (is_access)
     {
        _access_index_register(obj);

        evas_object_event_callback_add
          (sd->event[0], EVAS_CALLBACK_MOUSE_IN, _on_mouse_in_access, obj);
        evas_object_event_callback_add
          (sd->event[0], EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move_access, obj);
        evas_object_event_callback_add
          (sd->event[0], EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out_access, obj);
     }
   else
     {
        // opposition of  _access_index_register();
        elm_widget_can_focus_set(obj, EINA_FALSE);
        _elm_access_edje_object_part_object_unregister
             (obj, elm_layout_edje_get(obj), "access");

        evas_object_event_callback_del_full
          (sd->event[0], EVAS_CALLBACK_MOUSE_IN, _on_mouse_in_access, obj);
        evas_object_event_callback_del_full
          (sd->event[0], EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move_access, obj);
        evas_object_event_callback_del_full
          (sd->event[0], EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out_access, obj);
     }
}

static void
_access_hook(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_INDEX_CHECK(obj);
   ELM_INDEX_DATA_GET(obj, sd);

   if (is_access)
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next =
     _elm_index_smart_focus_next;
   else
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next = NULL;
   _access_obj_process(obj, is_access);
}

static void
_elm_index_smart_set_user(Elm_Index_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_index_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_index_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_index_smart_theme;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_index_smart_sizing_eval;

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     ELM_WIDGET_CLASS(sc)->focus_next = _elm_index_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _access_hook;
}

EAPI const Elm_Index_Smart_Class *
elm_index_smart_class_get(void)
{
   static Elm_Index_Smart_Class _sc =
     ELM_INDEX_SMART_CLASS_INIT_NAME_VERSION(ELM_INDEX_SMART_NAME);
   static const Elm_Index_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_index_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_index_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_index_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_index_autohide_disabled_set(Evas_Object *obj,
                                Eina_Bool disabled)
{
   ELM_INDEX_CHECK(obj);
   ELM_INDEX_DATA_GET(obj, sd);

   disabled = !!disabled;
   if (sd->autohide_disabled == disabled) return;
   sd->autohide_disabled = disabled;
   sd->level = 0;
   if (sd->autohide_disabled)
     {
        _index_box_clear(obj, sd->bx[1], 1);
        _index_box_auto_fill(obj, sd->bx[0], 0);
        elm_layout_signal_emit(obj, "elm,state,active", "elm");
     }
   else
     elm_layout_signal_emit(obj, "elm,state,inactive", "elm");

   //FIXME: Should be update indicator based on the indicator visiblility
}

EAPI Eina_Bool
elm_index_autohide_disabled_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) EINA_FALSE;
   ELM_INDEX_DATA_GET(obj, sd);

   return sd->autohide_disabled;
}

EAPI void
elm_index_item_level_set(Evas_Object *obj,
                         int level)
{
   ELM_INDEX_CHECK(obj);
   ELM_INDEX_DATA_GET(obj, sd);

   if (sd->level == level) return;
   sd->level = level;
}

EAPI int
elm_index_item_level_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) 0;
   ELM_INDEX_DATA_GET(obj, sd);

   return sd->level;
}

EAPI void
elm_index_item_selected_set(Elm_Object_Item *it,
                            Eina_Bool selected)
{
   Evas_Coord x, y, w, h;

   ELM_INDEX_ITEM_CHECK_OR_RETURN(it);

   //FIXME: Should be update indicator based on the autohidden status
   //& indicator visiblility

   if (selected)
     {
        evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
        _sel_eval(WIDGET(it), x + (w / 2), y + (h / 2));
     }
   else _sel_eval(WIDGET(it), -99999, -9999);
}

EAPI Elm_Object_Item *
elm_index_selected_item_get(const Evas_Object *obj,
                            int level)
{
   Eina_List *l;
   Elm_Index_Item *it;

   ELM_INDEX_CHECK(obj) NULL;
   ELM_INDEX_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if ((it->selected) && (it->level == level))
          return (Elm_Object_Item *)it;
     }

   return NULL;
}

EAPI Elm_Object_Item *
elm_index_item_append(Evas_Object *obj,
                      const char *letter,
                      Evas_Smart_Cb func,
                      const void *data)
{
   Elm_Index_Item *it;

   ELM_INDEX_CHECK(obj) NULL;
   ELM_INDEX_DATA_GET(obj, sd);

   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;

   sd->items = eina_list_append(sd->items, it);
   _index_box_clear(obj, sd->bx[sd->level], sd->level);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_prepend(Evas_Object *obj,
                       const char *letter,
                       Evas_Smart_Cb func,
                       const void *data)
{
   Elm_Index_Item *it;

   ELM_INDEX_CHECK(obj) NULL;
   ELM_INDEX_DATA_GET(obj, sd);

   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;

   sd->items = eina_list_prepend(sd->items, it);
   _index_box_clear(obj, sd->bx[sd->level], sd->level);

   return (Elm_Object_Item *)it;
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
   Elm_Index_Item *it;

   ELM_INDEX_CHECK(obj) NULL;
   ELM_INDEX_DATA_GET(obj, sd);

   if (!after) return elm_index_item_append(obj, letter, func, data);

   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;

   sd->items = eina_list_append_relative(sd->items, it, after);
   _index_box_clear(obj, sd->bx[sd->level], sd->level);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_insert_before(Evas_Object *obj,
                             Elm_Object_Item *before,
                             const char *letter,
                             Evas_Smart_Cb func,
                             const void *data)
{
   Elm_Index_Item *it;

   ELM_INDEX_CHECK(obj) NULL;
   ELM_INDEX_DATA_GET(obj, sd);

   if (!before) return elm_index_item_prepend(obj, letter, func, data);

   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;

   sd->items = eina_list_prepend_relative(sd->items, it, before);
   _index_box_clear(obj, sd->bx[sd->level], sd->level);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_sorted_insert(Evas_Object *obj,
                             const char *letter,
                             Evas_Smart_Cb func,
                             const void *data,
                             Eina_Compare_Cb cmp_func,
                             Eina_Compare_Cb cmp_data_func)
{
   Elm_Index_Item *it;
   Eina_List *lnear;
   int cmp;

   ELM_INDEX_CHECK(obj) NULL;
   ELM_INDEX_DATA_GET(obj, sd);

   if (!(sd->items)) return elm_index_item_append(obj, letter, func, data);

   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;

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
             _item_free(it);
             elm_widget_item_del(it);
          }
     }
   _index_box_clear(obj, sd->bx[sd->level], sd->level);

   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_find(Evas_Object *obj,
                    const void *data)
{
   ELM_INDEX_CHECK(obj) NULL;

   return (Elm_Object_Item *)_item_find(obj, data);
}

EAPI void
elm_index_item_clear(Evas_Object *obj)
{
   Elm_Index_Item *it;
   Eina_List *l, *clear = NULL;

   ELM_INDEX_CHECK(obj);
   ELM_INDEX_DATA_GET(obj, sd);

   _index_box_clear(obj, sd->bx[sd->level], sd->level);
   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if (it->level != sd->level) continue;
        clear = eina_list_append(clear, it);
     }
   EINA_LIST_FREE (clear, it)
     {
        _item_free(it);
        elm_widget_item_del(it);
     }
}

EAPI void
elm_index_level_go(Evas_Object *obj,
                   int level __UNUSED__)
{
   ELM_INDEX_CHECK(obj);
   ELM_INDEX_DATA_GET(obj, sd);

   _index_box_auto_fill(obj, sd->bx[0], 0);
   if (sd->level == 1) _index_box_auto_fill(obj, sd->bx[1], 1);
}

EAPI void
elm_index_indicator_disabled_set(Evas_Object *obj,
                                 Eina_Bool disabled)
{
   ELM_INDEX_CHECK(obj);
   ELM_INDEX_DATA_GET(obj, sd);

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
   ELM_INDEX_DATA_GET(obj, sd);

   return sd->indicator_disabled;
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
   ELM_INDEX_DATA_GET(obj, sd);

   horizontal = !!horizontal;
   if (horizontal == sd->horizontal) return;

   sd->horizontal = horizontal;
   _elm_index_smart_theme(obj);
}

EAPI Eina_Bool
elm_index_horizontal_get(const Evas_Object *obj)
{
   ELM_INDEX_CHECK(obj) EINA_FALSE;
   ELM_INDEX_DATA_GET(obj, sd);

   return sd->horizontal;
}
