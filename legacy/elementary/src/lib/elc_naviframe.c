#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_naviframe.h"

EAPI const char ELM_NAVIFRAME_SMART_NAME[] = "elm_naviframe";

static const char CONTENT_PART[] = "elm.swallow.content";
static const char PREV_BTN_PART[] = "elm.swallow.prev_btn";
static const char NEXT_BTN_PART[] = "elm.swallow.next_btn";
static const char ICON_PART[] = "elm.swallow.icon";

static const char SIG_TRANSITION_FINISHED[] = "transition,finished";
static const char SIG_TITLE_CLICKED[] = "title,clicked";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_TRANSITION_FINISHED, ""},
   {SIG_TITLE_CLICKED, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_NAVIFRAME_SMART_NAME, _elm_naviframe, Elm_Naviframe_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_item_content_del_cb(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->content = NULL;
   edje_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");
}

static void
_item_title_prev_btn_del_cb(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_prev_btn = NULL;
   edje_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");
}

static void
_item_title_next_btn_del_cb(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_next_btn = NULL;
   edje_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");
}

static void
_item_title_icon_del_cb(void *data,
                        Evas *e __UNUSED__,
                        Evas_Object *obj __UNUSED__,
                        void *event_info __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   it->title_icon = NULL;
   edje_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");
}

static void
_item_free(Elm_Naviframe_Item *it)
{
   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   eina_stringshare_del(it->style);
   eina_stringshare_del(it->title_label);

   if (it->title_prev_btn)
     evas_object_del(it->title_prev_btn);
   if (it->title_next_btn)
     evas_object_del(it->title_next_btn);
   if (it->title_icon) evas_object_del(it->title_icon);

   if (sd->preserve && it->content)
     {
        /* so that elm does not delete the contents with the item's
         * view after the del_pre_hook */
        edje_object_part_unswallow(VIEW(it), it->content);
        evas_object_event_callback_del
          (it->content, EVAS_CALLBACK_DEL, _item_content_del_cb);
     }
   else if (it->content)
     evas_object_del(it->content);
}

static void
_item_content_signals_emit(Elm_Naviframe_Item *it)
{
   Elm_Naviframe_Content_Item_Pair *content_pair;
   char buf[1024];
   //content
   if (it->content)
     edje_object_signal_emit(VIEW(it), "elm,state,content,show", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,content,hide", "elm");

   //prev button
   if (it->title_prev_btn)
     edje_object_signal_emit(VIEW(it), "elm,state,prev_btn,show", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,prev_btn,hide", "elm");

   //next button
   if (it->title_next_btn)
     edje_object_signal_emit(VIEW(it), "elm,state,next_btn,show", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,next_btn,hide", "elm");

   if (it->title_icon)
     edje_object_signal_emit(VIEW(it), "elm,state,icon,show", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,icon,hide", "elm");

   EINA_INLIST_FOREACH(it->content_list, content_pair)
     {
        if (edje_object_part_swallow_get(VIEW(it), content_pair->part))
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,show", content_pair->part);
             edje_object_signal_emit(VIEW(it), buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,hide", content_pair->part);
             edje_object_signal_emit(VIEW(it), buf, "elm");
          }
     }
}

static void
_item_text_signals_emit(Elm_Naviframe_Item *it)
{
   Elm_Naviframe_Text_Item_Pair *text_pair;
   char buf[1024];

   if ((it->title_label) && (it->title_label[0]))
     edje_object_signal_emit(VIEW(it), "elm,state,title_label,show", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,title_label,hide", "elm");

   if ((it->subtitle_label) && (it->subtitle_label[0]))
     edje_object_signal_emit(VIEW(it), "elm,state,subtitle,show", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,subtitle,hide", "elm");

   EINA_INLIST_FOREACH(it->text_list, text_pair)
     {
        if (edje_object_part_text_get(VIEW(it), text_pair->part))
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,show", text_pair->part);
             edje_object_signal_emit(VIEW(it), buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,hide", text_pair->part);
             edje_object_signal_emit(VIEW(it), buf, "elm");
          }
     }
}

/* FIXME: we need to handle the case when this function is called
 * during a transition */
static void
_item_style_set(Elm_Naviframe_Item *it,
                const char *item_style)
{
   char buf[256];

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   if (!item_style)
     {
        strcpy(buf, "item/basic");
        eina_stringshare_replace(&it->style, "basic");
     }
   else
     {
        snprintf(buf, sizeof(buf), "item/%s", item_style);
        eina_stringshare_replace(&it->style, item_style);
     }

   elm_widget_theme_object_set
     (WIDGET(it), VIEW(it), "naviframe", buf, elm_widget_style_get(WIDGET(it)));

   _item_text_signals_emit(it);
   _item_content_signals_emit(it);

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);
}

static void
_item_title_visible_update(Elm_Naviframe_Item *nit)
{
   if (nit->title_visible)
     edje_object_signal_emit(VIEW(nit), "elm,state,title,show", "elm");
   else
     edje_object_signal_emit(VIEW(nit), "elm,state,title,hide", "elm");
}

static Eina_Bool
_elm_naviframe_smart_theme(Evas_Object *obj)
{
   Elm_Naviframe_Item *it;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_naviframe_parent_sc)->theme(obj))
     return EINA_FALSE;

   EINA_INLIST_FOREACH (sd->stack, it)
     {
        _item_style_set(it, it->style);
        _item_title_visible_update(it);
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;
   Elm_Naviframe_Text_Item_Pair *pair = NULL;
   char buf[1024];

   if ((!part) || (!strcmp(part, "default")) ||
       (!strcmp(part, "elm.text.title")))
     {
        eina_stringshare_replace(&nit->title_label, label);
        snprintf(buf, sizeof(buf), "elm.text.title");

        //XXX: ACCESS
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          {
             nit->title = _elm_access_edje_object_part_object_register
                          (WIDGET(nit), VIEW(nit), buf);
             _elm_access_text_set(_elm_access_object_get(nit->title),
                                  ELM_ACCESS_TYPE, E_("title"));
          }
     }
   else if (!strcmp("subtitle", part))
     {
        eina_stringshare_replace(&nit->subtitle_label, label);
        snprintf(buf, sizeof(buf), "elm.text.subtitle");

        //XXX: ACCESS
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          {
             nit->subtitle = _elm_access_edje_object_part_object_register
                             (WIDGET(nit), VIEW(nit), buf);
             _elm_access_text_set(_elm_access_object_get(nit->subtitle),
                                  ELM_ACCESS_TYPE, E_("sub title"));
          }
     }
   else
     {
        snprintf(buf, sizeof(buf), "%s", part);
        EINA_INLIST_FOREACH(nit->text_list, pair)
          if (!strcmp(buf, pair->part)) break;

        if (!pair)
          {
             pair = ELM_NEW(Elm_Naviframe_Text_Item_Pair);
             if (!pair)
               {
                  ERR("Failed to allocate new text part of the item! : naviframe=%p",
                  WIDGET(it));
                  return;
               }
             eina_stringshare_replace(&pair->part, buf);
             nit->text_list = eina_inlist_append(nit->text_list,
                                                 EINA_INLIST_GET(pair));
          }

        //XXX: ACCESS
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          {
             pair->access_object = _elm_access_edje_object_part_object_register
                             (WIDGET(nit), VIEW(nit), buf);
             _elm_access_text_set(_elm_access_object_get(pair->access_object),
                                  ELM_ACCESS_TYPE, E_(buf));
          }
     }

   edje_object_part_text_set(VIEW(nit), buf, label);
   _item_text_signals_emit(nit);

   elm_layout_sizing_eval(WIDGET(nit));
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   char buf[1024];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), "elm.text.title");
   else if (!strcmp("subtitle", part))
     snprintf(buf, sizeof(buf), "elm.text.subtitle");
   else
     snprintf(buf, sizeof(buf), "%s", part);

   return edje_object_part_text_get(VIEW(it), buf);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit, *prev_it = NULL;
   Eina_Bool top;

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   top = (it == elm_naviframe_top_item_get(WIDGET(nit)));
   if (evas_object_data_get(VIEW(nit), "out_of_list"))
     goto end;

   if (nit->content && !sd->on_deletion)
     {
        nit->content_unfocusable =
          elm_widget_tree_unfocusable_get(nit->content);
        elm_widget_tree_unfocusable_set(nit->content, EINA_TRUE);
     }

   if (sd->stack->last->prev)
     prev_it = EINA_INLIST_CONTAINER_GET
         (sd->stack->last->prev, Elm_Naviframe_Item);

   sd->stack = eina_inlist_remove(sd->stack, EINA_INLIST_GET(nit));
   if (!sd->stack) elm_widget_resize_object_set(WIDGET(it), sd->dummy_edje);

   if (top && !sd->on_deletion) /* must raise another one */
     {
        if (!prev_it) goto end;

        if (sd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(prev_it), EINA_FALSE);
          }

        evas_object_show(VIEW(prev_it));
        evas_object_raise(VIEW(prev_it));

        edje_object_signal_emit(VIEW(prev_it), "elm,state,visible", "elm");
        edje_object_message_signal_process(VIEW(prev_it));
     }

end:

   _item_free(nit);

   return EINA_TRUE;
}

static void
_item_content_set(Elm_Naviframe_Item *it,
                  Evas_Object *content)
{
   if (it->content == content) return;

   if (it->content) evas_object_del(it->content);
   it->content = content;

   if (!content) return;

   edje_object_part_swallow(VIEW(it), CONTENT_PART, content);

   evas_object_event_callback_add
     (content, EVAS_CALLBACK_DEL, _item_content_del_cb, it);
}

static void
_item_title_prev_btn_set(Elm_Naviframe_Item *it,
                         Evas_Object *btn)
{
   if (it->title_prev_btn == btn) return;
   if (it->title_prev_btn) evas_object_del(it->title_prev_btn);
   it->title_prev_btn = btn;
   if (!btn) return;

   edje_object_part_swallow(VIEW(it), PREV_BTN_PART, btn);

   evas_object_event_callback_add
     (btn, EVAS_CALLBACK_DEL, _item_title_prev_btn_del_cb, it);
}

static void
_item_title_next_btn_set(Elm_Naviframe_Item *it,
                         Evas_Object *btn)
{
   if (it->title_next_btn == btn) return;
   if (it->title_next_btn) evas_object_del(it->title_next_btn);
   it->title_next_btn = btn;
   if (!btn) return;

   edje_object_part_swallow(VIEW(it), NEXT_BTN_PART, btn);

   evas_object_event_callback_add
     (btn, EVAS_CALLBACK_DEL, _item_title_next_btn_del_cb, it);
}

static void
_item_title_icon_set(Elm_Naviframe_Item *it,
                     Evas_Object *icon)
{
   if (it->title_icon == icon) return;
   if (it->title_icon) evas_object_del(it->title_icon);
   it->title_icon = icon;
   if (!icon) return;

   edje_object_part_swallow(VIEW(it), ICON_PART, icon);

   evas_object_event_callback_add
     (icon, EVAS_CALLBACK_DEL, _item_title_icon_del_cb, it);
}

static Evas_Object *
_item_content_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->content;

   if (!content) return NULL;

   edje_object_part_unswallow(VIEW(it), it->content);

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_content_del_cb);

   it->content = NULL;
   return content;
}

