#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_dayselector.h"
#include "elm_widget_container.h"

EAPI Eo_Op ELM_OBJ_DAYSELECTOR_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_DAYSELECTOR_CLASS

#define MY_CLASS_NAME "Elm_Dayselector"
#define MY_CLASS_NAME_LEGACY "elm_dayselector"

/* signals to edc */
#define ITEM_TYPE_WEEKDAY_DEFAULT "elm,type,weekday,default"
#define ITEM_TYPE_WEEKDAY_STYLE1  "elm,type,weekday,style1"
#define ITEM_TYPE_WEEKEND_DEFAULT "elm,type,weekend,default"
#define ITEM_TYPE_WEEKEND_STYLE1  "elm,type,weekend,style1"
#define ITEM_POS_LEFT             "elm,pos,check,left"
#define ITEM_POS_RIGHT            "elm,pos,check,right"
#define ITEM_POS_MIDDLE           "elm,pos,check,middle"

static const char SIG_CHANGED[] = "dayselector,changed";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static void
_elm_dayselector_smart_sizing_eval(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Coord min_w = -1, min_h = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_coords_finger_size_adjust(ELM_DAYSELECTOR_MAX, &min_w, 1, &min_h);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &min_w, &min_h, min_w, min_h);
   elm_coords_finger_size_adjust(ELM_DAYSELECTOR_MAX, &min_w, 1, &min_h);
   evas_object_size_hint_min_set(obj, min_w, min_h);
}

static void
_dayselector_resize(void *data,
                    Evas *e EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_elm_dayselector_smart_translate(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   time_t t;
   Eina_List *l;
   char buf[1024];
   struct tm time_daysel;
   Elm_Dayselector_Item *it;

   Elm_Dayselector_Smart_Data *sd = _pd;

   if (sd->weekdays_names_set)
     goto exit;

   t = time(NULL);
   localtime_r(&t, &time_daysel);
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        time_daysel.tm_wday = it->day;
        strftime(buf, sizeof(buf), "%a", &time_daysel);
        elm_object_text_set(VIEW(it), buf);
     }

   eo_do_super(obj, MY_CLASS, elm_obj_widget_translate(NULL));

 exit:
   if (ret) *ret = EINA_TRUE;
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
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        elm_object_signal_emit(VIEW(it), it->day_style, ""); // XXX: compat
        elm_object_signal_emit(VIEW(it), it->day_style, "elm");
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

static void
_elm_dayselector_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item *it;

   Elm_Dayselector_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   EINA_LIST_FOREACH(sd->items, l, it)
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

   if (ret) *ret = EINA_TRUE;
}

static void
_item_del_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj,
             void *event_info EINA_UNUSED)
{
   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item *it;

   ELM_DAYSELECTOR_DATA_GET(data, sd);

   EINA_LIST_FOREACH(sd->items, l, it)
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
                     Evas_Object *obj EINA_UNUSED,
                     const char *emission,
                     const char *source EINA_UNUSED)
{
   Elm_Dayselector_Item *it = data;

   eina_stringshare_replace(&it->day_style, emission);
}

static void
_item_clicked_cb(void *data,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
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

   EINA_LIST_FOREACH(sd->items, l, it)
     if (day == it->day) return it;

   return NULL;
}

