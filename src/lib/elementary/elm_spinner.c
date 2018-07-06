#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#define EFL_ACCESS_VALUE_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_UI_FOCUS_COMPOSITION_PROTECTED

#include <Elementary.h>
#include <ctype.h>

#include "elm_priv.h"
#include "elm_widget_spinner.h"
#include "elm_entry.eo.h"

#include "Eo.h"

#define MY_CLASS ELM_SPINNER_CLASS

#define MY_CLASS_NAME "Elm_Spinner"
#define MY_CLASS_NAME_LEGACY "elm_spinner"

#define ELM_SPINNER_DELAY_CHANGE_TIME 0.2

static const char SIG_CHANGED[] = "changed";
static const char SIG_DRAG_START[] = "spinner,drag,start";
static const char SIG_DRAG_STOP[] = "spinner,drag,stop";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const char SIG_MIN_REACHED[] = "min,reached";
static const char SIG_MAX_REACHED[] = "max,reached";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_DELAY_CHANGED, ""},
   {SIG_DRAG_START, ""},
   {SIG_DRAG_STOP, ""},
   {SIG_MIN_REACHED, ""},
   {SIG_MAX_REACHED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static Eina_Bool _key_action_toggle(Evas_Object *obj, const char *params);

static void
_inc_dec_button_clicked_cb(void *data, const Efl_Event *event);
static void
_inc_dec_button_pressed_cb(void *data, const Efl_Event *event);
static void
_inc_dec_button_unpressed_cb(void *data, const Efl_Event *event);
static void
_inc_dec_button_mouse_move_cb(void *data, const Efl_Event *event);
static void
_entry_focus_change(void *data, const Efl_Event *event);
static void
_entry_activated_cb(void *data, const Efl_Event *event);

EFL_CALLBACKS_ARRAY_DEFINE(_inc_dec_button_cb,
   { EFL_UI_EVENT_CLICKED, _inc_dec_button_clicked_cb},
   { EFL_UI_EVENT_PRESSED, _inc_dec_button_pressed_cb},
   { EFL_UI_EVENT_UNPRESSED, _inc_dec_button_unpressed_cb},
   { EFL_EVENT_POINTER_MOVE, _inc_dec_button_mouse_move_cb }
);

static void _access_increment_decrement_info_say(Evas_Object *obj,
                                                 Eina_Bool is_incremented);

static Eina_Bool
_is_valid_digit(char x)
{
   return ((x >= '0' && x <= '9') || (x == '.')) ? EINA_TRUE : EINA_FALSE;
}

static Elm_Spinner_Format_Type
_is_label_format_integer(const char *fmt)
{
   const char *itr = NULL;
   const char *start = NULL;
   Eina_Bool found = EINA_FALSE;
   Elm_Spinner_Format_Type ret_type = SPINNER_FORMAT_INVALID;

   start = strchr(fmt, '%');
   if (!start) return SPINNER_FORMAT_INVALID;

   while (start)
     {
        if (found && start[1] != '%')
          {
             return SPINNER_FORMAT_INVALID;
          }

        if (start[1] != '%' && !found)
          {
             found = EINA_TRUE;
             for (itr = start + 1; *itr != '\0'; itr++)
               {
                  if ((*itr == 'd') || (*itr == 'u') || (*itr == 'i') ||
                      (*itr == 'o') || (*itr == 'x') || (*itr == 'X'))
                    {
                       ret_type = SPINNER_FORMAT_INT;
                       break;
                    }
                  else if ((*itr == 'f') || (*itr == 'F'))
                    {
                       ret_type = SPINNER_FORMAT_FLOAT;
                       break;
                    }
                  else if (_is_valid_digit(*itr))
                    {
                       continue;
                    }
                  else
                    {
                       return SPINNER_FORMAT_INVALID;
                    }
               }
          }
        start = strchr(start + 2, '%');
     }

   return ret_type;
}

static void
_entry_show(Elm_Spinner_Data *sd)
{
   Eina_List *l;
   Elm_Spinner_Special_Value *sv;
   char buf[32], fmt[32] = "%0.f";

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value == sd->val)
          {
             snprintf(buf, sizeof(buf), "%s", sv->label);
             goto apply;
          }
     }
   /* try to construct just the format from given label
    * completely ignoring pre/post words
    */
   if (sd->label)
     {
        const char *start = strchr(sd->label, '%');
        while (start)
          {
             /* handle %% */
             if (start[1] != '%')
               break;
             else
               start = strchr(start + 2, '%');
          }

        if (start)
          {
             const char *itr, *end = NULL;
             for (itr = start + 1; *itr != '\0'; itr++)
               {
                  /* allowing '%d' is quite dangerous, remove it? */
                  if ((*itr == 'd') || (*itr == 'f'))
                    {
                       end = itr + 1;
                       break;
                    }
               }

             if ((end) && ((size_t)(end - start + 1) < sizeof(fmt)))
               {
                  memcpy(fmt, start, end - start);
                  fmt[end - start] = '\0';
               }
          }
     }

   if (_is_label_format_integer(fmt))
     snprintf(buf, sizeof(buf), fmt, (int)sd->val);
   else
     snprintf(buf, sizeof(buf), fmt, sd->val);

apply:
   elm_object_text_set(sd->ent, buf);
}

static void
_label_write(Evas_Object *obj)
{
   Eina_List *l;
   char buf[1024];
   Elm_Spinner_Special_Value *sv;

   ELM_SPINNER_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value == sd->val)
          {
             snprintf(buf, sizeof(buf), "%s", sv->label);
             goto apply;
          }
     }

   if (sd->label)
     {
        if (_is_label_format_integer(sd->label))
          snprintf(buf, sizeof(buf), sd->label, (int)sd->val);
        else
          snprintf(buf, sizeof(buf), sd->label, sd->val);
     }
   else
     snprintf(buf, sizeof(buf), "%.0f", sd->val);

