#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_plug.h"

EAPI Eo_Op ELM_OBJ_PLUG_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_PLUG_CLASS

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
   evas_object_smart_callback_call(plug, SIG_IMAGE_DELETED, NULL);
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
   evas_object_smart_callback_call(plug, SIG_IMAGE_RESIZED, &size);
}

static void
_elm_plug_smart_on_focus(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret = EINA_FALSE;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (ret) *ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_on_focus(&int_ret));
   if (!int_ret) return;

   if (elm_widget_focus_get(obj))
     {
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
     }
   else
     {
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_plug_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   _sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
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

   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_elm_plug_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   Evas_Object *p_obj;
   Ecore_Evas *ee;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());
   elm_widget_sub_object_parent_add(obj);

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   if (!ee) return;

   p_obj = ecore_evas_extn_plug_new(ee);
   if (!p_obj) return;

   elm_widget_resize_object_set(obj, p_obj, EINA_TRUE);

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

EAPI Evas_Object *
elm_plug_image_object_get(const Evas_Object *obj)
{
   ELM_PLUG_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_plug_image_object_get(&ret));
   return ret;
}

static void
_image_object_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   *ret = wd->resize_obj;
}

EAPI Eina_Bool
elm_plug_connect(Evas_Object *obj,
                 const char *svcname,
                 int svcnum,
                 Eina_Bool svcsys)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_plug_connect(svcname, svcnum, svcsys, &ret));
   return ret;
}

static void
_connect(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *svcname = va_arg(*list, const char *);
   int svcnum = va_arg(*list, int);
   Eina_Bool svcsys = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Object *plug_img = NULL;

   ELM_PLUG_CHECK(obj);

   plug_img = elm_plug_image_object_get(obj);
   if (!plug_img) return;

   if (ecore_evas_extn_plug_connect(plug_img, svcname, svcnum, svcsys))
     {
        Ecore_Evas *ee = NULL;
        ee = ecore_evas_object_ecore_evas_get(plug_img);
        if (!ee) return;

        ecore_evas_data_set(ee, PLUG_KEY, obj);
        ecore_evas_callback_delete_request_set(ee, _elm_plug_disconnected);
        ecore_evas_callback_resize_set(ee, _elm_plug_resized);
        if (ret) *ret = EINA_TRUE;
     }
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_plug_smart_add),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_plug_smart_theme),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ON_FOCUS),
                   _elm_plug_smart_on_focus),

        EO_OP_FUNC(ELM_OBJ_PLUG_ID(ELM_OBJ_PLUG_SUB_ID_IMAGE_OBJECT_GET), _image_object_get),
        EO_OP_FUNC(ELM_OBJ_PLUG_ID(ELM_OBJ_PLUG_SUB_ID_CONNECT), _connect),

        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_PLUG_SUB_ID_IMAGE_OBJECT_GET, "Get the basic Evas_Image object from this object (widget)."),
     EO_OP_DESCRIPTION(ELM_OBJ_PLUG_SUB_ID_CONNECT, "Connect a plug widget to service provided by socket image."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_PLUG_BASE_ID, op_desc, ELM_OBJ_PLUG_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_plug_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, NULL);
