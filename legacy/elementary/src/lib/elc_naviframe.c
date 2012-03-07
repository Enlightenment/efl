#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Naviframe_Item Elm_Naviframe_Item;
typedef struct _Elm_Naviframe_Content_Item_Pair Elm_Naviframe_Content_Item_Pair;
typedef struct _Elm_Naviframe_Text_Item_Pair Elm_Naviframe_Text_Item_Pair;

struct _Widget_Data
{
   Eina_Inlist  *stack;
   Evas_Object  *base;
   Eina_Bool     preserve: 1;
   Eina_Bool     auto_pushed: 1;
   Eina_Bool     freeze_events: 1;
};

struct _Elm_Naviframe_Content_Item_Pair
{
   EINA_INLIST;
   const char *part;
   Elm_Naviframe_Item *it;
};

struct _Elm_Naviframe_Text_Item_Pair
{
   EINA_INLIST;
   const char *part;
};

struct _Elm_Naviframe_Item
{
   ELM_WIDGET_ITEM;
   EINA_INLIST;
   Eina_Inlist       *content_list;
   Eina_Inlist       *text_list;
   Evas_Object       *content;
   Evas_Object       *title_prev_btn;
   Evas_Object       *title_next_btn;
   Evas_Object       *title_icon;
   const char        *style;
   Evas_Coord         minw;
   Evas_Coord         minh;
   Eina_Bool          back_btn: 1;
   Eina_Bool          title_visible: 1;
};

static const char *widtype = NULL;

//widget signals
static const char SIG_TRANSITION_FINISHED[] = "transition,finished";
static const char SIG_TITLE_CLICKED[] = "title,clicked";

//widget item signals
static const char SIG_ITEM_SHOW_BEGIN[] = "show,begin";
static const char SIG_ITEM_HIDE_FINISHED[] = "hide,finished";

static const Evas_Smart_Cb_Description _signals[] = {
       {SIG_TRANSITION_FINISHED, ""},
       {SIG_TITLE_CLICKED, ""},
       {SIG_ITEM_SHOW_BEGIN, ""},
       {SIG_ITEM_HIDE_FINISHED, ""},
       {NULL, NULL}
};

static void _text_set_hook(Evas_Object *obj,
                           const char *part,
                           const char *label);
static const char *_text_get_hook(const Evas_Object *obj, const char *part);
static void _content_set_hook(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content);
static Evas_Object *_content_get_hook(const Evas_Object *obj, const char *part);
static Evas_Object *_content_unset_hook(Evas_Object *obj, const char *part);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _emit_hook(Evas_Object *obj,
                       const char *emission,
                       const char *source);
static void _disable_hook(Evas_Object *obj);
static void _item_text_set_hook(Elm_Object_Item *it,
                                const char *part,
                                const char *label);
static const char *_item_text_get_hook(const Elm_Object_Item *it,
                                       const char *part);
static void _item_content_set_hook(Elm_Object_Item *it,
                                   const char *part,
                                   Evas_Object *content);
static Evas_Object *_item_content_get_hook(const Elm_Object_Item *it,
                                           const char *part);
static Evas_Object *_item_content_unset_hook(Elm_Object_Item *it,
                                             const char *part);
static void _item_signal_emit_hook(Elm_Object_Item *it,
                                   const char *emission,
                                   const char *source);
static void _item_title_visible_update(Elm_Naviframe_Item *navi_it);
static void _sizing_eval(Evas_Object *obj);
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
static void _item_content_del(void *data,
                              Evas *e,
                              Evas_Object *obj,
                              void *event_info);
static void _title_content_del(void *data,
                               Evas *e,
                               Evas_Object *obj,
                               void *event_info);
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
static void _title_content_set(Elm_Naviframe_Item *it,
                               const char *part,
                               Evas_Object *content);
static void _title_prev_btn_set(Elm_Naviframe_Item *it,
                                Evas_Object *btn,
                                Eina_Bool back_btn);
static void _title_next_btn_set(Elm_Naviframe_Item *it, Evas_Object *btn);
static void _title_icon_set(Elm_Naviframe_Item *it, Evas_Object *icon);
static Evas_Object * _item_content_unset(Elm_Naviframe_Item *it);
static Evas_Object * _title_prev_btn_unset(Elm_Naviframe_Item *it);
static Evas_Object * _title_next_btn_unset(Elm_Naviframe_Item *it);
static Evas_Object * _title_icon_unset(Elm_Naviframe_Item *it);
static Evas_Object * _title_content_unset(Elm_Naviframe_Item *it,
                                          const char *part);
