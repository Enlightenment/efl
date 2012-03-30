#include <Elementary.h>
#include "elm_priv.h"

#define ELM_POPUP_ACTION_BUTTON_MAX 3
typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Popup_Content_Item Elm_Popup_Content_Item;
typedef struct _Action_Area_Data Action_Area_Data;

struct _Widget_Data
{
   Evas_Object *base;
   Evas_Object *notify;
   Evas_Object *title_icon;
   Evas_Object *content_area;
   Evas_Object *content_text_obj;
   Evas_Object *action_area;
   Evas_Object *box;
   Evas_Object *scr;
   Evas_Object *content;
   Eina_List *items;
   const char *title_text;
   Action_Area_Data *buttons[ELM_POPUP_ACTION_BUTTON_MAX];
   Elm_Wrap_Type content_text_wrap_type;
   unsigned int button_count;
   Evas_Coord max_sc_w;
   Evas_Coord max_sc_h;
   Eina_Bool scr_size_recalc:1;
};

struct _Elm_Popup_Content_Item
{
   Elm_Widget_Item base;
   const char *label;
   Evas_Object *icon;
   Evas_Smart_Cb func;
   Eina_Bool disabled:1;
};

struct _Action_Area_Data
{
   Evas_Object *obj;
   Evas_Object *btn;
   Eina_Bool delete_me;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _text_set_hook(Evas_Object *obj, const char *part,
                                          const char *label);
static const char *_text_get_hook(const Evas_Object *obj,
                                                 const char *part);
static void _content_set_hook(Evas_Object *obj, const char *part,
                                             Evas_Object *content);
static Evas_Object *_content_get_hook(Evas_Object *obj,
                                                     const char *part);
static Evas_Object *_content_unset_hook(Evas_Object *obj,
                                                       const char *part);
static void _item_text_set_hook(Elm_Object_Item *it, const char *part,
                                const char *label);
static const char *_item_text_get_hook(const Elm_Object_Item *it,
                                       const char *part);
static void _item_content_set_hook(Elm_Object_Item *it, const char *part,
                                   Evas_Object *content);
static Evas_Object *_item_content_unset_hook(const Elm_Object_Item *it,
                                             const char *part);
static Evas_Object *_item_content_get_hook(const Elm_Object_Item *it,
                                           const char *part);
static void _item_disable_hook(Elm_Object_Item *it);
static void _item_del_pre_hook(Elm_Object_Item *it);
static void _item_signal_emit_hook(Elm_Object_Item *it, const char *emission,
                                   const char *source);
static void _sizing_eval(Evas_Object *obj);
static void _block_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _notify_resize(void *data, Evas *e, Evas_Object *obj,
                           void *event_info);
static void _scroller_resize(void *data, Evas *e, Evas_Object *obj,
                             void *event_info);
static void _timeout(void *data, Evas_Object *obj , void *event_info);
static void _sub_del(void *data, Evas_Object *obj, void *event_info);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
static void _restack(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _layout_change_cb(void *data, Evas_Object *obj __UNUSED__,
                              const char *emission __UNUSED__,
                              const char *source __UNUSED__);
static void _item_new(Elm_Popup_Content_Item *item);
static void _remove_items(Widget_Data *wd);
static void _list_new(Evas_Object *obj);
static void _list_del(Widget_Data *wd);
static void _item_sizing_eval(Elm_Popup_Content_Item *item);
static void _item_select_cb(void *data, Evas_Object *obj __UNUSED__,
                            const char *emission __UNUSED__,
                            const char *source __UNUSED__);
static void _scroller_size_calc(Evas_Object *obj);
static void _action_button_set(Evas_Object *obj, Evas_Object *btn,
                               unsigned int idx);
static Evas_Object *_action_button_get(Evas_Object *obj, unsigned int idx);
static Evas_Object *_action_button_unset(Evas_Object *obj, unsigned int idx);
static void _button_remove(Evas_Object *obj, Evas_Object *content,
                           Eina_Bool delete);
static const char SIG_BLOCK_CLICKED[] = "block,clicked";
static const char SIG_TIMEOUT[] = "timeout";
static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_BLOCK_CLICKED, ""},
       {SIG_TIMEOUT, ""},
       {NULL, NULL}
};

#define ELM_POPUP_CONTENT_ITEM_CHECK_RETURN(it, ...)                        \
  ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *)it, __VA_ARGS__); \
  ELM_CHECK_WIDTYPE(WIDGET(item), widtype) __VA_ARGS__;

static void
_layout_set(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if ((!(wd->title_text)) && (!(wd->title_icon)))
     elm_object_signal_emit(wd->base, "elm,state,title_area,hidden", "elm");
   else
     elm_object_signal_emit(wd->base, "elm,state,title_area,visible", "elm");
   if (wd->button_count)
     elm_object_signal_emit(wd->base, "elm,state,action_area,visible", "elm");
   else
     elm_object_signal_emit(wd->base, "elm,state,action_area,hidden", "elm");
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   free(wd);
}

