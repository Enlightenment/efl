#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_alert_scroll_private.h"
#include "efl_ui_popup_alert_scroll_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_POPUP_ALERT_SCROLL_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup_Alert_Scroll"

static const char PART_NAME_SCROLLER[] = "scroller";

EOLIAN static void
_efl_ui_popup_alert_scroll_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd EINA_UNUSED)
{
   elm_layout_sizing_eval(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
      (wd->resize_obj, &minw, &minh, minw, minh);
   efl_gfx_size_hint_min_set(obj, EINA_SIZE2D(minw, minh));
}

static Eina_Bool
_efl_ui_popup_alert_scroll_content_set(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd, const char *part, Eo *content)
{
   //For efl_content_set()
   if (part && !strcmp(part, "elm.swallow.content"))
     {
        pd->content = content;

        //Content should have expand propeties since the scroller is not layout layer
        efl_gfx_size_hint_weight_set(pd->content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        efl_gfx_size_hint_align_set(pd->content, EVAS_HINT_FILL, EVAS_HINT_FILL);

        efl_content_set(efl_part(pd->scroller, "default"), pd->content);
     }
   else
     {
        efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
     }

   return EINA_TRUE;
}

Eo *
_efl_ui_popup_alert_scroll_content_get(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd, const char *part)
{
   //For efl_content_set()
   if (part && !strcmp(part, "elm.swallow.content"))
     return pd->content;

   return efl_content_get(efl_part(efl_super(obj, MY_CLASS), part));
}

static Eo *
_efl_ui_popup_alert_scroll_content_unset(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd, const char *part)
{
   //For efl_content_set()
   if (part && !strcmp(part, "elm.swallow.content"))
     {
        Eo *content = pd->content;
        if (!content) return content;

        pd->content = NULL;

        return efl_content_unset(efl_part(pd->scroller, "default"));
     }

   return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
}

static Eina_Bool
_efl_ui_popup_alert_scroll_text_set(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd EINA_UNUSED, const char *part, const char *label)
{
   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), label);

   return EINA_TRUE;
}

const char *
_efl_ui_popup_alert_scroll_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Alert_Scroll_Data *pd EINA_UNUSED, const char *part)
{
   return efl_text_get(efl_part(efl_super(obj, MY_CLASS), part));
}

EOLIAN static Eo *
_efl_ui_popup_alert_scroll_efl_object_constructor(Eo *obj,
                                                  Efl_Ui_Popup_Alert_Scroll_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "popup_alert_scroll");
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);

   elm_widget_sub_object_parent_add(obj);

   // TODO: Change internal component to Efl.Ui.Widget
   pd->scroller = elm_scroller_add(obj);
   elm_object_style_set(pd->scroller, "popup/no_inset_shadow");
   //elm_widget_element_update(obj, pd->scroller, PART_NAME_SCROLLER);
   elm_scroller_policy_set(pd->scroller, ELM_SCROLLER_POLICY_AUTO,
                           ELM_SCROLLER_POLICY_AUTO);

   efl_content_set(efl_part(efl_super(obj, MY_CLASS), "elm.swallow.content"),
                   pd->scroller);

   return obj;
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(efl_ui_popup_alert_scroll, EFL_UI_POPUP_ALERT_SCROLL, Efl_Ui_Popup_Alert_Scroll_Data)
ELM_PART_OVERRIDE_CONTENT_SET(efl_ui_popup_alert_scroll, EFL_UI_POPUP_ALERT_SCROLL, Efl_Ui_Popup_Alert_Scroll_Data)
ELM_PART_OVERRIDE_CONTENT_GET(efl_ui_popup_alert_scroll, EFL_UI_POPUP_ALERT_SCROLL, Efl_Ui_Popup_Alert_Scroll_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(efl_ui_popup_alert_scroll, EFL_UI_POPUP_ALERT_SCROLL, Efl_Ui_Popup_Alert_Scroll_Data)
ELM_PART_OVERRIDE_TEXT_SET(efl_ui_popup_alert_scroll, EFL_UI_POPUP_ALERT_SCROLL, Efl_Ui_Popup_Alert_Scroll_Data)
ELM_PART_OVERRIDE_TEXT_GET(efl_ui_popup_alert_scroll, EFL_UI_POPUP_ALERT_SCROLL, Efl_Ui_Popup_Alert_Scroll_Data)
#include "efl_ui_popup_alert_scroll_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define EFL_UI_POPUP_ALERT_SCROLL_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup_alert_scroll)

#include "efl_ui_popup_alert_scroll.eo.c"
