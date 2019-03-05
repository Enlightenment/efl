#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_UI_L10N_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_dayselector.h"
#include "elm_widget_container.h"

#include "elm_dayselector_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_DAYSELECTOR_CLASS

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

EOLIAN static void
_elm_dayselector_elm_layout_sizing_eval(Eo *obj, Elm_Dayselector_Data *sd EINA_UNUSED)
{
   Evas_Coord min_w = -1, min_h = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_coords_finger_size_adjust(ELM_DAYSELECTOR_MAX, &min_w, 1, &min_h);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &min_w, &min_h, min_w, min_h);
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

EOLIAN static void
_elm_dayselector_efl_ui_l10n_translation_update(Eo *obj EINA_UNUSED, Elm_Dayselector_Data *sd)
{
   time_t t;
   Eina_List *l;
   char buf[1024];
   struct tm time_daysel;
   Elm_Dayselector_Item_Data *it;

   if (sd->weekdays_names_set)
     return;

   t = time(NULL);
   localtime_r(&t, &time_daysel);
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        time_daysel.tm_wday = it->day;
        strftime(buf, sizeof(buf), "%a", &time_daysel);
        elm_object_text_set(VIEW(it), buf);
     }

   efl_ui_l10n_translation_update(efl_super(obj, MY_CLASS));
}

static void
_update_items(Evas_Object *obj)
{
   Eina_List *l;
   Eina_Bool rtl;
   unsigned int last_day;
   Elm_Dayselector_Item_Data *it;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   last_day = sd->week_start + ELM_DAYSELECTOR_MAX - 1;
   if (last_day >= ELM_DAYSELECTOR_MAX)
     last_day = last_day % ELM_DAYSELECTOR_MAX;

   rtl = efl_ui_mirrored_get(obj);
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
_item_location_get(Elm_Dayselector_Data *sd,
                   Elm_Dayselector_Item_Data *it)
{
   return (ELM_DAYSELECTOR_MAX - sd->week_start + it->day) %
          ELM_DAYSELECTOR_MAX;
}

EOLIAN static Eina_Error
_elm_dayselector_efl_ui_widget_theme_apply(Eo *obj, Elm_Dayselector_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item_Data *it;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        snprintf
          (buf, sizeof(buf), "dayselector/%s", elm_object_style_get(obj));
        elm_object_style_set(VIEW(it), buf);

        /* XXX kept for legacy compatibility, remove eventually */
        snprintf
          (buf, sizeof(buf), "day%d,visible", _item_location_get(sd, it));
        elm_layout_signal_emit(obj, buf, "elm");
        /* XXX */
        snprintf
          (buf, sizeof(buf), "elm,day%d,visible", _item_location_get(sd, it));
        elm_layout_signal_emit(obj, buf, "elm");
     }

   _update_items(obj);
   elm_layout_sizing_eval(obj);

   return int_ret;
}

static void
_item_del_cb(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj,
             void *event_info EINA_UNUSED)
{
   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item_Data *it;

   ELM_DAYSELECTOR_DATA_GET(data, sd);

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (obj == VIEW(it))
          {
             sd->items = eina_list_remove(sd->items, it);
             eina_stringshare_del(it->day_style);
             /* XXX kept for legacy compatibility, remove eventually */
             snprintf(buf, sizeof(buf), "day%d,default",
                      _item_location_get(sd, it));
             elm_layout_signal_emit(obj, buf, "elm");
             /* XXX */
             snprintf(buf, sizeof(buf), "elm,day%d,default",
                      _item_location_get(sd, it));
             elm_layout_signal_emit(obj, buf, "elm");

             // The object is already being deleted, there is no point in calling efl_del on it nore setting it to NULL.

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
   Elm_Dayselector_Item_Data *it = data;

   eina_stringshare_replace(&it->day_style, emission);
}

static void
_item_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Elm_Dayselector_Item_Data *it = data;

   efl_event_callback_legacy_call(WIDGET(it), EFL_UI_CHECK_EVENT_CHANGED, (void *)it->day);
}

static Elm_Dayselector_Item_Data *
_item_find(const Evas_Object *obj,
           Elm_Dayselector_Day day)
{
   Eina_List *l;
   Elm_Dayselector_Item_Data *it;

   ELM_DAYSELECTOR_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->items, l, it)
     if (day == it->day) return it;

   return NULL;
}