static void _item_del(Elm_Naviframe_Item *it);
static Eina_Bool _item_del_pre_hook(Elm_Object_Item *it);
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
static void _changed_size_hints(void *data,
                                Evas *e,
                                Evas_Object *obj,
                                void *event_info);
static void _item_content_set(Elm_Naviframe_Item *navi_it,
                              Evas_Object *content);
static void _item_style_set(Elm_Naviframe_Item *navi_it,
                            const char *item_style);
static Elm_Naviframe_Item * _item_new(Evas_Object *obj,
                                      const char *title_label,
                                      Evas_Object *prev_btn,
                                      Evas_Object *next_btn,
                                      Evas_Object *content,
                                      const char *item_style);
static Eina_Bool _focus_next_hook(const Evas_Object *obj,
                                  Elm_Focus_Direction dir,
                                  Evas_Object **next);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   Elm_Naviframe_Item *it;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->stack)
     {
        while (wd->stack->last)
          {
             it = EINA_INLIST_CONTAINER_GET(wd->stack->last,
                                            Elm_Naviframe_Item);
             wd->stack = eina_inlist_remove(wd->stack, wd->stack->last);
             _item_del(it);
             elm_widget_item_free(it);
             if (!wd->stack) break;
          }
     }
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   Elm_Naviframe_Item *it;

   wd  = elm_widget_data_get(obj);
   if (!wd) return;

   _elm_theme_object_set(obj,
                         wd->base,
                         "naviframe",
                         "base",
                         elm_widget_style_get(obj));

   EINA_INLIST_FOREACH(wd->stack, it)
     {
        _item_style_set(it, it->style);
        _item_title_visible_update(it);
     }
}

static void _emit_hook(Evas_Object *obj,
                       const char *emission,
                       const char *source)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);
   if (!it) return;
   return elm_object_item_signal_emit(it, emission, source);
}

static void
_disable_hook(Evas_Object *obj __UNUSED__)
{
   //FIXME:
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Naviframe_Text_Item_Pair *pair = NULL;
   Elm_Naviframe_Item *navi_it = (Elm_Naviframe_Item *)it;
   char buf[1024];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), "elm.text.title");
   else if(!strcmp("subtitle", part))
     snprintf(buf, sizeof(buf), "elm.text.subtitle");
   else
     snprintf(buf, sizeof(buf), "%s", part);

   EINA_INLIST_FOREACH(navi_it->text_list, pair)
     if (!strcmp(buf, pair->part)) break;

   if (!pair)
     {
        pair = ELM_NEW(Elm_Naviframe_Text_Item_Pair);
        if (!pair)
          {
             ERR("Failed to allocate new text part of the item! : naviframe=%p",
             WIDGET(navi_it));
             return;
          }
        eina_stringshare_replace(&pair->part, buf);
        navi_it->text_list = eina_inlist_append(navi_it->text_list,
                                                EINA_INLIST_GET(pair));
     }
   elm_object_part_text_set(VIEW(navi_it), buf, label);

   if (label)
     {
        snprintf(buf, sizeof(buf), "elm,state,%s,show", buf);
        elm_object_signal_emit(VIEW(navi_it), buf, "elm");
     }
   else
     {
        snprintf(buf, sizeof(buf), "elm,state,%s,hide", buf);
        elm_object_signal_emit(VIEW(navi_it), buf, "elm");
     }

   _sizing_eval(WIDGET(navi_it));
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it, const char *part)
{
   char buf[1024];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), "elm.text.title");
   else if(!strcmp("subtitle", part))
     snprintf(buf, sizeof(buf), "elm.text.subtitle");
   else
     snprintf(buf, sizeof(buf), "%s", part);

   return elm_object_part_text_get(VIEW(it), buf);
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Naviframe_Item *navi_it = (Elm_Naviframe_Item *)it;

   //specified parts
   if (!part || !strcmp("default", part))
     {
        _item_content_set(navi_it, content);
        return;
     }
   else if (!strcmp(part, "prev_btn"))
     {
        _title_prev_btn_set(navi_it, content, EINA_FALSE);
        return;
     }
   else if (!strcmp(part, "next_btn"))
     {
        _title_next_btn_set(navi_it, content);
        return;
     }
   else if (!strcmp(part, "icon"))
     {
        _title_icon_set(navi_it, content);
        return;
     }

   //common part
   _title_content_set(navi_it, part, content);
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it, const char *part)
{
   Elm_Naviframe_Item *navi_it = (Elm_Naviframe_Item *)it;

   //specified parts
   if (!part || !strcmp("default", part))
     return navi_it->content;
   else if (!strcmp(part, "prev_btn"))
     return navi_it->title_prev_btn;
   else if (!strcmp(part, "next_btn"))
     return navi_it->title_next_btn;
   else if (!strcmp(part, "icon"))
     return navi_it->title_icon;

   //common parts
   return elm_object_part_content_get(VIEW(navi_it), part);
}

