#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_dayselector.h"

EAPI const char ELM_DAYSELECTOR_SMART_NAME[] = "elm_dayselector";

/* signals to edc */
#define ITEM_TYPE_WEEKDAY_DEFAULT "elm,type,weekday,default"
#define ITEM_TYPE_WEEKDAY_STYLE1  "elm,type,weekday,style1"
#define ITEM_TYPE_WEEKEND_DEFAULT "elm,type,weekend,default"
#define ITEM_TYPE_WEEKEND_STYLE1  "elm,type,weekend,style1"
#define ITEM_POS_LEFT             "elm,pos,check,left"
#define ITEM_POS_RIGHT            "elm,pos,check,right"
#define ITEM_POS_MIDDLE           "elm,pos,check,middle"

static const char SIG_CHANGED[] = "dayselector,changed";
static const char SIG_LANG_CHANGED[] = "language,changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_LANG_CHANGED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_DAYSELECTOR_SMART_NAME, _elm_dayselector, Elm_Dayselector_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_elm_dayselector_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord min_w = -1, min_h = -1;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(ELM_DAYSELECTOR_MAX, &min_w, 1, &min_h);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &min_w, &min_h, min_w, min_h);
   elm_coords_finger_size_adjust(ELM_DAYSELECTOR_MAX, &min_w, 1, &min_h);
   evas_object_size_hint_min_set(obj, min_w, min_h);
}

static void
_dayselector_resize(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static Eina_Bool
_elm_dayselector_smart_translate(Evas_Object *obj)
{
   time_t t;
   Eina_List *l;
   char buf[1024];
   struct tm time_daysel;
   Elm_Dayselector_Item *it;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   t = time(NULL);
   localtime_r(&t, &time_daysel);
   EINA_LIST_FOREACH (sd->items, l, it)
     {
        time_daysel.tm_wday = it->day;
        strftime(buf, sizeof(buf), "%a", &time_daysel);
        elm_object_text_set(VIEW(it), buf);
     }

   evas_object_smart_callback_call(obj, SIG_LANG_CHANGED, NULL);

   return EINA_TRUE;
}

static void
_update_items(Evas_Object *obj)
{
   Eina_List *l;
   Eina_Bool rtl;
   unsigned int last_day;
   Elm_Dayselector_Item *it;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   last_day = sd->week_start + ELM_DAYSELECTOR_MAX - 1;
   if (last_day >= ELM_DAYSELECTOR_MAX)
     last_day = last_day % ELM_DAYSELECTOR_MAX;

   rtl = elm_widget_mirrored_get(obj);
   EINA_LIST_FOREACH (sd->items, l, it)
     {
        elm_object_signal_emit(VIEW(it), it->day_style, "");
        if (it->day == sd->week_start)
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

static inline unsigned int
_item_location_get(Elm_Dayselector_Smart_Data *sd,
                   Elm_Dayselector_Item *it)
{
   return (ELM_DAYSELECTOR_MAX - sd->week_start + it->day) %
          ELM_DAYSELECTOR_MAX;
}

static Eina_Bool
_elm_dayselector_smart_theme(Evas_Object *obj)
{
   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item *it;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_dayselector_parent_sc)->theme(obj))
     return EINA_FALSE;

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        snprintf
          (buf, sizeof(buf), "dayselector/%s", elm_object_style_get(obj));
        elm_object_style_set(VIEW(it), buf);

        snprintf
          (buf, sizeof(buf), "day%d,visible", _item_location_get(sd, it));
        elm_layout_signal_emit(obj, buf, "elm");
     }

   _update_items(obj);
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_item_del_cb(void *data,
             Evas *e __UNUSED__,
             Evas_Object *obj,
             void *event_info __UNUSED__)
{
   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item *it;

   ELM_DAYSELECTOR_DATA_GET(data, sd);

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if (obj == VIEW(it))
          {
             sd->items = eina_list_remove(sd->items, it);
             eina_stringshare_del(it->day_style);
             snprintf(buf, sizeof(buf), "day%d,default",
                      _item_location_get(sd, it));
             elm_layout_signal_emit(obj, buf, "elm");

             VIEW(it) = NULL;
             elm_widget_item_free(it);

             elm_layout_sizing_eval(obj);
             break;
          }
     }
}

static void
_item_signal_emit_cb(void *data,
                     Evas_Object *obj __UNUSED__,
                     const char *emission,
                     const char *source __UNUSED__)
{
   Elm_Dayselector_Item *it = data;

   eina_stringshare_replace(&it->day_style, emission);
}

static void
_item_clicked_cb(void *data,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   Elm_Dayselector_Item *it = data;

   evas_object_smart_callback_call(WIDGET(it), SIG_CHANGED, (void *)it->day);
}

static Elm_Dayselector_Item *
_item_find(const Evas_Object *obj,
           Elm_Dayselector_Day day)
{
   Eina_List *l;
   Elm_Dayselector_Item *it;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->items, l, it)
     if (day == it->day) return it;

   return NULL;
}

