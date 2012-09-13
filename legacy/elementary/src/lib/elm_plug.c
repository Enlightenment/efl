#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_plug.h"

EAPI const char ELM_PLUG_SMART_NAME[] = "elm_plug";

static const char PLUG_KEY[] = "__Plug_Ecore_Evas";

static const char SIG_CLICKED[] = "clicked";
static const char SIG_IMAGE_DELETED[] = "image.deleted";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_IMAGE_DELETED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_PLUG_SMART_NAME, _elm_plug, Elm_Plug_Smart_Class,
  Elm_Widget_Smart_Class, elm_widget_smart_class_get, _smart_callbacks);

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   //TODO: get socket object size
   //this reset plug's min/max size
   //evas_object_size_hint_min_set(obj, minw, minh);
   //evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_elm_plug_disconnected(Ecore_Evas *ee)
{
   Evas_Object *plug = NULL;

   if (!ee) return;
   plug = ecore_evas_data_get(ee, PLUG_KEY);
   if (!plug) return;
   evas_object_smart_callback_call(plug, SIG_IMAGE_DELETED, NULL);
}

static Eina_Bool
_elm_plug_smart_theme(Evas_Object *obj)
{
   if (!_elm_plug_parent_sc->theme(obj)) return EINA_FALSE;

   _sizing_eval(obj);

   return EINA_TRUE;
}

static void
_on_mouse_up(void *data,
             Evas *e __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_elm_plug_smart_add(Evas_Object *obj)
{
   Evas_Object *p_obj;
   Ecore_Evas *ee;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Plug_Smart_Data);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   _elm_plug_parent_sc->base.add(obj);

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   if (!ee) return;

   p_obj = ecore_evas_extn_plug_new(ee);
   if (!p_obj) return;

   elm_widget_resize_object_set(obj, p_obj);

   evas_object_event_callback_add
     (ELM_WIDGET_DATA(priv)->resize_obj, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up,
     obj);

   _sizing_eval(obj);
}

static void
_elm_plug_smart_set_user(Elm_Plug_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_plug_smart_add;

   ELM_WIDGET_CLASS(sc)->theme = _elm_plug_smart_theme;
}

EAPI const Elm_Plug_Smart_Class *
elm_plug_smart_class_get(void)
{
   static Elm_Plug_Smart_Class _sc =
     ELM_PLUG_SMART_CLASS_INIT_NAME_VERSION(ELM_PLUG_SMART_NAME);
   static const Elm_Plug_Smart_Class *class = NULL;

   if (class) return class;

   _elm_plug_smart_set(&_sc);
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_plug_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_plug_smart_class_new(), parent);
   if (!obj) return NULL;

   ELM_PLUG_DATA_GET(obj, sd);
   if (!ELM_WIDGET_DATA(sd)->resize_obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Evas_Object *
elm_plug_image_object_get(const Evas_Object *obj)
{
   ELM_PLUG_CHECK(obj) NULL;
   ELM_PLUG_DATA_GET(obj, sd);

   return ELM_WIDGET_DATA(sd)->resize_obj;
}

EAPI Eina_Bool
elm_plug_connect(Evas_Object *obj,
                 const char *svcname,
                 int svcnum,
                 Eina_Bool svcsys)
{
   Evas_Object *plug_img = NULL;

   ELM_PLUG_CHECK(obj) EINA_FALSE;

   plug_img = elm_plug_image_object_get(obj);
   if (!plug_img) return EINA_FALSE;

   if (ecore_evas_extn_plug_connect(plug_img, svcname, svcnum, svcsys))
     {
        Ecore_Evas *ee = NULL;
        ee = ecore_evas_object_ecore_evas_get(plug_img);
        if (!ee) return EINA_FALSE;

        ecore_evas_data_set(ee, PLUG_KEY, obj);
        ecore_evas_callback_delete_request_set(ee, _elm_plug_disconnected);
        return EINA_TRUE;
     }
   else
     return EINA_FALSE;
}
