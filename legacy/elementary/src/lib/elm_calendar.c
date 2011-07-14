#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

typedef enum _Day_Color // EINA_DEPRECATED
{
   DAY_WEEKDAY = 0,
   DAY_SATURDAY = 1,
   DAY_SUNDAY = 2
} Day_Color;

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *calendar;
   Eina_List *marks;
   double interval, first_interval;
   int year_min, year_max, spin_speed;
   int today_it, selected_it, first_day_it;
   Ecore_Timer *spin, *update_timer;
   char * (*format_func) (struct tm *selected_time);
   const char *weekdays[7];
   struct tm current_time, selected_time;
   Day_Color day_color[42]; // EINA_DEPRECATED
   Eina_Bool selection_enabled : 1;
};

struct _Elm_Calendar_Mark
{
   Evas_Object *obj;
   Eina_List *node;
   struct tm mark_time;
   const char *mark_type;
   Elm_Calendar_Mark_Repeat repeat;
};

static const char *widtype = NULL;
static void _on_focus_hook(void *data, Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);

static const char SIG_CHANGED[] = "changed";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CHANGED, ""},
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
_mark_new(Evas_Object *obj, const char *mark_type, struct tm *mark_time, Elm_Calendar_Mark_Repeat repeat)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Calendar_Mark *mark;

   if (!wd) return NULL;
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

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(8, &minw, 7, &minh);
   edje_object_size_min_restricted_calc(wd->calendar, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static inline int
_maxdays_get(struct tm *selected_time)
{
   int month, year;

   month = selected_time->tm_mon;
   year = selected_time->tm_year + 1900;

   return _days_in_month[((!(year % 4)) &&
                          ((!(year % 400)) ||
                           (year % 100)))]
      [month];
}

static inline void
_unselect(Widget_Data *wd, int selected)
{
   char emission[32];
   snprintf(emission, sizeof(emission), "cit_%i,unselected", selected);
   edje_object_signal_emit(wd->calendar, emission, "elm");
}

static inline void
_select(Widget_Data *wd, int selected)
{
   char emission[32];
   snprintf(emission, sizeof(emission), "cit_%i,selected", selected);
   edje_object_signal_emit(wd->calendar, emission, "elm");
}

static inline void
_not_today(Widget_Data *wd)
{
   char emission[32];
   snprintf(emission, sizeof(emission), "cit_%i,not_today", wd->today_it);
   edje_object_signal_emit(wd->calendar, emission, "elm");
   wd->today_it = -1;
}

static inline void
_today(Widget_Data *wd, int it)
{
   char emission[32];
   snprintf(emission, sizeof(emission), "cit_%i,today", it);
   edje_object_signal_emit(wd->calendar, emission, "elm");
   wd->today_it = it;
}

static char *
_format_month_year(struct tm *selected_time)
{
   char buf[32];
   if (!strftime(buf, sizeof(buf), "%B %Y", selected_time)) return NULL;
   return strdup(buf);
}

static inline void
_cit_mark(Evas_Object *cal, int cit, const char *mtype)
{
   char sign[64];
   snprintf(sign, sizeof(sign), "cit_%i,%s", cit, mtype);
   edje_object_signal_emit(cal, sign, "elm");
}

static inline int
_weekday_get(int first_week_day, int day)
{
   return (day + first_week_day - 1) % 7;
}

// EINA_DEPRECATED
static void
_text_day_color_update(Widget_Data *wd, int pos)
{
   char emission[32];

   switch (wd->day_color[pos])
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

   edje_object_signal_emit(wd->calendar, emission, "elm");
}

// EINA_DEPRECATED
static void
_text_day_color_set(Widget_Data *wd, Day_Color col, int pos)
{
   if ((pos < 0) || (pos >= 42)) return;
   if (wd->day_color[pos] == col) return;
   wd->day_color[pos] = col;
   _text_day_color_update(wd, pos);
}

static void
_populate(Evas_Object *obj)
{
   int maxdays, day, mon, year, i;
   Elm_Calendar_Mark *mark;
   char part[12], day_s[3];
   struct tm first_day;
   Eina_List *l;
   char *buf;
   Eina_Bool last_row = EINA_TRUE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   if (wd->today_it > 0) _not_today(wd);

   maxdays = _maxdays_get(&wd->selected_time);
   mon = wd->selected_time.tm_mon;
   year = wd->selected_time.tm_year;

   /* Set selected month */
   buf = wd->format_func(&wd->selected_time);
   if (buf)
     {
        edje_object_part_text_set(wd->calendar, "month_text", buf);
        free(buf);
     }
   else
     edje_object_part_text_set(wd->calendar, "month_text", "");

   /* Set days */
   day = 0;
   first_day = wd->selected_time;
   first_day.tm_mday = 1;
   mktime(&first_day);

   // Layout of the calendar is changed for removing the unfilled last row.
   wd->first_day_it = first_day.tm_wday;

   if ((35 - wd->first_day_it) > (maxdays - 1)) last_row = EINA_FALSE;

   if (!last_row)
     {
        char emission[32];

        for (i = 0; i < 5; i++)
          {
             snprintf(emission, sizeof(emission), "cseph_%i,row_hide", i);
             edje_object_signal_emit(wd->calendar, emission, "elm");
          }
        snprintf(emission, sizeof(emission), "cseph_%i,row_invisible", 5);
        edje_object_signal_emit(wd->calendar, emission, "elm");
        for (i = 0; i < 35; i++)
          {
             snprintf(emission, sizeof(emission), "cit_%i,cell_expanded", i);
             edje_object_signal_emit(wd->calendar, emission, "elm");
          }
        for (i = 35; i < 42; i++)
          {
             snprintf(emission, sizeof(emission), "cit_%i,cell_invisible", i);
             edje_object_signal_emit(wd->calendar, emission, "elm");
          }
     }
   else
     {
        char emission[32];

        for (i = 0; i < 6; i++)
          {
             snprintf(emission, sizeof(emission), "cseph_%i,row_show", i);
             edje_object_signal_emit(wd->calendar, emission, "elm");
          }
        for (i = 0; i < 42; i++)
          {
             snprintf(emission, sizeof(emission), "cit_%i,cell_default", i);
             edje_object_signal_emit(wd->calendar, emission, "elm");
          }
     }

   for (i = 0; i < 42; i++)
     {
        _text_day_color_update(wd, i); // EINA_DEPRECATED
        if ((!day) && (i == first_day.tm_wday)) day = 1;

        if ((day == wd->current_time.tm_mday)
            && (mon == wd->current_time.tm_mon)
            && (year == wd->current_time.tm_year))
          _today(wd, i);

        if (day == wd->selected_time.tm_mday)
          {
             if ((wd->selected_it > -1) && (wd->selected_it != i))
               _unselect(wd, wd->selected_it);

             if (wd->selection_enabled) _select(wd, i);

             wd->selected_it = i;
          }

        if ((day) && (day <= maxdays))
          snprintf(day_s, sizeof(day_s), "%i", day++);
        else
          day_s[0] = 0;

        snprintf(part, sizeof(part), "cit_%i.text", i);
        edje_object_part_text_set(wd->calendar, part, day_s);
        /* Clear previous marks */
        _cit_mark(wd->calendar, i, "clear");
     }

   /* Set marks */
   EINA_LIST_FOREACH(wd->marks, l, mark)
     {
        struct tm *mtime = &mark->mark_time;
        int month = wd->selected_time.tm_mon;
        int year = wd->selected_time.tm_year;
        int mday_it = mtime->tm_mday + wd->first_day_it - 1;

        switch (mark->repeat)
          {
           case ELM_CALENDAR_UNIQUE:
              if ((mtime->tm_mon == month) && (mtime->tm_year == year))
                _cit_mark(wd->calendar, mday_it, mark->mark_type);
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
                _cit_mark(wd->calendar, day + wd->first_day_it - 1,
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
              for (; day <= maxdays; day++)
                if (mtime->tm_wday == _weekday_get(wd->first_day_it, day))
                  _cit_mark(wd->calendar, day + wd->first_day_it - 1,
                            mark->mark_type);
              break;
           case ELM_CALENDAR_MONTHLY:
              if (((mtime->tm_year < year) ||
                   ((mtime->tm_year == year) && (mtime->tm_mon <= month))) &&
                  (mtime->tm_mday <= maxdays))
                _cit_mark(wd->calendar, mday_it, mark->mark_type);
              break;
           case ELM_CALENDAR_ANNUALLY:
              if ((mtime->tm_year <= year) && (mtime->tm_mon == month) &&
                  (mtime->tm_mday <= maxdays))
                _cit_mark(wd->calendar, mday_it, mark->mark_type);
              break;
          }
     }
}

static void
_set_headers(Evas_Object *obj)
{
   static char part[] = "ch_0.text";
   int i;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   for (i = 0; i < 7; i++)
     {
        part[3] = i + '0';
        edje_object_part_text_set(wd->calendar, part, wd->weekdays[i]);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   int i;
   Elm_Calendar_Mark *mark;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   if (wd->spin) ecore_timer_del(wd->spin);
   if (wd->update_timer) ecore_timer_del(wd->update_timer);

   if (wd->marks)
     {
        EINA_LIST_FREE(wd->marks, mark)
          {
             _mark_free(mark);
          }
     }

   for (i = 0; i < 7; i++)
     eina_stringshare_del(wd->weekdays[i]);

   free(wd);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit(wd->calendar, "elm,action,focus", "elm");
        evas_object_focus_set(wd->calendar, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit(wd->calendar, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->calendar, EINA_FALSE);
     }
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->calendar, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _elm_theme_object_set(obj, wd->calendar, "calendar", "base",
                         elm_widget_style_get(obj));
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _set_headers(obj);
   _populate(obj);
   edje_object_message_signal_process(wd->calendar);
   edje_object_scale_set(wd->calendar,
                         elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_signal_emit_hook(Evas_Object *obj, const char *emission, const char *source)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_emit(wd->calendar, emission, source);
}

static void
_signal_callback_add_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_add(wd->calendar, emission,
                                   source, func_cb, data);
}

static void
_signal_callback_del_hook(Evas_Object *obj, const char *emission, const char *source, Edje_Signal_Cb func_cb, void *data)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_signal_callback_del_full(wd->calendar, emission, source, func_cb,
                                        data);
}

/* Set correct tm_wday and tm_yday after other fields changes*/
static inline void
_fix_selected_time(Widget_Data *wd)
{
   mktime(&wd->selected_time);
}

static Eina_Bool
_update_month(Evas_Object *obj, int delta)
{
   struct tm time_check;
   int maxdays;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   /* check if it's a valid time. for 32 bits, year greater than 2037 is not */
   time_check = wd->selected_time;
   time_check.tm_mon += delta;
   if (mktime(&time_check) == -1)
     return EINA_FALSE;

   wd->selected_time.tm_mon += delta;
   if (wd->selected_time.tm_mon < 0)
     {
        if (wd->selected_time.tm_year == wd->year_min)
          {
             wd->selected_time.tm_mon++;
             return EINA_FALSE;
          }
        wd->selected_time.tm_mon = 11;
        wd->selected_time.tm_year--;
     }
   else if (wd->selected_time.tm_mon > 11)
     {
        if (wd->selected_time.tm_year == wd->year_max)
          {
             wd->selected_time.tm_mon--;
             return EINA_FALSE;
          }
        wd->selected_time.tm_mon = 0;
        wd->selected_time.tm_year++;
     }

   maxdays = _maxdays_get(&wd->selected_time);
   if (wd->selected_time.tm_mday > maxdays)
     wd->selected_time.tm_mday = maxdays;

   _fix_selected_time(wd);
   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);

   return EINA_TRUE;
}

static Eina_Bool
_spin_value(void *data)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return ECORE_CALLBACK_CANCEL;
   if (_update_month(data, wd->spin_speed)) _populate(data);
   wd->interval = wd->interval / 1.05;
   ecore_timer_interval_set(wd->spin, wd->interval);
   return ECORE_CALLBACK_RENEW;
}