apply:
   if (sd->button_layout)
     elm_layout_text_set(sd->text_button, "elm.text", buf);
   else
     elm_layout_text_set(obj, "elm.text", buf);

   efl_access_name_changed_signal_emit(obj);
   if (sd->entry_visible) _entry_show(sd);
}

static Eina_Bool
_delay_change_timer_cb(void *data)
{
   ELM_SPINNER_DATA_GET(data, sd);

   sd->delay_change_timer = NULL;
   efl_event_callback_legacy_call(data, ELM_SPINNER_EVENT_DELAY_CHANGED, NULL);

   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_value_set(Evas_Object *obj,
           double new_val)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->round > 0)
     new_val = sd->val_base +
       (double)((((int)(new_val - sd->val_base)) / sd->round) * sd->round);

   if (sd->wrap)
     {
        if (new_val < sd->val_min)
          new_val = sd->val_max;
        else if (new_val > sd->val_max)
          new_val = sd->val_min;
     }
   else
     {
        if (new_val < sd->val_min)
          new_val = sd->val_min;
        else if (new_val > sd->val_max)
          new_val = sd->val_max;
     }

   if (new_val == sd->val) return EINA_FALSE;
   sd->val = new_val;

   if (sd->val == sd->val_min)
     efl_event_callback_legacy_call(obj, ELM_SPINNER_EVENT_MIN_REACHED, NULL);
   else if (sd->val == sd->val_max)
     efl_event_callback_legacy_call(obj, ELM_SPINNER_EVENT_MAX_REACHED, NULL);

   efl_event_callback_legacy_call(obj, ELM_SPINNER_EVENT_CHANGED, NULL);
   efl_access_value_changed_signal_emit(obj);
   ecore_timer_del(sd->delay_change_timer);
   sd->delay_change_timer = ecore_timer_add(ELM_SPINNER_DELAY_CHANGE_TIME,
                                            _delay_change_timer_cb, obj);

   return EINA_TRUE;
}

static void
_val_set(Evas_Object *obj)
{
   double pos = 0.0;

   ELM_SPINNER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->val_max > sd->val_min)
     pos = ((sd->val - sd->val_min) / (sd->val_max - sd->val_min));

   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0) pos = 1.0;

   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", pos, pos);
}

static void
_drag_cb(void *data,
         Evas_Object *_obj EINA_UNUSED,
         const char *emission EINA_UNUSED,
         const char *source EINA_UNUSED)
{
   double pos = 0.0, delta;
   Evas_Object *obj = data;
   const char *style;

   ELM_SPINNER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (elm_widget_disabled_get(data)) return;
   if (sd->entry_visible) return;

   style = elm_widget_style_get(obj);

   if (sd->button_layout)
     {
        if (!strncmp(style, "vertical", 8))
          efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable.slider"), NULL, &pos);
        else
          efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable.slider"), &pos, NULL);
     }
   else
     efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable.slider"), &pos, NULL);

   if (sd->drag_prev_pos != 0)
     sd->drag_val_step = pow((pos - sd->drag_prev_pos), 2);
   else
     sd->drag_val_step = 1;


   delta = sd->drag_val_step * sd->step * _elm_config->scale;
   if (pos < sd->drag_prev_pos) delta *= -1;
   sd->drag_prev_pos = pos;

   /* Dragable is inverse of spinner value */
   if (!strncmp(style, "vertical", 8)) delta *= -1;
   /* If we are on rtl mode, change the delta to be negative on such changes */
   if (efl_ui_mirrored_get(obj)) delta *= -1;
   if (_value_set(data, sd->val + delta)) _label_write(data);
   sd->dragging = 1;
}

static void
_drag_start_cb(void *data,
               Evas_Object *obj EINA_UNUSED,
               const char *emission EINA_UNUSED,
               const char *source EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(data, sd);
   if (elm_widget_disabled_get(data)) return;
   sd->drag_prev_pos = 0;
   sd->drag_val_step = 1;

   efl_event_callback_legacy_call
     (obj, ELM_SPINNER_EVENT_SPINNER_DRAG_START, NULL);
}

static void
_drag_stop_cb(void *data,
              Evas_Object *obj EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   if (elm_widget_disabled_get(data)) return;

   sd->drag_prev_pos = 0;
   sd->drag_val_step = 1;
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", 0.0, 0.0);

   efl_event_callback_legacy_call
     (obj, ELM_SPINNER_EVENT_SPINNER_DRAG_STOP, NULL);
}

static void
_entry_hide(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->button_layout)
     {
        elm_layout_signal_emit(obj, "elm,state,button,active", "elm");
        evas_object_show(sd->text_button);
        elm_layout_signal_emit(obj, "elm,state,entry,inactive", "elm");
        evas_object_hide(sd->ent);
     }
   else
     elm_layout_signal_emit(obj, "elm,state,inactive", "elm");

   if (sd->entry_visible && !evas_focus_state_get(evas_object_evas_get(obj)))
     sd->entry_reactivate = EINA_TRUE;

   sd->entry_visible = EINA_FALSE;
}

