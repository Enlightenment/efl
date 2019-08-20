#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_UI_WIDGET_SCROLLABLE_CONTENT_PROTECTED
#include "elm_priv.h"

#define MY_CLASS EFL_UI_WIDGET_SCROLLABLE_CONTENT_MIXIN

#define MY_CLASS_NAME "Efl_Ui_Widget_Scrollable_Content"

typedef struct Efl_Ui_Widget_Scrollable_Content_Data
{
   Eo *scroller;
   Eina_Bool did_group_calc : 1;
} Efl_Ui_Widget_Scrollable_Content_Data;

static void
_scroller_sizing_eval(Eo *obj, Efl_Ui_Widget_Scrollable_Content_Data *pd,
                      Eina_Size2D obj_min, Eina_Size2D scr_min)
{
   Eina_Size2D max_size, min_size, size;
   max_size = efl_gfx_hint_size_max_get(obj);

   if (max_size.w != -1)
     max_size.w = (obj_min.w > max_size.w) ? obj_min.w : max_size.w;
   if (max_size.h != -1)
     max_size.h = (obj_min.h > max_size.h) ? obj_min.h : max_size.h;

   min_size = efl_gfx_hint_size_min_get(obj);

   size.w = (obj_min.w > min_size.w) ? obj_min.w : min_size.w;
   size.h = (obj_min.h > min_size.h) ? obj_min.h : min_size.h;

   Eina_Size2D new_min = obj_min;

   if ((max_size.w == -1) && (max_size.h == -1))
     {
        efl_ui_scrollable_match_content_set(pd->scroller, EINA_FALSE, EINA_FALSE);
     }
   else if ((max_size.w == -1) && (max_size.h != -1))
     {
        if (max_size.h < scr_min.h)
          {
             efl_ui_scrollable_match_content_set(pd->scroller, EINA_FALSE, EINA_FALSE);
             size = EINA_SIZE2D(size.w, max_size.h);
          }
        else
          {
             new_min.h = scr_min.h;
             efl_ui_scrollable_match_content_set(pd->scroller, EINA_FALSE, EINA_TRUE);
             size = EINA_SIZE2D(size.w, scr_min.h);
          }
     }
   else if ((max_size.w != -1) && (max_size.h == -1))
     {
        if (max_size.w < scr_min.w)
          {
             efl_ui_scrollable_match_content_set(pd->scroller, EINA_FALSE, EINA_FALSE);
             size = EINA_SIZE2D(max_size.w, size.h);
          }
        else
          {
             new_min.w = scr_min.w;
             efl_ui_scrollable_match_content_set(pd->scroller, EINA_TRUE, EINA_FALSE);
             size = EINA_SIZE2D(scr_min.w, size.h);
          }
     }
   else if ((max_size.w != -1) && (max_size.h != -1))
     {
        Eina_Bool min_limit_w = EINA_FALSE;
        Eina_Bool min_limit_h = EINA_FALSE;

        if (max_size.w < scr_min.w)
          {
             size.w = max_size.w;
          }
        else
          {
             min_limit_w = EINA_TRUE;
             new_min.w = scr_min.w;
             size.w = scr_min.w;
          }

        if (max_size.h < scr_min.h)
          {
             size.h = max_size.h;
          }
        else
          {
             min_limit_h = EINA_TRUE;
             new_min.h = scr_min.h;
             size.h = scr_min.h;
          }

        efl_ui_scrollable_match_content_set(pd->scroller, min_limit_w, min_limit_h);
     }
   /* this event must come before the scroller recalc in order to ensure the scroller has the correct viewport size */
   efl_event_callback_call(obj, EFL_UI_WIDGET_SCROLLABLE_CONTENT_EVENT_OPTIMAL_SIZE_CALC, &size);
   efl_canvas_group_calculate(pd->scroller);

   efl_gfx_hint_size_restricted_min_set(obj, new_min);
}

