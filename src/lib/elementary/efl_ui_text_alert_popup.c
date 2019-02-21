#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_private.h"
#include "efl_ui_text_alert_popup_private.h"
#include "efl_ui_text_alert_popup_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_TEXT_ALERT_POPUP_CLASS
#define MY_CLASS_NAME "Efl.Ui.Text_Alert_Popup"

//static const char PART_NAME_TEXT[] = "text";

static void
_scroller_sizing_eval(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd, Eina_Size2D obj_min, Eina_Size2D text_min)
{
   Eina_Size2D max_size;
   max_size.w = -1;
   max_size.h = -1;

   if (pd->max_size.w != -1)
     max_size.w = (obj_min.w > pd->max_size.w) ? obj_min.w : pd->max_size.w;
   if (pd->max_size.h != -1)
     max_size.h = (obj_min.h > pd->max_size.h) ? obj_min.h : pd->max_size.h;

  Eina_Size2D size;
  size.w = (obj_min.w > pd->size.w) ? obj_min.w : pd->size.w;
  size.h = (obj_min.h > pd->size.h) ? obj_min.h : pd->size.h;

  text_min.w = (obj_min.w > text_min.w) ? obj_min.w : text_min.w;
  text_min.h = (obj_min.h > text_min.h) ? obj_min.h : text_min.h;

  Eina_Size2D new_min = obj_min;

  if ((max_size.w == -1) && (max_size.h == -1))
    {
       elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
       efl_gfx_entity_size_set(obj, size);
    }
  else if ((max_size.w == -1) && (max_size.h != -1))
    {
       if (max_size.h < text_min.h)
         {
            elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
            efl_gfx_entity_size_set(obj, EINA_SIZE2D(size.w, max_size.h));
         }
       else
         {
            new_min.h = text_min.h;
            elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_TRUE);
            efl_gfx_entity_size_set(obj, EINA_SIZE2D(size.w, text_min.h));
         }
    }
  else if ((max_size.w != -1) && (max_size.h == -1))
    {
       if (max_size.w < text_min.w)
         {
            elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
            efl_gfx_entity_size_set(obj, EINA_SIZE2D(max_size.w, size.h));
         }
       else
         {
            new_min.w = text_min.w;
            elm_scroller_content_min_limit(pd->scroller, EINA_TRUE, EINA_FALSE);
            efl_gfx_entity_size_set(obj, EINA_SIZE2D(text_min.w, size.h));
         }
    }
  else if ((max_size.w != -1) && (max_size.h != -1))
    {
       Eina_Size2D new_size;
       Eina_Bool min_limit_w = EINA_FALSE;
       Eina_Bool min_limit_h = EINA_FALSE;

       if (max_size.w < text_min.w)
         {
            new_size.w = max_size.w;
         }
       else
         {
            min_limit_w = EINA_TRUE;
            new_min.w = text_min.w;
            new_size.w = text_min.w;
         }

       if (max_size.h < text_min.h)
         {
            new_size.h = max_size.h;
         }
       else
         {
            min_limit_h = EINA_TRUE;
            new_min.h = text_min.h;
            new_size.h = text_min.h;
         }

        elm_scroller_content_min_limit(pd->scroller, min_limit_w, min_limit_h);
        efl_gfx_entity_size_set(obj, new_size);
    }

    efl_gfx_hint_size_min_set(obj, new_min);
}

EOLIAN static void
_efl_ui_text_alert_popup_efl_ui_popup_popup_size_set(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd, Eina_Size2D size)
{
   pd->size = size;

   efl_gfx_entity_size_set(obj, size);

   elm_layout_sizing_eval(obj);
}

static void
_sizing_eval(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord obj_minw = -1, obj_minh = -1;
   Evas_Coord text_minw = -1, text_minh = -1;
   Eina_Size2D text_min;

   //Calculate popup's min size including scroller's min size
     {
        elm_label_line_wrap_set(pd->message, ELM_WRAP_NONE);
        text_min = efl_gfx_hint_size_combined_min_get(pd->message);
        elm_label_line_wrap_set(pd->message, ELM_WRAP_MIXED);

        elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_TRUE);

        elm_coords_finger_size_adjust(1, &text_minw, 1, &text_minh);
        edje_object_size_min_restricted_calc
           (wd->resize_obj, &text_minw, &text_minh, text_minw, text_minh);
     }

   //Calculate popup's min size except scroller's min size
     {
        elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);

        elm_coords_finger_size_adjust(1, &obj_minw, 1, &obj_minh);
        edje_object_size_min_restricted_calc
           (wd->resize_obj, &obj_minw, &obj_minh, obj_minw, obj_minh);
     }

   text_min.h = text_minh;
   _scroller_sizing_eval(obj, pd, EINA_SIZE2D(obj_minw, obj_minh), text_min);
}

