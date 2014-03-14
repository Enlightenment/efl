#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_flipselector.h"

EAPI Eo_Op ELM_OBJ_FLIPSELECTOR_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_FLIPSELECTOR_CLASS

#define MY_CLASS_NAME "Elm_Flipselector"
#define MY_CLASS_NAME_LEGACY "elm_flipselector"

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
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void
_elm_flipselector_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   char *tmp = NULL;
   Evas_Coord minw = -1, minh = -1, w, h;

   Elm_Flipselector_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->evaluating) return;

   elm_coords_finger_size_adjust(1, &minw, 2, &minh);

   sd->evaluating = EINA_TRUE;

   if (sd->sentinel)
     {
        const char *label = elm_object_item_text_get(DATA_GET(sd->sentinel));
        const char *src = elm_layout_text_get(obj, "elm.top");

        if (src)
            tmp = strdup(src);
        elm_layout_text_set(obj, "elm.top", label);
     }

   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
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
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   label = NULL;
   item = DATA_GET(sd->current);
   if (item) label = item->label;

   elm_layout_text_set(obj, "elm.top", label ? label : "");
   elm_layout_text_set(obj, "elm.bottom", label ? label : "");

   edje_object_message_signal_process(wd->resize_obj);
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
   edje_object_signal_emit(VIEW(it), emission, source);
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
            (sd->obj, "elm,state,button,hidden", "elm");
        else
          elm_layout_signal_emit
            (sd->obj, "elm,state,button,visible", "elm");
     }

   if (!skip) _update_view(sd->obj);

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
       (sd->obj, SIG_SELECTED, item);
}

static void
_send_msg(Elm_Flipselector_Smart_Data *sd,
          int flipside,
          char *label)
{
   Edje_Message_String msg;
   ELM_WIDGET_DATA_GET_OR_RETURN(sd->obj, wd);

   msg.str = label;
   edje_object_message_send
     (wd->resize_obj, EDJE_MESSAGE_STRING, flipside, &msg);
   edje_object_message_signal_process(wd->resize_obj);

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

static void
_elm_flipselector_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   const char *max_len;

   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_FALSE;
   if (ret) *ret = EINA_FALSE;

   Elm_Flipselector_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   max_len = edje_object_data_get(wd->resize_obj, "max_len");
   if (!max_len) sd->max_len = MAX_LEN_DEFAULT;
   else
     {
        sd->max_len = atoi(max_len);

        if (!sd->max_len) sd->max_len = MAX_LEN_DEFAULT;
     }

   _update_view(obj);
   elm_layout_sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
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
          (sd->obj, SIG_UNDERFLOWED, NULL);
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
          (sd->obj, SIG_OVERFLOWED, NULL);
     }

   item = DATA_GET(sd->current);
   if (!item) return;

   _send_msg(sd, MSG_FLIP_DOWN, (char *)item->label);
}

static void
_elm_flipselector_smart_event(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Flipselector_Smart_Data *sd = _pd;
   Eina_Bool is_up = EINA_TRUE;

   if (ret) *ret = EINA_FALSE;
   (void) src;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   if ((!strcmp(ev->key, "Down")) || (!strcmp(ev->key, "KP_Down")))
     is_up = EINA_FALSE;
   else if ((strcmp(ev->key, "Up")) && (strcmp(ev->key, "KP_Up")))
     return;

   ELM_SAFE_FREE(sd->spin, ecore_timer_del);

   /* TODO: if direction setting via API is not coming in, replace
      these calls by flip_{next,prev} */
   _flipselector_walk(sd);

   if (is_up) _flip_up(sd);
   else _flip_down(sd);

   _flipselector_unwalk(sd);

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
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
                     Evas_Object *obj EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   ELM_FLIPSELECTOR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;

   ecore_timer_del(sd->spin);
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
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   ELM_FLIPSELECTOR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;

   ecore_timer_del(sd->spin);
   sd->spin = ecore_timer_add(sd->interval, _signal_val_down, data);

   _signal_val_down(data);
}

