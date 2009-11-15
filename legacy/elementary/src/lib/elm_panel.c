#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data 
{
   Evas_Object *scr, *bx, *content;
   Elm_Panel_Orient orient;
   Eina_Bool hidden : 1;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _resize(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _toggle(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _layout(Evas_Object *obj, Evas_Object_Box_Data *priv, void *data);

static void 
_del_hook(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   free(wd);
}

static void 
_theme_hook(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   edje_object_scale_set(wd->scr, elm_widget_scale_get(obj) * 
                         _elm_config->scale);
   _sizing_eval(obj);
}

static void 
_sizing_eval(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord mw = -1, mh = -1, vw = 0, vh = 0, w, h;

   evas_object_smart_calculate(wd->bx);
   edje_object_size_min_calc(elm_smart_scroller_edje_object_get(wd->scr), 
                             &mw, &mh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < mw) w = mw;
   if (h < mh) h = mh;
   evas_object_resize(wd->scr, w, h);

   evas_object_size_hint_min_get(wd->bx, &mw, &mh);
   if (w > mw) mw = w;
   if (h > mh) mh = h;
   evas_object_resize(wd->bx, mw, mh);

   elm_smart_scroller_child_viewport_size_get(wd->scr, &vw, &vh);
   mw = mw + (w - vw);
   mh = mh + (h - vh);
   evas_object_size_hint_min_set(obj, mw, mh);
   evas_object_size_hint_max_set(obj, -1, ,-1);
}

static void 
_resize(void *data, Evas *evas, Evas_Object *obj, void *event) 
{
   Widget_Data *wd = elm_widget_data_get(data);
}

static void 
_toggle(void *data, Evas_Object *obj, const char *emission, const char *source) 
{

}

static void 
_layout(Evas_Object *obj, Evas_Object_Box_Data *priv, void *data) 
{

}
