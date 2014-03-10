#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include <math.h>
#include "elm_priv.h"
#include "elm_widget_actionslider.h"
#include "elm_widget_layout.h"

EAPI Eo_Op ELM_OBJ_ACTIONSLIDER_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_ACTIONSLIDER_CLASS

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
   if (elm_widget_mirrored_get(obj))
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

static void
_elm_actionslider_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;

   Elm_Actionslider_Smart_Data *sd = _pd;
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

static void
_elm_actionslider_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool mirrored;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   mirrored = elm_object_mirrored_get(obj);

   eo_do_super(obj, MY_CLASS, elm_wdg_theme_apply(&int_ret));
   if (!int_ret) return;

   if (elm_object_mirrored_get(obj) != mirrored)
     _mirroredness_change_eval(obj);

   edje_object_message_signal_process(wd->resize_obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_drag_button_down_cb(void *data,
                     Evas_Object *o EINA_UNUSED,
                     const char *emission EINA_UNUSED,
                     const char *source EINA_UNUSED)
{
   Elm_Actionslider_Smart_Data *sd = eo_data_scope_get(data, MY_CLASS);

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
   if (pos == 0.0)
     evas_object_smart_callback_call
       (obj, SIG_CHANGED, !elm_widget_mirrored_get(obj) ? "left" : "right");
   else if (pos == 1.0)
     evas_object_smart_callback_call
       (obj, SIG_CHANGED, !elm_widget_mirrored_get(obj) ? "right" : "left");
   else if (pos >= 0.45 && pos <= 0.55)
     evas_object_smart_callback_call(obj, SIG_CHANGED, "center");
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

      adjusted_final = (!elm_widget_mirrored_get(obj)) ?
        sd->final_position : 1.0 - sd->final_position;

      if ((adjusted_final == 0.0) ||
          (adjusted_final == 0.5 && cur_position >= adjusted_final))
        {
           new_position = cur_position - move_amount;

           if (new_position <= adjusted_final)
             {
                new_position = adjusted_final;
                flag_finish_animation = EINA_TRUE;
             }
        }
      else if ((adjusted_final == 1.0) ||
               (adjusted_final == 0.5 && cur_position < adjusted_final))
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

        if ((!sd->final_position) &&
            (sd->enabled_position & ELM_ACTIONSLIDER_LEFT))
          evas_object_smart_callback_call(obj, SIG_SELECTED, (char *)left);
        else if ((sd->final_position == 0.5) &&
                 (sd->enabled_position & ELM_ACTIONSLIDER_CENTER))
          evas_object_smart_callback_call(obj, SIG_SELECTED, (char *)center);
        else if ((sd->final_position == 1) &&
                 (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
          evas_object_smart_callback_call(obj, SIG_SELECTED, (char *)right);

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
       ((!elm_widget_mirrored_get(obj) && position == 0.0) ||
        (elm_widget_mirrored_get(obj) && position == 1.0)))
     {
        sd->final_position = 0;
        evas_object_smart_callback_call(obj, SIG_SELECTED, (char *)left);

        return;
     }

   if (position >= 0.45 && position <= 0.55 &&
       (sd->enabled_position & ELM_ACTIONSLIDER_CENTER))
     {
        sd->final_position = 0.5;
        evas_object_smart_callback_call(obj, SIG_SELECTED, (char *)center);

        ecore_animator_del(sd->button_animator);
        sd->button_animator = ecore_animator_add(_button_animator, obj);

        return;
     }

   if ((sd->enabled_position & ELM_ACTIONSLIDER_RIGHT) &&
       ((!elm_widget_mirrored_get(obj) && position == 1.0) ||
        (elm_widget_mirrored_get(obj) && position == 0.0)))
     {
        sd->final_position = 1;
        evas_object_smart_callback_call(obj, SIG_SELECTED, (char *)right);
        return;
     }

   if (sd->magnet_position == ELM_ACTIONSLIDER_NONE) return;

#define _FIX_POS_ON_MIRROREDNESS(x) \
  ((!elm_widget_mirrored_get(obj)) ? x : 1.0 - x)

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
   sd->button_animator = ecore_animator_add(_button_animator, obj);
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
        if (sd->final_position == 0.0)
          {
             if (sd->enabled_position & ELM_ACTIONSLIDER_CENTER)
               {
                  evas_object_smart_callback_call(obj, SIG_CHANGED, "center");
                  sd->final_position = 0.5;
               }
             else if (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT)
               {
                  evas_object_smart_callback_call(obj, SIG_CHANGED, "right");
                  sd->final_position = 1.0;
               }
          }
        else if ((sd->final_position == 0.5) &&
                 (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
          {
             evas_object_smart_callback_call(obj, SIG_CHANGED, "right");
             sd->final_position = 1.0;
          }
     }
   else if (!strcmp(emission, "elm,action,down,center"))
     {
        if (sd->enabled_position & ELM_ACTIONSLIDER_CENTER)
          {
             evas_object_smart_callback_call(obj, SIG_CHANGED, "center");
             sd->final_position = 0.5;
          }
     }
   else
     {
        if (sd->final_position == 1.0)
          {
             if (sd->enabled_position & ELM_ACTIONSLIDER_CENTER)
               {
                  evas_object_smart_callback_call(obj, SIG_CHANGED, "center");
                  sd->final_position = 0.5;
               }
             else if (sd->enabled_position & ELM_ACTIONSLIDER_LEFT)
               {
                  evas_object_smart_callback_call(obj, SIG_CHANGED, "left");
                  sd->final_position = 0.0;
               }
          }
        else if (sd->final_position == 0.5 &&
                (sd->enabled_position & ELM_ACTIONSLIDER_LEFT))
          {
             evas_object_smart_callback_call(obj, SIG_CHANGED, "left");
             sd->final_position = 0.0;
          }
     }
   ecore_animator_del(sd->button_animator);
   sd->button_animator = ecore_animator_add(_button_animator, obj);
}

static void
_mirrored_part_fix(const Evas_Object *obj,
                   const char **part)
{
   if (elm_widget_mirrored_get(obj))
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

static void
_elm_actionslider_smart_text_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *text = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_FALSE;

   _mirrored_part_fix(obj, &part);
   eo_do_super(obj, MY_CLASS, elm_obj_layout_text_set(part, text, &int_ret));

   if (ret) *ret = int_ret;
}

static void
_elm_actionslider_smart_text_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)

{
   const char *part = va_arg(*list, const char *);
   const char **text = va_arg(*list, const char **);

   _mirrored_part_fix(obj, &part);

   eo_do_super(obj, MY_CLASS, elm_obj_layout_text_get(part, text));
}

static void
_elm_actionslider_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Actionslider_Smart_Data *priv = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   priv->enabled_position = ELM_ACTIONSLIDER_ALL;

   priv->drag_button_base =
     evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->drag_button_base, 0, 0, 0, 0);

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

static void
_elm_actionslider_smart_text_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _text_aliases;
}