static void
_button_inc_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->interval = wd->first_interval;
   wd->spin_speed = 1;
   if (wd->spin) ecore_timer_del(wd->spin);
   wd->spin = ecore_timer_add(wd->interval, _spin_value, data);
   _spin_value(data);
}

static void
_button_dec_start(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->interval = wd->first_interval;
   wd->spin_speed = -1;
   if (wd->spin) ecore_timer_del(wd->spin);
   wd->spin = ecore_timer_add(wd->interval, _spin_value, data);
   _spin_value(data);
}

static void
_button_stop(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->interval = wd->first_interval;
   if (wd->spin) ecore_timer_del(wd->spin);
   wd->spin = NULL;
}

static int
_get_item_day(Evas_Object *obj, int selected_it)
{
   int day;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0;

   day = selected_it - wd->first_day_it + 1;
   if ((day < 0) || (day > _maxdays_get(&wd->selected_time)))
     return 0;

   return day;
}

static void
_update_sel_it(Evas_Object *obj, int sel_it)
{
   int day;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->selection_enabled))
     return;

   day = _get_item_day(obj, sel_it);
   if (!day)
     return;

   _unselect(wd, wd->selected_it);

   wd->selected_it = sel_it;
   wd->selected_time.tm_mday = day;
   _select(wd, wd->selected_it);
   _fix_selected_time(wd);
   evas_object_smart_callback_call(obj, SIG_CHANGED, NULL);
}

