#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Naviframe_Item Elm_Naviframe_Item;

struct _Widget_Data
{
   Eina_List    *stack;
   Evas_Object  *base;
   Eina_Bool     preserve: 1;
   Eina_Bool     pass_events: 1;
};

struct _Elm_Naviframe_Item
{
   Elm_Widget_Item    base;
   Evas_Object       *title;
   Evas_Object       *content;
   const char        *title_label;
   const char        *title_sublabel;
   Evas_Object       *title_prev_btn;
   Evas_Object       *title_next_btn;
   Evas_Object       *title_icon;
   const char        *style;
   Eina_Bool          back_btn: 1;
   Eina_Bool          title_visible: 1;
};

static const char *widtype = NULL;

static const char SIG_TRANSITION_FINISHED[] = "transition,finished";
static const char SIG_TITLE_CLICKED[] = "title,clicked";

static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_TRANSITION_FINISHED, ""},
       {SIG_TITLE_CLICKED, ""},
       {NULL, NULL}
};

#define ELM_NAVIFRAME_ITEM_CHECK_RETURN(it, ...) \
   ELM_WIDGET_ITEM_CHECK_OR_RETURN((Elm_Widget_Item *) it, __VA_ARGS__); \
   ELM_CHECK_WIDTYPE(it->base.widget, widtype) __VA_ARGS__;

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _disable_hook(Evas_Object *obj);
static void _sizing_eval(Evas_Object *obj);
static void _item_sizing_eval(Elm_Naviframe_Item *it);
static void _move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _resize(void *data,
                    Evas *e,
                    Evas_Object *obj,
                    void *event_info);
static void _title_clicked(void *data, Evas_Object *obj,
                           const char *emission,
                           const char *source);
static void _back_btn_clicked(void *data,
                              Evas_Object *obj,
                              void *event_info);
static Evas_Object *_back_btn_new(Evas_Object *obj);
static void _title_prev_btn_del(void *data,
                                Evas *e,
                                Evas_Object *obj,
                                void *event_info);
static void _title_next_btn_del(void *data,
                                Evas *e,
                                Evas_Object *obj,
                                void *event_info);
static void _title_icon_del(void *data,
                            Evas *e,
                            Evas_Object *obj,
                            void *event_info);
static void _content_del(void *data,
                         Evas *e,
                         Evas_Object *obj,
                         void *event_info);
static void _title_prev_btn_set(Elm_Naviframe_Item *it,
                                Evas_Object *btn,
                                Eina_Bool back_btn);
static void _title_next_btn_set(Elm_Naviframe_Item *it, Evas_Object *btn);
static void _item_del(Elm_Naviframe_Item *it);
static void _pushed_finished(void *data,
                             Evas_Object *obj,
                             const char *emission,
                             const char *source);
static void _popped_finished(void *data,
                             Evas_Object *obj,
                             const char *emission,
                             const char *source);
static void _show_finished(void *data,
                           Evas_Object *obj,
                           const char *emission,
                           const char *source);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   Eina_List *list;
   Elm_Naviframe_Item *it;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   EINA_LIST_FOREACH(wd->stack, list, it)
     _item_del(it);
   eina_list_free(wd->stack);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj __UNUSED__)
{
   //FIXME:
}

static void
_disable_hook(Evas_Object *obj __UNUSED__)
{
   //FIXME:
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   Eina_List *list;
   Elm_Naviframe_Item *it;
   wd  = elm_widget_data_get(obj);
   if (!wd) return;

   list = eina_list_last(wd->stack);
   if (!list) return;

   EINA_LIST_FOREACH(wd->stack, list, it)
     _item_sizing_eval(it);
}

static void
_item_sizing_eval(Elm_Naviframe_Item *it)
{
   Widget_Data *wd;
   Evas_Coord x, y, w, h;
   if (!it) return;

   wd = elm_widget_data_get(it->base.widget);
   if (!wd) return;

   evas_object_geometry_get(it->base.widget, &x, &y, &w, &h);
   evas_object_move(it->base.view, x, y);
   evas_object_resize(it->base.view, w, h);
}

