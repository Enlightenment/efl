#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_VALUE_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_spin_private.h"

#define MY_CLASS EFL_UI_SPIN_CLASS

#define MY_CLASS_NAME "Efl.Ui.Spin"

static Eina_Bool
_is_valid_digit(char x)
{
   return ((x >= '0' && x <= '9') || (x == '.')) ? EINA_TRUE : EINA_FALSE;
}

static Efl_Ui_Spin_Format_Type
_is_label_format_integer(const char *fmt)
{
   const char *itr = NULL;
   const char *start = NULL;
   Eina_Bool found = EINA_FALSE;
   Efl_Ui_Spin_Format_Type ret_type = SPIN_FORMAT_INVALID;

   start = strchr(fmt, '%');
   if (!start) return SPIN_FORMAT_INVALID;

   while (start)
     {
        if (found && start[1] != '%')
          {
             return SPIN_FORMAT_INVALID;
          }

        if (start[1] != '%' && !found)
          {
             found = EINA_TRUE;
             for (itr = start + 1; *itr != '\0'; itr++)
               {
                  if ((*itr == 'd') || (*itr == 'u') || (*itr == 'i') ||
                      (*itr == 'o') || (*itr == 'x') || (*itr == 'X'))
                    {
                       ret_type = SPIN_FORMAT_INT;
                       break;
                    }
                  else if ((*itr == 'f') || (*itr == 'F'))
                    {
                       ret_type = SPIN_FORMAT_FLOAT;
                       break;
                    }
                  else if (_is_valid_digit(*itr))
                    {
                       continue;
                    }
                  else
                    {
                       return SPIN_FORMAT_INVALID;
                    }
               }
          }
        start = strchr(start + 2, '%');
     }

   return ret_type;
}
static void
_label_write(Evas_Object *obj)
{
   Efl_Ui_Spin_Special_Value *sv;
   unsigned int i;
   Eina_Array_Iterator iterator;

   Efl_Ui_Spin_Data *sd = efl_data_scope_get(obj, MY_CLASS);

   EINA_ARRAY_ITER_NEXT(sd->special_values, i, sv, iterator)
     {
        if (sv->value == sd->val)
          {
             char buf[1024];
             snprintf(buf, sizeof(buf), "%s", sv->label);
             elm_layout_text_set(obj, "elm.text", buf);
             sd->templates = sv->label;
             return;
          }
     }

   if (sd->format_cb)
     {
        const char *buf;
        Eina_Value val;

        if (sd->format_type == SPIN_FORMAT_INT)
          {
             eina_value_setup(&val, EINA_VALUE_TYPE_INT);
             eina_value_set(&val, (int)sd->val);
          }
        else
          {
             eina_value_setup(&val, EINA_VALUE_TYPE_DOUBLE);
             eina_value_set(&val, sd->val);
          }
        eina_strbuf_reset(sd->format_strbuf);
        sd->format_cb(sd->format_cb_data, sd->format_strbuf, val);

        buf = eina_strbuf_string_get(sd->format_strbuf);
        eina_value_flush(&val);
        elm_layout_text_set(obj, "efl.text", buf);
        sd->templates = buf;
     }
   else
     {
        char buf[1024];
        snprintf(buf, sizeof(buf), "%.0f", sd->val);
        elm_layout_text_set(obj, "efl.text", buf);
        evas_object_show(obj);
     }
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

EOLIAN static void
_efl_ui_spin_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Spin_Data *_pd EINA_UNUSED)
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
_efl_ui_spin_efl_ui_widget_widget_event(Eo *obj, Efl_Ui_Spin_Data *sd, const Efl_Event *eo_event, Evas_Object *src EINA_UNUSED)
{
   Eo *ev = eo_event->info;

   if (efl_input_processed_get(ev)) return EINA_FALSE;

   if (eo_event->desc == EFL_EVENT_POINTER_WHEEL)
     {
       if (efl_input_pointer_wheel_delta_get(ev) < 0)
         efl_ui_range_value_set(obj, (efl_ui_range_value_get(obj) + sd->step));
       else
         efl_ui_range_value_set(obj, (efl_ui_range_value_get(obj) - sd->step));
     }
   else
     return EINA_FALSE;

   efl_input_processed_set(ev, EINA_TRUE);
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_spin_special_value_set(Eo *obj, Efl_Ui_Spin_Data *sd, const Eina_Array *values)
{
   EINA_SAFETY_ON_NULL_RETURN(values);

   unsigned int i;
   Efl_Ui_Spin_Special_Value *sv;
   Efl_Ui_Spin_Special_Value *temp;
   Eina_Array_Iterator iterator;

   if (eina_array_count(sd->special_values))
     {
        EINA_ARRAY_ITER_NEXT(sd->special_values, i, sv, iterator)
          {
             eina_stringshare_del(sv->label);
             free(sv);
          }
        eina_array_clean(sd->special_values);
     }

   if (eina_array_count(values))
     EINA_ARRAY_ITER_NEXT(values, i, temp, iterator)
       {
          sv = calloc(1, sizeof(*sv));
          if (!sv) return;
          sv->value = temp->value;
          sv->label = eina_stringshare_add(temp->label);
          eina_array_push(sd->special_values, sv);
       }

   _label_write(obj);
}

EOLIAN static const Eina_Array*
_efl_ui_spin_special_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Data *sd)
{
   if (eina_array_count(sd->special_values))
     return sd->special_values;
   else
     return NULL;
}

