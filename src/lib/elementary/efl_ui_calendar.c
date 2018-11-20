#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_FOCUS_COMPOSITION_PROTECTED
#define EFL_UI_FOCUS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_calendar_private.h"
#include "efl_ui_calendar_item.eo.h"

#define MY_CLASS EFL_UI_CALENDAR_CLASS

#define MY_CLASS_NAME "Efl.Ui.Calendar"
#define MY_CLASS_PFX efl_ui_calendar

#define EFL_UI_CALENDAR_BUTTON_LEFT "efl.calendar.button.left"
#define EFL_UI_CALENDAR_BUTTON_RIGHT "efl.calendar.button.right"
#define EFL_UI_CALENDAR_BUTTON_YEAR_LEFT "efl.calendar.button_year.left"
#define EFL_UI_CALENDAR_BUTTON_YEAR_RIGHT "efl.calendar.button_year.right"

#define FIRST_INTERVAL 0.85
#define INTERVAL 0.2

static const char PART_NAME_DEC_BUTTON[] = "dec_button";
static const char PART_NAME_INC_BUTTON[] = "inc_button";

static const char SIG_CHANGED[] = "changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void
_inc_dec_btn_clicked_cb(void *data,
                        const Efl_Event *ev);

static void
_inc_dec_btn_repeated_cb(void *data,
                         const Efl_Event *ev);

static Eina_Bool _key_action_activate(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"activate", _key_action_activate},
   {NULL, NULL}
};

/* Should not be translated, it's used if we failed
 * getting from locale. */
static const char *_days_abbrev[] =
{
   "Sun", "Mon", "Tue", "Wed",
   "Thu", "Fri", "Sat"
};

static int _days_in_month[2][12] =
{
   {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
   {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
};

static Eina_Bool _efl_ui_calendar_smart_focus_next_enable = EINA_FALSE;

EOLIAN static void
_efl_ui_calendar_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Calendar_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   EFL_UI_CALENDAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->filling) return;
   // 7x8 (1 month+year, days, 6 dates.)
   elm_coords_finger_size_adjust(7, &minw, 8, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

// Get the max day number for each month
static inline int
_maxdays_get(struct tm *date, int month_offset)
{
   int month, year;

   month = (date->tm_mon + month_offset) % 12;
   year = date->tm_year + 1900;

   if (month < 0) month += 12;

   return _days_in_month
          [((!(year % 4)) && ((!(year % 400)) || (year % 100)))][month];
}

static inline void
_unselect(Evas_Object *obj,
          int selected)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%d,unselected", selected);
   elm_layout_signal_emit(obj, emission, "efl");
}

static inline void
_select(Evas_Object *obj,
        int selected)
{
   char emission[32];

   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   sd->focused_it = sd->selected_it = selected;
   snprintf(emission, sizeof(emission), "cit_%d,selected", selected);
   elm_layout_signal_emit(obj, emission, "efl");
}

static inline void
_not_today(Efl_Ui_Calendar_Data *sd)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%d,not_today", sd->today_it);
   elm_layout_signal_emit(sd->obj, emission, "efl");
   sd->today_it = -1;
}

static inline void
_today(Efl_Ui_Calendar_Data *sd,
       int it)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%d,today", it);
   elm_layout_signal_emit(sd->obj, emission, "efl");
   sd->today_it = it;
}

static inline void
_enable(Efl_Ui_Calendar_Data *sd,
        int it)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%d,enable", it);
   elm_layout_signal_emit(sd->obj, emission, "efl");
}

static inline void
_disable(Efl_Ui_Calendar_Data *sd,
         int it)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%d,disable", it);
   elm_layout_signal_emit(sd->obj, emission, "efl");
}

static void
_set_month_year(Efl_Ui_Calendar_Data *sd)
{

   sd->filling = EINA_TRUE;

   if (sd->format_cb)
     {
        Eina_Value val;
        const char *buf;

		eina_value_setup(&val, EINA_VALUE_TYPE_TM);
        eina_value_set(&val, sd->shown_date);
        eina_strbuf_reset(sd->format_strbuf);
        sd->format_cb(sd->format_cb_data, sd->format_strbuf, val);
        buf = eina_strbuf_string_get(sd->format_strbuf);
		eina_value_flush(&val);

        if (buf)
          elm_layout_text_set(sd->obj, "month_text", buf);
        else
          elm_layout_text_set(sd->obj, "month_text", "");
     }
   else
     {
        char *buf;
        buf = eina_strftime(E_("%B %Y"), &sd->shown_date);
        if (buf)
          {
             elm_layout_text_set(sd->obj, "month_text", buf);
             free(buf);
          }
        else elm_layout_text_set(sd->obj, "month_text", "");
     }

   sd->filling = EINA_FALSE;
}

