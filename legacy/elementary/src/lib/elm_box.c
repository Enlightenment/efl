#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *box;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Evas_Coord w, h;

   evas_object_size_hint_min_get(wd->box, &minw, &minh);
   evas_object_size_hint_max_get(wd->box, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;
   evas_object_resize(obj, w, h);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   evas_object_event_callback_del
     (sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints);
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_box_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->box = _els_smart_box_add(e);
   elm_widget_sub_object_add(obj, wd->box);
   evas_object_event_callback_add(wd->box, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   elm_widget_resize_object_set(obj, wd->box);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   
   return obj;
}

EAPI void
elm_box_horizontal_set(Evas_Object *obj, Evas_Bool horizontal)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _els_smart_box_orientation_set(wd->box, horizontal);
}

EAPI void
elm_box_homogenous_set(Evas_Object *obj, Evas_Bool homogenous)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _els_smart_box_homogenous_set(wd->box, homogenous);
}

EAPI void
elm_box_pack_start(Evas_Object *obj, Evas_Object *subobj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   _els_smart_box_pack_start(wd->box, subobj);
}

EAPI void
elm_box_pack_end(Evas_Object *obj, Evas_Object *subobj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   _els_smart_box_pack_end(wd->box, subobj);
}

EAPI void
elm_box_pack_before(Evas_Object *obj, Evas_Object *subobj, Evas_Object *before)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   _els_smart_box_pack_before(wd->box, subobj, before);
}

EAPI void
elm_box_pack_after(Evas_Object *obj, Evas_Object *subobj, Evas_Object *after)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_widget_sub_object_add(obj, subobj);
   _els_smart_box_pack_after(wd->box, subobj, after);
}
