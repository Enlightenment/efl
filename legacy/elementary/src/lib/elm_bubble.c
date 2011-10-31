#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *bbl;
   Evas_Object *content, *icon;
   const char *label, *info, *corner;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _content_set_hook(Evas_Object *obj, const char *part, Evas_Object *content);
static Evas_Object *_content_get_hook(const Evas_Object *obj, const char *part);
static Evas_Object *_content_unset_hook(Evas_Object *obj, const char *part);
static void _theme_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);

static const char SIG_CLICKED[] = "clicked";

static const Evas_Smart_Cb_Description _signals[] =
{
     {SIG_CLICKED, ""},
     {NULL, NULL}
};

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->label) eina_stringshare_del(wd->label);
   if (wd->info) eina_stringshare_del(wd->info);
   if (wd->corner) eina_stringshare_del(wd->corner);
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   edje_object_mirrored_set(wd->bbl, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->bbl, "bubble", wd->corner,
                         elm_widget_style_get(obj));
   edje_object_part_text_set(wd->bbl, "elm.text", wd->label);
   if (wd->label) edje_object_signal_emit(wd->bbl, "elm,state,text,visible", "elm");
   else edje_object_signal_emit(wd->bbl, "elm,state,text,hidden", "elm");
   edje_object_part_text_set(wd->bbl, "elm.info", wd->info);
   if (wd->info) edje_object_signal_emit(wd->bbl, "elm,state,info,visible", "elm");
   else edje_object_signal_emit(wd->bbl, "elm,state,info,hidden", "elm");
   if (wd->content)
     {
        edje_object_part_swallow(wd->bbl, "elm.swallow.content", wd->content);
        edje_object_message_signal_process(wd->bbl);
     }
   if (wd->icon)
     edje_object_signal_emit(wd->bbl, "elm,state,icon,visible", "elm");
   else
     edje_object_signal_emit(wd->bbl, "elm,state,icon,hidden", "elm");
   edje_object_scale_set(wd->bbl,
                         elm_widget_scale_get(obj) * _elm_config->scale);
   _sizing_eval(obj);
}

static void
_content_set_hook(Evas_Object *obj, const char *part __UNUSED__, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->content == content) return;
   if (wd->content) evas_object_del(wd->content);
   wd->content = content;
   if (content)
     {
        elm_widget_sub_object_add(obj, content);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_part_swallow(wd->bbl, "elm.swallow.content", content);
     }
   _sizing_eval(obj);
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *content;
   if (!wd) return NULL;
   if (!wd->content) return NULL;
   content = wd->content;
   elm_widget_sub_object_del(obj, content);
   evas_object_event_callback_del_full(content, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
   edje_object_part_unswallow(wd->bbl, content);
   wd->content = NULL;
   return content;
}

static Eina_Bool
_elm_bubble_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *cur;

   if ((!wd) || (!wd->content))
     return EINA_FALSE;

   cur = wd->content;

   /* Try Focus cycle in subitem */
   return elm_widget_focus_next_get(cur, dir, next);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   edje_object_size_min_restricted_calc(wd->bbl, &minw, &minh, minw, minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   _sizing_eval(data);
}

static void
_sub_del(void *data __UNUSED__, Evas_Object *obj, void *event_info)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *sub = event_info;
   if (!wd) return;
   evas_object_event_callback_del_full(sub, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
   if (sub == wd->content) wd->content = NULL;
   else if (sub == wd->icon)
     {
        edje_object_signal_emit(wd->bbl, "elm,state,icon,hidden", "elm");
        wd->icon = NULL;
        edje_object_message_signal_process(wd->bbl);
     }
   _sizing_eval(obj);
}

static void
_mouse_up(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return;
   evas_object_smart_callback_call(data, SIG_CLICKED, NULL);
}

