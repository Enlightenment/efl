#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED
#define EFL_UI_FOCUS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_calendar.h"
#include "elm_calendar_eo.h"
#include "elm_calendar_item_eo.h"

#define MY_CLASS ELM_CALENDAR_CLASS

#define MY_CLASS_NAME "Elm_Calendar"
#define MY_CLASS_NAME_LEGACY "elm_calendar"

#define ELM_CALENDAR_BUTTON_LEFT "elm,calendar,button,left"
#define ELM_CALENDAR_BUTTON_RIGHT "elm,calendar,button,right"
#define ELM_CALENDAR_BUTTON_YEAR_LEFT "elm,calendar,button_year,left"
#define ELM_CALENDAR_BUTTON_YEAR_RIGHT "elm,calendar,button_year,right"

#define ELM_CALENDAR_CH_TEXT_PART_STR "elm.ch_%d.text"
#define ELM_CALENDAR_CIT_TEXT_PART_STR "elm.cit_%d.text"
#define ELM_CALENDAR_CIT_ACCESS_PART_STR "elm.cit_%d.access"

static void _part_name_snprintf(char *buffer, int buffer_size,
   const Evas_Object *obj, const char *template, int n)
{
   snprintf(buffer, buffer_size, template, n);
   if (!edje_object_part_exists (obj, buffer))
     {
        // Skip the namespace prefix "elm." which was not present
        // in previous versions
        snprintf(buffer, buffer_size, template + 4, n);
     }
}

static const char SIG_CHANGED[] = "changed";
static const char SIG_DISPLAY_CHANGED[] = "display,changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_DISPLAY_CHANGED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

/* Originally, the button functionalities of month, year spinners were
 * implemented by its own edc. There was a bunch of callback functions
 * for handle the signals. The following functions are the old callback
 * functions for handle the signals.
 *    _button_month_dec_start
 *    _button_year_inc_start
 *    _button_year_dec_start
 *    _button_month_stop
 *    _button_year_stop
 *
 * But, it is replaced by elm_button widget objects. The following
 * callback functions are also newly added for button objects.
 * We still keep the old signal callback functions for backward compatibility. */

static void
_inc_dec_button_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED);
static void
_inc_dec_button_pressed_cb(void *data, const Efl_Event *event);
static void
_inc_dec_button_unpressed_cb(void *data, const Efl_Event *event EINA_UNUSED);

EFL_CALLBACKS_ARRAY_DEFINE( _inc_dec_button_cb,
   { EFL_UI_EVENT_CLICKED, _inc_dec_button_clicked_cb},
   { EFL_UI_EVENT_PRESSED, _inc_dec_button_pressed_cb},
   { EFL_UI_EVENT_UNPRESSED, _inc_dec_button_unpressed_cb}
);

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

static Elm_Calendar_Mark *
_mark_new(Evas_Object *obj,
          const char *mark_type,
          struct tm *mark_time,
          Elm_Calendar_Mark_Repeat_Type repeat)
{
   Elm_Calendar_Mark *mark;

   mark = calloc(1, sizeof(Elm_Calendar_Mark));
   if (!mark) return NULL;
   mark->obj = obj;
   mark->mark_type = eina_stringshare_add(mark_type);
   mark->mark_time = *mark_time;
   mark->repeat = repeat;

   return mark;
}

static inline void
_mark_free(Elm_Calendar_Mark *mark)
{
   eina_stringshare_del(mark->mark_type);
   free(mark);
}

EOLIAN static void
_elm_calendar_elm_layout_sizing_eval(Eo *obj, Elm_Calendar_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_CALENDAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->filling) return;
   // 7x8 (1 month+year, days, 6 dates.)
   elm_coords_finger_size_adjust(7, &minw, 8, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static inline int
_maxdays_get(struct tm *selected_time, int month_offset)
{
   int month, year;

   month = (selected_time->tm_mon + month_offset) % 12;
   year = selected_time->tm_year + 1900;

   if (month < 0) month += 12;

   return _days_in_month
          [((!(year % 4)) && ((!(year % 400)) || (year % 100)))][month];
}

static inline void
_unselect(Evas_Object *obj,
          int selected)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%i,unselected", selected);
   elm_layout_signal_emit(obj, emission, "elm");
}

static inline void
_select(Evas_Object *obj,
        int selected)
{
   char emission[32];

   ELM_CALENDAR_DATA_GET(obj, sd);

   sd->focused_it = sd->selected_it = selected;
   snprintf(emission, sizeof(emission), "cit_%i,selected", selected);
   elm_layout_signal_emit(obj, emission, "elm");
}

static inline void
_not_today(Elm_Calendar_Data *sd)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%i,not_today", sd->today_it);
   elm_layout_signal_emit(sd->obj, emission, "elm");
   sd->today_it = -1;
}

static inline void
_today(Elm_Calendar_Data *sd,
       int it)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%i,today", it);
   elm_layout_signal_emit(sd->obj, emission, "elm");
   sd->today_it = it;
}

static inline void
_enable(Elm_Calendar_Data *sd,
        int it)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%i,enable", it);
   elm_layout_signal_emit(sd->obj, emission, "elm");
}

static inline void
_disable(Elm_Calendar_Data *sd,
         int it)
{
   char emission[32];

   snprintf(emission, sizeof(emission), "cit_%i,disable", it);
   elm_layout_signal_emit(sd->obj, emission, "elm");
}

static char *
_format_month_year(struct tm *selected_time)
{
   return eina_strftime(E_("%B %Y"), selected_time);
}

static char *
_format_month(struct tm *selected_time)
{
   return eina_strftime(E_("%B"), selected_time);
}

static char *
_format_year(struct tm *selected_time)
{
   return eina_strftime(E_("%Y"), selected_time);
}

static inline void
_cit_mark(Evas_Object *cal,
          int cit,
          const char *mtype)
{
   ELM_CALENDAR_DATA_GET(cal, sd);

   int day = cit - sd->first_day_it + 1;
   int mon = sd->shown_time.tm_mon;
   int yr = sd->shown_time.tm_year;

   if (strcmp(mtype, "clear")
       && (((yr == sd->date_min.tm_year) && (mon == sd->date_min.tm_mon) && (day < sd->date_min.tm_mday))
           || ((yr == sd->date_max.tm_year) && (mon == sd->date_max.tm_mon) && (day > sd->date_max.tm_mday))))
     return;

   char sign[64];

   snprintf(sign, sizeof(sign), "cit_%i,%s", cit, mtype);
   elm_layout_signal_emit(cal, sign, "elm");
}

static inline int
_weekday_get(int first_week_day,
             int day)
{
   return (day + first_week_day - 1) % ELM_DAY_LAST;
}