static void
_del_pre_hook(Evas_Object *obj)
{
   unsigned int i;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   evas_object_smart_callback_del(wd->notify, "block,clicked",
                                  _block_clicked_cb);
   evas_object_smart_callback_del(wd->notify, "timeout", _timeout);
   evas_object_event_callback_del(wd->notify, EVAS_CALLBACK_RESIZE,
                                  _notify_resize);
   wd->button_count = 0;
   for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
     if (wd->buttons[i])
       {
          evas_object_del(wd->buttons[i]->btn);
          free(wd->buttons[i]);
          wd->buttons[i] = NULL;
       }
   if (wd->items)
     {
        _remove_items(wd);
        _list_del(wd);
     }
}

static void
_theme_hook(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Popup_Content_Item *item;
   char buf[128];
   unsigned int i = 0;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   elm_layout_theme_set(wd->base, "popup", "base", elm_widget_style_get(obj));
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   if (wd->button_count)
     {
        snprintf(buf, sizeof(buf), "buttons%u", wd->button_count);
        elm_layout_theme_set(wd->action_area, "popup", buf,
                             elm_widget_style_get(obj));
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          {
             if (!wd->buttons[i]) continue;
             elm_object_style_set(wd->buttons[i]->btn, buf);
          }
     }
   elm_layout_theme_set(wd->content_area, "popup", "content",
                        elm_widget_style_get(obj));
   if (wd->content_text_obj)
     {
        snprintf(buf, sizeof(buf), "popup/%s", elm_widget_style_get(obj));
        elm_object_style_set(wd->content_text_obj, buf);
     }
   else if (wd->items)
     {
        EINA_LIST_FOREACH(wd->items, elist, item)
          {
             _elm_theme_object_set(obj, VIEW(item), "popup",
                                   "item", elm_widget_style_get(obj));
             if (item->label)
               {
                  edje_object_part_text_set(VIEW(item), "elm.text",
                                            item->label);
                  edje_object_signal_emit(VIEW(item),
                                          "elm,state,item,text,visible", "elm");
               }
             if (item->icon)
               edje_object_signal_emit(VIEW(item),
                                       "elm,state,item,icon,visible", "elm");
             if (item->disabled)
               edje_object_signal_emit(VIEW(item),
                                       "elm,state,item,disabled", "elm");
             evas_object_show(VIEW(item));
             edje_object_message_signal_process(VIEW(item));
          }
        _scroller_size_calc(obj);
     }
   if (wd->title_text)
     {
        elm_object_part_text_set(wd->base, "elm.text.title", wd->title_text);
        elm_object_signal_emit(wd->base, "elm,state,title,text,visible", "elm");
     }
   if (wd->title_icon)
     elm_object_signal_emit(wd->base, "elm,state,title,icon,visible", "elm");
   _layout_set(obj);
   edje_object_message_signal_process(elm_layout_edje_get(wd->base));
   _sizing_eval(obj);
}

static void
_item_sizing_eval(Elm_Popup_Content_Item *item)
{
   Evas_Coord min_w = -1, min_h = -1, max_w = -1, max_h = -1;

   edje_object_size_min_restricted_calc(VIEW(item), &min_w, &min_h, min_w,
                                        min_h);
   evas_object_size_hint_min_set(VIEW(item), min_w, min_h);
   evas_object_size_hint_max_set(VIEW(item), max_w, max_h);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Popup_Content_Item *item;
   Evas_Coord w_box = 0, h_box = 0;
   Evas_Coord minw_box = 0, minh_box = 0;
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->items)
     {
        EINA_LIST_FOREACH(wd->items, elist, item)
          {
             _item_sizing_eval(item);
             evas_object_size_hint_min_get(VIEW(item), &minw_box,
                                           &minh_box);
             if (minw_box > w_box)
               w_box = minw_box;
             if (minh_box != -1)
               h_box += minh_box;
          }
        evas_object_size_hint_min_set(wd->box, w_box, h_box);
        evas_object_size_hint_min_set(wd->scr, w_box, MIN(h_box, wd->max_sc_h));
        evas_object_size_hint_max_set(wd->scr, w_box, wd->max_sc_h);
        evas_object_smart_calculate(wd->scr);
     }
   edje_object_size_min_calc(elm_layout_edje_get(wd->base), &minw, &minh);
   evas_object_size_hint_min_set(wd->base, minw, minh);
   evas_object_size_hint_max_set(wd->base, maxw, maxh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   elm_layout_sizing_eval(wd->base);
}

