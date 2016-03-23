#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

struct _api_data
{
   unsigned int state;  /* What state we are testing       */
   void *box;           /* box used in set_api_state       */
};
typedef struct _api_data api_data;

enum _api_state
{
   STATE_MARK_MONTHLY,
   STATE_MARK_WEEKLY,
   STATE_SUNDAY_HIGHLIGHT,
   STATE_SELECT_DATE_DISABLED_WITH_MARKS,
   STATE_SELECT_DATE_DISABLED_NO_MARKS,
   API_STATE_LAST
};
typedef enum _api_state api_state;

#define SEC_PER_DAY   24 * 60 * 60
#define SEC_PER_YEAR  365 * SEC_PER_DAY

static void
set_api_state(api_data *api)
{
   const Eina_List *items = elm_box_children_get(api->box);
   static Elm_Calendar_Mark *m = NULL;
   if (!eina_list_count(items))
     return;

   switch(api->state)
     { /* Put all api-changes under switch */
      case STATE_MARK_MONTHLY:
           {
              Evas_Object *cal = eina_list_nth(items, 0);
              time_t the_time = (SEC_PER_YEAR * 41) + (SEC_PER_DAY * 9); /* Set date to DEC 31, 2010 */
              elm_calendar_min_max_year_set(cal, 2010, 2011);
              m = elm_calendar_mark_add(cal, "checked", gmtime(&the_time), ELM_CALENDAR_MONTHLY);
              elm_calendar_selected_time_set(cal, gmtime(&the_time));
           }
         break;
      case STATE_MARK_WEEKLY:
           {
              Evas_Object *cal = eina_list_nth(items, 0);
              time_t the_time = (SEC_PER_YEAR * 41) + (SEC_PER_DAY * 4); /* Set date to DEC 26, 2010 */
              elm_calendar_mark_del(m);
              m = elm_calendar_mark_add(cal, "checked", gmtime(&the_time), ELM_CALENDAR_WEEKLY);
              elm_calendar_selected_time_set(cal, gmtime(&the_time));
           }
         break;
      case STATE_SUNDAY_HIGHLIGHT:
           {
              Evas_Object *cal = eina_list_nth(items, 0);
              time_t the_time = (SEC_PER_YEAR * 41) + (SEC_PER_DAY * 3); /* Set date to DEC 25, 2010 */
              /* elm_calendar_mark_del(m); */
              m = elm_calendar_mark_add(cal, "holiday", gmtime(&the_time), ELM_CALENDAR_WEEKLY);
              elm_calendar_selected_time_set(cal, gmtime(&the_time));
           }
         break;
      case STATE_SELECT_DATE_DISABLED_WITH_MARKS:
           {
              Evas_Object *cal = eina_list_nth(items, 0);
              time_t the_time = (SEC_PER_YEAR * 41) + (SEC_PER_DAY * 10); /* Set date to JAN 01, 2011 */
              elm_calendar_select_mode_set(cal, ELM_CALENDAR_SELECT_MODE_NONE);
              elm_calendar_selected_time_set(cal, gmtime(&the_time));
           }
         break;
      case STATE_SELECT_DATE_DISABLED_NO_MARKS:
           {
              Evas_Object *cal = eina_list_nth(items, 0);
              time_t the_time = (SEC_PER_YEAR * 41) + (SEC_PER_DAY * 40); /* Set date to FEB 01, 2011 */
              elm_calendar_marks_clear(cal);
              elm_calendar_select_mode_set(cal, ELM_CALENDAR_SELECT_MODE_NONE);
              elm_calendar_selected_time_set(cal, gmtime(&the_time));
           }
         break;
      case API_STATE_LAST:
         break;
      default:
         return;
     }
}

static void
_api_bt_clicked(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{  /* Will add here a SWITCH command containing code to modify test-object */
   /* in accordance a->state value. */
   api_data *a = data;
   char str[128];

   printf("clicked event on API Button: api_state=<%d>\n", a->state);
   set_api_state(a);
   a->state++;
   sprintf(str, "Next API function (%u)", a->state);
   elm_object_text_set(obj, str);
   elm_object_disabled_set(obj, a->state == API_STATE_LAST);
}

static void
_cleanup_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   free(data);
}