static void
_move(void *data __UNUSED__,
      Evas *e __UNUSED__,
      Evas_Object *obj,
      void *event_info __UNUSED__)
{
   _sizing_eval(obj);
}

static void
_resize(void *data __UNUSED__,
        Evas *e __UNUSED__,
        Evas_Object *obj,
        void *event_info __UNUSED__)
{
   _sizing_eval(obj);
}

static void
_title_clicked(void *data,
               Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__,
               const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   evas_object_smart_callback_call(it->base.widget, SIG_TITLE_CLICKED, it);
}

static void
_back_btn_clicked(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   elm_naviframe_item_pop(data);
}

static Evas_Object *
_back_btn_new(Evas_Object *obj)
{
   Evas_Object *btn;
   char buf[256];   //FIXME: How to guarantee the size of style name?
   btn = elm_button_add(obj);
   if (!btn) return NULL;
   evas_object_smart_callback_add(btn, "clicked", _back_btn_clicked, obj);
   snprintf(buf, sizeof(buf), "naviframe/back_btn/%s", elm_widget_style_get(obj));
   elm_object_style_set(btn, buf);
   return btn;
}

static void
_title_prev_btn_del(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->back_btn = EINA_FALSE;
   it->title_prev_btn = NULL;
}

static void
_title_next_btn_del(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->title_next_btn = NULL;
}

static void
_title_icon_del(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->title_icon = NULL;
   edje_object_signal_emit(it->base.view, "elm,state,icon,hide", "elm");
}

static void
_content_del(void *data,
             Evas *e __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->content = NULL;
   edje_object_signal_emit(it->base.view, "elm,state,content,hide", "elm");
}

static void
_title_prev_btn_set(Elm_Naviframe_Item *it, Evas_Object *btn, Eina_Bool back_btn)
{
   char buf[256];   //FIXME: How to guarantee the size of style name?

   if (it->title_prev_btn == btn) return;

   if (it->title_prev_btn)
     evas_object_del(it->title_prev_btn);

   it->title_prev_btn = btn;

   if (!btn) return;

   if (!back_btn)
     {
        snprintf(buf,
                 sizeof(buf),
                 "naviframe/prev_btn/%s",
                 elm_widget_style_get(it->base.widget));
        elm_object_style_set(btn, buf);
     }
   elm_widget_sub_object_add(it->base.widget, btn);
   evas_object_event_callback_add(btn,
                                  EVAS_CALLBACK_DEL,
                                  _title_prev_btn_del,
                                  it);
   edje_object_part_swallow(it->title, "elm.swallow.prev_btn", btn);
   it->back_btn = back_btn;
}

static void
_title_next_btn_set(Elm_Naviframe_Item *it, Evas_Object *btn)
{
   char buf[256];   //FIXME: How to guarantee the size of style name?

   if (it->title_next_btn == btn) return;

   if (it->title_next_btn)
     evas_object_del(it->title_next_btn);

   it->title_next_btn = btn;

   if (!btn) return;

   snprintf(buf,
            sizeof(buf),
            "naviframe/next_btn/%s",
            elm_widget_style_get(it->base.widget));
   elm_object_style_set(btn, buf);
   elm_widget_sub_object_add(it->base.widget, btn);
   evas_object_event_callback_add(btn,
                                  EVAS_CALLBACK_DEL,
                                  _title_next_btn_del,
                                  it);
   edje_object_part_swallow(it->title, "elm.swallow.next_btn", btn);
}

