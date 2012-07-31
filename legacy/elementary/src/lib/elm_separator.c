#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_separator.h"

EAPI const char ELM_SEPARATOR_SMART_NAME[] = "elm_separator";

EVAS_SMART_SUBCLASS_NEW
  (ELM_SEPARATOR_SMART_NAME, _elm_separator, Elm_Separator_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, NULL);

static Eina_Bool
_elm_separator_smart_theme(Evas_Object *obj)
{
   ELM_SEPARATOR_DATA_GET(obj, sd);

   if (sd->horizontal)
     eina_stringshare_replace(&(ELM_LAYOUT_DATA(sd)->group), "horizontal");
   else
     eina_stringshare_replace(&(ELM_LAYOUT_DATA(sd)->group), "vertical");

   if (!ELM_WIDGET_CLASS(_elm_separator_parent_sc)->theme(obj))
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_elm_separator_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   ELM_SEPARATOR_DATA_GET(obj, sd);

   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_size_hint_align_set(obj, maxw, maxh);
}

static void
_elm_separator_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Separator_Smart_Data);

   ELM_WIDGET_CLASS(_elm_separator_parent_sc)->base.add(obj);

   priv->horizontal = EINA_FALSE;

   elm_widget_can_focus_set(obj, EINA_FALSE);

   elm_layout_theme_set
     (obj, "separator", "vertical", elm_widget_style_get(obj));

   elm_layout_sizing_eval(obj);
}

static void
_elm_separator_smart_set_user(Elm_Separator_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_separator_smart_add;

   ELM_WIDGET_CLASS(sc)->theme = _elm_separator_smart_theme;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_separator_smart_sizing_eval;
}

EAPI const Elm_Separator_Smart_Class *
elm_separator_smart_class_get(void)
{
   static Elm_Separator_Smart_Class _sc =
     ELM_SEPARATOR_SMART_CLASS_INIT_NAME_VERSION(ELM_SEPARATOR_SMART_NAME);
   static const Elm_Separator_Smart_Class *class = NULL;

   if (class)
     return class;

   _elm_separator_smart_set(&_sc);
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_separator_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_separator_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_separator_horizontal_set(Evas_Object *obj,
                             Eina_Bool horizontal)
{
   ELM_SEPARATOR_CHECK(obj);
   ELM_SEPARATOR_DATA_GET(obj, sd);

   horizontal = !!horizontal;
   if (sd->horizontal == horizontal) return;

   sd->horizontal = horizontal;

   _elm_separator_smart_theme(obj);
}

EAPI Eina_Bool
elm_separator_horizontal_get(const Evas_Object *obj)
{
   ELM_SEPARATOR_CHECK(obj) EINA_FALSE;
   ELM_SEPARATOR_DATA_GET(obj, sd);

   return sd->horizontal;
}