static Evas_Object *
_item_title_prev_btn_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_prev_btn;

   if (!content) return NULL;

   edje_object_part_unswallow(VIEW(it), it->title_prev_btn);

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_prev_btn_del_cb);

   it->title_prev_btn = NULL;
   return content;
}

static Evas_Object *
_item_title_next_btn_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_next_btn;

   if (!content) return NULL;

   edje_object_part_unswallow(VIEW(it), it->title_next_btn);

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_next_btn_del_cb);

   it->title_next_btn = NULL;
   return content;
}

static Evas_Object *
_item_title_icon_unset(Elm_Naviframe_Item *it)
{
   Evas_Object *content = it->title_icon;

   if (!content) return NULL;

   edje_object_part_unswallow(VIEW(it), it->title_icon);

   evas_object_event_callback_del
     (content, EVAS_CALLBACK_DEL, _item_title_icon_del_cb);

   it->title_icon = NULL;
   return content;
}

/* since we have each item as layout, we can't reusing the layout's
 * aliasing, so let's do it ourselves */
static void
_part_aliasing_eval(const char **part)
{
   if (!*part || !strcmp("default", *part))
     *part = CONTENT_PART;
   else if (!strcmp(*part, "prev_btn"))
     *part = PREV_BTN_PART;
   else if (!strcmp(*part, "next_btn"))
     *part = NEXT_BTN_PART;
   else if (!strcmp(*part, "icon"))
     *part = ICON_PART;
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
   edje_object_signal_emit(VIEW(it), buf, "elm");
   it->content_list = eina_inlist_remove(it->content_list,
                                         EINA_INLIST_GET(pair));
   eina_stringshare_del(pair->part);
   free(pair);
}

