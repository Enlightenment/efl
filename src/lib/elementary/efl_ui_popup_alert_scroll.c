#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "efl_ui_popup_alert_scroll_private.h"
#include "efl_ui_popup_alert_scroll_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS EFL_UI_POPUP_ALERT_SCROLL_CLASS
#define MY_CLASS_NAME "Efl.Ui.Popup.Alert.Scroll"

static void
_scroller_sizing_eval(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd, Eina_Size2D min)
{
   Eina_Rect geom = efl_gfx_geometry_get(obj);

   pd->is_sizing_eval = EINA_TRUE;

   if (pd->is_expandable_w && !pd->is_expandable_h)
     {
        if ((pd->max_scroll.w > -1) && (min.w > pd->max_scroll.w))
          {
             elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
             efl_gfx_size_set(obj, EINA_SIZE2D(pd->max_scroll.w, geom.h));
          }
     }
   else if (!pd->is_expandable_w && pd->is_expandable_h)
     {
        if ((pd->max_scroll.h > -1) && (min.h > pd->max_scroll.h))
          {
             elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
             efl_gfx_size_set(obj, EINA_SIZE2D(geom.w, pd->max_scroll.h));
          }
     }
   else if (pd->is_expandable_w && pd->is_expandable_h)
     {
        Eina_Bool wdir, hdir;
        wdir = hdir = EINA_FALSE;

        if ((pd->max_scroll.w > -1) && (min.w > pd->max_scroll.w))
          wdir = 1;
        if ((pd->max_scroll.h > -1) && (min.h > pd->max_scroll.h))
          hdir = 1;
        if (wdir && !hdir)
          {
             elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_TRUE);
             efl_gfx_size_set(obj, EINA_SIZE2D(pd->max_scroll.w, geom.h));
          }
        else if (!wdir && hdir)
          {
             elm_scroller_content_min_limit(pd->scroller, EINA_TRUE, EINA_FALSE);
             efl_gfx_size_set(obj, EINA_SIZE2D(geom.w, pd->max_scroll.h));
          }
        else if(wdir && hdir)
          {
             elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
             efl_gfx_size_set(obj, pd->max_scroll);
          }
     }

   pd->is_sizing_eval = EINA_FALSE;
}

EOLIAN static void
_efl_ui_popup_alert_scroll_elm_layout_sizing_eval(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd)
{
   if (pd->is_sizing_eval) return;

   elm_layout_sizing_eval(efl_super(obj, MY_CLASS));

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw = -1, minh = -1;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
      (wd->resize_obj, &minw, &minh, minw, minh);
   efl_gfx_size_hint_min_set(obj, EINA_SIZE2D(minw, minh));

   _scroller_sizing_eval(obj, pd, EINA_SIZE2D(minw, minh));
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

static void
_efl_ui_popup_alert_scroll_expandable_set(Eo *obj EINA_UNUSED, Efl_Ui_Popup_Alert_Scroll_Data *pd, Eina_Bool is_expandable_w, Eina_Bool is_expandable_h)
{
   if (is_expandable_w && !is_expandable_h)
     {
        pd->is_expandable_w = EINA_TRUE;
        pd->is_expandable_h = EINA_FALSE;
        elm_scroller_content_min_limit(pd->scroller, EINA_TRUE, EINA_FALSE);
     }
   else if(!is_expandable_w && is_expandable_h)
     {
        pd->is_expandable_w = EINA_FALSE;
        pd->is_expandable_h = EINA_TRUE;
        elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_TRUE);
     }
   else if(is_expandable_w && is_expandable_h)
     {
        pd->is_expandable_w = EINA_TRUE;
        pd->is_expandable_h = EINA_TRUE;
        elm_scroller_content_min_limit(pd->scroller, EINA_TRUE, EINA_TRUE);
     }
   else
     {
        pd->is_expandable_w = EINA_FALSE;
        pd->is_expandable_h = EINA_FALSE;
        elm_scroller_content_min_limit(pd->scroller, EINA_FALSE, EINA_FALSE);
     }
}

static void
_efl_ui_popup_alert_scroll_efl_gfx_size_hint_hint_max_set(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd, Eina_Size2D size)
{
   efl_gfx_size_hint_max_set(efl_super(obj, MY_CLASS), size);
   pd->max_scroll = size;
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_efl_ui_popup_alert_scroll_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   pd->scroller = elm_scroller_add(obj);
   elm_object_style_set(pd->scroller, "popup/no_inset_shadow");
   elm_scroller_policy_set(pd->scroller, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);

   efl_content_set(efl_part(efl_super(obj, MY_CLASS), "elm.swallow.content"), pd->scroller);

   pd->max_scroll = EINA_SIZE2D(-1, -1);
}

EOLIAN static void
_efl_ui_popup_alert_scroll_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Popup_Alert_Scroll_Data *pd EINA_UNUSED)
{
   efl_canvas_group_del(efl_super(obj, MY_CLASS));
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
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_popup_alert_scroll), \
   ELM_LAYOUT_SIZING_EVAL_OPS(efl_ui_popup_alert_scroll)

#include "efl_ui_popup_alert_scroll.eo.c"
