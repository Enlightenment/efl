#include <Elementary.h>
#include "elm_priv.h"

static void _elm_icon_file_set(Elm_Icon *icon, const char *file, const char *group);
static void _elm_icon_layout_update(Elm_Icon *icon);

Elm_Icon_Class _elm_icon_class =
{
   &_elm_widget_class,
     ELM_OBJ_ICON,
     _elm_icon_file_set,
     _elm_icon_layout_update
};

static void
_elm_icon_file_set(Elm_Icon *icon, const char *file, const char *group)
{
   const char *p;
   
   if (!file) return;
   if (icon->file) evas_stringshare_del(icon->file);
   if (file) icon->file = evas_stringshare_add(file);
   else icon->file = NULL;
   if (icon->group) evas_stringshare_del(icon->group);
   if (group) icon->group = evas_stringshare_add(group);
   else icon->group = NULL;
   if (((p = strrchr(file, '.'))) && (!strcasecmp(p, ".edj")))
     _els_smart_icon_file_edje_set(icon->base, file, group);
   else 
     _els_smart_icon_file_key_set(icon->base, file, group);
   icon->layout_update(icon);
}

static void
_elm_icon_layout_update(Elm_Icon *icon)
{
   _els_smart_icon_scale_up_set(icon->base, icon->scale_up);
   _els_smart_icon_scale_down_set(icon->base, icon->scale_down);
   _els_smart_icon_smooth_scale_set(icon->base, icon->smooth);
   _els_smart_icon_fill_inside_set(icon->base, !(icon->fill_outside));
   if (icon->no_scale) _els_smart_icon_scale_set(icon->base, 1.0);
   else _els_smart_icon_scale_set(icon->base, _elm_config->scale);
   if ((!icon->scale_down) || (!icon->scale_up))
     ((Elm_Widget *)(icon->parent))->size_req(icon->parent, icon, 0, 0);
}

static void
_elm_icon_size_alloc(Elm_Icon *icon, int w, int h)
{
   int tw, th;
   
   _els_smart_icon_size_get(icon->base, &tw, &th);
   if (!icon->scale_down)
     {
	if (w < tw) w = tw;
	if (h < th) h = th;
     }
   if (!icon->scale_up)
     {
	if (w > tw) w = tw;
	if (h > th) h = th;
     }
   icon->req.w = w;
   icon->req.h = h;
}

static void
_elm_icon_del(Elm_Icon *icon)
{
   if (icon->group) evas_stringshare_del(icon->group);
   if (icon->file) evas_stringshare_del(icon->file);
   ((Elm_Obj_Class *)(((Elm_Icon_Class *)(icon->clas))->parent))->del(ELM_OBJ(icon));
}

EAPI Elm_Icon *
elm_icon_new(Elm_Win *win)
{
   Elm_Icon *icon;
   
   icon = ELM_NEW(Elm_Icon);
   
   _elm_widget_init(icon);
   icon->clas = &_elm_icon_class;
   icon->type = ELM_OBJ_ICON;

   icon->del = _elm_icon_del;
   
   icon->size_alloc = _elm_icon_size_alloc;
   
   icon->file_set = _elm_icon_file_set;
   icon->layout_update = _elm_icon_layout_update;

   icon->smooth = 1;
   icon->scale_up = 1;
   icon->scale_down = 1;
   
   icon->base = _els_smart_icon_add(win->evas);
   evas_object_repeat_events_set(icon->base, 1);
   
   _elm_widget_post_init(icon);
   win->child_add(win, icon);
   return icon;
}
