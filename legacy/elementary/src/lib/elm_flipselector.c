#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_flipselector.h"

/* TODO: ideally, the default theme would use map{} blocks on the TEXT
   parts to implement their fading in/out propertly (as in the clock
   widget) */
/* TODO: if one ever wants to extend it to receiving generic widgets
   as items, be my guest. in this case, remember to implement the
   items tooltip infra. */
/* TODO: fix default theme image borders for looong strings as item
   labels. */
/* TODO: set text elipsis on labels if one enforces mininum size on
 * the overall widget less the required for displaying it. */
/* TODO: find a way to, in the default theme, to detect we are
 * bootstrapping (receiving the 1st message) and populate the downmost
 * TEXT parts with the same text as the upmost, where appropriate. */

EAPI const char ELM_FLIPSELECTOR_SMART_NAME[] = "elm_flipselector";

#define FLIP_FIRST_INTERVAL (0.85)
#define FLIP_MIN_INTERVAL   (0.1)
#define MSG_FLIP_DOWN       (1)
#define MSG_FLIP_UP         (2)
#define MAX_LEN_DEFAULT     (50)

#define DATA_GET            eina_list_data_get

static const char SIG_SELECTED[] = "selected";
static const char SIG_UNDERFLOWED[] = "underflowed";
static const char SIG_OVERFLOWED[] = "overflowed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_SELECTED, ""},
   {SIG_UNDERFLOWED, ""},
   {SIG_OVERFLOWED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_FLIPSELECTOR_SMART_NAME, _elm_flipselector,
  Elm_Flipselector_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, _smart_callbacks);

static void
_elm_flipselector_smart_sizing_eval(Evas_Object *obj)
{
   char *tmp = NULL;
   Evas_Coord minw = -1, minh = -1, w, h;

   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   if (sd->evaluating) return;

   elm_coords_finger_size_adjust(1, &minw, 2, &minh);

   sd->evaluating = EINA_TRUE;

   if (sd->sentinel)
     {
        const char *label = elm_object_item_text_get(DATA_GET(sd->sentinel));

        tmp = strdup(elm_layout_text_get(obj, "elm.top"));
        elm_layout_text_set(obj, "elm.top", label);
     }

   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 2, &minh);
   evas_object_size_hint_min_get(obj, &w, &h);

   if (sd->sentinel)
     {
        elm_layout_text_set(obj, "elm.top", tmp);
        free(tmp);
     }

   if (w > minw) minw = w;
   if (h > minh) minh = h;

   sd->evaluating = EINA_FALSE;

   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_update_view(Evas_Object *obj)
{
   const char *label;
   Elm_Flipselector_Item *item;

   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   label = NULL;
   item = DATA_GET(sd->current);
   if (item) label = item->label;

   elm_layout_text_set(obj, "elm.top", label ? label : "");
   elm_layout_text_set(obj, "elm.bottom", label ? label : "");

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Eina_List *l;
   Elm_Flipselector_Item *item;

   if (!label) return;

   if (part && strcmp(part, "default")) return;

   item = (Elm_Flipselector_Item *)it;
   ELM_FLIPSELECTOR_DATA_GET(WIDGET(item), sd);

   if (!sd->items) return;

   l = eina_list_data_find_list(sd->items, item);
   if (!l) return;

   eina_stringshare_del(item->label);
   item->label = eina_stringshare_add_length(label, sd->max_len);

   if (strlen(label) > strlen(elm_object_item_text_get(DATA_GET(sd->sentinel))))
     sd->sentinel = l;

   if (sd->current == l)
     {
        _update_view(WIDGET(item));
        elm_layout_sizing_eval(WIDGET(item));
     }
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "default")) return NULL;

   return ((Elm_Flipselector_Item *)it)->label;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   Elm_Flipselector_Item *item = (Elm_Flipselector_Item *)it;

   edje_object_signal_emit(VIEW(item), emission, source);
}

static inline void
_flipselector_walk(Elm_Flipselector_Smart_Data *sd)
{
   if (sd->walking < 0)
     {
        ERR("walking was negative. fixed!\n");
        sd->walking = 0;
     }
   sd->walking++;
}

