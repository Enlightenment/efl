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
   Evas_Object  *rect;
   Eina_Bool     preserve: 1;
   Eina_Bool     auto_pushed: 1;
   Eina_Bool     freeze_events: 1;
};

struct _Elm_Naviframe_Content_Item_Pair
{
   EINA_INLIST;
   const char *part;
   Evas_Object *content;
   Elm_Naviframe_Item *it;
};

struct _Elm_Naviframe_Text_Item_Pair
{
   EINA_INLIST;
   const char *part;
   const char *text;
};

struct _Elm_Naviframe_Item
{
   Elm_Widget_Item    base;
   EINA_INLIST;
   Eina_Inlist       *content_list;
   Eina_Inlist       *text_list;
   Evas_Object       *content;
   Evas_Object       *title_prev_btn;
   Evas_Object       *title_next_btn;
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

static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _emit_hook(Evas_Object *obj,
                       const char *emission,
                       const char *source);
static void _disable_hook(Evas_Object *obj);
static void _mirrored_set(Evas_Object *obj, Eina_Bool rtl);
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
static void _sizing_eval(Evas_Object *obj);
static void _item_sizing_eval(Elm_Naviframe_Item *it);
static void _move(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _resize(void *data,
                    Evas *e,
                    Evas_Object *obj,
                    void *event_info);
static void _hide(void *data, Evas *e, Evas_Object *obj, void *event_info);
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
static void _title_content_set(Elm_Naviframe_Item *it,
                               Elm_Naviframe_Content_Item_Pair *pair,
                               const char *part,
                               Evas_Object *content);
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
static void _item_content_set(Elm_Naviframe_Item *navi_it,
                              Evas_Object *content);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   Elm_Naviframe_Item *it;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   EINA_INLIST_REVERSE_FOREACH(wd->stack, it)
     _item_del(it);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   //FIXME:
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
}

static void _emit_hook(Evas_Object *obj,
                       const char *emission,
                       const char *source)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   edje_object_signal_emit(wd->base, emission, source);
}

static void
_disable_hook(Evas_Object *obj __UNUSED__)
{
   //FIXME:
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd;
   Elm_Naviframe_Item *it;

   wd  = elm_widget_data_get(obj);
   if (!wd) return;

   EINA_INLIST_FOREACH(wd->stack, it)
     edje_object_mirrored_set(it->base.view, rtl);
   edje_object_mirrored_set(wd->base, rtl);
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);

   Elm_Naviframe_Text_Item_Pair *pair = NULL;
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   char buf[1024];

   if (!part)
     snprintf(buf, sizeof(buf), "elm.text.title");
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
             navi_it->base.widget);
             return;
          }
        eina_stringshare_replace(&pair->part, buf);
        navi_it->text_list = eina_inlist_append(navi_it->text_list,
                                                EINA_INLIST_GET(pair));
     }

   eina_stringshare_replace(&pair->text, label);
   edje_object_part_text_set(navi_it->base.view, buf, label);

   if (label)
     {
        snprintf(buf, sizeof(buf), "elm,state,%s,show", buf);
        edje_object_signal_emit(navi_it->base.view, buf, "elm");
     }
   else
     {
        snprintf(buf, sizeof(buf), "elm,state,%s,hide", buf);
        edje_object_signal_emit(navi_it->base.view, buf, "elm");
     }

   _item_sizing_eval(navi_it);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it, const char *part)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Naviframe_Text_Item_Pair *pair = NULL;
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   char buf[1024];

   if (!part)
     snprintf(buf, sizeof(buf), "elm.text.title");
   else
     snprintf(buf, sizeof(buf), "%s", part);

   EINA_INLIST_FOREACH(navi_it->text_list, pair)
     {
        if (!strcmp(buf, pair->part))
          return pair->text;
     }
   return NULL;
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);

   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);

   //specified parts
   if ((!part) || (!strcmp(part, "elm.swallow.content")))
     {
        _item_content_set(navi_it, content);
        return;
     }
   else if (!strcmp(part, "elm.swallow.prev_btn"))
     {
        _title_prev_btn_set(navi_it, content, EINA_FALSE);
        return;
     }
   else if(!strcmp(part, "elm.swallow.next_btn"))
     {
        _title_next_btn_set(navi_it, content);
        return;
     }

   //common part
   _title_content_set(navi_it, pair, part, content);
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it, const char *part)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);

   //specified parts
   if ((!part) || (!strcmp(part, "elm.swallow.content")))
     return navi_it->content;
   else if (!strcmp(part, "elm.swallow.prev_btn"))
     return navi_it->title_prev_btn;
   else if(!strcmp(part, "elm.swallow.next_btn"))
     return navi_it->title_next_btn;

   //common parts
   EINA_INLIST_FOREACH(navi_it->content_list, pair)
     {
        if (!strcmp(part, pair->part))
          return pair->content;
     }
   return NULL;
}