static Eina_Bool
_elm_dayselector_content_set(Eo *obj, Elm_Dayselector_Data *sd, const char *item, Evas_Object *content)
{
   Eina_Bool int_ret = EINA_FALSE;

   int day;
   char buf[1024];
   Elm_Dayselector_Item_Data *it = NULL;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(content, EFL_UI_NSTATE_CLASS), EINA_FALSE);
   if (!item) return EINA_FALSE;

   day = atoi(item + (strlen(item) - 1));
   if (day < 0 || day > ELM_DAYSELECTOR_MAX) return EINA_FALSE;

   it = _item_find(obj, day);
   if (it)
     {
        snprintf(buf, sizeof(buf), "elm.swallow.day%d", _item_location_get(sd, it));

        int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), buf), content);
        if (!int_ret)
          {
             /* XXX kept for legacy compatibility, remove eventually */
             snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));

             int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), buf), content);
             /* XXX */
          }
        if (!int_ret) return EINA_FALSE;

        if (!content) return EINA_TRUE; /* item deletion already handled */

        evas_object_del(VIEW(it));
        VIEW_SET(it, content);
     }
   else
     {
        Eo *eo_it = efl_add(ELM_DAYSELECTOR_ITEM_CLASS, obj);
        it = efl_data_scope_get(eo_it, ELM_DAYSELECTOR_ITEM_CLASS);
        it->day = day;

        snprintf(buf, sizeof(buf), "elm.swallow.day%d", _item_location_get(sd, it));

        int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), buf), content);
        if (!int_ret)
          {
             /* XXX kept for legacy compatibility, remove eventually */
             snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));

             int_ret = efl_content_set(efl_part(efl_super(obj, MY_CLASS), buf), content);
             /* XXX */
          }
        if (!int_ret)
          {
             efl_del(eo_it);
             return EINA_FALSE;
          }

        sd->items = eina_list_append(sd->items, it);
        VIEW_SET(it, content);
     }

   snprintf(buf, sizeof(buf), "elm,day%d,visible", _item_location_get(sd, it));
   elm_layout_signal_emit(obj, buf, "elm");
   /* XXX kept for legacy compatibility, remove eventually */
   snprintf(buf, sizeof(buf), "day%d,visible", _item_location_get(sd, it));
   elm_layout_signal_emit(obj, buf, "elm");

   efl_event_callback_add
     (VIEW(it), EFL_UI_CHECK_EVENT_CHANGED, _item_clicked_cb, it);
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

   return EINA_TRUE;
}

EOLIAN static Eo *
_elm_dayselector_item_efl_object_constructor(Eo *eo_item, Elm_Dayselector_Item_Data *item)
{
   eo_item = efl_constructor(efl_super(eo_item, ELM_DAYSELECTOR_ITEM_CLASS));
   item->base = efl_data_scope_get(eo_item, ELM_WIDGET_ITEM_CLASS);

   return eo_item;
}

static Evas_Object*
_elm_dayselector_content_unset(Eo *obj, Elm_Dayselector_Data *sd, const char *item)
{
   int day;
   char buf[1024];
   Evas_Object *content;
   Elm_Dayselector_Item_Data *it = NULL;

   day = atoi(item + (strlen(item) - 1));
   if (day < 0 || day > ELM_DAYSELECTOR_MAX) return NULL;

   it = _item_find(obj, day);
   if (!it) return NULL;

   content = efl_content_unset(efl_part(efl_super(obj, MY_CLASS), buf));
   if (!content) return NULL;

   sd->items = eina_list_remove(sd->items, it);
   efl_event_callback_del(content, EFL_UI_CHECK_EVENT_CHANGED, _item_clicked_cb, it);
   evas_object_event_callback_del(content, EVAS_CALLBACK_DEL, _item_del_cb);

   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKDAY_DEFAULT, "*", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKDAY_STYLE1, "*", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKEND_DEFAULT, "*", _item_signal_emit_cb);
   elm_object_signal_callback_del
     (content, ITEM_TYPE_WEEKEND_STYLE1, "*", _item_signal_emit_cb);

   snprintf(buf, sizeof(buf), "elm,day%d,default", _item_location_get(sd, it));
   elm_layout_signal_emit(obj, buf, "elm");
   /* XXX kept for legacy compatibility, remove eventually */
   snprintf(buf, sizeof(buf), "day%d,default", _item_location_get(sd, it));
   elm_layout_signal_emit(obj, buf, "elm");

   efl_del(EO_OBJ(it));

   elm_layout_sizing_eval(obj);

   return content;
}

static void
_items_style_set(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Dayselector_Item_Data *it;
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

        snprintf(buf, sizeof(buf), "elm.swallow.day%u", idx);
        if (!elm_layout_content_set(obj, buf, chk))
          {
             snprintf(buf, sizeof(buf), "day%u", idx);
             elm_layout_content_set(obj, buf, chk);
          }

        // XXX: ACCESS
        _elm_access_text_set(_elm_access_info_get(chk),
                 ELM_ACCESS_TYPE, E_("day selector item"));
     }

   _items_style_set(obj);
   _update_items(obj);
}