static Evas_Object *
_item_content_unset_hook(Elm_Object_Item *it, const char *part)
{
   Elm_Naviframe_Item *navi_it = (Elm_Naviframe_Item *)it;

   //specified parts
   if (!part || !strcmp("default", part))
     return _item_content_unset(navi_it);
   else if (!strcmp(part, "prev_btn"))
     return _title_prev_btn_unset(navi_it);
   else if (!strcmp(part, "next_btn"))
     return _title_next_btn_unset(navi_it);
   else if (!strcmp(part, "icon"))
     return _title_icon_unset(navi_it);

   return _title_content_unset(navi_it, part);
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   elm_object_signal_emit(VIEW(it), emission, source);
}

static void
_item_title_visible_update(Elm_Naviframe_Item *navi_it)
{
   if (navi_it->title_visible)
     elm_object_signal_emit(VIEW(navi_it), "elm,state,title,show", "elm");
   else
     elm_object_signal_emit(VIEW(navi_it), "elm,state,title,hide", "elm");
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   Elm_Naviframe_Item *it;
   Evas_Coord x, y, w, h;
   Evas_Coord minw = -1, minh = -1;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   EINA_INLIST_FOREACH(wd->stack, it)
     {
        evas_object_move(VIEW(it), x, y);
        evas_object_resize(VIEW(it), w, h);
        edje_object_size_min_calc(elm_layout_edje_get(VIEW(it)),
                                  &it->minw,
                                  &it->minh);
        if (it->minw > minw) minw = it->minw;
        if (it->minh > minh) minh = it->minh;
     }
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
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
   evas_object_smart_callback_call(WIDGET(it), SIG_TITLE_CLICKED, it);
}

static void
_back_btn_clicked(void *data,
                  Evas_Object *obj,
                  void *event_info __UNUSED__)
{
/* Since edje has the event queue, clicked event could be happend multiple times
   on some heavy environment. This callback del will prevent those  scenario and
   guarantee only one clicked for it's own page. */
   evas_object_smart_callback_del(obj, "clicked", _back_btn_clicked);
   elm_naviframe_item_pop(data);
}

static Evas_Object *
_back_btn_new(Evas_Object *obj)
{
   Evas_Object *btn;
   char buf[1024];
   btn = elm_button_add(obj);
   if (!btn) return NULL;
   evas_object_smart_callback_add(btn, "clicked", _back_btn_clicked, obj);
   snprintf(buf, sizeof(buf), "naviframe/back_btn/%s",
            elm_widget_style_get(obj));
   elm_object_style_set(btn, buf);
   return btn;
}

static void _text_set_hook(Evas_Object *obj,
                           const char *part,
                           const char *label)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);
   if (!it) return;
   elm_object_item_part_text_set(it, part, label);
}

static const char *_text_get_hook(const Evas_Object *obj, const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);
   if (!it) return NULL;
   return elm_object_item_part_text_get(it, part);
}

static void
_content_set_hook(Evas_Object *obj, const char *part, Evas_Object *content)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);
   if (!it) return;
   elm_object_item_part_content_set(it, part, content);
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);
   if (!it) return NULL;
   return elm_object_item_part_content_unset(it, part);
}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);
   if (!it) return NULL;
   return elm_object_item_part_content_get(it, part);
}

static void
_title_content_del(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   char buf[1024];
   Elm_Naviframe_Content_Item_Pair *pair = data;
   Elm_Naviframe_Item *it = pair->it;
   snprintf(buf, sizeof(buf), "elm,state,%s,hide", pair->part);
   elm_object_signal_emit(VIEW(it), buf, "elm");
   it->content_list = eina_inlist_remove(it->content_list,
                                         EINA_INLIST_GET(pair));
   eina_stringshare_del(pair->part);
   free(pair);
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
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");
}

static void
_title_next_btn_del(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->title_next_btn = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");
}

static void
_title_icon_del(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->title_icon = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");
}

static void
_item_content_del(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->content = NULL;
   elm_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");
}

