#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
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

   efl_text_set(efl_part(obj, "efl.text"), eina_strbuf_string_get(strbuf));

   eina_value_flush(&val);
   eina_strbuf_free(strbuf);
}

EOLIAN static Eo *
_efl_ui_spin_efl_object_constructor(Eo *obj, Efl_Ui_Spin_Data *sd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "spin");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   sd->val_max = 100.0;

   if (elm_widget_theme_object_set(obj, wd->resize_obj,
                                       elm_widget_theme_klass_get(obj),
                                       elm_widget_theme_element_get(obj),
                                       elm_widget_theme_style_get(obj)) == EFL_UI_THEME_APPLY_ERROR_GENERIC)
     CRI("Failed to set layout!");

   _label_write(obj, sd);
   efl_ui_widget_focus_allow_set(obj, EINA_TRUE);

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
   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_spin_efl_ui_range_display_range_limits_set(Eo *obj, Efl_Ui_Spin_Data *sd, double min, double max)
{
   if (max < min)
     {
        ERR("Wrong params. min(%lf) is greater than max(%lf).", min, max);
        return;
     }
   if (EINA_DBL_EQ(max, min))
     {
        ERR("min and max must have a different value");
        return;
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
_efl_ui_spin_efl_ui_range_display_range_value_set(Eo *obj, Efl_Ui_Spin_Data *sd, double val)
{
   if (val < sd->val_min)
     {
        ERR("Error, value is less than minimum");
        return;
     }

   if (val > sd->val_max)
     {
        ERR("Error, value is greater than maximum");
        return;
     }

   if (EINA_DBL_EQ(val, sd->val)) return;

   sd->val = val;

   if (EINA_DBL_EQ(sd->val, sd->val_min))
     efl_event_callback_call(obj, EFL_UI_RANGE_EVENT_MIN_REACHED, NULL);
   else if (EINA_DBL_EQ(sd->val, sd->val_max))
     efl_event_callback_call(obj, EFL_UI_RANGE_EVENT_MAX_REACHED, NULL);

   efl_event_callback_call(obj, EFL_UI_RANGE_EVENT_CHANGED, NULL);

   _label_write(obj, sd);
}

EOLIAN static double
_efl_ui_spin_efl_ui_range_display_range_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Spin_Data *sd)
{
   return sd->val;
}

#include "efl_ui_spin.eo.c"