static void
_item_del(Elm_Naviframe_Item *it)
{
   Widget_Data *wd;

   if (!it) return;

   wd = elm_widget_data_get(it->base.widget);
   if (!wd) return;

   if (it->title_prev_btn)
     evas_object_del(it->title_prev_btn);
   if (it->title_next_btn)
     evas_object_del(it->title_next_btn);
   if (it->title_icon)
     evas_object_del(it->title_icon);
   if ((it->content) && (!wd->preserve))
     evas_object_del(it->content);

   evas_object_del(it->title);
   evas_object_del(it->base.view);

   wd->stack = eina_list_remove(wd->stack, it);

   free(it);
}

static void
_pushed_finished(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   if (!it) return;
   evas_object_hide(it->base.view);
}

static void
_popped_finished(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   _item_del(data);
}

static void
_show_finished(void *data,
               Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__,
               const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it;
   Widget_Data *wd;

   it = data;
   if (!it) return;
   wd =  elm_widget_data_get(it->base.widget);
   if (!wd) return;

   evas_object_smart_callback_call(it->base.widget,
                                   SIG_TRANSITION_FINISHED,
                                   (void *) EINA_TRUE);
   if (wd->pass_events)
     evas_object_pass_events_set(wd->base, EINA_FALSE);
}

EAPI Evas_Object *
elm_naviframe_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);
   ELM_SET_WIDTYPE(widtype, "naviframe");
   elm_widget_type_set(obj, "naviframe");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_disable_hook_set(obj, _disable_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);

   //base
   wd->base = edje_object_add(e);
   elm_widget_resize_object_set(obj, wd->base);
   _elm_theme_object_set(obj, wd->base, "naviframe", "base", "default");

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   wd->pass_events = EINA_TRUE;

   return obj;
}

EAPI Elm_Object_Item *
elm_naviframe_item_push(Evas_Object *obj, const char *title_label, Evas_Object *prev_btn, Evas_Object *next_btn, Evas_Object *content, const char *item_style)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;
   Elm_Naviframe_Item *prev_it, *it;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   //create item
   it = elm_widget_item_new(obj, Elm_Naviframe_Item);
   if (!it)
     {
        ERR("Failed to allocate new item! : naviframe=%p", obj);
        return NULL;
     }
   //item base layout
   it->base.view = edje_object_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(it->base.view, wd->base);
   elm_widget_sub_object_add(obj, it->base.view);
   edje_object_signal_callback_add(it->base.view,
                                   "elm,action,show,finished",
                                   "",
                                   _show_finished, it);
   edje_object_signal_callback_add(it->base.view,
                                   "elm,action,pushed,finished",
                                   "",
                                   _pushed_finished, it);
   edje_object_signal_callback_add(it->base.view,
                                   "elm,action,popped,finished",
                                   "",
                                   _popped_finished, it);
   elm_naviframe_item_style_set(ELM_CAST(it), item_style);

   //title
   it->title = edje_object_add(evas_object_evas_get(obj));
   elm_widget_sub_object_add(obj, it->title);
   _elm_theme_object_set(obj,
                         it->title, "naviframe",
                         "title",
                         elm_widget_style_get(obj));
   edje_object_signal_callback_add(it->title,
                                   "elm,action,clicked",
                                   "elm",
                                   _title_clicked, it);

   elm_naviframe_item_title_label_set(ELM_CAST(it), title_label);

   //title buttons
   if ((!prev_btn) && (eina_list_count(wd->stack)))
     {
        prev_btn = _back_btn_new(obj);
        _title_prev_btn_set(it, prev_btn, EINA_TRUE);
     }
   else
     _title_prev_btn_set(it, prev_btn, EINA_FALSE);

   _title_next_btn_set(it, next_btn);
   edje_object_part_swallow(it->base.view, "elm.swallow.title", it->title);

   elm_naviframe_item_content_set(ELM_CAST(it), content);

   _item_sizing_eval(it);
   evas_object_show(it->base.view);

   prev_it = ELM_CAST(elm_naviframe_top_item_get(obj));
   if (prev_it)
     {
        if (wd->pass_events)
          evas_object_pass_events_set(wd->base, EINA_TRUE);
        edje_object_signal_emit(prev_it->base.view,
                                "elm,state,pushed",
                                "elm");
        edje_object_signal_emit(it->base.view,
                                "elm,state,show",
                                "elm");
     }
   else
     edje_object_signal_emit(it->base.view, "elm,state,visible", "elm");
   it->title_visible = EINA_TRUE;
   wd->stack = eina_list_append(wd->stack, it);
   return ELM_CAST(it);
}

