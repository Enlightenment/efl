#include <Elementary.h>
#include "elm_priv.h"

/**
 * TODO
 * Update the minimun height of the bar in the theme.
 * No minimun should be set in the vertical theme
 * Add events (move, start ...)
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *panes;

   struct
     {
        Evas_Object *left;
        Evas_Object *right;
     } contents;

   struct
     {
        int x_diff;
        int y_diff;
        Eina_Bool move;
     } move;

   Eina_Bool clicked_double;
   Eina_Bool horizontal;
   Eina_Bool fixed;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);

static const char SIG_CLICKED[] = "clicked";
static const char SIG_PRESS[] = "press";
static const char SIG_UNPRESS[] = "unpress";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CLICKED, ""},
   {SIG_PRESS, ""},
   {SIG_UNPRESS, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->panes, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const char *style = elm_widget_style_get(obj);
   double size;

   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   size = elm_panes_content_left_size_get(obj);

   if (wd->horizontal)
     _elm_theme_object_set(obj, wd->panes, "panes", "horizontal", style);
   else
     _elm_theme_object_set(obj, wd->panes, "panes", "vertical", style);

   if (wd->contents.left)
     edje_object_part_swallow(wd->panes, "elm.swallow.left", wd->contents.left);
   if (wd->contents.right)
     edje_object_part_swallow(wd->panes, "elm.swallow.right", wd->contents.right);
   if (wd->fixed)
     edje_object_signal_emit(wd->panes, "elm.panes.fixed", "elm");

   edje_object_scale_set(wd->panes, elm_widget_scale_get(obj) *
                         _elm_config->scale);
   _sizing_eval(obj);
   elm_panes_content_left_size_set(obj, size);
}

static Eina_Bool
_elm_panes_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd)
     return EINA_FALSE;

   double w, h;
   edje_object_part_drag_value_get(wd->panes, "elm.bar", &w, &h);
   if (((wd->horizontal) && ( h == 0.0 )) || ((!wd->horizontal) && ( w == 0.0 )))
     return elm_widget_focus_next_get(wd->contents.right, dir, next);

   Evas_Object *chain[2];

   /* Direction */
   if (dir == ELM_FOCUS_PREVIOUS)
     {
        chain[0] = wd->contents.right;
        chain[1] = wd->contents.left;
     }
   else if (dir == ELM_FOCUS_NEXT)
     {
        chain[0] = wd->contents.left;
        chain[1] = wd->contents.right;
     }
   else
     return EINA_FALSE;

   unsigned char i = elm_widget_focus_get(chain[1]);

   if (elm_widget_focus_next_get(chain[i], dir, next))
     return EINA_TRUE;

   i = !i;

   Evas_Object *to_focus;
   if (elm_widget_focus_next_get(chain[i], dir, &to_focus))
     {
        *next = to_focus;
        return !!i;
     }

   return EINA_FALSE;
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;

   if (!wd) return;
   if (sub == wd->contents.left)
     {
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->contents.left = NULL;
        _sizing_eval(obj);
     }
   else if (sub == wd->contents.right)
     {
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _changed_size_hints, obj);
        wd->contents.right= NULL;
        _sizing_eval(obj);
     }
}


static void
_clicked(void *data, Evas_Object *obj __UNUSED__ , const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_clicked_double(void *data, Evas_Object *obj __UNUSED__ , const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   wd->clicked_double = EINA_TRUE;
}

static void
_press(void *data, Evas_Object *obj __UNUSED__ , const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_PRESS, NULL);
}

static void
_unpress(void *data, Evas_Object *obj __UNUSED__ , const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   evas_object_smart_callback_call(data, SIG_UNPRESS, NULL);

   if (wd->clicked_double)
     {
        evas_object_smart_callback_call(data, SIG_CLICKED_DOUBLE, NULL);
        wd->clicked_double = EINA_FALSE;
     }
}

EAPI Evas_Object *
elm_panes_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "panes");
   elm_widget_type_set(obj, "panes");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_focus_next_hook_set(obj, _elm_panes_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   wd->panes = edje_object_add(e);
   _elm_theme_object_set(obj, wd->panes, "panes", "vertical", "default");
   elm_widget_resize_object_set(obj, wd->panes);
   evas_object_show(wd->panes);

   elm_panes_content_left_size_set(obj, 0.5);

   edje_object_signal_callback_add(wd->panes, "elm,action,click", "",
                                   _clicked, obj);
   edje_object_signal_callback_add(wd->panes, "elm,action,click,double", "",
                                   _clicked_double, obj);
   edje_object_signal_callback_add(wd->panes, "elm,action,press", "",
                                   _press, obj);
   edje_object_signal_callback_add(wd->panes, "elm,action,unpress", "",
                                   _unpress, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _sizing_eval(obj);
   return obj;
}


EAPI void
elm_panes_content_left_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->contents.left)
     {
        evas_object_del(wd->contents.left);
        wd->contents.left = NULL;
     }
   if (content)
     {
        wd->contents.left = content;
        elm_widget_sub_object_add(obj, content);
        edje_object_part_swallow(wd->panes, "elm.swallow.left", content);
     }
}

EAPI void
elm_panes_content_right_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->contents.right)
     {
        evas_object_del(wd->contents.right);
        wd->contents.right = NULL;
     }
   if (content)
     {
        wd->contents.right = content;
        elm_widget_sub_object_add(obj, content);
        edje_object_part_swallow(wd->panes, "elm.swallow.right", content);
     }
}

EAPI Evas_Object *
elm_panes_content_left_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->contents.left;
}

EAPI Evas_Object *
elm_panes_content_right_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->contents.right;
}

EAPI Evas_Object *
elm_panes_content_left_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->contents.left) return NULL;
   Evas_Object *content = wd->contents.left;
   elm_widget_sub_object_del(obj, content);
   edje_object_part_unswallow(wd->panes, content);
   wd->contents.left = NULL;
   return content;
}

EAPI Evas_Object *
elm_panes_content_right_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->contents.right) return NULL;
   Evas_Object *content = wd->contents.right;
   elm_widget_sub_object_del(obj, content);
   edje_object_part_unswallow(wd->panes, content);
   wd->contents.right = NULL;
   return content;
}

EAPI double
elm_panes_content_left_size_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);
   double w, h;

   edje_object_part_drag_value_get(wd->panes, "elm.bar", &w, &h);
   if (wd->horizontal) return h;
   else return w;
}

EAPI void
elm_panes_content_left_size_set(Evas_Object *obj, double size)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (size < 0.0) size = 0.0;
   else if (size > 1.0) size = 1.0;
   if (wd->horizontal)
     edje_object_part_drag_value_set(wd->panes, "elm.bar", 0.0, size);
   else
     edje_object_part_drag_value_set(wd->panes, "elm.bar", size, 0.0);
}

EAPI void
elm_panes_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   wd->horizontal = horizontal;
   _theme_hook(obj);
   elm_panes_content_left_size_set(obj, 0.5);
}

EAPI Eina_Bool
elm_panes_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->horizontal;
}

EAPI void
elm_panes_fixed_set(Evas_Object *obj, Eina_Bool fixed)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->fixed = fixed;
   if (wd->fixed == EINA_TRUE)
     edje_object_signal_emit(wd->panes, "elm.panes.fixed", "elm");
   else
     edje_object_signal_emit(wd->panes, "elm.panes.unfixed", "elm");
}

EAPI Eina_Bool
elm_panes_fixed_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   return wd->fixed;
}