static char *
_access_info_cb(void *data EINA_UNUSED, Evas_Object *obj)
{
   char *ret;
   Eina_Strbuf *buf;
   buf = eina_strbuf_new();

   eina_strbuf_append_printf(buf, "day %s", elm_widget_access_info_get(obj));

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_access_calendar_item_register(Evas_Object *obj)
{
   unsigned int maxdays, i;
   char day_s[13], pname[18];
   unsigned day = 0;
   Evas_Object *ao;

   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   maxdays = _maxdays_get(&sd->shown_date, 0);
   for (i = 0; i < 42; i++)
     {
        if ((!day) && (i == sd->first_day_it)) day = 1;
        if ((day) && (day <= maxdays))
          {
             snprintf(pname, sizeof(pname), "efl.cit_%d.access", i);

             ao = _elm_access_edje_object_part_object_register
                        (obj, elm_layout_edje_get(obj), pname);
             _elm_access_text_set(_elm_access_info_get(ao),
                         ELM_ACCESS_TYPE, E_("calendar item"));
             _elm_access_callback_set(_elm_access_info_get(ao),
                           ELM_ACCESS_INFO, _access_info_cb, NULL);

             snprintf(day_s, sizeof(day_s), "%d", (int) (day++));
             elm_widget_access_info_set(ao, (const char*)day_s);
          }
        else
          {
             snprintf(pname, sizeof(pname), "efl.cit_%d.access", i);
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), pname);
          }
     }
}

static void
_access_calendar_spinner_register(Evas_Object *obj)
{
   Evas_Object *po, *o;
   Elm_Access_Info *ai;
   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   if (!sd->dec_btn_month)
     sd->dec_btn_month = _elm_access_edje_object_part_object_register
        (obj, elm_layout_edje_get(obj), "left_bt");
   ai = _elm_access_info_get(sd->dec_btn_month);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar decrement month button"));

   if (!sd->inc_btn_month)
     sd->inc_btn_month = _elm_access_edje_object_part_object_register
        (obj, elm_layout_edje_get(obj), "right_bt");
   ai = _elm_access_info_get(sd->inc_btn_month);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar increment month button"));

   sd->month_access = _elm_access_edje_object_part_object_register
                          (obj, elm_layout_edje_get(obj), "text_month");
   ai = _elm_access_info_get(sd->month_access);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar month"));

   o = elm_layout_edje_get(obj);
   edje_object_freeze(o);
   po = (Evas_Object *)edje_object_part_object_get(o, "month_text");
   edje_object_thaw(o);
   evas_object_pass_events_set(po, EINA_FALSE);
}

static void
_access_calendar_register(Evas_Object *obj)
{
   _access_calendar_spinner_register(obj);
   _access_calendar_item_register(obj);
}

static void
_flush_calendar_composite_elements(Evas_Object *obj, Efl_Ui_Calendar_Data *sd)
{
   Eina_List *items = NULL;
   int max_day = _maxdays_get(&sd->shown_date, 0);

#define EXTEND(v) \
    if (v) items = eina_list_append(items, v); \

    EXTEND(sd->month_access);
    EXTEND(sd->dec_btn_month);
    EXTEND(sd->inc_btn_month);

#undef EXTEND

   for (int i = sd->first_day_it; i <= max_day; ++i)
     items = eina_list_append(items, sd->items[i]);

   efl_ui_focus_composition_elements_set(obj, items);
}

