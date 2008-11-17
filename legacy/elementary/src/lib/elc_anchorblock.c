#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *entry;
   Evas_Object *hover_parent;
   Evas_Object *pop, *hover;
   const char *hover_style;
};

static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_del_pre_hook(Evas_Object *obj)
{
   elm_anchorblock_hover_end(obj);
   elm_anchorblock_hover_parent_set(obj, NULL);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover_style) eina_stringshare_del(wd->hover_style);
   free(wd);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   evas_object_size_hint_min_get(wd->entry, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   _sizing_eval(data);
}

static void
_hover_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->hover = NULL;
}

static void
_hover_clicked(void *data, Evas_Object *obj, void *event_info)
{
   elm_anchorblock_hover_end(data);
}

static void
_anchor_clicked(void *data, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   Elm_Entry_Anchor_Info *info = event_info;
   Evas_Object *hover_parent;
   Elm_Entry_Anchorblock_Info ei;
   Evas_Coord x, w, y, h, px, py;
   
   wd->pop = elm_icon_add(obj);
   evas_object_move(wd->pop, info->x, info->y);
   evas_object_resize(wd->pop, info->w, info->h);
   
   wd->hover = elm_hover_add(obj);
   if (wd->hover_style) elm_hover_style_set(wd->hover, wd->hover_style);
   hover_parent = wd->hover_parent;
   if (!hover_parent) hover_parent = obj;
   elm_hover_parent_set(wd->hover, hover_parent);
   elm_hover_target_set(wd->hover, wd->pop);

   ei.name = info->name;
   ei.button = info->button;
   ei.hover = wd->hover;
   ei.anchor.x = info->x;
   ei.anchor.y = info->y;
   ei.anchor.w = info->w;
   ei.anchor.h = info->h;
   evas_object_geometry_get(hover_parent, &x, &y, &w, &h);
   ei.hover_parent.x = x;
   ei.hover_parent.y = y;
   ei.hover_parent.w = w;
   ei.hover_parent.h = h;
   px = info->x + (info->w / 2);
   py = info->y + (info->h / 2);
   ei.hover_left = 1;
   if (px < (x + (w / 3))) ei.hover_left = 0; 
   ei.hover_right = 1;
   if (px > (x + ((w * 2) / 3))) ei.hover_right = 0; 
   ei.hover_top = 1;
   if (py < (y + (h / 3))) ei.hover_top = 0; 
   ei.hover_bottom = 1;
   if (py > (y + ((h * 2) / 3))) ei.hover_bottom = 0; 
   evas_object_smart_callback_call(data, "anchor,clicked", &ei);
   evas_object_smart_callback_add(wd->hover, "clicked", _hover_clicked, data);
   evas_object_show(wd->hover);
}

static void
_parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(data);
   wd->hover_parent = NULL;
}

EAPI Evas_Object *
elm_anchorblock_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   
   wd = ELM_NEW(Widget_Data);
   e = evas_object_evas_get(parent);
   obj = elm_widget_add(e);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   
   wd->entry = elm_entry_add(parent);
   elm_widget_resize_object_set(obj, wd->entry);
   elm_entry_editable_set(wd->entry, 0);
   evas_object_size_hint_weight_set(wd->entry, 1.0, 1.0);
   evas_object_size_hint_align_set(wd->entry, -1.0, -1.0);
   
   evas_object_event_callback_add(wd->entry, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _changed_size_hints, obj);
   
   elm_entry_entry_set(wd->entry, "");
   
   evas_object_smart_callback_add(wd->entry, "anchor,clicked", _anchor_clicked, obj);
   
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_anchorblock_text_set(Evas_Object *obj, const char *text)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   elm_entry_entry_set(wd->entry, text);
   if (wd->hover) evas_object_del(wd->hover);
   if (wd->pop) evas_object_del(wd->pop);
   wd->hover = NULL;
   wd->pop = NULL;
   _sizing_eval(obj);
}

EAPI void
elm_anchorblock_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover_parent)
     evas_object_event_callback_del(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del);
   wd->hover_parent = parent;
   if (wd->hover_parent)
     evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL, _parent_del, obj);
}

EAPI void
elm_anchorblock_hover_style_set(Evas_Object *obj, const char *style)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover_style) eina_stringshare_del(wd->hover_style);
   wd->hover_style = NULL;
   if (style) wd->hover_style = eina_stringshare_add(style);
}  

EAPI void
elm_anchorblock_hover_end(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (wd->hover) evas_object_del(wd->hover);
   if (wd->pop) evas_object_del(wd->pop);
   wd->hover = NULL;
   wd->pop = NULL;
}
