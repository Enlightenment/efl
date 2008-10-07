#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *scr;
   Evas_Object *child;
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
   Evas_Coord  vw, vh, minw, minh, maxw, maxh;
   double xw, xy;
   
   evas_object_size_hint_min_get(wd->child, &minw, &minh);
   evas_object_size_hint_max_get(wd->child, &maxw, &maxh);
   evas_object_size_hint_weight_get(wd->child, &xw, &xy);
   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   if (xw > 0.0)
     {
	if ((minw > 0) && (vw < minw)) vw = minw;
	else if ((maxw > 0) && (vw > maxw)) vw = maxw;
     }
   else if (minw > 0) vw = minw;
   if (xy > 0.0)
     {
	if ((minh > 0) && (vh < minh)) vh = minh;
	else if ((maxh > 0) && (vh > maxh)) vh = maxh;
     }
   else if (minh > 0) vh = minh;
   evas_object_resize(wd->child, vw, vh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

EAPI Evas_Object *
elm_scroller_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord vw, vh, minw, minh;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->scr = elm_smart_scroller_add(e);
   elm_widget_resize_object_set(obj, wd->scr);
   
   edje_object_size_min_calc(wd->scr, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_scroller_child_set(Evas_Object *obj, Evas_Object *child)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   wd->child = child;
   elm_smart_scroller_child_set(wd->scr, child);
   evas_object_event_callback_add(child, EVAS_CALLBACK_CHANGED_SIZE_HINTS, 
				  _changed_size_hints, obj);
   elm_widget_sub_object_add(obj, child);
   // FIXME: track new sub obj...
   _sizing_eval(obj);
}