static void
_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_sub_del(void *data, Evas_Object *obj, void *event_info)
{
   Elm_Popup_Content_Item *item;
   unsigned int i;
   Evas_Object *sub = event_info;
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;
   if (obj == data)
     {
        if ((item = evas_object_data_get(sub, "_popup_content_item")) != NULL)
          {
             if (sub == item->icon)
               {
                  edje_object_part_unswallow(VIEW(item), sub);
                  edje_object_signal_emit(VIEW(item),
                                          "elm,state,item,icon,hidden", "elm");
                  item->icon = NULL;
               }
          }
     }
   else if (obj == wd->content_area)
     {
        if (sub == wd->content)
          {
             wd->content = NULL;
             elm_object_part_content_unset(wd->base, "elm.swallow.content");
             _sizing_eval(data);
          }
        else if (sub == wd->content_text_obj)
          {
             wd->content_text_obj = NULL;
             elm_object_part_content_unset(wd->base, "elm.swallow.content");
             _sizing_eval(data);
          }
        else if (sub == wd->scr)
          {
             wd->scr = NULL;
             wd->box = NULL;
             elm_object_part_content_unset(wd->base, "elm.swallow.content");
             _sizing_eval(data);
          }
     }
   else if (obj == wd->action_area)
     {
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          {
             if (wd->buttons[i] && sub == wd->buttons[i]->btn &&
                      wd->buttons[i]->delete_me == EINA_TRUE)
               {
                  _button_remove(data, sub, EINA_FALSE);
                  break;
               }
          }
     }
   else if (obj == wd->base)
     {
        if (sub == wd->title_icon)
          {
             elm_object_signal_emit(wd->base, "elm,state,title,icon,hidden",
                                    "elm");
             edje_object_message_signal_process(wd->base);
             wd->title_icon = NULL;
          }
     }
}

static void
_block_clicked_cb(void *data, Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;
   evas_object_smart_callback_call(data, SIG_BLOCK_CLICKED, NULL);
}

static void
_timeout(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_TIMEOUT, NULL);
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Eina_List *elist;
   Elm_Popup_Content_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   elm_object_mirrored_set(wd->notify, rtl);
   if (wd->items)
     EINA_LIST_FOREACH(wd->items, elist, item)
       edje_object_mirrored_set(VIEW(item), rtl);
}

static void
_layout_change_cb(void *data, Evas_Object *obj __UNUSED__,
                  const char *emission __UNUSED__,
                  const char *source __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get((Evas_Object*)data);

   if (!wd) return;
   if (wd->base)
     elm_layout_sizing_eval(wd->base);
}

static void
_restack(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
         void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   evas_object_layer_set(wd->notify, evas_object_layer_get(obj));
}

static void
_scroller_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj,
                 void *event_info __UNUSED__)
{
   Evas_Coord w, h;
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd || wd->scr_size_recalc) return;
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w && h)
     {
        if ((w <= wd->max_sc_w) && (h <= wd->max_sc_h))
          {
             _sizing_eval(data);
             wd->scr_size_recalc = EINA_TRUE;
             return;
          }
     }
   if (wd->max_sc_w < w)
     wd->max_sc_w = w;
   if (wd->max_sc_h < h)
     wd->max_sc_h = h;
   _sizing_eval(data);
}

static void
_list_new(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   //Scroller
   wd->scr = elm_scroller_add(obj);
   elm_scroller_bounce_set(wd->scr, EINA_FALSE, EINA_TRUE);
   elm_object_content_set(wd->scr, wd->box);
   evas_object_size_hint_weight_set(wd->scr, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->scr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_RESIZE,
                                  _scroller_resize, obj);
   evas_object_event_callback_add(wd->scr,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);
   elm_object_part_content_set(wd->base, "elm.swallow.content", wd->scr);

   //Box
   wd->box = elm_box_add(obj);
   evas_object_size_hint_weight_set(wd->box, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->box, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_object_content_set(wd->scr, wd->box);
}

static void
_list_del(Widget_Data *wd)
{
   if (!wd->scr) return;
   elm_object_part_content_unset(wd->base, "elm.swallow.content");
   evas_object_event_callback_del(wd->scr, EVAS_CALLBACK_RESIZE,
                                  _scroller_resize);
   evas_object_event_callback_del(wd->scr, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints);
   evas_object_del(wd->scr);
   wd->scr = NULL;
   wd->box = NULL;
}

static void
_scroller_size_calc(Evas_Object *obj)
{
   Evas_Coord h;
   Evas_Coord h_title = 0;
   Evas_Coord h_action_area = 0;
   Evas_Coord h_content = 0;
   const char *action_area_height;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !wd->items) return;
   wd->scr_size_recalc = EINA_FALSE;
   wd->max_sc_h = -1;
   wd->max_sc_w = -1;
   evas_object_geometry_get(wd->notify, NULL, NULL, NULL, &h);
   if (wd->title_text || wd->title_icon)
     edje_object_part_geometry_get(elm_layout_edje_get(wd->base),
                                   "elm.bg.title", NULL, NULL, NULL, &h_title);
   if (wd->button_count)
     {
        action_area_height = edje_object_data_get(
                 elm_layout_edje_get(wd->action_area), "action_area_height");
        if (action_area_height) h_action_area = (int)(atoi(action_area_height)
                 * _elm_config->scale * elm_object_scale_get(obj));
     }
   h_content = h - (h_title + h_action_area);
   wd->max_sc_h = h_content;
}

