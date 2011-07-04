#include <Elementary.h>
#include <math.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *as;     // actionslider
   Evas_Object *drag_button_base;
   Elm_Actionslider_Pos magnet_position, enabled_position;
   const char *text_left, *text_right, *text_center;
   const char *indicator_label;
   Ecore_Animator *button_animator;
   double final_position;
   Eina_Bool mouse_down : 1;
};

static const char *widtype = NULL;

static const char SIG_CHANGED[] = "pos_changed";
static const char SIG_SELECTED[] = "selected";

static const Evas_Smart_Cb_Description _signals[] =
{
     {SIG_CHANGED, ""},
     {SIG_SELECTED, ""},
     {NULL, NULL}
};


static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->drag_button_base)
     {
        evas_object_del(wd->drag_button_base);
        wd->drag_button_base = NULL;
     }
   if (wd->text_left) eina_stringshare_del(wd->text_left);
   if (wd->text_right) eina_stringshare_del(wd->text_right);
   if (wd->text_center) eina_stringshare_del(wd->text_center);
   if (wd->indicator_label) eina_stringshare_del(wd->indicator_label);
   free(wd);
}

static Elm_Actionslider_Pos
_get_pos_by_orientation(const Evas_Object *obj, Elm_Actionslider_Pos pos)
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
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   double pos;

   if (!wd) return;
   if (edje_object_mirrored_get(wd->as) == rtl)
     return;

   edje_object_mirrored_set(wd->as, rtl);
   if (!elm_widget_mirrored_get(obj))
     {
        edje_object_part_text_set(wd->as, "elm.text.left", wd->text_left);
        edje_object_part_text_set(wd->as, "elm.text.right", wd->text_right);
     }
   else
     {
        edje_object_part_text_set(wd->as, "elm.text.left", wd->text_right);
        edje_object_part_text_set(wd->as, "elm.text.right", wd->text_left);
     }
   edje_object_part_drag_value_get(wd->as, "elm.drag_button_base", &pos, NULL);
   edje_object_part_drag_value_set(wd->as, "elm.drag_button_base", 1.0 - pos, 0.5);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1;

   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(wd->drag_button_base, minw, minh);
   evas_object_size_hint_max_set(wd->drag_button_base, -1, -1);

   minw = -1;
   minh = -1;
   elm_coords_finger_size_adjust(3, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->as, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   if (!edje_object_part_swallow_get(wd->as, "elm.drag_button_base"))
     edje_object_part_unswallow(wd->as, wd->drag_button_base);

   _elm_theme_object_set(obj, wd->as, "actionslider",
                         "base", elm_widget_style_get(obj));
   _elm_theme_object_set(obj, wd->drag_button_base, "actionslider",
                         "drag_button", elm_widget_style_get(obj));
   edje_object_part_swallow(wd->as, "elm.drag_button_base", wd->drag_button_base);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   edje_object_part_text_set(wd->as, "elm.text.center", wd->text_center);
   edje_object_part_text_set(wd->as, "elm.text.indicator", wd->indicator_label);
   edje_object_message_signal_process(wd->as);
   _sizing_eval(obj);
}

static void
_drag_button_down_cb(void *data, Evas_Object *o __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get((Evas_Object *) data);
   if (!wd) return;
   wd->mouse_down = EINA_TRUE;
}

static void
_drag_button_move_cb(void *data, Evas_Object *o __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *obj = (Evas_Object *) data;
   Widget_Data *wd = elm_widget_data_get(obj);
   double pos = 0.0;
   if (!wd) return;

   if (!wd->mouse_down) return;
   edje_object_part_drag_value_get(wd->as, "elm.drag_button_base", &pos, NULL);
   if (pos == 0.0)
     evas_object_smart_callback_call(obj, SIG_CHANGED,
                                     (void *) ((!elm_widget_mirrored_get(obj)) ?
                                               "left" : "right"));
   else if (pos == 1.0)
     evas_object_smart_callback_call(obj, SIG_CHANGED,
                                     (void *) ((!elm_widget_mirrored_get(obj)) ?
                                               "right" : "left"));
   else if (pos >= 0.45 && pos <= 0.55)
     evas_object_smart_callback_call(obj, SIG_CHANGED, (void *)"center");
}