static void
_sentinel_eval(Elm_Flipselector_Smart_Data *sd)
{
   Elm_Flipselector_Item *it;
   Eina_List *l;

   if (!sd->items)
     {
        sd->sentinel = NULL;
        return;
     }

   sd->sentinel = sd->items;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (strlen(elm_object_item_text_get((Elm_Object_Item *)it)) >
            strlen(elm_object_item_text_get(DATA_GET(sd->sentinel))))
          sd->sentinel = l;
     }
}

/* TODO: create a flag to avoid looping here all times */
static void
_flipselector_process_deletions(Elm_Flipselector_Smart_Data *sd)
{
   Eina_List *l;
   Elm_Flipselector_Item *it;
   Eina_Bool skip = EINA_TRUE;
   Eina_Bool sentinel_eval = EINA_FALSE;

   sd->walking++; /* avoid nested deletions */

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (!it->deleted) continue;

        if (sd->current == l)
          {
             if (sd->current == sd->sentinel) sentinel_eval = EINA_TRUE;
             sd->current = eina_list_prev(sd->current);
          }
        sd->items = eina_list_remove(sd->items, it);

        if (!sd->current) sd->current = sd->items;

        elm_widget_item_del(it);
        skip = EINA_FALSE;

        if (eina_list_count(sd->items) <= 1)
          elm_layout_signal_emit
            (ELM_WIDGET_DATA(sd)->obj, "elm,state,button,hidden", "elm");
        else
          elm_layout_signal_emit
            (ELM_WIDGET_DATA(sd)->obj, "elm,state,button,visible", "elm");
     }

   if (!skip) _update_view(ELM_WIDGET_DATA(sd)->obj);

   if (sentinel_eval) _sentinel_eval(sd);

   sd->walking--;
}

static inline void
_flipselector_unwalk(Elm_Flipselector_Smart_Data *sd)
{
   sd->walking--;

   if (sd->walking < 0)
     {
        ERR("walking became negative. fixed!\n");
        sd->walking = 0;
     }
   if (sd->walking) return;

   _flipselector_process_deletions(sd);
}

static void
_on_item_changed(Elm_Flipselector_Smart_Data *sd)
{
   Elm_Flipselector_Item *item;

   item = DATA_GET(sd->current);
   if (!item) return;
   if (sd->deleting) return;
   
   if (item->func)
     item->func((void *)item->base.data, WIDGET(item), item);
   if (!item->deleted)
     evas_object_smart_callback_call
       (ELM_WIDGET_DATA(sd)->obj, SIG_SELECTED, item);
}

static void
_send_msg(Elm_Flipselector_Smart_Data *sd,
          int flipside,
          char *label)
{
   Edje_Message_String msg;

   msg.str = label;
   edje_object_message_send
     (ELM_WIDGET_DATA(sd)->resize_obj, EDJE_MESSAGE_STRING, flipside, &msg);
   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);

   _on_item_changed(sd);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Flipselector_Item *item, *item2;
   Eina_List *l;

   item = (Elm_Flipselector_Item *)it;
   ELM_FLIPSELECTOR_DATA_GET(WIDGET(item), sd);

   if (sd->walking > 0)
     {
        item->deleted = EINA_TRUE;
        return EINA_FALSE;
     }

   _flipselector_walk(sd);

   EINA_LIST_FOREACH(sd->items, l, item2)
     {
        if (item2 == item)
          {
             if (sd->current == l)
               {
                  sd->current = l->prev;
                  if (!sd->current) sd->current = l->next;
                  if (sd->current)
                    {
                       item2 = sd->current->data;
                       _send_msg(sd, MSG_FLIP_DOWN, (char *)item2->label);
                    }
                  else _send_msg(sd, MSG_FLIP_DOWN, "");
               }
             sd->items = eina_list_remove_list(sd->items, l);
             break;
          }
     }

   eina_stringshare_del(item->label);
   _sentinel_eval(sd);
   _flipselector_unwalk(sd);

   return EINA_TRUE;
}

static Elm_Flipselector_Item *
_item_new(Evas_Object *obj,
          const char *label,
          Evas_Smart_Cb func,
          const void *data)
{
   unsigned int len;
   Elm_Flipselector_Item *it;

   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   it = elm_widget_item_new(obj, Elm_Flipselector_Item);
   if (!it) return NULL;

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);

   len = strlen(label);
   if (len > sd->max_len) len = sd->max_len;

   it->label = eina_stringshare_add_length(label, len);
   it->func = func;
   it->base.data = data;

   /* TODO: no view here, but if one desires general contents in the
    * future... */
   return it;
}