static void
_populate(Evas_Object *obj)
{
   int maxdays, prev_month_maxdays, day, mon, yr, i;
   char part[16], day_s[16];
   struct tm first_day;

   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   elm_layout_freeze(obj);

   sd->filling = EINA_FALSE;
   if (sd->today_it > 0) _not_today(sd);

   maxdays = _maxdays_get(&sd->shown_date, 0);
   prev_month_maxdays = _maxdays_get(&sd->shown_date, -1);
   mon = sd->shown_date.tm_mon;
   yr = sd->shown_date.tm_year;

   _set_month_year(sd);
   sd->filling = EINA_TRUE;

   /* Set days */
   day = 0;
   first_day = sd->shown_date;
   first_day.tm_mday = 1;
   if (mktime(&first_day) == -1)
     {
        ERR("mktime can not give week day for this month properly. Please check year or month is proper.");
        return;
     }

   // Layout of the calendar is changed for removing the unfilled last row.
   if (first_day.tm_wday < (int)sd->first_week_day)
     sd->first_day_it = first_day.tm_wday + ELM_DAY_LAST - sd->first_week_day;
   else
     sd->first_day_it = first_day.tm_wday - sd->first_week_day;

   for (i = 0; i < 42; i++)
     {
        if ((!day) && (i == sd->first_day_it)) day = 1;

        if ((day == sd->current_date.tm_mday)
            && (mon == sd->current_date.tm_mon)
            && (yr == sd->current_date.tm_year))
          _today(sd, i);

        if (day == sd->date.tm_mday)
          {
             if ((sd->selected_it > -1) && (sd->selected_it != i))
               _unselect(obj, sd->selected_it);

             if ((mon == sd->date.tm_mon) && (yr == sd->date.tm_year))
               _select(obj, i);
          }

        if ((day) && (day <= maxdays))
          {
             if (((yr == sd->date_min.tm_year) && (mon == sd->date_min.tm_mon) && (day < sd->date_min.tm_mday))
                 || ((yr == sd->date_max.tm_year) && (mon == sd->date_max.tm_mon) && (day > sd->date_max.tm_mday)))
               _disable(sd, i);
             else
               _enable(sd, i);

             snprintf(day_s, sizeof(day_s), "%d", day++);
          }
        else
          {
             _disable(sd, i);

             if (day <= maxdays)
               snprintf(day_s, sizeof(day_s), "%d", prev_month_maxdays - sd->first_day_it + i + 1);
             else
               snprintf(day_s, sizeof(day_s), "%d", i - sd->first_day_it - maxdays + 1);
          }

        snprintf(part, sizeof(part), "efl.cit_%d.text", i);
        elm_layout_text_set(obj, part, day_s);
     }

   // ACCESS
   if ((_elm_config->access_mode != ELM_ACCESS_MODE_OFF))
     _access_calendar_item_register(obj);

   sd->filling = EINA_FALSE;

   elm_layout_thaw(obj);
   edje_object_message_signal_process(elm_layout_edje_get(obj));

   _flush_calendar_composite_elements(obj, sd);
}

static void
_set_headers(Evas_Object *obj)
{
   static char part[] = "efl.ch_0.text";
   int i;
   struct tm *t;
   time_t temp = 259200; // the first sunday since epoch
   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   elm_layout_freeze(obj);

   sd->filling = EINA_TRUE;

   t = gmtime(&temp);
   if (t)
     {
        t->tm_wday = 0;
        for (i = 0; i < ELM_DAY_LAST; i++)
          {
             char *buf;
             buf = eina_strftime("%a", t);
             if (buf)
               {
                  sd->weekdays[i] = eina_stringshare_add(buf);
                  free(buf);
               }
             else
               {
                  /* If we failed getting day, get a default value */
                  sd->weekdays[i] = _days_abbrev[i];
                  WRN("Failed getting weekday name for '%s' from locale.",
                      _days_abbrev[i]);
               }
             t->tm_wday++;
          }
     }

   for (i = 0; i < ELM_DAY_LAST; i++)
     {
        part[7] = i + '0';
        elm_layout_text_set(obj, part, sd->weekdays[(i + sd->first_week_day) % ELM_DAY_LAST]);
     }

   sd->filling = EINA_FALSE;

   elm_layout_thaw(obj);
}

static Eo *
_btn_create(Eo *obj, const char *style, char *part)
{
   return efl_add(EFL_UI_BUTTON_CLASS, obj,
                  elm_widget_element_update(obj, efl_added, style),
                  efl_ui_autorepeat_enabled_set(efl_added, EINA_TRUE),
                  efl_ui_autorepeat_initial_timeout_set(efl_added, FIRST_INTERVAL),
                  efl_ui_autorepeat_gap_timeout_set(efl_added, INTERVAL),
                  efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED,
                                         _inc_dec_btn_clicked_cb, obj),
                  efl_event_callback_add(efl_added, EFL_UI_EVENT_REPEATED,
                                         _inc_dec_btn_repeated_cb, obj),
                  efl_content_set(efl_part(obj, part), efl_added));
}

static void
_spinner_buttons_add(Evas_Object *obj, Efl_Ui_Calendar_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (edje_object_part_exists(wd->resize_obj, EFL_UI_CALENDAR_BUTTON_LEFT))
     {
        if (sd->dec_btn_month && efl_isa(sd->dec_btn_month, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "left_bt");
             sd->dec_btn_month = NULL;
          }

        if (!sd->dec_btn_month)
          sd->dec_btn_month = _btn_create(obj, PART_NAME_DEC_BUTTON, EFL_UI_CALENDAR_BUTTON_LEFT);
     }

   else if (sd->dec_btn_month && !efl_isa(sd->dec_btn_month, ELM_ACCESS_CLASS))
     {
        evas_object_del(sd->dec_btn_month);
        sd->dec_btn_month = NULL;
     }

   if (edje_object_part_exists(wd->resize_obj, EFL_UI_CALENDAR_BUTTON_RIGHT))
     {
        if (sd->inc_btn_month && efl_isa(sd->inc_btn_month, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "right_bt");
             sd->inc_btn_month = NULL;
          }

        if (!sd->inc_btn_month)
             sd->inc_btn_month = _btn_create(obj, PART_NAME_INC_BUTTON, EFL_UI_CALENDAR_BUTTON_RIGHT);
     }
   else if (sd->inc_btn_month && !efl_isa(sd->inc_btn_month, ELM_ACCESS_CLASS))
     {
        evas_object_del(sd->inc_btn_month);
        sd->inc_btn_month = NULL;
     }
}