static Eina_Bool
_button_animation(void *data)
{
   Evas_Object *obj = data;
   Widget_Data *wd = elm_widget_data_get(obj);
   double cur_position = 0.0, new_position = 0.0;
   double move_amount = 0.05;
   Eina_Bool flag_finish_animation = EINA_FALSE;
   if (!wd)
     {
        wd->button_animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   edje_object_part_drag_value_get(wd->as,
                                   "elm.drag_button_base", &cur_position, NULL);
     {
        double adjusted_final;
        adjusted_final = (!elm_widget_mirrored_get(obj)) ?
           wd->final_position : 1.0 - wd->final_position;
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
        edje_object_part_drag_value_set(wd->as,
                                        "elm.drag_button_base", new_position, 0.5);
     }

   if (flag_finish_animation)
     {
        if ((!wd->final_position) &&
            (wd->enabled_position & ELM_ACTIONSLIDER_LEFT))
          evas_object_smart_callback_call(data, SIG_SELECTED,
                                          (void *)wd->text_left);
        else if ((wd->final_position == 0.5) &&
                 (wd->enabled_position & ELM_ACTIONSLIDER_CENTER))
          evas_object_smart_callback_call(data, SIG_SELECTED,
                                          (void *)wd->text_center);
        else if ((wd->final_position == 1) &&
                 (wd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
          evas_object_smart_callback_call(data, SIG_SELECTED,
                                          (void *)wd->text_right);
        wd->button_animator = NULL;
        return ECORE_CALLBACK_CANCEL;
     }
   return ECORE_CALLBACK_RENEW;
}

static void
_drag_button_up_cb(void *data, Evas_Object *o __UNUSED__, const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Evas_Object *obj = (Evas_Object *) data;
   Widget_Data *wd = elm_widget_data_get(obj);
   double position = 0.0;
   if (!wd) return;

   wd->mouse_down = EINA_FALSE;

   edje_object_part_drag_value_get(wd->as, "elm.drag_button_base",
                                   &position, NULL);

   if ((wd->enabled_position & ELM_ACTIONSLIDER_LEFT) &&
       ((!elm_widget_mirrored_get(obj) && position == 0.0) ||
        (elm_widget_mirrored_get(obj) && position == 1.0)))
     {
        wd->final_position = 0;
        evas_object_smart_callback_call(data, SIG_SELECTED,
                                        (void *) wd->text_left);
        return;
     }
   if (position >= 0.45 && position <= 0.55 &&
       (wd->enabled_position & ELM_ACTIONSLIDER_CENTER))
     {
        wd->final_position = 0.5;
        evas_object_smart_callback_call(data, SIG_SELECTED,
                                        (void *)wd->text_center);
        if (wd->button_animator) ecore_animator_del(wd->button_animator);
        wd->button_animator = ecore_animator_add(_button_animation, data);
        return;
     }
   if ((wd->enabled_position & ELM_ACTIONSLIDER_RIGHT) &&
       ((!elm_widget_mirrored_get(obj) && position == 1.0) ||
        (elm_widget_mirrored_get(obj) && position == 0.0)))
     {
        wd->final_position = 1;
        evas_object_smart_callback_call(data, SIG_SELECTED,
                                        (void *) wd->text_right);
        return;
     }

   if (wd->magnet_position == ELM_ACTIONSLIDER_NONE) return;

#define _FINAL_POS_BY_ORIENTATION(x) (x)
#define _POS_BY_ORIENTATION(x) \
   ((!elm_widget_mirrored_get(obj)) ? \
    x : 1.0 - x)

   position = _POS_BY_ORIENTATION(position);

   if (position < 0.3)
     {
        if (wd->magnet_position & ELM_ACTIONSLIDER_LEFT)
          wd->final_position = _FINAL_POS_BY_ORIENTATION(0);
        else if (wd->magnet_position & ELM_ACTIONSLIDER_CENTER)
          wd->final_position = 0.5;
        else if (wd->magnet_position & ELM_ACTIONSLIDER_RIGHT)
          wd->final_position = _FINAL_POS_BY_ORIENTATION(1);
     }
   else if ((position >= 0.3) && (position <= 0.7))
     {
        if (wd->magnet_position & ELM_ACTIONSLIDER_CENTER)
          wd->final_position = 0.5;
        else if (position < 0.5)
          {
             if (wd->magnet_position & ELM_ACTIONSLIDER_LEFT)
               wd->final_position = _FINAL_POS_BY_ORIENTATION(0);
             else
               wd->final_position = _FINAL_POS_BY_ORIENTATION(1);
          }
        else
          {
             if (wd->magnet_position & ELM_ACTIONSLIDER_RIGHT)
               wd->final_position = _FINAL_POS_BY_ORIENTATION(1);
             else
               wd->final_position = _FINAL_POS_BY_ORIENTATION(0);
          }
     }
   else
     {
        if (wd->magnet_position & ELM_ACTIONSLIDER_RIGHT)
          wd->final_position = _FINAL_POS_BY_ORIENTATION(1);
        else if (wd->magnet_position & ELM_ACTIONSLIDER_CENTER)
          wd->final_position = 0.5;
        else
          wd->final_position = _FINAL_POS_BY_ORIENTATION(0);
     }
   if (wd->button_animator) ecore_animator_del(wd->button_animator);
   wd->button_animator = ecore_animator_add(_button_animation, data);

#undef _FINAL_POS_BY_ORIENTATION
}

static void
_elm_actionslider_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (!item)
     {
        eina_stringshare_replace(&wd->indicator_label, label);
        edje_object_part_text_set(wd->as, "elm.text.indicator",
              wd->indicator_label);
     }
   else if (!strcmp(item, "left"))
     {
        eina_stringshare_replace(&wd->text_left, label);
        if (!elm_widget_mirrored_get(obj))
          {
             edje_object_part_text_set(wd->as, "elm.text.left", wd->text_left);
          }
        else
          {
             edje_object_part_text_set(wd->as, "elm.text.right", wd->text_left);
          }
     }
   else if (!strcmp(item, "center"))
     {
        eina_stringshare_replace(&wd->text_center, label);
        edje_object_part_text_set(wd->as, "elm.text.center", wd->text_center);
     }
   else if (!strcmp(item, "right"))
     {
        eina_stringshare_replace(&wd->text_right, label);
        if (!elm_widget_mirrored_get(obj))
          {
             edje_object_part_text_set(wd->as, "elm.text.right", wd->text_right);
          }
        else
          {
             edje_object_part_text_set(wd->as, "elm.text.left", wd->text_right);
          }
     }
}

static const char *
_elm_actionslider_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   if (!item)
     {
        return wd->indicator_label;
     }
   else if (!strcmp(item, "left"))
     {
        return wd->text_left;
     }
   else if (!strcmp(item, "center"))
     {
        return wd->text_center;
     }
   else if (!strcmp(item, "right"))
     {
        return wd->text_right;
     }

   return NULL;
}

EAPI Evas_Object *
elm_actionslider_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Widget_Data *wd;
   Evas *e;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "actionslider");
   elm_widget_type_set(obj, "actionslider");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_label_set_hook_set(obj, _elm_actionslider_label_set);
   elm_widget_label_get_hook_set(obj, _elm_actionslider_label_get);

   wd->mouse_down = EINA_FALSE;
   wd->enabled_position = ELM_ACTIONSLIDER_ALL;

   wd->as = edje_object_add(e);
   _elm_theme_object_set(obj, wd->as, "actionslider", "base", "default");
   elm_widget_resize_object_set(obj, wd->as);

   wd->drag_button_base = evas_object_rectangle_add(e);
   evas_object_color_set(wd->drag_button_base, 0, 0, 0, 0);
   edje_object_part_swallow(wd->as, "elm.drag_button_base", wd->drag_button_base);

   edje_object_signal_callback_add(wd->as,
                                   "elm.drag_button,mouse,up", "",
                                   _drag_button_up_cb, obj);
   edje_object_signal_callback_add(wd->as,
                                   "elm.drag_button,mouse,down", "",
                                   _drag_button_down_cb, obj);
   edje_object_signal_callback_add(wd->as,
                                   "elm.drag_button,mouse,move", "",
                                   _drag_button_move_cb, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_actionslider_indicator_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   double position = 0.0;
   if (!wd) return;
   pos = _get_pos_by_orientation(obj, pos);
   if (pos == ELM_ACTIONSLIDER_CENTER) position = 0.5;
   else if (pos == ELM_ACTIONSLIDER_RIGHT) position = 1.0;
   edje_object_part_drag_value_set(wd->as, "elm.drag_button_base", position, 0.5);
}