EOLIAN static Eo *
_efl_ui_spin_efl_object_constructor(Eo *obj, Efl_Ui_Spin_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "spin");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   elm_widget_sub_object_parent_add(obj);

   sd->val_max = 100.0;
   sd->step = 1.0;
   sd->special_values = eina_array_new(sizeof(Efl_Ui_Spin_Special_Value));

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   _label_write(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   return obj;
}

EOLIAN static void
_efl_ui_spin_efl_object_destructor(Eo *obj, Efl_Ui_Spin_Data *sd EINA_UNUSED)
{
   Efl_Ui_Spin_Special_Value *sv;
   Eina_Array_Iterator iterator;
   unsigned int i;

   efl_ui_format_cb_set(obj, NULL, NULL, NULL);

   EINA_ARRAY_ITER_NEXT(sd->special_values, i, sv, iterator)
     {
        eina_stringshare_del(sv->label);
        free(sv);
     }
   eina_array_free(sd->special_values);

   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_spin_efl_ui_format_format_cb_set(Eo *obj, Efl_Ui_Spin_Data *sd, void *func_data, Efl_Ui_Format_Func_Cb func, Eina_Free_Cb func_free_cb)
{
   if (sd->format_cb_data == func_data && sd->format_cb == func)
     return;

   if (sd->format_cb_data && sd->format_free_cb)
     sd->format_free_cb(sd->format_cb_data);

   sd->format_cb = func;
   sd->format_cb_data = func_data;
   sd->format_free_cb = func_free_cb;
   if (!sd->format_strbuf) sd->format_strbuf = eina_strbuf_new();

   const char *format = efl_ui_format_string_get(obj);
   if (format)
     {
        sd->format_type = _is_label_format_integer(format);
        if (sd->format_type == SPIN_FORMAT_INVALID)
          {
             ERR("format:\"%s\" is invalid, cannot be set", format);
             return;
          }
        else if (sd->format_type == SPIN_FORMAT_FLOAT)
          sd->decimal_points = _decimal_points_get(format);
     }

   _label_write(obj);
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_spin_efl_ui_range_display_range_min_max_set(Eo *obj, Efl_Ui_Spin_Data *sd, double min, double max)
{
   if (max < min)
     {
        ERR("Wrong params. min(%lf) is bigger than max(%lf). It will swaped.", min, max);
        double t = min;
        min = max;
        max = t;
     }
   if ((EINA_DBL_EQ(sd->val_min, min)) && (EINA_DBL_EQ(sd->val_max, max))) return;

   sd->val_min = min;
   sd->val_max = max;

   if (sd->val < sd->val_min) sd->val = sd->val_min;
   if (sd->val > sd->val_max) sd->val = sd->val_max;

   _label_write(obj);
}

EOLIAN static void
_efl_ui_spin_efl_ui_range_display_range_min_max_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Data *sd, double *min, double *max)
{
   if (min) *min = sd->val_min;
   if (max) *max = sd->val_max;
}

EOLIAN static void
_efl_ui_spin_efl_ui_range_interactive_range_step_set(Eo *obj EINA_UNUSED, Efl_Ui_Spin_Data *sd, double step)
{
   if (step <= 0)
     {
        ERR("Wrong param. The step(%lf) should be bigger than 0.0", step);
        return;
     }

   sd->step = step;
}

EOLIAN static double
_efl_ui_spin_efl_ui_range_interactive_range_step_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Data *sd)
{
   return sd->step;
}

EOLIAN static void
_efl_ui_spin_efl_ui_range_display_range_value_set(Eo *obj, Efl_Ui_Spin_Data *sd, double val)
{
   if (val < sd->val_min)
     val = sd->val_min;
   else if (val > sd->val_max)
     val = sd->val_max;

   if (EINA_DBL_EQ(val, sd->val)) return;

   sd->val = val;

   if (EINA_DBL_EQ(sd->val, sd->val_min))
     efl_event_callback_call(obj, EFL_UI_SPIN_EVENT_MIN_REACHED, NULL);
   else if (EINA_DBL_EQ(sd->val, sd->val_max))
     efl_event_callback_call(obj, EFL_UI_SPIN_EVENT_MAX_REACHED, NULL);

   efl_event_callback_call(obj, EFL_UI_SPIN_EVENT_CHANGED, NULL);

   _label_write(obj);
}

EOLIAN static double
_efl_ui_spin_efl_ui_range_display_range_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Data *sd)
{
   return sd->val;
}

#define EFL_UI_SPIN_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_spin), \

#include "efl_ui_spin.eo.c"