/* A simple test, just displaying calendar in it's default state */
void
test_calendar(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *cal, *bx, *bxx, *bt;
   api_data *api = calloc(1, sizeof(api_data));

   win = elm_win_util_standard_add("calendar", "Calendar");
   elm_win_autodel_set(win, EINA_TRUE);
   evas_object_event_callback_add(win, EVAS_CALLBACK_FREE, _cleanup_cb, api);

   bxx = elm_box_add(win);
   evas_object_size_hint_weight_set(bxx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bxx);
   evas_object_show(bxx);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   api->box = bx;
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Next API function");
   evas_object_smart_callback_add(bt, "clicked", _api_bt_clicked, (void *) api);
   elm_box_pack_end(bxx, bt);
   elm_object_disabled_set(bt, api->state == API_STATE_LAST);
   evas_object_show(bt);

   elm_box_pack_end(bxx, bx);

   cal = elm_calendar_add(win);
   elm_calendar_first_day_of_week_set(cal, ELM_DAY_MONDAY);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, cal);

   time_t the_time = (SEC_PER_YEAR * 41) + (SEC_PER_DAY * 9); /* Set date to DEC 31, 2010 */
   elm_calendar_selected_time_set(cal, gmtime(&the_time));
   elm_calendar_min_max_year_set(cal, 2010, 2012);

   evas_object_show(cal);

   evas_object_show(win);
}

void
_print_cal_info(Evas_Object *cal, Evas_Object *en)
{
   char info[1024];
   double interval;
   int year_min, year_max;
   Eina_Bool sel_enabled;
   const char **wds;
   struct tm stm;

   if (!elm_calendar_selected_time_get(cal, &stm))
     return;

   interval = elm_calendar_interval_get(cal);
   elm_calendar_min_max_year_get(cal, &year_min, &year_max);
   sel_enabled = !!(elm_calendar_select_mode_get(cal) != ELM_CALENDAR_SELECT_MODE_NONE);
   wds = elm_calendar_weekdays_names_get(cal);

   snprintf(info, sizeof(info),
            "  Day: %i, Mon: %i, Year %i, WeekDay: %i<br/>"
            "  Interval: %0.2f, Year_Min: %i, Year_Max %i, Sel Enabled : %i<br/>"
            "  Weekdays: %s, %s, %s, %s, %s, %s, %s<br/>",
            stm.tm_mday, stm.tm_mon, stm.tm_year + 1900, stm.tm_wday,
            interval, year_min, year_max, sel_enabled,
            wds[0], wds[1], wds[2], wds[3], wds[4], wds[5], wds[6]);

   elm_object_text_set(en, info);
}

void
_print_cal_shown_info(Evas_Object *cal, Evas_Object *en)
{
   char info[1024];
   struct tm stm;

   elm_calendar_displayed_time_get(cal, &stm);
   snprintf(info, sizeof(info),
            "  Mon: %i, Year %i",
            stm.tm_mon, stm.tm_year + 1900);

   elm_object_text_set(en, info);
}

void
_print_cal_shown_info_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   _print_cal_shown_info(obj, data);
}

static void
_print_cal_info_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   _print_cal_info(obj, data);
}

static char *
_format_month_year(struct tm *stm)
{
   char buf[32];
   if (!strftime(buf, sizeof(buf), "%b %y", stm)) return NULL;
   return strdup(buf);
}

static Evas_Object *
_calendar_create(Evas_Object *parent)
{
   Evas_Object *cal;
   Elm_Calendar_Mark *mark;
   struct tm selected_time;
   time_t current_time;
   const char *weekdays[] =
   {
      "Sunday", "Monday", "Tuesday", "Wednesday",
      "Thursday", "Friday", "Saturday"
   };

   cal = elm_calendar_add(parent);
   elm_calendar_weekdays_names_set(cal, weekdays); // FIXME: this causes smart cal recalc loop error
   elm_calendar_first_day_of_week_set(cal, ELM_DAY_SATURDAY);
   elm_calendar_interval_set(cal, 0.4);
   elm_calendar_format_function_set(cal, _format_month_year);
   elm_calendar_min_max_year_set(cal, 2010, 2020);

   current_time = time(NULL) + 4 * SEC_PER_DAY;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "holiday", &selected_time, ELM_CALENDAR_ANNUALLY);

   current_time = time(NULL) + 1 * SEC_PER_DAY;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "checked", &selected_time, ELM_CALENDAR_UNIQUE);

   current_time = time(NULL) - 363 * SEC_PER_DAY;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "checked", &selected_time, ELM_CALENDAR_MONTHLY);

   current_time = time(NULL) - 5 * SEC_PER_DAY;
   localtime_r(&current_time, &selected_time);
   mark = elm_calendar_mark_add(cal, "holiday", &selected_time,
                                ELM_CALENDAR_WEEKLY);

   current_time = time(NULL) + 1 * SEC_PER_DAY;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal, "holiday", &selected_time, ELM_CALENDAR_WEEKLY);

   elm_calendar_mark_del(mark);
   elm_calendar_marks_draw(cal);

   return cal;
}