static void
_item_select_cb(void *data, Evas_Object *obj __UNUSED__,
                const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Elm_Popup_Content_Item *item = data;

   if (!item || item->disabled) return;
   if (item->func)
     item->func((void*)item->base.data, WIDGET(item), data);
}

static void
_item_new(Elm_Popup_Content_Item *item)
{
   Widget_Data *wd = elm_widget_data_get(WIDGET(item));

   if (!wd) return;
   elm_widget_item_text_set_hook_set(item, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(item, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(item, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(item, _item_content_unset_hook);
   elm_widget_item_disable_hook_set(item, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_signal_emit_hook_set(item, _item_signal_emit_hook);
   VIEW(item) = edje_object_add(evas_object_evas_get(wd->base));
   _elm_theme_object_set(WIDGET(item), VIEW(item), "popup", "item",
                         elm_widget_style_get(WIDGET(item)));
   edje_object_mirrored_set(VIEW(item), elm_widget_mirrored_get(WIDGET(item)));
   edje_object_signal_callback_add(VIEW(item), "elm,action,click", "",
                                   _item_select_cb, item);
   evas_object_size_hint_align_set(VIEW(item), EVAS_HINT_FILL,
                                   EVAS_HINT_FILL);
   evas_object_show(VIEW(item));
}

static void
_remove_items(Widget_Data *wd)
{
   Eina_List *elist;
   Elm_Popup_Content_Item *item;

   if (!wd->items) return;
   EINA_LIST_FOREACH(wd->items, elist, item)
     {
        if (item->label)
          eina_stringshare_del(item->label);
        if (item->icon)
          {
             evas_object_del(item->icon);
             item->icon = NULL;
          }
        evas_object_del(VIEW(item));
        wd->items = eina_list_remove(wd->items, item);
        free(item);
     }
   wd->items = NULL;
}

static void
_notify_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
               void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (wd->items)
     _scroller_size_calc(data);
   _sizing_eval(data);
}

static void
_title_text_set(Evas_Object *obj, const char *text)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_Bool title_visibility_old, title_visibility_current;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->title_text == text) return;
   title_visibility_old = (wd->title_text) || (wd->title_icon);
   eina_stringshare_replace(&wd->title_text, text);
   elm_object_part_text_set(wd->base, "elm.text.title", text);
   if (wd->title_text)
     elm_object_signal_emit(wd->base, "elm,state,title,text,visible", "elm");
   else
     elm_object_signal_emit(wd->base, "elm,state,title,text,hidden", "elm");
   title_visibility_current = (wd->title_text) || (wd->title_icon);
   if (title_visibility_old != title_visibility_current)
     _layout_set(obj);
   edje_object_message_signal_process(wd->base);
   _sizing_eval(obj);
}

static void
_content_text_set(Evas_Object *obj, const char *text)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Evas_Object *prev_content;
   char buf[128];
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->items)
     {
        _remove_items(wd);
        _list_del(wd);
     }
   prev_content = elm_object_part_content_get(wd->content_area,
                                              "elm.swallow.content");
   if (prev_content)
     evas_object_del(prev_content);
   if (text)
     {
        elm_object_part_content_set(wd->base, "elm.swallow.content",
                                    wd->content_area);
        wd->content_text_obj = elm_label_add(obj);
        snprintf(buf, sizeof(buf), "popup/%s",
                 elm_widget_style_get(obj));
        elm_object_style_set(wd->content_text_obj, buf);
        elm_label_line_wrap_set(wd->content_text_obj,
                                wd->content_text_wrap_type);
        elm_object_text_set(wd->content_text_obj, text);
        evas_object_size_hint_weight_set(wd->content_text_obj, EVAS_HINT_EXPAND,
                                         0.0);
        evas_object_size_hint_align_set(wd->content_text_obj, EVAS_HINT_FILL,
                                        EVAS_HINT_FILL);
        elm_object_part_content_set(wd->content_area, "elm.swallow.content",
                                    wd->content_text_obj);
     }
   _sizing_eval(obj);
}

static void
_text_set_hook(Evas_Object *obj, const char *part,
                              const char *label)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (!part || !strcmp(part, "default"))
     _content_text_set(obj, label);
   else if (!strcmp(part, "title,text"))
     _title_text_set(obj, label);
   else
     WRN("The part name is invalid! : popup=%p", obj);
}

static const char*
_title_text_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   return wd->title_text;
}

static const char*
_content_text_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   const char *str = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   if (wd->content_text_obj)
     str = elm_object_text_get(wd->content_text_obj);
   return str;
}

