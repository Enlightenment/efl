#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_LAYOUT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_widget_panes.h"

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

EOLIAN static Elm_Theme_Apply
_elm_panes_elm_widget_theme_apply(Eo *obj, Elm_Panes_Data *sd)
{
   double size;
   Evas_Coord minw = 0, minh = 0;

   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;
   ELM_LAYOUT_DATA_GET(obj, ld);

   if (sd->orientation == EFL_ORIENT_HORIZONTAL)
     eina_stringshare_replace(&ld->group, "horizontal");
   else
     eina_stringshare_replace(&ld->group, "vertical");

   evas_object_hide(sd->event);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->event, minw, minh);

   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

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

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_panes_elm_widget_focus_next(Eo *obj, Elm_Panes_Data *sd EINA_UNUSED, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   Eina_Bool int_ret = EINA_FALSE;

   Eina_List *items = NULL;
   Eina_List *(*list_free)(Eina_List *list);
   void *(*list_data_get)(const Eina_List *list);
   Evas_Object *left = NULL;
   Evas_Object *right = NULL;

   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = (Eina_List *)elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        left = elm_layout_content_get(obj, "left");
        if (left)
          items = eina_list_append(items, left);

        right = elm_layout_content_get(obj, "right");
        if (right)
          items = eina_list_append(items, right);

        list_data_get = eina_list_data_get;
        list_free = eina_list_free;

        if (!items) return EINA_FALSE;
     }

   int_ret = elm_widget_focus_list_next_get
      (obj, (const Eina_List *)items, list_data_get, dir, next, next_item);

   if (list_free) list_free((Eina_List *)items);

   return int_ret;
}

static void
_on_clicked(void *data,
            Evas_Object *obj EINA_UNUSED,
            const char *emission EINA_UNUSED,
            const char *source EINA_UNUSED)
{
   efl_event_callback_legacy_call(data, EFL_UI_EVENT_CLICKED, NULL);
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
   efl_event_callback_legacy_call(data, ELM_PANES_EVENT_PRESS, NULL);
}

static void
_on_unpressed(void *data,
              Evas_Object *obj EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   ELM_PANES_DATA_GET(data, sd);
   efl_event_callback_legacy_call(data, ELM_PANES_EVENT_UNPRESS, NULL);

   if (sd->double_clicked)
     {
        efl_event_callback_legacy_call(data, EFL_UI_EVENT_CLICKED_DOUBLE, NULL);
        sd->double_clicked = EINA_FALSE;
     }
}

static void
_set_min_size(void *data)
{
   ELM_PANES_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   double sizer = sd->right_min_relative_size;
   double sizel = sd->left_min_relative_size;
   if ((sd->left_min_relative_size + sd->right_min_relative_size) > 1)
     {
        double sum = sizer + sizel;
        sizer = sizer / sum;
        sizel = sizel / sum;
     }
   if (sd->orientation == EFL_ORIENT_HORIZONTAL)
     {
        edje_object_part_drag_value_set
           (wd->resize_obj, "right_constraint", 0.0, (1 - sizer));
        edje_object_part_drag_value_set
           (wd->resize_obj, "left_constraint", 0.0, sizel);
     }
   else
     {
        edje_object_part_drag_value_set
           (wd->resize_obj, "right_constraint", (1 - sizer), 0.0);
        edje_object_part_drag_value_set
           (wd->resize_obj, "left_constraint", sizel, 0.0);
     }
}

static void
_update_fixed_sides(void *data)
{
   ELM_PANES_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);
   Evas_Coord w, h;
   evas_object_geometry_get(wd->resize_obj, NULL, NULL, &w, &h);

   if (sd->right_min_size_is_relative)
     {
        if (sd->orientation == EFL_ORIENT_HORIZONTAL)
           sd->right_min_size = (int)(h * sd->right_min_relative_size);
        else
           sd->right_min_size =(int)(w * sd->right_min_relative_size);
     }
   else
     {
        sd->right_min_relative_size = 0;
        if (sd->orientation == EFL_ORIENT_HORIZONTAL && (h > 0))
              sd->right_min_relative_size = sd->right_min_size / (double)h;
        if (sd->orientation == EFL_ORIENT_VERTICAL && (w > 0))
              sd->right_min_relative_size = sd->right_min_size / (double)w;
     }

   if(sd->left_min_size_is_relative)
     {
        if (sd->orientation == EFL_ORIENT_HORIZONTAL)
             sd->left_min_size = (int)(h * sd->left_min_relative_size);
        else
           sd->left_min_size = (int)(w * sd->left_min_relative_size);
     }
   else
     {
        sd->left_min_relative_size = 0;
        if (sd->orientation == EFL_ORIENT_HORIZONTAL && (h > 0))
           sd->left_min_relative_size = sd->left_min_size / (double)h;
        if (sd->orientation == EFL_ORIENT_VERTICAL && (w > 0))
           sd->left_min_relative_size = sd->left_min_size / (double)w;
     }
   _set_min_size(data);
}

static void
_on_resize(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
  _update_fixed_sides(data);
}