static Evas_Object *
_item_content_unset_hook(Elm_Object_Item *it, const char *part)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   Evas_Object *content = NULL;
   char buf[1028];

   //specified parts
   //FIXME: could be unset the below specified contents also.
   if (!part ||
       !strcmp(part, "elm.swallow.content") ||
       !strcmp(part, "elm.swallow.prev_btn") ||
       !strcmp(part, "elm.swallow.next_btn"))
     {
        WRN("You can not unset the content! : naviframe=%p",
            navi_it->base.widget);
        return NULL;
     }

   //common parts
   EINA_INLIST_FOREACH(navi_it->content_list, pair)
     {
        if (!strcmp(part, pair->part))
          {
             content = pair->content;
             eina_stringshare_del(pair->part);
             navi_it->content_list = eina_inlist_remove(navi_it->content_list,
                                                        EINA_INLIST_GET(pair));
             free(pair);
             break;
          }
     }

   if (!content) return NULL;

   elm_widget_sub_object_del(navi_it->base.widget, content);
   edje_object_part_unswallow(navi_it->base.view, content);
   snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
   edje_object_signal_emit(navi_it->base.view, buf, "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_content_del);
   _item_sizing_eval(navi_it);

   return content;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   edje_object_signal_emit(navi_it->base.view, emission, source);
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   Elm_Naviframe_Item *it;
   wd  = elm_widget_data_get(obj);
   if (!wd) return;

   EINA_INLIST_FOREACH(wd->stack, it)
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
   Evas_Coord x, y;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_move(wd->rect, x, y);

   _sizing_eval(obj);
}

static void
_resize(void *data __UNUSED__,
        Evas *e __UNUSED__,
        Evas_Object *obj,
        void *event_info __UNUSED__)
{
   Evas_Coord w, h;
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(wd->rect, w, h);

   _sizing_eval(obj);
}

static void
_hide(void *data __UNUSED__,
      Evas *e __UNUSED__,
      Evas_Object *obj,
      void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   if (wd->freeze_events)
     evas_object_hide(wd->rect);
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
   char buf[1024];
   btn = elm_button_add(obj);
   if (!btn) return NULL;
   evas_object_smart_callback_add(btn, "clicked", _back_btn_clicked, obj);
   snprintf(buf, sizeof(buf), "naviframe/back_btn/%s", elm_widget_style_get(obj));
   elm_object_style_set(btn, buf);
   return btn;
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
   edje_object_signal_emit(it->base.view, buf, "elm");
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
   edje_object_signal_emit(it->base.view, "elm,state,prev_btn,hide", "elm");
}

static void
_title_next_btn_del(void *data,
                    Evas *e __UNUSED__,
                    Evas_Object *obj __UNUSED__,
                    void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->title_next_btn = NULL;
   edje_object_signal_emit(it->base.view, "elm,state,next_btn,hide", "elm");
}

static void
_item_content_del(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;
   it->content = NULL;
   edje_object_signal_emit(it->base.view, "elm,state,content,hide", "elm");
}

