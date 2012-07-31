#include <Elementary.h>
#include <math.h>
#include "elm_priv.h"
#include "elm_widget_actionslider.h"

EAPI const char ELM_ACTIONSLIDER_SMART_NAME[] = "elm_actionslider";

static const Elm_Layout_Part_Alias_Description _text_aliases[] =
{
   {"indicator", "elm.text.indicator"},
   {"left", "elm.text.left"},
   {"right", "elm.text.right"},
   {"center", "elm.text.center"},
   {NULL, NULL}
};

static const char SIG_CHANGED[] = "pos_changed";
static const char SIG_SELECTED[] = "selected";
static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_CHANGED, ""},
   {SIG_SELECTED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_ACTIONSLIDER_SMART_NAME, _elm_actionslider,
  Elm_Actionslider_Smart_Class, Elm_Layout_Smart_Class,
  elm_layout_smart_class_get, _smart_callbacks);

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
_elm_actionslider_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->drag_button_base, minw, minh);
   evas_object_size_hint_max_set(sd->drag_button_base, -1, -1);

   minw = -1;
   minh = -1;
   elm_coords_finger_size_adjust(3, &minw, 1, &minh);
   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_mirroredness_change_eval(Evas_Object *obj)
{
   double pos;
   char *left;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   left = (char *)elm_layout_text_get(obj, "elm.text.left");
   if (left) left = strdup(left);

   elm_layout_text_set
     (obj, "elm.text.left", elm_layout_text_get(obj, "elm.text.right"));
   elm_layout_text_set(obj, "elm.text.right", left);

   free(left);

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.drag_button_base", &pos, NULL);
   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.drag_button_base", 1.0 - pos, 0.5);
}

static Eina_Bool
_elm_actionslider_smart_theme(Evas_Object *obj)
{
   Eina_Bool mirrored;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   mirrored = elm_object_mirrored_get(obj);

   if (!ELM_WIDGET_CLASS(_elm_actionslider_parent_sc)->theme(obj))
     return EINA_FALSE;

   if (elm_object_mirrored_get(obj) != mirrored)
     _mirroredness_change_eval(obj);

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);

   return EINA_TRUE;
}

static void
_drag_button_down_cb(void *data,
                     Evas_Object *o __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
{
   Elm_Actionslider_Smart_Data *sd = data;

   sd->mouse_down = EINA_TRUE;
}

static void
_drag_button_move_cb(void *data,
                     Evas_Object *o __UNUSED__,
                     const char *emission __UNUSED__,
                     const char *source __UNUSED__)
{
   Evas_Object *obj = data;
   double pos = 0.0;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   if (!sd->mouse_down) return;

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.drag_button_base", &pos, NULL);
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

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.drag_button_base",
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
        (ELM_WIDGET_DATA(sd)->resize_obj, "elm.drag_button_base",
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
                   Evas_Object *o __UNUSED__,
                   const char *emission __UNUSED__,
                   const char *source __UNUSED__)
{
   Evas_Object *obj = data;
   double position = 0.0;

   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   sd->mouse_down = EINA_FALSE;

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.drag_button_base", &position,
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

        if (sd->button_animator) ecore_animator_del(sd->button_animator);
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
   if (sd->button_animator) ecore_animator_del(sd->button_animator);
   sd->button_animator = ecore_animator_add(_button_animator, obj);

#undef _FINAL_FIX_POS_ON_MIRROREDNESS
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

static Eina_Bool
_elm_actionslider_smart_text_set(Evas_Object *obj,
                                 const char *item,
                                 const char *label)
{
   _mirrored_part_fix(obj, &item);

   return _elm_actionslider_parent_sc->text_set(obj, item, label);
}

static const char *
_elm_actionslider_smart_text_get(const Evas_Object *obj,
                                 const char *item)
{
   _mirrored_part_fix(obj, &item);

   return _elm_actionslider_parent_sc->text_get(obj, item);
}

static void
_elm_actionslider_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Actionslider_Smart_Data);

   ELM_WIDGET_CLASS(_elm_actionslider_parent_sc)->base.add(obj);

   priv->mouse_down = EINA_FALSE;
   priv->enabled_position = ELM_ACTIONSLIDER_ALL;

   priv->drag_button_base =
     evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(priv->drag_button_base, 0, 0, 0, 0);

   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm.drag_button,mouse,up", "",
     _drag_button_up_cb, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm.drag_button,mouse,down", "",
     _drag_button_down_cb, priv);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm.drag_button,mouse,move", "",
     _drag_button_move_cb, obj);

   elm_layout_theme_set
     (obj, "actionslider", "base", elm_widget_style_get(obj));

   elm_layout_content_set(obj, "elm.drag_button_base", priv->drag_button_base);

   elm_layout_sizing_eval(obj);
}

