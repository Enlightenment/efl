#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *img;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_CLICKED, ""},
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
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   evas_object_del(wd->img);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   _sizing_eval(obj);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   if (!wd) return;
   //TODO: get socket object size
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}


EAPI Evas_Object *
elm_plug_image_object_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   if (!wd->img) return NULL;
   return wd->img;
}

EAPI Evas_Object *
elm_plug_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Ecore_Evas *ee;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "plug");
   elm_widget_type_set(obj, "plug");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
   if (!ee) return NULL;
   wd->img = ecore_evas_extn_plug_new(ee);
   if (!wd->img) return NULL;

   evas_object_event_callback_add(wd->img, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);
   elm_widget_resize_object_set(obj, wd->img);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   _sizing_eval(obj);
   return obj;
}

EAPI Eina_Bool
elm_plug_connect(Evas_Object *obj, const char *svcname, int svcnum, Eina_Bool svcsys)
{
   Evas_Object *plug_img = NULL;

   plug_img = elm_plug_image_object_get(obj);
   if (!plug_img) return EINA_FALSE;

   if (ecore_evas_extn_plug_connect(plug_img, svcname, svcnum, svcsys))
     return EINA_TRUE;
   else
     return EINA_FALSE;
}