static void
_elm_dayselector_smart_content_set(Eo *obj, void *_pd, va_list *list)
{
   const char *item= va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   int day;
   char buf[1024];
   Elm_Dayselector_Item *it = NULL;

   Elm_Dayselector_Smart_Data *sd = _pd;

   if (strcmp(elm_object_widget_type_get(content), "Elm_Check"))
     return;

   if (!item) return;

   day = atoi(item + (strlen(item) - 1));
   if (day < 0 || day > ELM_DAYSELECTOR_MAX) return;

   it = _item_find(obj, day);
   if (it)
     {
        snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));

        eo_do_super(obj, MY_CLASS, elm_obj_container_content_set(buf, content, &int_ret));
        if (!int_ret) return;

        if (!content)
          {
             if (ret) *ret = EINA_TRUE;  /* item deletion already handled */
             return;
          }

        evas_object_del(VIEW(it));
        VIEW(it) = content;
     }
   else
     {
        it = elm_widget_item_new(obj, Elm_Dayselector_Item);
        it->day = day;

        snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));

        eo_do_super(obj, MY_CLASS, elm_obj_container_content_set(buf, content, &int_ret));
        if (!int_ret)
          {
             elm_widget_item_free(it);
             return;
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
     (VIEW(it), ITEM_TYPE_WEEKDAY_DEFAULT, "*", _item_signal_emit_cb, it);
   elm_object_signal_callback_add
     (VIEW(it), ITEM_TYPE_WEEKDAY_STYLE1, "*", _item_signal_emit_cb, it);
   elm_object_signal_callback_add
     (VIEW(it), ITEM_TYPE_WEEKEND_DEFAULT, "*", _item_signal_emit_cb, it);
   elm_object_signal_callback_add
     (VIEW(it), ITEM_TYPE_WEEKEND_STYLE1, "*", _item_signal_emit_cb, it);

   elm_layout_sizing_eval(obj);
   _update_items(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_dayselector_smart_content_unset(Eo *obj, void *_pd, va_list *list)
{
   const char *item = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (ret) *ret = NULL;

   int day;
   char buf[1024];
   Evas_Object *content;
   Elm_Dayselector_Item *it = NULL;

   Elm_Dayselector_Smart_Data *sd = _pd;

   day = atoi(item + (strlen(item) - 1));
   if (day < 0 || day > ELM_DAYSELECTOR_MAX) return;

   it = _item_find(obj, day);
   if (!it) return;

   content = VIEW(it);

   eo_do_super(obj, MY_CLASS, elm_obj_container_content_unset(buf, &content));
   if (!content) return;

   sd->items = eina_list_remove(sd->items, it);
   evas_object_smart_callback_del(content, "changed", _item_clicked_cb);
   evas_object_event_callback_del(content, EVAS_CALLBACK_DEL, _item_del_cb);

   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKDAY_DEFAULT, "*", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKDAY_STYLE1, "*", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKEND_DEFAULT, "*", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKEND_STYLE1, "*", _item_signal_emit_cb);

   snprintf(buf, sizeof(buf), "day%d,default", _item_location_get(sd, it));
   elm_layout_signal_emit(obj, buf, "elm");

   VIEW(it) = NULL;
   elm_widget_item_free(it);

   elm_layout_sizing_eval(obj);

   if (ret) *ret = content;
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

   EINA_LIST_FOREACH(sd->items, l, it)
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
        _elm_access_text_set(_elm_access_info_get(chk),
                 ELM_ACCESS_TYPE, E_("day selector item"));
     }

   _items_style_set(obj);
   _update_items(obj);
}

static void
_elm_dayselector_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Dayselector_Smart_Data *priv = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set(obj, "dayselector", "base",
                             elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   priv->week_start = _elm_config->week_start;
   priv->weekend_start = _elm_config->weekend_start;
   priv->weekend_len = _elm_config->weekend_len;
   _items_create(obj);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _dayselector_resize, obj);

   elm_layout_sizing_eval(obj);
}

static void
_elm_dayselector_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Dayselector_Smart_Data *sd = _pd;

   Elm_Dayselector_Item *it;

   EINA_LIST_FREE(sd->items, it)
     {
        sd->items = eina_list_remove(sd->items, it);
        eina_stringshare_del(it->day_style);
        elm_widget_item_free(it);
     }

   /* handles freeing sd */
   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

EAPI Evas_Object *
elm_dayselector_add(Evas_Object *parent)
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
elm_dayselector_day_selected_set(Evas_Object *obj,
                                 Elm_Dayselector_Day day,
                                 Eina_Bool selected)
{
   ELM_DAYSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_dayselector_day_selected_set(day, selected));
}

