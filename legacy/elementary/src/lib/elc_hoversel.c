#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *btn, *hover;
   Evas_Object *hover_parent;
   Eina_List *items;
   Eina_Bool horizontal : 1;
   Eina_Bool expanded   : 1;
};

struct _Elm_Hoversel_Item
{
   ELM_WIDGET_ITEM;
   const char *label;
   const char *icon_file;
   const char *icon_group;
   Elm_Icon_Type icon_type;
   Evas_Smart_Cb func;
};

static const char *widtype = NULL;
static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _activate(Evas_Object *obj);
static void _activate_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _parent_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static const char SIG_CLICKED[] = "clicked";
static const char SIG_SELECTED[] = "selected";
static const char SIG_DISMISSED[] = "dismissed";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_CLICKED, ""},
   {SIG_SELECTED, ""},
   {SIG_DISMISSED, ""},
   {NULL, NULL}
};

static void
_del_pre_hook(Evas_Object *obj)
{
   Elm_Hoversel_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_hoversel_hover_end(obj);
   elm_hoversel_hover_parent_set(obj, NULL);
   EINA_LIST_FREE(wd->items, item)
     {
        elm_widget_item_pre_notify_del(item);
        eina_stringshare_del(item->label);
        eina_stringshare_del(item->icon_file);
        eina_stringshare_del(item->icon_group);
        elm_widget_item_del(item);
     }
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_mirrored_set(wd->btn, rtl);
   elm_widget_mirrored_set(wd->hover, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   char buf[4096];
   if (!wd) return;
   _elm_widget_mirrored_reload(obj);

   elm_hoversel_hover_end(obj);
   if (wd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal/%s", elm_widget_style_get(obj));
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical/%s", elm_widget_style_get(obj));
   elm_object_style_set(wd->btn, buf);
   elm_object_disabled_set(wd->btn, elm_widget_disabled_get(obj));
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
}

static void
_disable_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_object_disabled_set(wd->btn, elm_widget_disabled_get(obj));
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   if (!wd) return;
   evas_object_size_hint_min_get(wd->btn, &minw, &minh);
   evas_object_size_hint_max_get(wd->btn, &maxw, &maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_on_focus_hook(void *data __UNUSED__, Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (elm_widget_focus_get(obj))
     elm_widget_focus_steal(wd->btn);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_hover_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   elm_hoversel_hover_end(data);
}

static void
_item_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_Hoversel_Item *item = data;
   Evas_Object *obj2 = WIDGET(item);

   elm_hoversel_hover_end(obj2);
   if (item->func) item->func((void *)item->base.data, obj2, item);
   evas_object_smart_callback_call(obj2, SIG_SELECTED, item);
}

static void
_activate(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   Evas_Object *bt, *bx, *ic;
   const Eina_List *l;
   const Elm_Hoversel_Item *item;
   char buf[4096];

   if (!wd) return;
   if (wd->expanded)
     {
        elm_hoversel_hover_end(obj);
        return;
     }
   wd->expanded = EINA_TRUE;

   if (elm_widget_disabled_get(obj)) return;
   wd->hover = elm_hover_add(obj);
   elm_widget_mirrored_automatic_set(wd->hover, EINA_FALSE);
   if (wd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal/%s", elm_widget_style_get(obj));
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical/%s", elm_widget_style_get(obj));
   elm_object_style_set(wd->hover, buf);
   evas_object_smart_callback_add(wd->hover, "clicked", _hover_clicked, obj);
   elm_hover_parent_set(wd->hover, wd->hover_parent);
   elm_hover_target_set(wd->hover, wd->btn);

   bx = elm_box_add(wd->hover);
   elm_widget_mirrored_automatic_set(bx, EINA_FALSE);
   elm_box_homogeneous_set(bx, 1);

   elm_box_horizontal_set(bx, wd->horizontal);

   if (wd->horizontal)
     snprintf(buf, sizeof(buf), "hoversel_horizontal_entry/%s",
              elm_widget_style_get(obj));
   else
     snprintf(buf, sizeof(buf), "hoversel_vertical_entry/%s",
              elm_widget_style_get(obj));
   EINA_LIST_FOREACH(wd->items, l, item)
     {
        bt = elm_button_add(wd->hover);
        elm_widget_mirrored_automatic_set(bt, EINA_FALSE);
        elm_widget_mirrored_set(bt, elm_widget_mirrored_get(obj));
        elm_object_style_set(bt, buf);
        elm_object_text_set(bt, item->label);
        if (item->icon_file)
          {
             ic = elm_icon_add(obj);
             elm_icon_scale_set(ic, 0, 1);
             if (item->icon_type == ELM_ICON_FILE)
               elm_icon_file_set(ic, item->icon_file, item->icon_group);
             else if (item->icon_type == ELM_ICON_STANDARD)
               elm_icon_standard_set(ic, item->icon_file);
             elm_object_content_set(bt, ic);
             evas_object_show(ic);
          }
        evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
        evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_box_pack_end(bx, bt);
        evas_object_smart_callback_add(bt, "clicked", _item_clicked, item);
        evas_object_show(bt);
     }

   if (wd->horizontal)
     elm_hover_content_set(wd->hover,
                           elm_hover_best_content_location_get(wd->hover,
                                                               ELM_HOVER_AXIS_HORIZONTAL),
                           bx);
   else
     elm_hover_content_set(wd->hover,
                           elm_hover_best_content_location_get(wd->hover,
                                                               ELM_HOVER_AXIS_VERTICAL),
                           bx);
   evas_object_show(bx);

   evas_object_show(wd->hover);
   evas_object_smart_callback_call(obj, SIG_CLICKED, NULL);

   //   if (wd->horizontal) evas_object_hide(wd->btn);
}

static void
_activate_hook(Evas_Object *obj)
{
   _activate(obj);
}

static void
_button_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   _activate(data);
}

static void
_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);
   if (!wd) return;
   wd->hover_parent = NULL;
}

