#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_widget_panes.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define MY_CLASS ELM_PANES_CLASS

#define MY_CLASS_NAME "Elm_Panes"
#define MY_CLASS_NAME_LEGACY "elm_panes"
/**
 * TODO
 * Update the minimun height of the bar in the theme.
 * No minimun should be set in the vertical theme
 * Add events (move, start ...)
 */

static const char SIG_CLICKED[] = "clicked";
static const char SIG_PRESS[] = "press";
static const char SIG_UNPRESS[] = "unpress";
static const char SIG_DOUBLE_CLICKED[] = "clicked,double";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_PRESS, ""},
   {SIG_UNPRESS, ""},
   {SIG_DOUBLE_CLICKED, ""},
   {NULL, NULL}
};

static const Elm_Layout_Part_Alias_Description _content_aliases[] =
{
   {"left", "elm.swallow.left"},
   {"right", "elm.swallow.right"},
   {"top", "elm.swallow.left"},
   {"bottom", "elm.swallow.right"},
   {NULL, NULL}
};

EOLIAN static Eina_Bool
_elm_panes_elm_widget_theme_apply(Eo *obj, Elm_Panes_Data *sd)
{
   double size;
   Evas_Coord minw = 0, minh = 0;

   Eina_Bool int_ret = EINA_FALSE;
   ELM_LAYOUT_DATA_GET(obj, ld);

   if (sd->horizontal)
     eina_stringshare_replace(&ld->group, "horizontal");
   else
     eina_stringshare_replace(&ld->group, "vertical");

   evas_object_hide(sd->event);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->event, minw, minh);

   eo_do_super(obj, MY_CLASS, int_ret = elm_obj_widget_theme_apply());
   if (!int_ret) return EINA_FALSE;

   size = elm_panes_content_left_size_get(obj);

   if (sd->fixed)
     {
        elm_layout_signal_emit(obj, "elm,panes,fixed", "elm");

        //TODO: remove this signal on EFL 2.0.
        // I left this due to the backward compatibility.
        elm_layout_signal_emit(obj, "elm.panes.fixed", "elm");
     }

   elm_layout_sizing_eval(obj);

   elm_panes_content_left_size_set(obj, size);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_panes_elm_widget_focus_next(Eo *obj, Elm_Panes_Data *sd, Elm_Focus_Direction dir, Evas_Object **next)
{
   double w, h;
   unsigned char i;
   Evas_Object *to_focus;
   Evas_Object *chain[2];
   Evas_Object *left, *right;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.bar", &w, &h);

   left = elm_layout_content_get(obj, "left");
   right = elm_layout_content_get(obj, "right");

   if (((sd->horizontal) && (h == 0.0)) || ((!sd->horizontal) && (w == 0.0)))
     {
       return elm_widget_focus_next_get(right, dir, next);
     }

   /* Direction */
   if (dir == ELM_FOCUS_PREVIOUS)
     {
        chain[0] = right;
        chain[1] = left;
     }
   else if (dir == ELM_FOCUS_NEXT)
     {
        chain[0] = left;
        chain[1] = right;
     }
   else return EINA_FALSE;

   i = elm_widget_focus_get(chain[1]);

   if (elm_widget_focus_next_get(chain[i], dir, next)) return EINA_TRUE;

   i = !i;

   if (elm_widget_focus_next_get(chain[i], dir, &to_focus))
     {
        *next = to_focus;
        return !!i;
     }

   return EINA_FALSE;
}