static void
_title_content_set(Elm_Naviframe_Item *it,
                   const char *part,
                   Evas_Object *content)
{
   Elm_Naviframe_Content_Item_Pair *pair = NULL;
   Evas_Object *prev_content = NULL;
   char buf[1024];

   EINA_INLIST_FOREACH(it->content_list, pair)
     if (!strcmp(part, pair->part)) break;
   if (pair)
     {
        prev_content = edje_object_part_swallow_get(VIEW(it), part);
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
             ERR("Failed to allocate new content part of the item! : naviframe=%p",
             WIDGET(it));
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
    edje_object_part_swallow(VIEW(it), part, content);
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     _item_content_set(nit, content);
   else if (!strcmp(part, PREV_BTN_PART))
     _item_title_prev_btn_set(nit, content);
   else if (!strcmp(part, NEXT_BTN_PART))
     _item_title_next_btn_set(nit, content);
   else if (!strcmp(part, ICON_PART))
     _item_title_icon_set(nit, content);
   else
     _title_content_set(nit, part, content);

   _item_content_signals_emit(nit);

   elm_layout_sizing_eval(WIDGET(it));
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     return nit->content;
   else if (!strcmp(part, PREV_BTN_PART))
     return nit->title_prev_btn;
   else if (!strcmp(part, NEXT_BTN_PART))
     return nit->title_next_btn;
   else if (!strcmp(part, ICON_PART))
     return nit->title_icon;

   //common parts
   return edje_object_part_swallow_get(VIEW(nit), part);
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
             content = edje_object_part_swallow_get(VIEW(it), part);
             eina_stringshare_del(pair->part);
             it->content_list = eina_inlist_remove(it->content_list,
                                                   EINA_INLIST_GET(pair));
             free(pair);
             break;
          }
     }

   if (!content) return NULL;

   edje_object_part_unswallow(VIEW(it), content);
   snprintf(buf, sizeof(buf), "elm,state,%s,hide", part);
   edje_object_signal_emit(VIEW(it), buf, "elm");
   evas_object_event_callback_del(content,
                                  EVAS_CALLBACK_DEL,
                                  _title_content_del);
   return content;
}

