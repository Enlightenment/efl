#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *tbl;
};

static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);

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
   
   evas_object_size_hint_min_get(wd->tbl, &minw, &minh);
   evas_object_size_hint_max_get(wd->tbl, &maxw, &maxh);
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

EAPI Evas_Object *
elm_table_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->tbl = _els_smart_table_add(e);
   elm_widget_sub_object_add(obj, wd->tbl);
   evas_object_event_callback_add(wd->tbl, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   elm_widget_resize_object_set(obj, wd->tbl);
   return obj;
}

EAPI void
elm_table_homogenous_set(Evas_Object *obj, Evas_Bool homogenous)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _els_smart_table_homogenous_set(wd->tbl, homogenous);
}

EAPI void
elm_table_pack(Evas_Object *obj, Evas_Object *subobj, int x, int y, int w, int h)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   _els_smart_table_pack(wd->tbl, subobj, x, y, w, h);
   elm_widget_sub_object_add(obj, subobj);
   // FIXME: track new sub obj...
}
