#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_alert_text_private.h"
#include "efl_ui_popup_alert_text_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_POPUP_ALERT_TEXT_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup.Alert.Text"

static void
_scroller_sizing_eval(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd, Evas_Coord minh)
{
   Eina_Rect geom = efl_gfx_geometry_get(obj);

   if (pd->is_expandable_h)
     {
        if ((pd->max_scroll_h > -1) && (minh > pd->max_scroll_h))
          {
             elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
             efl_gfx_size_set(obj, EINA_SIZE2D(geom.w, pd->max_scroll_h));
          }
     }
}

EOLIAN static void
_efl_ui_popup_alert_text_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd)
{
   elm_layout_sizing_eval(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->resize_obj, &minw, &minh, minw, minh);
   efl_gfx_size_hint_min_set(obj, EINA_SIZE2D(minw, minh));

   _scroller_sizing_eval(obj, pd, minh);
}

static Eina_Bool
_efl_ui_popup_alert_text_content_set(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd EINA_UNUSED, const char *part, Eo *content)
{
   return efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
}

Eo *
_efl_ui_popup_alert_text_content_get(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd EINA_UNUSED, const char *part)
{
   return efl_content_get(efl_part(efl_super(obj, MY_CLASS), part));
}

static Eo *
_efl_ui_popup_alert_text_content_unset(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd EINA_UNUSED, const char *part)
{
   return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
}

static Eina_Bool
_efl_ui_popup_alert_text_text_set(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd, const char *part, const char *label)
{
   if (part && !strcmp(part, "elm.text"))
     {
        if (!pd->message)
          {
             pd->message = elm_label_add(obj);
             elm_label_line_wrap_set(pd->message, ELM_WRAP_MIXED);
             efl_gfx_size_hint_weight_set(pd->message, EVAS_HINT_EXPAND,
                                          EVAS_HINT_EXPAND);
             efl_content_set(efl_part(pd->scroller, "default"), pd->message);
          }
        elm_object_text_set(pd->message, label);
        elm_layout_sizing_eval(obj);
     }
   else
     efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);

   return EINA_TRUE;
}

const char *
_efl_ui_popup_alert_text_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Alert_Text_Data *pd, const char *part)
{
   if (part && !strcmp(part, "elm.text"))
     {
        if (pd->message)
          return elm_object_text_get(pd->message);

        return NULL;
     }

   return efl_text_get(efl_part(efl_super(obj, MY_CLASS), part));
}

EOLIAN static void
_efl_ui_popup_alert_text_efl_text_text_set(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd, const char *label)
{
   _efl_ui_popup_alert_text_text_set(obj, pd, "elm.text", label);
}

EOLIAN static const char*
_efl_ui_popup_alert_text_efl_text_text_get(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd)
{
   return _efl_ui_popup_alert_text_text_get(obj, pd, "elm.text");
}

static void
_efl_ui_popup_alert_text_expandable_set(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Alert_Text_Data *pd, Eina_Bool is_expandable_h)
{
   if (is_expandable_h)
     {
        pd->is_expandable_h = EINA_TRUE;
        elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_TRUE);
     }
   else
     {
        pd->is_expandable_h = EINA_FALSE;
        elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
     }
}

static void
_efl_ui_popup_alert_text_efl_gfx_size_hint_hint_max_set(Eo *obj, Efl_Ui_Popup_Alert_Text_Data *pd, Eina_Size2D size)
{
   efl_gfx_size_hint_max_set(efl_super(obj, MY_CLASS), size);
   pd->max_scroll_h = size.h;
   elm_layout_sizing_eval(obj);
}

EOLIAN static Eo *
_efl_ui_popup_alert_text_efl_object_constructor(Eo *obj,
                                                Efl_Ui_Popup_Alert_Text_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   elm_widget_sub_object_parent_add(obj);

   pd->scroller = elm_scroller_add(obj);
   elm_object_style_set(pd->scroller, "popup/no_inset_shadow");
   elm_scroller_policy_set(pd->scroller, ELM_SCROLLER_POLICY_OFF,
                           ELM_SCROLLER_POLICY_AUTO);

   efl_content_set(efl_part(efl_super(obj, MY_CLASS), "elm.swallow.content"),
                   pd->scroller);

   pd->max_scroll_h = -1;

   return obj;
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(efl_ui_popup_alert_text, EFL_UI_POPUP_ALERT_TEXT, Efl_Ui_Popup_Alert_Text_Data)
ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_popup_alert_text, EFL_UI_POPUP_ALERT_TEXT, Efl_Ui_Popup_Alert_Text_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_popup_alert_text, EFL_UI_POPUP_ALERT_TEXT, Efl_Ui_Popup_Alert_Text_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_popup_alert_text, EFL_UI_POPUP_ALERT_TEXT, Efl_Ui_Popup_Alert_Text_Data)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_popup_alert_text, EFL_UI_POPUP_ALERT_TEXT, Efl_Ui_Popup_Alert_Text_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_popup_alert_text, EFL_UI_POPUP_ALERT_TEXT, Efl_Ui_Popup_Alert_Text_Data)
#include "efl_ui_popup_alert_text_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_POPUP_ALERT_TEXT_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup_alert_text)

#include "efl_ui_popup_alert_text.eo.c"