static void
_signal_val_change_stop(void *data,
                        Evas_Object *obj EINA_UNUSED,
                        const char *emission EINA_UNUSED,
                        const char *source EINA_UNUSED)
{
   ELM_FLIPSELECTOR_DATA_GET(data, sd);

   ELM_SAFE_FREE(sd->spin, ecore_timer_del);
}

static void
_elm_flipselector_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Flipselector_Smart_Data *priv = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set
       (obj, "flipselector", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_signal_callback_add
     (obj, "elm,action,up,start", "*", _signal_val_up_start, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,up,stop", "*", _signal_val_change_stop, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,down,start", "*", _signal_val_down_start, obj);
   elm_layout_signal_callback_add
     (obj, "elm,action,down,stop", "*", _signal_val_change_stop, obj);

   priv->first_interval = FLIP_FIRST_INTERVAL;

   elm_widget_can_focus_set(obj, EINA_TRUE);

   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

static void
_elm_flipselector_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Flipselector_Smart_Data *sd = _pd;

   sd->deleting = EINA_TRUE;

   if (sd->walking) ERR("flipselector deleted while walking.\n");

   while (sd->items)
     elm_widget_item_del(DATA_GET(sd->items));

   ecore_timer_del(sd->spin);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_flipselector_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Flipselector_Smart_Data *sd = _pd;
   sd->obj = obj;
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_flipselector_flip_next(Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_flipselector_flip_next());
}

static void
_flip_next(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Flipselector_Smart_Data *sd = _pd;

   ELM_SAFE_FREE(sd->spin, ecore_timer_del);

   _flipselector_walk(sd);
   _flip_down(sd);
   _flipselector_unwalk(sd);
}

EAPI void
elm_flipselector_flip_prev(Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_flipselector_flip_prev());
}

static void
_flip_prev(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Flipselector_Smart_Data *sd = _pd;

   ELM_SAFE_FREE(sd->spin, ecore_timer_del);

   _flipselector_walk(sd);
   _flip_up(sd);
   _flipselector_unwalk(sd);
}

EAPI Elm_Object_Item *
elm_flipselector_item_append(Evas_Object *obj,
                             const char *label,
                             void (*func)(void *, Evas_Object *, void *),
                             const void *data)
{
   Elm_Flipselector_Item *ret = NULL;
   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   eo_do((Eo *) obj, elm_obj_flipselector_item_append(label, func, data, &ret));
   return (Elm_Object_Item *)ret;
}

static void
_item_append(Eo *obj, void *_pd, va_list *list)
{
   const char *label = va_arg(*list, const char *);
   flipselector_func_type func = va_arg(*list, flipselector_func_type);
   void *data = va_arg(*list, void *);
   Elm_Flipselector_Item **ret = va_arg(*list, Elm_Flipselector_Item **);
   *ret = NULL;

   Elm_Flipselector_Item *item;

   Elm_Flipselector_Smart_Data *sd = _pd;

   item = _item_new(obj, label, func, data);
   if (!item) return;

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

   *ret = item;
}

EAPI Elm_Object_Item *
elm_flipselector_item_prepend(Evas_Object *obj,
                              const char *label,
                              void (*func)(void *, Evas_Object *, void *),
                              void *data)
{
   Elm_Flipselector_Item *ret = NULL;
   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   eo_do((Eo *) obj, elm_obj_flipselector_item_prepend(label, func, data, &ret));
   return (Elm_Object_Item *)ret;
}

static void
_item_prepend(Eo *obj, void *_pd, va_list *list)
{
   const char *label = va_arg(*list, const char *);
   flipselector_func_type func = va_arg(*list, flipselector_func_type);
   void *data = va_arg(*list, void *);
   Elm_Flipselector_Item **ret = va_arg(*list, Elm_Flipselector_Item **);
   *ret = NULL;

   Elm_Flipselector_Item *item;

   Elm_Flipselector_Smart_Data *sd = _pd;

   item = _item_new(obj, label, func, data);
   if (!item) return;

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

   *ret = item;
}

