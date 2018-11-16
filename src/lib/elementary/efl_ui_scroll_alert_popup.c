#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_UI_SCROLL_ALERT_POPUP_BETA
#define EFL_PART_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_private.h"
#include "efl_ui_scroll_alert_popup_private.h"
#include "efl_ui_scroll_alert_popup_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_SCROLL_ALERT_POPUP_CLASS
#define MY_CLASS_NAME "Efl.Ui.Scroll_Alert_Popup"

static void
_scroller_sizing_eval(Eo *obj, Efl_Ui_Scroll_Alert_Popup_Data *pd,
                      Eina_Size2D obj_min, Eina_Size2D scr_min)
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

   Eina_Size2D new_min = obj_min;

   if ((max_size.w == -1) && (max_size.h == -1))
     {
        elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
        efl_gfx_entity_size_set(obj, size);
     }
   else if ((max_size.w == -1) && (max_size.h != -1))
     {
        if (max_size.h < scr_min.h)
          {
             elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
             efl_gfx_entity_size_set(obj, EINA_SIZE2D(size.w, max_size.h));
          }
        else
          {
             new_min.h = scr_min.h;
             elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_TRUE);
             efl_gfx_entity_size_set(obj, EINA_SIZE2D(size.w, scr_min.h));
          }
     }
   else if ((max_size.w != -1) && (max_size.h == -1))
     {
        if (max_size.w < scr_min.w)
          {
             elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
             efl_gfx_entity_size_set(obj, EINA_SIZE2D(max_size.w, size.h));
          }
        else
          {
             new_min.w = scr_min.w;
             elm_scroller_content_min_limit(pd->scroller, EINA_TRUE, EINA_FALSE);
             efl_gfx_entity_size_set(obj, EINA_SIZE2D(scr_min.w, size.h));
          }
     }
   else if ((max_size.w != -1) && (max_size.h != -1))
     {
        Eina_Size2D new_size;
        Eina_Bool min_limit_w = EINA_FALSE;
        Eina_Bool min_limit_h = EINA_FALSE;

        if (max_size.w < scr_min.w)
          {
             new_size.w = max_size.w;
          }
        else
          {
             min_limit_w = EINA_TRUE;
             new_min.w = scr_min.w;
             new_size.w = scr_min.w;
          }

        if (max_size.h < scr_min.h)
          {
             new_size.h = max_size.h;
          }
        else
          {
             min_limit_h = EINA_TRUE;
             new_min.h = scr_min.h;
             new_size.h = scr_min.h;
          }

        elm_scroller_content_min_limit(pd->scroller, min_limit_w, min_limit_h);
        efl_gfx_entity_size_set(obj, new_size);
     }

   efl_gfx_size_hint_min_set(obj, new_min);
}

static void
_sizing_eval(Eo *obj, Efl_Ui_Scroll_Alert_Popup_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord obj_minw = -1, obj_minh = -1;
   Evas_Coord scr_minw = -1, scr_minh = -1;

   //Calculate popup's min size including scroller's min size
     {
        elm_scroller_content_min_limit(pd->scroller, EINA_TRUE, EINA_TRUE);

        elm_coords_finger_size_adjust(1, &scr_minw, 1, &scr_minh);
        edje_object_size_min_restricted_calc
           (wd->resize_obj, &scr_minw, &scr_minh, scr_minw, scr_minh);
     }

   //Calculate popup's min size except scroller's min size
     {
        elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);

        elm_coords_finger_size_adjust(1, &obj_minw, 1, &obj_minh);
        edje_object_size_min_restricted_calc
           (wd->resize_obj, &obj_minw, &obj_minh, obj_minw, obj_minh);
     }

   _scroller_sizing_eval(obj, pd, EINA_SIZE2D(obj_minw, obj_minh), EINA_SIZE2D(scr_minw, scr_minh));
}