static void
_title_content_set(Elm_Naviframe_Item *it,
                   Elm_Naviframe_Content_Item_Pair *pair,
                   const char *part,
                   Evas_Object *content)
{
   char buf[1024];

   EINA_INLIST_FOREACH(it->content_list, pair)
     if (!strcmp(part, pair->part)) break;

   if (!pair)
     {
        pair = ELM_NEW(Elm_Naviframe_Content_Item_Pair);
        if (!pair)
          {
             ERR("Failed to allocate new content part of the item! : naviframe=%p", it->base.widget);
             return;
          }
        pair->it = it;
        eina_stringshare_replace(&pair->part, part);
        it->content_list = eina_inlist_append(it->content_list,
                                              EINA_INLIST_GET(pair));
     }

   if ((pair->content) && (pair->content != content))
     evas_object_del(pair->content);

   if (!content)
     {
        snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
        edje_object_signal_emit(it->base.view, buf, "elm");
        pair->content = NULL;
        return;
     }

   if (pair->content != content)
     {
        elm_widget_sub_object_add(it->base.widget, content);
        evas_object_event_callback_add(content,
                                       EVAS_CALLBACK_DEL,
                                       _title_content_del,
                                       pair);
     }

   pair->content = content;

   edje_object_part_swallow(it->base.view, part, content);
   snprintf(buf, sizeof(buf), "elm,state,%s,show", part);
   edje_object_signal_emit(it->base.view, buf, "elm");
   _item_sizing_eval(it);
}

static void
_title_prev_btn_set(Elm_Naviframe_Item *it,
                    Evas_Object *btn,
                    Eina_Bool back_btn)
{
   if (it->title_prev_btn == btn) return;

   if (it->title_prev_btn)
     evas_object_del(it->title_prev_btn);

   it->title_prev_btn = btn;

   if (!btn)
     {
        edje_object_signal_emit(it->base.view,
                                "elm,state,prev_btn,hide",
                                "elm");
        return;
     }

   elm_widget_sub_object_add(it->base.widget, btn);
   evas_object_event_callback_add(btn,
                                  EVAS_CALLBACK_DEL,
                                  _title_prev_btn_del,
                                  it);
   edje_object_part_swallow(it->base.view, "elm.swallow.prev_btn", btn);
   edje_object_signal_emit(it->base.view, "elm,state,prev_btn,show", "elm");
   it->back_btn = back_btn;

   _item_sizing_eval(it);
}

static void
_title_next_btn_set(Elm_Naviframe_Item *it, Evas_Object *btn)
{
   if (it->title_next_btn == btn) return;

   if (it->title_next_btn)
     evas_object_del(it->title_next_btn);

   it->title_next_btn = btn;

   if (!btn)
     {
        edje_object_signal_emit(it->base.view,
                                "elm,state,next_btn,hide",
                                "elm");
        return;
     }

   elm_widget_sub_object_add(it->base.widget, btn);
   evas_object_event_callback_add(btn,
                                  EVAS_CALLBACK_DEL,
                                  _title_next_btn_del,
                                  it);
   edje_object_part_swallow(it->base.view, "elm.swallow.next_btn", btn);
   edje_object_signal_emit(it->base.view, "elm,state,next_btn,show", "elm");

   _item_sizing_eval(it);
}

static void
_item_del(Elm_Naviframe_Item *it)
{
   Widget_Data *wd;
   Elm_Naviframe_Content_Item_Pair *content_pair;
   Elm_Naviframe_Text_Item_Pair *text_pair;
   Eina_Inlist *l;

   if (!it) return;

   wd = elm_widget_data_get(it->base.widget);
   if (!wd) return;

   if (it->title_prev_btn)
     evas_object_del(it->title_prev_btn);
   if (it->title_next_btn)
     evas_object_del(it->title_next_btn);
   if ((it->content) && (!wd->preserve))
     evas_object_del(it->content);

   EINA_INLIST_FOREACH_SAFE(it->content_list, l, content_pair)
     {
        evas_object_event_callback_del(content_pair->content,
                                       EVAS_CALLBACK_DEL,
                                       _title_content_del);
        evas_object_del(content_pair->content);
        eina_stringshare_del(content_pair->part);
        it->content_list =
           eina_inlist_remove(it->content_list,
                              EINA_INLIST_GET(content_pair));
        free(content_pair);
     }

   EINA_INLIST_FOREACH_SAFE(it->text_list, l, text_pair)
     {
        eina_stringshare_del(text_pair->part);
        eina_stringshare_del(text_pair->text);
        it->text_list = eina_inlist_remove(it->text_list,
                                           EINA_INLIST_GET(text_pair));
        free(text_pair);
     }

   wd->stack = eina_inlist_remove(wd->stack, EINA_INLIST_GET(it));

   elm_widget_item_del(it);
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
   if (wd->freeze_events)
     evas_object_hide(wd->rect);
}