EOLIAN static Efl_Ui_Theme_Apply_Result
_efl_ui_calendar_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Calendar_Data *sd)
{
   Efl_Ui_Theme_Apply_Result int_ret = EFL_UI_THEME_APPLY_RESULT_FAIL;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_RESULT_FAIL;

   _spinner_buttons_add(obj, sd);

   evas_object_smart_changed(obj);
   return int_ret;
}

/* Set correct tm_wday and tm_yday after other fields changes*/
static inline Eina_Bool
_fix_date(Efl_Ui_Calendar_Data *sd)
{
   Eina_Bool no_change = EINA_TRUE;

   if ((sd->date.tm_year < sd->date_min.tm_year) ||
       ((sd->date.tm_year == sd->date_min.tm_year) &&
        (sd->date.tm_mon < sd->date_min.tm_mon)) ||
       ((sd->date.tm_year == sd->date_min.tm_year) &&
        (sd->date.tm_mon == sd->date_min.tm_mon) &&
        (sd->date.tm_mday < sd->date_min.tm_mday)))
     {
        sd->date.tm_year = sd->shown_date.tm_year = sd->date_min.tm_year;
        sd->date.tm_mon = sd->shown_date.tm_mon = sd->date_min.tm_mon;
        sd->date.tm_mday = sd->shown_date.tm_mday = sd->date_min.tm_mday;
        no_change = EINA_FALSE;
     }
   else if ((sd->date_max.tm_year != -1) &&
            ((sd->date.tm_year > sd->date_max.tm_year) ||
            ((sd->date.tm_year == sd->date_max.tm_year) &&
             (sd->date.tm_mon > sd->date_max.tm_mon)) ||
            ((sd->date.tm_year == sd->date_max.tm_year) &&
             (sd->date.tm_mon == sd->date_max.tm_mon) &&
             (sd->date.tm_mday > sd->date_max.tm_mday))))
     {
        sd->date.tm_year = sd->shown_date.tm_year = sd->date_max.tm_year;
        sd->date.tm_mon = sd->shown_date.tm_mon = sd->date_max.tm_mon;
        sd->date.tm_mday = sd->shown_date.tm_mday = sd->date_max.tm_mday;
        no_change = EINA_FALSE;
     }
   else
     {
        if (sd->date.tm_mon != sd->shown_date.tm_mon)
          sd->date.tm_mon = sd->shown_date.tm_mon;
        if (sd->date.tm_year != sd->shown_date.tm_year)
          sd->date.tm_year = sd->shown_date.tm_year;
     }

   return no_change;
}

static Eina_Bool
_update_data(Evas_Object *obj, int delta)
{
   struct tm time_check;
   int maxdays;

   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   /* check if it's a valid time. for 32 bits, year greater than 2037 is not */
   time_check = sd->shown_date;
   time_check.tm_mon += delta;

   if (mktime(&time_check) == -1)
     {
        ERR("mktime can not give week day for the next month. Please check what is wrong with udpate date.");
        return EINA_FALSE;
     }

   sd->shown_date.tm_mon += delta;

   if (delta < 0)
     {
        if (sd->shown_date.tm_year == sd->date_min.tm_year)
          {
             if (sd->shown_date.tm_mon < sd->date_min.tm_mon)
               {
                  sd->shown_date.tm_mon = sd->date_min.tm_mon;
                  return EINA_FALSE;
               }
          }
        else if (sd->shown_date.tm_mon < 0)
          {
             sd->shown_date.tm_mon = 11;
             sd->shown_date.tm_year--;
          }
     }
   else
     {
        if (sd->shown_date.tm_year == sd->date_max.tm_year)
          {
             if (sd->shown_date.tm_mon > sd->date_max.tm_mon)
               {
                  sd->shown_date.tm_mon = sd->date_max.tm_mon;
                  return EINA_FALSE;
               }
          }
        else if (sd->shown_date.tm_mon > 11)
          {
             sd->shown_date.tm_mon = 0;
             sd->shown_date.tm_year++;
          }
     }

   maxdays = _maxdays_get(&sd->shown_date, 0);
   if (sd->date.tm_mday > maxdays)
     sd->date.tm_mday = maxdays;

   return EINA_TRUE;
}