// EINA_DEPRECATED
static void
_text_day_color_update(Elm_Calendar_Data *sd,
                       int pos)
{
   char emission[32];

   switch (sd->day_color[pos])
     {
      case DAY_WEEKDAY:
        snprintf(emission, sizeof(emission), "cit_%i,weekday", pos);
        break;

      case DAY_SATURDAY:
        snprintf(emission, sizeof(emission), "cit_%i,saturday", pos);
        break;

      case DAY_SUNDAY:
        snprintf(emission, sizeof(emission), "cit_%i,sunday", pos);
        break;

      default:
        return;
     }

   elm_layout_signal_emit(sd->obj, emission, "elm");
}

static void
_set_month_year(Elm_Calendar_Data *sd)
{
   char *buf;

   sd->filling = EINA_TRUE;
   if (sd->double_spinners) /* theme has spinner for year */
     {
        buf = _format_year(&sd->shown_time);
        if (buf)
          {
             elm_layout_text_set(sd->obj, "year_text", buf);
             free(buf);
          }
        else elm_layout_text_set(sd->obj, "year_text", "");

        buf = _format_month(&sd->shown_time);
     }
   else
      buf = sd->format_func(&sd->shown_time);

   if (buf)
     {
        elm_layout_text_set(sd->obj, "month_text", buf);
        free(buf);
     }
   else elm_layout_text_set(sd->obj, "month_text", "");
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

   ELM_CALENDAR_DATA_GET(obj, sd);

   maxdays = _maxdays_get(&sd->shown_time, 0);
   for (i = 0; i < 42; i++)
     {
        if ((!day) && (i == sd->first_day_it)) day = 1;
        if ((day) && (day <= maxdays))
          {
             _part_name_snprintf(pname, sizeof(pname),
                elm_layout_edje_get(obj), ELM_CALENDAR_CIT_ACCESS_PART_STR, i);

             ao = _elm_access_edje_object_part_object_register
                        (obj, elm_layout_edje_get(obj), pname);
             _elm_access_text_set(_elm_access_info_get(ao),
                         ELM_ACCESS_TYPE, E_("calendar item"));
             _elm_access_callback_set(_elm_access_info_get(ao),
                           ELM_ACCESS_INFO, _access_info_cb, NULL);

             snprintf(day_s, sizeof(day_s), "%i", (int) (day++));
             elm_widget_access_info_set(ao, (const char*)day_s);
          }
        else
          {
             _part_name_snprintf(pname, sizeof(pname),
                elm_layout_edje_get(obj), ELM_CALENDAR_CIT_ACCESS_PART_STR, i);
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
   ELM_CALENDAR_DATA_GET(obj, sd);

   if (!sd->dec_btn_month)
     sd->dec_btn_month = _elm_access_edje_object_part_object_register
        (obj, elm_layout_edje_get(obj), "left_bt");
   ai = _elm_access_info_get(sd->dec_btn_month);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar decrement month button"));

   if (!sd->dec_btn_year)
     sd->dec_btn_year = _elm_access_edje_object_part_object_register
        (obj, elm_layout_edje_get(obj), "left_bt_year");
   ai = _elm_access_info_get(sd->dec_btn_year);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar decrement year button"));

   if (!sd->inc_btn_month)
     sd->inc_btn_month = _elm_access_edje_object_part_object_register
        (obj, elm_layout_edje_get(obj), "right_bt");
   ai = _elm_access_info_get(sd->inc_btn_month);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar increment month button"));

   if (!sd->inc_btn_year)
     sd->inc_btn_year = _elm_access_edje_object_part_object_register
        (obj, elm_layout_edje_get(obj), "right_bt_year");
   ai = _elm_access_info_get(sd->inc_btn_year);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar increment year button"));

   sd->month_access = _elm_access_edje_object_part_object_register
                          (obj, elm_layout_edje_get(obj), "text_month");
   ai = _elm_access_info_get(sd->month_access);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar month"));

   sd->year_access = _elm_access_edje_object_part_object_register
                          (obj, elm_layout_edje_get(obj), "year_text");
   ai = _elm_access_info_get(sd->year_access);
   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("calendar year"));

   o = elm_layout_edje_get(obj);
   edje_object_freeze(o);
   po = (Evas_Object *)edje_object_part_object_get(o, "month_text");
   evas_object_pass_events_set(po, EINA_FALSE);

   po = (Evas_Object *)edje_object_part_object_get(o, "year_text");
   evas_object_pass_events_set(po, EINA_FALSE);
   edje_object_thaw(o);
}

static void
_access_calendar_register(Evas_Object *obj)
{
   _access_calendar_spinner_register(obj);
   _access_calendar_item_register(obj);
}

static void
_flush_calendar_composite_elements(Evas_Object *obj, Elm_Calendar_Data *sd)
{
   Eina_List *items = NULL;
   int max_day = _maxdays_get(&sd->shown_time, 0);

#define EXTEND(v) \
    if (v) items = eina_list_append(items, v); \

    EXTEND(sd->month_access);
    EXTEND(sd->dec_btn_month);
    EXTEND(sd->inc_btn_month);
    EXTEND(sd->year_access);
    EXTEND(sd->dec_btn_year);
    EXTEND(sd->inc_btn_year);

#undef EXTEND

   for (int i = sd->first_day_it; i <= max_day; ++i)
     items = eina_list_append(items, sd->items[i]);

   efl_ui_focus_composition_elements_set(obj, items);
}

static void
_populate(Evas_Object *obj)
{
   int maxdays, adjusted_wday, prev_month_maxdays, day, mon, yr, i;
   Elm_Calendar_Mark *mark;
   char part[16], day_s[16];
   struct tm first_day;
   Eina_List *l;
   Eina_Bool last_row = EINA_TRUE;

   ELM_CALENDAR_DATA_GET(obj, sd);

   elm_layout_freeze(obj);

   sd->filling = EINA_FALSE;
   if (sd->today_it > 0) _not_today(sd);

   maxdays = _maxdays_get(&sd->shown_time, 0);
   prev_month_maxdays = _maxdays_get(&sd->shown_time, -1);
   mon = sd->shown_time.tm_mon;
   yr = sd->shown_time.tm_year;

   _set_month_year(sd);
   sd->filling = EINA_TRUE;

   /* Set days */
   day = 0;
   first_day = sd->shown_time;
   first_day.tm_mday = 1;
   mktime(&first_day);

   // Layout of the calendar is changed for removing the unfilled last row.
   if (first_day.tm_wday < (int)sd->first_week_day)
     sd->first_day_it = first_day.tm_wday + ELM_DAY_LAST - sd->first_week_day;
   else
     sd->first_day_it = first_day.tm_wday - sd->first_week_day;

   if ((35 - sd->first_day_it) > (maxdays - 1)) last_row = EINA_FALSE;

   if (!last_row)
     {
        char emission[32];

        for (i = 0; i < 5; i++)
          {
             snprintf(emission, sizeof(emission), "cseph_%i,row_hide", i);
             elm_layout_signal_emit(obj, emission, "elm");
          }
        snprintf(emission, sizeof(emission), "cseph_%i,row_invisible", 5);
        elm_layout_signal_emit(obj, emission, "elm");
        for (i = 0; i < 35; i++)
          {
             snprintf(emission, sizeof(emission), "cit_%i,cell_expanded", i);
             elm_layout_signal_emit(obj, emission, "elm");
          }
        for (i = 35; i < 42; i++)
          {
             snprintf(emission, sizeof(emission), "cit_%i,cell_invisible", i);
             elm_layout_signal_emit(obj, emission, "elm");
          }
     }
   else
     {
        char emission[32];

        for (i = 0; i < 6; i++)
          {
             snprintf(emission, sizeof(emission), "cseph_%i,row_show", i);
             elm_layout_signal_emit(obj, emission, "elm");
          }
        for (i = 0; i < 42; i++)
          {
             snprintf(emission, sizeof(emission), "cit_%i,cell_default", i);
             elm_layout_signal_emit(obj, emission, "elm");
          }
     }

   for (i = 0; i < 42; i++)
     {
        _text_day_color_update(sd, i); // EINA_DEPRECATED
        if ((!day) && (i == sd->first_day_it)) day = 1;

        if ((day == sd->current_time.tm_mday)
            && (mon == sd->current_time.tm_mon)
            && (yr == sd->current_time.tm_year))
          _today(sd, i);

        if (day == sd->selected_time.tm_mday)
          {
             if ((sd->selected_it > -1) && (sd->selected_it != i))
               _unselect(obj, sd->selected_it);

             if (sd->select_mode == ELM_CALENDAR_SELECT_MODE_ONDEMAND)
               {
                  if ((mon == sd->selected_time.tm_mon)
                      && (yr == sd->selected_time.tm_year)
                      && (sd->selected))
                    {
                       _select(obj, i);
                    }
               }
             else if (sd->select_mode != ELM_CALENDAR_SELECT_MODE_NONE)
               {
                  _select(obj, i);
               }
          }

        if ((day) && (day <= maxdays))
          {
             if (((yr == sd->date_min.tm_year) && (mon == sd->date_min.tm_mon) && (day < sd->date_min.tm_mday))
                 || ((yr == sd->date_max.tm_year) && (mon == sd->date_max.tm_mon) && (day > sd->date_max.tm_mday)))
               _disable(sd, i);
             else
               _enable(sd, i);

             snprintf(day_s, sizeof(day_s), "%i", day++);
          }
        else
          {
             _disable(sd, i);

             if (day <= maxdays)
               snprintf(day_s, sizeof(day_s), "%i", prev_month_maxdays - sd->first_day_it + i + 1);
             else
               snprintf(day_s, sizeof(day_s), "%i", i - sd->first_day_it - maxdays + 1);
          }

        _part_name_snprintf(part, sizeof(part), obj, ELM_CALENDAR_CIT_TEXT_PART_STR, i);
        elm_layout_text_set(obj, part, day_s);

        /* Clear previous marks */
        _cit_mark(obj, i, "clear");
     }

   // ACCESS
   if ((_elm_config->access_mode != ELM_ACCESS_MODE_OFF))
     _access_calendar_item_register(obj);

   /* Set marks */
   EINA_LIST_FOREACH(sd->marks, l, mark)
     {
        struct tm *mtime = &mark->mark_time;
        int month = sd->shown_time.tm_mon;
        int year = sd->shown_time.tm_year;
        int mday_it = mtime->tm_mday + sd->first_day_it - 1;

        switch (mark->repeat)
          {
           case ELM_CALENDAR_UNIQUE:
             if ((mtime->tm_mon == month) && (mtime->tm_year == year))
               _cit_mark(obj, mday_it, mark->mark_type);
             break;

           case ELM_CALENDAR_DAILY:
             if (((mtime->tm_year == year) && (mtime->tm_mon < month)) ||
                 (mtime->tm_year < year))
               day = 1;
             else if ((mtime->tm_year == year) && (mtime->tm_mon == month))
               day = mtime->tm_mday;
             else
               break;
             for (; day <= maxdays; day++)
               _cit_mark(obj, day + sd->first_day_it - 1,
                         mark->mark_type);
             break;

           case ELM_CALENDAR_WEEKLY:
             if (((mtime->tm_year == year) && (mtime->tm_mon < month)) ||
                 (mtime->tm_year < year))
               day = 1;
             else if ((mtime->tm_year == year) && (mtime->tm_mon == month))
               day = mtime->tm_mday;
             else
               break;

             adjusted_wday = (mtime->tm_wday - sd->first_week_day);
             if (adjusted_wday < 0)
               adjusted_wday = ELM_DAY_LAST + adjusted_wday;

             for (; day <= maxdays; day++)
               if (adjusted_wday == _weekday_get(sd->first_day_it, day))
                 _cit_mark(obj, day + sd->first_day_it - 1,
                           mark->mark_type);
             break;

           case ELM_CALENDAR_MONTHLY:
             if (((mtime->tm_year < year) ||
                  ((mtime->tm_year == year) && (mtime->tm_mon <= month))) &&
                 (mtime->tm_mday <= maxdays))
               _cit_mark(obj, mday_it, mark->mark_type);
             break;

           case ELM_CALENDAR_ANNUALLY:
             if ((mtime->tm_year <= year) && (mtime->tm_mon == month) &&
                 (mtime->tm_mday <= maxdays))
               _cit_mark(obj, mday_it, mark->mark_type);
             break;

           case ELM_CALENDAR_LAST_DAY_OF_MONTH:
             if (((mtime->tm_year < year) ||
                  ((mtime->tm_year == year) && (mtime->tm_mon <= month))))
               _cit_mark(obj, maxdays + sd->first_day_it - 1, mark->mark_type);
             break;

           case ELM_CALENDAR_REVERSE_DAILY:
             if (((mtime->tm_year == year) && (mtime->tm_mon > month)) ||
                 (mtime->tm_year > year))
               day = maxdays;
             else if ((mtime->tm_year == year) && (mtime->tm_mon == month))
               day = mtime->tm_mday - 1;
             else
               break;
             for (; day >= 1; day--)
               _cit_mark(obj, day + sd->first_day_it - 1,
                         mark->mark_type);
             break;

          }
     }
   sd->filling = EINA_FALSE;

   elm_layout_thaw(obj);
   edje_object_message_signal_process(elm_layout_edje_get(obj));

   _flush_calendar_composite_elements(obj, sd);
}

static void
_set_headers(Evas_Object *obj)
{
   static char part[64];
   int i;
   struct tm *t;
   time_t temp = 259200; // the first sunday since epoch
   ELM_CALENDAR_DATA_GET(obj, sd);

   elm_layout_freeze(obj);

   sd->filling = EINA_TRUE;

   t = gmtime(&temp);
   if (t && !sd->weekdays_set)
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
        _part_name_snprintf(part, sizeof(part), obj, ELM_CALENDAR_CH_TEXT_PART_STR, i);
        elm_layout_text_set(obj, part, sd->weekdays[(i + sd->first_week_day) % ELM_DAY_LAST]);
     }

   sd->filling = EINA_FALSE;

   elm_layout_thaw(obj);
}