static void
_elm_hoversel_label_set(Evas_Object *obj, const char *item, const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return;
   if (!wd) return;
   elm_object_text_set(wd->btn, label);
}

static const char *
_elm_hoversel_label_get(const Evas_Object *obj, const char *item)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (item && strcmp(item, "default")) return NULL;
   if ((!wd) || (!wd->btn)) return NULL;
   return elm_object_text_get(wd->btn);
}

EAPI Evas_Object *
elm_hoversel_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "hoversel");
   elm_widget_type_set(obj, "hoversel");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_activate_hook_set(obj, _activate_hook);
   elm_widget_on_focus_hook_set(obj, _on_focus_hook, NULL);
   elm_widget_can_focus_set(obj, EINA_TRUE);
   elm_widget_text_set_hook_set(obj, _elm_hoversel_label_set);
   elm_widget_text_get_hook_set(obj, _elm_hoversel_label_get);

   wd->btn = elm_button_add(parent);
   elm_widget_mirrored_automatic_set(wd->btn, EINA_FALSE);
   wd->expanded = EINA_FALSE;
   elm_widget_resize_object_set(obj, wd->btn);
   evas_object_event_callback_add(wd->btn, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);
   evas_object_smart_callback_add(wd->btn, "clicked", _button_clicked, obj);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   elm_widget_sub_object_add(obj, wd->btn);

   elm_hoversel_hover_parent_set(obj, parent);
   _theme_hook(obj);

   return obj;
}

EAPI void
elm_hoversel_hover_parent_set(Evas_Object *obj, Evas_Object *parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover_parent)
     evas_object_event_callback_del_full(wd->hover_parent, EVAS_CALLBACK_DEL,
                                         _parent_del, obj);
   wd->hover_parent = parent;
   if (wd->hover_parent)
     evas_object_event_callback_add(wd->hover_parent, EVAS_CALLBACK_DEL,
                                    _parent_del, obj);
}