static void
_spin_value(void *data)
{
   EFL_UI_CALENDAR_DATA_GET(data, sd);

   if (_update_data(data, sd->spin_speed))
     evas_object_smart_changed(data);
}

static void
_inc_dec_btn_clicked_cb(void *data,
                        const Efl_Event *ev)
{
   EFL_UI_CALENDAR_DATA_GET(data, sd);

   sd->spin_speed = (ev->object == sd->inc_btn_month) ? 1 : -1;

   _spin_value(data);
}

static void
_inc_dec_btn_repeated_cb(void *data,
                         const Efl_Event *ev)
{
   EFL_UI_CALENDAR_DATA_GET(data, sd);

   sd->spin_speed = (ev->object == sd->inc_btn_month) ? 1 : -1;

   _spin_value(data);
}

static int
_get_item_day(Evas_Object *obj,
              int selected_it)
{
   int day;

   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   day = selected_it - sd->first_day_it + 1;
   if ((day < 0) || (day > _maxdays_get(&sd->shown_date, 0)))
     return 0;

   if ((sd->shown_date.tm_year == sd->date_min.tm_year)
       && (sd->shown_date.tm_mon == sd->date_min.tm_mon)
       && (day < sd->date_min.tm_mday))
     {
        return 0;
     }
   else if ((sd->shown_date.tm_year == sd->date_max.tm_year)
            && (sd->shown_date.tm_mon == sd->date_max.tm_mon)
            && (day > sd->date_max.tm_mday))
     {
        return 0;
     }

   return day;
}

static void
_update_unfocused_it(Evas_Object *obj, int unfocused_it)
{
   int day;
   char emission[32];

   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   day = _get_item_day(obj, unfocused_it);
   if (!day)
     return;

   sd->focused_it = -1;

   snprintf(emission, sizeof(emission), "cit_%d,unfocused", unfocused_it);
   elm_layout_signal_emit(obj, emission, "efl");
}

static Eina_Bool
_update_focused_it(Evas_Object *obj, int focused_it)
{
   int day;
   char emission[32];

   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   day = _get_item_day(obj, focused_it);
   if (!day)
     return EINA_FALSE;

   snprintf(emission, sizeof(emission), "cit_%d,unfocused", sd->focused_it);
   elm_layout_signal_emit(obj, emission, "efl");

   sd->focused_it = focused_it;

   snprintf(emission, sizeof(emission), "cit_%d,focused", sd->focused_it);
   elm_layout_signal_emit(obj, emission, "efl");

   return EINA_TRUE;
}

static void
_update_sel_it(Evas_Object *obj,
               int sel_it)
{
   int day;

   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   day = _get_item_day(obj, sel_it);
   if (!day)
     return;

   _unselect(obj, sd->selected_it);
   if (!sd->selected)
     sd->selected = EINA_TRUE;
   if (sd->focused_it)
     _update_unfocused_it(obj, sd->focused_it);

   sd->date.tm_mday = day;
   _fix_date(sd);
   _select(obj, sel_it);
   efl_event_callback_legacy_call(obj, EFL_UI_CALENDAR_EVENT_CHANGED, NULL);
}

static void
_day_selected(void *data,
              Evas_Object *obj EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source)
{
   int sel_it;

   sel_it = atoi(source);

   _update_sel_it(data, sel_it);
}

static inline int
_time_to_next_day(struct tm *t)
{
   return ((((24 - t->tm_hour) * 60) - t->tm_min) * 60) - t->tm_sec;
}

