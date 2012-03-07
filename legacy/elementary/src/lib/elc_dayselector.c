#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Dayselector_Item Elm_Dayselector_Item;

/* signals to edc */
#define ITEM_TYPE_WEEKDAY_DEFAULT    "elm,type,weekday,default"
#define ITEM_TYPE_WEEKDAY_STYLE1     "elm,type,weekday,style1"
#define ITEM_TYPE_WEEKEND_DEFAULT    "elm,type,weekend,default"
#define ITEM_TYPE_WEEKEND_STYLE1     "elm,type,weekend,style1"
#define ITEM_POS_LEFT                "elm,pos,check,left"
#define ITEM_POS_RIGHT               "elm,pos,check,right"
#define ITEM_POS_MIDDLE              "elm,pos,check,middle"

struct _Widget_Data
{
   Evas_Object *base;
   Eina_List *items;
   Elm_Dayselector_Day week_start;
   Elm_Dayselector_Day weekend_start;
   unsigned int weekend_len;
};

struct _Elm_Dayselector_Item
{
   ELM_WIDGET_ITEM;
   Elm_Dayselector_Day day;
   const char *day_style;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object* obj);
static void _dayselector_resize(void *data, Evas *e __UNUSED__,
            Evas_Object *obj __UNUSED__, void *event_info __UNUSED__);
static void _disable_hook(Evas_Object *obj);
static Eina_Bool _focus_next_hook(const Evas_Object *obj,
                                  Elm_Focus_Direction dir __UNUSED__,
                                  Evas_Object **next __UNUSED__);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _translate_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _item_text_set_hook(Evas_Object *obj, const char *item,
                                const char *label);
static const char *_item_text_get_hook(const Evas_Object *obj,
                                       const char *item);
static void _content_set_hook(Evas_Object *obj, const char *item,
                              Evas_Object *content);
static Evas_Object *_content_get_hook(const Evas_Object *obj, const char *item);
static Evas_Object *_content_unset_hook(Evas_Object *obj, const char *item);
static void _signal_emit_cb(void *data, Evas_Object *obj, const char *emission,
                            const char *source __UNUSED__);
static void _item_clicked_cb(void *data, Evas_Object *obj,
                             void *event_info __UNUSED__);
static Elm_Dayselector_Item * _item_find(const Evas_Object *obj,
                                         Elm_Dayselector_Day day);
static void _items_style_set(Evas_Object *obj);
static void _update_items(Evas_Object *obj);
static void _create_items(Evas_Object *obj);

static const char SIG_CHANGED[] = "dayselector,changed";
static const char SIG_LANG_CHANGED[] = "language,changed";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_CHANGED, ""},
       {SIG_LANG_CHANGED, ""},
       {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Elm_Dayselector_Item *it;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   EINA_LIST_FREE(wd->items, it)
     {
        wd->items = eina_list_remove(wd->items, it);
        eina_stringshare_del(it->day_style);
        elm_widget_item_free(it);
     }
   free(wd);
}

static void
_sizing_eval(Evas_Object* obj)
{
   Evas_Coord min_w = -1, min_h = -1;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_coords_finger_size_adjust(ELM_DAYSELECTOR_MAX, &min_w, 1, &min_h);
   edje_object_size_min_restricted_calc(wd->base, &min_w, &min_h, min_w, min_h);
   elm_coords_finger_size_adjust(ELM_DAYSELECTOR_MAX, &min_w, 1, &min_h);
   evas_object_size_hint_min_set(obj, min_w, min_h);
}

static void
_dayselector_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (elm_widget_disabled_get(obj))
     edje_object_signal_emit(wd->base, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(wd->base, "elm,state,enabled", "elm");
}

static Eina_Bool
_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir __UNUSED__,
                 Evas_Object **next __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   /* TODO: Focus switch support to Elm_widget_Item is not supported yet.*/
   return EINA_FALSE;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   edje_object_mirrored_set(wd->base, rtl);
}

static void
_translate_hook(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Dayselector_Item *it;
   struct tm time_daysel;
   time_t t;
   char buf[1024];
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   t = time(NULL);
   localtime_r(&t, &time_daysel);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        time_daysel.tm_wday = it->day;
        strftime(buf, sizeof(buf), "%a", &time_daysel);
        elm_object_text_set(VIEW(it), buf);
     }
   evas_object_smart_callback_call(obj, SIG_LANG_CHANGED, NULL);
}

