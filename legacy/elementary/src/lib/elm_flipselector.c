#include <Elementary.h>
#include "elm_priv.h"

/* TODO: ideally, the default theme would use map{} blocks on the TEXT
   parts to implement their fading in/out propertly (as in the clock
   widget) */
/* TODO: if one ever wants to extend it to receiving generic widgets
   as items, be my guest. in this case, remember to implement the
   items tooltip infra. */
/* TODO: implement disabled mode -- disable_hook() and stuff. */
/* TODO: fix default theme image borders for looong strings as item
   labels. */
/* TODO: set text elipsis on labels if one enforces mininum size on
 * the overall widget less the required for displaying it. */
/* TODO: find a way to, in the default theme, to detect we are
 * bootstrapping (receiving the 1st message) and populate the downmost
 * TEXT parts with the same text as the upmost, where appropriate. */

#define FLIP_FIRST_INTERVAL (0.85)
#define FLIP_MIN_INTERVAL (0.1)
#define MSG_FLIP_DOWN (1)
#define MSG_FLIP_UP (2)
#define MAX_LEN_DEFAULT (50)

#define DATA_GET eina_list_data_get

struct _Elm_Flipselector_Item
{
   Elm_Widget_Item base;
   const char *label;
   Evas_Smart_Cb func;
   void *data;
   int deleted : 1;
};

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *self;
   Evas_Object *base;
   Eina_List *items;
   Eina_List *current;
   Eina_List *sentinel; /* item containing the largest label string */
   int walking;
   unsigned int max_len;
   Ecore_Timer *spin;
   double interval, first_interval;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _update_view(Evas_Object *obj);
static void _callbacks_set(Evas_Object *obj);
static void _flip_up(Widget_Data *wd);
static void _flip_down(Widget_Data *wd);

static const char SIG_SELECTED[] = "selected";
static const char SIG_UNDERFLOWED[] = "underflowed";
static const char SIG_OVERFLOWED[] = "overflowed";
static const Evas_Smart_Cb_Description _signals[] = {
  {SIG_SELECTED, ""},
  {SIG_UNDERFLOWED, ""},
  {SIG_OVERFLOWED, ""},
  {NULL, NULL}
};

#define ELM_FLIPSELECTOR_ITEM_CHECK_DELETED_RETURN(it, ...)             \
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(it, __VA_ARGS__);            \
  if (it->deleted)                                                      \
    {                                                                   \
       ERR(""#it" has been DELETED.\n");                                \
       return __VA_ARGS__;                                              \
    }                                                                   \

static Elm_Flipselector_Item *
_item_new(Evas_Object *obj, const char *label, Evas_Smart_Cb func, const void *data)
{
   unsigned int len;
   Elm_Flipselector_Item *it;
   Widget_Data *wd = elm_widget_data_get(obj);

   it = elm_widget_item_new(obj, Elm_Flipselector_Item);
   if (!it)
     return NULL;

   len = strlen(label);
   if (len > wd->max_len)
     len = wd->max_len;

   it->label = eina_stringshare_add_length(label, len);
   it->func = func;
   it->base.data = data;

   /* TODO: no view here, but if one desires general contents in the
    * future... */
   return it;
}

static inline void
_item_free(Elm_Flipselector_Item *it)
{
   eina_stringshare_del(it->label);
   elm_widget_item_del(it);
}

static void
_del_hook(Evas_Object *obj)
{
   Elm_Flipselector_Item *item;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (wd->walking)
     ERR("flipselector deleted while walking.\n");

   EINA_LIST_FREE(wd->items, item)
      _item_free(item);

   if (wd->spin) ecore_timer_del(wd->spin);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   const char *max_len;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   _elm_theme_object_set(obj, wd->base, "flipselector", "base",
                         elm_widget_style_get(obj));
   edje_object_scale_set(wd->base,
                         elm_widget_scale_get(obj) * _elm_config->scale);

   max_len = edje_object_data_get(wd->base, "max_len");
   if (!max_len)
     wd->max_len = MAX_LEN_DEFAULT;
   else
     {
        wd->max_len = atoi(max_len);
        if (!wd->max_len)
          wd->max_len = MAX_LEN_DEFAULT;
     }

   _update_view(obj);
   _sizing_eval(obj);
}

static void
_sentinel_eval(Widget_Data *wd)
{
   Elm_Flipselector_Item *it;
   Eina_List *l;

   if (!wd->items)
     {
        wd->sentinel = NULL;
        return;
     }

   wd->sentinel = wd->items;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (strlen(elm_flipselector_item_label_get(it)) >
            strlen(elm_flipselector_item_label_get(DATA_GET(wd->sentinel))))
          wd->sentinel = l;
     }
}