static void
_item_content_set(Elm_Naviframe_Item *navi_it, Evas_Object *content)
{
   if (navi_it->content == content) return;
   if (navi_it->content) evas_object_del(navi_it->content);
   elm_widget_sub_object_add(navi_it->base.widget, content);
   edje_object_part_swallow(navi_it->base.view,
                            "elm.swallow.content",
                            content);
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
                                  _item_content_del,
                                  navi_it);
   navi_it->content = content;
   _item_sizing_eval(navi_it);
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
   elm_widget_signal_emit_hook_set(obj, _emit_hook);

   //base
   wd->base = edje_object_add(e);
   edje_object_mirrored_set(wd->base, elm_widget_mirrored_get(obj));
   elm_widget_resize_object_set(obj, wd->base);
   _elm_theme_object_set(obj, wd->base, "naviframe", "base", "default");

   //rect
   wd->rect = evas_object_rectangle_add(e);
   evas_object_color_set(wd->rect, 0, 0, 0, 0);
   elm_widget_sub_object_add(obj, wd->rect);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _hide, obj);
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

   //create item
   it = elm_widget_item_new(obj, Elm_Naviframe_Item);
   if (!it)
     {
        ERR("Failed to allocate new item! : naviframe=%p", obj);
        return NULL;
     }

   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_signal_emit_hook_set(it, _item_signal_emit_hook);

   //item base layout
   it->base.view = edje_object_add(evas_object_evas_get(obj));
   edje_object_mirrored_set(it->base.view, elm_widget_mirrored_get(obj));
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
   edje_object_signal_callback_add(it->base.view,
                                   "elm,action,title,clicked",
                                   "",
                                   _title_clicked, it);

   elm_naviframe_item_style_set(ELM_CAST(it), item_style);

   _item_text_set_hook(ELM_CAST(it), "elm.text.title", title_label);

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

   _item_sizing_eval(it);
   evas_object_show(it->base.view);

   prev_it = ELM_CAST(elm_naviframe_top_item_get(obj));
   if (prev_it)
     {
        if (wd->freeze_events)
          evas_object_show(wd->rect);
        edje_object_signal_emit(prev_it->base.view,
                                "elm,state,cur,pushed",
                                "elm");
        edje_object_signal_emit(it->base.view,
                                "elm,state,new,pushed",
                                "elm");
     }
   it->title_visible = EINA_TRUE;
   wd->stack = eina_inlist_append(wd->stack, EINA_INLIST_GET(it));
   return ELM_CAST(it);
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

   it = ELM_CAST(elm_naviframe_top_item_get(obj));
   if (!it) return NULL;
   if (wd->preserve)
     content = it->content;

   if (wd->stack->last->prev)
     prev_it = EINA_INLIST_CONTAINER_GET(wd->stack->last->prev,
                                         Elm_Naviframe_Item);
   if (prev_it)
     {
        if (wd->freeze_events)
          evas_object_show(wd->rect);
        edje_object_signal_emit(it->base.view, "elm,state,cur,popped", "elm");
        evas_object_show(prev_it->base.view);
        evas_object_raise(prev_it->base.view);
        edje_object_signal_emit(prev_it->base.view,
                                "elm,state,prev,popped",
                                "elm");
     }
   else
     _item_del(it);

   return content;
}

