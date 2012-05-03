#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_layout.h"

static const char SEPARATOR_SMART_NAME[] = "elm_separator";

typedef struct _Elm_Separator_Smart_Data Elm_Separator_Smart_Data;

struct _Elm_Separator_Smart_Data
{
   Elm_Layout_Smart_Data base;

   Eina_Bool             horizontal : 1;
};

#define ELM_SEPARATOR_DATA_GET(o, sd) \
  Elm_Separator_Smart_Data * sd = evas_object_smart_data_get(o)

#define ELM_SEPARATOR_DATA_GET_OR_RETURN(o, ptr)     \
  ELM_SEPARATOR_DATA_GET(o, ptr);                    \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_SEPARATOR_DATA_GET_OR_RETURN_VAL(o, ptr, val) \
  ELM_SEPARATOR_DATA_GET(o, ptr);                         \
  if (!ptr)                                               \
    {                                                     \
       CRITICAL("No widget data for object %p (%s)",      \
                o, evas_object_type_get(o));              \
       return val;                                        \
    }

#define ELM_SEPARATOR_CHECK(obj)                                             \
  if (!obj || !elm_widget_type_check((obj), SEPARATOR_SMART_NAME, __func__)) \
    return

/* Inheriting from elm_layout. Besides, we need no more than what is
 * there */
EVAS_SMART_SUBCLASS_NEW
  (SEPARATOR_SMART_NAME, _elm_separator, Elm_Layout_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, NULL);

static Eina_Bool
_elm_separator_smart_theme(Evas_Object *obj)
{
   ELM_SEPARATOR_DATA_GET(obj, sd);

   if (sd->horizontal)
     eina_stringshare_replace(&(ELM_LAYOUT_DATA(sd)->group), "horizontal");
   else
     eina_stringshare_replace(&(ELM_LAYOUT_DATA(sd)->group), "vertical");

   if (!ELM_WIDGET_CLASS(_elm_separator_parent_sc)->theme(obj)) return EINA_FALSE;

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
_elm_separator_smart_set_user(Elm_Layout_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_separator_smart_add;

   ELM_WIDGET_CLASS(sc)->theme = _elm_separator_smart_theme;

   /* not a 'focus chain manager' */
   ELM_WIDGET_CLASS(sc)->focus_next = NULL;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;

   sc->sizing_eval = _elm_separator_smart_sizing_eval;
}

EAPI Evas_Object *
elm_separator_add(Evas_Object *parent)
{
   Evas *e;
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   e = evas_object_evas_get(parent);
   if (!e) return NULL;

   obj = evas_object_smart_add(e, _elm_separator_smart_class_new());

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