static void
_entry_value_apply(Evas_Object *obj)
{
   const char *str;
   double val;
   char *end;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (!sd->entry_visible) return;

   efl_event_callback_del
    (sd->ent, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED, _entry_focus_change, obj);
   _entry_hide(obj);
   str = elm_object_text_get(sd->ent);
   if (!str) return;

   val = strtod(str, &end);
   if (((*end != '\0') && (!isspace(*end))) || (fabs(val - sd->val) < DBL_EPSILON)) return;
   elm_spinner_value_set(obj, val);

   efl_event_callback_legacy_call(obj, ELM_SPINNER_EVENT_CHANGED, NULL);
   ecore_timer_del(sd->delay_change_timer);
   sd->delay_change_timer = ecore_timer_add(ELM_SPINNER_DELAY_CHANGE_TIME,
                                            _delay_change_timer_cb, obj);
}

static int
_decimal_points_get(const char *label)
{
   char result[16] = "0";
   const char *start = strchr(label, '%');

   while (start)
     {
        if (start[1] != '%')
          {
             start = strchr(start, '.');
             if (start)
                start++;
             break;
          }
        else
          start = strchr(start + 2, '%');
     }

   if (start)
     {
        const char *p = strchr(start, 'f');

        if ((p) && ((p - start) < 15))
          sscanf(start, "%[^f]", result);
     }

   return atoi(result);
}

static void
_invalid_input_validity_filter(void *data EINA_UNUSED, Evas_Object *obj, char **text)
{
   char *insert = NULL;
   const char *str = NULL;
   int cursor_pos = 0;
   int read_idx = 0, read_char, cmp_char;

   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(text);

   insert = *text;
   str = elm_object_text_get(obj);

   evas_string_char_next_get(*text, 0, &read_char);
   cursor_pos = elm_entry_cursor_pos_get(obj);
   if (read_char)
     {
       if (read_char == '-')
         {
            if (cursor_pos != 0)
              {
                 goto invalid_input;
              }
         }
       if (read_char == '.')
         {
            read_idx = evas_string_char_next_get(str, 0, &cmp_char);
            while (cmp_char)
              {
                 if (read_char == cmp_char)
                   {
                      goto invalid_input;
                   }
                 read_idx = evas_string_char_next_get(str, read_idx, &cmp_char);
               }
         }
       read_idx = evas_string_char_next_get(str, 0, &cmp_char);
       if ((cmp_char == '-') && (cursor_pos == 0))
         {
            goto invalid_input;
         }
     }
   return;

invalid_input:
   *insert = 0;
}

static void
_entry_accept_filter_add(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);
   static Elm_Entry_Filter_Accept_Set digits_filter_data;

   if (!sd->ent) return;

   elm_entry_markup_filter_remove(sd->ent, elm_entry_filter_accept_set, &digits_filter_data);

   if (sd->decimal_points > 0)
     digits_filter_data.accepted = "-.0123456789";
   else
     digits_filter_data.accepted = "-0123456789";

   elm_entry_markup_filter_prepend(sd->ent, elm_entry_filter_accept_set, &digits_filter_data);
}

char *
_text_insert(const char *text, const char *input, int pos)
{
   char *result = NULL;
   int text_len, input_len;

   text_len = evas_string_char_len_get(text);
   input_len = evas_string_char_len_get(input);
   result = (char *)calloc(text_len + input_len + 1, sizeof(char));
   if (!result) return NULL;

   strncpy(result, text, pos);
   strcpy(result + pos, input);
   strcpy(result + pos + input_len, text + pos);

   return result;
}

static void
_min_max_validity_filter(void *data, Evas_Object *obj, char **text)
{
   const char *str, *point;
   char *insert, *new_str = NULL;
   double val;
   int max_len, len;

   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(text);

   ELM_SPINNER_DATA_GET(data, sd);

   str = elm_object_text_get(obj);
   if (!str) return;

   insert = *text;
   new_str = _text_insert(str, insert, elm_entry_cursor_pos_get(obj));
   if (!new_str) return;
   max_len = log10(fabs(sd->val_max)) + 1;

   if (sd->format_type == SPINNER_FORMAT_INT)
     {
        len = strlen(new_str);
        if (len < max_len) goto end;
     }
   else if (sd->format_type == SPINNER_FORMAT_FLOAT)
     {
        point = strchr(new_str, '.');
        if (point)
          {
             if ((int) strlen(point + 1) > sd->decimal_points)
               {
                  *insert = 0;
                  goto end;
               }
          }
     }

   val = strtod(new_str, NULL);
   if ((val < sd->val_min) || (val > sd->val_max))
     *insert = 0;

end:
   free(new_str);
}

static void
_entry_show_cb(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *obj,
               void *event_info EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(data, sd);

   _entry_show(sd);
   elm_object_focus_set(obj, EINA_TRUE);
   elm_entry_select_all(obj);
   sd->entry_visible = EINA_TRUE;
   elm_layout_signal_emit(data, "elm,state,button,inactive", "elm");
   evas_object_hide(sd->text_button);
}

