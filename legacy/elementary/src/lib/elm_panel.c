#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Panel Panel
 * 
 * This is a panel object
 */

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data 
{
   Evas_Object *parent, *panel, *btn, *icn;
   Elm_Panel_Orient orient;
   int timeout;
   Ecore_Timer *timer;
};

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _parent_resize(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _btn_click(void *data, Evas_Object *obj, void *event);

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

   _elm_theme_set(wd->panel, "panel", "base", "default");
   edje_object_scale_set(wd->panel, elm_widget_scale_get(obj) * 
                         _elm_config->scale);
   _sizing_eval(obj);
}

static void 
_sizing_eval(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x, y, w, h;

   evas_object_geometry_get(wd->parent, &x, &y, &w, &h);
   switch (wd->orient) 
     {
      case ELM_PANEL_ORIENT_TOP:
        evas_object_move(wd->panel, x, y);
        evas_object_resize(wd->panel, w, 48);
        break;
      case ELM_PANEL_ORIENT_BOTTOM:
        evas_object_move(wd->panel, x, h - 48);
        evas_object_resize(wd->panel, w, 48);
        break;
      case ELM_PANEL_ORIENT_LEFT:
        evas_object_move(wd->panel, x, y);
        evas_object_resize(wd->panel, 48, h);
        break;
      case ELM_PANEL_ORIENT_RIGHT:
        evas_object_move(wd->panel, (x + w - 48), y);
        evas_object_resize(wd->panel, 48, h);
        break;
     }
}

static void 
_parent_resize(void *data, Evas *evas, Evas_Object *obj, void *event) 
{
   _sizing_eval(data);
}

static void 
_btn_click(void *data, Evas_Object *obj, void *event) 
{
   Widget_Data *wd = elm_widget_data_get(data);

   elm_icon_standard_set(wd->icn, "arrow_right");
}

EAPI Evas_Object *
elm_panel_add(Evas_Object *parent) 
{
   Evas_Object *obj;
   Evas *evas;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   wd->parent = parent;
   evas = evas_object_evas_get(parent);

   obj = elm_widget_add(evas);
   elm_widget_type_set(obj, "panel");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->panel = edje_object_add(evas);
   elm_panel_orient_set(obj, ELM_PANEL_ORIENT_LEFT);
   elm_widget_resize_object_set(obj, wd->panel);

   wd->icn = elm_icon_add(obj);
   elm_icon_standard_set(wd->icn, "arrow_left");
   evas_object_size_hint_aspect_set(wd->icn, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   elm_widget_sub_object_add(obj, wd->icn);

   wd->btn = elm_button_add(obj);
   elm_button_icon_set(wd->btn, wd->icn);
   elm_object_style_set(wd->btn, "anchor");
   evas_object_size_hint_align_set(wd->btn, 0.0, 0.0);
   elm_widget_sub_object_add(obj, wd->btn);
   evas_object_smart_callback_add(wd->btn, "clicked", _btn_click, obj);
   edje_object_part_swallow(wd->panel, "elm.swallow.arrow", wd->btn);

   evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_RESIZE, 
                                  _parent_resize, obj);

   return obj;
}

EAPI void 
elm_panel_orient_set(Evas_Object *obj, Elm_Panel_Orient orient) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   wd->orient = orient;
   switch (orient) 
     {
      case ELM_PANEL_ORIENT_TOP:
        _elm_theme_set(wd->panel, "panel", "base", "top");
        break;
      case ELM_PANEL_ORIENT_BOTTOM:
        _elm_theme_set(wd->panel, "panel", "base", "bottom");
        break;
      case ELM_PANEL_ORIENT_LEFT:
        _elm_theme_set(wd->panel, "panel", "base", "left");
        break;
      case ELM_PANEL_ORIENT_RIGHT:
        _elm_theme_set(wd->panel, "panel", "base", "right");
        break;
     }
   _sizing_eval(obj);
}

EAPI void 
elm_panel_timeout_set(Evas_Object *obj, int timeout) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   wd->timeout = timeout;
//   _timer_init(obj);
}