/* TODO: create a flag to avoid looping here all times */
static void
_flipselector_process_deletions(Widget_Data *wd)
{
   Elm_Flipselector_Item *it;
   Eina_List *l;
   Eina_Bool skip = EINA_TRUE;
   Eina_Bool sentinel_eval = EINA_FALSE;

   wd->walking++; /* avoid nested deletions */

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (!it->deleted)
          continue;

        if (wd->current == l)
          {
             if (wd->current == wd->sentinel)
               sentinel_eval = EINA_TRUE;

             wd->current = eina_list_prev(wd->current);
          }
        wd->items = eina_list_remove(wd->items, it);

        if (!wd->current)
          wd->current = wd->items;

        _item_free(it);
        skip = EINA_FALSE;

        if (eina_list_count(wd->items) <= 1)
          edje_object_signal_emit(wd->base, "elm,state,button,hidden", "elm");
        else
          edje_object_signal_emit(wd->base, "elm,state,button,visible", "elm");
     }

   if (!skip)
     _update_view(wd->self);

   if (sentinel_eval)
     _sentinel_eval(wd);

   wd->walking--;
}

static inline void
_flipselector_walk(Widget_Data *wd)
{
   if (wd->walking < 0)
     {
        ERR("walking was negative. fixed!\n");
        wd->walking = 0;
     }
   wd->walking++;
}

static inline void
_flipselector_unwalk(Widget_Data *wd)
{
   wd->walking--;
   if (wd->walking < 0)
     {
        ERR("walking became negative. fixed!\n");
        wd->walking = 0;
     }

   if (wd->walking)
     return;

   _flipselector_process_deletions(wd);
}

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   Evas_Event_Key_Down *ev;
   Widget_Data *wd;
   Eina_Bool is_up = EINA_TRUE;

   if (type != EVAS_CALLBACK_KEY_DOWN)
     return EINA_FALSE;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return EINA_FALSE;

   ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return EINA_FALSE;

   if (elm_widget_disabled_get(obj))
     return EINA_FALSE;

   if ((!strcmp(ev->keyname, "Down")) || (!strcmp(ev->keyname, "KP_Down")))
     is_up = EINA_FALSE;
   else if ((strcmp(ev->keyname, "Up")) && (strcmp(ev->keyname, "KP_Up")))
     return EINA_FALSE;

   if (wd->spin) ecore_timer_del(wd->spin);

   /* TODO: if direction setting via API is not coming in, replace
      these calls by flip_{next,prev} */
   _flipselector_walk(wd);
   if (is_up)
     _flip_up(wd);
   else
     _flip_down(wd);
   _flipselector_unwalk(wd);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   /* FIXME: no treatment of this signal so far */
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->base, "elm,action,focus", "elm");
        evas_object_focus_set(wd->base, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->base, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->base, EINA_FALSE);
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   const char *tmp = NULL;
   Evas_Coord minw = -1, minh = -1, w, h;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   elm_coords_finger_size_adjust(1, &minw, 2, &minh);

   if (wd->sentinel)
     {
        const char *label = \
                            elm_flipselector_item_label_get(DATA_GET(wd->sentinel));

        tmp = edje_object_part_text_get(wd->base, "top");
        edje_object_part_text_set(wd->base, "top", label);
     }

   edje_object_size_min_restricted_calc(wd->base, &minw, &minh, minw, minh);
   elm_coords_finger_size_adjust(1, &minw, 2, &minh);
   evas_object_size_hint_min_get(obj, &w, &h);

   if (wd->sentinel)
     edje_object_part_text_set(wd->base, "top", tmp);

   if (w > minw) minw = w;
   if (h > minh) minh = h;

   evas_object_size_hint_min_set(obj, minw, minh);
}

static void
_update_view(Evas_Object *obj)
{
   Widget_Data *wd;
   const char *label;
   Elm_Flipselector_Item *item;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   label = NULL;
   item = DATA_GET(wd->current);
   if (item)
     label = item->label;

   edje_object_part_text_set(wd->base, "top", label ? label : "");
   edje_object_part_text_set(wd->base, "bottom", label ? label : "");

   edje_object_message_signal_process(wd->base);
}

