#include <Elementary.h>
#include "elm_priv.h"
#include "els_box.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Index_Item Elm_Index_Item;

struct _Widget_Data
{
   Evas_Object *base;
   Evas_Object *event[2];
   Evas_Object *bx[2]; // 2 - for now all that's supported
   Eina_List *items; // 1 list. yes N levels, but only 2 for now and # of items will be small
   int level;
   Evas_Coord dx, dy;
   Ecore_Timer *delay;
   Eina_Bool level_active[2];
   Eina_Bool horizontal : 1;
   Eina_Bool autohide_disabled : 1;
   Eina_Bool down : 1;
   Eina_Bool indicator_disabled : 1;
};

struct _Elm_Index_Item
{
   ELM_WIDGET_ITEM;
   const char *letter;
   int level;
   Evas_Smart_Cb func;
   Eina_Bool selected : 1;
};

static const char *widtype = NULL;

static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _index_box_auto_fill(Evas_Object *obj, Evas_Object *box, int level);
static void _index_box_clear(Evas_Object *obj, Evas_Object *box, int level);
static void _item_free(Elm_Index_Item *it);

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const char SIG_SELECTED[] = "selected";
static const char SIG_LEVEL_UP[] = "level,up";
static const char SIG_LEVEL_DOWN[] = "level,down";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CHANGED, ""},
   {SIG_DELAY_CHANGED, ""},
   {SIG_SELECTED, ""},
   {SIG_LEVEL_UP, ""},
   {SIG_LEVEL_DOWN, ""},
   {NULL, NULL}
};

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Index_Item *it;
   if (!wd) return;
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   _index_box_clear(obj, wd->bx[0], 0);
   while (wd->items)
     {
        it = wd->items->data;
        _item_free(it);
        elm_widget_item_free(it);
     }
   if (wd->delay) ecore_timer_del(wd->delay);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_layout(Evas_Object *o, Evas_Object_Box_Data *priv, void *data)
{
   Widget_Data *wd = data;
   if (!wd) return;
   _els_box_layout(o, priv, wd->horizontal, 1, 0);
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(wd->base, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_add(wd->base, emission, source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_signal_callback_del_full(wd->base, emission, source, func_cb,
                                        data);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!wd->horizontal)
     edje_object_mirrored_set(wd->base, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Evas_Coord minw = 0, minh = 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);

   _index_box_clear(obj, wd->bx[0], 0);
   _index_box_clear(obj, wd->bx[1], 1);
   if (wd->horizontal)
     _elm_theme_object_set(obj, wd->base, "index", "base/horizontal", elm_widget_style_get(obj));
   else
     {
        _elm_theme_object_set(obj, wd->base, "index", "base/vertical", elm_widget_style_get(obj));
        _mirrored_set(obj, elm_widget_mirrored_get(obj));
     }
   edje_object_part_swallow(wd->base, "elm.swallow.event.0", wd->event[0]);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(wd->event[0], minw, minh);
   edje_object_part_swallow(wd->base, "elm.swallow.index.0", wd->bx[0]);
   if (edje_object_part_exists(wd->base, "elm.swallow.index.1"))
     {
        if (!wd->bx[1])
          {
             wd->bx[1] = evas_object_box_add(evas_object_evas_get(wd->base));
             evas_object_box_layout_set(wd->bx[1], _layout, wd, NULL);
             elm_widget_sub_object_add(obj, wd->bx[1]);
          }
        edje_object_part_swallow(wd->base, "elm.swallow.index.1", wd->bx[1]);
        evas_object_show(wd->bx[1]);
     }
   else if (wd->bx[1])
     {
        evas_object_del(wd->bx[1]);
        wd->bx[1] = NULL;
     }
   if (edje_object_part_exists(wd->base, "elm.swallow.event.1"))
     {
        if (!wd->event[1])
          {
             wd->event[1] = evas_object_rectangle_add(evas_object_evas_get(wd->base));
             evas_object_color_set(wd->event[1], 0, 0, 0, 0);
             elm_widget_sub_object_add(obj, wd->event[1]);
          }
        edje_object_part_swallow(wd->base, "elm.swallow.event.1", wd->event[1]);
        evas_object_size_hint_min_set(wd->event[1], minw, minh);
     }
   else if (wd->event[1])
     {
        evas_object_del(wd->event[1]);
        wd->event[1] = NULL;
     }
   edje_object_message_signal_process(wd->base);
   edje_object_scale_set(wd->base, elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
   _index_box_auto_fill(obj, wd->bx[0], 0);

   if (wd->autohide_disabled)
     {
        if (wd->level == 1) _index_box_auto_fill(obj, wd->bx[1], 1);
        edje_object_signal_emit(wd->base, "elm,state,active", "elm");
     }
   else
     edje_object_signal_emit(wd->base, "elm,state,inactive", "elm");
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   edje_object_size_min_calc(wd->base, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return EINA_FALSE;
   _item_free((Elm_Index_Item *)it);
   _index_box_clear(WIDGET(it), wd->bx[wd->level], wd->level);
   return EINA_TRUE;
}

static Elm_Index_Item *
_item_new(Evas_Object *obj, const char *letter, Evas_Smart_Cb func, const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Index_Item *it;
   if (!wd) return NULL;
   it = elm_widget_item_new(obj, Elm_Index_Item);
   if (!it) return NULL;
   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   if (letter) it->letter = eina_stringshare_add(letter);
   it->func = func;
   it->base.data = data;
   it->level = wd->level;
   return it;
}

static Elm_Index_Item *
_item_find(Evas_Object *obj, const void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Index_Item *it;
   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->items, l, it)
     if (it->base.data == data) return it;
   return NULL;
}

static void
_item_free(Elm_Index_Item *it)
{
   Widget_Data *wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return;
   wd->items = eina_list_remove(wd->items, it);
   if (it->letter) eina_stringshare_del(it->letter);
}

// FIXME: always have index filled
static void
_index_box_auto_fill(Evas_Object *obj, Evas_Object *box, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_Bool rtl;
   Eina_List *l;
   Elm_Index_Item *it;
   Evas_Coord mw, mh, w, h;
   int i = 0;
   if (!wd) return;
   if (wd->level_active[level]) return;
   rtl = elm_widget_mirrored_get(obj);
   evas_object_geometry_get(box, NULL, NULL, &w, &h);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        Evas_Object *o;
        const char *stacking;

        if (it->level != level) continue;
        o = edje_object_add(evas_object_evas_get(obj));
        VIEW(it) = o;
        edje_object_mirrored_set(VIEW(it), rtl);

        if (wd->horizontal)
          {
             if (i & 0x1)
               _elm_theme_object_set(obj, o, "index", "item_odd/horizontal",
                                     elm_widget_style_get(obj));
             else
               _elm_theme_object_set(obj, o, "index", "item/horizontal",
                                     elm_widget_style_get(obj));
          }
        else
          {
             if (i & 0x1)
               _elm_theme_object_set(obj, o, "index", "item_odd/vertical",
                                     elm_widget_style_get(obj));
             else
               _elm_theme_object_set(obj, o, "index", "item/vertical",
                                     elm_widget_style_get(obj));
          }

        edje_object_part_text_set(o, "elm.text", it->letter);
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
             else if (!strcmp(stacking, "above")) evas_object_raise(o);
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
     }
   evas_object_smart_calculate(box);
   wd->level_active[level] = 1;
}

static void
_index_box_clear(Evas_Object *obj, Evas_Object *box __UNUSED__, int level)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Index_Item *it;
   if (!wd) return;
   if (!wd->level_active[level]) return;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (!VIEW(it)) continue;
        if (it->level != level) continue;
        evas_object_del(VIEW(it));
        VIEW(it) = NULL;
     }
   wd->level_active[level] = 0;
}