static Eina_Bool
_update_cur_date(void *data)
{
   time_t current_date;
   int t, day;
   EFL_UI_CALENDAR_DATA_GET(data, sd);

   if (sd->today_it > 0) _not_today(sd);

   current_date = time(NULL);
   localtime_r(&current_date, &sd->current_date);
   t = _time_to_next_day(&sd->current_date);
   ecore_timer_interval_set(sd->update_timer, t);

   if ((sd->current_date.tm_mon != sd->shown_date.tm_mon) ||
       (sd->current_date.tm_year != sd->shown_date.tm_year))
     return ECORE_CALLBACK_RENEW;

   day = sd->current_date.tm_mday + sd->first_day_it - 1;
   _today(sd, day);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   EFL_UI_CALENDAR_DATA_GET(obj, sd);

   _update_sel_it(obj, sd->focused_it);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_calendar_efl_ui_focus_object_on_focus_update(Eo *obj, Efl_Ui_Calendar_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   // FIXME : Currently, focused item is same with selected item.
   //         After arranging focus logic in this widget, we need to make
   //         focused item which is for indicating direction key input movement
   //         on the calendar widget.
   if (efl_ui_focus_object_focus_get(obj))
     _update_focused_it(obj, sd->selected_it);
   else
     _update_unfocused_it(obj, sd->focused_it);

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_calendar_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Calendar_Data *_pd EINA_UNUSED)
{
   elm_layout_freeze(obj);

   _set_headers(obj);
   _populate(obj);

   elm_layout_thaw(obj);
}

EOLIAN static void
_efl_ui_calendar_efl_object_destructor(Eo *obj, Efl_Ui_Calendar_Data *sd)
{
   int i;

   ecore_timer_del(sd->update_timer);

   efl_ui_format_cb_set(obj, NULL, NULL, NULL);
   eina_strbuf_free(sd->format_strbuf);

   for (i = 0; i < ELM_DAY_LAST; i++)
     eina_stringshare_del(sd->weekdays[i]);

   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   int maxdays, day, i;

   EFL_UI_CALENDAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (is_access)
     _access_calendar_register(obj);
   else
     {
        day = 0;
        maxdays = _maxdays_get(&sd->shown_date, 0);
        for (i = 0; i < 42; i++)
          {
             if ((!day) && (i == sd->first_day_it)) day = 1;
             if ((day) && (day <= maxdays))
               {
                  char pname[18];
                  snprintf(pname, sizeof(pname), "efl.cit_%d.access", i);

                  _elm_access_edje_object_part_object_unregister
                    (obj, elm_layout_edje_get(obj), pname);
               }
          }

        if (sd->dec_btn_month && efl_isa(sd->dec_btn_month, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "left_bt");
             sd->dec_btn_month = NULL;
          }
        if (sd->inc_btn_month && efl_isa(sd->inc_btn_month, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "right_bt");
             sd->inc_btn_month = NULL;
          }
        if (sd->month_access)
          _elm_access_edje_object_part_object_unregister
            (obj, elm_layout_edje_get(obj), "month_text");
     }
}

EOLIAN static void
_efl_ui_calendar_efl_ui_widget_on_access_update(Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Data *_pd EINA_UNUSED, Eina_Bool acs)
{
   _efl_ui_calendar_smart_focus_next_enable = acs;
   _access_obj_process(obj, _efl_ui_calendar_smart_focus_next_enable);
}

static Eo *
_efl_ui_calendar_constructor_internal(Eo *obj, Efl_Ui_Calendar_Data *priv)
{
   time_t current_date;
   int t;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   elm_widget_sub_object_parent_add(obj);

   priv->date_min.tm_year = 2;
   priv->date_min.tm_mon = 0;
   priv->date_min.tm_mday = 1;
   priv->date_max.tm_year = -1;
   priv->date_max.tm_mon = 11;
   priv->date_max.tm_mday = 31;
   priv->today_it = -1;
   priv->selected_it = -1;
   priv->first_day_it = -1;
   priv->format_cb = NULL;

   edje_object_signal_callback_add
     (wd->resize_obj, "efl,action,selected", "*",
     _day_selected, obj);

   current_date = time(NULL);
   localtime_r(&current_date, &priv->shown_date);
   priv->current_date = priv->shown_date;
   priv->date = priv->shown_date;
   t = _time_to_next_day(&priv->current_date);
   priv->update_timer = ecore_timer_add(t, _update_cur_date, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "calendar");
   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   evas_object_smart_changed(obj);

   // ACCESS
   if ((_elm_config->access_mode != ELM_ACCESS_MODE_OFF))
      _access_calendar_spinner_register(obj);

   // Items for composition
   for (int i = 0; i < 42; ++i)
     {
        priv->items[i] = efl_add(EFL_UI_CALENDAR_ITEM_CLASS, obj,
                                 efl_ui_calendar_item_day_number_set(efl_added, i));
     }

   return obj;
}

EOLIAN static Eo *
_efl_ui_calendar_efl_object_constructor(Eo *obj, Efl_Ui_Calendar_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   sd->obj = obj;
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_DATE_EDITOR);

   obj = _efl_ui_calendar_constructor_internal(obj, sd);

   return obj;
}