static Evas_Object *
_item_content_unset_hook(Elm_Object_Item *it,
                         const char *part)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;
   Evas_Object *o = NULL;

   _part_aliasing_eval(&part);

   //specified parts
   if (!part || !strcmp(CONTENT_PART, part))
     o = _item_content_unset(nit);
   else if (!strcmp(part, PREV_BTN_PART))
     o = _item_title_prev_btn_unset(nit);
   else if (!strcmp(part, NEXT_BTN_PART))
     o = _item_title_next_btn_unset(nit);
   else if (!strcmp(part, ICON_PART))
     o = _item_title_icon_unset(nit);
   else
     o = _title_content_unset(nit, part);

   _item_content_signals_emit(nit);

   elm_layout_sizing_eval(WIDGET(it));

   return o;
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   edje_object_signal_emit(VIEW(it), emission, source);
}

static void
_elm_naviframe_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1;
   Elm_Naviframe_Item *it;
   Evas_Coord x, y, w, h;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   evas_object_geometry_get(obj, &x, &y, &w, &h);

   EINA_INLIST_FOREACH (sd->stack, it)
     {
        evas_object_move(VIEW(it), x, y);
        evas_object_resize(VIEW(it), w, h);
        edje_object_size_min_calc(VIEW(it), &it->minw, &it->minh);
        if (it->minw > minw) minw = it->minw;
        if (it->minh > minh) minh = it->minh;
     }

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

static Eina_Bool
_item_pop_idler(void *data)
{
   elm_naviframe_item_pop(data);

   return ECORE_CALLBACK_CANCEL;
}

static void
_on_item_back_btn_clicked(void *data,
                          Evas_Object *obj,
                          void *event_info __UNUSED__)
{
   /* Since edje has the event queue, clicked event could be happend
      multiple times on some heavy environment. This callback del will
      prevent those scenario and guarantee only one clicked for it's own
      page. */
   evas_object_smart_callback_del(obj, "clicked", _on_item_back_btn_clicked);

   ecore_idler_add(_item_pop_idler, data);
}

static Evas_Object *
_back_btn_new(Evas_Object *obj, const char *title_label)
{
   Evas_Object *btn, *ed;
   char buf[1024];

   btn = elm_button_add(obj);

   if (!btn) return NULL;
   evas_object_smart_callback_add
     (btn, "clicked", _on_item_back_btn_clicked, obj);
   snprintf
     (buf, sizeof(buf), "naviframe/back_btn/%s", elm_widget_style_get(obj));
   elm_object_style_set(btn, buf);
   if (title_label)
     elm_layout_text_set(btn, NULL, title_label);
   else
     elm_object_domain_translatable_text_set(btn, PACKAGE, N_("Back"));

   /* HACK NOTE: this explicit check only exists to avoid an ERR()
    * message from elm_layout_content_set().
    *
    * The button was ALWAYS supposed to support an elm.swallow.content, but
    * default naviframe/back_btn/default theme did not provide such, then
    * old themes would emit such error message.
    *
    * Once we can break the theme API, remove this check and always
    * set an icon.
    */
   ed = elm_layout_edje_get(btn);
   if (edje_object_part_exists(ed, "elm.swallow.content"))
     {
        Evas_Object *ico = elm_icon_add(btn);
        elm_icon_standard_set(ico, "arrow_left");
        elm_layout_content_set(btn, "elm.swallow.content", ico);
     }

   return btn;
}

static void
_elm_naviframe_smart_signal(Evas_Object *obj,
                            const char *emission,
                            const char *source)
{
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   if (!sd->stack) return;

   _elm_naviframe_parent_sc->signal(obj, emission, source);
}

/* content/text smart functions proxying things to the top item, which
 * is the resize object of the layout */