static void
_day_selected_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Dayselector_Day day = va_arg(*list, Elm_Dayselector_Day);
   Eina_Bool selected = va_arg(*list, int);

   elm_check_state_set(VIEW(_item_find(obj, day)), selected);
}

EAPI Eina_Bool
elm_dayselector_day_selected_get(const Evas_Object *obj,
                                 Elm_Dayselector_Day day)
{
   ELM_DAYSELECTOR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_dayselector_day_selected_get(day, &ret));
   return ret;
}

static void
_day_selected_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Dayselector_Day day = va_arg(*list, Elm_Dayselector_Day);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   *ret = elm_check_state_get(VIEW(_item_find(obj, day)));
}

EAPI void
elm_dayselector_week_start_set(Evas_Object *obj,
                               Elm_Dayselector_Day day)
{
   ELM_DAYSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_dayselector_week_start_set(day));
}

static void
_week_start_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Dayselector_Day day = va_arg(*list, Elm_Dayselector_Day);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item *it;

   Elm_Dayselector_Smart_Data *sd = _pd;

   /* just shuffling items, so swalling them directly */
   sd->week_start = day;
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));
        edje_object_part_swallow
          (wd->resize_obj, buf, VIEW(it));
     }

   _update_items(obj);
}

EAPI Elm_Dayselector_Day
elm_dayselector_week_start_get(const Evas_Object *obj)
{
   ELM_DAYSELECTOR_CHECK(obj) ELM_DAYSELECTOR_MAX;
   Elm_Dayselector_Day ret = ELM_DAYSELECTOR_MAX;
   eo_do((Eo *) obj, elm_obj_dayselector_week_start_get(&ret));
   return ret;
}

static void
_week_start_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Dayselector_Day *ret = va_arg(*list, Elm_Dayselector_Day *);
   Elm_Dayselector_Smart_Data *sd = _pd;

   *ret = sd->week_start;
}

EAPI void
elm_dayselector_weekend_start_set(Evas_Object *obj,
                                  Elm_Dayselector_Day day)
{
   ELM_DAYSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_dayselector_weekend_start_set(day));
}

static void
_weekend_start_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Dayselector_Day day = va_arg(*list, Elm_Dayselector_Day);
   Elm_Dayselector_Smart_Data *sd = _pd;

   sd->weekend_start = day;

   _items_style_set(obj);
   _update_items(obj);
}

EAPI Elm_Dayselector_Day
elm_dayselector_weekend_start_get(const Evas_Object *obj)
{
   ELM_DAYSELECTOR_CHECK(obj) ELM_DAYSELECTOR_MAX;
   Elm_Dayselector_Day ret = ELM_DAYSELECTOR_MAX;
   eo_do((Eo *) obj, elm_obj_dayselector_weekend_start_get(&ret));
   return ret;
}

static void
_weekend_start_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Dayselector_Day *ret = va_arg(*list, Elm_Dayselector_Day *);
   Elm_Dayselector_Smart_Data *sd = _pd;

   *ret = sd->weekend_start;
}

EAPI void
elm_dayselector_weekend_length_set(Evas_Object *obj,
                                   unsigned int length)
{
   ELM_DAYSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_dayselector_weekend_length_set(length));
}

static void
_weekend_length_set(Eo *obj, void *_pd, va_list *list)
{
   unsigned int length = va_arg(*list, unsigned int);
   Elm_Dayselector_Smart_Data *sd = _pd;

   sd->weekend_len = length;

   _items_style_set(obj);
   _update_items(obj);
}

EAPI unsigned int
elm_dayselector_weekend_length_get(const Evas_Object *obj)
{
   ELM_DAYSELECTOR_CHECK(obj) 0;
   unsigned int ret = 0;
   eo_do((Eo *) obj, elm_obj_dayselector_weekend_length_get(&ret));
   return ret;
}

static void
_weekend_length_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int *ret = va_arg(*list, unsigned int *);
   Elm_Dayselector_Smart_Data *sd = _pd;

   *ret = sd->weekend_len;
}