static void
_title_content_set(Elm_Naviframe_Item *it,
                   const char *part,
                   Evas_Object *content)
{
   char buf[1024];
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   Evas_Object *prev_content = NULL;

   EINA_INLIST_FOREACH(it->content_list, pair)
     if (!strcmp(part, pair->part)) break;

   if (pair)
     {
        prev_content = elm_object_part_content_get(VIEW(it), part);
        if (prev_content != content)
          {
             if (content)
               {
                  evas_object_event_callback_del(prev_content,
                                                 EVAS_CALLBACK_DEL,
                                                 _title_content_del);
                  snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
                  elm_object_signal_emit(VIEW(it), buf, "elm");
               }
             evas_object_del(prev_content);
             if (!content) return;
          }
     }
   else
     {
        if (!content) return;
        pair = ELM_NEW(Elm_Naviframe_Content_Item_Pair);
        if (!pair)
          {
             ERR("Failed to allocate new content part of the item! : naviframe=%p", WIDGET(it));
             return;
          }
        pair->it = it;
        eina_stringshare_replace(&pair->part, part);
        it->content_list = eina_inlist_append(it->content_list,
                                              EINA_INLIST_GET(pair));
     }
   if (prev_content != content)
     {
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_DEL,
                                       _title_content_del,
                                       pair);
     }
   elm_object_part_content_set(VIEW(it), part, content);
   snprintf(buf, sizeof(buf), "elm,state,%s,show", part);
   elm_object_signal_emit(VIEW(it), buf, "elm");

   _sizing_eval(WIDGET(it));
}

static void
_title_prev_btn_set(Elm_Naviframe_Item *it,
                    Evas_Object *btn,
                    Eina_Bool back_btn)
{
   if (it->title_prev_btn == btn) return;
   if (it->title_prev_btn) evas_object_del(it->title_prev_btn);
   it->title_prev_btn = btn;
   if (!btn) return;

   evas_object_event_callback_add(btn,
                                  EVAS_CALLBACK_DEL,
                                  _title_prev_btn_del,
                                  it);
   elm_object_part_content_set(VIEW(it), "elm.swallow.prev_btn", btn);
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,show", "elm");
   it->back_btn = back_btn;

   _sizing_eval(WIDGET(it));
}

static void
_title_next_btn_set(Elm_Naviframe_Item *it, Evas_Object *btn)
{
   if (it->title_next_btn == btn) return;
   if (it->title_next_btn) evas_object_del(it->title_next_btn);
   it->title_next_btn = btn;
   if (!btn) return;

   evas_object_event_callback_add(btn,
                                  EVAS_CALLBACK_DEL,
                                  _title_next_btn_del,
                                  it);
   elm_object_part_content_set(VIEW(it), "elm.swallow.next_btn", btn);
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,show", "elm");

   _sizing_eval(WIDGET(it));
}

static void
_title_icon_set(Elm_Naviframe_Item *it, Evas_Object *icon)
{
   if (it->title_icon == icon) return;
   if (it->title_icon) evas_object_del(it->title_icon);
   it->title_icon = icon;
   if (!icon) return;

   evas_object_event_callback_add(icon,
                                  EVAS_CALLBACK_DEL,
                                  _title_icon_del,
                                  it);
   elm_object_part_content_set(VIEW(it), "elm.swallow.icon", icon);
   elm_object_signal_emit(VIEW(it), "elm,state,icon,show", "elm");

   _sizing_eval(WIDGET(it));
}

static Evas_Object *
_item_content_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->content;
   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), "elm.swallow.content");
   elm_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _item_content_del);
   _sizing_eval(WIDGET(it));

   it->content = NULL;
   return content;
}

static Evas_Object *
_title_prev_btn_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_prev_btn;
   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), "elm.swallow.prev_btn");
   elm_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_prev_btn_del);
   _sizing_eval(WIDGET(it));

   it->title_prev_btn = NULL;
   return content;
}

static Evas_Object *
_title_next_btn_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_next_btn;
   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), "elm.swallow.next_btn");
   elm_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_next_btn_del);
   _sizing_eval(WIDGET(it));

   it->title_next_btn = NULL;
   return content;
}

static Evas_Object *
_title_icon_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_icon;
   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), "elm.swallow.icon");
   elm_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_icon_del);
   _sizing_eval(WIDGET(it));
   it->title_icon = NULL;

   return content;
}