static void
_spinner_buttons_add(Evas_Object *obj, Elm_Calendar_Data *sd)
{
   char left_buf[255] = { 0 };
   char right_buf[255] = { 0 };

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   snprintf(left_buf, sizeof(left_buf), "calendar/decrease/%s", elm_object_style_get(obj));
   snprintf(right_buf, sizeof(right_buf), "calendar/increase/%s", elm_object_style_get(obj));

   if (edje_object_part_exists(wd->resize_obj, ELM_CALENDAR_BUTTON_LEFT))
     {
        if (sd->dec_btn_month && efl_isa(sd->dec_btn_month, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "left_bt");
             sd->dec_btn_month = NULL;
          }

        if (!sd->dec_btn_month)
          {
             sd->dec_btn_month = elm_button_add(obj);
             efl_event_callback_array_add(sd->dec_btn_month, _inc_dec_button_cb(), obj);
          }

        elm_object_style_set(sd->dec_btn_month, left_buf);
        elm_layout_content_set(obj, ELM_CALENDAR_BUTTON_LEFT, sd->dec_btn_month);
     }
   else if (sd->dec_btn_month && !efl_isa(sd->dec_btn_month, ELM_ACCESS_CLASS))
     {
        evas_object_del(sd->dec_btn_month);
        sd->dec_btn_month = NULL;
     }

   if (edje_object_part_exists(wd->resize_obj, ELM_CALENDAR_BUTTON_RIGHT))
     {
        if (sd->inc_btn_month && efl_isa(sd->inc_btn_month, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "right_bt");
             sd->inc_btn_month = NULL;
          }

        if (!sd->inc_btn_month)
          {
             sd->inc_btn_month = elm_button_add(obj);
             efl_event_callback_array_add(sd->inc_btn_month, _inc_dec_button_cb(), obj);
          }

        elm_object_style_set(sd->inc_btn_month, right_buf);
        elm_layout_content_set(obj, ELM_CALENDAR_BUTTON_RIGHT, sd->inc_btn_month);
     }
   else if (sd->inc_btn_month && !efl_isa(sd->inc_btn_month, ELM_ACCESS_CLASS))
     {
        evas_object_del(sd->inc_btn_month);
        sd->inc_btn_month = NULL;
     }

   if (edje_object_part_exists(wd->resize_obj, ELM_CALENDAR_BUTTON_YEAR_LEFT))
     {
        if (sd->dec_btn_year && efl_isa(sd->dec_btn_year, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "left_bt_year");
             sd->dec_btn_year = NULL;
          }

        if (!sd->dec_btn_year)
          {
             sd->dec_btn_year = elm_button_add(obj);
             efl_event_callback_array_add(sd->dec_btn_year, _inc_dec_button_cb(), obj);
          }

        elm_object_style_set(sd->dec_btn_year, left_buf);
        elm_layout_content_set(obj, ELM_CALENDAR_BUTTON_YEAR_LEFT, sd->dec_btn_year);
     }
   else if (sd->dec_btn_year && !efl_isa(sd->dec_btn_year, ELM_ACCESS_CLASS))
     {
        evas_object_del(sd->dec_btn_year);
        sd->dec_btn_year = NULL;
     }

   if (edje_object_part_exists(wd->resize_obj, ELM_CALENDAR_BUTTON_YEAR_RIGHT))
     {
        if (sd->inc_btn_year && efl_isa(sd->inc_btn_year, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "right_bt_year");
             sd->inc_btn_year = NULL;
          }

        if (!sd->inc_btn_year)
          {
             sd->inc_btn_year = elm_button_add(obj);
             efl_event_callback_array_add(sd->inc_btn_year, _inc_dec_button_cb(), obj);
          }

        elm_object_style_set(sd->inc_btn_year, right_buf);
        elm_layout_content_set(obj, ELM_CALENDAR_BUTTON_YEAR_RIGHT, sd->inc_btn_year);
     }
   else if (sd->inc_btn_year && !efl_isa(sd->inc_btn_year, ELM_ACCESS_CLASS))
     {
        evas_object_del(sd->inc_btn_year);
        sd->inc_btn_year = NULL;
     }
}

