#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_slider_private.h"
#include "efl_ui_slider_interval_private.h"

#define MY_CLASS EFL_UI_SLIDER_INTERVAL_CLASS
#define MY_CLASS_PFX efl_ui_slider_interval
#define MY_CLASS_NAME "Efl.Ui.Slider_Interval"

#define SLIDER_DELAY_CHANGED_INTERVAL 1.2

static Eina_Bool
_delay_change(void *data)
{
   EFL_UI_SLIDER_INTERVAL_DATA_GET(data, pd);

   pd->delay = NULL;
   efl_event_callback_call(data, EFL_UI_SLIDER_EVENT_DELAY_CHANGED, NULL);

   if (_elm_config->atspi_mode)
     efl_access_value_changed_signal_emit(data);

   return ECORE_CALLBACK_CANCEL;
}

void
_efl_ui_slider_interval_val_fetch(Evas_Object *obj, Efl_Ui_Slider_Interval_Data *pd, Eina_Bool user_event)
{
   double posx = 0.0, posy = 0.0, pos = 0.0, val;
   double posx2 = 0.0, posy2 = 0.0, pos2 = 0.0, val2;

   EFL_UI_SLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (elm_widget_is_legacy(obj))
     efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable.slider"),
                           &posx, &posy);
   else
     efl_ui_drag_value_get(efl_part(wd->resize_obj, "efl.dragable.slider"),
                           &posx, &posy);
   if (efl_ui_layout_orientation_is_horizontal(sd->dir, EINA_TRUE)) pos = posx;
   else pos = posy;

   if (elm_widget_is_legacy(obj))
     efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                           &posx2, &posy2);
   else
     efl_ui_drag_value_get(efl_part(wd->resize_obj, "efl.dragable2.slider"),
                           &posx2, &posy2);
   if (efl_ui_layout_orientation_is_horizontal(sd->dir, EINA_TRUE)) pos2 = posx2;
   else pos2 = posy2;

   val = (pos * (sd->val_max - sd->val_min)) + sd->val_min;
   val2 = (pos2 * (sd->val_max - sd->val_min)) + sd->val_min;

   if (val > pd->intvl_to)
     {
        val = pd->intvl_to;
        efl_ui_slider_val_set(obj);
     }
   else if (val2 < pd->intvl_from)
     {
        val2 = pd->intvl_from;
        efl_ui_slider_val_set(obj);
     }

   if (fabs(val - pd->intvl_from) > DBL_EPSILON)
     {
        sd->val = val;
        pd->intvl_from = val;
        if (user_event)
          {
             efl_event_callback_call(obj, EFL_UI_SLIDER_EVENT_CHANGED, NULL);
             efl_event_callback_legacy_call(obj, EFL_UI_SLIDER_EVENT_CHANGED, NULL);
             ecore_timer_del(pd->delay);
             pd->delay = ecore_timer_add(SLIDER_DELAY_CHANGED_INTERVAL, _delay_change, obj);
          }
     }

   if (fabs(val2 - pd->intvl_to) > DBL_EPSILON)
     {
        pd->intvl_to = val2;
        if (user_event)
          {
             efl_event_callback_call(obj, EFL_UI_SLIDER_EVENT_CHANGED, NULL);
             efl_event_callback_legacy_call(obj, EFL_UI_SLIDER_EVENT_CHANGED, NULL);
             ecore_timer_del(pd->delay);
             pd->delay = ecore_timer_add(SLIDER_DELAY_CHANGED_INTERVAL, _delay_change, obj);
          }
     }
}

void
_efl_ui_slider_interval_val_set(Evas_Object *obj, Efl_Ui_Slider_Interval_Data *pd)
{
   double pos, pos2;

   EFL_UI_SLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->val_max > sd->val_min)
     {
        pos = (pd->intvl_from - sd->val_min) / (sd->val_max - sd->val_min);
        pos2 = (pd->intvl_to - sd->val_min) / (sd->val_max - sd->val_min);
     }
   else
     {
        pos = 0.0;
        pos2 = 0.0;
     }

   if (pos < 0.0) pos = 0.0;
   else if (pos > 1.0)
     pos = 1.0;

   if (pos2 < 0.0) pos2 = 0.0;
   else if (pos2 > 1.0)
     pos2 = 1.0;

   if (elm_widget_is_legacy(obj))
     {
        efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                              pos, pos);
        efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                              pos2, pos2);
     }
   else
     {
        efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable.slider"),
                              pos, pos);
        efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable2.slider"),
                              pos2, pos2);
     }

   // emit accessibility event also if value was changed by API
   if (_elm_config->atspi_mode)
     efl_access_value_changed_signal_emit(obj);

   evas_object_smart_changed(obj);
}