static Eina_Bool
_elm_naviframe_smart_text_set(Evas_Object *obj,
                              const char *part,
                              const char *label)
{
   Elm_Object_Item *it;

   it = elm_naviframe_top_item_get(obj);
   if (!it) return EINA_FALSE;

   elm_object_item_part_text_set(it, part, label);

   return !strcmp(elm_object_item_part_text_get(it, part), label);
}

static const char *
_elm_naviframe_smart_text_get(const Evas_Object *obj,
                              const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);

   if (!it) return NULL;

   return elm_object_item_part_text_get(it, part);
}

/* we have to keep a "manual" set here because of the callbacks on the
 * children */
static Eina_Bool
_elm_naviframe_smart_content_set(Evas_Object *obj,
                                 const char *part,
                                 Evas_Object *content)
{
   Elm_Object_Item *it;

   it = elm_naviframe_top_item_get(obj);
   if (!it) return EINA_FALSE;

   elm_object_item_part_content_set(it, part, content);

   return content == elm_object_item_part_content_get(it, part);
}

static Evas_Object *
_elm_naviframe_smart_content_get(const Evas_Object *obj,
                                 const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);

   if (!it) return NULL;

   return elm_object_item_part_content_get(it, part);
}

static Evas_Object *
_elm_naviframe_smart_content_unset(Evas_Object *obj,
                                   const char *part)
{
   Elm_Object_Item *it = elm_naviframe_top_item_get(obj);

   if (!it) return NULL;

   return elm_object_item_part_content_unset(it, part);
}

static void
_on_item_title_clicked(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   evas_object_smart_callback_call(WIDGET(it), SIG_TITLE_CLICKED, it);
}

/* "elm,state,cur,pushed"
 */
static void
_on_item_push_finished(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   if (!it) return;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   evas_object_hide(VIEW(it));

   if (it->content)
     elm_widget_tree_unfocusable_set(it->content, it->content_unfocusable);

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);
}

/* "elm,state,cur,popped"
 */
static void
_on_item_pop_finished(void *data,
                      Evas_Object *obj __UNUSED__,
                      const char *emission __UNUSED__,
                      const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   if (!it) return;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   if (sd->preserve && it->content)
     elm_widget_tree_unfocusable_set(it->content, it->content_unfocusable);

   elm_widget_item_del(data);
}

/* "elm,state,new,pushed",
 * "elm,state,prev,popped
 */
static void
_on_item_show_finished(void *data,
                       Evas_Object *obj __UNUSED__,
                       const char *emission __UNUSED__,
                       const char *source __UNUSED__)
{
   Elm_Naviframe_Item *it = data;

   ELM_NAVIFRAME_DATA_GET(WIDGET(it), sd);

   edje_object_signal_emit(VIEW(it), "elm,state,visible", "elm");
   edje_object_message_signal_process(VIEW(it));

   elm_widget_tree_unfocusable_set(it->content, it->content_unfocusable);

   evas_object_smart_callback_call(WIDGET(it), SIG_TRANSITION_FINISHED, data);

   if (sd->freeze_events)
     evas_object_freeze_events_set(VIEW(it), EINA_FALSE);
}

static void
_on_item_size_hints_changed(void *data,
                            Evas *e __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static Elm_Naviframe_Item *
_item_new(Evas_Object *obj,
          const Elm_Naviframe_Item *prev_it,
          const char *title_label,
          Evas_Object *prev_btn,
          Evas_Object *next_btn,
          Evas_Object *content,
          const char *item_style)
{
   Elm_Naviframe_Item *it;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   it = elm_widget_item_new(obj, Elm_Naviframe_Item);
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
   VIEW(it) = edje_object_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(VIEW(it), obj);

   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _on_item_size_hints_changed, obj);
   edje_object_signal_callback_add
     (VIEW(it), "elm,action,show,finished", "", _on_item_show_finished, it);
   edje_object_signal_callback_add
     (VIEW(it), "elm,action,pushed,finished", "", _on_item_push_finished, it);
   edje_object_signal_callback_add
     (VIEW(it), "elm,action,popped,finished", "", _on_item_pop_finished, it);
   edje_object_signal_callback_add
     (VIEW(it), "elm,action,title,clicked", "", _on_item_title_clicked, it);

   _item_style_set(it, item_style);
   _item_text_set_hook((Elm_Object_Item *)it, "elm.text.title", title_label);

   //title buttons
   if ((!prev_btn) && sd->auto_pushed && prev_it)
     {
        const char *prev_title = prev_it->title_label;
        prev_btn = _back_btn_new(obj, prev_title);
     }

   if (prev_btn)
     _item_content_set_hook((Elm_Object_Item *)it, PREV_BTN_PART, prev_btn);

   if (next_btn)
     _item_content_set_hook((Elm_Object_Item *)it, NEXT_BTN_PART, next_btn);

   _item_content_set(it, content);
   it->title_visible = EINA_TRUE;

   return it;
}