EOLIAN static Eina_Error
_elm_calendar_efl_ui_widget_theme_apply(Eo *obj, Elm_Calendar_Data *sd)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   _spinner_buttons_add(obj, sd);

   evas_object_smart_changed(obj);
   return int_ret;
}

/* Set correct tm_wday and tm_yday after other fields changes*/
static inline void
_fix_selected_time(Elm_Calendar_Data *sd)
{
   if (sd->selected_time.tm_mon != sd->shown_time.tm_mon)
     sd->selected_time.tm_mon = sd->shown_time.tm_mon;
   if (sd->selected_time.tm_year != sd->shown_time.tm_year)
     sd->selected_time.tm_year = sd->shown_time.tm_year;

   if ((sd->selected_time.tm_year == sd->date_min.tm_year)
       && (sd->selected_time.tm_mon == sd->date_min.tm_mon)
       && (sd->selected_time.tm_mday < sd->date_min.tm_mday))
     {
        sd->selected_time.tm_mday = sd->date_min.tm_mday;
     }
   else if ((sd->selected_time.tm_year == sd->date_max.tm_year)
            && (sd->selected_time.tm_mon == sd->date_max.tm_mon)
            && (sd->selected_time.tm_mday > sd->date_max.tm_mday))
     {
        sd->selected_time.tm_mday = sd->date_max.tm_mday;
     }

   mktime(&sd->selected_time);
}

static Eina_Bool
_update_data(Evas_Object *obj, Eina_Bool month,
              int delta)
{
   struct tm time_check;
   int maxdays, years;

   ELM_CALENDAR_DATA_GET(obj, sd);

   /* check if it's a valid time. for 32 bits, year greater than 2037 is not */
   time_check = sd->shown_time;
   if (month)
       time_check.tm_mon += delta;
   else
       time_check.tm_year += delta;
   if (mktime(&time_check) == -1)
     return EINA_FALSE;

   if (month)
     {
        sd->shown_time.tm_mon += delta;
        if (delta < 0)
          {
             if (sd->shown_time.tm_year == sd->date_min.tm_year)
               {
                  if (sd->shown_time.tm_mon < sd->date_min.tm_mon)
                    {
                       sd->shown_time.tm_mon = sd->date_min.tm_mon;
                       return EINA_FALSE;
                    }
               }
             else if (sd->shown_time.tm_mon < 0)
               {
                  sd->shown_time.tm_mon = 11;
                  sd->shown_time.tm_year--;
               }
          }
        else
          {
             if (sd->shown_time.tm_year == sd->date_max.tm_year)
               {
                  if (sd->shown_time.tm_mon > sd->date_max.tm_mon)
                    {
                       sd->shown_time.tm_mon = sd->date_max.tm_mon;
                       return EINA_FALSE;
                    }
               }
             else if (sd->shown_time.tm_mon > 11)
               {
                  sd->shown_time.tm_mon = 0;
                  sd->shown_time.tm_year++;
               }
          }
     }
   else
     {
        years = sd->shown_time.tm_year + delta;
        if (((years > sd->date_max.tm_year) && (sd->date_max.tm_year != -1)) ||
            years < sd->date_min.tm_year)
          return EINA_FALSE;

        sd->shown_time.tm_year = years;
        if ((years == sd->date_min.tm_year) && (sd->shown_time.tm_mon < sd->date_min.tm_mon))
          {
             sd->shown_time.tm_mon = sd->date_min.tm_mon;
          }
        else if ((years == sd->date_max.tm_year) && (sd->shown_time.tm_mon > sd->date_max.tm_mon))
          {
             sd->shown_time.tm_mon = sd->date_max.tm_mon;
          }
     }

   if ((sd->select_mode != ELM_CALENDAR_SELECT_MODE_ONDEMAND)
       && (sd->select_mode != ELM_CALENDAR_SELECT_MODE_NONE))
     {
        maxdays = _maxdays_get(&sd->shown_time, 0);
        if (sd->selected_time.tm_mday > maxdays)
          sd->selected_time.tm_mday = maxdays;

        _fix_selected_time(sd);
        efl_event_callback_legacy_call(obj, ELM_CALENDAR_EVENT_CHANGED, NULL);
     }
   efl_event_callback_legacy_call(obj, ELM_CALENDAR_EVENT_DISPLAY_CHANGED, NULL);

   return EINA_TRUE;
}

