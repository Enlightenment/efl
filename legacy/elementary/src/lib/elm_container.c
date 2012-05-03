#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_container.h"

static const char CONTAINER_SMART_NAME[] = "elm_container";

/* Elementary smart class for all widgets containing others */
EVAS_SMART_SUBCLASS_NEW
  (CONTAINER_SMART_NAME, _elm_container, Elm_Container_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, NULL);

/* no *direct* instantiation of this class, so far */
__UNUSED__ static Evas_Smart *_elm_container_smart_class_new(void);

static Eina_Bool
_unimplemented_smart_content_set(Evas_Object *obj,
                                 const char *name __UNUSED__,
                                 Evas_Object *content __UNUSED__)
{
    WRN("The %s widget does not implement the \"content_set()\" function.",
         elm_widget_type_get(obj));

    return EINA_FALSE;
}

static Evas_Object *
_unimplemented_smart_content_get(const Evas_Object *obj,
                                 const char *name __UNUSED__)
{
    WRN("The %s widget does not implement the \"content_get()\" function.",
         elm_widget_type_get(obj));

    return NULL;
}

static Evas_Object *
_unimplemented_smart_content_unset(Evas_Object *obj,
                                   const char *name __UNUSED__)
{
    WRN("The %s widunset does not implement the \"content_unset()\" function.",
         elm_widget_type_get(obj));

    return NULL;
}

EAPI const Elm_Container_Smart_Class *
elm_container_smart_class_get(void)
{
   static Elm_Container_Smart_Class _sc =
     ELM_CONTAINER_SMART_CLASS_INIT_NAME_VERSION(CONTAINER_SMART_NAME);
   static const Elm_Container_Smart_Class *class = NULL;

   if (class)
     return class;

   _elm_container_smart_set(&_sc);
   class = &_sc;

   return class;
}

static void
_elm_container_smart_set_user(Elm_Container_Smart_Class *sc)
{
   sc->content_set = _unimplemented_smart_content_set;
   sc->content_get = _unimplemented_smart_content_get;
   sc->content_unset = _unimplemented_smart_content_unset;
}