EOLIAN static void
_elm_panes_efl_canvas_group_group_add(Eo *obj, Elm_Panes_Data *_pd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   ELM_PANES_DATA_GET(obj, sd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
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
   evas_object_event_callback_add
     (wd->resize_obj, EVAS_CALLBACK_RESIZE,
     _on_resize, obj);

   sd->right_min_size_is_relative = EINA_TRUE;
   sd->left_min_size_is_relative = EINA_TRUE;
   sd->right_min_size = 0;
   sd->left_min_size = 0;
   sd->right_min_relative_size = 0;
   sd->left_min_relative_size = 0;
   _update_fixed_sides(obj);

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
   Evas_Object *obj = efl_add(MY_CLASS, parent);
   return obj;
}

EOLIAN static Eo *
_elm_panes_efl_object_constructor(Eo *obj, Elm_Panes_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_SPLIT_PANE);

   return obj;
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

   if (sd->orientation == EFL_ORIENT_HORIZONTAL)
     return h;
   else return w;
}

EOLIAN static void
_elm_panes_content_left_size_set(Eo *obj, Elm_Panes_Data *sd, double size)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (size < 0.0) size = 0.0;
   else if (size > 1.0) size = 1.0;

   if (sd->orientation == EFL_ORIENT_HORIZONTAL)
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
_elm_panes_efl_orientation_orientation_set(Eo *obj, Elm_Panes_Data *sd, Efl_Orient dir)
{
   if ((dir != EFL_ORIENT_HORIZONTAL) && (dir != EFL_ORIENT_VERTICAL))
     return;

   double size = elm_panes_content_left_size_get(obj);

   sd->orientation = dir;
   elm_obj_widget_theme_apply(obj);
   _update_fixed_sides(obj);

   elm_panes_content_left_size_set(obj, size);
}

EOLIAN static Efl_Orient
_elm_panes_efl_orientation_orientation_get(Eo *obj EINA_UNUSED, Elm_Panes_Data *sd)
{
   return sd->orientation;
}

EAPI void
elm_panes_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   Efl_Orient orient;

   if (horizontal)
     orient = EFL_ORIENT_HORIZONTAL;
   else
     orient = EFL_ORIENT_VERTICAL;

   efl_orientation_set(obj, orient);
}

EAPI Eina_Bool
elm_panes_horizontal_get(const Evas_Object *obj)
{
   Efl_Orient orient = efl_orientation_get(obj);

   if (orient == EFL_ORIENT_HORIZONTAL)
     return EINA_TRUE;

   return EINA_FALSE;
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

EOLIAN static void
_elm_panes_content_left_min_relative_size_set(Eo *obj, Elm_Panes_Data *_pd, double size)
{
   _pd->left_min_relative_size = size;
   if (_pd->left_min_relative_size < 0) _pd->left_min_relative_size = 0;
   _pd->left_min_size_is_relative = EINA_TRUE;
   _update_fixed_sides(obj);
}

EOLIAN static double
_elm_panes_content_left_min_relative_size_get(Eo *obj EINA_UNUSED, Elm_Panes_Data *_pd)
{
   return _pd->left_min_relative_size;
}

EOLIAN static void
_elm_panes_content_right_min_relative_size_set(Eo *obj, Elm_Panes_Data *_pd, double size)
{
   _pd->right_min_relative_size = size;
   if (_pd->right_min_relative_size < 0) _pd->right_min_relative_size = 0;
   _pd->right_min_size_is_relative = EINA_TRUE;
   _update_fixed_sides(obj);
}

EOLIAN static double
_elm_panes_content_right_min_relative_size_get(Eo *obj EINA_UNUSED, Elm_Panes_Data *_pd)
{
   return _pd->right_min_relative_size;
}

EOLIAN static void
_elm_panes_content_left_min_size_set(Eo *obj, Elm_Panes_Data *_pd, Evas_Coord size)
{
   _pd->left_min_size = size;
   if (_pd->left_min_size < 0) _pd->left_min_size = 0;
   _pd->left_min_size_is_relative = EINA_FALSE;
   _update_fixed_sides(obj);
}

EOLIAN static Evas_Coord
_elm_panes_content_left_min_size_get(Eo *obj EINA_UNUSED, Elm_Panes_Data *_pd)
{
   return _pd->left_min_size;
}

EOLIAN static void
_elm_panes_content_right_min_size_set(Eo *obj, Elm_Panes_Data *_pd, Evas_Coord size)
{
   _pd->right_min_size = size;
   if (_pd->right_min_size < 0) _pd->right_min_size = 0;
   _pd->right_min_size_is_relative = EINA_FALSE;
   _update_fixed_sides(obj);
}

EOLIAN static Evas_Coord
_elm_panes_content_right_min_size_get(Eo *obj EINA_UNUSED, Elm_Panes_Data *_pd)
{
   return _pd->right_min_size;
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
_elm_panes_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Internal EO APIs and hidden overrides */

#define ELM_PANES_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(elm_panes)

#include "elm_panes.eo.c"