static void
_toggle_entry(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->dragging)
     {
        sd->dragging = 0;
        return;
     }
   if (elm_widget_disabled_get(obj)) return;
   if (!sd->editable) return;
   if (sd->entry_visible) _entry_value_apply(obj);
   else
     {
        if (!sd->ent)
          {
             sd->ent = elm_entry_add(obj);
             Eina_Strbuf *buf = eina_strbuf_new();
             eina_strbuf_append_printf(buf, "spinner/%s", elm_widget_style_get(obj));
             elm_widget_style_set(sd->ent, eina_strbuf_string_get(buf));
             eina_strbuf_free(buf);
             if (sd->button_layout)
               {
                  evas_object_event_callback_add
                    (sd->ent, EVAS_CALLBACK_SHOW, _entry_show_cb, obj);
               }
             elm_entry_single_line_set(sd->ent, EINA_TRUE);
             elm_layout_content_set(obj, "elm.swallow.entry", sd->ent);
             _entry_accept_filter_add(obj);
             elm_entry_markup_filter_append(sd->ent, _invalid_input_validity_filter, NULL);
             if (_elm_config->spinner_min_max_filter_enable)
               elm_entry_markup_filter_append(sd->ent, _min_max_validity_filter, obj);
             efl_event_callback_add
                (sd->ent, ELM_ENTRY_EVENT_ACTIVATED, _entry_activated_cb, obj);
          }
        if (!sd->button_layout)
          {
             elm_layout_signal_emit(obj, "elm,state,active", "elm");
             _entry_show(sd);
             elm_entry_select_all(sd->ent);
             sd->entry_visible = EINA_TRUE;
          }

        efl_event_callback_add
           (sd->ent, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED, _entry_focus_change, obj);
        sd->entry_visible = EINA_TRUE;
        elm_layout_signal_emit(obj, "elm,state,entry,active", "elm");
        evas_object_show(sd->ent);
        {
           Eina_List *items = NULL;

           items = eina_list_append(items, sd->dec_button);
           items = eina_list_append(items, sd->text_button);
           items = eina_list_append(items, sd->ent);
           items = eina_list_append(items, sd->inc_button);

           efl_ui_focus_composition_elements_set(obj, items);
        }
        efl_ui_focus_manager_focus_set(efl_ui_focus_object_focus_manager_get(obj), sd->ent);
     }
}

static void
_entry_toggle_cb(void *data EINA_UNUSED,
                 Evas_Object *obj,
                 const char *emission EINA_UNUSED,
                 const char *source EINA_UNUSED)
{
   _toggle_entry(obj);
}

static Eina_Bool
_spin_value(void *data)
{
   ELM_SPINNER_DATA_GET(data, sd);
   double real_speed = sd->spin_speed;

   /* Sanity check: our step size should be at least as large as our rounding value */
   if ((sd->spin_speed != 0.0) && (fabs(sd->spin_speed) < sd->round))
     {
        WRN("The spinning step is smaller than the rounding value, please check your code");
        real_speed = sd->spin_speed > 0 ? sd->round : -sd->round;
     }

   sd->interval = sd->interval / 1.05;

   // spin_timer does not exist when _spin_value() is called from wheel event
   if (sd->spin_timer)
     ecore_timer_interval_set(sd->spin_timer, sd->interval);
   if (_value_set(data, sd->val + real_speed)) _label_write(data);

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_val_inc_dec_start(void *data)
{
   ELM_SPINNER_DATA_GET(data, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = sd->inc_btn_activated ? sd->step : -sd->step;
   sd->longpress_timer = NULL;
   ecore_timer_del(sd->spin_timer);
   sd->spin_timer = ecore_timer_add(sd->interval, _spin_value, data);
   _spin_value(data);

   elm_widget_scroll_freeze_push(data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_spin_stop(Evas_Object *obj)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   sd->interval = sd->first_interval;
   sd->spin_speed = 0;
   ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);

   elm_widget_scroll_freeze_pop(obj);
}

static Eina_Bool
_key_action_toggle(Evas_Object *obj, const char *params EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->spin_timer) _spin_stop(obj);
   else if (sd->entry_visible) _entry_toggle_cb(NULL, obj, NULL, NULL);

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_elm_spinner_efl_ui_widget_widget_event(Eo *obj, Elm_Spinner_Data *sd EINA_UNUSED, const Efl_Event *eo_event, Evas_Object *src EINA_UNUSED)
{
   Eo *ev = eo_event->info;

   if (efl_input_processed_get(ev)) return EINA_FALSE;
   if (eo_event->desc == EFL_EVENT_KEY_DOWN)
     {
        if (sd->spin_timer) _spin_stop(obj);
        else return EINA_FALSE;
     }
   else if (eo_event->desc == EFL_EVENT_KEY_UP)
     {
        if (sd->spin_timer) _spin_stop(obj);
        else return EINA_FALSE;
     }
   else if (eo_event->desc == EFL_EVENT_POINTER_WHEEL)
     {
        sd->interval = sd->first_interval;
        if (efl_input_pointer_wheel_delta_get(ev) < 0)
          {
             sd->spin_speed = sd->step;
             elm_layout_signal_emit(obj, "elm,right,anim,activate", "elm");
          }
        else
          {
             sd->spin_speed = -sd->step;
             elm_layout_signal_emit(obj, "elm,left,anim,activate", "elm");
          }
        _spin_value(obj);
     }
   else return EINA_FALSE;

   efl_input_processed_set(ev, EINA_TRUE);
   return EINA_TRUE;
}

static void
_button_inc_dec_start_cb(void *data,
                     Evas_Object *obj,
                     const char *emission,
                     const char *source EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(data, sd);

   sd->inc_btn_activated =
      !strcmp(emission, "elm,action,increment,start") ? EINA_TRUE : EINA_FALSE;

   if (sd->entry_visible)
     {
        _entry_value_apply(obj);

        if (sd->val_updated)
          {
             if (sd->inc_btn_activated)
               {
                  if (sd->val == sd->val_min) return;
               }
             else
               {
                  if (sd->val == sd->val_max) return;
               }
          }
     }

   ecore_timer_del(sd->longpress_timer);
   sd->longpress_timer = ecore_timer_add
     (_elm_config->longpress_timeout, _val_inc_dec_start, data);
}

static void
_button_inc_dec_stop_cb(void *data,
                    Evas_Object *obj EINA_UNUSED,
                    const char *emission EINA_UNUSED,
                    const char *source EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->longpress_timer)
     {
        ELM_SAFE_FREE(sd->longpress_timer, ecore_timer_del);

        if (sd->inc_btn_activated)
          sd->spin_speed = sd->step;
        else
          sd->spin_speed = -sd->step;

        _spin_value(data);
     }

   _spin_stop(data);
}

static void
_inc_dec_button_clicked_cb(void *data, const Efl_Event *event)
{
   ELM_SPINNER_DATA_GET(data, sd);

   _spin_stop(data);
   sd->inc_btn_activated = sd->inc_button == event->object ? EINA_TRUE : EINA_FALSE;
   sd->spin_speed = sd->inc_btn_activated ? sd->step : -sd->step;
   _spin_value(data);

   if (_elm_config->access_mode)
     _access_increment_decrement_info_say(data, EINA_TRUE);
}

static void
_inc_dec_button_pressed_cb(void *data, const Efl_Event *event)
{
   ELM_SPINNER_DATA_GET(data, sd);

   sd->inc_btn_activated = sd->inc_button == event->object ? EINA_TRUE : EINA_FALSE;

   if (sd->longpress_timer) ecore_timer_del(sd->longpress_timer);

   sd->longpress_timer = ecore_timer_add
                           (_elm_config->longpress_timeout,
                            _val_inc_dec_start, data);

   if (sd->entry_visible) _entry_value_apply(data);
}

static void
_inc_dec_button_unpressed_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->longpress_timer)
     {
        ecore_timer_del(sd->longpress_timer);
        sd->longpress_timer = NULL;
     }

   _spin_stop(data);
}