EOLIAN static Eina_Bool
_efl_ui_calendar_date_min_set(Eo *obj, Efl_Ui_Calendar_Data *sd, Efl_Time min)
{
   Eina_Bool upper = EINA_FALSE;
   struct tm temp;

   temp = min;
   if (mktime(&temp) == -1)
     {
        ERR("mktime can not give week day for your minimum date. Please check the date.");
        return EINA_FALSE;
     }

   if ((sd->date_min.tm_year == min.tm_year)
       && (sd->date_min.tm_mon == min.tm_mon)
       && (sd->date_min.tm_mday == min.tm_mday))
     return EINA_TRUE;

   if (min.tm_year < 2)
     {
        sd->date_min.tm_year = 2;
        sd->date_min.tm_mon = 0;
        sd->date_min.tm_mday = 1;
     }
   else
     {
        if (sd->date_max.tm_year != -1)
          {
             if (min.tm_year > sd->date_max.tm_year)
               {
                  upper = EINA_TRUE;
               }
             else if (min.tm_year == sd->date_max.tm_year)
               {
                  if (min.tm_mon > sd->date_max.tm_mon)
                    upper = EINA_TRUE;
                  else if ((min.tm_mon == sd->date_max.tm_mon) && (min.tm_mday > sd->date_max.tm_mday))
                    upper = EINA_TRUE;
               }
          }

        if (upper)
          {
             sd->date_min.tm_year = sd->date_max.tm_year;
             sd->date_min.tm_mon = sd->date_max.tm_mon;
             sd->date_min.tm_mday = sd->date_max.tm_mday;
          }
        else
          {
             sd->date_min.tm_year = min.tm_year;
             sd->date_min.tm_mon = min.tm_mon;
             sd->date_min.tm_mday = min.tm_mday;
          }
     }

   _fix_date(sd);

   evas_object_smart_changed(obj);

   if (upper)
     {
        ERR("Your minimum date is greater than current maximum date.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EOLIAN static Efl_Time
_efl_ui_calendar_date_min_get(const Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Data *sd)
{
   return sd->date_min;
}

EOLIAN static Eina_Bool
_efl_ui_calendar_date_max_set(Eo *obj, Efl_Ui_Calendar_Data *sd, Efl_Time max)
{
   Eina_Bool lower = EINA_FALSE;
   struct tm temp;

   temp = max;
   if (mktime(&temp) == -1)
     {
        ERR("mktime can not give week day for your maximum date. Please check the date.");
        return EINA_FALSE;
     }

   if ((sd->date_max.tm_year == max.tm_year)
       && (sd->date_max.tm_mon == max.tm_mon)
       && (sd->date_max.tm_mday == max.tm_mday))
     return EINA_TRUE;

   if (max.tm_year < sd->date_min.tm_year)
     {
        lower = EINA_TRUE;
     }
   else if (max.tm_year == sd->date_min.tm_year)
     {
        if (max.tm_mon < sd->date_min.tm_mon)
          lower = EINA_TRUE;
        else if ((max.tm_mon == sd->date_min.tm_mon) && (max.tm_mday < sd->date_min.tm_mday))
          lower = EINA_TRUE;
     }

   if (lower)
     {
        sd->date_max.tm_year = sd->date_min.tm_year;
        sd->date_max.tm_mon = sd->date_min.tm_mon;
        sd->date_max.tm_mday = sd->date_min.tm_mday;
     }
   else
     {
        sd->date_max.tm_year = max.tm_year;
        sd->date_max.tm_mon = max.tm_mon;
        sd->date_max.tm_mday = max.tm_mday;
     }

   _fix_date(sd);

   evas_object_smart_changed(obj);

   if (lower)
     {
        ERR("Your maximum date is less than current minimum date.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EOLIAN static Efl_Time
_efl_ui_calendar_date_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Data *sd)
{
   return sd->date_max;
}

EOLIAN static Eina_Bool
_efl_ui_calendar_date_set(Eo *obj, Efl_Ui_Calendar_Data *sd, Efl_Time date)
{
   Eina_Bool ret = EINA_TRUE;
   struct tm temp;

   temp = date;
   if (mktime(&temp) == -1)
     {
        ERR("mktime can not give week day for your new date. Please check the date.");
        return EINA_FALSE;
     }

   sd->date.tm_year = date.tm_year;
   sd->date.tm_mon = date.tm_mon;
   sd->date.tm_mday = date.tm_mday;
   if (!sd->selected)
     sd->selected = EINA_TRUE;

   if (sd->date.tm_year != sd->shown_date.tm_year)
     sd->shown_date.tm_year = sd->date.tm_year;
   if (sd->date.tm_mon != sd->shown_date.tm_mon)
     sd->shown_date.tm_mon = sd->date.tm_mon;

   ret = _fix_date(sd);

   evas_object_smart_changed(obj);

   if (!ret)
     ERR("The current date is greater than the maximum date or less than the minimum date.");

   return ret;
}

EOLIAN static Efl_Time
_efl_ui_calendar_date_get(const Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Data *sd)
{
   return sd->date;
}

EOLIAN static void
_efl_ui_calendar_efl_ui_format_format_cb_set(Eo *obj, Efl_Ui_Calendar_Data *sd, void *func_data, Efl_Ui_Format_Func_Cb func, Eina_Free_Cb func_free_cb)
{
   if ((sd->format_cb_data == func_data) && (sd->format_cb == func))
     return;

   if (sd->format_cb_data && sd->format_free_cb)
     sd->format_free_cb(sd->format_cb_data);

   sd->format_cb = func;
   sd->format_cb_data = func_data;
   sd->format_free_cb = func_free_cb;
   if (!sd->format_strbuf) sd->format_strbuf = eina_strbuf_new();

   evas_object_smart_changed(obj);
}

static void
_calendar_format_cb(void *data, Eina_Strbuf *str, const Eina_Value value)
{
   Efl_Ui_Calendar_Data *sd = data;
   const Eina_Value_Type *type = eina_value_type_get(&value);
   struct tm v;

   if (type == EINA_VALUE_TYPE_TM)
     {
        eina_value_get(&value, &v);
        eina_strbuf_append_strftime(str, sd->format_template, &v);
     }
}

static void
_calendar_format_free_cb(void *data)
{
   Efl_Ui_Calendar_Data *sd = data;

   if (sd && sd->format_template)
     {
        eina_stringshare_del(sd->format_template);
        sd->format_template = NULL;
     }
}

EOLIAN static void
_efl_ui_calendar_efl_ui_format_format_string_set(Eo *obj, Efl_Ui_Calendar_Data *sd, const char *template)
{
   if (!template) return;

   eina_stringshare_replace(&sd->format_template, template);

   efl_ui_format_cb_set(obj, sd, _calendar_format_cb, _calendar_format_free_cb);
}

EOLIAN static const char *
_efl_ui_calendar_efl_ui_format_format_string_get(const Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Data *sd)
{
   return sd->format_template;
}

EOLIAN static void
_efl_ui_calendar_first_day_of_week_set(Eo *obj, Efl_Ui_Calendar_Data *sd, Efl_Ui_Calendar_Weekday day)
{
   if (day >= EFL_UI_CALENDAR_WEEKDAY_LAST) return;
   if (sd->first_week_day != day)
     {
        sd->first_week_day = day;
        evas_object_smart_changed(obj);
     }
}

EOLIAN static Efl_Ui_Calendar_Weekday
_efl_ui_calendar_first_day_of_week_get(const Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Data *sd)
{
   return sd->first_week_day;
}

static void
_efl_ui_calendar_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);

   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
      _efl_ui_calendar_smart_focus_next_enable = EINA_TRUE;
}

EOLIAN static const Efl_Access_Action_Data*
_efl_ui_calendar_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Data *sd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "activate", "activate", NULL, _key_action_activate},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(efl_ui_calendar, Efl_Ui_Calendar_Data)

/* Internal EO APIs and hidden overrides */

#define EFL_UI_CALENDAR_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_calendar)

#include "efl_ui_calendar.eo.c"

typedef struct {
   int v;
   Evas_Object *part;
}  Efl_Ui_Calendar_Item_Data;

EOLIAN static void
_efl_ui_calendar_item_day_number_set(Eo *obj, Efl_Ui_Calendar_Item_Data *pd, int i)
{
   char pname[18];
   Evas_Object *po, *o;

   pd->v = i;
   snprintf(pname, sizeof(pname), "efl.cit_%i.access", i);

   o = elm_layout_edje_get(efl_parent_get(obj));
   edje_object_freeze(o);
   po = (Evas_Object *)edje_object_part_object_get(o, pname);
   edje_object_thaw(o);

   if (_elm_config->access_mode != ELM_ACCESS_MODE_ON)
     pd->part = po;
   else
     pd->part = evas_object_data_get(po, "_part_access_obj");

   EINA_SAFETY_ON_NULL_RETURN(pd->part);
}

EOLIAN static int
_efl_ui_calendar_item_day_number_get(const Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Item_Data *pd)
{
   return pd->v;
}

EOLIAN static void
_efl_ui_calendar_item_efl_ui_focus_object_focus_set(Eo *obj, Efl_Ui_Calendar_Item_Data *pd, Eina_Bool focus)
{
   efl_ui_focus_object_focus_set(efl_super(obj, EFL_UI_CALENDAR_ITEM_CLASS), focus);

   _update_focused_it(efl_parent_get(obj), pd->v);
   evas_object_focus_set(pd->part, efl_ui_focus_object_focus_get(obj));
}

EOLIAN static Eina_Rect
_efl_ui_calendar_item_efl_ui_focus_object_focus_geometry_get(const Eo *obj EINA_UNUSED, Efl_Ui_Calendar_Item_Data *pd)
{
   return efl_gfx_entity_geometry_get(pd->part);
}

#include "efl_ui_calendar_item.eo.c"