/* A test intended to cover all the calendar api and much use cases as
   possible */
void
test_calendar2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *bxh, *cal, *cal2, *cal3, *en;
   struct tm selected_time;
   time_t current_time;

   win = elm_win_util_standard_add("calendar2", "Calendar 2");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   bxh = elm_box_add(win);
   elm_box_horizontal_set(bxh, EINA_TRUE);
   evas_object_size_hint_weight_set(bxh, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bxh, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(bxh);
   elm_box_pack_end(bx, bxh);

   // calendar 1
   cal = _calendar_create(win);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cal, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(cal);
   elm_box_pack_end(bx, cal);

   en = elm_entry_add(win);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);
   elm_entry_editable_set(en, EINA_FALSE);

   _print_cal_info(cal, en);
   evas_object_smart_callback_add(cal, "changed", _print_cal_info_cb, en);

   // calendar 2
   cal2 = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cal2, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_calendar_select_mode_set(cal2, ELM_CALENDAR_SELECT_MODE_NONE);
   evas_object_show(cal2);
   elm_box_pack_end(bxh, cal2);

   // calendar 3
   cal3 = elm_calendar_add(win);
   evas_object_size_hint_weight_set(cal3, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(cal3, EVAS_HINT_FILL, EVAS_HINT_FILL);
   current_time = time(NULL) + 34 * SEC_PER_DAY;
   localtime_r(&current_time, &selected_time);
   elm_calendar_selected_time_set(cal3, &selected_time);
   current_time = time(NULL) + 1 * SEC_PER_DAY;
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal3, "checked", &selected_time, ELM_CALENDAR_UNIQUE);
   elm_calendar_marks_clear(cal3);
   current_time = time(NULL);
   localtime_r(&current_time, &selected_time);
   elm_calendar_mark_add(cal3, "checked", &selected_time, ELM_CALENDAR_DAILY);
   elm_calendar_mark_add(cal3, "holiday", &selected_time, ELM_CALENDAR_DAILY);
   elm_calendar_marks_draw(cal3);
   evas_object_show(cal3);
   elm_box_pack_end(bxh, cal3);
   elm_calendar_min_max_year_set(cal3, -1, -1);

   evas_object_show(win);
}

void
test_calendar3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *cal, *en, *bx;
   struct tm selected_time;
   time_t current_time;

   win = elm_win_util_standard_add("calendar", "Calendar");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(en);
   elm_box_pack_end(bx, en);

   elm_entry_editable_set(en, EINA_FALSE);
   cal = elm_calendar_add(win);
   elm_object_style_set(cal, "double_spinners");
   elm_calendar_first_day_of_week_set(cal, ELM_DAY_THURSDAY);
   elm_calendar_select_mode_set(cal, ELM_CALENDAR_SELECT_MODE_ONDEMAND);
   elm_calendar_selectable_set(cal,
                               (ELM_CALENDAR_SELECTABLE_YEAR
                                | ELM_CALENDAR_SELECTABLE_MONTH));
   current_time = time(NULL) + 34 * SEC_PER_DAY;
   localtime_r(&current_time, &selected_time);
   evas_object_size_hint_weight_set(cal, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx, cal);

   evas_object_show(cal);
   elm_calendar_selected_time_set(cal, &selected_time);
   _print_cal_shown_info(cal, en);
   evas_object_smart_callback_add(cal, "display,changed", _print_cal_shown_info_cb, en);

   evas_object_show(win);
}