static void
_text_button_focus_change(void *data, const Efl_Event *event)
{
   if (efl_ui_focus_object_focus_get(event->object))
     _toggle_entry(data);
}

static void
_entry_activated_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   _toggle_entry(data);
}

static void
_entry_focus_change(void *data, const Efl_Event *event)
{
   if (!efl_ui_focus_object_focus_get(event->object))
     _toggle_entry(data);
}

static void
_text_button_clicked_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (sd->entry_visible) return;
   _toggle_entry(data);
}

static void
_inc_dec_button_mouse_move_cb(void *data, const Efl_Event *event)
{
   Efl_Input_Pointer *ev = event->info;
   ELM_SPINNER_DATA_GET(data, sd);

   if (efl_input_processed_get(ev) && sd->longpress_timer)
     {
        ecore_timer_del(sd->longpress_timer);
        sd->longpress_timer = NULL;
     }
}

EOLIAN static void
_elm_spinner_elm_layout_sizing_eval(Eo *obj, Elm_Spinner_Data *_pd EINA_UNUSED)
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
_elm_spinner_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Spinner_Data *sd)
{
   Eina_Bool int_ret = EINA_FALSE;

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (!efl_ui_focus_object_focus_get(obj))
     {
        ELM_SAFE_FREE(sd->delay_change_timer, ecore_timer_del);
        ELM_SAFE_FREE(sd->spin_timer, ecore_timer_del);
     }
   else
     {
        if (sd->entry_reactivate)
          {
             _toggle_entry(obj);

             sd->entry_reactivate = EINA_FALSE;
          }
     }

   return EINA_TRUE;
}