static void
_changed(Widget_Data *wd)
{
   Elm_Flipselector_Item *item;

   item = DATA_GET(wd->current);
   if (!item)
     return;

   if (item->func)
     item->func((void *)item->base.data, item->base.widget, item);
   if (!item->deleted)
     evas_object_smart_callback_call(wd->self, SIG_SELECTED, item);
}

static void
_send_msg(Widget_Data *wd, int flipside, char *label)
{
   Edje_Message_String msg;

   msg.str = label;
   edje_object_message_send(wd->base, EDJE_MESSAGE_STRING, flipside, &msg);
   edje_object_message_signal_process(wd->base);

   _changed(wd);
}

static void
_flip_up(Widget_Data *wd)
{
   Elm_Flipselector_Item *item;

   if (!wd->current)
     return;

   if (wd->current == wd->items)
     {
        wd->current = eina_list_last(wd->items);
        evas_object_smart_callback_call(wd->self, SIG_UNDERFLOWED, NULL);
     }
   else
     wd->current = eina_list_prev(wd->current);

   item = DATA_GET(wd->current);
   if (!item)
     return;

   _send_msg(wd, MSG_FLIP_UP, (char *)item->label);
}

static Eina_Bool
_signal_val_up(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd)
     goto val_up_exit_on_error;

   _flipselector_walk(wd);

   if (wd->interval > FLIP_MIN_INTERVAL)
     wd->interval = wd->interval / 1.05;

   ecore_timer_interval_set(wd->spin, wd->interval);

   _flip_up(wd);

   _flipselector_unwalk(wd);

   return ECORE_CALLBACK_RENEW;

val_up_exit_on_error:
   return ECORE_CALLBACK_CANCEL;
}

static void
_signal_val_up_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd)
     return;

   wd->interval = wd->first_interval;

   if (wd->spin)
     ecore_timer_del(wd->spin);
   wd->spin = ecore_timer_add(wd->interval, _signal_val_up, data);

   _signal_val_up(data);
}

static void
_flip_down(Widget_Data *wd)
{
   Elm_Flipselector_Item *item;

   if (!wd->current)
     return;

   wd->current = eina_list_next(wd->current);
   if (!wd->current)
     {
        wd->current = wd->items;
        evas_object_smart_callback_call(wd->self, SIG_OVERFLOWED, NULL);
     }

   item = DATA_GET(wd->current);
   if (!item)
     return;

   _send_msg(wd, MSG_FLIP_DOWN, (char *)item->label);
}

static Eina_Bool
_signal_val_down(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd)
     goto val_down_exit_on_error;

   _flipselector_walk(wd);

   if (wd->interval > FLIP_MIN_INTERVAL)
     wd->interval = wd->interval / 1.05;
   ecore_timer_interval_set(wd->spin, wd->interval);

   _flip_down(wd);

   _flipselector_unwalk(wd);

   return ECORE_CALLBACK_RENEW;

val_down_exit_on_error:
   return ECORE_CALLBACK_CANCEL;
}

static void
_signal_val_down_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd)
     return;

   wd->interval = wd->first_interval;

   if (wd->spin)
     ecore_timer_del(wd->spin);
   wd->spin = ecore_timer_add(wd->interval, _signal_val_down, data);

   _signal_val_down(data);
}

static void
_signal_val_change_stop(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd)
     return;

   if (wd->spin)
     ecore_timer_del(wd->spin);
   wd->spin = NULL;
}

static void
_callbacks_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   edje_object_signal_callback_add(wd->base, "elm,action,up,start",
                                   "", _signal_val_up_start, obj);
   edje_object_signal_callback_add(wd->base, "elm,action,up,stop",
                                   "", _signal_val_change_stop, obj);
   edje_object_signal_callback_add(wd->base, "elm,action,down,start",
                                   "", _signal_val_down_start, obj);
   edje_object_signal_callback_add(wd->base, "elm,action,down,stop",
                                   "", _signal_val_change_stop, obj);
}

EAPI Evas_Object *
elm_flipselector_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "flipselector");
   elm_widget_type_set(obj, "flipselector");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);

   wd->self = obj;
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   /* TODO: elm_widget_disable_hook_set(obj, _disable_hook); */

   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_event_hook_set(obj, _event_hook);

   wd->base = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->base);

   _callbacks_set(obj);

   wd->first_interval = FLIP_FIRST_INTERVAL;

   _theme_hook(obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   return obj;
}

EAPI void
elm_flipselector_flip_next(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (wd->spin) ecore_timer_del(wd->spin);

   _flipselector_walk(wd);
   _flip_down(wd);
   _flipselector_unwalk(wd);
}