static Eina_Bool
_elm_naviframe_smart_focus_next(const Evas_Object *obj,
                                Elm_Focus_Direction dir,
                                Evas_Object **next)
{
   Eina_Bool ret;
   Eina_List *l = NULL;
   Elm_Naviframe_Item *top_it;
   Elm_Naviframe_Content_Item_Pair *content_pair = NULL;
   Elm_Naviframe_Text_Item_Pair *text_pair = NULL;
   void *(*list_data_get)(const Eina_List *list);

   top_it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!top_it) return EINA_FALSE;

   if (!top_it->title_visible)
     {
        return elm_widget_focus_next_get(top_it->content, dir, next);
     }

   list_data_get = eina_list_data_get;

   //Forcus order: prev button, next button, contents
   //XXX: ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        if (top_it->title) l = eina_list_append(l, top_it->title);
        if (top_it->subtitle) l = eina_list_append(l, top_it->subtitle);
        EINA_INLIST_FOREACH(top_it->text_list, text_pair)
          {
             if (text_pair->access_object)
               l = eina_list_append(l, text_pair->access_object);
          }
     }

   if (top_it->title_prev_btn)
     l = eina_list_append(l, top_it->title_prev_btn);
   if (top_it->title_next_btn)
     l = eina_list_append(l, top_it->title_next_btn);
   if (top_it->content)
     l = eina_list_append(l, top_it->content);
   EINA_INLIST_FOREACH(top_it->content_list, content_pair)
     {
        if (edje_object_part_swallow_get(VIEW(top_it), content_pair->part))
          l = eina_list_append(l, edje_object_part_swallow_get(VIEW(top_it),
                                                               content_pair->part));
     }
   l = eina_list_append(l, VIEW(top_it));

   ret = elm_widget_focus_list_next_get(obj, l, list_data_get, dir, next);
   eina_list_free(l);

   return ret;
}

static void
_elm_naviframe_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Naviframe_Smart_Data);

   priv->dummy_edje = ELM_WIDGET_DATA(priv)->resize_obj =
       edje_object_add(evas_object_evas_get(obj));

   ELM_WIDGET_CLASS(_elm_naviframe_parent_sc)->base.add(obj);

   priv->auto_pushed = EINA_TRUE;
   priv->freeze_events = EINA_TRUE;

   elm_widget_can_focus_set(obj, EINA_FALSE);
}

static void
_elm_naviframe_smart_del(Evas_Object *obj)
{
   Elm_Naviframe_Item *it;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   sd->on_deletion = EINA_TRUE;

   while (sd->stack)
     {
        it = EINA_INLIST_CONTAINER_GET(sd->stack, Elm_Naviframe_Item);
        elm_widget_item_del(it);
     }

   sd->on_deletion = EINA_FALSE;

   evas_object_del(sd->dummy_edje);

   ELM_WIDGET_CLASS(_elm_naviframe_parent_sc)->base.del(obj);
}

static void
_elm_naviframe_smart_set_user(Elm_Naviframe_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_naviframe_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_naviframe_smart_del;

   ELM_WIDGET_CLASS(sc)->theme = _elm_naviframe_smart_theme;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_naviframe_smart_focus_next;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_naviframe_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_naviframe_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_naviframe_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->signal = _elm_naviframe_smart_signal;
   ELM_LAYOUT_CLASS(sc)->text_set = _elm_naviframe_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_naviframe_smart_text_get;
   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_naviframe_smart_sizing_eval;
}