EOLIAN static void
_efl_ui_text_alert_popup_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd)
{
   /* When elm_layout_sizing_eval() is called, just flag is set instead of size
    * calculation.
    * The actual size calculation is done here when the object is rendered to
    * avoid duplicate size calculations. */
   EFL_UI_POPUP_DATA_GET_OR_RETURN(obj, ppd);

   if (ppd->needs_group_calc)
     {
        if (ppd->needs_size_calc)
          _sizing_eval(obj, pd);

        //Not to calculate size by super class
        ppd->needs_size_calc = EINA_FALSE;
        efl_canvas_group_calculate(efl_super(obj, MY_CLASS));
     }
}

static Eina_Bool
_efl_ui_text_alert_popup_content_set(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd EINA_UNUSED, const char *part, Eo *content)
{
   return efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
}

Eo *
_efl_ui_text_alert_popup_content_get(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd EINA_UNUSED, const char *part)
{
   return efl_content_get(efl_part(efl_super(obj, MY_CLASS), part));
}

static Eo *
_efl_ui_text_alert_popup_content_unset(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd EINA_UNUSED, const char *part)
{
   return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
}

static Eina_Bool
_efl_ui_text_alert_popup_text_set(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd, const char *part, const char *label)
{
   if (part && !strcmp(part, "efl.text"))
     {
        if (!pd->message)
          {
             // TODO: Change internal component to Efl.Ui.Widget
             pd->message = elm_label_add(obj);
             //elm_widget_element_update(obj, pd->message, PART_NAME_TEXT);
             efl_gfx_hint_weight_set(pd->message, EVAS_HINT_EXPAND,
                                          EVAS_HINT_EXPAND);
             efl_content_set(pd->scroller, pd->message);
          }
        elm_object_text_set(pd->message, label);
        elm_layout_sizing_eval(obj);
     }
   else
     efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);

   return EINA_TRUE;
}

const char *
_efl_ui_text_alert_popup_text_get(const Eo *obj EINA_UNUSED, Efl_Ui_Text_Alert_Popup_Data *pd, const char *part)
{
   if (part && !strcmp(part, "efl.text"))
     {
        if (pd->message)
          return elm_object_text_get(pd->message);

        return NULL;
     }

   return efl_text_get(efl_part(efl_super(obj, MY_CLASS), part));
}

EOLIAN static void
_efl_ui_text_alert_popup_efl_text_text_set(Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd, const char *label)
{
   _efl_ui_text_alert_popup_text_set(obj, pd, "efl.text", label);
}

EOLIAN static const char*
_efl_ui_text_alert_popup_efl_text_text_get(const Eo *obj, Efl_Ui_Text_Alert_Popup_Data *pd)
{
   return _efl_ui_text_alert_popup_text_get(obj, pd, "efl.text");
}

static void
_efl_ui_text_alert_popup_expandable_set(Eo *obj EINA_UNUSED, Efl_Ui_Text_Alert_Popup_Data *pd, Eina_Size2D max_size)
{
   Eina_Bool valid_max_w = EINA_FALSE;
   Eina_Bool valid_max_h = EINA_FALSE;

   if ((max_size.w == -1) || (max_size.w >= 0))
     valid_max_w = EINA_TRUE;

   if ((max_size.h == -1) || (max_size.h >= 0))
     valid_max_h = EINA_TRUE;

   if (!valid_max_w || !valid_max_h)
     {
        ERR("Invalid max size(%d, %d)!"
            "The max size should be equal to or bigger than 0. "
            "To disable expandable property, set -1 to the max size.",
            max_size.w, max_size.h);
        return;
     }

   pd->max_size = max_size;

   elm_layout_sizing_eval(obj);
}

EOLIAN static Eo *
_efl_ui_text_alert_popup_efl_object_constructor(Eo *obj,
                                                Efl_Ui_Text_Alert_Popup_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "scroll_alert_popup");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   elm_widget_sub_object_parent_add(obj);

   pd->scroller = elm_scroller_add(obj);
   elm_object_style_set(pd->scroller, "popup/no_inset_shadow");
   elm_scroller_policy_set(pd->scroller, ELM_SCROLLER_POLICY_OFF,
                           ELM_SCROLLER_POLICY_AUTO);

   efl_content_set(efl_part(efl_super(obj, MY_CLASS), "efl.content"),
                   pd->scroller);

   pd->size = EINA_SIZE2D(0, 0);
   pd->max_size = EINA_SIZE2D(-1, -1);

   return obj;
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(efl_ui_text_alert_popup, EFL_UI_TEXT_ALERT_POPUP, Efl_Ui_Text_Alert_Popup_Data)
ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_text_alert_popup, EFL_UI_TEXT_ALERT_POPUP, Efl_Ui_Text_Alert_Popup_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_text_alert_popup, EFL_UI_TEXT_ALERT_POPUP, Efl_Ui_Text_Alert_Popup_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_text_alert_popup, EFL_UI_TEXT_ALERT_POPUP, Efl_Ui_Text_Alert_Popup_Data)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_text_alert_popup, EFL_UI_TEXT_ALERT_POPUP, Efl_Ui_Text_Alert_Popup_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_text_alert_popup, EFL_UI_TEXT_ALERT_POPUP, Efl_Ui_Text_Alert_Popup_Data)
#include "efl_ui_text_alert_popup_part.eo.c"

/* Efl.Part end */

#include "efl_ui_text_alert_popup.eo.c"