static Eina_Bool
_spin_value(void *data)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   if (_update_data(data, sd->month_btn_clicked, sd->spin_speed))
     evas_object_smart_changed(data);

   sd->interval = sd->interval / 1.05;
   ecore_timer_interval_set(sd->spin_timer, sd->interval);

   return ECORE_CALLBACK_RENEW;
}

/* Legacy callbacks for signals from edje */
static void
_button_month_inc_start(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = 1;
   sd->month_btn_clicked = EINA_TRUE;
   ecore_timer_del(sd->spin_timer);
   sd->spin_timer = ecore_timer_add(sd->interval, _spin_value, data);

   elm_widget_scroll_freeze_push(data);

   _spin_value(data);
}

/* Legacy callbacks for signals from edje */
static void
_button_month_dec_start(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = -1;
   sd->month_btn_clicked = EINA_TRUE;
   ecore_timer_del(sd->spin_timer);
   sd->spin_timer = ecore_timer_add(sd->interval, _spin_value, data);

   elm_widget_scroll_freeze_push(data);

   _spin_value(data);
}

/* Legacy callbacks for signals from edje */
static void
_button_month_stop(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   const char *emission EINA_UNUSED,
                   const char *source EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);

   elm_widget_scroll_freeze_pop(obj);
}

/* Legacy callbacks for signals from edje */
static void
_button_year_inc_start(void *data,
                       Evas_Object *obj EINA_UNUSED,
                       const char *emission EINA_UNUSED,
                       const char *source EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = 1;
   sd->month_btn_clicked = EINA_FALSE;
   ecore_timer_del(sd->spin_timer);
   sd->spin_timer = ecore_timer_add(sd->interval, _spin_value, data);

   elm_widget_scroll_freeze_push(data);

   _spin_value(data);
}

/* Legacy callbacks for signals from edje */
static void
_button_year_dec_start(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = -1;
   sd->month_btn_clicked = EINA_FALSE;
   ecore_timer_del(sd->spin_timer);
   sd->spin_timer = ecore_timer_add(sd->interval, _spin_value, data);

   elm_widget_scroll_freeze_push(data);

   _spin_value(data);
}

/* Legacy callbacks for signals from edje */
static void
_button_year_stop(void *data,
                  Evas_Object *obj EINA_UNUSED,
                  const char *emission EINA_UNUSED,
                  const char *source EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);

   elm_widget_scroll_freeze_pop(obj);
}

static void
_inc_dec_button_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);

   _spin_value(data);
}
static void
_inc_dec_button_pressed_cb(void *data, const Efl_Event *event)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = ((sd->inc_btn_month == event->object) ||
                     (sd->inc_btn_year == event->object)) ? 1 : -1;

   sd->month_btn_clicked = ((sd->inc_btn_month == event->object) ||
                            (sd->dec_btn_month == event->object)) ? EINA_TRUE : EINA_FALSE;

   ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);
   sd->spin_timer = ecore_timer_add(sd->interval, _spin_value, data);

   elm_widget_scroll_freeze_push(data);
}

static void
_inc_dec_button_unpressed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);

   elm_widget_scroll_freeze_pop(data);
}