static Eina_Bool
_elm_dayselector_smart_content_set(Evas_Object *obj,
                                   const char *item,
                                   Evas_Object *content)
{
   int day;
   char buf[1024];
   Elm_Dayselector_Item *it = NULL;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   if (strcmp(elm_object_widget_type_get(content), "elm_check"))
     return EINA_FALSE;

   if (!item) return EINA_FALSE;

   day = atoi(item + (strlen(item) - 1));
   if (day < 0 || day > ELM_DAYSELECTOR_MAX) return EINA_FALSE;

   it = _item_find(obj, day);
   if (it)
     {
        snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));

        if (!ELM_CONTAINER_CLASS(_elm_dayselector_parent_sc)->content_set
              (obj, buf, content))
          return EINA_FALSE;

        if (!content) return EINA_TRUE;  /* item deletion already
                                          * handled */

        evas_object_del(VIEW(it));
        VIEW(it) = content;
     }
   else
     {
        it = elm_widget_item_new(obj, Elm_Dayselector_Item);
        it->day = day;

        snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));

        if (!ELM_CONTAINER_CLASS(_elm_dayselector_parent_sc)->content_set
              (obj, buf, content))
          {
             elm_widget_item_free(it);
             return EINA_FALSE;
          }

        sd->items = eina_list_append(sd->items, it);
        VIEW(it) = content;
     }

   snprintf(buf, sizeof(buf), "day%d,visible", _item_location_get(sd, it));
   elm_layout_signal_emit(obj, buf, "elm");

   evas_object_smart_callback_add(VIEW(it), "changed", _item_clicked_cb, it);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_DEL, _item_del_cb, obj);

   elm_object_signal_callback_add
     (VIEW(it), ITEM_TYPE_WEEKDAY_DEFAULT, "", _item_signal_emit_cb, it);
   elm_object_signal_callback_add
     (VIEW(it), ITEM_TYPE_WEEKDAY_STYLE1, "", _item_signal_emit_cb, it);
   elm_object_signal_callback_add
     (VIEW(it), ITEM_TYPE_WEEKEND_DEFAULT, "", _item_signal_emit_cb, it);
   elm_object_signal_callback_add
     (VIEW(it), ITEM_TYPE_WEEKEND_STYLE1, "", _item_signal_emit_cb, it);

   elm_layout_sizing_eval(obj);
   _update_items(obj);

   return EINA_TRUE;
}

static Evas_Object *
_elm_dayselector_smart_content_unset(Evas_Object *obj,
                                     const char *item)
{
   int day;
   char buf[1024];
   Evas_Object *content;
   Elm_Dayselector_Item *it = NULL;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   day = atoi(item + (strlen(item) - 1));
   if (day < 0 || day > ELM_DAYSELECTOR_MAX) return EINA_FALSE;

   it = _item_find(obj, day);
   if (!it) return NULL;

   content = VIEW(it);

   if (!ELM_CONTAINER_CLASS(_elm_dayselector_parent_sc)->content_unset
         (obj, buf))
     return EINA_FALSE;

   sd->items = eina_list_remove(sd->items, it);
   evas_object_smart_callback_del(content, "changed", _item_clicked_cb);
   evas_object_event_callback_del(content, EVAS_CALLBACK_DEL, _item_del_cb);

   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKDAY_DEFAULT, "", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKDAY_STYLE1, "", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKEND_DEFAULT, "", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKEND_STYLE1, "", _item_signal_emit_cb);

   snprintf(buf, sizeof(buf), "day%d,default", _item_location_get(sd, it));
   elm_layout_signal_emit(obj, buf, "elm");

   VIEW(it) = NULL;
   elm_widget_item_free(it);

   elm_layout_sizing_eval(obj);

   return content;
}

static void
_items_style_set(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Dayselector_Item *it;
   unsigned int weekend_last;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   weekend_last = sd->weekend_start + sd->weekend_len - 1;
   if (weekend_last >= ELM_DAYSELECTOR_MAX)
     weekend_last = weekend_last % ELM_DAYSELECTOR_MAX;

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if (weekend_last >= sd->weekend_start)
          {
             if ((it->day >= sd->weekend_start) && (it->day <= weekend_last))
               eina_stringshare_replace(&it->day_style,
                                        ITEM_TYPE_WEEKEND_DEFAULT);
             else
               eina_stringshare_replace(&it->day_style,
                                        ITEM_TYPE_WEEKDAY_DEFAULT);
          }
        else
          {
             if ((it->day >= sd->weekend_start) || (it->day <= weekend_last))
               eina_stringshare_replace(&it->day_style,
                                        ITEM_TYPE_WEEKEND_DEFAULT);
             else
               eina_stringshare_replace(&it->day_style,
                                        ITEM_TYPE_WEEKDAY_DEFAULT);
          }
     }
}