static Eina_Bool
_delay_change(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Object_Item *item;
   if (!wd) return ECORE_CALLBACK_CANCEL;
   wd->delay = NULL;
   item = elm_index_selected_item_get(data, wd->level);
   if (item) evas_object_smart_callback_call(data, SIG_DELAY_CHANGED, item);
   return ECORE_CALLBACK_CANCEL;
}

static void
_sel_eval(Evas_Object *obj, Evas_Coord evx, Evas_Coord evy)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Index_Item *it, *it_closest, *it_last;
   Eina_List *l;
   Evas_Coord x, y, w, h, bx, by, bw, bh, xx, yy;
   double cdv = 0.5;
   Evas_Coord dist;
   char *label = NULL, *last = NULL;
   int i;
   if (!wd) return;
   for (i = 0; i <= wd->level; i++)
     {
        it_last = NULL;
        it_closest  = NULL;
        dist = 0x7fffffff;
        evas_object_geometry_get(wd->bx[i], &bx, &by, &bw, &bh);
        EINA_LIST_FOREACH(wd->items, l, it)
          {
             if (it->level != i) continue;
             if (it->level != wd->level)
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
                  if (wd->horizontal)
                    cdv = (double)(xx - bx) / (double)bw;
                  else
                    cdv = (double)(yy - by) / (double)bh;
                  it_closest = it;
                  dist = x;
               }
          }
        if ((i == 0) && (wd->level == 0))
          edje_object_part_drag_value_set(wd->base, "elm.dragable.index.1",
                                          cdv, cdv);
        if (it_closest) it_closest->selected = 1;
        if (it_closest != it_last)
          {
             if (it_last)
               {
                  const char *stacking, *selectraise;

                  it = it_last;
                  edje_object_signal_emit(VIEW(it), "elm,state,inactive", "elm");
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
                  selectraise = edje_object_data_get(VIEW(it), "selectraise");
                  if ((selectraise) && (!strcmp(selectraise, "on")))
                    evas_object_raise(VIEW(it));
                  evas_object_smart_callback_call((void *)obj, SIG_CHANGED, (void *)it);
                  if (wd->delay) ecore_timer_del(wd->delay);
                  wd->delay = ecore_timer_add(0.2, _delay_change, obj);
               }
          }
        if (it_closest)
          {
             it = it_closest;
             if (!last && it->letter)
               last = strdup(it->letter);
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
   edje_object_part_text_set(wd->base, "elm.text.body", label);
   edje_object_part_text_set(wd->base, "elm.text", last);
   free(label);
   free(last);
}