EAPI Evas_Object *
elm_actionslider_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_actionslider_indicator_pos_set(Evas_Object *obj,
                                   Elm_Actionslider_Pos pos)
{
   ELM_ACTIONSLIDER_CHECK(obj);
   eo_do(obj, elm_obj_actionslider_indicator_pos_set(pos));
}

static void
_indicator_pos_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   double position = 0.0;

   Elm_Actionslider_Pos pos = va_arg(*list, Elm_Actionslider_Pos);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   pos = _get_pos_by_orientation(obj, pos);
   if (pos == ELM_ACTIONSLIDER_CENTER) position = 0.5;
   else if (pos == ELM_ACTIONSLIDER_RIGHT)
     position = 1.0;

   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.drag_button_base", position, 0.5);
}

EAPI Elm_Actionslider_Pos
elm_actionslider_indicator_pos_get(const Evas_Object *obj)
{
   ELM_ACTIONSLIDER_CHECK(obj) ELM_ACTIONSLIDER_NONE;
   Elm_Actionslider_Pos ret = ELM_ACTIONSLIDER_NONE;
   eo_do((Eo *) obj, elm_obj_actionslider_indicator_pos_get(&ret));
   return ret;
}

static void
_indicator_pos_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   double position;

   Elm_Actionslider_Pos *ret = va_arg(*list, Elm_Actionslider_Pos *);

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.drag_button_base", &position,
     NULL);
   if (position < 0.3)
     *ret = _get_pos_by_orientation(obj, ELM_ACTIONSLIDER_LEFT);
   else if (position < 0.7)
     *ret = ELM_ACTIONSLIDER_CENTER;
   else
     *ret = _get_pos_by_orientation(obj, ELM_ACTIONSLIDER_RIGHT);
}

EAPI void
elm_actionslider_magnet_pos_set(Evas_Object *obj,
                                Elm_Actionslider_Pos pos)
{
   ELM_ACTIONSLIDER_CHECK(obj);
   eo_do(obj, elm_obj_actionslider_magnet_pos_set(pos));
}

