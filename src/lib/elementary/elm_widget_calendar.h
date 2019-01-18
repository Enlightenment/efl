#ifndef ELM_WIDGET_CALENDAR_H
#define ELM_WIDGET_CALENDAR_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * @addtogroup Widget
 * @{
 *
 * @section elm-calendar-class The Elementary Calendar Class
 *
 * Elementary, besides having the @ref Calendar widget, exposes its
 * foundation -- the Elementary Calendar Class -- in order to create other
 * widgets which are a calendar with some more logic on top.
 */

/**
 * Base layout smart data extended with calendar instance data.
 */
typedef struct _Elm_Calendar_Data Elm_Calendar_Data;

typedef enum _Day_Color // EINA_DEPRECATED
{
   DAY_WEEKDAY = 0,
   DAY_SATURDAY = 1,
   DAY_SUNDAY = 2
} Day_Color;

struct _Elm_Calendar_Data
{
   Evas_Object             *obj; // the object itself
   Eina_List               *marks;
   double                   interval, first_interval;
   int                      spin_speed;
   int                      today_it, selected_it, focused_it;
   Ecore_Timer             *update_timer, *spin_timer;
   Elm_Calendar_Format_Cb   format_func;
   const char              *weekdays[ELM_DAY_LAST];
   struct tm                current_time, selected_time, shown_time, date_min, date_max;
   Day_Color                day_color[42]; // EINA_DEPRECATED
   Evas_Object             *inc_btn_month;
   Evas_Object             *dec_btn_month;
   Evas_Object             *month_access;
   Evas_Object             *inc_btn_year;
   Evas_Object             *dec_btn_year;
   Evas_Object             *year_access;
   Eo                      *items[42];

   Elm_Calendar_Weekday     first_week_day;
   Elm_Calendar_Select_Mode select_mode;
   Elm_Calendar_Selectable  selectable;

   unsigned char            first_day_it;

   Eina_Bool                selected : 1;
   Eina_Bool                double_spinners : 1;
   Eina_Bool                filling : 1;
   Eina_Bool                weekdays_set : 1;
   Eina_Bool                month_btn_clicked : 1;
};

struct _Elm_Calendar_Mark
{
   Evas_Object                  *obj;
   Eina_List                    *node;
   struct tm                     mark_time;
   const char                   *mark_type;
   Elm_Calendar_Mark_Repeat_Type repeat;
};

/**
 * @}
 */

#define ELM_CALENDAR_DATA_GET(o, sd) \
  Elm_Calendar_Data * sd = efl_data_scope_get(o, ELM_CALENDAR_CLASS)

#define ELM_CALENDAR_CHECK(obj)                              \
  if (EINA_UNLIKELY(!efl_isa((obj), ELM_CALENDAR_CLASS))) \
    return

#endif