EAPI void
elm_flipselector_flip_prev(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   if (wd->spin) ecore_timer_del(wd->spin);

   _flipselector_walk(wd);
   _flip_up(wd);
   _flipselector_unwalk(wd);
}

EAPI Elm_Flipselector_Item *
elm_flipselector_item_append(Evas_Object *obj, const char *label, void (*func)(void *data, Evas_Object *obj, void *event_info), void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Elm_Flipselector_Item *item;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return NULL;

   item = _item_new(obj, label, func, data);
   if (!item)
     return NULL;

   wd->items = eina_list_append(wd->items, item);
   if (!wd->current) {
        wd->current = wd->items;
        _update_view(obj);
   }

   if (!wd->sentinel ||
       (strlen(elm_flipselector_item_label_get(item)) >
        strlen(elm_flipselector_item_label_get(DATA_GET(wd->sentinel)))))
     {
        wd->sentinel = eina_list_last(wd->items);
        _sizing_eval(obj);
     }

   if (eina_list_count(wd->items) >= 2)
     edje_object_signal_emit(wd->base, "elm,state,button,visible", "elm");

   return item;
}

EAPI Elm_Flipselector_Item *
elm_flipselector_item_prepend(Evas_Object *obj, const char *label, void (*func)(void *data, Evas_Object *obj, void *event_info), void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Elm_Flipselector_Item *item;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd)
     return NULL;

   item = _item_new(obj, label, func, data);
   if (!item)
     return NULL;

   wd->items = eina_list_prepend(wd->items, item);
   if (!wd->current) {
        wd->current = wd->items;
        _update_view(obj);
   }

   if (!wd->sentinel ||
       (strlen(elm_flipselector_item_label_get(item)) >
        strlen(elm_flipselector_item_label_get(DATA_GET(wd->sentinel)))))
     {
        wd->sentinel = wd->items;
        _sizing_eval(obj);
     }

   if (eina_list_count(wd->items) >= 2)
     edje_object_signal_emit(wd->base, "elm,state,button,visible", "elm");

   return item;
}

/* TODO: account for deleted items?  */
EAPI const Eina_List *
elm_flipselector_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return NULL;

   return wd->items;
}

EAPI Elm_Flipselector_Item *
elm_flipselector_first_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Elm_Flipselector_Item *it;
   Widget_Data *wd;
   Eina_List *l;

   wd = elm_widget_data_get(obj);
   if (!wd || !wd->items)
     return NULL;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (it->deleted)
          continue;

        return it;
     }

   return NULL;
}

EAPI Elm_Flipselector_Item *
elm_flipselector_last_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Elm_Flipselector_Item *it;
   Widget_Data *wd;
   Eina_List *l;

   wd = elm_widget_data_get(obj);
   if (!wd || !wd->items)
     return NULL;

   EINA_LIST_REVERSE_FOREACH(wd->items, l, it)
     {
        if (it->deleted)
          continue;

        return it;
     }

   return NULL;
}

EAPI Elm_Flipselector_Item *
elm_flipselector_selected_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd || !wd->current)
     return NULL;

   return DATA_GET(wd->current);
}

EAPI void
elm_flipselector_item_selected_set(Elm_Flipselector_Item *item, Eina_Bool selected)
{
   ELM_FLIPSELECTOR_ITEM_CHECK_DELETED_RETURN(item);

   Elm_Flipselector_Item *_item, *cur;
   int flipside = MSG_FLIP_UP;
   Widget_Data *wd;
   Eina_List *l;

   wd = elm_widget_data_get(item->base.widget);
   if (!wd)
     return;

   cur = DATA_GET(wd->current);
   if ((selected) && (cur == item))
     return;

   _flipselector_walk(wd);

   if ((!selected) && (cur == item))
     {
        EINA_LIST_FOREACH(wd->items, l, _item)
          {
             if (!_item->deleted)
               {
                  wd->current = l;
                  _send_msg(wd, MSG_FLIP_UP, (char *)_item->label);
                  break;
               }
          }
        _flipselector_unwalk(wd);
        return;
     }

   EINA_LIST_FOREACH(wd->items, l, _item)
     {
        if (_item == cur)
          flipside = MSG_FLIP_DOWN;

        if (_item == item)
          {
             wd->current = l;
             _send_msg(wd, flipside, (char *)item->label);
             break;
          }
     }

   _flipselector_unwalk(wd);
}