static int
_get_item_day(Evas_Object *obj,
              int selected_it)
{
   int day;

   ELM_CALENDAR_DATA_GET(obj, sd);

   day = selected_it - sd->first_day_it + 1;
   if ((day < 0) || (day > _maxdays_get(&sd->shown_time, 0)))
     return 0;

   if ((sd->shown_time.tm_year == sd->date_min.tm_year)
       && (sd->shown_time.tm_mon == sd->date_min.tm_mon)
       && (day < sd->date_min.tm_mday))
     {
        return 0;
     }
   else if ((sd->shown_time.tm_year == sd->date_max.tm_year)
            && (sd->shown_time.tm_mon == sd->date_max.tm_mon)
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

   ELM_CALENDAR_DATA_GET(obj, sd);

   day = _get_item_day(obj, unfocused_it);
   if (!day)
     return;

   sd->focused_it = -1;

   snprintf(emission, sizeof(emission), "cit_%i,unfocused", unfocused_it);
   elm_layout_signal_emit(obj, emission, "elm");
}

static Eina_Bool
_update_focused_it(Evas_Object *obj, int focused_it)
{
   int day;
   char emission[32];

   ELM_CALENDAR_DATA_GET(obj, sd);

   day = _get_item_day(obj, focused_it);
   if (!day)
     return EINA_FALSE;

   snprintf(emission, sizeof(emission), "cit_%i,unfocused", sd->focused_it);
   elm_layout_signal_emit(obj, emission, "elm");

   sd->focused_it = focused_it;

   snprintf(emission, sizeof(emission), "cit_%i,focused", sd->focused_it);
   elm_layout_signal_emit(obj, emission, "elm");

   return EINA_TRUE;
}

static void
_update_sel_it(Evas_Object *obj,
               int sel_it)
{
   int day;

   ELM_CALENDAR_DATA_GET(obj, sd);

   if (sd->select_mode == ELM_CALENDAR_SELECT_MODE_NONE)
     return;

   day = _get_item_day(obj, sel_it);
   if (!day)
     return;

   _unselect(obj, sd->selected_it);
   if (!sd->selected)
     sd->selected = EINA_TRUE;
   if (sd->focused_it)
     _update_unfocused_it(obj, sd->focused_it);

   sd->selected_time.tm_mday = day;
   _fix_selected_time(sd);
   _select(obj, sel_it);
   efl_event_callback_legacy_call(obj, ELM_CALENDAR_EVENT_CHANGED, NULL);
}

static void
_day_selected(void *data,
              Evas_Object *obj EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source)
{
   int sel_it;

   ELM_CALENDAR_DATA_GET(data, sd);

   if (sd->select_mode == ELM_CALENDAR_SELECT_MODE_NONE)
     return;

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
   time_t current_time;
   int t, day;
   ELM_CALENDAR_DATA_GET(data, sd);

   if (sd->today_it > 0) _not_today(sd);

   current_time = time(NULL);
   localtime_r(&current_time, &sd->current_time);
   t = _time_to_next_day(&sd->current_time);
   ecore_timer_interval_set(sd->update_timer, t);

   if ((sd->current_time.tm_mon != sd->shown_time.tm_mon) ||
       (sd->current_time.tm_year != sd->shown_time.tm_year))
     return ECORE_CALLBACK_RENEW;

   day = sd->current_time.tm_mday + sd->first_day_it - 1;
   _today(sd, day);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_key_action_activate(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(obj, sd);

   _update_sel_it(obj, sd->focused_it);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_calendar_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Calendar_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (efl_ui_focus_object_focus_get(obj))
     _update_focused_it(obj, sd->selected_it);
   else
     _update_unfocused_it(obj, sd->focused_it);

   return EINA_TRUE;
}

EOLIAN static void
_elm_calendar_efl_canvas_group_group_calculate(Eo *obj, Elm_Calendar_Data *_pd EINA_UNUSED)
{
   elm_layout_freeze(obj);

   _set_headers(obj);
   _populate(obj);

   elm_layout_thaw(obj);
}

static void
_style_changed(void *data,
               Evas_Object *obj EINA_UNUSED,
               const char *emission EINA_UNUSED,
               const char *source EINA_UNUSED)
{
   ELM_CALENDAR_DATA_GET(data, sd);

   if (!strcmp("double_spinners", elm_object_style_get(sd->obj)))
      sd->double_spinners = EINA_TRUE;
   else
      sd->double_spinners = EINA_FALSE;

   _set_month_year(sd);
}

EOLIAN static void
_elm_calendar_efl_canvas_group_group_add(Eo *obj, Elm_Calendar_Data *priv)
{
   time_t current_time;
   int t;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   priv->first_interval = 0.85;
   priv->date_min.tm_year = 2;
   priv->date_min.tm_mon = 0;
   priv->date_min.tm_mday = 1;
   priv->date_max.tm_year = -1;
   priv->date_max.tm_mon = 11;
   priv->date_max.tm_mday = 31;
   priv->today_it = -1;
   priv->selected_it = -1;
   priv->first_day_it = -1;
   priv->format_func = _format_month_year;
   priv->selectable = (~(ELM_CALENDAR_SELECTABLE_NONE));

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,increment,start", "*",
     _button_month_inc_start, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,decrement,start", "*",
      _button_month_dec_start, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,increment,startyear", "*",
     _button_year_inc_start, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,decrement,startyear", "*",
     _button_year_dec_start, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,stop", "*",
     _button_month_stop, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,stopyear", "*",
     _button_year_stop, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,selected", "*",
     _day_selected, obj);
   edje_object_signal_callback_add
      (wd->resize_obj, "load", "*",
       _style_changed, obj);

   current_time = time(NULL);
   localtime_r(&current_time, &priv->shown_time);
   priv->current_time = priv->shown_time;
   priv->selected_time = priv->shown_time;
   t = _time_to_next_day(&priv->current_time);
   priv->update_timer = ecore_timer_add(t, _update_cur_date, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   if (!elm_layout_theme_set(obj, "calendar", "base",
                             elm_object_style_get(obj)))
     CRI("Failed to set layout!");

   _spinner_buttons_add(obj, priv);

   evas_object_smart_changed(obj);

   // ACCESS
   if ((_elm_config->access_mode != ELM_ACCESS_MODE_OFF))
      _access_calendar_spinner_register(obj);

   // Items for composition
   for (int i = 0; i < 42; ++i)
     {
        priv->items[i] = efl_add(ELM_CALENDAR_ITEM_CLASS, obj, elm_calendar_item_day_number_set(efl_added, i));
     }
}

EOLIAN static void
_elm_calendar_efl_canvas_group_group_del(Eo *obj, Elm_Calendar_Data *sd)
{
   int i;
   Elm_Calendar_Mark *mark;

   ecore_timer_del(sd->spin_timer);
   ecore_timer_del(sd->update_timer);

   if (sd->marks)
     {
        EINA_LIST_FREE(sd->marks, mark)
          {
             _mark_free(mark);
          }
     }

   for (i = 0; i < ELM_DAY_LAST; i++)
     eina_stringshare_del(sd->weekdays[i]);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

static Eina_Bool _elm_calendar_smart_focus_next_enable = EINA_FALSE;

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   int maxdays, day, i;

   ELM_CALENDAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (is_access)
     _access_calendar_register(obj);
   else
     {
        day = 0;
        maxdays = _maxdays_get(&sd->shown_time, 0);
        for (i = 0; i < 42; i++)
          {
             if ((!day) && (i == sd->first_day_it)) day = 1;
             if ((day) && (day <= maxdays))
               {
                  char pname[18];
                  _part_name_snprintf(pname, sizeof(pname),
                     obj, ELM_CALENDAR_CIT_ACCESS_PART_STR, i);

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

        if (sd->dec_btn_year && efl_isa(sd->dec_btn_year, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "left_bt_year");
             sd->dec_btn_year = NULL;
          }
        if (sd->inc_btn_year && efl_isa(sd->inc_btn_year, ELM_ACCESS_CLASS))
          {
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "right_bt_year");
             sd->inc_btn_year = NULL;
          }
        if (sd->year_access)
          _elm_access_edje_object_part_object_unregister
            (obj, elm_layout_edje_get(obj), "year_text");
     }
}

EOLIAN static void
_elm_calendar_efl_ui_widget_on_access_update(Eo *obj EINA_UNUSED, Elm_Calendar_Data *_pd EINA_UNUSED, Eina_Bool acs)
{
   _elm_calendar_smart_focus_next_enable = acs;
   _access_obj_process(obj, _elm_calendar_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_calendar_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_calendar_efl_object_constructor(Eo *obj, Elm_Calendar_Data *sd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   sd->obj = obj;

   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_CALENDAR);

   return obj;
}

EOLIAN static void
_elm_calendar_weekdays_names_set(Eo *obj, Elm_Calendar_Data *sd, const char **weekdays)
{
   int i;

   EINA_SAFETY_ON_NULL_RETURN(weekdays);

   for (i = 0; i < ELM_DAY_LAST; i++)
     {
        eina_stringshare_replace(&sd->weekdays[i], weekdays[i]);
     }
   sd->weekdays_set = EINA_TRUE;

   evas_object_smart_changed(obj);
}

EOLIAN static const char**
_elm_calendar_weekdays_names_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   return sd->weekdays;
}

EOLIAN static void
_elm_calendar_interval_set(Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd, double interval)
{
   sd->first_interval = interval;
}

EOLIAN static double
_elm_calendar_interval_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   return sd->first_interval;
}

EAPI void
elm_calendar_min_max_year_set(Elm_Calendar *obj, int min, int max)
{
   Elm_Calendar_Data *sd;

   if (!efl_isa(obj, ELM_CALENDAR_CLASS)) return ;
   sd = efl_data_scope_get(obj, ELM_CALENDAR_CLASS);

   min -= 1900;
   max -= 1900;
   if ((sd->date_min.tm_year == min) && (sd->date_max.tm_year == max)) return;
   sd->date_min.tm_year = min > 2 ? min : 2;
   if (max > sd->date_min.tm_year)
     sd->date_max.tm_year = max;
   else
     sd->date_max.tm_year = sd->date_min.tm_year;
   sd->date_min.tm_mon = 0;
   sd->date_min.tm_mday = 1;
   sd->date_max.tm_mon = 11;
   sd->date_max.tm_mday = 31;

   if (sd->shown_time.tm_year > sd->date_max.tm_year)
     sd->shown_time.tm_year = sd->date_max.tm_year;
   if (sd->shown_time.tm_year < sd->date_min.tm_year)
     sd->shown_time.tm_year = sd->date_min.tm_year;

   evas_object_smart_changed(obj);
}

EAPI void
elm_calendar_min_max_year_get(const Elm_Calendar *obj, int *min, int *max)
{
   Elm_Calendar_Data *sd;

   if (!efl_isa(obj, ELM_CALENDAR_CLASS)) return ;
   sd = efl_data_scope_get(obj, ELM_CALENDAR_CLASS);

   if (min) *min = sd->date_min.tm_year + 1900;
   if (max) *max = sd->date_max.tm_year + 1900;
}

EOLIAN static void
_elm_calendar_date_min_set(Eo *obj, Elm_Calendar_Data *sd, const struct tm *min)
{
   Eina_Bool upper = EINA_FALSE;

   if ((sd->date_min.tm_year == min->tm_year)
       && (sd->date_min.tm_mon == min->tm_mon)
       && (sd->date_min.tm_mday == min->tm_mday))
     return;

   if (min->tm_year < 2)
     {
        sd->date_min.tm_year = 2;
        sd->date_min.tm_mon = 0;
        sd->date_min.tm_mday = 1;
     }
   else
     {
        if (sd->date_max.tm_year != -1)
          {
             if (min->tm_year > sd->date_max.tm_year)
               {
                  upper = EINA_TRUE;
               }
             else if (min->tm_year == sd->date_max.tm_year)
               {
                  if (min->tm_mon > sd->date_max.tm_mon)
                    upper = EINA_TRUE;
                  else if ((min->tm_mon == sd->date_max.tm_mon) && (min->tm_mday > sd->date_max.tm_mday))
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
             sd->date_min.tm_year = min->tm_year;
             sd->date_min.tm_mon = min->tm_mon;
             sd->date_min.tm_mday = min->tm_mday;
          }
     }

   if (sd->shown_time.tm_year <= sd->date_min.tm_year)
     {
        sd->shown_time.tm_year = sd->date_min.tm_year;
        if (sd->shown_time.tm_mon < sd->date_min.tm_mon)
          sd->shown_time.tm_mon = sd->date_min.tm_mon;
     }

   _fix_selected_time(sd);

   evas_object_smart_changed(obj);
}

EOLIAN static const struct tm *
_elm_calendar_date_min_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   return &(sd->date_min);
}

EOLIAN static void
_elm_calendar_date_max_set(Eo *obj, Elm_Calendar_Data *sd, const struct tm *max)
{
   Eina_Bool lower = EINA_FALSE;

   if ((sd->date_max.tm_year == max->tm_year)
       && (sd->date_max.tm_mon == max->tm_mon)
       && (sd->date_max.tm_mday == max->tm_mday))
     return;

   if (max->tm_year < sd->date_min.tm_year)
     {
        lower = EINA_TRUE;
     }
   else if (max->tm_year == sd->date_min.tm_year)
     {
        if (max->tm_mon < sd->date_min.tm_mon)
          lower = EINA_TRUE;
        else if ((max->tm_mon == sd->date_min.tm_mon) && (max->tm_mday < sd->date_min.tm_mday))
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
        sd->date_max.tm_year = max->tm_year;
        sd->date_max.tm_mon = max->tm_mon;
        sd->date_max.tm_mday = max->tm_mday;
     }

   if (sd->shown_time.tm_year >= sd->date_max.tm_year)
     {
        sd->shown_time.tm_year = sd->date_max.tm_year;
        if (sd->shown_time.tm_mon > sd->date_max.tm_mon)
          sd->shown_time.tm_mon = sd->date_max.tm_mon;
     }

   _fix_selected_time(sd);

   evas_object_smart_changed(obj);
}

EOLIAN static const struct tm *
_elm_calendar_date_max_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   return &(sd->date_max);
}

EINA_DEPRECATED EAPI void
elm_calendar_day_selection_disabled_set(Evas_Object *obj,
                                        Eina_Bool disabled)
{
   ELM_CALENDAR_CHECK(obj);

   if (disabled)
     elm_calendar_select_mode_set(obj, ELM_CALENDAR_SELECT_MODE_NONE);
   else
     elm_calendar_select_mode_set(obj, ELM_CALENDAR_SELECT_MODE_DEFAULT);
}

EINA_DEPRECATED EAPI Eina_Bool
elm_calendar_day_selection_disabled_get(const Evas_Object *obj)
{
   ELM_CALENDAR_CHECK(obj) EINA_FALSE;
   ELM_CALENDAR_DATA_GET(obj, sd);

   return !!(sd->select_mode == ELM_CALENDAR_SELECT_MODE_NONE);
}

EOLIAN static void
_elm_calendar_selected_time_set(Eo *obj, Elm_Calendar_Data *sd, struct tm *selected_time)
{
   EINA_SAFETY_ON_NULL_RETURN(selected_time);

   if (sd->selectable & ELM_CALENDAR_SELECTABLE_YEAR)
     sd->selected_time.tm_year = selected_time->tm_year;
   if (sd->selectable & ELM_CALENDAR_SELECTABLE_MONTH)
     sd->selected_time.tm_mon = selected_time->tm_mon;
   if (sd->selectable & ELM_CALENDAR_SELECTABLE_DAY)
       {
          sd->selected_time.tm_mday = selected_time->tm_mday;
          if (!sd->selected)
            sd->selected = EINA_TRUE;
       }
   else if (sd->select_mode != ELM_CALENDAR_SELECT_MODE_ONDEMAND)
     {
        if (!sd->selected)
          sd->selected = EINA_TRUE;
     }
   if (sd->selected_time.tm_year != sd->shown_time.tm_year)
     sd->shown_time.tm_year = sd->selected_time.tm_year;
   if (sd->selected_time.tm_mon != sd->shown_time.tm_mon)
     sd->shown_time.tm_mon = sd->selected_time.tm_mon;

   _fix_selected_time(sd);

   evas_object_smart_changed(obj);
}

EOLIAN static Eina_Bool
_elm_calendar_selected_time_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd, struct tm *selected_time)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(selected_time, EINA_FALSE);

   if ((sd->select_mode == ELM_CALENDAR_SELECT_MODE_ONDEMAND)
       && (!sd->selected))
     return EINA_FALSE;
   *selected_time = sd->selected_time;

   return EINA_TRUE;
}

