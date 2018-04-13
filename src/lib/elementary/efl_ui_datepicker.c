#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_datepicker_private.h"

#define MY_CLASS EFL_UI_DATEPICKER_CLASS

#define MY_CLASS_NAME "Efl.Ui.Datepicker"

#define FMT_LEN_MAX 32

#define DATE_GET(obj)                                                \
   do {                                                              \
     Efl_Time t = efl_ui_datetime_value_get(obj);                    \
     sd->cur_date[DATEPICKER_YEAR] = t.tm_year + 1900;               \
     sd->cur_date[DATEPICKER_MONTH] = t.tm_mon + 1;                  \
     sd->cur_date[DATEPICKER_DAY] = t.tm_mday;                       \
   } while (0)

#define DATE_SET(obj)                                                \
   do {                                                              \
     Efl_Time t;                                                     \
     t.tm_year = sd->cur_date[DATEPICKER_YEAR] - 1900;               \
     t.tm_mon = sd->cur_date[DATEPICKER_MONTH] - 1;                  \
     t.tm_mday = sd->cur_date[DATEPICKER_DAY];                       \
     efl_ui_datetime_value_set(obj, t);                              \
   } while (0)

static const char *fmt_char[] = {"Yy", "mbBh", "de"};

static Eina_Bool
_validate_params(int year, int month, int day)
{
  if (year < 1900 || year > 2037 || month < 1 || month > 12 || day < 0 || day > 31)
    return EINA_FALSE;
  else return EINA_TRUE;
}

