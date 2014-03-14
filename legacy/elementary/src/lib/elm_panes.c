#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_layout.h"
#include "elm_widget_panes.h"

EAPI Eo_Op ELM_OBJ_PANES_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_PANES_CLASS

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

static void
_elm_panes_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   double size;
   Evas_Coord minw = 0, minh = 0;

   Elm_Panes_Smart_Data *sd = _pd;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   ELM_LAYOUT_DATA_GET(obj, ld);

   if (sd->horizontal)
     eina_stringshare_replace(&ld->group, "horizontal");
   else
     eina_stringshare_replace(&ld->group, "vertical");

   evas_object_hide(sd->event);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->event, minw, minh);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

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

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_panes_smart_focus_next(Eo *obj, void *_pd, va_list *list)
{
   double w, h;
   unsigned char i;
   Evas_Object *to_focus;
   Evas_Object *chain[2];
   Evas_Object *left, *right;

   Elm_Panes_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.bar", &w, &h);

   left = elm_layout_content_get(obj, "left");
   right = elm_layout_content_get(obj, "right");

   if (((sd->horizontal) && (h == 0.0)) || ((!sd->horizontal) && (w == 0.0)))
     {
       int_ret = elm_widget_focus_next_get(right, dir, next);
       if (ret) *ret = int_ret;
       return;
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
   else return;

   i = elm_widget_focus_get(chain[1]);

   if (elm_widget_focus_next_get(chain[i], dir, next))
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }

   i = !i;

   if (elm_widget_focus_next_get(chain[i], dir, &to_focus))
     {
        *next = to_focus;
        if (ret) *ret = !!i;
        return;
     }
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

static void
_elm_panes_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
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

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
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

EAPI double
elm_panes_content_left_size_get(const Evas_Object *obj)
{
   ELM_PANES_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_panes_content_left_size_get(&ret));
   return ret;
}

static void
_elm_panes_smart_content_left_size_get(Eo *obj, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   double w, h;

   Elm_Panes_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_part_drag_value_get
     (wd->resize_obj, "elm.bar", &w, &h);

   if (sd->horizontal) *ret = h;
   else *ret = w;
}

EAPI void
elm_panes_content_left_size_set(Evas_Object *obj,
                                double size)
{
   ELM_PANES_CHECK(obj);
   eo_do(obj, elm_obj_panes_content_left_size_set(size));
}

static void
_elm_panes_smart_content_left_size_set(Eo *obj, void *_pd, va_list *list)
{
   double size = va_arg(*list, double);
   Elm_Panes_Smart_Data *sd = _pd;
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

EAPI double
elm_panes_content_right_size_get(const Evas_Object *obj)
{
   double ret = 1.0;
   eo_do((Eo *) obj, elm_obj_panes_content_right_size_get(&ret));
   return ret;
}

static void
_elm_panes_smart_content_right_size_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   double *ret = va_arg(*list, double *);
   *ret = 1.0 - elm_panes_content_left_size_get(obj);
}

EAPI void
elm_panes_content_right_size_set(Evas_Object *obj,
                                 double size)
{
   elm_panes_content_left_size_set(obj, (1.0 - size));
   eo_do(obj, elm_obj_panes_content_right_size_set(size));
}

static void
_elm_panes_smart_content_right_size_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   double size = va_arg(*list, double);
   elm_panes_content_left_size_set(obj, (1.0 - size));
}

EAPI void
elm_panes_horizontal_set(Evas_Object *obj,
                         Eina_Bool horizontal)
{
   ELM_PANES_CHECK(obj);
   eo_do(obj, elm_obj_panes_horizontal_set(horizontal));
}

static void
_elm_panes_smart_horizontal_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool horizontal = va_arg(*list, int);
   Elm_Panes_Smart_Data *sd = _pd;

   sd->horizontal = horizontal;
   eo_do(obj, elm_obj_widget_theme_apply(NULL));

   elm_panes_content_left_size_set(obj, 0.5);
}

EAPI Eina_Bool
elm_panes_horizontal_get(const Evas_Object *obj)
{
   ELM_PANES_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_panes_horizontal_get(&ret));
   return ret;
}

static void
_elm_panes_smart_horizontal_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Panes_Smart_Data *sd = _pd;
   *ret = sd->horizontal;
}

EAPI void
elm_panes_fixed_set(Evas_Object *obj, Eina_Bool fixed)
{
   ELM_PANES_CHECK(obj);
   eo_do(obj, elm_obj_panes_fixed_set(fixed));
}

static void
_elm_panes_smart_fixed_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool fixed = va_arg(*list, int);
   Elm_Panes_Smart_Data *sd = _pd;

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

EAPI Eina_Bool
elm_panes_fixed_get(const Evas_Object *obj)
{
   ELM_PANES_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_panes_fixed_get(&ret));
   return ret;
}

static void
_elm_panes_smart_fixed_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Panes_Smart_Data *sd = _pd;
   *ret = sd->fixed;
}

static void
_elm_panes_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_panes_smart_content_aliases_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   const Elm_Layout_Part_Alias_Description **aliases = va_arg(*list, const Elm_Layout_Part_Alias_Description **);
   *aliases = _content_aliases;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_panes_smart_add),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_panes_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_panes_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_panes_smart_focus_next),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_CONTENT_ALIASES_GET), _elm_panes_smart_content_aliases_get),

        EO_OP_FUNC(ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_GET), _elm_panes_smart_content_left_size_get),
        EO_OP_FUNC(ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_SET), _elm_panes_smart_content_left_size_set),
        EO_OP_FUNC(ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_GET), _elm_panes_smart_content_right_size_get),
        EO_OP_FUNC(ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_SET), _elm_panes_smart_content_right_size_set),
        EO_OP_FUNC(ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_HORIZONTAL_SET), _elm_panes_smart_horizontal_set),
        EO_OP_FUNC(ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_HORIZONTAL_GET), _elm_panes_smart_horizontal_get),
        EO_OP_FUNC(ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_FIXED_SET), _elm_panes_smart_fixed_set),
        EO_OP_FUNC(ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_FIXED_GET), _elm_panes_smart_fixed_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_GET, "Get the size proportion of panes widget's left side."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_SET, "Set the size proportion of panes widget's left side."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_GET, "Get the size proportion of panes widget's right side."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_SET, "Set the size proportion of panes widget's right side."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANES_SUB_ID_HORIZONTAL_SET, "Set how to split and dispose each content."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANES_SUB_ID_HORIZONTAL_GET, "Retrieve the split direction of a given panes widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_PANES_SUB_ID_FIXED_SET, "Set whether the left and right panes can be resized by user interaction"),
     EO_OP_DESCRIPTION(ELM_OBJ_PANES_SUB_ID_FIXED_GET, "Retrieve the resize mode for the panes of a given panes widget."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_PANES_BASE_ID, op_desc, ELM_OBJ_PANES_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Panes_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_panes_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