static void
_items_create(Evas_Object *obj)
{
   time_t t;
   char buf[1024];
   unsigned int idx;
   struct tm time_daysel;

   t = time(NULL);
   localtime_r(&t, &time_daysel);

   for (idx = 0; idx < ELM_DAYSELECTOR_MAX; idx++)
     {
        Evas_Object *chk;

        chk = elm_check_add(obj);
        elm_object_style_set(chk, "dayselector/default");

        time_daysel.tm_wday = idx;
        strftime(buf, sizeof(buf), "%a", &time_daysel);
        elm_object_text_set(chk, buf);

        snprintf(buf, sizeof(buf), "day%d", idx);
        elm_layout_content_set(obj, buf, chk);

        // XXX: ACCESS
        _elm_access_text_set(_elm_access_object_get(chk),
                 ELM_ACCESS_TYPE, E_("day selector item"));
     }

   _items_style_set(obj);
   _update_items(obj);
}

static void
_elm_dayselector_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Dayselector_Smart_Data);

   ELM_WIDGET_CLASS(_elm_dayselector_parent_sc)->base.add(obj);

   elm_layout_theme_set(obj, "dayselector", "base", "dayselector");

   priv->week_start = _elm_config->week_start;
   priv->weekend_start = _elm_config->weekend_start;
   priv->weekend_len = _elm_config->weekend_len;
   _items_create(obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _dayselector_resize, obj);

   elm_layout_sizing_eval(obj);
}

static void
_elm_dayselector_smart_del(Evas_Object *obj)
{
   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   Elm_Dayselector_Item *it;

   EINA_LIST_FREE (sd->items, it)
     {
        sd->items = eina_list_remove(sd->items, it);
        eina_stringshare_del(it->day_style);
        elm_widget_item_free(it);
     }

   /* handles freeing sd */
   ELM_WIDGET_CLASS(_elm_dayselector_parent_sc)->base.del(obj);
}

static void
_elm_dayselector_smart_set_user(Elm_Dayselector_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_dayselector_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_dayselector_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_dayselector_smart_theme;
   ELM_WIDGET_CLASS(sc)->translate = _elm_dayselector_smart_translate;

   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_dayselector_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_unset =
     _elm_dayselector_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_dayselector_smart_sizing_eval;
}

EAPI const Elm_Dayselector_Smart_Class *
elm_dayselector_smart_class_get(void)
{
   static Elm_Dayselector_Smart_Class _sc =
     ELM_DAYSELECTOR_SMART_CLASS_INIT_NAME_VERSION(ELM_DAYSELECTOR_SMART_NAME);
   static const Elm_Dayselector_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_dayselector_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_dayselector_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_dayselector_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_dayselector_day_selected_set(Evas_Object *obj,
                                 Elm_Dayselector_Day day,
                                 Eina_Bool selected)
{
   ELM_DAYSELECTOR_CHECK(obj);

   elm_check_state_set(VIEW(_item_find(obj, day)), selected);
}

EAPI Eina_Bool
elm_dayselector_day_selected_get(const Evas_Object *obj,
                                 Elm_Dayselector_Day day)
{
   ELM_DAYSELECTOR_CHECK(obj) EINA_FALSE;

   return elm_check_state_get(VIEW(_item_find(obj, day)));
}

EAPI void
elm_dayselector_week_start_set(Evas_Object *obj,
                               Elm_Dayselector_Day day)
{
   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item *it;

   ELM_DAYSELECTOR_CHECK(obj);
   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   /* just shuffling items, so swalling them directly */
   sd->week_start = day;
   EINA_LIST_FOREACH (sd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));
        edje_object_part_swallow
          (ELM_WIDGET_DATA(sd)->resize_obj, buf, VIEW(it));
     }

   _update_items(obj);
}

EAPI Elm_Dayselector_Day
elm_dayselector_week_start_get(const Evas_Object *obj)
{
   ELM_DAYSELECTOR_CHECK(obj) ELM_DAYSELECTOR_MAX;
   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   return sd->week_start;
}

EAPI void
elm_dayselector_weekend_start_set(Evas_Object *obj,
                                  Elm_Dayselector_Day day)
{
   ELM_DAYSELECTOR_CHECK(obj);
   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   sd->weekend_start = day;

   _items_style_set(obj);
   _update_items(obj);
}

EAPI Elm_Dayselector_Day
elm_dayselector_weekend_start_get(const Evas_Object *obj)
{
   ELM_DAYSELECTOR_CHECK(obj) ELM_DAYSELECTOR_MAX;
   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   return sd->weekend_start;
}

EAPI void
elm_dayselector_weekend_length_set(Evas_Object *obj,
                                   unsigned int length)
{
   ELM_DAYSELECTOR_CHECK(obj);
   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   sd->weekend_len = length;

   _items_style_set(obj);
   _update_items(obj);
}

EAPI unsigned int
elm_dayselector_weekend_length_get(const Evas_Object *obj)
{
   ELM_DAYSELECTOR_CHECK(obj) 0;
   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   return sd->weekend_len;
}