void
_efl_ui_slider_interval_down_knob(Evas_Object *obj, Efl_Ui_Slider_Interval_Data *pd, double button_x, double button_y)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   double posx = 0.0, posy = 0.0, posx2 = 0.0, posy2 = 0.0, diff1, diff2, diff3;

   pd->intvl_flag = 0;

   if (elm_widget_is_legacy(obj))
     {
        efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable.slider"),
                              &posx, &posy);
        efl_ui_drag_value_get(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                              &posx2, &posy2);
     }
   else
     {
        efl_ui_drag_value_get(efl_part(wd->resize_obj, "efl.dragable.slider"),
                              &posx, &posy);
        efl_ui_drag_value_get(efl_part(wd->resize_obj, "efl.dragable2.slider"),
                              &posx2, &posy2);
     }

   if (efl_ui_layout_orientation_is_horizontal(sd->dir, EINA_TRUE))
     {
        diff1 = fabs(button_x - posx);
        diff2 = fabs(button_x - posx2);
        diff3 = button_x - posx;
     }
   else
     {
        diff1 = fabs(button_y - posy);
        diff2 = fabs(button_y - posy2);
        diff3 = button_y - posy;
     }

   if (diff1 < diff2)
     {
        if (elm_widget_is_legacy(obj))
          efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                                button_x, button_y);
        else
          efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable.slider"),
                                button_x, button_y);
        pd->intvl_flag = 1;
     }
   else if (diff1 > diff2)
     {
        if (elm_widget_is_legacy(obj))
          efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                                button_x, button_y);
        else
          efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable2.slider"),
                                button_x, button_y);
        pd->intvl_flag = 2;
     }
   else
     {
        if (diff3 < 0)
          {
             if (elm_widget_is_legacy(obj))
               efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                                     button_x, button_y);
             else
               efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable.slider"),
                                     button_x, button_y);
             pd->intvl_flag = 1;
          }
        else
          {
             if (elm_widget_is_legacy(obj))
               efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                                     button_x, button_y);
             else
               efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable2.slider"),
                                     button_x, button_y);
             pd->intvl_flag = 2;
          }
     }
}

void
_efl_ui_slider_interval_move_knob(Evas_Object *obj, Efl_Ui_Slider_Interval_Data *pd, double button_x, double button_y)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (pd->intvl_flag == 1)
     {
        if (elm_widget_is_legacy(obj))
          efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable.slider"),
                                button_x, button_y);
        else
          efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable.slider"),
                                button_x, button_y);
     }
   else if (pd->intvl_flag == 2)
     {
        if (elm_widget_is_legacy(obj))
          efl_ui_drag_value_set(efl_part(wd->resize_obj, "elm.dragable2.slider"),
                                button_x, button_y);
        else
          efl_ui_drag_value_set(efl_part(wd->resize_obj, "efl.dragable2.slider"),
                                button_x, button_y);
     }
}

EOLIAN static void
_efl_ui_slider_interval_interval_value_set(Eo *obj, Efl_Ui_Slider_Interval_Data *pd, double from, double to)
{
   EFL_UI_SLIDER_DATA_GET(obj, sd);
   pd->intvl_from = from;
   sd->val = from;
   pd->intvl_to = to;

   if (pd->intvl_from < sd->val_min) {
        pd->intvl_from = sd->val_min;
        sd->val = sd->val_min;
   }
   if (pd->intvl_to > sd->val_max) pd->intvl_to = sd->val_max;

   efl_ui_slider_val_set(obj);
}

EOLIAN static void
_efl_ui_slider_interval_interval_value_get(const Eo *obj EINA_UNUSED, Efl_Ui_Slider_Interval_Data *pd, double *from, double *to)
{
   if (from) *from = fmin(pd->intvl_from, pd->intvl_to);
   if (to) *to = fmax(pd->intvl_from, pd->intvl_to);
}

EOLIAN static Efl_Object *
_efl_ui_slider_interval_efl_object_constructor(Eo *obj, Efl_Ui_Slider_Interval_Data *pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "slider_interval");
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   return obj;
}

EOLIAN static void
_efl_ui_slider_interval_efl_object_destructor(Eo *obj,
                                      Efl_Ui_Slider_Interval_Data *pd)
{
   ecore_timer_del(pd->delay);

   efl_destructor(efl_super(obj, MY_CLASS));
}

#define EFL_UI_SLIDER_INTERVAL_EXTRA_OPS \
   EFL_UI_SLIDER_VAL_FETCH_OPS(efl_ui_slider_interval), \
   EFL_UI_SLIDER_VAL_SET_OPS(efl_ui_slider_interval), \
   EFL_UI_SLIDER_DOWN_KNOB_OPS(efl_ui_slider_interval), \
   EFL_UI_SLIDER_MOVE_KNOB_OPS(efl_ui_slider_interval), \

#include "efl_ui_slider_interval.eo.c"