static const char *
_text_get_hook(const Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   const char *str = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   if (!part || !strcmp(part, "default"))
     str = _content_text_get(obj);
   else if (!strcmp(part, "title,text"))
     str = _title_text_get(obj);
   else
     WRN("The part name is invalid! : popup=%p", obj);
   return str;
}

static void
_title_icon_set(Evas_Object *obj, Evas_Object *icon)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_Bool title_visibility_old, title_visibility_current;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->title_icon == icon) return;
   title_visibility_old = (wd->title_text) || (wd->title_icon);
   if (wd->title_icon) evas_object_del(wd->title_icon);

   wd->title_icon = icon;
   title_visibility_current = (wd->title_text) || (wd->title_icon);
   elm_object_part_content_set(wd->base, "elm.swallow.title.icon",
                               wd->title_icon);
   if (wd->title_icon)
     elm_object_signal_emit(wd->base, "elm,state,title,icon,visible", "elm");
   if (title_visibility_old != title_visibility_current) _layout_set(obj);
   edje_object_message_signal_process(wd->base);
   _sizing_eval(obj);
}

static void
_content_set(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Evas_Object *prev_content;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->content && wd->content == content) return;
   if (wd->items)
     {
        _remove_items(wd);
        _list_del(wd);
     }
   prev_content = elm_object_part_content_get(wd->content_area,
                                              "elm.swallow.content");
   if (prev_content)
     evas_object_del(prev_content);
   wd->content = content;
   if (content)
     {
        elm_object_part_content_set(wd->base, "elm.swallow.content",
                                    wd->content_area);
        elm_object_part_content_set(wd->content_area, "elm.swallow.content",
                                    content);
     }
   _sizing_eval(obj);
}

static void
_button_remove(Evas_Object *obj, Evas_Object *content, Eina_Bool delete)
{
   unsigned int i = 0, position = 0;
   char buf[128];
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd->button_count) return;
   for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
    {
       if (wd->buttons[i])
         {
            position++;
            wd->buttons[i]->delete_me = EINA_FALSE;
            if (wd->buttons[i]->btn == content)
              {
                 snprintf(buf, sizeof(buf), "actionbtn%u", position);
                 elm_object_part_content_unset(wd->action_area, buf);
                 evas_object_hide(content);
                 if (delete) evas_object_del(content);
                 free(wd->buttons[i]);
                 wd->buttons[i] = NULL;
                 wd->button_count -= 1;
               }
         }
    }
   position = 0;
   for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
    {
       if (!wd->buttons[i]) continue;
       position++;
       snprintf(buf, sizeof(buf), "actionbtn%u", position);
       elm_object_part_content_unset(wd->action_area, buf);
       elm_object_part_content_set(wd->action_area, buf,
                                   wd->buttons[i]->btn);
       wd->buttons[i]->delete_me = EINA_TRUE;
    }
   if (!wd->button_count)
    {
       _layout_set(obj);
       edje_object_message_signal_process(elm_layout_edje_get(wd->base));
    }
   else
    {
       snprintf(buf, sizeof(buf), "buttons%u", wd->button_count);
       elm_layout_theme_set(wd->action_area, "popup", buf,
                            elm_widget_style_get(obj));
    }
}

static void
_action_button_set(Evas_Object *obj, Evas_Object *btn, unsigned int idx)
{
   Action_Area_Data *adata;
   char buf[128];
   unsigned int num = idx - 1, i = 0, position = 0;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (num >= ELM_POPUP_ACTION_BUTTON_MAX) return;
   if (wd->buttons[num])
     _button_remove(obj, wd->buttons[num]->btn, EINA_TRUE);
   if (btn)
     {
        wd->button_count++;
        snprintf(buf, sizeof(buf), "buttons%u", wd->button_count);
        elm_layout_theme_set(wd->action_area, "popup", buf,
                             elm_widget_style_get(obj));
        adata = ELM_NEW(Action_Area_Data);
        adata->obj = obj;
        adata->btn = btn;
        wd->buttons[num] = adata;
        /* Adding delete_me state inside action data as unset calls _sub_del
           too and before setting a new content, the previous one needs to
           be unset in order to avoid unwanted deletion. This way rearrangement
            of buttons can be achieved in action area.
         */
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          if (wd->buttons[i])
            wd->buttons[i]->delete_me = EINA_FALSE;
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          {
             if (!wd->buttons[i]) continue;
             position++;
             snprintf(buf, sizeof(buf), "actionbtn%u", position);
             elm_object_part_content_unset(wd->action_area, buf);
             elm_object_part_content_set(wd->action_area, buf,
                                         wd->buttons[i]->btn);
             /* Setting delete_me to TRUE in order to let _sub_del handle it
                if deleted externally and update the buttons array after freeing
                action data allocated earlier.
              */
             wd->buttons[i]->delete_me = EINA_TRUE;
          }
        elm_object_part_content_set(wd->base, "elm.swallow.action_area",
                                    wd->action_area);
        if (wd->button_count == 1)
          _layout_set(obj);
        edje_object_message_signal_process(wd->base);
        if (wd->items)
          _scroller_size_calc(obj);
        _sizing_eval(obj);
     }
}

