#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_timepicker_private.h"

#define MY_CLASS EFL_UI_TIMEPICKER_CLASS

#define MY_CLASS_NAME "Efl.Ui.Timepicker"

#define FMT_LEN_MAX 32

#define TIME_GET()                                                   \
   do {                                                              \
     Efl_Time t = efl_datetime_manager_value_get(pd->dt_manager);    \
     pd->cur_time[TIMEPICKER_HOUR] = t.tm_hour;                      \
     pd->cur_time[TIMEPICKER_MIN] = t.tm_min;                        \
   } while (0)

#define TIME_SET()                                                   \
   do {                                                              \
     Efl_Time t = { 0 };                                             \
     t.tm_hour = pd->cur_time[TIMEPICKER_HOUR];                      \
     t.tm_min = pd->cur_time[TIMEPICKER_MIN];                        \
     efl_datetime_manager_value_set(pd->dt_manager, t);              \
   } while (0)

static const char *fmt_char[] = {"IHkl", "M", "Aa"};

static Eina_Bool
_validate_params(int hour, int min)
{
  if (hour < 0 || hour > 23 || min < 0 || min > 59)
    return EINA_FALSE;
  else return EINA_TRUE;
}

static Eina_Bool
_time_cmp(int time1[], int time2[])
{
   unsigned int idx;

   for (idx = 0; idx < EFL_UI_TIMEPICKER_TYPE_COUNT -1; idx++)
     {
        if (time1[idx] != time2[idx])
          return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_field_value_update(Eo *obj)
{
   Efl_Ui_Timepicker_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   if (!pd->is_24hour)
     {
        if (pd->cur_time[TIMEPICKER_HOUR] > 12)
          {
             //TODO: gets text from strftime.
             efl_text_set(pd->ampm, "PM");
             efl_ui_range_value_set(pd->hour, pd->cur_time[TIMEPICKER_HOUR] - 12);
          }
        else
          {
             efl_text_set(pd->ampm, "AM");
             efl_ui_range_value_set(pd->hour, pd->cur_time[TIMEPICKER_HOUR]);
          }
     }
   else
     {
        efl_ui_range_value_set(pd->hour, pd->cur_time[TIMEPICKER_HOUR]);
     }

   efl_ui_range_value_set(pd->min, pd->cur_time[TIMEPICKER_MIN]);

   TIME_SET();
}

static void
_field_changed_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Timepicker_Data *pd = efl_data_scope_get(data, MY_CLASS);

   //TODO: hour value increase when min reached max.
   if (ev->object == pd->hour)
     {
        pd->cur_time[TIMEPICKER_HOUR] = efl_ui_range_value_get(pd->hour);
        if (!pd->is_24hour && eina_streq(efl_text_get(pd->ampm), "PM"))
          pd->cur_time[TIMEPICKER_HOUR] += 12;
     }
   else if (ev->object == pd->min)
     pd->cur_time[TIMEPICKER_MIN] = efl_ui_range_value_get(pd->min);
   else
     {
        if (eina_streq(efl_text_get(pd->ampm), "PM"))
          {
             efl_text_set(pd->ampm, "AM");
             pd->cur_time[TIMEPICKER_HOUR] -= 12;
          }
        else
          {
             efl_text_set(pd->ampm, "PM");
             pd->cur_time[TIMEPICKER_HOUR] += 12;
          }
     }

   TIME_SET();
   efl_event_callback_call(data, EFL_UI_TIMEPICKER_EVENT_TIME_CHANGED, NULL);
}

static void
_fields_init(Eo *obj)
{
   Efl_Ui_Timepicker_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   //Field create.
   pd->hour = efl_add(EFL_UI_SPIN_BUTTON_CLASS, obj,
                      efl_ui_range_limits_set(efl_added, 1, 12),
                      efl_ui_spin_button_wraparound_set(efl_added, EINA_TRUE),
                      efl_ui_spin_button_direct_text_input_set(efl_added, EINA_TRUE),
                      efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
                      efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED,_field_changed_cb, obj));

   pd->min = efl_add(EFL_UI_SPIN_BUTTON_CLASS, obj,
                     efl_ui_range_limits_set(efl_added, 0, 59),
                     efl_ui_spin_button_wraparound_set(efl_added, EINA_TRUE),
                     efl_ui_spin_button_direct_text_input_set(efl_added, EINA_TRUE),
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
                     efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED,_field_changed_cb, obj));

   pd->ampm = efl_add(EFL_UI_BUTTON_CLASS, obj,
                      efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _field_changed_cb, obj),
                      elm_widget_element_update(obj, efl_added, "button"));

   pd->dt_manager = efl_add(EFL_DATETIME_MANAGER_CLASS, obj);

   TIME_GET();

   pd->is_24hour = EINA_FALSE;

   _field_value_update(obj);

}


