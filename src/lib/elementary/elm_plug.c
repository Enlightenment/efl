#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_plug.h"

#define MY_CLASS ELM_PLUG_CLASS

#define MY_CLASS_NAME "Elm_Plug"
#define MY_CLASS_NAME_LEGACY "elm_plug"

static const char PLUG_KEY[] = "__Plug_Ecore_Evas";

static const char SIG_CLICKED[] = "clicked";
static const char SIG_IMAGE_DELETED[] = "image,deleted";
static const char SIG_IMAGE_RESIZED[] = "image,resized";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CLICKED, ""},
   {SIG_IMAGE_DELETED, ""},
   {SIG_IMAGE_RESIZED, "ii"},
   {NULL, NULL}
};

static void
_sizing_eval(Evas_Object *obj EINA_UNUSED)
{
   //Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   //TODO: get socket object size
   //this reset plug's min/max size
   //evas_object_size_hint_min_set(obj, minw, minh);
   //evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_elm_plug_disconnected(Ecore_Evas *ee)
{
   Evas_Object *plug = ecore_evas_data_get(ee, PLUG_KEY);
   EINA_SAFETY_ON_NULL_RETURN(plug);
   efl_event_callback_legacy_call(plug, ELM_PLUG_EVENT_IMAGE_DELETED, NULL);
   /* TODO: was a typo. Deprecated, remove in future releases: */
   evas_object_smart_callback_call(plug, "image.deleted", NULL);
}

static void
_elm_plug_resized(Ecore_Evas *ee)
{
   Evas_Coord_Size size = {0, 0};
   Evas_Object *plug = ecore_evas_data_get(ee, PLUG_KEY);
   EINA_SAFETY_ON_NULL_RETURN(plug);

   ecore_evas_geometry_get(ee, NULL, NULL, &(size.w), &(size.h));
   efl_event_callback_legacy_call(plug, ELM_PLUG_EVENT_IMAGE_RESIZED, &size);
}

EOLIAN static Eina_Bool
_elm_plug_efl_ui_focus_object_on_focus_update(Eo *obj, void *sd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (efl_ui_focus_object_focus_get(obj))
     {
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
     }
   else
     {
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Error
_elm_plug_efl_ui_widget_theme_apply(Eo *obj, void *sd EINA_UNUSED)
{
   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;
   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   _sizing_eval(obj);

   return int_ret;
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;

   if (ev->button != 1) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   evas_object_smart_callback_call(data, "clicked", NULL);
}

EOLIAN static void
_elm_plug_efl_canvas_group_group_add(Eo *obj, void *sd EINA_UNUSED)
{
   Evas_Object *p_obj;
   Ecore_Evas *ee;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   if (!ee) return;

   p_obj = ecore_evas_extn_plug_new(ee);
   if (!p_obj) return;

   elm_widget_resize_object_set(obj, p_obj);

   evas_object_event_callback_add
     (wd->resize_obj, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up,
     obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_plug_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_plug_efl_object_constructor(Eo *obj, void *sd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_IMAGE);

   return obj;
}

EOLIAN static Evas_Object*
_elm_plug_image_object_get(const Eo *obj, void *sd EINA_UNUSED)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, NULL);
   return wd->resize_obj;
}

EOLIAN static Eina_Bool
_elm_plug_connect(Eo *obj, void *sd EINA_UNUSED, const char *svcname, int svcnum, Eina_Bool svcsys)
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
        ecore_evas_callback_resize_set(ee, _elm_plug_resized);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static void
_elm_plug_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Internal EO APIs and hidden overrides */

#define ELM_PLUG_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(elm_plug)

#include "elm_plug_eo.c"