static void
_access_activate_cb(void *data,
                    Evas_Object *part_obj,
                    Elm_Object_Item *item EINA_UNUSED)
{
   char *text;
   Eina_Strbuf *buf;
   Evas_Object *eo, *inc_btn;
   const char* increment_part;
   ELM_SPINNER_DATA_GET(data, sd);

   if (!strncmp(elm_widget_style_get(data), "vertical", 8))
     increment_part = "up_bt";
   else
     increment_part = "right_bt";

   eo = elm_layout_edje_get(data);
   inc_btn = (Evas_Object *)edje_object_part_object_get(eo, increment_part);

   if (part_obj != inc_btn)
     {
        sd->inc_btn_activated = EINA_FALSE;
        _val_inc_dec_start(data);
        elm_layout_signal_emit(data, "elm,left,anim,activate", "elm");
        _spin_stop(data);
        text = "decremented";
     }
   else
     {
        sd->inc_btn_activated = EINA_TRUE;
        _val_inc_dec_start(data);
        elm_layout_signal_emit(data, "elm,right,anim,activate", "elm");
        _spin_stop(data);
        text = "incremented";
     }

   buf = eina_strbuf_new();

   eina_strbuf_append_printf(buf, "%s, %s", text,
                             elm_layout_text_get(data, "elm.text"));

   _elm_access_say(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   const char *txt = NULL;
   Evas_Object *spinner = (Evas_Object *)(data);
   ELM_SPINNER_DATA_GET(spinner, sd);

   if (sd->button_layout)
     {
        if (sd->entry_visible)
          txt = elm_object_text_get(sd->ent);
        else
          txt = elm_object_text_get(sd->text_button);
     }
   else
     {
        txt = elm_layout_text_get(spinner, "elm.text");
     }

   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   if (elm_widget_disabled_get(data))
     return strdup(E_("State: Disabled"));

   return NULL;
}

static void
_access_activate_spinner_cb(void *data,
                            Evas_Object *part_obj EINA_UNUSED,
                            Elm_Object_Item *item EINA_UNUSED)
{
   ELM_SPINNER_DATA_GET(data, sd);

   if (elm_widget_disabled_get(data)) return;
   if (!sd->entry_visible)
     _toggle_entry(data);
}

static void
_access_increment_decrement_info_say(Evas_Object *obj,
                                     Eina_Bool is_incremented)
{
   Eina_Strbuf *buf;

   ELM_SPINNER_DATA_GET(obj, sd);

    buf = eina_strbuf_new();
    if (is_incremented)
      {
         elm_object_signal_emit
            (sd->inc_button, "elm,action,anim,activate", "elm");
         eina_strbuf_append(buf, E_("incremented"));
      }
    else
      {
         elm_object_signal_emit
            (sd->dec_button, "elm,action,anim,activate", "elm");
         eina_strbuf_append(buf, E_("decremented"));
      }

   eina_strbuf_append_printf
      (buf, "%s", elm_object_text_get(sd->text_button));

   _elm_access_say(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
}

static void
_access_spinner_register(Evas_Object *obj, Eina_Bool is_access)
{
   Evas_Object *ao;
   Elm_Access_Info *ai;

   ELM_SPINNER_DATA_GET(obj, sd);

   if (sd->button_layout)
     {
        if (!is_access)
          {
             /* unregister access */
             _elm_access_edje_object_part_object_unregister
             (obj, elm_layout_edje_get(obj), "access");
             elm_layout_signal_emit(obj, "elm,state,access,inactive", "elm");
             return;
          }
        elm_layout_signal_emit(obj, "elm,state,access,active", "elm");
        ao = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), "access");

        ai = _elm_access_info_get(ao);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner"));
        _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, obj);
        _elm_access_activate_callback_set(ai, _access_activate_spinner_cb, obj);

        /*Do not register spinner buttons if widget is disabled*/
        if (!elm_widget_disabled_get(obj))
          {
             ai = _elm_access_info_get(sd->inc_button);
             _elm_access_text_set(ai, ELM_ACCESS_TYPE,
                                  E_("spinner increment button"));
             ai = _elm_access_info_get(sd->dec_button);
             _elm_access_text_set(ai, ELM_ACCESS_TYPE,
                                  E_("spinner decrement button"));
             ai = _elm_access_info_get(sd->text_button);
             _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner text"));
             _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, obj);
          }
     }
   else
     {
        const char* increment_part;
        const char* decrement_part;

        if (!strncmp(elm_widget_style_get(obj), "vertical", 8))
          {
             increment_part = "up_bt";
             decrement_part = "down_bt";
          }
        else
          {
             increment_part = "right_bt";
             decrement_part = "left_bt";
          }
        if (!is_access)
          {
             /* unregister increment button, decrement button and spinner label */
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), increment_part);
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), decrement_part);
             _elm_access_edje_object_part_object_unregister
               (obj, elm_layout_edje_get(obj), "access.text");
             return;
          }

        /* register increment button */
        ao = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), increment_part);
        ai = _elm_access_info_get(ao);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE,
                             E_("spinner increment button"));
        _elm_access_activate_callback_set(ai, _access_activate_cb, obj);

        /* register decrement button */
        ao = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), decrement_part);

        ai = _elm_access_info_get(ao);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE,
                             E_("spinner decrement button"));
        _elm_access_activate_callback_set(ai, _access_activate_cb, obj);

        /* register spinner label */
        ao = _elm_access_edje_object_part_object_register
          (obj, elm_layout_edje_get(obj), "access.text");

        ai = _elm_access_info_get(ao);
        _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("spinner"));
        _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, obj);
        _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, obj);
     }
}