static void
_magnet_pos_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Actionslider_Pos pos = va_arg(*list, Elm_Actionslider_Pos);
   Elm_Actionslider_Smart_Data *sd = _pd;
   sd->magnet_position = pos;
}

EAPI Elm_Actionslider_Pos
elm_actionslider_magnet_pos_get(const Evas_Object *obj)
{
   ELM_ACTIONSLIDER_CHECK(obj) ELM_ACTIONSLIDER_NONE;
   Elm_Actionslider_Pos ret = ELM_ACTIONSLIDER_NONE;
   eo_do((Eo *) obj, elm_obj_actionslider_magnet_pos_get(&ret));
   return ret;
}

static void
_magnet_pos_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Actionslider_Pos *ret = va_arg(*list, Elm_Actionslider_Pos *);
   Elm_Actionslider_Smart_Data *sd = _pd;
   *ret =  sd->magnet_position;
}

EAPI void
elm_actionslider_enabled_pos_set(Evas_Object *obj,
                                 Elm_Actionslider_Pos pos)
{
   ELM_ACTIONSLIDER_CHECK(obj);
   eo_do(obj, elm_obj_actionslider_enabled_pos_set(pos));
}

static void
_enabled_pos_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Actionslider_Pos pos = va_arg(*list, Elm_Actionslider_Pos);
   Elm_Actionslider_Smart_Data *sd = _pd;
   sd->enabled_position = pos;
}

EAPI Elm_Actionslider_Pos
elm_actionslider_enabled_pos_get(const Evas_Object *obj)
{
   ELM_ACTIONSLIDER_CHECK(obj) ELM_ACTIONSLIDER_NONE;
   Elm_Actionslider_Pos ret = ELM_ACTIONSLIDER_NONE;
   eo_do((Eo *) obj, elm_obj_actionslider_enabled_pos_get(&ret));
   return ret;
}

static void
_enabled_pos_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Actionslider_Pos *ret = va_arg(*list, Elm_Actionslider_Pos *);
   Elm_Actionslider_Smart_Data *sd = _pd;
   *ret = sd->enabled_position;
}

EAPI const char *
elm_actionslider_selected_label_get(const Evas_Object *obj)
{
   ELM_ACTIONSLIDER_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_actionslider_selected_label_get(&ret));
   return ret;
}

static void
_selected_label_get(Eo *obj, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   *ret = NULL;
   const char *left, *right, *center;

   Elm_Actionslider_Smart_Data *sd = _pd;

   _text_get(obj, &left, &right, &center);

   if ((sd->final_position == 0.0) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_LEFT))
     *ret = left;

   if ((sd->final_position == 0.5) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_CENTER))
     *ret = center;

   if ((sd->final_position == 1.0) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
     *ret = right;
}

static void
_elm_actionslider_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_elm_actionslider_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_actionslider_smart_add),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME_APPLY), _elm_actionslider_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_actionslider_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_actionslider_smart_focus_direction_manager_is),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_actionslider_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET), _elm_actionslider_smart_text_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET), _elm_actionslider_smart_text_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_ALIASES_GET), _elm_actionslider_smart_text_aliases_get),

        EO_OP_FUNC(ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_SET), _indicator_pos_set),
        EO_OP_FUNC(ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_GET), _indicator_pos_get),
        EO_OP_FUNC(ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_SET), _magnet_pos_set),
        EO_OP_FUNC(ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_GET), _magnet_pos_get),
        EO_OP_FUNC(ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_SET), _enabled_pos_set),
        EO_OP_FUNC(ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_GET), _enabled_pos_get),
        EO_OP_FUNC(ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_SELECTED_LABEL_GET), _selected_label_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_SET, "Set actionslider indicator position."),
     EO_OP_DESCRIPTION(ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_GET, "Get actionslider indicator position."),
     EO_OP_DESCRIPTION(ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_SET, "Set actionslider magnet position."),
     EO_OP_DESCRIPTION(ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_GET, "Get actionslider magnet position."),
     EO_OP_DESCRIPTION(ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_SET, "Set actionslider enabled position."),
     EO_OP_DESCRIPTION(ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_GET, "Get actionslider enabled position."),
     EO_OP_DESCRIPTION(ELM_OBJ_ACTIONSLIDER_SUB_ID_SELECTED_LABEL_GET, "Get actionslider selected label."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_ACTIONSLIDER_BASE_ID, op_desc, ELM_OBJ_ACTIONSLIDER_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Actionslider_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_actionslider_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