static Evas_Object *
_title_content_unset(Elm_Naviframe_Item *it, const char *part)
{
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   char buf[1028];
   Evas_Object *content = NULL;

   EINA_INLIST_FOREACH(it->content_list, pair)
     {
        if (!strcmp(part, pair->part))
          {
             content = elm_object_part_content_get(VIEW(it), part);
             eina_stringshare_del(pair->part);
             it->content_list = eina_inlist_remove(it->content_list,
                                                   EINA_INLIST_GET(pair));
             free(pair);
             break;
          }
     }

   if (!content) return NULL;

   elm_object_part_content_unset(VIEW(it), part);
   snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
   elm_object_signal_emit(VIEW(it), buf, "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_content_del);
   _sizing_eval(WIDGET(it));

   return content;
}

static void
_item_del(Elm_Naviframe_Item *it)
{
   Widget_Data *wd;
   Elm_Naviframe_Content_Item_Pair *content_pair;
   Elm_Naviframe_Text_Item_Pair *text_pair;
   Evas_Object *content;

   if (!it) return;

   wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return;

   while (it->content_list)
     {
        content_pair =
           EINA_INLIST_CONTAINER_GET(it->content_list,
                                     Elm_Naviframe_Content_Item_Pair);
        content = elm_object_part_content_get(VIEW(it), content_pair->part);
        evas_object_event_callback_del(content,
                                       EVAS_CALLBACK_DEL,
                                       _title_content_del);
        eina_stringshare_del(content_pair->part);
        it->content_list = eina_inlist_remove(it->content_list,
                                              it->content_list);
        free(content_pair);
     }

   while (it->text_list)
     {
        text_pair = EINA_INLIST_CONTAINER_GET(it->text_list,
                                              Elm_Naviframe_Text_Item_Pair);
        eina_stringshare_del(text_pair->part);
        it->text_list = eina_inlist_remove(it->text_list,
                                           it->text_list);
        free(text_pair);
     }

   eina_stringshare_del(it->style);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Naviframe_Item *navi_it;
   Widget_Data *wd;

   navi_it =(Elm_Naviframe_Item *)it;
   wd = elm_widget_data_get(WIDGET(navi_it));
   if (!wd) return EINA_FALSE;

   if (it == elm_naviframe_top_item_get(WIDGET(navi_it)))
     {
        wd->stack = eina_inlist_remove(wd->stack, EINA_INLIST_GET(navi_it));
        _item_del(navi_it);
        //If the item is only one, the stack will be empty
        if (!wd->stack) return EINA_TRUE;
        navi_it = EINA_INLIST_CONTAINER_GET(wd->stack->last,
                                            Elm_Naviframe_Item);
        evas_object_show(VIEW(navi_it));
        evas_object_raise(VIEW(navi_it));
        elm_object_signal_emit(VIEW(navi_it), "elm,state,visible", "elm");
     }
   else
     {
        wd->stack = eina_inlist_remove(wd->stack, EINA_INLIST_GET(navi_it));
        _item_del(navi_it);
     }

   return EINA_TRUE;
}

static void
_pushed_finished(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   Widget_Data *wd;
   Elm_Naviframe_Item *it = data;
   if (!it) return;

   wd = elm_widget_data_get(WIDGET(it));
   if (!wd) return;

   evas_object_hide(VIEW(it));
   if (wd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);

   elm_widget_item_smart_callback_call(it,
                                       SIG_ITEM_HIDE_FINISHED,
                                       NULL);
}

static void
_popped_finished(void *data,
                 Evas_Object *obj __UNUSED__,
                 const char *emission __UNUSED__,
                 const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   if (!it) return;
   _item_del(data);
   elm_widget_item_free(data);
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
   wd =  elm_widget_data_get(WIDGET(it));
   if (!wd) return;

   evas_object_smart_callback_call(WIDGET(it),
                                   SIG_TRANSITION_FINISHED,
                                   data);
   if (wd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);
}

static void
_changed_size_hints(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   _sizing_eval(data);
}

static void
_item_content_set(Elm_Naviframe_Item *navi_it, Evas_Object *content)
{
   if (navi_it->content == content) return;
   if (navi_it->content) evas_object_del(navi_it->content);
   elm_object_part_content_set(VIEW(navi_it), "elm.swallow.content", content);

   if (content)
     elm_object_signal_emit(VIEW(navi_it), "elm,state,content,show", "elm");
   else
     elm_object_signal_emit(VIEW(navi_it), "elm,state,content,hide", "elm");

   evas_object_event_callback_add(content,
                                  EVAS_CALLBACK_DEL,
                                  _item_content_del,
                                  navi_it);
   navi_it->content = content;
   _sizing_eval(WIDGET(navi_it));
}

//FIXME: need to handle if this function is called while transition
static void
_item_style_set(Elm_Naviframe_Item *navi_it, const char *item_style)
{
   Elm_Naviframe_Content_Item_Pair *content_pair;
   Elm_Naviframe_Text_Item_Pair *text_pair;
   Widget_Data *wd;

   char buf[256];

   if (!item_style)
     {
        strcpy(buf, "item/basic");
        eina_stringshare_replace(&navi_it->style, "basic");
     }
   else
     {
        snprintf(buf, sizeof(buf), "item/%s", item_style);
        eina_stringshare_replace(&navi_it->style, item_style);
     }
   elm_layout_theme_set(VIEW(navi_it),
                        "naviframe",
                        buf,
                        elm_widget_style_get(WIDGET(navi_it)));
   //recover item
   EINA_INLIST_FOREACH(navi_it->text_list, text_pair)
     _item_text_set_hook((Elm_Object_Item *) navi_it,
                         text_pair->part,
                         elm_object_part_text_get(VIEW(navi_it),
                                                  text_pair->part));

   EINA_INLIST_FOREACH(navi_it->content_list, content_pair)
     _item_content_set_hook((Elm_Object_Item *) navi_it,
                            content_pair->part,
                            elm_object_part_content_get(VIEW(navi_it),
                                                        content_pair->part));
   //content
   if (navi_it->content)
     elm_object_signal_emit(VIEW(navi_it), "elm,state,content,show", "elm");

   //prev button
   if (navi_it->title_prev_btn)
     elm_object_signal_emit(VIEW(navi_it), "elm,state,prev_btn,show", "elm");

   //next button
   if (navi_it->title_next_btn)
     elm_object_signal_emit(VIEW(navi_it), "elm,state,next_btn,show", "elm");

// why does this forcibly enable title? isnt that separate to style?   
//   navi_it->title_visible = EINA_TRUE;
   _sizing_eval(WIDGET(navi_it));

   wd = elm_widget_data_get(WIDGET(navi_it));
   if (!wd) return;

   if (wd->freeze_events)
     evas_object_freeze_events_set(VIEW(navi_it), EINA_FALSE);
}

static Elm_Naviframe_Item *
_item_new(Evas_Object *obj,
          const char *title_label,
          Evas_Object *prev_btn,
          Evas_Object *next_btn,
          Evas_Object *content,
          const char *item_style)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   //create item
   Elm_Naviframe_Item *it = elm_widget_item_new(obj, Elm_Naviframe_Item);
   if (!it)
     {
        ERR("Failed to allocate new item! : naviframe=%p", obj);
        return NULL;
     }

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);

   //item base layout
   VIEW(it) = elm_layout_add(obj);
   evas_object_smart_member_add(VIEW(it), wd->base);

   evas_object_event_callback_add(VIEW(it),
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints,
                                  obj);
   elm_object_signal_callback_add(VIEW(it),
                                   "elm,action,show,finished",
                                   "",
                                   _show_finished, it);
   elm_object_signal_callback_add(VIEW(it),
                                   "elm,action,pushed,finished",
                                   "",
                                   _pushed_finished, it);
   elm_object_signal_callback_add(VIEW(it),
                                   "elm,action,popped,finished",
                                   "",
                                   _popped_finished, it);
   elm_object_signal_callback_add(VIEW(it),
                                   "elm,action,title,clicked",
                                   "",
                                   _title_clicked, it);

   _item_style_set(it, item_style);
   _item_text_set_hook((Elm_Object_Item *)it, "elm.text.title", title_label);

   //title buttons
   if ((!prev_btn) && wd->auto_pushed && eina_inlist_count(wd->stack))
     {
        prev_btn = _back_btn_new(obj);
        _title_prev_btn_set(it, prev_btn, EINA_TRUE);
     }
   else
     _title_prev_btn_set(it, prev_btn, EINA_FALSE);

   _title_next_btn_set(it, next_btn);
   _item_content_set(it, content);
   it->title_visible = EINA_TRUE;
   return it;
}