EAPI const Elm_Naviframe_Smart_Class *
elm_naviframe_smart_class_get(void)
{
   static Elm_Naviframe_Smart_Class _sc =
     ELM_NAVIFRAME_SMART_CLASS_INIT_NAME_VERSION
       (ELM_NAVIFRAME_SMART_NAME);
   static const Elm_Naviframe_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_naviframe_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_naviframe_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_naviframe_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

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
   Elm_Naviframe_Item *prev_it, *it;

   ELM_NAVIFRAME_CHECK(obj) NULL;

   ELM_NAVIFRAME_DATA_GET(obj, sd);

   prev_it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   it = _item_new(obj, prev_it,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return NULL;

   evas_object_show(VIEW(it));
   elm_widget_resize_object_set(obj, VIEW(it));

   if (prev_it)
     {
        /* re-add as smart member */
        evas_object_smart_member_add(VIEW(prev_it), obj);

        if (sd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
             evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
          }

        edje_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed", "elm");
        edje_object_signal_emit(VIEW(it), "elm,state,new,pushed", "elm");

        /* animate previous */
        edje_object_message_signal_process(VIEW(prev_it));
        if (prev_it->content)
          {
             prev_it->content_unfocusable =
               elm_widget_tree_unfocusable_get(prev_it->content);
             elm_widget_tree_unfocusable_set(prev_it->content, EINA_TRUE);
          }

        /* animate new one */
        edje_object_message_signal_process(VIEW(it));
     }

   sd->stack = eina_inlist_append(sd->stack, EINA_INLIST_GET(it));
   evas_object_raise(VIEW(it));

   //XXX: ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     elm_object_focus_set(it->title, EINA_TRUE);

   elm_layout_sizing_eval(obj);

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
   Elm_Naviframe_Item *it, *prev_it;

   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(before, NULL);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   prev_it = NULL;
   if (before)
     {
        it = (Elm_Naviframe_Item *)before;
        prev_it = EINA_INLIST_CONTAINER_GET(EINA_INLIST_GET(it)->prev,
                                            Elm_Naviframe_Item);
     }

   it = _item_new(obj, prev_it,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return NULL;

   sd->stack = eina_inlist_prepend_relative
       (sd->stack, EINA_INLIST_GET(it),
       EINA_INLIST_GET(((Elm_Naviframe_Item *)before)));

   elm_layout_sizing_eval(obj);

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
   Elm_Naviframe_Item *it;

   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(after, NULL);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   it = _item_new(obj, (Elm_Naviframe_Item *)after,
                  title_label, prev_btn, next_btn, content, item_style);
   if (!it) return NULL;

   /* let's share that whole logic, if it goes to the top */
   if (elm_naviframe_top_item_get(obj) == after)
     return elm_naviframe_item_push
              (obj, title_label, prev_btn, next_btn, content, item_style);

   sd->stack = eina_inlist_append_relative
       (sd->stack, EINA_INLIST_GET(it),
       EINA_INLIST_GET(((Elm_Naviframe_Item *)after)));

   elm_layout_sizing_eval(obj);

   return (Elm_Object_Item *)it;
}

EAPI Evas_Object *
elm_naviframe_item_pop(Evas_Object *obj)
{
   Elm_Naviframe_Item *it, *prev_it = NULL;
   Evas_Object *content = NULL;

   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   it = (Elm_Naviframe_Item *)elm_naviframe_top_item_get(obj);
   if (!it) return NULL;

   if (sd->preserve)
     content = it->content;

   evas_object_data_set(VIEW(it), "out_of_list", (void *)1);

   if (it->content)
     {
        it->content_unfocusable = elm_widget_tree_unfocusable_get(it->content);
        elm_widget_tree_unfocusable_set(it->content, EINA_TRUE);
     }

   if (sd->stack->last->prev)
     prev_it = EINA_INLIST_CONTAINER_GET
         (sd->stack->last->prev, Elm_Naviframe_Item);

   sd->stack = eina_inlist_remove(sd->stack, EINA_INLIST_GET(it));
   if (!sd->stack) elm_widget_resize_object_set(obj, sd->dummy_edje);

   if (prev_it)
     {
        if (sd->freeze_events)
          {
             evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
             evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
          }

        elm_widget_resize_object_set(obj, VIEW(it));
        evas_object_raise(VIEW(prev_it));

        //XXX: ACCESS
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          elm_object_focus_set(prev_it->title, EINA_TRUE);

        /* these 2 signals MUST take place simultaneously */
        edje_object_signal_emit(VIEW(it), "elm,state,cur,popped", "elm");
        evas_object_show(VIEW(prev_it));
        edje_object_signal_emit(VIEW(prev_it), "elm,state,prev,popped", "elm");

        edje_object_message_signal_process(VIEW(it));
        edje_object_message_signal_process(VIEW(prev_it));
     }
   else
     elm_widget_item_del(it);

   return content;
}

EAPI void
elm_naviframe_item_pop_to(Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit;
   Eina_Inlist *l, *prev_l;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   if (it == elm_naviframe_top_item_get(WIDGET(nit))) return;

   l = sd->stack->last->prev;

   sd->on_deletion = EINA_TRUE;

   while (l)
     {
        Elm_Naviframe_Item *iit = EINA_INLIST_CONTAINER_GET
            (l, Elm_Naviframe_Item);

        if (iit == nit) break;

        prev_l = l->prev;
        sd->stack = eina_inlist_remove(sd->stack, l);

        elm_widget_item_del(iit);

        l = prev_l;
     }

   sd->on_deletion = EINA_FALSE;

   elm_naviframe_item_pop(WIDGET(nit));
}

EAPI void
elm_naviframe_item_promote(Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit;
   Elm_Naviframe_Item *prev_it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   nit = (Elm_Naviframe_Item *)it;
   ELM_NAVIFRAME_DATA_GET(WIDGET(nit), sd);

   if (it == elm_naviframe_top_item_get(nit->base.widget)) return;

   /* remember, last is 1st on the naviframe, push it to last pos. */
   sd->stack = eina_inlist_demote(sd->stack, EINA_INLIST_GET(nit));

   elm_widget_resize_object_set(WIDGET(it), VIEW(nit));

   /* this was the previous top one */
   prev_it = EINA_INLIST_CONTAINER_GET
       (sd->stack->last->prev, Elm_Naviframe_Item);

   /* re-add as smart member */
   evas_object_smart_member_add(VIEW(prev_it), WIDGET(it));

   if (prev_it->content)
     {
        prev_it->content_unfocusable =
          elm_widget_tree_unfocusable_get(prev_it->content);
        elm_widget_tree_unfocusable_set(prev_it->content, EINA_TRUE);
     }

   if (sd->freeze_events)
     {
        evas_object_freeze_events_set(VIEW(it), EINA_TRUE);
        evas_object_freeze_events_set(VIEW(prev_it), EINA_TRUE);
     }

   edje_object_signal_emit(VIEW(prev_it), "elm,state,cur,pushed", "elm");

   evas_object_show(VIEW(nit));
   evas_object_raise(VIEW(nit));

   edje_object_signal_emit(VIEW(nit), "elm,state,new,pushed", "elm");

   edje_object_message_signal_process(VIEW(prev_it));
   edje_object_message_signal_process(VIEW(nit));
}

EAPI void
elm_naviframe_item_simple_promote(Evas_Object *obj,
                                  Evas_Object *content)
{
   Elm_Naviframe_Item *itr;

   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH (sd->stack, itr)
     {
        if (elm_object_item_content_get((Elm_Object_Item *)itr) == content)
          {
             elm_naviframe_item_promote((Elm_Object_Item *)itr);
             break;
          }
     }
}

EAPI void
elm_naviframe_content_preserve_on_pop_set(Evas_Object *obj,
                                          Eina_Bool preserve)
{
   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   sd->preserve = !!preserve;
}

EAPI Eina_Bool
elm_naviframe_content_preserve_on_pop_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   return sd->preserve;
}

EAPI Elm_Object_Item *
elm_naviframe_top_item_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   if (!sd->stack) return NULL;
   return (Elm_Object_Item *)(EINA_INLIST_CONTAINER_GET
                                (sd->stack->last, Elm_Naviframe_Item));
}