EOLIAN static void
_elm_dayselector_efl_canvas_group_group_add(Eo *obj, Elm_Dayselector_Data *priv)
{
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
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

EOLIAN static void
_elm_dayselector_efl_canvas_group_group_del(Eo *obj, Elm_Dayselector_Data *sd)
{
   Elm_Dayselector_Item_Data *it;

   EINA_LIST_FREE(sd->items, it)
     {
        sd->items = eina_list_remove(sd->items, it);
        eina_stringshare_del(it->day_style);
        // No need to efl_del the object as they have been created by efl_add and are dead by now.
     }

   /* handles freeing sd */
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EAPI Evas_Object *
elm_dayselector_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_dayselector_efl_object_constructor(Eo *obj, Elm_Dayselector_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_PANEL);

   return obj;
}

EOLIAN static void
_elm_dayselector_day_selected_set(Eo *obj, Elm_Dayselector_Data *_pd EINA_UNUSED, Elm_Dayselector_Day day, Eina_Bool selected)
{
   Elm_Dayselector_Item_Data *it = _item_find(obj, day);
   if (!it)
     {
        ERR("Failed to find item");
        return;
     }
   elm_check_state_set(VIEW(it), selected);
}

EOLIAN static Eina_Bool
_elm_dayselector_day_selected_get(const Eo *obj, Elm_Dayselector_Data *_pd EINA_UNUSED, Elm_Dayselector_Day day)
{
   Elm_Dayselector_Item_Data *it = _item_find(obj, day);
   if (!it)
     {
        ERR("Failed to find item");
        return EINA_FALSE;
     }
   return elm_check_state_get(VIEW(it));
}

EOLIAN static void
_elm_dayselector_week_start_set(Eo *obj, Elm_Dayselector_Data *sd, Elm_Dayselector_Day day)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Eina_List *l;
   char buf[1024];
   Elm_Dayselector_Item_Data *it;

   /* just shuffling items, so swalling them directly */
   sd->week_start = day;
   EINA_LIST_FOREACH(sd->items, l, it)
     {
        snprintf(buf, sizeof(buf), "elm.swallow.day%d", _item_location_get(sd, it));
        if (!edje_object_part_swallow(wd->resize_obj, buf, VIEW(it)))
          {
             snprintf(buf, sizeof(buf), "day%d", _item_location_get(sd, it));
             edje_object_part_swallow(wd->resize_obj, buf, VIEW(it));
          }
     }

   _update_items(obj);
}

EOLIAN static Elm_Dayselector_Day
_elm_dayselector_week_start_get(const Eo *obj EINA_UNUSED, Elm_Dayselector_Data *sd)
{
   return sd->week_start;
}

EOLIAN static void
_elm_dayselector_weekend_start_set(Eo *obj, Elm_Dayselector_Data *sd, Elm_Dayselector_Day day)
{
   sd->weekend_start = day;

   _items_style_set(obj);
   _update_items(obj);
}

EOLIAN static Elm_Dayselector_Day
_elm_dayselector_weekend_start_get(const Eo *obj EINA_UNUSED, Elm_Dayselector_Data *sd)
{
   return sd->weekend_start;
}

EOLIAN static void
_elm_dayselector_weekend_length_set(Eo *obj, Elm_Dayselector_Data *sd, unsigned int length)
{
   sd->weekend_len = length;

   _items_style_set(obj);
   _update_items(obj);
}

EOLIAN static unsigned int
_elm_dayselector_weekend_length_get(const Eo *obj EINA_UNUSED, Elm_Dayselector_Data *sd)
{
   return sd->weekend_len;
}

EOLIAN static void
_elm_dayselector_weekdays_names_set(Eo *obj, Elm_Dayselector_Data *sd, const char **weekdays)
{
   int idx;
   time_t now;
   struct tm time_daysel;
   Elm_Dayselector_Item_Data *it;
   char buf[1024];

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
        if (!it) continue;

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

EOLIAN static Eina_List *
_elm_dayselector_weekdays_names_get(const Eo *obj, Elm_Dayselector_Data *sd EINA_UNUSED)
{
   int idx;
   const char *weekday;
   Elm_Dayselector_Item_Data *it;
   Eina_List *weekdays = NULL;

   for (idx = 0; idx < ELM_DAYSELECTOR_MAX; idx++)
     {
        it = _item_find(obj, idx);
        if (!it) continue ;

        weekday = elm_object_text_get(VIEW(it));
        weekdays = eina_list_append(weekdays, eina_stringshare_add(weekday));
     }
   return weekdays;
}

static void
_elm_dayselector_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_dayselector, ELM_DAYSELECTOR, Elm_Dayselector_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_dayselector, ELM_DAYSELECTOR, Elm_Dayselector_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_dayselector, ELM_DAYSELECTOR, Elm_Dayselector_Data)
#include "elm_dayselector_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define ELM_DAYSELECTOR_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_dayselector), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_dayselector)

#include "elm_dayselector_eo.c"
#include "elm_dayselector_item_eo.c"