EOLIAN static void
_elm_spinner_efl_canvas_group_group_add(Eo *obj, Elm_Spinner_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   ELM_SPINNER_DATA_GET(obj, sd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->val_max = 100.0;
   priv->step = 1.0;
   priv->first_interval = 0.85;

   if (!elm_layout_theme_set(obj, "spinner", "base",
                             elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   if (edje_object_part_exists(wd->resize_obj, "elm.swallow.dec_button"))
     sd->button_layout = EINA_TRUE;
   else
     sd->button_layout = EINA_FALSE;

   elm_layout_signal_callback_add(obj, "drag", "*", _drag_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,start", "*", _drag_start_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,stop", "*", _drag_stop_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,step", "*", _drag_stop_cb, obj);
   elm_layout_signal_callback_add(obj, "drag,page", "*", _drag_stop_cb, obj);

   if (sd->button_layout)
     {
        priv->inc_button = elm_button_add(obj);
        elm_object_style_set(priv->inc_button, "spinner/increase/default");

        efl_event_callback_array_add(priv->inc_button, _inc_dec_button_cb(), obj);

        elm_layout_content_set(obj, "elm.swallow.inc_button", priv->inc_button);
        elm_widget_sub_object_add(obj, priv->inc_button);

        priv->text_button = elm_button_add(obj);
        elm_object_style_set(priv->text_button, "spinner/default");

        efl_event_callback_add
          (priv->text_button, EFL_UI_EVENT_CLICKED, _text_button_clicked_cb, obj);

        efl_event_callback_add
          (priv->text_button, EFL_UI_FOCUS_OBJECT_EVENT_FOCUS_CHANGED, _text_button_focus_change, obj);

        elm_layout_content_set(obj, "elm.swallow.text_button", priv->text_button);
        elm_widget_sub_object_add(obj, priv->text_button);

        priv->dec_button = elm_button_add(obj);
        elm_object_style_set(priv->dec_button, "spinner/decrease/default");

        efl_event_callback_array_add(priv->dec_button, _inc_dec_button_cb(), obj);

        elm_layout_content_set(obj, "elm.swallow.dec_button", priv->dec_button);
        elm_widget_sub_object_add(obj, priv->dec_button);

        {
           Eina_List *items = NULL;

           items = eina_list_append(items, priv->dec_button);
           items = eina_list_append(items, priv->text_button);
           items = eina_list_append(items, priv->inc_button);

           efl_ui_focus_composition_elements_set(obj, items);
        }
     }
   else
     {
        elm_layout_signal_callback_add
          (obj, "elm,action,increment,start", "*", _button_inc_dec_start_cb, obj);
        elm_layout_signal_callback_add
          (obj, "elm,action,increment,stop", "*", _button_inc_dec_stop_cb, obj);
        elm_layout_signal_callback_add
          (obj, "elm,action,decrement,start", "*", _button_inc_dec_start_cb, obj);
        elm_layout_signal_callback_add
          (obj, "elm,action,decrement,stop", "*", _button_inc_dec_stop_cb, obj);
     }

   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.slider", 0.0, 0.0);

   elm_layout_signal_callback_add
     (obj, "elm,action,entry,toggle", "*", _entry_toggle_cb, NULL);

   _label_write(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   /* access */
   if (_elm_config->access_mode)
     _access_spinner_register(obj, EINA_TRUE);
}

EOLIAN static void
_elm_spinner_efl_canvas_group_group_del(Eo *obj, Elm_Spinner_Data *sd)
{
   Elm_Spinner_Special_Value *sv;

   eina_stringshare_del(sd->label);
   ecore_timer_del(sd->delay_change_timer);
   ecore_timer_del(sd->spin_timer);
   ecore_timer_del(sd->longpress_timer);

   if (sd->special_values)
     {
        EINA_LIST_FREE(sd->special_values, sv)
          {
             eina_stringshare_del(sv->label);
             free(sv);
          }
     }

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Efl_Ui_Theme_Apply
_elm_spinner_efl_ui_widget_theme_apply(Eo *obj, Elm_Spinner_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_FAILED);

   if (!efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS)))
     {
        CRI("Failed to set layout!");
        return EFL_UI_THEME_APPLY_FAILED;
     }

   if (edje_object_part_exists(wd->resize_obj, "elm.swallow.dec_button"))
     sd->button_layout = EINA_TRUE;
   else
     sd->button_layout = EINA_FALSE;

   if (sd->ent)
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        eina_strbuf_append_printf(buf, "spinner/%s", elm_widget_style_get(obj));
        elm_widget_style_set(sd->ent, eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
     }

   if (sd->inc_button)
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        eina_strbuf_append_printf(buf, "spinner/increase/%s", elm_widget_style_get(obj));
        elm_widget_style_set(sd->inc_button, eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
     }

   if (sd->text_button)
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        eina_strbuf_append_printf(buf, "spinner/%s", elm_widget_style_get(obj));
        elm_widget_style_set(sd->text_button, eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
     }

   if (sd->dec_button)
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        eina_strbuf_append_printf(buf, "spinner/decrease/%s", elm_widget_style_get(obj));
        elm_widget_style_set(sd->dec_button, eina_strbuf_string_get(buf));
        eina_strbuf_free(buf);
     }

   if (_elm_config->access_mode)
     _access_spinner_register(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);
   return EFL_UI_THEME_APPLY_SUCCESS;
}

static Eina_Bool _elm_spinner_smart_focus_next_enable = EINA_FALSE;

EOLIAN static void
_elm_spinner_efl_ui_widget_on_access_update(Eo *obj, Elm_Spinner_Data *_pd EINA_UNUSED, Eina_Bool acs)
{
   _elm_spinner_smart_focus_next_enable = acs;
   _access_spinner_register(obj, _elm_spinner_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_spinner_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EAPI void
elm_spinner_min_max_set(Evas_Object *obj, double min, double max)
{
   efl_ui_range_min_max_set(obj, min, max);
}

EAPI void
elm_spinner_min_max_get(const Evas_Object *obj, double *min, double *max)
{
   efl_ui_range_min_max_get(obj, min, max);
}

EAPI void
elm_spinner_step_set(Evas_Object *obj, double step)
{
   efl_ui_range_step_set(obj, step);
}

EAPI double
elm_spinner_step_get(const Evas_Object *obj)
{
   return efl_ui_range_step_get(obj);
}

EAPI void
elm_spinner_value_set(Evas_Object *obj, double val)
{
   efl_ui_range_value_set(obj, val);
}

EAPI double
elm_spinner_value_get(const Evas_Object *obj)
{
   return efl_ui_range_value_get(obj);
}

EOLIAN static Eo *
_elm_spinner_efl_object_constructor(Eo *obj, Elm_Spinner_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_role_set(obj, EFL_ACCESS_ROLE_SPIN_BUTTON);

   return obj;
}

EOLIAN static void
_elm_spinner_label_format_set(Eo *obj, Elm_Spinner_Data *sd, const char *fmt)
{
   Elm_Spinner_Format_Type type;

   if (!fmt) fmt = "%.0f";
   type = _is_label_format_integer(fmt);
   if (type == SPINNER_FORMAT_INVALID)
     {
        ERR("format:\"%s\" is invalid, cannot be set", fmt);
        return;
     }
   else if (type == SPINNER_FORMAT_FLOAT)
     {
        sd->decimal_points = _decimal_points_get(fmt);
     }

   eina_stringshare_replace(&sd->label, fmt);

   sd->format_type = type;
   _label_write(obj);
   elm_layout_sizing_eval(obj);
   _entry_accept_filter_add(obj);
}

EOLIAN static const char*
_elm_spinner_label_format_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->label;
}

EOLIAN static void
_elm_spinner_efl_ui_range_range_min_max_set(Eo *obj, Elm_Spinner_Data *sd, double min, double max)
{
   if ((sd->val_min == min) && (sd->val_max == max)) return;

   sd->val_min = min;
   sd->val_max = max;

   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _val_set(obj);
   _label_write(obj);
}

EOLIAN static void
_elm_spinner_efl_ui_range_range_min_max_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, double *min, double *max)
{
   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EOLIAN static void
_elm_spinner_efl_ui_range_range_step_set(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, double step)
{
   sd->step = step;
}

EOLIAN static double
_elm_spinner_efl_ui_range_range_step_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->step;
}

EOLIAN static void
_elm_spinner_efl_ui_range_range_value_set(Eo *obj, Elm_Spinner_Data *sd, double val)
{
   if (sd->val == val) return;

   sd->val = val;
   sd->val_updated = EINA_FALSE;

   if (sd->val < sd->val_min)
     {
        sd->val = sd->val_min;
        sd->val_updated = EINA_TRUE;
     }
   if (sd->val > sd->val_max)
     {
        sd->val = sd->val_max;
        sd->val_updated = EINA_TRUE;
     }

   _val_set(obj);
   _label_write(obj);
}

EOLIAN static double
_elm_spinner_efl_ui_range_range_value_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->val;
}

EOLIAN static void
_elm_spinner_wrap_set(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, Eina_Bool wrap)
{
   sd->wrap = wrap;
}

EOLIAN static Eina_Bool
_elm_spinner_wrap_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->wrap;
}