static void
_wheel(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   //   Evas_Event_Mouse_Wheel *ev = event_info;
   //   Evas_Object *obj = o;
   if (!wd) return;
}

static void
_mouse_down(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w;
   if (!wd) return;
   if (ev->button != 1) return;
   wd->down = 1;
   evas_object_geometry_get(wd->base, &x, &y, &w, NULL);
   wd->dx = ev->canvas.x - x;
   wd->dy = ev->canvas.y - y;
   if (!wd->autohide_disabled)
     {
        _index_box_clear(data, wd->bx[1], 1);
        _index_box_auto_fill(data, wd->bx[0], 0);
        edje_object_signal_emit(wd->base, "elm,state,active", "elm");
     }
   _sel_eval(data, ev->canvas.x, ev->canvas.y);
   edje_object_part_drag_value_set(wd->base, "elm.dragable.pointer",
                                   (!edje_object_mirrored_get(wd->base)) ? wd->dx : (wd->dx - w), wd->dy);
   if (wd->items && !wd->indicator_disabled)
     edje_object_signal_emit(wd->base, "elm,indicator,state,active", "elm");
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Up *ev = event_info;
   Elm_Object_Item *item;
   Elm_Index_Item *id_item;
   if (!wd) return;
   if (ev->button != 1) return;
   wd->down = 0;
   item = elm_index_selected_item_get(data, wd->level);
   if (item)
     {
        evas_object_smart_callback_call(data, SIG_SELECTED, item);
        id_item = (Elm_Index_Item *)item;
        if (id_item->func)
          id_item->func((void *)id_item->base.data, WIDGET(id_item), id_item);
     }
   if (!wd->autohide_disabled)
     edje_object_signal_emit(wd->base, "elm,state,inactive", "elm");
   edje_object_signal_emit(wd->base, "elm,state,level,0", "elm");
   if (wd->items && !wd->indicator_disabled)
     edje_object_signal_emit(wd->base, "elm,indicator,state,inactive", "elm");
}