static void
_day_selected(void *data, Evas_Object *obj __UNUSED__, const char *emission __UNUSED__, const char *source)
{
   int sel_it;
   Widget_Data *wd = elm_widget_data_get(data);
   if ((!wd) || (!wd->selection_enabled))
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
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return ECORE_CALLBACK_RENEW;

   if (wd->today_it > 0) _not_today(wd);

   current_time = time(NULL);
   localtime_r(&current_time, &wd->current_time);
   t = _time_to_next_day(&wd->current_time);
   ecore_timer_interval_set(wd->update_timer, t);

   if ((wd->current_time.tm_mon != wd->selected_time.tm_mon) ||
       (wd->current_time.tm_year!= wd->selected_time.tm_year))
     return ECORE_CALLBACK_RENEW;

   day = wd->current_time.tm_mday + wd->first_day_it - 1;
   _today(wd, day);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_event_hook(Evas_Object *obj, Evas_Object *src __UNUSED__, Evas_Callback_Type type, void *event_info)
{
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   Evas_Event_Key_Down *ev = event_info;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (!wd->selection_enabled) return EINA_FALSE;

   if ((!strcmp(ev->keyname, "Left")) ||
       (!strcmp(ev->keyname, "KP_Left")))
     {
        _update_sel_it(obj, wd->selected_it-1);
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            (!strcmp(ev->keyname, "KP_Right")))
     {
        _update_sel_it(obj, wd->selected_it+1);
     }
   else if ((!strcmp(ev->keyname, "Up"))  ||
            (!strcmp(ev->keyname, "KP_Up")))
     {
        _update_sel_it(obj, wd->selected_it-7);
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            (!strcmp(ev->keyname, "KP_Down")))
     {
        _update_sel_it(obj, wd->selected_it+7);
     }
   else if ((!strcmp(ev->keyname, "Prior")) ||
            (!strcmp(ev->keyname, "KP_Prior")))
     {
        if (_update_month(obj, -1)) _populate(obj);
     }
   else if ((!strcmp(ev->keyname, "Next")) ||
            (!strcmp(ev->keyname, "KP_Next")))
     {
        if (_update_month(obj, 1)) _populate(obj);
     }
   else return EINA_FALSE;

   return EINA_TRUE;
}

EAPI Evas_Object *
elm_calendar_add(Evas_Object *parent)
{
   time_t current_time;
   time_t weekday = 259200; /* Just the first sunday since epoch */
   Evas_Object *obj;
   Widget_Data *wd;
   int i, t;
   Evas *e;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "calendar");
   elm_widget_type_set(obj, "calendar");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_signal_emit_hook_set(obj, _signal_emit_hook);
   elm_widget_signal_callback_add_hook_set(obj, _signal_callback_add_hook);
   elm_widget_signal_callback_del_hook_set(obj, _signal_callback_del_hook);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_event_hook_set(obj, _event_hook);

   wd->first_interval = 0.85;
   wd->year_min = 2;
   wd->year_max = -1;
   wd->today_it = -1;
   wd->selected_it = -1;
   wd->first_day_it = -1;
   wd->selection_enabled = EINA_TRUE;
   wd->format_func = _format_month_year;
   wd->marks = NULL;

   wd->calendar = edje_object_add(e);
   _elm_theme_object_set(obj, wd->calendar, "calendar", "base", "default");
   elm_widget_resize_object_set(obj, wd->calendar);

   edje_object_signal_callback_add(wd->calendar, "elm,action,increment,start",
                                   "*", _button_inc_start, obj);
   edje_object_signal_callback_add(wd->calendar, "elm,action,decrement,start",
                                   "*", _button_dec_start, obj);
   edje_object_signal_callback_add(wd->calendar, "elm,action,stop",
                                   "*", _button_stop, obj);
   edje_object_signal_callback_add(wd->calendar, "elm,action,selected",
                                   "*", _day_selected, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   for (i = 0; i < 7; i++)
     {
        /* FIXME: I'm not aware of a known max, so if it fails,
         * just make it larger. :| */
        char buf[20];
        /* I don't know of a better way of doing it */
        if (strftime(buf, sizeof(buf), "%a", gmtime(&weekday)))
          {
             wd->weekdays[i] = eina_stringshare_add(buf);
          }
        else
          {
             /* If we failed getting day, get a default value */
             wd->weekdays[i] = _days_abbrev[i];
             WRN("Failed getting weekday name for '%s' from locale.",
                 _days_abbrev[i]);
          }
        weekday += 86400; /* Advance by a day */
     }

   current_time = time(NULL);
   localtime_r(&current_time, &wd->selected_time);
   wd->current_time = wd->selected_time;
   t = _time_to_next_day(&wd->current_time);
   wd->update_timer = ecore_timer_add(t, _update_cur_date, obj);

   _set_headers(obj);
   _populate(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_calendar_weekdays_names_set(Evas_Object *obj, const char *weekdays[])
{
   int i;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   EINA_SAFETY_ON_NULL_RETURN(weekdays);

   for (i = 0; i < 7; i++)
     {
        eina_stringshare_replace(&wd->weekdays[i], weekdays[i]);
     }
   _set_headers(obj);
}

EAPI const char **
elm_calendar_weekdays_names_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->weekdays;
}

EAPI void
elm_calendar_interval_set(Evas_Object *obj, double interval)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->first_interval = interval;
}

EAPI double
elm_calendar_interval_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return 0.0;
   return wd->first_interval;
}