EAPI Eina_Bool
elm_flipselector_item_selected_get(const Elm_Flipselector_Item *item)
{
   ELM_FLIPSELECTOR_ITEM_CHECK_DELETED_RETURN(item, EINA_FALSE);
   Widget_Data *wd;

   wd = elm_widget_data_get(item->base.widget);
   if (!wd) return EINA_FALSE;
   return (eina_list_data_get(wd->current) == item);
}

EAPI void
elm_flipselector_item_del(Elm_Flipselector_Item *item)
{
   ELM_FLIPSELECTOR_ITEM_CHECK_DELETED_RETURN(item);
   Widget_Data *wd;
   Elm_Flipselector_Item *item2;
   Eina_List *l;

   wd = elm_widget_data_get(item->base.widget);
   if (!wd)
     return;

   if (wd->walking > 0)
     {
        item->deleted = EINA_TRUE;
        return;
     }

   _flipselector_walk(wd);

   EINA_LIST_FOREACH(wd->items, l, item2)
     {
        if (item2 == item)
          {
             wd->items = eina_list_remove_list(wd->items, l);
             if (wd->current == l)
               {
                  wd->current = l->prev;
                  if (!wd->current) wd->current = l->next;
                  if (wd->current)
                    {
                       item2 = wd->current->data;
                       _send_msg(wd, MSG_FLIP_DOWN, (char *)item2->label);
                    }
                  else
                     _send_msg(wd, MSG_FLIP_DOWN, "");
               }
             break;
          }
     }
   _item_free(item);
   _sentinel_eval(wd);

   _flipselector_unwalk(wd);
}

EAPI const char *
elm_flipselector_item_label_get(const Elm_Flipselector_Item *item)
{
   ELM_FLIPSELECTOR_ITEM_CHECK_DELETED_RETURN(item, NULL);

   Elm_Flipselector_Item *_item;
   Widget_Data *wd;
   Eina_List *l;

   wd = elm_widget_data_get(item->base.widget);
   if ((!wd) || (!wd->items))
     return NULL;

   EINA_LIST_FOREACH(wd->items, l, _item)
      if (_item == item)
        return item->label;

   return NULL;
}

EAPI void
elm_flipselector_item_label_set(Elm_Flipselector_Item *item, const char *label)
{
   ELM_FLIPSELECTOR_ITEM_CHECK_DELETED_RETURN(item);

   Widget_Data *wd;
   Eina_List *l;

   if ((!item) || (!label))
     return;

   wd = elm_widget_data_get(item->base.widget);
   if ((!wd) || (!wd->items))
     return;

   l = eina_list_data_find_list(wd->items, item);
   if (!l)
     return;

   eina_stringshare_del(item->label);
   item->label = eina_stringshare_add_length(label, wd->max_len);

   if (strlen(label) >
       strlen(elm_flipselector_item_label_get(DATA_GET(wd->sentinel))))
     wd->sentinel = l;

   if (wd->current == l)
     {
        _update_view(item->base.widget);
        _sizing_eval(wd->self);
     }

   return;
}

EAPI Elm_Flipselector_Item *
elm_flipselector_item_prev_get(Elm_Flipselector_Item *item)
{
   ELM_FLIPSELECTOR_ITEM_CHECK_DELETED_RETURN(item, NULL);

   Elm_Flipselector_Item *_item;
   Widget_Data *wd;
   Eina_List *l;

   wd = elm_widget_data_get(item->base.widget);
   if ((!wd) || (!wd->items))
     return NULL;

   EINA_LIST_FOREACH(wd->items, l, _item)
      if (_item == item)
        {
           l = eina_list_prev(l);
           if (!l)
             return NULL;
           return DATA_GET(l);
        }

   return NULL;
}

EAPI Elm_Flipselector_Item *
elm_flipselector_item_next_get(Elm_Flipselector_Item *item)
{
   ELM_FLIPSELECTOR_ITEM_CHECK_DELETED_RETURN(item, NULL);

   Elm_Flipselector_Item *_item;
   Widget_Data *wd;
   Eina_List *l;

   wd = elm_widget_data_get(item->base.widget);
   if ((!wd) || (!wd->items))
     return NULL;

   EINA_LIST_FOREACH(wd->items, l, _item)
      if (_item == item)
        {
           l = eina_list_next(l);
           if (!l)
             return NULL;
           return DATA_GET(l);
        }

   return NULL;
}

EAPI void
elm_flipselector_interval_set(Evas_Object *obj, double interval)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd)
     return;

   wd->first_interval = interval;
}

EAPI double
elm_flipselector_interval_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;

   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd)
     return 0.0;
   return wd->first_interval;
}