EOLIAN static void
_elm_spinner_special_value_add(Eo *obj, Elm_Spinner_Data *sd, double value, const char *label)
{
   Elm_Spinner_Special_Value *sv;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value != value)
          continue;

        eina_stringshare_replace(&sv->label, label);
        _label_write(obj);
        return;
     }

   sv = calloc(1, sizeof(*sv));
   if (!sv) return;
   sv->value = value;
   sv->label = eina_stringshare_add(label);

   sd->special_values = eina_list_append(sd->special_values, sv);
   _label_write(obj);
}

EAPI void
elm_spinner_special_value_del(Evas_Object *obj,
                              double value)
{
   Elm_Spinner_Special_Value *sv;
   Eina_List *l;

   ELM_SPINNER_CHECK(obj);
   ELM_SPINNER_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value != value)
          continue;

        sd->special_values = eina_list_remove_list(sd->special_values, l);
        eina_stringshare_del(sv->label);
        free(sv);
        _label_write(obj);
        return;
     }
}

EAPI const char *
elm_spinner_special_value_get(Evas_Object *obj,
                              double value)
{
   Elm_Spinner_Special_Value *sv;
   Eina_List *l;

   ELM_SPINNER_CHECK(obj) NULL;
   ELM_SPINNER_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->special_values, l, sv)
     {
        if (sv->value == value)
          return sv->label;
     }

   return NULL;
}

EOLIAN static void
_elm_spinner_editable_set(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, Eina_Bool editable)
{
   sd->editable = editable;
}

EOLIAN static Eina_Bool
_elm_spinner_editable_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->editable;
}

EOLIAN static void
_elm_spinner_interval_set(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, double interval)
{
   sd->first_interval = interval;
}

EOLIAN static double
_elm_spinner_interval_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->first_interval;
}

EOLIAN static void
_elm_spinner_base_set(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, double base)
{
   sd->val_base = base;
}

EOLIAN static double
_elm_spinner_base_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->val_base;
}

EOLIAN static void
_elm_spinner_round_set(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, int rnd)
{
   sd->round = rnd;
}

EOLIAN static int
_elm_spinner_round_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->round;
}

EOLIAN static void
_elm_spinner_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   if (_elm_config->access_mode)
      _elm_spinner_smart_focus_next_enable = EINA_TRUE;
}

EOLIAN static const Efl_Access_Action_Data *
_elm_spinner_efl_access_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
      { "toggle", "toggle", NULL, _key_action_toggle},
      { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

// A11Y Accessibility

EOLIAN static void
_elm_spinner_efl_access_value_value_and_text_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, double *value, const char **text)
{
   if (value) *value = sd->val;
   if (text) *text = NULL;
}

EOLIAN static Eina_Bool
_elm_spinner_efl_access_value_value_and_text_set(Eo *obj, Elm_Spinner_Data *sd, double value, const char *text EINA_UNUSED)
{
   if (sd->val_min > value) return EINA_FALSE;
   if (sd->val_max < value) return EINA_FALSE;

   sd->val = value;
   _val_set(obj);

   return EINA_TRUE;
}

EOLIAN static void
_elm_spinner_efl_access_value_range_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd, double *lower, double *upper, const char **descr)
{
   if (lower) *lower = sd->val_min;
   if (upper) *upper = sd->val_max;
   if (descr) *descr = NULL;
}

EOLIAN static double
_elm_spinner_efl_access_value_increment_get(Eo *obj EINA_UNUSED, Elm_Spinner_Data *sd)
{
   return sd->step;
}

EOLIAN static const char*
_elm_spinner_efl_access_name_get(Eo *obj, Elm_Spinner_Data *sd)
{
   const char *name, *ret;
   name = efl_access_name_get(efl_super(obj, ELM_SPINNER_CLASS));
   if (name) return name;
   if (sd->button_layout)
     {
        if (sd->entry_visible)
          ret = elm_object_text_get(sd->ent);
        else
          ret = elm_object_text_get(sd->text_button);
     }
   else
     ret = elm_layout_text_get(obj, "elm.text");
   return ret;
}

// A11Y Accessibility - END

/* Internal EO APIs and hidden overrides */

#define ELM_SPINNER_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_spinner), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_spinner)

#include "elm_spinner.eo.c"