EAPI void
elm_naviframe_item_pop_to(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   Widget_Data *wd = elm_widget_data_get(navi_it->base.widget);
   Eina_Inlist *l, *prev_l;

   if (it == elm_naviframe_top_item_get(navi_it->base.widget)) return;

   l = wd->stack->last->prev;

   while(l)
     {
        if (ELM_CAST(EINA_INLIST_CONTAINER_GET(l, Elm_Naviframe_Item)) == it)
          break;
        prev_l = l->prev;
        _item_del(EINA_INLIST_CONTAINER_GET(l, Elm_Naviframe_Item));
        l = prev_l;
     }
   elm_naviframe_item_pop(navi_it->base.widget);
}

EAPI void
elm_naviframe_item_del(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   Widget_Data *wd = elm_widget_data_get(navi_it->base.widget);
   if (it == elm_naviframe_top_item_get(navi_it->base.widget))
     {
        _item_del(navi_it);
        navi_it = ELM_CAST(EINA_INLIST_CONTAINER_GET(wd->stack->last, Elm_Naviframe_Item));
        evas_object_show(navi_it->base.view);
        evas_object_raise(navi_it->base.view);
        edje_object_signal_emit(navi_it->base.view, "elm,state,visible", "elm");
     }
   else
     _item_del(navi_it);
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
   return ELM_CAST(EINA_INLIST_CONTAINER_GET(wd->stack->last,
                                             Elm_Naviframe_Item));
}

EAPI Elm_Object_Item*
elm_naviframe_bottom_item_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->stack)) return NULL;
   return ELM_CAST(EINA_INLIST_CONTAINER_GET(wd->stack,
                                             Elm_Naviframe_Item));
}

EAPI void
elm_naviframe_item_style_set(Elm_Object_Item *it, const char *item_style)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   Elm_Naviframe_Content_Item_Pair *content_pair;
   Elm_Naviframe_Text_Item_Pair *text_pair;
   Widget_Data *wd;

   char buf[256];

   if (!item_style) sprintf(buf, "item/basic");
   else
     {
        if (strlen(item_style) > sizeof(buf))
          WRN("too much long style name! : naviframe=%p", navi_it->base.widget);
        sprintf(buf, "item/%s", item_style);
     }
   _elm_theme_object_set(navi_it->base.widget,
                         navi_it->base.view,
                         "naviframe",
                         buf,
                         elm_widget_style_get(navi_it->base.widget));
   //recover item
   EINA_INLIST_FOREACH(navi_it->text_list, text_pair)
     _item_text_set_hook(it, text_pair->part, text_pair->text);

   EINA_INLIST_FOREACH(navi_it->content_list, content_pair)
     _item_content_set_hook(it, content_pair->part, content_pair->content);

   //content
   if (navi_it->content)
     {
        edje_object_part_swallow(navi_it->base.view,
                                 "elm.swallow.content",
                                 navi_it->content);
        edje_object_signal_emit(navi_it->base.view,
                                "elm,state,content,show",
                                "elm");
     }

   //prev button
   if (navi_it->title_prev_btn)
     edje_object_part_swallow(navi_it->base.view,
                              "elm.swallow.prev_btn",
                              navi_it->title_prev_btn);

   //next button
   if (navi_it->title_next_btn)
     edje_object_part_swallow(navi_it->base.view,
                              "elm.swallow.next_btn",
                              navi_it->title_next_btn);

   navi_it->title_visible = EINA_TRUE;
   _item_sizing_eval(navi_it);

   wd = elm_widget_data_get(navi_it->base.widget);
   if (wd && wd->freeze_events)
     evas_object_hide(wd->rect);
}

EAPI const char *
elm_naviframe_item_style_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
   return navi_it->style;
}

EAPI void
elm_naviframe_item_title_visible_set(Elm_Object_Item *it, Eina_Bool visible)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);

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
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Elm_Naviframe_Item *navi_it = ELM_CAST(it);
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