static Eina_Bool
_date_cmp(int time1[], int time2[])
{
   unsigned int idx;

   for (idx = 0; idx < EFL_UI_DATEPICKER_TYPE_COUNT; idx++)
     {
        if (time1[idx] != time2[idx])
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
_validate_date_limits(int time1[], int time2[], Eina_Bool swap)
{
   unsigned int idx;
   int *t1, *t2;

   t1 = (swap) ? time2 : time1;
   t2 = (swap) ? time1 : time2;

   for (idx = 0; idx < EFL_UI_DATEPICKER_TYPE_COUNT; idx++)
     {
        if (time1[idx] < time2[idx])
          {
             memcpy(t1, t2, (sizeof(int) * EFL_UI_DATEPICKER_TYPE_COUNT));
             return EINA_TRUE;
          }
        else if (time1[idx] > time2[idx])
          return EINA_FALSE;
     }

   return EINA_FALSE;
}

static int
_max_days_get(int year, int month)
{
   struct tm time1;
   time_t t;
   int day;

   t = time(NULL);
   localtime_r(&t, &time1);
   time1.tm_year = year;
   time1.tm_mon = month;
   for (day = 28; day <= 31;
        day++)
     {
        time1.tm_mday = day;
        mktime(&time1);
        /* To restrict month wrapping because of summer time in some locales,
         * ignore day light saving mode in mktime(). */
        time1.tm_isdst = -1;
        if (time1.tm_mday == 1) break;
     }
   day--;

   return day;
}

static void
_field_value_update(Eo *obj)
{
   Efl_Ui_Datepicker_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   efl_ui_range_value_set(sd->year, sd->cur_date[DATEPICKER_YEAR]);
   efl_ui_range_value_set(sd->month, sd->cur_date[DATEPICKER_MONTH]);
   efl_ui_range_value_set(sd->day, sd->cur_date[DATEPICKER_DAY]);

   DATE_SET(obj);
}

static void
_field_changed_cb(void *data, const Efl_Event *ev)
{
   int max_day;

   Efl_Ui_Datepicker_Data *sd = efl_data_scope_get(data, MY_CLASS);

   if (ev->object == sd->year)
     sd->cur_date[DATEPICKER_YEAR] = efl_ui_range_value_get(sd->year);
   else if (ev->object == sd->month)
     sd->cur_date[DATEPICKER_MONTH] = efl_ui_range_value_get(sd->month);
   else
     sd->cur_date[DATEPICKER_DAY] = efl_ui_range_value_get(sd->day);

   if (!(ev->object == sd->day))
     {
        max_day = _max_days_get(sd->cur_date[DATEPICKER_YEAR], sd->cur_date[DATEPICKER_MONTH]);
        efl_ui_range_min_max_set(sd->day, 1, max_day);
     }

   if (_validate_date_limits(sd->cur_date, sd->min_date, EINA_FALSE) ||
       _validate_date_limits(sd->max_date, sd->cur_date, EINA_TRUE))
     {
        _field_value_update(data);
        return;
     }

   DATE_SET(data);
   efl_event_callback_call(data, EFL_UI_DATEPICKER_EVENT_CHANGED, NULL);
}

static void
_fields_init(Eo *obj)
{
   const char *fmt;
   char ch;
   int i;
   int field = 0;
   char buf[FMT_LEN_MAX];

   Efl_Ui_Datepicker_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   //Field create.
   sd->year = efl_add(EFL_UI_SPIN_BUTTON_CLASS, obj,
                      efl_ui_range_min_max_set(efl_added, 1970, 2037),
                      efl_ui_spin_button_circulate_set(efl_added, EINA_TRUE),
                      efl_ui_spin_button_editable_set(efl_added, EINA_TRUE),
                      efl_event_callback_add(efl_added, EFL_UI_SPIN_EVENT_CHANGED,_field_changed_cb, obj),
                      elm_object_style_set(efl_added, "picker"));

   sd->month = efl_add(EFL_UI_SPIN_BUTTON_CLASS, obj,
                       efl_ui_range_min_max_set(efl_added, 1, 12),
                       efl_ui_spin_button_circulate_set(efl_added, EINA_TRUE),
                       efl_ui_spin_button_editable_set(efl_added, EINA_TRUE),
                       efl_event_callback_add(efl_added, EFL_UI_SPIN_EVENT_CHANGED,_field_changed_cb, obj),
                       elm_object_style_set(efl_added, "picker"));

   sd->day = efl_add(EFL_UI_SPIN_BUTTON_CLASS, obj,
                     efl_ui_range_min_max_set(efl_added, 1, 31),
                     efl_ui_spin_button_circulate_set(efl_added, EINA_TRUE),
                     efl_ui_spin_button_editable_set(efl_added, EINA_TRUE),
                     efl_event_callback_add(efl_added, EFL_UI_SPIN_EVENT_CHANGED,_field_changed_cb, obj),
                     elm_object_style_set(efl_added, "picker"));

   DATE_GET(obj);
   //Using system config?
   sd->min_date[DATEPICKER_YEAR] = 1970;
   sd->min_date[DATEPICKER_MONTH] = 1;
   sd->min_date[DATEPICKER_DAY] = 1;
   sd->max_date[DATEPICKER_YEAR] = 2037;
   sd->max_date[DATEPICKER_MONTH] = 12;
   sd->max_date[DATEPICKER_DAY] = 31;

   _field_value_update(obj);

   fmt = efl_ui_datetime_format_get(obj);
   if (!fmt)
     {
        ERR("Failed to get current format.");
        //Gives default format when the gets format failed.
        fmt = "%Y %b %d";
     }

   //Sort fields by format.
   while((ch = *fmt))
     {
        //TODO: ignore extensions and separators.
        for (i = 0; i < EFL_UI_DATEPICKER_TYPE_COUNT; i++)
          {
             if (strchr(fmt_char[i], ch))
               {
                  snprintf(buf, sizeof(buf), "field%d", field++);
                  if (i == DATEPICKER_YEAR)
                    efl_content_set(efl_part(obj, buf), sd->year);
                  else if (i == DATEPICKER_MONTH)
                    efl_content_set(efl_part(obj, buf), sd->month);
                  else
                    efl_content_set(efl_part(obj, buf), sd->day);

                  break;
               }
          }
        fmt++;
     }
}

EOLIAN static void
_efl_ui_datepicker_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Datepicker_Data *_pd EINA_UNUSED)
{
    Evas_Coord minw = -1, minh = -1;
    ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

    elm_coords_finger_size_adjust(1, &minw, 1, &minh);
    edje_object_size_min_restricted_calc
    (wd->resize_obj, &minw, &minh, minw, minh);
    elm_coords_finger_size_adjust(1, &minw, 1, &minh);
    evas_object_size_hint_min_set(obj, minw, minh);
    evas_object_size_hint_max_set(obj, -1, -1);
}

EOLIAN static Eina_Bool
_efl_ui_datepicker_efl_ui_widget_widget_event(Eo *obj, Efl_Ui_Datepicker_Data *sd, const Efl_Event *eo_event, Evas_Object *src EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eo *
_efl_ui_datepicker_efl_object_constructor(Eo *obj, Efl_Ui_Datepicker_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "datepicker");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   if (!elm_widget_theme_object_set(obj, wd->resize_obj,
                                    elm_widget_theme_klass_get(obj),
                                    elm_widget_theme_element_get(obj),
                                    elm_widget_theme_style_get(obj)))
     CRI("Failed to set layout!");

   _fields_init(obj);

   elm_widget_sub_object_parent_add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   return obj;
}

EOLIAN static void
_efl_ui_datepicker_efl_object_destructor(Eo *obj, Efl_Ui_Datepicker_Data *sd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_datepicker_efl_ui_date_min_set(Eo *obj, Efl_Ui_Datepicker_Data *sd EINA_UNUSED, int year, int month, int day)
{
   int new_time[EFL_UI_DATEPICKER_TYPE_COUNT] = {year, month, day};

   if (!_validate_params(year, month, day)) return;
   if (_date_cmp(sd->min_date, new_time)) return;

   memcpy(sd->min_date, new_time, (sizeof(int) * EFL_UI_DATEPICKER_TYPE_COUNT));

   _validate_date_limits(sd->max_date, sd->min_date, EINA_FALSE);
   _validate_date_limits(sd->cur_date, sd->min_date, EINA_FALSE);

   DATE_SET(obj);
   _field_value_update(obj);
}

EOLIAN static void
_efl_ui_datepicker_efl_ui_date_min_get(Eo *obj EINA_UNUSED, Efl_Ui_Datepicker_Data *sd, int *year, int *month, int *day)
{
   *year = sd->min_date[DATEPICKER_YEAR];
   *month = sd->min_date[DATEPICKER_MONTH];
   *day = sd->min_date[DATEPICKER_DAY];
}

EOLIAN static void
_efl_ui_datepicker_efl_ui_date_max_set(Eo *obj, Efl_Ui_Datepicker_Data *sd EINA_UNUSED, int year, int month, int day)
{
   int new_time[EFL_UI_DATEPICKER_TYPE_COUNT] = {year, month, day};

   if (!_validate_params(year, month, day)) return;
   if (_date_cmp(sd->max_date, new_time)) return;

   memcpy(sd->max_date, new_time, (sizeof(int) * EFL_UI_DATEPICKER_TYPE_COUNT));

   _validate_date_limits(sd->max_date, sd->min_date, EINA_TRUE);
   _validate_date_limits(sd->max_date, sd->cur_date, EINA_TRUE);

   DATE_SET(obj);
   _field_value_update(obj);
}

EOLIAN static void
_efl_ui_datepicker_efl_ui_date_max_get(Eo *obj EINA_UNUSED, Efl_Ui_Datepicker_Data *sd, int *year, int *month, int *day)
{
   *year = sd->max_date[DATEPICKER_YEAR];
   *month = sd->max_date[DATEPICKER_MONTH];
   *day = sd->max_date[DATEPICKER_DAY];
}

EOLIAN static void
_efl_ui_datepicker_efl_ui_date_date_set(Eo *obj, Efl_Ui_Datepicker_Data *sd, int year, int month, int day)
{
   int new_time[EFL_UI_DATEPICKER_TYPE_COUNT] = {year, month, day};

   if (!_validate_params(year, month, day)) return;
   if (_date_cmp(sd->cur_date, new_time)) return;

   memcpy(sd->cur_date, new_time, (sizeof(int) * EFL_UI_DATEPICKER_TYPE_COUNT));

   _validate_date_limits(sd->cur_date, sd->min_date, EINA_FALSE);
   _validate_date_limits(sd->max_date, sd->cur_date, EINA_TRUE);

   DATE_SET(obj);
   _field_value_update(obj);
}

EOLIAN static void
_efl_ui_datepicker_efl_ui_date_date_get(Eo *obj EINA_UNUSED, Efl_Ui_Datepicker_Data *sd, int *year, int *month, int *day)
{
   *year = sd->cur_date[DATEPICKER_YEAR];
   *month = sd->cur_date[DATEPICKER_MONTH];
   *day = sd->cur_date[DATEPICKER_DAY];
}

#define EFL_UI_DATEPICKER_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_datepicker), \

#include "efl_ui_datepicker.eo.c"