EOLIAN static Eina_Error
_efl_ui_timepicker_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Timepicker_Data *pd)
{
   const char *fmt;
   char ch;
   int i;
   char buf[FMT_LEN_MAX];
   int field = 0;
   Eina_Error ret = EFL_UI_THEME_APPLY_ERROR_NONE;

   ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));

   if (ret != EFL_UI_THEME_APPLY_ERROR_NONE)
     goto end;


   fmt = efl_datetime_manager_format_get(pd->dt_manager);
   if (!fmt)
     {
        ERR("Failed to get current format.");
        //Gives default format when the gets format failed.
        fmt = "%H:%M %a";
     }

   //Sort fields by format.
   while((ch = *fmt))
     {
        //TODO: ignore extensions and separators.
        for (i = 0; i < EFL_UI_TIMEPICKER_TYPE_COUNT; i++)
          {
             if (strchr(fmt_char[i], ch))
               {
                  snprintf(buf, sizeof(buf), "efl.field%d", field);
                  if (i == TIMEPICKER_HOUR)
                    efl_content_set(efl_part(obj, buf), pd->hour);
                  else if (i == TIMEPICKER_MIN)
                    efl_content_set(efl_part(obj, buf), pd->min);
                  else
                    {
                       //TODO: monitoring locale change and update field location.
                       //FIXME: disabled this code, as it caused issues, see T8546
                       /*if (field == 0)
                         {
                            elm_object_signal_emit(obj, "efl,colon_field1,visible,on", "efl");
                            elm_object_signal_emit(obj, "efl,colon_field0,visible,off", "efl");
                         }
                       else
                         {
                            elm_object_signal_emit(obj, "efl,colon_field0,visible,on", "efl");
                            elm_object_signal_emit(obj, "efl,colon_field1,visible,off", "efl");
                         }

                       if (pd->is_24hour)
                         elm_layout_signal_emit(obj, "efl,ampm,visible,off", "efl");
                       else
                         elm_layout_signal_emit(obj, "efl,ampm,visible,on", "efl");
                       edje_object_message_signal_process(elm_layout_edje_get(obj));*/
                       efl_content_set(efl_part(obj, buf), pd->ampm);
                    }

                  field++;
                  break;
               }
          }
        fmt++;
     }
end:
   return ret;
}



EOLIAN static Eo *
_efl_ui_timepicker_efl_object_constructor(Eo *obj, Efl_Ui_Timepicker_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "timepicker");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   _fields_init(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   return obj;
}

EOLIAN static void
_efl_ui_timepicker_efl_object_destructor(Eo *obj, Efl_Ui_Timepicker_Data *pd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_timepicker_time_set(Eo *obj, Efl_Ui_Timepicker_Data *pd, int hour, int min)
{
   int new_time[EFL_UI_TIMEPICKER_TYPE_COUNT - 1] = {hour, min};

   if (!_validate_params(hour, min)) return;
   if (_time_cmp(pd->cur_time, new_time)) return;

   memcpy(pd->cur_time, new_time, (sizeof(int) * (EFL_UI_TIMEPICKER_TYPE_COUNT -1)));

   TIME_SET();
   _field_value_update(obj);
}

EOLIAN static void
_efl_ui_timepicker_time_get(const Eo *obj EINA_UNUSED, Efl_Ui_Timepicker_Data *pd, int *hour, int *min)
{
   *hour = pd->cur_time[TIMEPICKER_HOUR];
   *min = pd->cur_time[TIMEPICKER_MIN];
}

EOLIAN static void
_efl_ui_timepicker_is_24hour_set(Eo *obj, Efl_Ui_Timepicker_Data *pd, Eina_Bool is_24hour)
{
   if (pd->is_24hour == is_24hour) return;

   pd->is_24hour = is_24hour;
   if (!pd->is_24hour)
     {
        efl_ui_widget_disabled_set(pd->ampm, EINA_FALSE);
        efl_ui_range_limits_set(pd->hour, 1, 12);
     }
   else
     {
        efl_ui_widget_disabled_set(pd->ampm, EINA_TRUE);
        efl_ui_range_limits_set(pd->hour, 0, 23);
     }

   _field_value_update(obj);
}

EOLIAN static Eina_Bool
_efl_ui_timepicker_is_24hour_get(const Eo *obj EINA_UNUSED, Efl_Ui_Timepicker_Data *pd)
{
   return pd->is_24hour;
}

#include "efl_ui_timepicker.eo.c"