static Eina_Bool
_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd || !wd->stack) return EINA_FALSE;
   return elm_widget_focus_next_get(VIEW(elm_naviframe_top_item_get(obj)),
                                    dir,
                                    next);
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
   elm_widget_text_set_hook_set(obj, _text_set_hook);
   elm_widget_text_get_hook_set(obj, _text_get_hook);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);
   elm_widget_signal_emit_hook_set(obj, _emit_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_focus_next_hook_set(obj, _focus_next_hook);

   //base
   //FIXME: Is this base layout really needed?
   wd->base = elm_layout_add(parent);
   evas_object_event_callback_add(wd->base,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _changed_size_hints,
                                  obj);
   elm_widget_resize_object_set(obj, wd->base);
   elm_layout_theme_set(wd->base, "naviframe", "base", "default");

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);
   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   wd->auto_pushed = EINA_TRUE;
   wd->freeze_events = EINA_TRUE;

   return obj;
}

EAPI Elm_Object_Item *
elm_naviframe_item_push(Evas_Object *obj,
                        const char *title_label,
                        Evas_Object *prev_btn,
                        Evas_Object *next_btn,
                        Evas_Object *content,
                        const char *item_style)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd;
   Elm_Naviframe_Item *prev_it, *it;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_new(obj, title_label, prev_btn, next_btn, content, item_style);
   if (!it) return NULL;

   evas_object_show(VIEW(it));

   prev_it = (Elm_Naviframe_Item *) elm_naviframe_top_item_get(obj);
   if (prev_it)
     {
        if (wd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
             evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
          }
        elm_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed", "elm");
        elm_object_signal_emit(VIEW(it), "elm,state,new,pushed", "elm");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
        elm_widget_tree_unfocusable_set(it->content, EINA_TRUE);
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));
     }
   wd->stack = eina_inlist_append(wd->stack, EINA_INLIST_GET(it));
   _sizing_eval(obj);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_naviframe_item_insert_before(Evas_Object *obj,
                                 Elm_Object_Item *before,
                                 const char *title_label,
                                 Evas_Object *prev_btn,
                                 Evas_Object *next_btn,
                                 Evas_Object *content,
                                 const char *item_style)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   ELM_OBJ_ITEM_CHECK_OR_RETURN(before, NULL);
   Elm_Naviframe_Item *it;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_new(WIDGET(before), title_label, prev_btn, next_btn, content,
                  item_style);
   if (!it) return NULL;

   wd->stack =
      eina_inlist_prepend_relative(wd->stack,
                                   EINA_INLIST_GET(it),
                                   EINA_INLIST_GET(((Elm_Naviframe_Item *) before)));
   _sizing_eval(WIDGET(before));
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_naviframe_item_insert_after(Evas_Object *obj,
                                Elm_Object_Item *after,
                                const char *title_label,
                                Evas_Object *prev_btn,
                                Evas_Object *next_btn,
                                Evas_Object *content,
                                const char *item_style)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   ELM_OBJ_ITEM_CHECK_OR_RETURN(after, NULL);
   Elm_Naviframe_Item *it;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = _item_new(WIDGET(after), title_label, prev_btn, next_btn, content,
                  item_style);
   if (!it) return NULL;

   if (elm_naviframe_top_item_get(WIDGET(after)) == after)
     {
        evas_object_hide(VIEW(after));
        evas_object_show(VIEW(it));
     }
   wd->stack =
      eina_inlist_append_relative(wd->stack,
                                  EINA_INLIST_GET(it),
                                  EINA_INLIST_GET(((Elm_Naviframe_Item *) after)));
   _sizing_eval(WIDGET(after));
   return (Elm_Object_Item *)it;
}