EAPI Evas_Object *
elm_naviframe_item_pop(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Naviframe_Item *it, *prev_it;
   Widget_Data *wd;
   Evas_Object *content = NULL;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = ELM_CAST(elm_naviframe_top_item_get(obj));
   if (!it) return NULL;
   wd->stack = eina_list_remove(wd->stack, it);

   if (wd->preserve)
     content = it->content;

   prev_it = ELM_CAST(elm_naviframe_top_item_get(obj));
   if (prev_it)
     {
        if (wd->pass_events)
          evas_object_pass_events_set(wd->base, EINA_TRUE);
        evas_object_show(prev_it->base.view);
        evas_object_raise(prev_it->base.view);
        edje_object_signal_emit(prev_it->base.view,
                                "elm,state,show",
                                "elm");
        edje_object_signal_emit(it->base.view, "elm,state,popped", "elm");
     }
   else
     _item_del(it);

   return content;
}

EAPI void
elm_naviframe_content_preserve_on_pop_set(Evas_Object *obj, Eina_Bool preserve)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->preserve = !!preserve;
}

EAPI Eina_Bool
elm_naviframe_content_preserve_on_pop_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->preserve;
}

EAPI void
elm_naviframe_item_content_set(Elm_Object_Item *it, Evas_Object *content)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it);

   if (navi_it->content == content) return;
   if (navi_it->content) evas_object_del(navi_it->content);
   elm_widget_sub_object_add(navi_it->base.widget, content);
   edje_object_part_swallow(navi_it->base.view, "elm.swallow.content", content);
   if (content)
     edje_object_signal_emit(navi_it->base.view,
                             "elm,state,content,show",
                             "elm");
   else
     edje_object_signal_emit(navi_it->base.view,
                             "elm,state,content,hide",
                             "elm");
   evas_object_event_callback_add(content,
                                  EVAS_CALLBACK_DEL,
                                  _content_del,
                                  navi_it);
   navi_it->content = content;
   _item_sizing_eval(navi_it);
}

EAPI Evas_Object *
elm_naviframe_item_content_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it, NULL);
   return navi_it->content;
}

EAPI void
elm_naviframe_item_title_label_set(Elm_Object_Item *it, const char *label)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it);

   edje_object_part_text_set(navi_it->title, "elm.text.title", label);
   if (label)
     edje_object_signal_emit(navi_it->title, "elm,state,title,show", "elm");
   else
     edje_object_signal_emit(navi_it->title, "elm,state,title,hidew", "elm");

   eina_stringshare_replace(&navi_it->title_label, label);
   _item_sizing_eval(navi_it);
}

EAPI const char *
elm_naviframe_item_title_label_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it, NULL);

   return navi_it->title_label;
}

EAPI void
elm_naviframe_item_subtitle_label_set(Elm_Object_Item *it, const char *label)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it);

   edje_object_part_text_set(navi_it->title, "elm.text.subtitle", label);
   if (label)
     edje_object_signal_emit(navi_it->title, "elm,state,subtitle,show", "elm");
   else
     edje_object_signal_emit(navi_it->title, "elm,state,subtitle,hide", "elm");

   eina_stringshare_replace(&navi_it->title_sublabel, label);
   _item_sizing_eval(navi_it);
}

EAPI const char *
elm_naviframe_item_subtitle_label_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it, NULL);
   return navi_it->title_sublabel;
}