static Eina_Bool
_elm_flipselector_smart_theme(Evas_Object *obj)
{
   const char *max_len;

   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_flipselector_parent_sc)->theme(obj))
     return EINA_FALSE;

   max_len = edje_object_data_get(ELM_WIDGET_DATA(sd)->resize_obj, "max_len");
   if (!max_len) sd->max_len = MAX_LEN_DEFAULT;
   else
     {
        sd->max_len = atoi(max_len);

        if (!sd->max_len) sd->max_len = MAX_LEN_DEFAULT;
     }

   _update_view(obj);
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_flip_up(Elm_Flipselector_Smart_Data *sd)
{
   Elm_Flipselector_Item *item;

   if (!sd->current) return;

   if (sd->deleting) return;
   if (sd->current == sd->items)
     {
        sd->current = eina_list_last(sd->items);
        evas_object_smart_callback_call
          (ELM_WIDGET_DATA(sd)->obj, SIG_UNDERFLOWED, NULL);
     }
   else
     sd->current = eina_list_prev(sd->current);

   item = DATA_GET(sd->current);
   if (!item) return;

   _send_msg(sd, MSG_FLIP_UP, (char *)item->label);
}

static void
_flip_down(Elm_Flipselector_Smart_Data *sd)
{
   Elm_Flipselector_Item *item;

   if (!sd->current) return;

   if (sd->deleting) return;
   sd->current = eina_list_next(sd->current);
   if (!sd->current)
     {
        sd->current = sd->items;
        evas_object_smart_callback_call
          (ELM_WIDGET_DATA(sd)->obj, SIG_OVERFLOWED, NULL);
     }

   item = DATA_GET(sd->current);
   if (!item) return;

   _send_msg(sd, MSG_FLIP_DOWN, (char *)item->label);
}

static Eina_Bool
_elm_flipselector_smart_event(Evas_Object *obj,
                              Evas_Object *src __UNUSED__,
                              Evas_Callback_Type type,
                              void *event_info)
{
   Evas_Event_Key_Down *ev;
   Eina_Bool is_up = EINA_TRUE;

   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;

   ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if ((!strcmp(ev->keyname, "Down")) || (!strcmp(ev->keyname, "KP_Down")))
     is_up = EINA_FALSE;
   else if ((strcmp(ev->keyname, "Up")) && (strcmp(ev->keyname, "KP_Up")))
     return EINA_FALSE;

   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;

   /* TODO: if direction setting via API is not coming in, replace
      these calls by flip_{next,prev} */
   _flipselector_walk(sd);

   if (is_up) _flip_up(sd);
   else _flip_down(sd);

   _flipselector_unwalk(sd);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   return EINA_TRUE;
}

static Eina_Bool
_signal_val_up(void *data)
{
   ELM_FLIPSELECTOR_DATA_GET(data, sd);

   _flipselector_walk(sd);

   if (sd->interval > FLIP_MIN_INTERVAL) sd->interval = sd->interval / 1.05;

   ecore_timer_interval_set(sd->spin, sd->interval);

   _flip_up(sd);

   _flipselector_unwalk(sd);

   return ECORE_CALLBACK_RENEW;
}

static void
_signal_val_up_start(void *data,
                     Evas_Object *obj __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
{
   ELM_FLIPSELECTOR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;

   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _signal_val_up, data);

   _signal_val_up(data);
}

static Eina_Bool
_signal_val_down(void *data)
{
   ELM_FLIPSELECTOR_DATA_GET(data, sd);

   _flipselector_walk(sd);

   if (sd->interval > FLIP_MIN_INTERVAL) sd->interval = sd->interval / 1.05;
   ecore_timer_interval_set(sd->spin, sd->interval);

   _flip_down(sd);

   _flipselector_unwalk(sd);

   return ECORE_CALLBACK_RENEW;
}

static void
_signal_val_down_start(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   ELM_FLIPSELECTOR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;

   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _signal_val_down, data);

   _signal_val_down(data);
}

