#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_panes.h"

/**
 * TODO
 * Update the minimun height of the bar in the theme.
 * No minimun should be set in the vertical theme
 * Add events (move, start ...)
 */

EAPI const char ELM_PANES_SMART_NAME[] = "elm_panes";

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
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_PANES_SMART_NAME, _elm_panes, Elm_Panes_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static Eina_Bool
_elm_panes_smart_theme(Evas_Object *obj)
{
   double size;

   ELM_PANES_DATA_GET(obj, sd);

   if (sd->horizontal)
     eina_stringshare_replace(&(ELM_LAYOUT_DATA(sd)->group), "horizontal");
   else
     eina_stringshare_replace(&(ELM_LAYOUT_DATA(sd)->group), "vertical");

   if (!ELM_WIDGET_CLASS(_elm_panes_parent_sc)->theme(obj)) return EINA_FALSE;

   size = elm_panes_content_left_size_get(obj);

   if (sd->fixed) elm_layout_signal_emit(obj, "elm.panes.fixed", "elm");

   elm_layout_sizing_eval(obj);

   elm_panes_content_left_size_set(obj, size);

   return EINA_TRUE;
}

static Eina_Bool
_elm_panes_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   double w, h;
   unsigned char i;
   Evas_Object *to_focus;
   Evas_Object *chain[2];
   Evas_Object *left, *right;

   ELM_PANES_DATA_GET(obj, sd);

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.bar", &w, &h);

   left = elm_layout_content_get(obj, "left");
   right = elm_layout_content_get(obj, "right");

   if (((sd->horizontal) && (h == 0.0)) || ((!sd->horizontal) && (w == 0.0)))
     return elm_widget_focus_next_get(right, dir, next);

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
            Evas_Object *obj __UNUSED__,
            const char *emission __UNUSED__,
            const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_double_clicked(void *data,
                Evas_Object *obj __UNUSED__,
                const char *emission __UNUSED__,
                const char *source __UNUSED__)
{
   ELM_PANES_DATA_GET(data, sd);

   sd->double_clicked = EINA_TRUE;
}

static void
_on_pressed(void *data,
            Evas_Object *obj __UNUSED__,
            const char *emission __UNUSED__,
            const char *source __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_PRESS, NULL);
}

static void
_on_unpressed(void *data,
              Evas_Object *obj __UNUSED__,
              const char *emission __UNUSED__,
              const char *source __UNUSED__)
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
_elm_panes_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Panes_Smart_Data);

   ELM_WIDGET_CLASS(_elm_panes_parent_sc)->base.add(obj);

   elm_layout_theme_set(obj, "panes", "vertical", elm_widget_style_get(obj));

   elm_panes_content_left_size_set(obj, 0.5);

   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,click", "",
     _on_clicked, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,click,double", "",
     _double_clicked, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,press", "",
     _on_pressed, obj);
   edje_object_signal_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, "elm,action,unpress", "",
     _on_unpressed, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   elm_layout_sizing_eval(obj);
}

static void
_elm_panes_smart_set_user(Elm_Panes_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_panes_smart_add;

   ELM_WIDGET_CLASS(sc)->theme = _elm_panes_smart_theme;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_panes_smart_focus_next;

   ELM_LAYOUT_CLASS(sc)->content_aliases = _content_aliases;
}

EAPI const Elm_Panes_Smart_Class *
elm_panes_smart_class_get(void)
{
   static Elm_Panes_Smart_Class _sc =
     ELM_PANES_SMART_CLASS_INIT_NAME_VERSION(ELM_PANES_SMART_NAME);
   static const Elm_Panes_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_panes_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_panes_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_panes_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

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

EAPI double
elm_panes_content_left_size_get(const Evas_Object *obj)
{
   double w, h;

   ELM_PANES_CHECK(obj) 0.0;
   ELM_PANES_DATA_GET(obj, sd);

   edje_object_part_drag_value_get
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.bar", &w, &h);

   if (sd->horizontal) return h;
   else return w;
}

EAPI void
elm_panes_content_left_size_set(Evas_Object *obj,
                                double size)
{
   ELM_PANES_CHECK(obj);
   ELM_PANES_DATA_GET(obj, sd);

   if (size < 0.0) size = 0.0;
   else if (size > 1.0)
     size = 1.0;
   if (sd->horizontal)
     edje_object_part_drag_value_set
       (ELM_WIDGET_DATA(sd)->resize_obj, "elm.bar", 0.0, size);
   else
     edje_object_part_drag_value_set
       (ELM_WIDGET_DATA(sd)->resize_obj, "elm.bar", size, 0.0);
}

EAPI double
elm_panes_content_right_size_get(const Evas_Object *obj)
{
   return 1.0 - elm_panes_content_left_size_get(obj);
}

EAPI void
elm_panes_content_right_size_set(Evas_Object *obj,
                                 double size)
{
   elm_panes_content_left_size_set(obj, (1.0 - size));
}

EAPI void
elm_panes_horizontal_set(Evas_Object *obj,
                         Eina_Bool horizontal)
{
   ELM_PANES_CHECK(obj);
   ELM_PANES_DATA_GET(obj, sd);

   sd->horizontal = horizontal;
   _elm_panes_smart_theme(obj);

   elm_panes_content_left_size_set(obj, 0.5);
}

EAPI Eina_Bool
elm_panes_horizontal_get(const Evas_Object *obj)
{
   ELM_PANES_CHECK(obj) EINA_FALSE;
   ELM_PANES_DATA_GET(obj, sd);

   return sd->horizontal;
}

EAPI void
elm_panes_fixed_set(Evas_Object *obj, Eina_Bool fixed)
{
   ELM_PANES_CHECK(obj);
   ELM_PANES_DATA_GET(obj, sd);

   sd->fixed = !!fixed;
   if (sd->fixed == EINA_TRUE)
     elm_layout_signal_emit(obj, "elm.panes.fixed", "elm");
   else
     elm_layout_signal_emit(obj, "elm.panes.unfixed", "elm");
}

EAPI Eina_Bool
elm_panes_fixed_get(const Evas_Object *obj)
{
   ELM_PANES_CHECK(obj) EINA_FALSE;
   ELM_PANES_DATA_GET(obj, sd);

   return sd->fixed;
}