EAPI void
elm_dayselector_weekdays_names_set(Evas_Object *obj, const char *weekdays[])
{
   ELM_DAYSELECTOR_CHECK(obj);
   eo_do((Eo *)obj, elm_obj_dayselector_weekdays_names_set(weekdays));
}

EAPI Eina_List *
elm_dayselector_weekdays_names_get(const Evas_Object *obj)
{
   Eina_List *weekdays = NULL;

   ELM_DAYSELECTOR_CHECK(obj) NULL;

   eo_do((Eo *)obj, elm_obj_dayselector_weekdays_names_get(&weekdays));

   return weekdays;
}

static void
_weekdays_name_set(Eo *obj, void *_pd, va_list *list)
{
   int idx;
   time_t now;
   struct tm time_daysel;
   Elm_Dayselector_Item *it;
   char buf[1024];
   const char **weekdays = va_arg(*list, const char **);
   Elm_Dayselector_Smart_Data *sd = _pd;

   if (weekdays)
     sd->weekdays_names_set = EINA_TRUE;
   else
     {
        now = time(NULL);
        localtime_r(&now, &time_daysel);
        sd->weekdays_names_set = EINA_FALSE;
     }

   for (idx = 0; idx < ELM_DAYSELECTOR_MAX; idx++)
     {
        it = _item_find(obj, idx);

        if (sd->weekdays_names_set)
          elm_object_text_set(VIEW(it), weekdays[idx]);
        else
          {
             time_daysel.tm_wday = idx;
             strftime(buf, sizeof(buf), "%a", &time_daysel);
             elm_object_text_set(VIEW(it), buf);
          }
     }
}

static void
_weekdays_name_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   int idx;
   const char *weekday;
   Elm_Dayselector_Item *it;
   Eina_List **weekdays = va_arg(*list, Eina_List **);

   for (idx = 0; idx < ELM_DAYSELECTOR_MAX; idx++)
     {
        it = _item_find(obj, idx);
        weekday = elm_object_text_get(VIEW(it));
        *weekdays = eina_list_append(*weekdays, eina_stringshare_add(weekday));
     }
}

static void
_elm_dayselector_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_dayselector_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_dayselector_smart_del),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_dayselector_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_TRANSLATE), _elm_dayselector_smart_translate),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_dayselector_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_dayselector_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_dayselector_smart_content_unset),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_dayselector_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_DAY_SELECTED_SET), _day_selected_set),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_DAY_SELECTED_GET), _day_selected_get),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEK_START_SET), _week_start_set),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEK_START_GET), _week_start_get),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_START_SET), _weekend_start_set),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_START_GET), _weekend_start_get),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_LENGTH_SET), _weekend_length_set),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_LENGTH_GET), _weekend_length_get),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKDAYS_NAMES_SET), _weekdays_name_set),
        EO_OP_FUNC(ELM_OBJ_DAYSELECTOR_ID(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKDAYS_NAMES_GET), _weekdays_name_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_DAY_SELECTED_SET, "Set the state of given Dayselector_Day."),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_DAY_SELECTED_GET, "Get the state of given Dayselector_Day."),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEK_START_SET, "Set the starting day of Dayselector."),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEK_START_GET, "Get the starting day of Dayselector."),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_START_SET, "Set the weekend starting day of Dayselector."),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_START_GET, "Get the weekend starting day of Dayselector."),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_LENGTH_SET, "Set the weekend length of Dayselector."),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKEND_LENGTH_GET, "Get the weekend length of Dayselector."),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKDAYS_NAMES_SET, "Set how the weekdays are displayed to the user"),
     EO_OP_DESCRIPTION(ELM_OBJ_DAYSELECTOR_SUB_ID_WEEKDAYS_NAMES_GET, "Get how the weekdays are displayed to the user"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_DAYSELECTOR_BASE_ID, op_desc, ELM_OBJ_DAYSELECTOR_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Dayselector_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_dayselector_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