static void
_signal_val_change_stop(void *data,
                        Evas_Object *obj __UNUSED__,
                        const char *emission __UNUSED__,
                        const char *source __UNUSED__)
{
   ELM_FLIPSELECTOR_DATA_GET(data, sd);

   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;
}

static void
_elm_flipselector_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Flipselector_Smart_Data);

   ELM_WIDGET_CLASS(_elm_flipselector_parent_sc)->base.add(obj);

   elm_layout_theme_set
     (obj, "flipselector", "base", elm_widget_style_get(obj));

   elm_layout_signal_callback_add
     (obj, "elm,action,up,start", "", _signal_val_up_start, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,up,stop", "", _signal_val_change_stop, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,down,start", "", _signal_val_down_start, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,down,stop", "", _signal_val_change_stop, obj);

   priv->first_interval = FLIP_FIRST_INTERVAL;
   priv->evaluating = EINA_FALSE;

   elm_widget_can_focus_set(obj, EINA_TRUE);

   _elm_flipselector_smart_theme(obj);
}

static void
_elm_flipselector_smart_del(Evas_Object *obj)
{
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   sd->deleting = EINA_TRUE;
   
   if (sd->walking) ERR("flipselector deleted while walking.\n");

   while (sd->items)
     elm_widget_item_del(DATA_GET(sd->items));

   if (sd->spin) ecore_timer_del(sd->spin);

   ELM_WIDGET_CLASS(_elm_flipselector_parent_sc)->base.del(obj);
}

static void
_elm_flipselector_smart_set_user(Elm_Flipselector_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_flipselector_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_flipselector_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_flipselector_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_flipselector_smart_event;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_flipselector_smart_sizing_eval;
}

EAPI const Elm_Flipselector_Smart_Class *
elm_flipselector_smart_class_get(void)
{
   static Elm_Flipselector_Smart_Class _sc =
     ELM_FLIPSELECTOR_SMART_CLASS_INIT_NAME_VERSION
       (ELM_FLIPSELECTOR_SMART_NAME);
   static const Elm_Flipselector_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_flipselector_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_flipselector_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_flipselector_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_flipselector_flip_next(Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj);
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;

   _flipselector_walk(sd);
   _flip_down(sd);
   _flipselector_unwalk(sd);
}

EAPI void
elm_flipselector_flip_prev(Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj);

   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   if (sd->spin) ecore_timer_del(sd->spin);
   sd->spin = NULL;

   _flipselector_walk(sd);
   _flip_up(sd);
   _flipselector_unwalk(sd);
}

EAPI Elm_Object_Item *
elm_flipselector_item_append(Evas_Object *obj,
                             const char *label,
                             void (*func)(void *, Evas_Object *, void *),
                             void *data)
{
   Elm_Flipselector_Item *item;

   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   item = _item_new(obj, label, func, data);
   if (!item) return NULL;

   sd->items = eina_list_append(sd->items, item);
   if (!sd->current)
     {
        sd->current = sd->items;
        _update_view(obj);
     }

   if (!sd->sentinel ||
       (strlen(elm_object_item_text_get((Elm_Object_Item *)item)) >
        strlen(elm_object_item_text_get(DATA_GET(sd->sentinel)))))
     {
        sd->sentinel = eina_list_last(sd->items);
        elm_layout_sizing_eval(obj);
     }

   if (eina_list_count(sd->items) > 1)
     elm_layout_signal_emit(obj, "elm,state,button,visible", "elm");

   return (Elm_Object_Item *)item;
}

EAPI Elm_Object_Item *
elm_flipselector_item_prepend(Evas_Object *obj,
                              const char *label,
                              void (*func)(void *, Evas_Object *, void *),
                              void *data)
{
   Elm_Flipselector_Item *item;

   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   item = _item_new(obj, label, func, data);
   if (!item) return NULL;

   sd->items = eina_list_prepend(sd->items, item);
   if (!sd->current)
     {
        sd->current = sd->items;
        _update_view(obj);
     }

   if (!sd->sentinel ||
       (strlen(elm_object_item_text_get((Elm_Object_Item *)item)) >
        strlen(elm_object_item_text_get(DATA_GET(sd->sentinel)))))
     {
        sd->sentinel = sd->items;
        elm_layout_sizing_eval(obj);
     }

   if (eina_list_count(sd->items) >= 2)
     elm_layout_signal_emit(obj, "elm,state,button,visible", "elm");

   return (Elm_Object_Item *)item;
}