EAPI Evas_Object *
elm_naviframe_item_pop(Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Elm_Naviframe_Item *it, *prev_it = NULL;
   Widget_Data *wd;
   Evas_Object *content = NULL;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   it = (Elm_Naviframe_Item *) elm_naviframe_top_item_get(obj);
   if (!it) return NULL;
   if (wd->preserve)
     content = it->content;

   if (wd->stack->last->prev)
     prev_it = EINA_INLIST_CONTAINER_GET(wd->stack->last->prev,
                                         Elm_Naviframe_Item);
   wd->stack = eina_inlist_remove(wd->stack, EINA_INLIST_GET(it));
   if (prev_it)
     {
        if (wd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
             evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
          }
        elm_object_signal_emit(VIEW(it), "elm,state,cur,popped", "elm");
        evas_object_show(VIEW(prev_it));
        evas_object_raise(VIEW(prev_it));
        elm_object_signal_emit(VIEW(prev_it),
                               "elm,state,prev,popped",
                               "elm");
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(it)));
        edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
        elm_widget_item_smart_callback_call(prev_it,
                                            SIG_ITEM_SHOW_BEGIN,
                                            NULL);
     }
   else
     {
        _item_del(it);
        elm_widget_item_free(it);
     }

   return content;
}

EAPI void
elm_naviframe_item_pop_to(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it;
   Widget_Data *wd;
   Eina_Inlist *l, *prev_l;

   navi_it = (Elm_Naviframe_Item *)it;
   wd = elm_widget_data_get(WIDGET(navi_it));
   if (!wd) return;

   if (it == elm_naviframe_top_item_get(WIDGET(navi_it))) return;

   l = wd->stack->last->prev;

   while(l)
     {
        if (EINA_INLIST_CONTAINER_GET(l, Elm_Naviframe_Item) ==
            ((Elm_Naviframe_Item *)it)) break;
        prev_l = l->prev;
        wd->stack = eina_inlist_remove(wd->stack, l);
        _item_del(EINA_INLIST_CONTAINER_GET(l, Elm_Naviframe_Item));
        elm_widget_item_free(EINA_INLIST_CONTAINER_GET(l,
                                                       Elm_Naviframe_Item));
        l = prev_l;
     }
   elm_naviframe_item_pop(WIDGET(navi_it));
}