static void
_on_clicked(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_double_clicked(void *data,
                Evas_Object *obj EINA_UNUSED,
                const char *emission EINA_UNUSED,
                const char *source EINA_UNUSED)
{
   ELM_PANES_DATA_GET(data, sd);

   sd->double_clicked = EINA_TRUE;
}

static void
_on_pressed(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   evas_object_smart_callback_call(data, SIG_PRESS, NULL);
}

static void
_on_unpressed(void *data,
              Evas_Object *obj EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   ELM_PANES_DATA_GET(data, sd);
   evas_object_smart_callback_call(data, SIG_UNPRESS, NULL);

   if (sd->double_clicked)
     {
        evas_object_smart_callback_call(data, SIG_DOUBLE_CLICKED, NULL);
        sd->double_clicked = EINA_FALSE;
     }
}

EOLIAN static void
_elm_panes_evas_object_smart_add(Eo *obj, Elm_Panes_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   ELM_PANES_DATA_GET(obj, sd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set
       (obj, "panes", "vertical", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   elm_panes_content_left_size_set(obj, 0.5);

   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,click", "*",
     _on_clicked, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,click,double", "*",
     _double_clicked, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,press", "*",
     _on_pressed, obj);
   edje_object_signal_callback_add
     (wd->resize_obj, "elm,action,unpress", "*",
     _on_unpressed, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   sd->event = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(sd->event, 0, 0, 0, 0);
   evas_object_pass_events_set(sd->event, EINA_TRUE);
   if (edje_object_part_exists
       (wd->resize_obj, "elm.swallow.event"))
     {
        Evas_Coord minw = 0, minh = 0;

        elm_coords_finger_size_adjust(1, &minw, 1, &minh);
        evas_object_size_hint_min_set(sd->event, minw, minh);
        elm_layout_content_set(obj, "elm.swallow.event", sd->event);
     }
   elm_widget_sub_object_add(obj, sd->event);

   elm_layout_sizing_eval(obj);
}

EAPI Evas_Object *
elm_panes_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

EOLIAN static void
_elm_panes_eo_base_constructor(Eo *obj, Elm_Panes_Data *_pd EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks),
         elm_interface_atspi_accessible_role_set(ELM_ATSPI_ROLE_SPLIT_PANE));
}

EINA_DEPRECATED EAPI void
elm_panes_content_left_set(Evas_Object *obj,
                           Evas_Object *content)
{
   elm_layout_content_set(obj, "left", content);
}

EINA_DEPRECATED EAPI void
elm_panes_content_right_set(Evas_Object *obj,
                            Evas_Object *content)
{
   elm_layout_content_set(obj, "right", content);
}

EINA_DEPRECATED EAPI Evas_Object *
elm_panes_content_left_get(const Evas_Object *obj)
{
   return elm_layout_content_get(obj, "left");
}

EINA_DEPRECATED EAPI Evas_Object *
elm_panes_content_right_get(const Evas_Object *obj)
{
   return elm_layout_content_get(obj, "right");
}

EINA_DEPRECATED EAPI Evas_Object *
elm_panes_content_left_unset(Evas_Object *obj)
{
   return elm_layout_content_unset(obj, "left");
}

EINA_DEPRECATED EAPI Evas_Object *
elm_panes_content_right_unset(Evas_Object *obj)
{
   return elm_layout_content_unset(obj, "right");
}

EOLIAN static double
_elm_panes_content_left_size_get(Eo *obj, Elm_Panes_Data *sd)
{
   double w, h;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0.0);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.bar", &w, &h);

   if (sd->horizontal) return h;
   else return w;
}

EOLIAN static void
_elm_panes_content_left_size_set(Eo *obj, Elm_Panes_Data *sd, double size)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (size < 0.0) size = 0.0;
   else if (size > 1.0) size = 1.0;

   if (sd->horizontal)
     edje_object_part_drag_value_set
       (wd->resize_obj, "elm.bar", 0.0, size);
   else
     edje_object_part_drag_value_set
       (wd->resize_obj, "elm.bar", size, 0.0);
}

EOLIAN static double
_elm_panes_content_right_size_get(Eo *obj, Elm_Panes_Data *_pd EINA_UNUSED)
{
   return 1.0 - elm_panes_content_left_size_get(obj);
}

EOLIAN static void
_elm_panes_content_right_size_set(Eo *obj, Elm_Panes_Data *_pd EINA_UNUSED, double size)
{
   elm_panes_content_left_size_set(obj, (1.0 - size));
}

EOLIAN static void
_elm_panes_horizontal_set(Eo *obj, Elm_Panes_Data *sd, Eina_Bool horizontal)
{
   sd->horizontal = horizontal;
   eo_do(obj, elm_obj_widget_theme_apply());

   elm_panes_content_left_size_set(obj, 0.5);
}

EOLIAN static Eina_Bool
_elm_panes_horizontal_get(Eo *obj EINA_UNUSED, Elm_Panes_Data *sd)
{
   return sd->horizontal;
}

EOLIAN static void
_elm_panes_fixed_set(Eo *obj, Elm_Panes_Data *sd, Eina_Bool fixed)
{
   sd->fixed = !!fixed;
   if (sd->fixed == EINA_TRUE)
     {
        elm_layout_signal_emit(obj, "elm,panes,fixed", "elm");

        //TODO: remove this signal on EFL 2.0.
        // I left this due to the backward compatibility.
        elm_layout_signal_emit(obj, "elm.panes.fixed", "elm");
     }
   else
     {
        elm_layout_signal_emit(obj, "elm,panes,unfixed", "elm");

        //TODO: remove this signal on EFL 2.0.
        // I left this due to the backward compatibility.
        elm_layout_signal_emit(obj, "elm.panes.unfixed", "elm");
     }
}

EOLIAN static Eina_Bool
_elm_panes_fixed_get(Eo *obj EINA_UNUSED, Elm_Panes_Data *sd)
{
   return sd->fixed;
}

EOLIAN static Eina_Bool
_elm_panes_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Panes_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static const Elm_Layout_Part_Alias_Description*
_elm_panes_elm_layout_content_aliases_get(Eo *obj EINA_UNUSED, Elm_Panes_Data *_pd EINA_UNUSED)
{
   return _content_aliases;
}

static void
_elm_panes_class_constructor(Eo_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

#include "elm_panes.eo.c"