EAPI Elm_Actionslider_Pos
elm_actionslider_indicator_pos_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_ACTIONSLIDER_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   double position;
   if (!wd) return ELM_ACTIONSLIDER_NONE;

   edje_object_part_drag_value_get(wd->as, "elm.drag_button_base", &position, NULL);
   if (position < 0.3)
     return _get_pos_by_orientation(obj, ELM_ACTIONSLIDER_LEFT);
   else if (position < 0.7)
     return ELM_ACTIONSLIDER_CENTER;
   else
     return _get_pos_by_orientation(obj, ELM_ACTIONSLIDER_RIGHT);
}

EAPI void
elm_actionslider_magnet_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->magnet_position = pos;
}

EAPI Elm_Actionslider_Pos
elm_actionslider_magnet_pos_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_ACTIONSLIDER_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_ACTIONSLIDER_NONE;
   return wd->magnet_position;
}

EAPI void
elm_actionslider_enabled_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->enabled_position = pos;
}

EAPI Elm_Actionslider_Pos
elm_actionslider_enabled_pos_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_ACTIONSLIDER_NONE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return ELM_ACTIONSLIDER_NONE;
   return wd->enabled_position;
}

EAPI void
elm_actionslider_labels_set(Evas_Object *obj, const char *left_label, const char *center_label, const char *right_label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   _elm_actionslider_label_set(obj, "left", left_label);
   _elm_actionslider_label_set(obj, "center", center_label);
   _elm_actionslider_label_set(obj, "right", right_label);
}