EAPI Elm_Object_Item *
elm_naviframe_bottom_item_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   if (!sd->stack) return NULL;
   return (Elm_Object_Item *)(EINA_INLIST_CONTAINER_GET
                                (sd->stack, Elm_Naviframe_Item));
}

EAPI void
elm_naviframe_item_style_set(Elm_Object_Item *it,
                             const char *item_style)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   if (item_style && !strcmp(item_style, nit->style)) return;

   if (!item_style)
     if (!strcmp("basic", nit->style)) return;

   _item_style_set(nit, item_style);
   _item_title_visible_update(nit);
}

EAPI const char *
elm_naviframe_item_style_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it, NULL);

   return nit->style;
}

EAPI void
elm_naviframe_item_title_visible_set(Elm_Object_Item *it,
                                     Eina_Bool visible)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it);

   visible = !!visible;
   if (nit->title_visible == visible) return;

   nit->title_visible = visible;
   _item_title_visible_update(nit);
}

EAPI Eina_Bool
elm_naviframe_item_title_visible_get(const Elm_Object_Item *it)
{
   Elm_Naviframe_Item *nit = (Elm_Naviframe_Item *)it;

   ELM_NAVIFRAME_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return nit->title_visible;
}

EAPI void
elm_naviframe_prev_btn_auto_pushed_set(Evas_Object *obj,
                                       Eina_Bool auto_pushed)
{
   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   sd->auto_pushed = !!auto_pushed;
}

EAPI Eina_Bool
elm_naviframe_prev_btn_auto_pushed_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   return sd->auto_pushed;
}

EAPI Eina_List *
elm_naviframe_items_get(const Evas_Object *obj)
{
   Eina_List *ret = NULL;
   Elm_Naviframe_Item *itr;

   ELM_NAVIFRAME_CHECK(obj) NULL;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH (sd->stack, itr)
     ret = eina_list_append(ret, itr);

   return ret;
}

EAPI void
elm_naviframe_event_enabled_set(Evas_Object *obj,
                                Eina_Bool enabled)
{
   ELM_NAVIFRAME_CHECK(obj);
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   enabled = !!enabled;
   if (sd->freeze_events == !enabled) return;
   sd->freeze_events = !enabled;
}

EAPI Eina_Bool
elm_naviframe_event_enabled_get(const Evas_Object *obj)
{
   ELM_NAVIFRAME_CHECK(obj) EINA_FALSE;
   ELM_NAVIFRAME_DATA_GET(obj, sd);

   return !sd->freeze_events;
}
