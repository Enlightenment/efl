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
     Efl_Time t;                                                     \
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
        if (pd->cur_time[TIMEPICKER_HOUR] >= 12)
          {
             //TODO: gets text from strftime.
             efl_text_set(pd->ampm, "PM");
             efl_ui_range_value_set(pd->hour, pd->cur_time[TIMEPICKER_HOUR] - 12);
          }
        else
          {
             efl_text_set(pd->ampm, "AM");
             efl_ui_range_value_set(pd->hour, pd->cur_time[TIMEPICKER_HOUR] + 12);
          }
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
        if (!pd->is_24hour && !strcmp(efl_text_get(pd->ampm), "PM"))
          pd->cur_time[TIMEPICKER_HOUR] += 12;
     }
   else if (ev->object == pd->min)
     pd->cur_time[TIMEPICKER_MIN] = efl_ui_range_value_get(pd->min);
   else
     {
        if (!strcmp(efl_text_get(pd->ampm), "PM"))
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
   efl_event_callback_call(data, EFL_UI_TIMEPICKER_EVENT_CHANGED, NULL);
}

static void
_fields_init(Eo *obj)
{
   const char *fmt;
   char ch;
   int i;
   int field = 0;
   char buf[FMT_LEN_MAX];

   Efl_Ui_Timepicker_Data *pd = efl_data_scope_get(obj, MY_CLASS);

   //Field create.
   pd->hour = efl_add(EFL_UI_SPIN_BUTTON_CLASS, obj,
                      efl_ui_range_min_max_set(efl_added, 1, 12),
                      efl_ui_spin_button_circulate_set(efl_added, EINA_TRUE),
                      efl_ui_spin_button_editable_set(efl_added, EINA_TRUE),
                      efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
                      efl_event_callback_add(efl_added, EFL_UI_SPIN_EVENT_CHANGED,_field_changed_cb, obj));

   pd->min = efl_add(EFL_UI_SPIN_BUTTON_CLASS, obj,
                     efl_ui_range_min_max_set(efl_added, 0, 59),
                     efl_ui_spin_button_circulate_set(efl_added, EINA_TRUE),
                     efl_ui_spin_button_editable_set(efl_added, EINA_TRUE),
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
                     efl_event_callback_add(efl_added, EFL_UI_SPIN_EVENT_CHANGED,_field_changed_cb, obj));

   pd->ampm = efl_add(EFL_UI_BUTTON_CLASS, obj,
                      efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _field_changed_cb, obj),
                      elm_widget_element_update(obj, efl_added, "button"));

   pd->dt_manager = efl_add(EFL_DATETIME_MANAGER_CLASS, obj);

   TIME_GET();

   pd->is_24hour = EINA_FALSE;

   _field_value_update(obj);

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
                       if (field == 0)
                         {
                            elm_object_signal_emit(obj, "efl,state,colon,visible,field1", "efl");
                            elm_object_signal_emit(obj, "efl,state,colon,invisible,field0", "efl");
                         }
                       else
                         {
                            elm_object_signal_emit(obj, "efl,state,colon,visible,field0", "efl");
                            elm_object_signal_emit(obj, "efl,state,colon,invisible,field1", "efl");
                         }

                       elm_layout_signal_emit(obj, "efl,state,ampm,visible", "efl");
                       edje_object_message_signal_process(elm_layout_edje_get(obj));
                       efl_content_set(efl_part(obj, buf), pd->ampm);
                    }

                  field++;
                  break;
               }
          }
        fmt++;
     }
}

EOLIAN static void
_efl_ui_timepicker_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Timepicker_Data *_pd EINA_UNUSED)
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

EOLIAN static Eo *
_efl_ui_timepicker_efl_object_constructor(Eo *obj, Efl_Ui_Timepicker_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "timepicker");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

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
_efl_ui_timepicker_ampm_set(Eo *obj, Efl_Ui_Timepicker_Data *pd, Eina_Bool is_24hour)
{
   if (pd->is_24hour == is_24hour) return;

   pd->is_24hour = is_24hour;
   if (pd->is_24hour == EINA_TRUE)
     elm_layout_signal_emit(obj, "efl,state,ampm,invisible", "efl");
   else
     elm_layout_signal_emit(obj, "efl,state,ampm,visible", "efl");
   _field_value_update(obj);
}

EOLIAN static Eina_Bool
_efl_ui_timepicker_ampm_get(const Eo *obj EINA_UNUSED, Efl_Ui_Timepicker_Data *pd)
{
   return pd->is_24hour;
}

#define EFL_UI_TIMEPICKER_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_timepicker), \

#include "efl_ui_timepicker.eo.c"
