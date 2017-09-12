#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED
#define ELM_INTERFACE_ATSPI_VALUE_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_slider_private.h"

#define MY_CLASS EFL_UI_SLIDER_INTERVAL_CLASS
#define MY_CLASS_NAME "Efl.Ui.Slider_Interval"

typedef struct
{
} Efl_Ui_Slider_Interval_Data;

EOLIAN static void
_efl_ui_slider_interval_interval_value_get(Eo *obj, Efl_Ui_Slider_Interval_Data *sd EINA_UNUSED, double *from, double *to)
{
   Efl_Ui_Slider_Data *pd =  efl_data_scope_safe_get(obj, EFL_UI_SLIDER_CLASS);
   if (from) *from = fmin(pd->intvl_from, pd->intvl_to);
   if (to) *to = fmax(pd->intvl_from, pd->intvl_to);
}

static inline Eina_Bool
_is_inverted(Efl_Ui_Dir dir)
{
   if ((dir == EFL_UI_DIR_LEFT) || (dir == EFL_UI_DIR_UP))
     return EINA_TRUE;

   return EINA_FALSE;
}

static void
_val_set(Evas_Object *obj)
{
   Eina_Bool rtl;
   double pos, pos2;

   EFL_UI_SLIDER_DATA_GET(obj, sd);

   if (sd->val_max > sd->val_min)
     {
        pos = (sd->val - sd->val_min) / (sd->val_max - sd->val_min);
        pos2 = (sd->intvl_to - sd->val_min) / (sd->val_max - sd->val_min);
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

   rtl = efl_ui_mirrored_get(obj);
   if ((!rtl && _is_inverted(sd->dir)) ||
       (rtl && ((sd->dir == EFL_UI_DIR_UP) ||
                (sd->dir == EFL_UI_DIR_RIGHT))))
     {
        pos = 1.0 - pos;
        pos2 = 1.0 - pos2;
     }

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   edje_object_part_drag_value_set
      (wd->resize_obj, "elm.dragable.slider", pos, pos);

   if (sd->intvl_enable)
     edje_object_part_drag_value_set
        (wd->resize_obj, "elm.dragable2.slider", pos2, pos2);

   // emit accessiblity event also if value was chagend by API
   if (_elm_config->atspi_mode)
     elm_interface_atspi_accessible_value_changed_signal_emit(obj);
}

static void
_visuals_refresh(Eo *obj)
{
   _val_set(obj);
   evas_object_smart_changed(obj);    
}

EOLIAN static void
_efl_ui_slider_interval_interval_value_set(Eo *obj, Efl_Ui_Slider_Interval_Data *sd EINA_UNUSED, double from, double to)
{
   Efl_Ui_Slider_Data *pd =  efl_data_scope_safe_get(obj, EFL_UI_SLIDER_CLASS);

   pd->intvl_from = from;
   pd->val = from;
   pd->intvl_to = to;

   if (pd->intvl_from < pd->val_min) pd->intvl_from = pd->val_min;
   if (pd->intvl_to > pd->val_max) pd->intvl_to = pd->val_max;

   _visuals_refresh(obj);
}

EOLIAN static Efl_Object *
_efl_ui_slider_interval_efl_object_finalize(Eo *obj, Efl_Ui_Slider_Interval_Data *sd EINA_UNUSED)
{
   Efl_Ui_Slider_Data *pd =  efl_data_scope_safe_get(obj, EFL_UI_SLIDER_CLASS);

   pd->intvl_enable = EINA_TRUE;

   elm_obj_widget_theme_apply(obj);

   elm_layout_signal_emit(obj, "elm,slider,range,enable", "elm");
   if (pd->indicator_show)
     edje_object_signal_emit(pd->popup2, "elm,state,val,show", "elm");

   obj = efl_finalize(efl_super(obj, MY_CLASS));

   return obj;
}

#include "efl_ui_slider_interval.eo.c"