static void
_theme_hook(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Dayselector_Item *it;
   char buf[1024];
   unsigned int loc;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->base, "dayselector", "base",
                         elm_widget_style_get(obj));

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "dayselector/%s", elm_object_style_get(obj));
        elm_object_style_set(VIEW(it), buf);
        loc = (ELM_DAYSELECTOR_MAX - wd->week_start + it->day) % ELM_DAYSELECTOR_MAX;
        snprintf(buf, sizeof(buf), "day%d,visible", loc);
        edje_object_signal_emit(wd->base, buf, "elm");
        snprintf(buf, sizeof(buf), "day%d", loc);
        edje_object_part_swallow(wd->base, buf, VIEW(it));
     }
   _update_items(obj);
   _sizing_eval(obj);
}

static void
_item_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj,
             void *event_info __UNUSED__)
{
   ELM_CHECK_WIDTYPE(data, widtype);
   Eina_List *l;
   Elm_Dayselector_Item *it;
   char buf[1024];
   unsigned int loc = 0;
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (obj == VIEW(it))
          {
             wd->items = eina_list_remove(wd->items, it);
             eina_stringshare_del(it->day_style);
             evas_object_smart_callback_del(VIEW(it), "changed", _item_clicked_cb);
             evas_object_event_callback_del(VIEW(it), EVAS_CALLBACK_DEL, _item_del_cb);
             edje_object_part_unswallow(wd->base, VIEW(it));
             loc = (ELM_DAYSELECTOR_MAX - wd->week_start + it->day) % ELM_DAYSELECTOR_MAX;
             snprintf(buf, sizeof(buf), "day%d,default", loc);
             edje_object_signal_emit(wd->base, buf, "elm");
             elm_widget_item_free(it);
             _sizing_eval(obj);
             break;
          }
     }
}

static void
_signal_emit_cb(void *data, Evas_Object *obj, const char *emission, const char *source __UNUSED__)
{
   Eina_List *l;
   Elm_Dayselector_Item *it;
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (obj == VIEW(it))
          {
             eina_stringshare_replace(&it->day_style, emission);
             return ;
          }
     }
}

static void
_item_text_set_hook(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_List *l;
   Elm_Dayselector_Item *it;
   char buf[1024];
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && !strcmp(item, "default")) return;
   if (!wd) return;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "day%d", it->day);
        if (!strncmp(buf, item, sizeof(buf)))
          {
             elm_object_text_set(VIEW(it), label);
             _sizing_eval(obj);
             break;
          }
     }
}

static const char *
_item_text_get_hook(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Eina_List *l;
   Elm_Dayselector_Item *it;
   char buf[1024];
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && !strcmp(item, "default")) return NULL;
   if (!wd) return NULL;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "day%d", it->day);
        if (!strncmp(buf, item, sizeof(buf)))
          return elm_object_text_get(VIEW(it));
     }
   return NULL;
}

static void
_content_set_hook(Evas_Object *obj, const char *item, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_List *l;
   Elm_Dayselector_Item *it;
   char buf[1024];
   unsigned int idx, loc;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd || !content) return;
   if (item && !strcmp(item, "default")) return;
   if(strcmp(elm_object_widget_type_get(content), "check")) return;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "day%d", it->day);
        if (!strncmp(buf, item, sizeof(buf)))
          {
             if (VIEW(it)) evas_object_del(VIEW(it));
             break;
          }
     }
   for (idx = 0; idx < ELM_DAYSELECTOR_MAX; idx++)
     {
        snprintf(buf, sizeof(buf), "day%d", idx);
        if (!strncmp(buf, item, sizeof(buf)))
          {
             it = elm_widget_item_new(obj, Elm_Dayselector_Item);
             if (!it) continue;
             VIEW(it) = content;
             it->day = idx;
             wd->items = eina_list_append(wd->items, it);
             elm_widget_sub_object_add(obj, content);
             loc = (ELM_DAYSELECTOR_MAX - wd->week_start + idx) % ELM_DAYSELECTOR_MAX;
             snprintf(buf, sizeof(buf), "day%d", loc);
             edje_object_part_swallow(wd->base, buf, VIEW(it));
             snprintf(buf, sizeof(buf), "day%d,visible", loc);
             edje_object_signal_emit(wd->base, buf, "elm");
             evas_object_smart_callback_add(VIEW(it), "changed", _item_clicked_cb, obj);
             evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_DEL, _item_del_cb, obj);
             elm_object_signal_callback_add(VIEW(it), ITEM_TYPE_WEEKDAY_DEFAULT, "", _signal_emit_cb, obj);
             elm_object_signal_callback_add(VIEW(it), ITEM_TYPE_WEEKDAY_STYLE1, "", _signal_emit_cb, obj);
             elm_object_signal_callback_add(VIEW(it), ITEM_TYPE_WEEKEND_DEFAULT, "", _signal_emit_cb, obj);
             elm_object_signal_callback_add(VIEW(it), ITEM_TYPE_WEEKEND_STYLE1, "", _signal_emit_cb, obj);
             _sizing_eval(obj);
             _update_items(obj);
             break;
          }
     }
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Eina_List *l;
   Elm_Dayselector_Item *it;
   char buf[1024];
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (item && !strcmp(item, "default")) return NULL;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "day%d", it->day);
        if (!strncmp(buf, item, sizeof(buf))) return VIEW(it);
     }
   return NULL;
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Eina_List *l;
   Elm_Dayselector_Item *it;
   Evas_Object *content;
   char buf[1024];
   unsigned int loc;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (item && !strcmp(item, "default")) return NULL;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "day%d", it->day);
        if (!strncmp(buf, item, sizeof(buf)))
          {
             content = VIEW(it);
             wd->items = eina_list_remove(wd->items, it);
             evas_object_smart_callback_del(VIEW(it), "changed", _item_clicked_cb);
             evas_object_event_callback_del(VIEW(it), EVAS_CALLBACK_DEL, _item_del_cb);
             edje_object_part_unswallow(wd->base, VIEW(it));
             loc = (ELM_DAYSELECTOR_MAX - wd->week_start + it->day) % ELM_DAYSELECTOR_MAX;
             snprintf(buf, sizeof(buf), "day%d,default", loc);
             edje_object_signal_emit(wd->base, buf, "elm");
             _sizing_eval(obj);
             return content;
          }
     }
   return NULL;
}

