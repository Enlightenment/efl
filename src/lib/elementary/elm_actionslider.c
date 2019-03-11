#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define ELM_LAYOUT_PROTECTED
#define EFL_PART_PROTECTED

#include <Elementary.h>
#include <math.h>
#include "elm_priv.h"
#include "elm_widget_actionslider.h"
#include "elm_widget_layout.h"

#include "elm_actionslider_eo.h"

#include "elm_actionslider_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_ACTIONSLIDER_CLASS
#define MY_CLASS_FPX elm_actionslider

#define MY_CLASS_NAME "Elm_Actionslider"
#define MY_CLASS_NAME_LEGACY "elm_actionslider"

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"indicator", "elm.text.indicator"},
   {"left", "elm.text.left"},
   {"right", "elm.text.right"},
   {"center", "elm.text.center"},
   {NULL, NULL}
};

#define ELM_PRIV_ACTIONSLIDER_SIGNALS(cmd) \
   cmd(SIG_CHANGED, "pos_changed", "") \
   cmd(SIG_SELECTED, "selected", "")

ELM_PRIV_ACTIONSLIDER_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   ELM_PRIV_ACTIONSLIDER_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};
#undef ELM_PRIV_ACTIONSLIDER_SIGNALS

static Elm_Actionslider_Pos
_get_pos_by_orientation(const Evas_Object *obj,
                        Elm_Actionslider_Pos pos)
{
   if (efl_ui_mirrored_get(obj))
     {
        switch (pos)
          {
           case ELM_ACTIONSLIDER_LEFT:
             pos = ELM_ACTIONSLIDER_RIGHT;
             break;

           case ELM_ACTIONSLIDER_RIGHT:
             pos = ELM_ACTIONSLIDER_LEFT;
             break;

           default:
             break;
          }
     }
   return pos;
}

EOLIAN static void
_elm_actionslider_elm_layout_sizing_eval(Eo *obj, Elm_Actionslider_Data *sd)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->drag_button_base, minw, minh);
   evas_object_size_hint_max_set(sd->drag_button_base, -1, -1);

   minw = -1;
   minh = -1;
   elm_coords_finger_size_adjust(3, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_mirroredness_change_eval(Evas_Object *obj)
{
   double pos;
   char *left;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   left = (char *)elm_layout_text_get(obj, "elm.text.left");
   if (left) left = strdup(left);

   elm_layout_text_set
     (obj, "elm.text.left", elm_layout_text_get(obj, "elm.text.right"));
   elm_layout_text_set(obj, "elm.text.right", left);

   free(left);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.drag_button_base", &pos, NULL);
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.drag_button_base", 1.0 - pos, 0.5);
}

EOLIAN static Eina_Error
_elm_actionslider_efl_ui_widget_theme_apply(Eo *obj, Elm_Actionslider_Data *sd EINA_UNUSED)
{
   Eina_Bool mirrored;
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);

   mirrored = elm_object_mirrored_get(obj);

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   if (elm_object_mirrored_get(obj) != mirrored)
     _mirroredness_change_eval(obj);

   edje_object_message_signal_process(wd->resize_obj);

   return int_ret;
}

static void
_drag_button_down_cb(void *data,
                     Evas_Object *o EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   Elm_Actionslider_Data *sd = efl_data_scope_get(data, MY_CLASS);

   sd->mouse_down = EINA_TRUE;
}

static void
_drag_button_move_cb(void *data,
                     Evas_Object *o EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   Evas_Object *obj = data;
   double pos = 0.0;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!sd->mouse_down) return;

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.drag_button_base", &pos, NULL);
   if (EINA_DBL_EQ(pos, 0.0))
     efl_event_callback_legacy_call
       (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, !efl_ui_mirrored_get(obj) ? "left" : "right");
   else if (EINA_DBL_EQ(pos, 1.0))
     efl_event_callback_legacy_call
       (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, !efl_ui_mirrored_get(obj) ? "right" : "left");
   else if (pos >= 0.45 && pos <= 0.55)
     efl_event_callback_legacy_call
       (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, "center");
}

static void
_text_get(const Evas_Object *obj,
          const char **left,
          const char **right,
          const char **center)
{
   /* mirroredness already evaluated by the two calls below */
   *left = elm_layout_text_get(obj, "left");
   *right = elm_layout_text_get(obj, "right");
   *center = elm_layout_text_get(obj, "center");
}