EAPI void
elm_naviframe_item_promote(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it;
   Elm_Naviframe_Item *prev_it;
   Widget_Data *wd;

   navi_it = (Elm_Naviframe_Item *)it;
   wd = elm_widget_data_get(navi_it->base.widget);
   if (!wd) return;

   if (it == elm_naviframe_top_item_get(navi_it->base.widget)) return;
   elm_widget_tree_unfocusable_set(navi_it->content, EINA_FALSE);
   wd->stack = eina_inlist_demote(wd->stack, EINA_INLIST_GET(navi_it));
   prev_it = EINA_INLIST_CONTAINER_GET(wd->stack->last->prev,
                                         Elm_Naviframe_Item);
   elm_widget_tree_unfocusable_set(prev_it->content, EINA_FALSE);
   if (wd->freeze_events)
     {
        evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
        evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
     }
   elm_object_signal_emit(VIEW(prev_it),
                          "elm,state,cur,pushed",
                          "elm");
   evas_object_show(VIEW(navi_it));
   evas_object_raise(VIEW(navi_it));
   elm_object_signal_emit(VIEW(navi_it),
                          "elm,state,new,pushed",
                          "elm");
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(prev_it)));
   edje_object_message_signal_process(elm_layout_edje_get(VIEW(navi_it)));
}

EAPI void
elm_naviframe_item_simple_promote(Evas_Object *obj, Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   Elm_Naviframe_Item *itr;
   EINA_INLIST_FOREACH(wd->stack, itr)
     {
        if (elm_object_item_content_get((Elm_Object_Item *)itr) == content)
          {
             elm_naviframe_item_promote((Elm_Object_Item *)itr);
             break;
          }
     }
}


EAPI void
elm_naviframe_item_del(Elm_Object_Item *it)
{
   elm_object_item_del(it);
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

EAPI Elm_Object_Item*
elm_naviframe_top_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->stack)) return NULL;
   return (Elm_Object_Item *) (EINA_INLIST_CONTAINER_GET(wd->stack->last,
                                                         Elm_Naviframe_Item));
}

EAPI Elm_Object_Item*
elm_naviframe_bottom_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->stack)) return NULL;
   return (Elm_Object_Item *) (EINA_INLIST_CONTAINER_GET(wd->stack,
                                                         Elm_Naviframe_Item));
}

EAPI void
elm_naviframe_item_style_set(Elm_Object_Item *it, const char *item_style)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it = (Elm_Naviframe_Item *)it;

   if (item_style)
     if (!strcmp(item_style, navi_it->style)) return;

   if (!item_style)
     if (!strcmp("basic", navi_it->style)) return;

   _item_style_set(navi_it, item_style);
}

EAPI const char *
elm_naviframe_item_style_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Naviframe_Item *navi_it = (Elm_Naviframe_Item *)it;
   return navi_it->style;
}

EAPI void
elm_naviframe_item_title_visible_set(Elm_Object_Item *it, Eina_Bool visible)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it = (Elm_Naviframe_Item *)it;

   visible = !!visible;
   if (navi_it->title_visible == visible) return;

   navi_it->title_visible = visible;
   _item_title_visible_update(navi_it);
}

EAPI Eina_Bool
elm_naviframe_item_title_visible_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Elm_Naviframe_Item *navi_it = (Elm_Naviframe_Item *)it;
   return navi_it->title_visible;
}

EAPI void
elm_naviframe_prev_btn_auto_pushed_set(Evas_Object *obj, Eina_Bool auto_pushed)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   wd->auto_pushed = !!auto_pushed;
}

EAPI Eina_Bool
elm_naviframe_prev_btn_auto_pushed_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return wd->auto_pushed;
}

EAPI Eina_List *
elm_naviframe_items_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   Eina_List *ret = NULL;
   Elm_Naviframe_Item *itr;
   EINA_INLIST_FOREACH(wd->stack, itr)
     ret = eina_list_append(ret, itr);
   return ret;
}

EAPI void
elm_naviframe_event_enabled_set(Evas_Object *obj, Eina_Bool enabled)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   enabled = !!enabled;
   if (wd->freeze_events == !enabled) return;
   wd->freeze_events = !enabled;
}

EAPI Eina_Bool
elm_naviframe_event_enabled_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;
   return !wd->freeze_events;
}