static void
_sizing_eval(Eo *obj, Efl_Ui_Widget_Scrollable_Content_Data *pd)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord obj_minw = -1, obj_minh = -1;
   Evas_Coord scr_minw = -1, scr_minh = -1;

   //Calculate popup's min size including scroller's min size
     {
        efl_ui_scrollable_match_content_set(pd->scroller, EINA_TRUE, EINA_TRUE);
        efl_canvas_group_calculate(pd->scroller);

        elm_coords_finger_size_adjust(1, &scr_minw, 1, &scr_minh);
        edje_object_size_min_restricted_calc
           (wd->resize_obj, &scr_minw, &scr_minh, scr_minw, scr_minh);
     }

   //Calculate popup's min size except scroller's min size
     {
        efl_ui_scrollable_match_content_set(pd->scroller, EINA_FALSE, EINA_FALSE);
        efl_canvas_group_calculate(pd->scroller);

        elm_coords_finger_size_adjust(1, &obj_minw, 1, &obj_minh);
        edje_object_size_min_restricted_calc
           (wd->resize_obj, &obj_minw, &obj_minh, obj_minw, obj_minh);
     }
   _scroller_sizing_eval(obj, pd, EINA_SIZE2D(obj_minw, obj_minh), EINA_SIZE2D(scr_minw, scr_minh));
}

EOLIAN static void
_efl_ui_widget_scrollable_content_efl_canvas_group_calculate(Eo *obj, Efl_Ui_Widget_Scrollable_Content_Data *pd)
{
   pd->did_group_calc = EINA_FALSE;
   if (!pd->scroller)
     {
        efl_canvas_group_calculate(efl_super(obj, MY_CLASS));
        return;
     }
   pd->did_group_calc = EINA_TRUE;
   _sizing_eval(obj, pd);
   efl_canvas_group_need_recalculate_set(pd->scroller, EINA_FALSE);
   efl_canvas_group_need_recalculate_set(obj, EINA_FALSE);
}

static void
_scroller_setup(Eo *obj, Efl_Ui_Widget_Scrollable_Content_Data *pd)
{
   pd->scroller = efl_add(EFL_UI_SCROLLER_CLASS, obj,
     efl_ui_widget_style_set(efl_added, "popup/no_inset_shadow")
     );
   efl_wref_add(pd->scroller, &pd->scroller);
   efl_content_set(obj, pd->scroller);
}

EOLIAN static Eina_Bool
_efl_ui_widget_scrollable_content_scrollable_content_did_group_calc_get(const Eo *obj EINA_UNUSED, Efl_Ui_Widget_Scrollable_Content_Data *pd)
{
   return pd->did_group_calc;
}

EOLIAN static Eina_Bool
_efl_ui_widget_scrollable_content_scrollable_content_set(Eo *obj, Efl_Ui_Widget_Scrollable_Content_Data *pd, Eo *content)
{
   Eina_Bool ret;

   if (!pd->scroller)
     _scroller_setup(obj, pd);
   ret = efl_content_set(pd->scroller, content);
   if (ret) efl_canvas_group_change(obj);
   return ret;
}

EOLIAN static Eo *
_efl_ui_widget_scrollable_content_scrollable_content_get(const Eo *obj EINA_UNUSED, Efl_Ui_Widget_Scrollable_Content_Data *pd)
{
   return efl_content_get(pd->scroller);
}

EOLIAN static void
_efl_ui_widget_scrollable_content_efl_object_destructor(Eo *obj, Efl_Ui_Widget_Scrollable_Content_Data *pd)
{
   if (pd->scroller) efl_wref_del(pd->scroller, &pd->scroller);
   efl_destructor(efl_super(obj, MY_CLASS));
}

#define EFL_UI_WIDGET_SCROLLABLE_CONTENT_EXTRA_OPS \
   EFL_CANVAS_GROUP_CALC_OPS(efl_ui_widget_scrollable_content)

#include "efl_ui_widget_scrollable_content.eo.c"