static void
_elm_actionslider_smart_set_user(Elm_Actionslider_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_actionslider_smart_add;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_WIDGET_CLASS(sc)->theme = _elm_actionslider_smart_theme;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_actionslider_smart_sizing_eval;
   ELM_LAYOUT_CLASS(sc)->text_set = _elm_actionslider_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_actionslider_smart_text_get;

   ELM_LAYOUT_CLASS(sc)->text_aliases = _text_aliases;
}

EAPI const Elm_Actionslider_Smart_Class *
elm_actionslider_smart_class_get(void)
{
   static Elm_Actionslider_Smart_Class _sc =
     ELM_ACTIONSLIDER_SMART_CLASS_INIT_NAME_VERSION
       (ELM_ACTIONSLIDER_SMART_NAME);
   static const Elm_Actionslider_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_actionslider_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_actionslider_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_actionslider_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_actionslider_indicator_pos_set(Evas_Object *obj,
                                   Elm_Actionslider_Pos pos)
{
   double position = 0.0;

   ELM_ACTIONSLIDER_CHECK(obj);
   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   pos = _get_pos_by_orientation(obj, pos);
   if (pos == ELM_ACTIONSLIDER_CENTER) position = 0.5;
   else if (pos == ELM_ACTIONSLIDER_RIGHT)
     position = 1.0;

   edje_object_part_drag_value_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.drag_button_base", position, 0.5);
}

EAPI Elm_Actionslider_Pos
elm_actionslider_indicator_pos_get(const Evas_Object *obj)
{
   double position;

   ELM_ACTIONSLIDER_CHECK(obj) ELM_ACTIONSLIDER_NONE;
   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.drag_button_base", &position,
     NULL);
   if (position < 0.3)
     return _get_pos_by_orientation(obj, ELM_ACTIONSLIDER_LEFT);
   else if (position < 0.7)
     return ELM_ACTIONSLIDER_CENTER;
   else
     return _get_pos_by_orientation(obj, ELM_ACTIONSLIDER_RIGHT);
}

EAPI void
elm_actionslider_magnet_pos_set(Evas_Object *obj,
                                Elm_Actionslider_Pos pos)
{
   ELM_ACTIONSLIDER_CHECK(obj);
   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   sd->magnet_position = pos;
}

EAPI Elm_Actionslider_Pos
elm_actionslider_magnet_pos_get(const Evas_Object *obj)
{
   ELM_ACTIONSLIDER_CHECK(obj) ELM_ACTIONSLIDER_NONE;
   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   return sd->magnet_position;
}

EAPI void
elm_actionslider_enabled_pos_set(Evas_Object *obj,
                                 Elm_Actionslider_Pos pos)
{
   ELM_ACTIONSLIDER_CHECK(obj);
   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   sd->enabled_position = pos;
}

EAPI Elm_Actionslider_Pos
elm_actionslider_enabled_pos_get(const Evas_Object *obj)
{
   ELM_ACTIONSLIDER_CHECK(obj) ELM_ACTIONSLIDER_NONE;
   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   return sd->enabled_position;
}

EAPI const char *
elm_actionslider_selected_label_get(const Evas_Object *obj)
{
   const char *left, *right, *center;

   ELM_ACTIONSLIDER_CHECK(obj) NULL;
   ELM_ACTIONSLIDER_DATA_GET(obj, sd);

   _text_get(obj, &left, &right, &center);

   if ((sd->final_position == 0.0) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_LEFT))
     return left;

   if ((sd->final_position == 0.5) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_CENTER))
     return center;

   if ((sd->final_position == 1.0) &&
       (sd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
     return right;

   return NULL;
}