EOLIAN static void
_efl_ui_scroll_alert_popup_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Scroll_Alert_Popup_Data *pd)
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
_efl_ui_scroll_alert_popup_content_set(Eo *obj, Efl_Ui_Scroll_Alert_Popup_Data *pd, const char *part, Eo *content)
{
   //For efl_content_set()
   if (part && !strcmp(part, "efl.content"))
     {
        pd->content = content;

        //Content should have expand propeties since the scroller is not layout layer
        efl_gfx_size_hint_weight_set(pd->content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        efl_gfx_size_hint_align_set(pd->content, EVAS_HINT_FILL, EVAS_HINT_FILL);

        efl_content_set(pd->scroller, pd->content);
     }
   else
     {
        efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
     }

   return EINA_TRUE;
}

Eo *
_efl_ui_scroll_alert_popup_content_get(Eo *obj, Efl_Ui_Scroll_Alert_Popup_Data *pd, const char *part)
{
   //For efl_content_set()
   if (part && !strcmp(part, "efl.content"))
     return pd->content;

   return efl_content_get(efl_part(efl_super(obj, MY_CLASS), part));
}

static Eo *
_efl_ui_scroll_alert_popup_content_unset(Eo *obj, Efl_Ui_Scroll_Alert_Popup_Data *pd, const char *part)
{
   //For efl_content_set()
   if (part && !strcmp(part, "efl.content"))
     {
        Eo *content = pd->content;
        if (!content) return content;

        pd->content = NULL;

        return efl_content_unset(pd->scroller);
     }

   return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
}

static Eina_Bool
_efl_ui_scroll_alert_popup_text_set(Eo *obj, Efl_Ui_Scroll_Alert_Popup_Data *pd EINA_UNUSED, const char *part, const char *label)
{
   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);

   return EINA_TRUE;
}

const char *
_efl_ui_scroll_alert_popup_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Alert_Popup_Data *pd EINA_UNUSED, const char *part)
{
   return efl_text_get(efl_part(efl_super(obj, MY_CLASS), part));
}

static void
_efl_ui_scroll_alert_popup_expandable_set(Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Alert_Popup_Data *pd, Eina_Size2D max_size)
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

static Eina_Size2D
_efl_ui_scroll_alert_popup_expandable_get(const Eo *obj EINA_UNUSED, Efl_Ui_Scroll_Alert_Popup_Data *pd)
{
   return pd->max_size;
}

EOLIAN static void
_efl_ui_scroll_alert_popup_efl_ui_popup_popup_size_set(Eo *obj, Efl_Ui_Scroll_Alert_Popup_Data *pd, Eina_Size2D size)
{
   pd->size = size;

   efl_gfx_entity_size_set(obj, size);

   elm_layout_sizing_eval(obj);
}

EOLIAN static Eo *
_efl_ui_scroll_alert_popup_efl_object_constructor(Eo *obj,
                                                  Efl_Ui_Scroll_Alert_Popup_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "scroll_alert_popup");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   elm_widget_sub_object_parent_add(obj);

   pd->scroller = elm_scroller_add(obj);
   elm_object_style_set(pd->scroller, "popup/no_inset_shadow");
   elm_scroller_policy_set(pd->scroller, ELM_SCROLLER_POLICY_AUTO,
                           ELM_SCROLLER_POLICY_AUTO);

   efl_content_set(efl_part(efl_super(obj, MY_CLASS), "efl.content"),
                   pd->scroller);

   pd->size = EINA_SIZE2D(0, 0);
   pd->max_size = EINA_SIZE2D(-1, -1);

   return obj;
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(efl_ui_scroll_alert_popup, EFL_UI_SCROLL_ALERT_POPUP, Efl_Ui_Scroll_Alert_Popup_Data)
ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_scroll_alert_popup, EFL_UI_SCROLL_ALERT_POPUP, Efl_Ui_Scroll_Alert_Popup_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_scroll_alert_popup, EFL_UI_SCROLL_ALERT_POPUP, Efl_Ui_Scroll_Alert_Popup_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_scroll_alert_popup, EFL_UI_SCROLL_ALERT_POPUP, Efl_Ui_Scroll_Alert_Popup_Data)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_scroll_alert_popup, EFL_UI_SCROLL_ALERT_POPUP, Efl_Ui_Scroll_Alert_Popup_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_scroll_alert_popup, EFL_UI_SCROLL_ALERT_POPUP, Efl_Ui_Scroll_Alert_Popup_Data)
#include "efl_ui_scroll_alert_popup_part.eo.c"

/* Efl.Part end */

#include "efl_ui_scroll_alert_popup.eo.c"