EAPI const Eina_List *
elm_flipselector_items_get(const Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   return sd->items;
}

EAPI Elm_Object_Item *
elm_flipselector_first_item_get(const Evas_Object *obj)
{
   Elm_Flipselector_Item *it;
   Eina_List *l;

   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   if (!sd->items) return NULL;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->deleted) continue;
        return (Elm_Object_Item *)it;
     }

   return NULL;
}

EAPI Elm_Object_Item *
elm_flipselector_last_item_get(const Evas_Object *obj)
{
   Elm_Flipselector_Item *it;
   Eina_List *l;

   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   if (!sd->items) return NULL;

   EINA_LIST_REVERSE_FOREACH(sd->items, l, it)
     {
        if (it->deleted) continue;
        return (Elm_Object_Item *)it;
     }

   return NULL;
}

EAPI Elm_Object_Item *
elm_flipselector_selected_item_get(const Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   return DATA_GET(sd->current);
}

EAPI void
elm_flipselector_item_selected_set(Elm_Object_Item *it,
                                   Eina_Bool selected)
{
   Elm_Flipselector_Item *item, *_item, *cur;
   int flipside = MSG_FLIP_UP;
   Eina_List *l;

   ELM_FLIPSELECTOR_ITEM_CHECK_OR_RETURN(it);

   item = (Elm_Flipselector_Item *)it;
   ELM_FLIPSELECTOR_DATA_GET(WIDGET(item), sd);

   cur = DATA_GET(sd->current);
   if ((selected) && (cur == item)) return;

   _flipselector_walk(sd);

   if ((!selected) && (cur == item))
     {
        EINA_LIST_FOREACH(sd->items, l, _item)
          {
             if (!_item->deleted)
               {
                  sd->current = l;
                  _send_msg(sd, MSG_FLIP_UP, (char *)_item->label);
                  break;
               }
          }
        _flipselector_unwalk(sd);
        return;
     }

   EINA_LIST_FOREACH(sd->items, l, _item)
     {
        if (_item == cur) flipside = MSG_FLIP_DOWN;

        if (_item == item)
          {
             sd->current = l;
             _send_msg(sd, flipside, (char *)item->label);
             break;
          }
     }

   _flipselector_unwalk(sd);
}

EAPI Eina_Bool
elm_flipselector_item_selected_get(const Elm_Object_Item *it)
{
   Elm_Flipselector_Item *item;

   ELM_FLIPSELECTOR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   item = (Elm_Flipselector_Item *)it;
   ELM_FLIPSELECTOR_DATA_GET(WIDGET(item), sd);

   return eina_list_data_get(sd->current) == item;
}

EAPI Elm_Object_Item *
elm_flipselector_item_prev_get(const Elm_Object_Item *it)
{
   Elm_Flipselector_Item *item = (Elm_Flipselector_Item *)it;
   Eina_List *l;

   ELM_FLIPSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_FLIPSELECTOR_DATA_GET(WIDGET(item), sd);

   if ((!sd->items)) return NULL;

   l = eina_list_data_find_list(sd->items, it);
   if (l && l->prev) return DATA_GET(l->prev);

   return NULL;
}

EAPI Elm_Object_Item *
elm_flipselector_item_next_get(const Elm_Object_Item *it)
{
   Eina_List *l;
   Elm_Flipselector_Item *item = (Elm_Flipselector_Item *)it;

   ELM_FLIPSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);
   ELM_FLIPSELECTOR_DATA_GET(WIDGET(item), sd);

   if ((!sd->items)) return NULL;

   l = eina_list_data_find_list(sd->items, it);
   if (l && l->next) return DATA_GET(l->next);

   return NULL;
}

EAPI void
elm_flipselector_first_interval_set(Evas_Object *obj,
                                    double interval)
{
   ELM_FLIPSELECTOR_CHECK(obj);
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   sd->first_interval = interval;
}

EAPI double
elm_flipselector_first_interval_get(const Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj) 0;
   ELM_FLIPSELECTOR_DATA_GET(obj, sd);

   return sd->first_interval;
}