static void
_item_clicked_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Eina_List *l;
   Elm_Dayselector_Item *it;
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (obj != VIEW(it)) continue;
        evas_object_smart_callback_call(data, SIG_CHANGED, (void *)it->day);
        return;
     }
}

static Elm_Dayselector_Item *
_item_find(const Evas_Object *obj, Elm_Dayselector_Day day)
{
   Eina_List *l;
   Elm_Dayselector_Item *it;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   EINA_LIST_FOREACH(wd->items, l, it)
     if (day == it->day) return it;

   return NULL;
}

static void
_items_style_set(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_List *l;
   Elm_Dayselector_Item *it;
   unsigned int weekend_last;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   weekend_last = wd->weekend_start + wd->weekend_len - 1;
   if (weekend_last >= ELM_DAYSELECTOR_MAX)
     weekend_last = weekend_last % ELM_DAYSELECTOR_MAX;

   EINA_LIST_FOREACH(wd->items, l, it)
     {
        if (weekend_last >= wd->weekend_start)
          {
             if ((it->day >= wd->weekend_start) && (it->day <= weekend_last))
               eina_stringshare_replace(&it->day_style,
                                        ITEM_TYPE_WEEKEND_DEFAULT);
             else
               eina_stringshare_replace(&it->day_style,
                                        ITEM_TYPE_WEEKDAY_DEFAULT);
          }
        else
          {
             if ((it->day >= wd->weekend_start) || (it->day <= weekend_last))
               eina_stringshare_replace(&it->day_style,
                                        ITEM_TYPE_WEEKEND_DEFAULT);
             else
               eina_stringshare_replace(&it->day_style,
                                        ITEM_TYPE_WEEKDAY_DEFAULT);
          }
     }
}

static void
_update_items(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Dayselector_Item *it;
   unsigned int last_day;
   Eina_Bool rtl;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   last_day = wd->week_start + ELM_DAYSELECTOR_MAX - 1;
   if (last_day >= ELM_DAYSELECTOR_MAX)
     last_day = last_day % ELM_DAYSELECTOR_MAX;

   rtl = elm_widget_mirrored_get(obj);
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        elm_object_signal_emit(VIEW(it), it->day_style, "");
        if (it->day == wd->week_start)
          {
             if (rtl) elm_object_signal_emit(VIEW(it), ITEM_POS_RIGHT, "elm");
             else elm_object_signal_emit(VIEW(it), ITEM_POS_LEFT, "elm");
          }
        else if (it->day == last_day)
          {
             if (rtl) elm_object_signal_emit(VIEW(it), ITEM_POS_LEFT, "elm");
             else elm_object_signal_emit(VIEW(it), ITEM_POS_RIGHT, "elm");
          }
        else
          elm_object_signal_emit(VIEW(it), ITEM_POS_MIDDLE, "elm");
     }
}