EAPI void
elm_calendar_min_max_year_set(Evas_Object *obj, int min, int max)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   min -= 1900;
   max -= 1900;
   if ((wd->year_min == min) && (wd->year_max == max)) return;
   wd->year_min = min > 2 ? min : 2;
   wd->year_max = max;
   if ((max >= wd->year_min) && (wd->selected_time.tm_year > wd->year_max))
     wd->selected_time.tm_year = wd->year_max;
   if (wd->selected_time.tm_year < wd->year_min)
     wd->selected_time.tm_year = wd->year_min;
   _fix_selected_time(wd);
   _populate(obj);
}

EAPI void
elm_calendar_min_max_year_get(const Evas_Object *obj, int *min, int *max)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (min) *min = wd->year_min + 1900;
   if (max) *max = wd->year_max + 1900;
}

EAPI void
elm_calendar_day_selection_enabled_set(Evas_Object *obj, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->selection_enabled = enabled;
   if (enabled)
     _select(wd, wd->selected_it);
   else
     _unselect(wd, wd->selected_it);
}

EAPI Eina_Bool
elm_calendar_day_selection_enabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->selection_enabled;
}

EAPI void
elm_calendar_selected_time_set(Evas_Object *obj, struct tm *selected_time)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   EINA_SAFETY_ON_NULL_RETURN(selected_time);
   wd->selected_time = *selected_time;
   _populate(obj);
   return;
}