static void
_mouse_move(void *data, Evas *e __UNUSED__, Evas_Object *o __UNUSED__, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, adx, w;
   char buf[1024];
   if (!wd) return;
   if (!wd->down) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(wd->base, &x, &y, &w, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - wd->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   edje_object_part_drag_value_set(wd->base, "elm.dragable.pointer"
                                   , (!edje_object_mirrored_get(wd->base)) ? x : (x - w), y);
   if (!wd->horizontal)
     {
        if (adx > minw)
          {
             if (!wd->level)
               {
                  wd->level = 1;
                  snprintf(buf, sizeof(buf), "elm,state,level,%i", wd->level);
                  edje_object_signal_emit(wd->base, buf, "elm");
                  evas_object_smart_callback_call(data, SIG_LEVEL_UP, NULL);
               }
          }
        else
          {
             if (wd->level == 1)
               {
                  wd->level = 0;
                  snprintf(buf, sizeof(buf), "elm,state,level,%i", wd->level);
                  edje_object_signal_emit(wd->base, buf, "elm");
                  evas_object_smart_callback_call(data, SIG_LEVEL_DOWN, NULL);
               }
          }
     }
   _sel_eval(data, ev->cur.canvas.x, ev->cur.canvas.y);
}

EAPI Evas_Object *
elm_index_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas_Object *o;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord minw, minh;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "index");
   elm_widget_type_set(obj, "index");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->indicator_disabled = EINA_FALSE;
   wd->horizontal = EINA_FALSE;
   wd->autohide_disabled = EINA_FALSE;

   wd->base = edje_object_add(e);
   _elm_theme_object_set(obj, wd->base, "index", "base/vertical", "default");
   elm_widget_resize_object_set(obj, wd->base);

   o = evas_object_rectangle_add(e);
   wd->event[0] = o;
   evas_object_color_set(o, 0, 0, 0, 0);
   minw = minh = 0;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(o, minw, minh);
   edje_object_part_swallow(wd->base, "elm.swallow.event.0", o);
   elm_widget_sub_object_add(obj, o);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_WHEEL, _wheel, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _mouse_down, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_UP, _mouse_up, obj);
   evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_MOVE, _mouse_move, obj);
   evas_object_show(o);
   if (edje_object_part_exists(wd->base, "elm.swallow.event.1"))
     {
        o = evas_object_rectangle_add(e);
        wd->event[1] = o;
        evas_object_color_set(o, 0, 0, 0, 0);
        evas_object_size_hint_min_set(o, minw, minh);
        edje_object_part_swallow(wd->base, "elm.swallow.event.1", o);
        elm_widget_sub_object_add(obj, o);
     }

   wd->bx[0] = evas_object_box_add(e);
   evas_object_box_layout_set(wd->bx[0], _layout, wd, NULL);
   elm_widget_sub_object_add(obj, wd->bx[0]);
   edje_object_part_swallow(wd->base, "elm.swallow.index.0", wd->bx[0]);
   evas_object_show(wd->bx[0]);

   if (edje_object_part_exists(wd->base, "elm.swallow.index.1"))
     {
        wd->bx[1] = evas_object_box_add(e);
        evas_object_box_layout_set(wd->bx[1], _layout, wd, NULL);
        elm_widget_sub_object_add(obj, wd->bx[1]);
        edje_object_part_swallow(wd->base, "elm.swallow.index.1", wd->bx[1]);
        evas_object_show(wd->bx[1]);
     }

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_index_autohide_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   disabled = !!disabled;
   if (wd->autohide_disabled == disabled) return;
   wd->autohide_disabled = disabled;
   wd->level = 0;
   if (wd->autohide_disabled)
     {
        _index_box_clear(obj, wd->bx[1], 1);
        _index_box_auto_fill(obj, wd->bx[0], 0);
        edje_object_signal_emit(wd->base, "elm,state,active", "elm");
     }
   else
     edje_object_signal_emit(wd->base, "elm,state,inactive", "elm");

   //FIXME: Should be update indicator based on the indicator visiblility 
}

EAPI Eina_Bool
elm_index_autohide_disabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->autohide_disabled;
}

EAPI void
elm_index_item_level_set(Evas_Object *obj, int level)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->level == level) return;
   wd->level = level;
}

EAPI int
elm_index_item_level_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;
   return wd->level;
}

EAPI void
elm_index_item_selected_set(Elm_Object_Item *it, Eina_Bool selected)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Evas_Coord x, y, w, h;
   Widget_Data *wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return;

   //FIXME: Should be update indicator based on the autohidden status & indicator visiblility 

   if (selected)
     {
        evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
        _sel_eval(WIDGET(it), x + (w/2), y + (h/2));
     }
   else
     _sel_eval(WIDGET(it), -99999, -9999);

}

EAPI Elm_Object_Item *
elm_index_selected_item_get(const Evas_Object *obj, int level)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *l;
   Elm_Index_Item *it;
   if (!wd) return NULL;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if ((it->selected) && (it->level == level))
          return (Elm_Object_Item *)it;
     }
   return NULL;
}