static void
_content_set_hook(Evas_Object *obj, const char *part,
                                 Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   char buff[3];
   unsigned int i;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (!part || !strcmp(part, "default"))
     _content_set(obj, content);
   else if (!strcmp(part, "title,icon"))
     _title_icon_set(obj, content);
   else if (!strncmp(part, "button", 6))
     {
        part += 6;
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          {
             snprintf(buff, sizeof(buff), "%u", i+1);
             if (!strncmp(part, buff, sizeof(buff)))
               {
                  _action_button_set(obj, content, i+1);
                  break;
               }
          }
      }
   else
     WRN("The part name is invalid! : popup=%p", obj);
}

static Evas_Object *
_title_icon_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   return wd->title_icon;
}

static Evas_Object *
_content_get(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   return wd->content;
}

static Evas_Object *
_action_button_get(Evas_Object *obj, unsigned int idx)
{
   unsigned int num = idx - 1;
   Evas_Object *button = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !wd->button_count) return NULL;
   if (wd->buttons[num])
     button = wd->buttons[num]->btn;
   return button;
}

static Evas_Object *
_content_get_hook(Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Evas_Object *content = NULL;
   char buff[3];
   unsigned int i;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   if (!part || !strcmp(part, "default"))
     content = _content_get(obj);
   else if (!strcmp(part, "title,text"))
     content = _title_icon_get(obj);
   else if (!strncmp(part, "button", 6))
     {
        part += 6;
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          {
             snprintf(buff, sizeof(buff), "%u", i+1);
             if (!strncmp(part, buff, sizeof(buff)))
               {
                  content = _action_button_get(obj, i+1);
                  break;
               }
          }
      }
   else
     WRN("The part name is invalid! : popup=%p", obj);
   return content;
}

static Evas_Object *
_content_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Evas_Object *content;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !wd->content) return NULL;
   content = elm_object_part_content_unset(wd->content_area,
                                           "elm.swallow.content");
   wd->content = NULL;
   _sizing_eval(obj);
   return content;
}

static Evas_Object *
_title_icon_unset(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Evas_Object *icon;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd || !wd->title_icon) return NULL;
   icon = elm_object_part_content_unset(wd->base, "elm.swallow.title.icon");
   wd->title_icon = NULL;
   return icon;
}

static Evas_Object *
_action_button_unset(Evas_Object *obj, unsigned int idx)
{
   unsigned int num = idx -1;
   Evas_Object *button = NULL;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   if ((!wd->button_count) || (num >= ELM_POPUP_ACTION_BUTTON_MAX))
     return NULL;

   if (wd->buttons[num])
     {
        button = wd->buttons[num]->btn;
        _button_remove(obj, button, EINA_FALSE);
      }
   return button;
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Evas_Object *content = NULL;
   char buff[3];
   unsigned int i;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   if (!part || !strcmp(part, "default"))
     content = _content_unset(obj);
   else if (!strcmp(part, "title,icon"))
     content = _title_icon_unset(obj);
   else if (!strncmp(part, "button", 6))
     {
        part += 6;
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          {
             snprintf(buff, sizeof(buff), "%u", i+1);
             if (!strncmp(part, buff, sizeof(buff)))
               {
                  content = _action_button_unset(obj, i+1);
                  break;
               }
          }
      }
   else
     WRN("The part name is invalid! : popup=%p", obj);
   return content;
}

static Eina_Bool
_focus_next_hook(const Evas_Object *obj __UNUSED__,
                 Elm_Focus_Direction dir __UNUSED__,
                 Evas_Object **next __UNUSED__)
{
   //TODO: Implement Focus chanin Handling in Popup for action area buttons
   return EINA_FALSE;
}
static void
_item_text_set(Elm_Popup_Content_Item *item, const char *label)
{
   if (!eina_stringshare_replace(&item->label, label)) return;
   edje_object_part_text_set(VIEW(item), "elm.text", label);
   if (item->label)
     edje_object_signal_emit(VIEW(item),
                             "elm,state,item,text,visible", "elm");
   else
     edje_object_signal_emit(VIEW(item),
                             "elm,state,item,text,hidden", "elm");
   edje_object_message_signal_process(VIEW(item));
}

static void
_item_text_set_hook(Elm_Object_Item *it, const char *part, const char *label)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Popup_Content_Item *item = (Elm_Popup_Content_Item *)it;

   if ((!part) || (!strcmp(part, "default")))
     {
        _item_text_set(item, label);
        return;
     }
   WRN("The part name is invalid! : popup=%p", WIDGET(item));
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it, const char *part)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Popup_Content_Item *item = (Elm_Popup_Content_Item *)it;

   if ((!part) || (!strcmp(part, "default")))
     return item->label;
   WRN("The part name is invalid! : popup=%p", WIDGET(item));
   return NULL;
}