EAPI const Eina_List *
elm_flipselector_items_get(const Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   const Eina_List *ret = NULL;
   eo_do((Eo *) obj, elm_obj_flipselector_items_get(&ret));
   return ret;
}

static void
_items_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Elm_Flipselector_Smart_Data *sd = _pd;
   *ret = sd->items;
}

EAPI Elm_Object_Item *
elm_flipselector_first_item_get(const Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_flipselector_first_item_get(&ret));
   return ret;
}

static void
_first_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Flipselector_Item *it;
   Eina_List *l;

   Elm_Flipselector_Smart_Data *sd = _pd;

   if (!sd->items) return;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (it->deleted) continue;
        *ret = (Elm_Object_Item *)it;
        return;
     }
}

EAPI Elm_Object_Item *
elm_flipselector_last_item_get(const Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_flipselector_last_item_get(&ret));
   return ret;
}

static void
_last_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;

   Elm_Flipselector_Item *it;
   Eina_List *l;

   Elm_Flipselector_Smart_Data *sd = _pd;

   if (!sd->items) return;

   EINA_LIST_REVERSE_FOREACH(sd->items, l, it)
     {
        if (it->deleted) continue;
        *ret = (Elm_Object_Item *)it;
        return;
     }
}

EAPI Elm_Object_Item *
elm_flipselector_selected_item_get(const Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_flipselector_selected_item_get(&ret));
   return ret;
}

static void
_selected_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Flipselector_Smart_Data *sd = _pd;
   *ret = DATA_GET(sd->current);
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
   eo_do(obj, elm_obj_flipselector_first_interval_set(interval));
}

static void
_first_interval_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double interval = va_arg(*list, double);
   Elm_Flipselector_Smart_Data *sd = _pd;
   sd->first_interval = interval;
}

EAPI double
elm_flipselector_first_interval_get(const Evas_Object *obj)
{
   ELM_FLIPSELECTOR_CHECK(obj) 0;
   double ret = 0;
   eo_do((Eo *) obj, elm_obj_flipselector_first_interval_get(&ret));
   return ret;
}

static void
_first_interval_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Flipselector_Smart_Data *sd = _pd;
   *ret = sd->first_interval;
}

static void
_elm_flipselector_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_flipselector_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_flipselector_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_flipselector_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_flipselector_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_flipselector_smart_event),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_flipselector_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_flipselector_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_flipselector_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FLIP_NEXT), _flip_next),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FLIP_PREV), _flip_prev),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEM_APPEND), _item_append),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEM_PREPEND), _item_prepend),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEMS_GET), _items_get),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_ITEM_GET), _first_item_get),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_LAST_ITEM_GET), _last_item_get),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_SELECTED_ITEM_GET), _selected_item_get),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_INTERVAL_SET), _first_interval_set),
        EO_OP_FUNC(ELM_OBJ_FLIPSELECTOR_ID(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_INTERVAL_GET), _first_interval_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_FLIP_NEXT, "Programmatically select the next item of a flip selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_FLIP_PREV, "Programmatically select the previous item of a flip selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEM_APPEND, "Append a (text) item to a flip selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEM_PREPEND, "Prepend a (text) item to a flip selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_ITEMS_GET, "Get the internal list of items in a given flip selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_ITEM_GET, "Get the first item in the given flip selector widget's list of items."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_LAST_ITEM_GET, "Get the last item in the given flip selector widget's list of items."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_SELECTED_ITEM_GET, "Get the currently selected item in a flip selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_INTERVAL_SET, "Set the interval on time updates for a user mouse button hold on a flip selector widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_FLIPSELECTOR_SUB_ID_FIRST_INTERVAL_GET, "Get the interval on time updates for a user mouse button hold on a flip selector widget."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_FLIPSELECTOR_BASE_ID, op_desc, ELM_OBJ_FLIPSELECTOR_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Flipselector_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_flipselector_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