EAPI Elm_Object_Item *
elm_index_item_append(Evas_Object *obj, const char *letter, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Index_Item *it;
   if (!wd) return NULL;
   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;
   wd->items = eina_list_append(wd->items, it);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_prepend(Evas_Object *obj, const char *letter, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Index_Item *it;

   if (!wd) return NULL;
   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;
   wd->items = eina_list_prepend(wd->items, it);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   return (Elm_Object_Item *)it;
}

EINA_DEPRECATED EAPI Elm_Object_Item *
elm_index_item_prepend_relative(Evas_Object *obj, const char *letter, const void *item, const Elm_Object_Item *relative)
{
   return elm_index_item_insert_before(obj, (Elm_Object_Item *) relative, letter, NULL, item);
}

EAPI Elm_Object_Item *
elm_index_item_insert_after(Evas_Object *obj, Elm_Object_Item *after, const char *letter, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Index_Item *it;
   if (!wd) return NULL;
   if (!after) return elm_index_item_append(obj, letter, func, data);
   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;
   wd->items = eina_list_append_relative(wd->items, it, after);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_insert_before(Evas_Object *obj, Elm_Object_Item *before, const char *letter, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Index_Item *it;
   if (!wd) return NULL;
   if (!before) return elm_index_item_prepend(obj, letter, func, data);
   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;
   wd->items = eina_list_prepend_relative(wd->items, it, before);
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_sorted_insert(Evas_Object *obj, const char *letter, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func, Eina_Compare_Cb cmp_data_func)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Eina_List *lnear;
   Elm_Index_Item *it;
   int cmp;

   if (!wd) return NULL;
   if (!(wd->items))
     return elm_index_item_append(obj, letter, func, data);

   it = _item_new(obj, letter, func, data);
   if (!it) return NULL;

   lnear = eina_list_search_sorted_near_list(wd->items, cmp_func, it, &cmp);
   if (cmp < 0)
     wd->items =  eina_list_append_relative_list(wd->items, it, lnear);
   else if (cmp > 0)
     wd->items = eina_list_prepend_relative_list(wd->items, it, lnear);
   else
     {
        /* If cmp_data_func is not provided, append a duplicated item */
        if (!cmp_data_func)
          wd->items =  eina_list_append_relative_list(wd->items, it, lnear);
        else
          {
             Elm_Index_Item *p_it = eina_list_data_get(lnear);
             if (cmp_data_func(p_it->base.data, it->base.data) >= 0)
               p_it->base.data = it->base.data;
             _item_free(it);
             elm_widget_item_free(it);
          }
     }
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_index_item_find(Evas_Object *obj, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return (Elm_Object_Item *) _item_find(obj, data);
}

EAPI void
elm_index_item_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Index_Item *it;
   Eina_List *l, *clear = NULL;
   if (!wd) return;
   _index_box_clear(obj, wd->bx[wd->level], wd->level);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->level != wd->level) continue;
        clear = eina_list_append(clear, it);
     }
   EINA_LIST_FREE(clear, it)
     {
        _item_free(it);
        elm_widget_item_free(it);
     }
}

EAPI void
elm_index_level_go(Evas_Object *obj, int level __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _index_box_auto_fill(obj, wd->bx[0], 0);
   if (wd->level == 1) _index_box_auto_fill(obj, wd->bx[1], 1);
}

EAPI void
elm_index_indicator_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   disabled = !!disabled;
   if (wd->indicator_disabled == disabled) return;
   wd->indicator_disabled = disabled;
   if (!wd->items) return;
   if (disabled)
     edje_object_signal_emit(wd->base, "elm,indicator,state,inactive", "elm");
   else
     edje_object_signal_emit(wd->base, "elm,indicator,state,active", "elm");
}

EAPI Eina_Bool
elm_index_indicator_disabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   return wd->indicator_disabled;
}

EAPI const char *
elm_index_item_letter_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   return ((Elm_Index_Item *)it)->letter;
}

EAPI void
elm_index_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   horizontal = !!horizontal;
   if (horizontal == wd->horizontal) return;
   wd->horizontal = horizontal;
   _theme_hook(obj);
}

EAPI Eina_Bool
elm_index_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->horizontal;
}