EOLIAN static void
_elm_calendar_format_function_set(Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd, Elm_Calendar_Format_Cb format_function)
{
   sd->format_func = format_function;
   if (sd->double_spinners) /* theme has spinner for year */
      _set_month_year(sd);
}

EOLIAN static Elm_Calendar_Mark*
_elm_calendar_mark_add(Eo *obj, Elm_Calendar_Data *sd, const char *mark_type, struct tm *mark_time, Elm_Calendar_Mark_Repeat_Type repeat)
{
   Elm_Calendar_Mark *mark;

   mark = _mark_new(obj, mark_type, mark_time, repeat);
   if (!mark) return NULL;
   sd->marks = eina_list_append(sd->marks, mark);
   mark->node = eina_list_last(sd->marks);

   return mark;
}

EOLIAN static void
_elm_calendar_mark_del(Eo *obj, Elm_Calendar_Data *sd, Elm_Calendar_Mark *mark)
{
   EINA_SAFETY_ON_NULL_RETURN(mark);
   EINA_SAFETY_ON_TRUE_RETURN(mark->obj != obj);

   sd->marks = eina_list_remove_list(sd->marks, mark->node);
   _mark_free(mark);
}

EAPI void
elm_calendar_mark_del(Elm_Calendar_Mark *mark)
{
   EINA_SAFETY_ON_NULL_RETURN(mark);
   ELM_CALENDAR_CHECK(mark->obj);
   ELM_CALENDAR_DATA_GET(mark->obj, sd);

   sd->marks = eina_list_remove_list(sd->marks, mark->node);
   _mark_free(mark);
}

