#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_VALUE_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_UI_FORMAT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_spin_private.h"

#define MY_CLASS EFL_UI_SPIN_CLASS

#define MY_CLASS_NAME "Efl.Ui.Spin"

static void
_label_write(Evas_Object *obj, Efl_Ui_Spin_Data *sd)
{
   Eina_Strbuf *strbuf = eina_strbuf_new();
   Eina_Value val = eina_value_double_init(sd->val);
   efl_ui_format_formatted_value_get(obj, strbuf, val);

   elm_layout_text_set(obj, "efl.text", eina_strbuf_string_get(strbuf));

   eina_value_flush(&val);
   eina_strbuf_free(strbuf);
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
_efl_ui_spin_efl_ui_widget_widget_input_event_handler(Eo *obj, Efl_Ui_Spin_Data *sd, const Efl_Event *eo_event, Evas_Object *src EINA_UNUSED)
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

EOLIAN static Eo *
_efl_ui_spin_efl_object_constructor(Eo *obj, Efl_Ui_Spin_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "spin");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   sd->val_max = 100.0;
   sd->step = 1.0;

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   _label_write(obj, sd);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_sizing_eval(obj);

   return obj;
}

EOLIAN static void
_efl_ui_spin_efl_object_destructor(Eo *obj, Efl_Ui_Spin_Data *sd EINA_UNUSED)
{
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_spin_efl_ui_format_apply_formatted_value(Eo *obj, Efl_Ui_Spin_Data *sd EINA_UNUSED)
{
   _label_write(obj, sd);
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_spin_efl_ui_range_display_range_limits_set(Eo *obj, Efl_Ui_Spin_Data *sd, double min, double max)
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

   _label_write(obj, sd);
}

EOLIAN static void
_efl_ui_spin_efl_ui_range_display_range_limits_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Data *sd, double *min, double *max)
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

   _label_write(obj, sd);
}

EOLIAN static double
_efl_ui_spin_efl_ui_range_display_range_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Data *sd)
{
   return sd->val;
}

#define EFL_UI_SPIN_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_spin), \

#include "efl_ui_spin.eo.c"