static void
_create_items(Evas_Object *obj)
{
   Elm_Dayselector_Item *it;
   struct tm time_daysel;
   time_t t;
   char buf[1024];
   unsigned int idx, loc;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   t = time(NULL);
   localtime_r(&t, &time_daysel);

   for (idx = 0; idx < ELM_DAYSELECTOR_MAX; idx++)
     {
        it = elm_widget_item_new(obj, Elm_Dayselector_Item);
        if (!it) continue;
        VIEW(it) = elm_check_add(wd->base);
        elm_widget_sub_object_add(obj, VIEW(it));
        wd->items = eina_list_append(wd->items, it);
        elm_object_style_set(VIEW(it), "dayselector/default");
        time_daysel.tm_wday = idx;
        strftime(buf, sizeof(buf), "%a", &time_daysel);
        elm_object_text_set(VIEW(it), buf);
        it->day = idx;
        loc = (ELM_DAYSELECTOR_MAX - wd->week_start + idx) % ELM_DAYSELECTOR_MAX;
        snprintf(buf, sizeof(buf), "day%d", loc);
        edje_object_part_swallow(wd->base, buf, VIEW(it));
        snprintf(buf, sizeof(buf), "day%d,visible", idx);
        edje_object_signal_emit(wd->base, buf, "elm");
        evas_object_smart_callback_add(VIEW(it), "changed", _item_clicked_cb,
                                       obj);
        evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_DEL,
                                       _item_del_cb, obj);
        elm_object_signal_callback_add(VIEW(it), ITEM_TYPE_WEEKDAY_DEFAULT, "",
                                       _signal_emit_cb, obj);
        elm_object_signal_callback_add(VIEW(it), ITEM_TYPE_WEEKDAY_STYLE1, "",
                                       _signal_emit_cb, obj);
        elm_object_signal_callback_add(VIEW(it), ITEM_TYPE_WEEKEND_DEFAULT, "",
                                       _signal_emit_cb, obj);
        elm_object_signal_callback_add(VIEW(it), ITEM_TYPE_WEEKEND_STYLE1, "",
                                       _signal_emit_cb, obj);
     }
   _items_style_set(obj);
   _update_items(obj);
}

EAPI Evas_Object *
elm_dayselector_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "dayselector");
   elm_widget_type_set(obj, widtype);
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_focus_next_hook_set(obj, _focus_next_hook);
   elm_widget_translate_hook_set(obj, _translate_hook);
   elm_widget_text_set_hook_set(obj, _item_text_set_hook);
   elm_widget_text_get_hook_set(obj, _item_text_get_hook);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);

   wd->base = edje_object_add(e);
   _elm_theme_object_set(obj, wd->base, "dayselector", "base", "default");
   elm_object_style_set(wd->base, "dayselector");
   elm_widget_resize_object_set(obj, wd->base);

   wd->week_start = _elm_config->week_start;
   wd->weekend_start = _elm_config->weekend_start;
   wd->weekend_len = _elm_config->weekend_len;
   printf("%d %d %d\n", wd->week_start, wd->weekend_start, wd->weekend_len);
   _create_items(obj);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE,
                                  _dayselector_resize, obj);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_dayselector_day_selected_set(Evas_Object *obj, Elm_Dayselector_Day day, Eina_Bool selected)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_check_state_set(VIEW(_item_find(obj, day)), selected);
}

EAPI Eina_Bool
elm_dayselector_day_selected_get(const Evas_Object *obj, Elm_Dayselector_Day day)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   return elm_check_state_get(VIEW(_item_find(obj, day)));
}

EAPI void
elm_dayselector_week_start_set(Evas_Object *obj, Elm_Dayselector_Day day)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_List *l;
   Elm_Dayselector_Item *it;
   char buf[1024];
   unsigned int loc;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->week_start = day;
   EINA_LIST_FOREACH(wd->items, l, it)
     {
        loc = (ELM_DAYSELECTOR_MAX - wd->week_start + it->day) % ELM_DAYSELECTOR_MAX;
        snprintf(buf, sizeof(buf), "day%d", loc);
        edje_object_part_swallow(wd->base, buf, VIEW(it));
     }
   _update_items(obj);
}

EAPI Elm_Dayselector_Day
elm_dayselector_week_start_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_DAYSELECTOR_MAX;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return ELM_DAYSELECTOR_MAX;

   return wd->week_start;
}

EAPI void
elm_dayselector_weekend_start_set(Evas_Object *obj, Elm_Dayselector_Day day)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->weekend_start = day;
   _items_style_set(obj);
   _update_items(obj);
}

EAPI Elm_Dayselector_Day
elm_dayselector_weekend_start_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_DAYSELECTOR_MAX;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return ELM_DAYSELECTOR_MAX;

   return wd->weekend_start;
}

EAPI void
elm_dayselector_weekend_length_set(Evas_Object *obj, unsigned int length)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->weekend_len = length;
   _items_style_set(obj);
   _update_items(obj);
}

EAPI unsigned int
elm_dayselector_weekend_length_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0;
   Widget_Data* wd = elm_widget_data_get(obj);
   if (!wd) return 0;

   return wd->weekend_len;
}