static Eina_Bool
_button_animator(void *data)
{
   Evas_Object *obj = data;
   double move_amount = 0.05;
   Eina_Bool flag_finish_animation = EINA_FALSE;
   double cur_position = 0.0, new_position = 0.0;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ECORE_CALLBACK_CANCEL);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.drag_button_base",
     &cur_position, NULL);
   {
      double adjusted_final;

      adjusted_final = (!efl_ui_mirrored_get(obj)) ?
        sd->final_position : 1.0 - sd->final_position;

      if ((EINA_DBL_EQ(adjusted_final, 0.0)) ||
          (EINA_DBL_EQ(adjusted_final, 0.5) && cur_position >= adjusted_final))
        {
           new_position = cur_position - move_amount;

           if (new_position <= adjusted_final)
             {
                new_position = adjusted_final;
                flag_finish_animation = EINA_TRUE;
             }
        }
      else if ((EINA_DBL_EQ(adjusted_final, 1.0)) ||
               (EINA_DBL_EQ(adjusted_final, 0.5) && cur_position < adjusted_final))
        {
           new_position = cur_position + move_amount;

           if (new_position >= adjusted_final)
             {
                new_position = adjusted_final;
                flag_finish_animation = EINA_TRUE;
             }
        }
      edje_object_part_drag_value_set
        (wd->resize_obj, "elm.drag_button_base",
        new_position, 0.5);
   }

   if (flag_finish_animation)
     {
        const char *left, *right, *center;

        _text_get(obj, &left, &right, &center);

        if ((!EINA_DBL_EQ(sd->final_position, 0)) &&
            (sd->enabled_position & ELM_ACTIONSLIDER_LEFT))
          efl_event_callback_legacy_call
            (obj, EFL_UI_EVENT_SELECTABLE_SELECTED, (char *)left);
        else if ((EINA_DBL_EQ(sd->final_position, 0.5)) &&
                 (sd->enabled_position & ELM_ACTIONSLIDER_CENTER))
          efl_event_callback_legacy_call
            (obj, EFL_UI_EVENT_SELECTABLE_SELECTED, (char *)center);
        else if ((EINA_DBL_EQ(sd->final_position, 1)) &&
                 (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
          efl_event_callback_legacy_call
            (obj, EFL_UI_EVENT_SELECTABLE_SELECTED, (char *)right);

        sd->button_animator = NULL;

        return ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static void
_drag_button_up_cb(void *data,
                   Evas_Object *o EINA_UNUSED,
                   const char *emission EINA_UNUSED,
                   const char *source EINA_UNUSED)
{
   Evas_Object *obj = data;
   double position = 0.0;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   sd->mouse_down = EINA_FALSE;

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.drag_button_base", &position,
     NULL);

   const char *left, *right, *center;

   _text_get(obj, &left, &right, &center);

   if ((sd->enabled_position & ELM_ACTIONSLIDER_LEFT) &&
       ((!efl_ui_mirrored_get(obj) && EINA_DBL_EQ(position, 0.0)) ||
        (efl_ui_mirrored_get(obj) && EINA_DBL_EQ(position, 1.0))))
     {
        sd->final_position = 0;
        efl_event_callback_legacy_call
          (obj, EFL_UI_EVENT_SELECTABLE_SELECTED, (char *)left);

        return;
     }

   if (position >= 0.45 && position <= 0.55 &&
       (sd->enabled_position & ELM_ACTIONSLIDER_CENTER))
     {
        sd->final_position = 0.5;
        efl_event_callback_legacy_call
          (obj, EFL_UI_EVENT_SELECTABLE_SELECTED, (char *)center);

        ecore_animator_del(sd->button_animator);
        sd->button_animator = ecore_evas_animator_add(obj, _button_animator, obj);

        return;
     }

   if ((sd->enabled_position & ELM_ACTIONSLIDER_RIGHT) &&
       ((!efl_ui_mirrored_get(obj) && EINA_DBL_EQ(position, 1)) ||
        (efl_ui_mirrored_get(obj) && EINA_DBL_EQ(position, 0))))
     {
        sd->final_position = 1;
        efl_event_callback_legacy_call
          (obj, EFL_UI_EVENT_SELECTABLE_SELECTED, (char *)right);
        return;
     }

   if (sd->magnet_position == ELM_ACTIONSLIDER_NONE) return;

#define _FIX_POS_ON_MIRROREDNESS(x) \
  ((!efl_ui_mirrored_get(obj)) ? x : 1.0 - x)

   position = _FIX_POS_ON_MIRROREDNESS(position);

   if (position < 0.3)
     {
        if (sd->magnet_position & ELM_ACTIONSLIDER_LEFT)
          sd->final_position = 0;
        else if (sd->magnet_position & ELM_ACTIONSLIDER_CENTER)
          sd->final_position = 0.5;
        else if (sd->magnet_position & ELM_ACTIONSLIDER_RIGHT)
          sd->final_position = 1;
     }
   else if ((position >= 0.3) && (position <= 0.7))
     {
        if (sd->magnet_position & ELM_ACTIONSLIDER_CENTER)
          sd->final_position = 0.5;
        else if (position < 0.5)
          {
             if (sd->magnet_position & ELM_ACTIONSLIDER_LEFT)
               sd->final_position = 0;
             else
               sd->final_position = 1;
          }
        else
          {
             if (sd->magnet_position & ELM_ACTIONSLIDER_RIGHT)
               sd->final_position = 1;
             else
               sd->final_position = 0;
          }
     }
   else
     {
        if (sd->magnet_position & ELM_ACTIONSLIDER_RIGHT)
          sd->final_position = 1;
        else if (sd->magnet_position & ELM_ACTIONSLIDER_CENTER)
          sd->final_position = 0.5;
        else
          sd->final_position = 0;
     }
   ecore_animator_del(sd->button_animator);
   sd->button_animator = ecore_evas_animator_add(obj, _button_animator, obj);
}

static void
_track_move_cb(void *data,
       Evas_Object *o EINA_UNUSED,
       const char *emission,
       const char *source EINA_UNUSED)
{
   Evas_Object *obj = data;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (!strcmp(emission, "elm,action,down,right"))
     {
        if (EINA_DBL_EQ(sd->final_position, 0.0))
          {
             if (sd->enabled_position & ELM_ACTIONSLIDER_CENTER)
               {
                  efl_event_callback_legacy_call
                    (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, "center");
                  sd->final_position = 0.5;
               }
             else if (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT)
               {
                  efl_event_callback_legacy_call
                    (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, "right");
                  sd->final_position = 1.0;
               }
          }
        else if (EINA_DBL_EQ(sd->final_position, 0.5) &&
                 (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
          {
             efl_event_callback_legacy_call
               (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, "right");
             sd->final_position = 1.0;
          }
     }
   else if (!strcmp(emission, "elm,action,down,center"))
     {
        if (sd->enabled_position & ELM_ACTIONSLIDER_CENTER)
          {
             efl_event_callback_legacy_call
               (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, "center");
             sd->final_position = 0.5;
          }
     }
   else
     {
        if (EINA_DBL_EQ(sd->final_position, 1.0))
          {
             if (sd->enabled_position & ELM_ACTIONSLIDER_CENTER)
               {
                  efl_event_callback_legacy_call
                    (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, "center");
                  sd->final_position = 0.5;
               }
             else if (sd->enabled_position & ELM_ACTIONSLIDER_LEFT)
               {
                  efl_event_callback_legacy_call
                    (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, "left");
                  sd->final_position = 0.0;
               }
          }
        else if (EINA_DBL_EQ(sd->final_position, 0.5) &&
                (sd->enabled_position & ELM_ACTIONSLIDER_LEFT))
          {
             efl_event_callback_legacy_call
               (obj, ELM_ACTIONSLIDER_EVENT_POS_CHANGED, "left");
             sd->final_position = 0.0;
          }
     }
   ecore_animator_del(sd->button_animator);
   sd->button_animator = ecore_evas_animator_add(obj, _button_animator, obj);
}

static void
_mirrored_part_fix(const Evas_Object *obj,
                   const char **part)
{
   if (efl_ui_mirrored_get(obj))
     {
        /* exchange left and right */
        if (!strcmp(*part, "left")) *part = "right";
        else if (!strcmp(*part, "right"))
          *part = "left";
        else if (!strcmp(*part, "elm.text.right"))
          *part = "elm.text.left";
        else if (!strcmp(*part, "elm.text.left"))
          *part = "elm.text.right";
     }
}

static Eina_Bool
_elm_actionslider_text_set(Eo *obj, Elm_Actionslider_Data *_pd EINA_UNUSED, const char *part, const char *text)
{
   Eina_Bool int_ret = EINA_TRUE;

   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return EINA_FALSE;

   _mirrored_part_fix(obj, &part);

   efl_text_set(efl_part(efl_super(obj, MY_CLASS), part), text);

   return int_ret;
}

static const char *
_elm_actionslider_text_get(Eo *obj, Elm_Actionslider_Data *_pd EINA_UNUSED, const char *part)
{
   const char *text = NULL;

   if (!_elm_layout_part_aliasing_eval(obj, &part, EINA_TRUE))
     return NULL;

   _mirrored_part_fix(obj, &part);

   text = efl_text_get(efl_part(efl_super(obj, MY_CLASS), part));

   return text;
}

EOLIAN static void
_elm_actionslider_efl_canvas_group_group_add(Eo *obj, Elm_Actionslider_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   priv->enabled_position = ELM_ACTIONSLIDER_ALL;

   priv->drag_button_base =
     evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->drag_button_base, 0, 0, 0, 0);

   // dirty support for the backward compatibility
   edje_object_signal_callback_add
     (wd->resize_obj, "elm.drag_button,mouse,up", "*",
     _drag_button_up_cb, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm.drag_button,mouse,down", "*",
     _drag_button_down_cb, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm.drag_button,mouse,move", "*",
     _drag_button_move_cb, obj);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,up,drag_button", "elm",
     _drag_button_up_cb, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,down,drag_button", "elm",
     _drag_button_down_cb, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,move,drag_button", "elm",
     _drag_button_move_cb, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,down,right", "*",
     _track_move_cb, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,down,left", "*",
     _track_move_cb, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,down,center", "*",
     _track_move_cb, obj);

   if (!elm_layout_theme_set
       (obj, "actionslider", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_layout_content_set(obj, "elm.drag_button_base", priv->drag_button_base);

   elm_layout_sizing_eval(obj);
}

EAPI Evas_Object *
elm_actionslider_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_actionslider_efl_object_constructor(Eo *obj, Elm_Actionslider_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_SLIDER);

   return obj;
}

EOLIAN static void
_elm_actionslider_indicator_pos_set(Eo *obj, Elm_Actionslider_Data *_pd EINA_UNUSED, Elm_Actionslider_Pos pos)
{
   double position = 0.0;


   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   pos = _get_pos_by_orientation(obj, pos);
   if (pos == ELM_ACTIONSLIDER_CENTER) position = 0.5;
   else if (pos == ELM_ACTIONSLIDER_RIGHT)
     position = 1.0;

   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.drag_button_base", position, 0.5);
}

EOLIAN static Elm_Actionslider_Pos
_elm_actionslider_indicator_pos_get(const Eo *obj, Elm_Actionslider_Data *_pd EINA_UNUSED)
{
   double position;

   Elm_Actionslider_Pos ret = ELM_ACTIONSLIDER_NONE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, ELM_ACTIONSLIDER_NONE);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.drag_button_base", &position,
     NULL);
   if (position < 0.3)
     ret = _get_pos_by_orientation(obj, ELM_ACTIONSLIDER_LEFT);
   else if (position < 0.7)
     ret = ELM_ACTIONSLIDER_CENTER;
   else
     ret = _get_pos_by_orientation(obj, ELM_ACTIONSLIDER_RIGHT);

   return ret;
}

EOLIAN static void
_elm_actionslider_magnet_pos_set(Eo *obj EINA_UNUSED, Elm_Actionslider_Data *sd, Elm_Actionslider_Pos pos)
{
   sd->magnet_position = pos;
}

EOLIAN static Elm_Actionslider_Pos
_elm_actionslider_magnet_pos_get(const Eo *obj EINA_UNUSED, Elm_Actionslider_Data *sd)
{
   return sd->magnet_position;
}

EOLIAN static void
_elm_actionslider_enabled_pos_set(Eo *obj EINA_UNUSED, Elm_Actionslider_Data *sd, Elm_Actionslider_Pos pos)
{
   sd->enabled_position = pos;
}

EOLIAN static Elm_Actionslider_Pos
_elm_actionslider_enabled_pos_get(const Eo *obj EINA_UNUSED, Elm_Actionslider_Data *sd)
{
   return sd->enabled_position;
}

EOLIAN static const char*
_elm_actionslider_selected_label_get(const Eo *obj, Elm_Actionslider_Data *sd)
{
   const char *ret;
   ret = NULL;
   const char *left, *right, *center;

   _text_get(obj, &left, &right, &center);

   if ((EINA_DBL_EQ(sd->final_position, 0.0)) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_LEFT))
     ret = left;

   if ((EINA_DBL_EQ(sd->final_position, 0.5)) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_CENTER))
     ret = center;

   if ((EINA_DBL_EQ(sd->final_position, 1.0)) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
     ret = right;

   return ret;
}

static void
_elm_actionslider_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Efl.Part begin */
ELM_PART_OVERRIDE(elm_actionslider, ELM_ACTIONSLIDER, Elm_Actionslider_Data)
ELM_PART_OVERRIDE_TEXT_SET(elm_actionslider, ELM_ACTIONSLIDER, Elm_Actionslider_Data)
ELM_PART_OVERRIDE_TEXT_GET(elm_actionslider, ELM_ACTIONSLIDER, Elm_Actionslider_Data)

#include "elm_actionslider_part.eo.c"
/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

ELM_LAYOUT_TEXT_ALIASES_IMPLEMENT(MY_CLASS_PFX)

#define ELM_ACTIONSLIDER_EXTRA_OPS \
   ELM_LAYOUT_TEXT_ALIASES_OPS(MY_CLASS_PFX), \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_actionslider), \
   EFL_CANVAS_GROUP_ADD_OPS(elm_actionslider)

#include "elm_actionslider_eo.c"