static void
_item_icon_set(Elm_Popup_Content_Item *item, Evas_Object *icon)
{
   if (item->icon == icon) return;
   if (item->icon)
     {
        evas_object_data_del(item->icon, "_popup_content_item");
        evas_object_del(item->icon);
     }
   item->icon = icon;
   if (item->icon)
     {
        elm_widget_sub_object_add(WIDGET(item), item->icon);
        evas_object_data_set(item->icon, "_popup_content_item", item);
        edje_object_part_swallow(VIEW(item), "elm.swallow.content",
                                 item->icon);
        edje_object_signal_emit(VIEW(item),
                                "elm,state,item,icon,visible", "elm");
     }
   else
     edje_object_signal_emit(VIEW(item),
                             "elm,state,item,icon,hidden", "elm");
   edje_object_message_signal_process(item->base.view);
}

static void
_item_content_set_hook(Elm_Object_Item *it, const char *part,
                       Evas_Object *content)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Popup_Content_Item *item = (Elm_Popup_Content_Item *)it;

   if ((!(part)) || (!strcmp(part, "default")))
     _item_icon_set(item, content);
   else
     WRN("The part name is invalid! : popup=%p", WIDGET(item));
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Popup_Content_Item *item = (Elm_Popup_Content_Item *)it;

   if ((!(part)) || (!strcmp(part, "default")))
     return item->icon;
   WRN("The part name is invalid! : popup=%p", WIDGET(item));
   return NULL;
}

static Evas_Object *
_item_icon_unset(Elm_Popup_Content_Item *item)
{
   Evas_Object *icon = item->icon;

   if (!item->icon) return NULL;
   elm_widget_sub_object_del(WIDGET(item), icon);
   evas_object_data_del(icon, "_popup_content_item");
   edje_object_part_unswallow(item->base.view, icon);
   edje_object_signal_emit(VIEW(item),
                           "elm,state,item,icon,hidden", "elm");
   item->icon = NULL;
   return icon;
}

static Evas_Object *
_item_content_unset_hook(const Elm_Object_Item *it,
                         const char *part)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Evas_Object *content = NULL;
   Elm_Popup_Content_Item *item = (Elm_Popup_Content_Item *)it;

   if ((!(part)) || (!strcmp(part, "default")))
     content = _item_icon_unset(item);
   else
     WRN("The part name is invalid! : popup=%p", WIDGET(item));
   return content;
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Popup_Content_Item *item = (Elm_Popup_Content_Item *)it;
   Widget_Data *wd = elm_widget_data_get(WIDGET(item));

   if (!wd) return;
   if (elm_widget_item_disabled_get(it))
     edje_object_signal_emit(VIEW(item), "elm,state,item,disabled", "elm");
   else
     edje_object_signal_emit(VIEW(item), "elm,state,item,enabled", "elm");
}

static void
_item_del_pre_hook(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Popup_Content_Item *item = (Elm_Popup_Content_Item *)it;
   Widget_Data *wd = elm_widget_data_get(WIDGET(item));

   if (!wd) return;
   if (item->icon)
     evas_object_del(item->icon);
   eina_stringshare_del(item->label);
   wd->items = eina_list_remove(wd->items, item);
   if (!eina_list_count(wd->items))
     {
        wd->items = NULL;
        _list_del(wd);
     }
}

static void
_item_signal_emit_hook(Elm_Object_Item *it, const char *emission,
                       const char *source)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Popup_Content_Item *item = (Elm_Popup_Content_Item *)it;

   edje_object_signal_emit(VIEW(item), emission, source);
}

static void
_popup_show(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_show(wd->notify);
}