EAPI Eina_Bool
elm_calendar_selected_time_get(const Evas_Object *obj, struct tm *selected_time)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(selected_time, EINA_FALSE);
   *selected_time = wd->selected_time;
   return EINA_TRUE;
}

EAPI void
elm_calendar_format_function_set(Evas_Object *obj, char * (*format_function) (struct tm *selected_time))
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->format_func = format_function;
}

EAPI Elm_Calendar_Mark *
elm_calendar_mark_add(Evas_Object *obj, const char *mark_type, struct tm *mark_time, Elm_Calendar_Mark_Repeat repeat)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Calendar_Mark *mark;
   if (!wd) return NULL;

   mark = _mark_new(obj, mark_type, mark_time, repeat);
   wd->marks = eina_list_append(wd->marks, mark);
   mark->node = eina_list_last(wd->marks);
   return mark;
}

EAPI void
elm_calendar_mark_del(Elm_Calendar_Mark *mark)
{
   Evas_Object *obj;
   Widget_Data *wd;

   EINA_SAFETY_ON_NULL_RETURN(mark);

   obj = mark->obj;
   wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->marks = eina_list_remove_list(wd->marks, mark->node);
   _mark_free(mark);
}

EAPI void
elm_calendar_marks_clear(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   Elm_Calendar_Mark *mark;

   if (!wd) return;
   EINA_LIST_FREE(wd->marks, mark)
      _mark_free(mark);
}

EAPI const Eina_List *
elm_calendar_marks_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->marks;
}

EAPI void
elm_calendar_marks_draw(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _populate(obj);
}

EINA_DEPRECATED EAPI void
elm_calendar_text_saturday_color_set(Evas_Object *obj, int pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _text_day_color_set(wd, DAY_SATURDAY, pos);
}

EINA_DEPRECATED EAPI void
elm_calendar_text_sunday_color_set(Evas_Object *obj, int pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _text_day_color_set(wd, DAY_SUNDAY, pos);
}

EINA_DEPRECATED EAPI void
elm_calendar_text_weekday_color_set(Evas_Object *obj, int pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _text_day_color_set(wd, DAY_WEEKDAY, pos);
}