static void
_elm_bubble_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (!item || !strcmp(item, "default"))
     {
        eina_stringshare_replace(&wd->label, label);
        edje_object_part_text_set(wd->bbl, "elm.text", label);
        if (label) edje_object_signal_emit(wd->bbl, "elm,state,text,visible",
              "elm");
        else edje_object_signal_emit(wd->bbl, "elm,state,text,hidden", "elm");
        _sizing_eval(obj);
     }
   else if (!strcmp(item, "info"))
     {
        eina_stringshare_replace(&wd->info, label);
        edje_object_part_text_set(wd->bbl, "elm.info", label);
        if (label) edje_object_signal_emit(wd->bbl, "elm,state,info,visible",
              "elm");
        else edje_object_signal_emit(wd->bbl, "elm,state,info,hidden", "elm");
        _sizing_eval(obj);
     }
}

static const char*
_elm_bubble_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   if (!item || !strcmp(item, "default"))
     {
        return wd->label;
     }
   else if (!strcmp(item, "info"))
     {
        return wd->info;
     }

   return NULL;
}

EAPI Evas_Object *
elm_bubble_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "bubble");
   elm_widget_type_set(obj, "bubble");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_focus_next_hook_set(obj, _elm_bubble_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_text_set_hook_set(obj, _elm_bubble_label_set);
   elm_widget_text_get_hook_set(obj, _elm_bubble_label_get);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);

   wd->corner = eina_stringshare_add("base");

   wd->bbl = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->bbl);

   evas_object_smart_callback_add(obj, "sub-object-del", _sub_del, obj);
   evas_object_event_callback_add(wd->bbl, EVAS_CALLBACK_MOUSE_UP,
                                  _mouse_up, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   _elm_theme_object_set(obj, wd->bbl, "bubble", wd->corner,
                         elm_widget_style_get(obj));
   _sizing_eval(obj);
   return obj;
}

EAPI void
elm_bubble_label_set(Evas_Object *obj, const char *label)
{
   _elm_bubble_label_set(obj, NULL, label);
}

EAPI const char*
elm_bubble_label_get(const Evas_Object *obj)
{
   return _elm_bubble_label_get(obj, NULL);
}

EAPI void
elm_bubble_info_set(Evas_Object *obj, const char *info)
{
   _elm_bubble_label_set(obj, "info", info);
}

EAPI const char *
elm_bubble_info_get(const Evas_Object *obj)
{
   return _elm_bubble_label_get(obj, "info");
}

EAPI void
elm_bubble_content_set(Evas_Object *obj, Evas_Object *content)
{
   _content_set_hook(obj, NULL, content);
}

EAPI Evas_Object *
elm_bubble_content_get(const Evas_Object *obj)
{
   return _content_get_hook(obj, NULL);
}

EAPI Evas_Object *
elm_bubble_content_unset(Evas_Object *obj)
{
   return _content_unset_hook(obj, NULL);
}

EAPI void
elm_bubble_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->icon == icon) return;
   if (wd->icon) evas_object_del(wd->icon);
   wd->icon = icon;
   if (icon)
     {
        elm_widget_sub_object_add(obj, icon);
        edje_object_part_swallow(wd->bbl, "elm.swallow.icon", icon);
        evas_object_event_callback_add(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
        edje_object_signal_emit(wd->bbl, "elm,state,icon,visible", "elm");
        edje_object_message_signal_process(wd->bbl);
     }
   _sizing_eval(obj);
}

EAPI Evas_Object *
elm_bubble_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->icon;
}

EAPI Evas_Object *
elm_bubble_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *icon;
   if (!wd) return NULL;
   if (!wd->icon) return NULL;
   icon = wd->icon;
   elm_widget_sub_object_del(obj, icon);
   evas_object_event_callback_del_full(icon, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _changed_size_hints, obj);
   edje_object_part_unswallow(wd->bbl, icon);
   wd->icon = NULL;
   return icon;
}

EAPI void
elm_bubble_corner_set(Evas_Object *obj, const char *corner)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_SAFETY_ON_NULL_RETURN(corner);
   eina_stringshare_replace(&wd->corner, corner);
   _theme_hook(obj);
}

EAPI const char*
elm_bubble_corner_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->corner;
}