EOLIAN static void
_elm_calendar_marks_clear(Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   Elm_Calendar_Mark *mark;

   EINA_LIST_FREE(sd->marks, mark)
     _mark_free(mark);
}

EOLIAN static const Eina_List*
_elm_calendar_marks_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   return sd->marks;
}

EOLIAN static void
_elm_calendar_marks_draw(Eo *obj, Elm_Calendar_Data *_pd EINA_UNUSED)
{
   evas_object_smart_changed(obj);
}

EOLIAN static void
_elm_calendar_first_day_of_week_set(Eo *obj, Elm_Calendar_Data *sd, Elm_Calendar_Weekday day)
{
   if (day >= ELM_DAY_LAST) return;
   if (sd->first_week_day != day)
     {
        sd->first_week_day = day;
        evas_object_smart_changed(obj);
     }
}

EOLIAN static Elm_Calendar_Weekday
_elm_calendar_first_day_of_week_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   return sd->first_week_day;
}

EOLIAN static void
_elm_calendar_select_mode_set(Eo *obj, Elm_Calendar_Data *sd, Elm_Calendar_Select_Mode mode)
{

   if ((mode <= ELM_CALENDAR_SELECT_MODE_ONDEMAND)
       && (sd->select_mode != mode))
     {
        sd->select_mode = mode;
        if (sd->select_mode == ELM_CALENDAR_SELECT_MODE_ONDEMAND)
          sd->selected = EINA_FALSE;
        if ((sd->select_mode == ELM_CALENDAR_SELECT_MODE_ALWAYS)
            || (sd->select_mode == ELM_CALENDAR_SELECT_MODE_DEFAULT))
          _select(obj, sd->selected_it);
        else
          _unselect(obj, sd->selected_it);
     }
}

EOLIAN static Elm_Calendar_Select_Mode
_elm_calendar_select_mode_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   return sd->select_mode;
}

EOLIAN static void
_elm_calendar_selectable_set(Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd, Elm_Calendar_Selectable selectable)
{
   sd->selectable = selectable;
}

EOLIAN static Elm_Calendar_Selectable
_elm_calendar_selectable_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd)
{
   return sd->selectable;
}

EOLIAN static Eina_Bool
_elm_calendar_displayed_time_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd, struct tm *displayed_time)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(displayed_time, EINA_FALSE);
   *displayed_time = sd->shown_time;
   return EINA_TRUE;
}

static void
_elm_calendar_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
      _elm_calendar_smart_focus_next_enable = EINA_TRUE;
}

EOLIAN static const Efl_Access_Action_Data*
_elm_calendar_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Data *sd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "activate", "activate", NULL, _key_action_activate},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(elm_calendar, Elm_Calendar_Data)

/* Internal EO APIs and hidden overrides */

#define ELM_CALENDAR_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_calendar), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_calendar)

#include "elm_calendar_eo.c"

typedef struct {
   int v;
   Evas_Object *part;
}  Elm_Calendar_Item_Data;

EOLIAN static void
_elm_calendar_item_day_number_set(Eo *obj, Elm_Calendar_Item_Data *pd, int i)
{
   char pname[18];
   Evas_Object *po, *o;

   pd->v = i;

   o = elm_layout_edje_get(efl_parent_get(obj));
   _part_name_snprintf(pname, sizeof(pname), o, ELM_CALENDAR_CIT_ACCESS_PART_STR, i);
   edje_object_freeze(o);
   po = (Evas_Object *)edje_object_part_object_get(o, pname);
   edje_object_thaw(o);

   if (_elm_config->access_mode != ELM_ACCESS_MODE_ON)
     pd->part = po;
   else
     pd->part = evas_object_data_get(po, "_part_access_obj");

   _efl_ui_focus_event_redirector(pd->part, obj);

   EINA_SAFETY_ON_NULL_RETURN(pd->part);
}

EOLIAN static int
_elm_calendar_item_day_number_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Item_Data *pd)
{
   return pd->v;
}

EOLIAN static void
_elm_calendar_item_efl_ui_focus_object_focus_set(Eo *obj, Elm_Calendar_Item_Data *pd, Eina_Bool focus)
{
   efl_ui_focus_object_focus_set(efl_super(obj, ELM_CALENDAR_ITEM_CLASS), focus);

   _update_focused_it(efl_parent_get(obj), pd->v);
   evas_object_focus_set(pd->part, efl_ui_focus_object_focus_get(obj));
}

EOLIAN static Eina_Rect
_elm_calendar_item_efl_ui_focus_object_focus_geometry_get(const Eo *obj EINA_UNUSED, Elm_Calendar_Item_Data *pd)
{
   return efl_gfx_entity_geometry_get(pd->part);
}

EOLIAN static Efl_Ui_Focus_Object*
_elm_calendar_item_efl_ui_focus_object_focus_parent_get(const Eo *obj, Elm_Calendar_Item_Data *pd EINA_UNUSED)
{
   return efl_parent_get(obj);
}


#include "elm_calendar_item_eo.c"