EAPI void
elm_actionslider_labels_get(const Evas_Object *obj, const char **left_label, const char **center_label, const char **right_label)
{
   if (left_label) *left_label= NULL;
   if (center_label) *center_label= NULL;
   if (right_label) *right_label= NULL;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (left_label) *left_label = _elm_actionslider_label_get(obj, "left");
   if (center_label) *center_label = _elm_actionslider_label_get(obj, "center");

   if (right_label) *right_label = _elm_actionslider_label_get(obj, "right");

}

EAPI const char *
elm_actionslider_selected_label_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   if ((wd->final_position == 0.0) &&
       (wd->enabled_position & ELM_ACTIONSLIDER_LEFT))
     return wd->text_left;

   if ((wd->final_position == 0.5) &&
       (wd->enabled_position & ELM_ACTIONSLIDER_CENTER))
     return wd->text_center;

   if ((wd->final_position == 1.0) &&
       (wd->enabled_position & ELM_ACTIONSLIDER_RIGHT))
     return wd->text_right;

   return NULL;
}

EAPI void
elm_actionslider_indicator_label_set(Evas_Object *obj, const char *label)
{
   _elm_actionslider_label_set(obj, NULL, label);
}

EAPI const char *
elm_actionslider_indicator_label_get(Evas_Object *obj)
{
   return _elm_actionslider_label_get(obj, NULL);
}
