#ifndef EFL_UI_CALENDAR_PRIVATE_H
#define EFL_UI_CALENDAR_PRIVATE_H

#include "Elementary.h"

/* DO NOT USE THIS HEADER UNLESS YOU ARE PREPARED FOR BREAKING OF YOUR
 * CODE. THIS IS ELEMENTARY'S INTERNAL WIDGET API (for now) AND IS NOT
 * FINAL. CALL elm_widget_api_check(ELM_INTERNAL_API_VERSION) TO CHECK
 * IT AT RUNTIME.
 */

/**
 * Base layout smart data extended with calendar instance data.
 */
typedef struct _Efl_Ui_Calendar_Data Efl_Ui_Calendar_Data;

struct _Efl_Ui_Calendar_Data
{
   Evas_Object             *obj; // the object itself
   double                   interval, first_interval;
   int                      spin_speed;
   int                      today_it, selected_it, focused_it;
   Ecore_Timer             *update_timer;
   const char              *weekdays[ELM_DAY_LAST];
   struct tm                current_date, shown_date, date, date_min, date_max;
   Evas_Object             *inc_btn_month;
   Evas_Object             *dec_btn_month;
   Evas_Object             *month_access;
   Eo                      *items[42];

   Efl_Ui_Calendar_Weekday  first_week_day;
   unsigned char            first_day_it;

   Eina_Bool                selected : 1;
   Eina_Bool                filling : 1;
   Eina_Bool                weekdays_set : 1;
};

#define EFL_UI_CALENDAR_DATA_GET(o, sd) \
  Efl_Ui_Calendar_Data * sd = efl_data_scope_get(o, EFL_UI_CALENDAR_CLASS)

#endif