EAPI Evas_Object *
elm_hoversel_hover_parent_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->hover_parent;
}

EAPI void
elm_hoversel_label_set(Evas_Object *obj, const char *label)
{
   _elm_hoversel_label_set(obj, NULL, label);
}

EAPI const char *
elm_hoversel_label_get(const Evas_Object *obj)
{
   return _elm_hoversel_label_get(obj, NULL);
}

EAPI void
elm_hoversel_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->horizontal = !!horizontal;
}

EAPI Eina_Bool
elm_hoversel_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->horizontal;
}

EAPI void
elm_hoversel_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_object_content_set(wd->btn, icon);
}

EAPI Evas_Object *
elm_hoversel_icon_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->btn)) return NULL;
   return elm_object_content_get(wd->btn);
}

EAPI Evas_Object *
elm_hoversel_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->btn)) return NULL;
   return elm_object_content_unset(wd->btn);
}

EAPI void
elm_hoversel_hover_begin(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->hover) return;
   _activate(obj);
}

EAPI void
elm_hoversel_hover_end(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (!wd->hover) return;
   wd->expanded = EINA_FALSE;
   evas_object_del(wd->hover);
   wd->hover = NULL;
   evas_object_smart_callback_call(obj, SIG_DISMISSED, NULL);
}

EAPI Eina_Bool
elm_hoversel_expanded_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return (wd->hover) ? EINA_TRUE : EINA_FALSE;
}

EAPI void
elm_hoversel_clear(Evas_Object *obj)
{
   Elm_Hoversel_Item *item;
   Eina_List *l, *ll;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   EINA_LIST_FOREACH_SAFE(wd->items, l, ll, item) elm_hoversel_item_del(item);
}

EAPI const Eina_List *
elm_hoversel_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->items;
}

EAPI Elm_Hoversel_Item *
elm_hoversel_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Elm_Hoversel_Item *item = elm_widget_item_new(obj, Elm_Hoversel_Item);
   if (!item) return NULL;
   wd->items = eina_list_append(wd->items, item);
   item->label = eina_stringshare_add(label);
   item->icon_file = eina_stringshare_add(icon_file);
   item->icon_type = icon_type;
   item->func = func;
   item->base.data = data;
   return item;
}

EAPI void
elm_hoversel_item_del(Elm_Hoversel_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   Widget_Data *wd = elm_widget_data_get(WIDGET(item));
   if (!wd) return;
   elm_hoversel_hover_end(WIDGET(item));
   wd->items = eina_list_remove(wd->items, item);
   elm_widget_item_pre_notify_del(item);
   eina_stringshare_del(item->label);
   eina_stringshare_del(item->icon_file);
   eina_stringshare_del(item->icon_group);
   elm_widget_item_del(item);
}

EAPI void
elm_hoversel_item_del_cb_set(Elm_Hoversel_Item *item, Evas_Smart_Cb func)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   elm_widget_item_del_cb_set(item, func);
}

EAPI void *
elm_hoversel_item_data_get(const Elm_Hoversel_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   return elm_widget_item_data_get(item);
}

EAPI const char *
elm_hoversel_item_label_get(const Elm_Hoversel_Item *item)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item, NULL);
   return item->label;
}

EAPI void
elm_hoversel_item_icon_set(Elm_Hoversel_Item *item, const char *icon_file, const char *icon_group, Elm_Icon_Type icon_type)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   eina_stringshare_replace(&item->icon_file, icon_file);
   eina_stringshare_replace(&item->icon_group, icon_group);
   item->icon_type = icon_type;
}

EAPI void
elm_hoversel_item_icon_get(const Elm_Hoversel_Item *item, const char **icon_file, const char **icon_group, Elm_Icon_Type *icon_type)
{
   ELM_WIDGET_ITEM_WIDTYPE_CHECK_OR_RETURN(item);
   if (icon_file) *icon_file = item->icon_file;
   if (icon_group) *icon_group = item->icon_group;
   if (icon_type) *icon_type = item->icon_type;
}