EAPI Elm_Object_Item*
elm_naviframe_top_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->stack)) return NULL;
   return eina_list_last(wd->stack)->data;
}

EAPI Elm_Object_Item*
elm_naviframe_bottom_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return NULL;
}

EAPI void
elm_naviframe_item_button_set(Elm_Object_Item *it, Evas_Object *btn, Elm_Naviframe_Button_Type btn_type)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it);

   switch(btn_type)
     {
        case ELM_NAVIFRAME_PREV_BUTTON:
          _title_prev_btn_set(navi_it, btn, EINA_FALSE);
          break;
        case ELM_NAVIFRAME_NEXT_BUTTON:
          _title_next_btn_set(navi_it, btn);
          break;
        default:
          WRN("Button type is invalid! : naviframe=%p", navi_it->base.widget);
          break;
     }
   _item_sizing_eval(navi_it);
}

EAPI Evas_Object *
elm_naviframe_item_button_get(const Elm_Object_Item *it, Elm_Naviframe_Button_Type btn_type)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it, NULL);

   switch(btn_type)
     {
        case ELM_NAVIFRAME_PREV_BUTTON:
           return navi_it->title_prev_btn;
        case ELM_NAVIFRAME_NEXT_BUTTON:
           return navi_it->title_next_btn;
        default:
           WRN("Button type is invalid! : naviframe=%p", navi_it->base.widget);
           break;
     }
   return NULL;
}

EAPI void
elm_naviframe_item_icon_set(Elm_Object_Item *it, Evas_Object *icon)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it);

   Widget_Data *wd = elm_widget_data_get(navi_it->base.widget);
   if (!wd) return;

   if (navi_it->title_icon == icon) return;
   if (navi_it->title_icon) evas_object_del(navi_it->title_icon);
   navi_it->title_icon = icon;
   if (!icon)
     {
        edje_object_signal_emit(navi_it->title,
                                "elm,state,icon,hide",
                                "elm");
        return;
     }
   elm_widget_sub_object_add(navi_it->base.widget, icon);
   edje_object_part_swallow(navi_it->title, "elm.swallow.icon", icon);
   edje_object_signal_emit(navi_it->title, "elm,state,icon,show", "elm");

   evas_object_event_callback_add(icon, EVAS_CALLBACK_DEL, _title_icon_del, navi_it);
   _item_sizing_eval(navi_it);
}

EAPI Evas_Object *
elm_naviframe_item_icon_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it, NULL);
   return navi_it->title_icon;
}

EAPI void
elm_naviframe_item_style_set(Elm_Object_Item *it, const char *item_style)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it);

   char buf[256];

   if (!item_style) sprintf(buf, "item/basic");
   else
     {
        if (strlen(item_style) > sizeof(buf))
          WRN("too much long style name! : naviframe=%p", navi_it->base.widget);
        else
          sprintf(buf, "item/%s", item_style);
     }
   _elm_theme_object_set(navi_it->base.widget,
                         navi_it->base.view,
                         "naviframe",
                         buf,
                         elm_widget_style_get(navi_it->base.widget));
}

EAPI const char *
elm_naviframe_item_style_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it, NULL);
   return navi_it->style;
}

EAPI void
elm_naviframe_item_title_visible_set(Elm_Object_Item *it, Eina_Bool visible)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it);

   visible = !!visible;
   if (navi_it->title_visible == visible) return;

   if (visible)
     edje_object_signal_emit(navi_it->base.view, "elm,state,title,show", "elm");
   else
     edje_object_signal_emit(navi_it->base.view, "elm,state,title,hide", "elm");

   navi_it->title_visible = visible;
}

EAPI Eina_Bool
elm_naviframe_item_title_visible_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   ELM_NAVIFRAME_ITEM_CHECK_RETURN(navi_it, EINA_FALSE);
   return navi_it->title_visible;
}

