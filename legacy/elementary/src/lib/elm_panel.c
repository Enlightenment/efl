#include <Elementary.h>
#include "elm_priv.h"

#define PNL_BTN_WIDTH 32

/**
 * @defgroup Panel Panel
 * 
 * This is a panel object
 */

typedef struct _Widget_Data Widget_Data;
struct _Widget_Data 
{
   Evas_Object *parent, *panel, *content;
   Elm_Panel_Orient orient;
   Eina_Bool hidden : 1;
};

static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _parent_resize(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _toggle_panel(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _changed_size_hints(void *data, Evas *evas, Evas_Object *obj, void *event);
static void _sub_del(void *data, Evas_Object *obj, void *event);

static void 
_del_pre_hook(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_RESIZE, 
                                       _parent_resize, obj);
}

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
   if (wd->content)
     edje_object_part_swallow(wd->panel, "elm.swallow.content", wd->content);
   edje_object_scale_set(wd->panel, elm_widget_scale_get(obj) * 
                         _elm_config->scale);

   edje_object_signal_emit(wd->panel, "elm,action,show", "elm");
   edje_object_message_signal_process(wd->panel);
   _sizing_eval(obj);
}

static void 
_sizing_eval(Evas_Object *obj) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord x, y, w, h;
   Evas_Coord pw, ph;

   evas_object_geometry_get(wd->parent, &x, &y, &w, &h);
   edje_object_size_min_calc(wd->panel, &pw, &ph);
   printf("Panel Min Size: %d %d\n", pw, ph);
//   if (pw < 64) pw = 64;
   switch (wd->orient) 
     {
      case ELM_PANEL_ORIENT_TOP:
        evas_object_resize(wd->panel, w, pw + PNL_BTN_WIDTH);
        break;
      case ELM_PANEL_ORIENT_BOTTOM:
        evas_object_resize(wd->panel, w, pw + PNL_BTN_WIDTH);
        break;
      case ELM_PANEL_ORIENT_LEFT:
        evas_object_resize(wd->panel, pw + PNL_BTN_WIDTH, h);
        break;
      case ELM_PANEL_ORIENT_RIGHT:
        evas_object_resize(wd->panel, pw + PNL_BTN_WIDTH, h);
        break;
     }
}

static void 
_parent_resize(void *data, Evas *evas, Evas_Object *obj, void *event) 
{
   _sizing_eval(data);
}

static void 
_toggle_panel(void *data, Evas_Object *obj, const char *emission, const char *source) 
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->hidden) 
     {
        edje_object_signal_emit(wd->panel, "elm,action,show", "elm");
        wd->hidden = EINA_FALSE;
     }
   else 
     {
        edje_object_signal_emit(wd->panel, "elm,action,hide", "elm");
        wd->hidden = EINA_TRUE;
     }
}

static void 
_changed_size_hints(void *data, Evas *evas, Evas_Object *obj, void *event) 
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->content)
     edje_object_part_swallow(wd->panel, "elm.swallow.content", wd->content);
   _sizing_eval(data);
}

static void 
_sub_del(void *data, Evas_Object *obj, void *event) 
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event;

   if (sub == wd->content) 
     {
        evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS, 
                                            _changed_size_hints, obj);
        wd->content = NULL;
        _sizing_eval(obj);
     }
}

/**
 * Add a new panel to the parent
 * 
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 * 
 * @ingroup Panel
 */
EAPI Evas_Object *
elm_panel_add(Evas_Object *parent) 
{
   Evas_Object *obj;
   Evas *evas;
   Widget_Data *wd;

   wd = ELM_NEW(Widget_Data);
   wd->parent = parent;
   wd->hidden = EINA_FALSE;
   evas = evas_object_evas_get(parent);

   obj = elm_widget_add(evas);
   elm_widget_type_set(obj, "panel");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->panel = edje_object_add(evas);
   elm_panel_orient_set(obj, ELM_PANEL_ORIENT_LEFT);
   elm_widget_resize_object_set(obj, wd->panel);

   edje_object_signal_callback_add(wd->panel, "elm,action,panel,toggle", 
                                   "*", _toggle_panel, obj);
   evas_object_event_callback_add(wd->parent, EVAS_CALLBACK_RESIZE, 
                                  _parent_resize, obj);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);

   _sizing_eval(obj);
   return obj;
}

/**
 * Set the panel orientation
 * 
 * @param obj The panel object
 * @param orient The orientation to set for this panel object
 * 
 * @ingroup Panel
 */
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

/**
 * Set the panel content
 * 
 * @param obj The panel object
 * @param content The content will be filled in this panel object
 * 
 * @ingroup Panel
 */
EAPI void 
elm_panel_content_set(Evas_Object *obj, Evas_Object *content) 
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((wd->content != content) && (wd->content))
     elm_widget_sub_object_del(obj, wd->content);
   wd->content = content;
   if (content) 
     {
        elm_widget_sub_object_add(obj, content);
        edje_object_part_swallow(wd->panel, "elm.swallow.content", content);
        evas_object_event_callback_add(content, 
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS, 
                                       _changed_size_hints, obj);
        _sizing_eval(obj);
     }
}