EAPI Evas_Object *
elm_popup_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   int i = 0;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);
   ELM_SET_WIDTYPE(widtype, "popup");
   elm_widget_type_set(obj, widtype);
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_text_set_hook_set(obj, _text_set_hook);
   elm_widget_text_get_hook_set(obj, _text_get_hook);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_content_unset_hook_set(obj,_content_unset_hook);
   elm_widget_focus_next_hook_set(obj, _focus_next_hook);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   wd->notify = elm_notify_add(obj);
   elm_notify_parent_set(wd->notify, parent);
   elm_notify_orient_set(wd->notify, ELM_NOTIFY_ORIENT_CENTER);
   elm_notify_allow_events_set(wd->notify, EINA_FALSE);
   evas_object_size_hint_weight_set(wd->notify,
                                    EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->notify, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_style_set(wd->notify, "popup");

   evas_object_event_callback_add(wd->notify, EVAS_CALLBACK_RESIZE,
                                  _notify_resize, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _popup_show,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESTACK, _restack, NULL);
   wd->base = elm_layout_add(obj);
   evas_object_size_hint_weight_set(wd->base, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->base, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_layout_theme_set(wd->base, "popup", "base", elm_widget_style_get(obj));
   elm_object_content_set(wd->notify, wd->base);

   elm_object_signal_callback_add(wd->base, "elm,state,title_area,visible",
                                  "elm", _layout_change_cb, obj);
   elm_object_signal_callback_add(wd->base, "elm,state,title_area,hidden",
                                  "elm", _layout_change_cb, obj);
   elm_object_signal_callback_add(wd->base, "elm,state,action_area,visible",
                                  "elm", _layout_change_cb, obj);
   elm_object_signal_callback_add(wd->base, "elm,state,action_area,hidden",
                                  "elm", _layout_change_cb, obj);
   wd->content_area = elm_layout_add(obj);
   elm_layout_theme_set(wd->content_area, "popup", "content",
                        elm_widget_style_get(obj));
   wd->action_area = elm_layout_add(obj);
   evas_object_size_hint_weight_set(wd->action_area, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(wd->action_area, EVAS_HINT_FILL,
                                   EVAS_HINT_FILL);
   evas_object_event_callback_add(wd->action_area,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);
   evas_object_event_callback_add(wd->content_area,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints, obj);
   evas_object_smart_callback_add(wd->base, "sub-object-del",
                                  _sub_del, obj);
   evas_object_smart_callback_add(wd->content_area, "sub-object-del",
                                  _sub_del, obj);
   evas_object_smart_callback_add(wd->action_area, "sub-object-del",
                                  _sub_del, obj);
   evas_object_smart_callback_add(obj, "sub-object-del",
                                  _sub_del, obj);

   wd->content_text_wrap_type = ELM_WRAP_MIXED;
   evas_object_smart_callback_add(wd->notify, "block,clicked",
                                  _block_clicked_cb, obj);
   evas_object_smart_callback_add(wd->notify, "timeout", _timeout, obj);
   for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
     wd->buttons[i] = NULL;
   _layout_set(obj);
   //TODO: To use scroller for description also
   return obj;
}

EAPI void
elm_popup_content_text_wrap_type_set(Evas_Object *obj, Elm_Wrap_Type wrap)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Evas_Object *content_text_obj;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (wd->content_text_wrap_type == ELM_WRAP_NONE) return;
   //Need to wrap the content text, so not allowing ELM_WRAP_NONE
   wd->content_text_wrap_type = wrap;
   if (wd->content_text_obj)
     {
        content_text_obj = elm_object_part_content_get(wd->content_area,
                                                       "elm.swallow.content");
        elm_label_line_wrap_set(content_text_obj, wrap);
     }
}

EAPI Elm_Wrap_Type
elm_popup_content_text_wrap_type_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) ELM_WRAP_LAST;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return ELM_WRAP_LAST;
   return wd->content_text_wrap_type;
}

EAPI void
elm_popup_orient_set(Evas_Object *obj, Elm_Popup_Orient orient)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   if (orient >= ELM_POPUP_ORIENT_LAST) return;
   elm_notify_orient_set(wd->notify, (Elm_Notify_Orient)orient);
}

EAPI Elm_Popup_Orient
elm_popup_orient_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) -1;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return ELM_POPUP_ORIENT_LAST;
   return (Elm_Popup_Orient)elm_notify_orient_get(wd->notify);
}

EAPI void
elm_popup_timeout_set(Evas_Object *obj, double timeout)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   elm_notify_timeout_set(wd->notify, timeout);
}

EAPI double
elm_popup_timeout_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) 0.0;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return 0.0;
   return elm_notify_timeout_get(wd->notify);
}

EAPI void
elm_popup_allow_events_set(Evas_Object *obj, Eina_Bool allow)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Eina_Bool allow_events = !!allow;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;
   elm_notify_allow_events_set(wd->notify, allow_events);
}

EAPI Eina_Bool
elm_popup_allow_events_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return EINA_FALSE;
   return elm_notify_allow_events_get(wd->notify);
}

EAPI Elm_Object_Item *
elm_popup_item_append(Evas_Object *obj, const char *label,
                              Evas_Object *icon, Evas_Smart_Cb func,
                              const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Evas_Object *prev_content;
   Elm_Popup_Content_Item *item;
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return NULL;
   item = elm_widget_item_new(obj, Elm_Popup_Content_Item);
   if (!item) return NULL;
   if (wd->content || wd->content_text_obj)
     {
        prev_content = elm_object_part_content_get(wd->content_area,
                                                   "elm.swallow.content");
        if (prev_content)
          evas_object_del(prev_content);
     }

   //The first item is appended.
   if (!wd->items)
     _list_new(obj);

   item->func = func;
   item->base.data = data;

   _item_new(item);
   _item_icon_set(item, icon);
   _item_text_set(item, label);
   elm_box_pack_end(wd->box, VIEW(item));
   wd->items = eina_list_append(wd->items, item);

   _scroller_size_calc(obj);
   _sizing_eval(obj);
   return (Elm_Object_Item *)item;
}
