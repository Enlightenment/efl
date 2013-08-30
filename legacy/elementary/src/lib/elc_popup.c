#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_popup.h"

EAPI Eo_Op ELM_OBJ_POPUP_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_POPUP_CLASS

#define MY_CLASS_NAME "elm_popup"

#define ELM_POPUP_ACTION_BUTTON_MAX 3

static void _button_remove(Evas_Object *, int, Eina_Bool);

static const char ACCESS_TITLE_PART[] = "access.title";
static const char ACCESS_BODY_PART[] = "access.body";
static const char CONTENT_PART[] = "elm.swallow.content";

static const char SIG_BLOCK_CLICKED[] = "block,clicked";
static const char SIG_TIMEOUT[] = "timeout";
static const char SIG_LANG_CHANGED[] = "language,changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_BLOCK_CLICKED, ""},
   {SIG_TIMEOUT, ""},
   {SIG_LANG_CHANGED, ""},
   {"focused", ""}, /**< handled by elm_widget */
   {"unfocused", ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static void  _on_content_del(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void
_elm_popup_smart_translate(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Popup_Smart_Data *sd = _pd;
   Elm_Popup_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
      elm_widget_item_translate(it);

   evas_object_smart_callback_call(obj, SIG_LANG_CHANGED, NULL);

   if (ret) *ret = EINA_TRUE;
}

static void
_visuals_set(Evas_Object *obj)
{
   ELM_POPUP_DATA_GET(obj, sd);

   if (!sd->title_text && !sd->title_icon)
     elm_layout_signal_emit(obj, "elm,state,title_area,hidden", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,title_area,visible", "elm");

   if (sd->action_area)
     elm_layout_signal_emit(obj, "elm,state,action_area,visible", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,action_area,hidden", "elm");
}

static void
_block_clicked_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   evas_object_smart_callback_call(data, SIG_BLOCK_CLICKED, NULL);
}

static void
_timeout_cb(void *data,
            Evas_Object *obj __UNUSED__,
            void *event_info __UNUSED__)
{
   evas_object_hide(data);
   evas_object_smart_callback_call(data, SIG_TIMEOUT, NULL);
}

static Evas_Object *
_access_object_get(const Evas_Object *obj, const char* part)
{
   Evas_Object *po, *ao;
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   po = (Evas_Object *)edje_object_part_object_get(wd->resize_obj, part);
   ao = evas_object_data_get(po, "_part_access_obj");

   return ao;
}

static void
_on_show(void *data __UNUSED__,
         Evas *e __UNUSED__,
         Evas_Object *obj,
         void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(obj, sd);

   /* yeah, ugly, but again, this widget needs a rewrite */
   if (elm_widget_parent_get(sd->notify) == obj)
     elm_widget_sub_object_del(obj, sd->notify);

/* FIXME: Should be rewritten popup. This code is for arranging child-parent relation well. Without this code, Popup't top parent will be notify. And there will be no parent for notify. Terrible! */
   elm_widget_sub_object_add(elm_widget_parent_get(obj), sd->notify);

   elm_object_content_set(sd->notify, obj);

   evas_object_show(sd->notify);

   elm_object_focus_set(obj, EINA_TRUE);
}

static void
_on_hide(void *data __UNUSED__,
            Evas *e __UNUSED__,
            Evas_Object *obj,
            void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(obj, sd);

   //Revert the obj-tree again.
   elm_object_content_unset(sd->notify);
   elm_widget_sub_object_add(elm_widget_parent_get(sd->notify), obj);
   elm_widget_sub_object_add(obj, sd->notify);

   evas_object_hide(sd->notify);

/* FIXME:elm_object_content_unset(notify) deletes callback to revert focus status. */
   elm_object_focus_set(obj, EINA_FALSE);
}

static void
_scroller_size_calc(Evas_Object *obj)
{
   Evas_Coord h;
   Evas_Coord h_title = 0;
   Evas_Coord h_action_area = 0;
   const char *action_area_height;

   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (!sd->items) return;

   sd->scr_size_recalc = EINA_FALSE;
   sd->max_sc_h = -1;
   sd->max_sc_w = -1;
   evas_object_geometry_get(sd->notify, NULL, NULL, NULL, &h);
   if (sd->title_text || sd->title_icon)
     edje_object_part_geometry_get(wd->resize_obj,
                                   "elm.bg.title", NULL, NULL, NULL, &h_title);
   if (sd->action_area)
     {
        action_area_height = edje_object_data_get(
            elm_layout_edje_get(sd->action_area), "action_area_height");
        if (action_area_height)
          h_action_area =
            (int)(atoi(action_area_height)
                  * elm_config_scale_get() * elm_object_scale_get(obj));
     }

   sd->max_sc_h = h - (h_title + h_action_area);
}

static void
_size_hints_changed_cb(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_list_del(Elm_Popup_Smart_Data *sd)
{
   if (!sd->scr) return;

   evas_object_event_callback_del
     (sd->scr, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _size_hints_changed_cb);

   evas_object_del(sd->tbl);
   sd->scr = NULL;
   sd->box = NULL;
   sd->spacer = NULL;
   sd->tbl = NULL;
}

static void
_items_remove(Elm_Popup_Smart_Data *sd)
{
   Elm_Popup_Item *item;

   if (!sd->items) return;

   EINA_LIST_FREE(sd->items, item)
     elm_widget_item_del(item);

   sd->items = NULL;
}

static void
_elm_popup_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   unsigned int i;

   Elm_Popup_Smart_Data *sd = _pd;

   evas_object_smart_callback_del
     (sd->notify, "block,clicked", _block_clicked_cb);
   evas_object_smart_callback_del(sd->notify, "timeout", _timeout_cb);
   evas_object_event_callback_del
     (sd->content, EVAS_CALLBACK_DEL, _on_content_del);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_SHOW, _on_show);

   sd->button_count = 0;

   for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
     {
        if (sd->buttons[i])
          {
             evas_object_del(sd->buttons[i]->btn);
             free(sd->buttons[i]);
             sd->buttons[i] = NULL;
          }
     }
   if (sd->items)
     {
        _items_remove(sd);
        _list_del(sd);
     }

   // XXX? delete other objects? just to be sure.
   ELM_SAFE_FREE(sd->notify, evas_object_del);
   ELM_SAFE_FREE(sd->title_icon, evas_object_del);
   ELM_SAFE_FREE(sd->content_area, evas_object_del);
   ELM_SAFE_FREE(sd->text_content_obj, evas_object_del);
   ELM_SAFE_FREE(sd->action_area, evas_object_del);
   ELM_SAFE_FREE(sd->box, evas_object_del);
   ELM_SAFE_FREE(sd->tbl, evas_object_del);
   ELM_SAFE_FREE(sd->spacer, evas_object_del);
   ELM_SAFE_FREE(sd->scr, evas_object_del);
   ELM_SAFE_FREE(sd->content, evas_object_del);
   ELM_SAFE_FREE(sd->title_text, eina_stringshare_del);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool rtl)
{
   Eina_List *elist;
   Elm_Popup_Item *item;

   ELM_POPUP_DATA_GET(obj, sd);

   elm_object_mirrored_set(sd->notify, rtl);
   if (sd->items)
     EINA_LIST_FOREACH(sd->items, elist, item)
       edje_object_mirrored_set(VIEW(item), rtl);
}

static void
_access_obj_process(Eo *obj, Eina_Bool is_access)
{
   Evas_Object *ao;

   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (is_access)
     {
        if (sd->title_text)
          {
             ao = _elm_access_edje_object_part_object_register
                    (obj, wd->resize_obj, ACCESS_TITLE_PART);
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_TYPE, E_("Popup Title"));
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_INFO, sd->title_text);
          }

        if (sd->text_content_obj)
          {
             ao = _elm_access_edje_object_part_object_register
                    (obj, wd->resize_obj, ACCESS_BODY_PART);
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_TYPE, E_("Popup Body Text"));
             _elm_access_text_set(_elm_access_info_get(ao),
               ELM_ACCESS_INFO, elm_object_text_get(sd->text_content_obj));
          }
     }
   else
     {
        if (sd->title_text)
          {
             _elm_access_edje_object_part_object_unregister
                    (obj, wd->resize_obj, ACCESS_TITLE_PART);
          }

        if (sd->text_content_obj)
          {
             _elm_access_edje_object_part_object_unregister
                    (obj, wd->resize_obj, ACCESS_BODY_PART);
          }
     }
}

static void
_elm_popup_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Elm_Popup_Item *item;
   unsigned int i = 0;
   Eina_List *elist;
   char buf[128];

   Eina_Bool int_ret;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Popup_Smart_Data *sd = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   eo_do_super(obj, MY_CLASS, elm_wdg_theme(&int_ret));
   if (!int_ret) return;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   //FIXME: theme set seems corrupted.
   //if (elm_widget_parent_get(sd->notify) != obj)
     elm_widget_style_set(sd->notify, elm_widget_style_get(obj));

   if (sd->action_area)
     {
        snprintf(buf, sizeof(buf), "buttons%u", sd->button_count);
        if (!elm_layout_theme_set(sd->action_area, "popup", buf,
                                  elm_widget_style_get(obj)))
          CRITICAL("Failed to set layout!");
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          {
             if (!sd->buttons[i]) continue;
             elm_object_style_set(sd->buttons[i]->btn, buf);
          }
     }
   if (!elm_layout_theme_set(sd->content_area, "popup", "content",
                             elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");
   if (sd->text_content_obj)
     {
        snprintf(buf, sizeof(buf), "popup/%s", elm_widget_style_get(obj));
        elm_object_style_set(sd->text_content_obj, buf);
     }
   else if (sd->items)
     {
        EINA_LIST_FOREACH(sd->items, elist, item)
          {
             elm_widget_theme_object_set
               (obj, VIEW(item), "popup", "item", elm_widget_style_get(obj));
             if (item->label)
               {
                  edje_object_part_text_escaped_set
                    (VIEW(item), "elm.text", item->label);
                  edje_object_signal_emit
                    (VIEW(item), "elm,state,item,text,visible", "elm");
               }
             if (item->icon)
               edje_object_signal_emit
                 (VIEW(item), "elm,state,item,icon,visible", "elm");
             if (item->disabled)
               edje_object_signal_emit
                 (VIEW(item), "elm,state,item,disabled", "elm");
             evas_object_show(VIEW(item));
             edje_object_message_signal_process(VIEW(item));
          }
        _scroller_size_calc(obj);
     }
   if (sd->title_text)
     {
        elm_layout_text_set(obj, "elm.text.title", sd->title_text);
        elm_layout_signal_emit(obj, "elm,state,title,text,visible", "elm");
     }
   if (sd->title_icon)
     elm_layout_signal_emit(obj, "elm,state,title,icon,visible", "elm");

   _visuals_set(obj);
   edje_object_message_signal_process(wd->resize_obj);
   elm_layout_sizing_eval(obj);

   /* access */
   if (_elm_config->access_mode) _access_obj_process(obj, EINA_TRUE);

   if (ret) *ret = EINA_TRUE;
}

static void
_item_sizing_eval(Elm_Popup_Item *item)
{
   Evas_Coord min_w = -1, min_h = -1, max_w = -1, max_h = -1;

   edje_object_size_min_restricted_calc
     (VIEW(item), &min_w, &min_h, min_w, min_h);
   evas_object_size_hint_min_set(VIEW(item), min_w, min_h);
   evas_object_size_hint_max_set(VIEW(item), max_w, max_h);
}

static void
_elm_popup_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Eina_List *elist;
   Elm_Popup_Item *item;
   Evas_Coord h_box = 0, minh_box = 0;
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   Elm_Popup_Smart_Data *sd = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (sd->items)
     {
        EINA_LIST_FOREACH(sd->items, elist, item)
          {
             _item_sizing_eval(item);
             evas_object_size_hint_min_get(VIEW(item), NULL, &minh_box);
             if (minh_box != -1) h_box += minh_box;
          }
        evas_object_size_hint_min_set(sd->spacer, 0, MIN(h_box, sd->max_sc_h));
        evas_object_size_hint_max_set(sd->spacer, -1, sd->max_sc_h);

        evas_object_size_hint_min_get(sd->scr, &minw, &minh);
        evas_object_size_hint_max_get(sd->scr, &minw, &minh);
     }

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static void
_elm_popup_smart_sub_object_del(Eo *obj, void *_pd, va_list *list)
{
   Elm_Popup_Item *item;

   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   Elm_Popup_Smart_Data *sd = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   eo_do_super(obj, MY_CLASS, elm_wdg_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

   if (sobj == sd->title_icon)
     {
        elm_layout_signal_emit(obj, "elm,state,title,icon,hidden", "elm");
        edje_object_message_signal_process(wd->resize_obj);
        sd->title_icon = NULL;
     }
   else if ((item =
               evas_object_data_get(sobj, "_popup_icon_parent_item")) != NULL)
     {
        if (sobj == item->icon)
          {
             edje_object_part_unswallow(VIEW(item), sobj);
             edje_object_signal_emit
               (VIEW(item), "elm,state,item,icon,hidden", "elm");
             item->icon = NULL;
          }
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_on_content_del(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(data, sd);

   sd->content = NULL;
   elm_layout_sizing_eval(data);
}

static void
_on_text_content_del(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj __UNUSED__,
                     void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(data, sd);

   sd->text_content_obj = NULL;
   elm_layout_sizing_eval(data);
}

static void
_on_table_del(void *data,
              Evas *e __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(data, sd);

   sd->tbl = NULL;
   sd->spacer = NULL;
   sd->scr = NULL;
   sd->box = NULL;
   elm_layout_sizing_eval(data);
}

static void
_on_button_del(void *data,
               Evas *e __UNUSED__,
               Evas_Object *obj,
               void *event_info __UNUSED__)
{
   int i;

   ELM_POPUP_DATA_GET(data, sd);

   for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
     {
        if (sd->buttons[i] && obj == sd->buttons[i]->btn &&
            sd->buttons[i]->delete_me == EINA_TRUE)
          {
             _button_remove(data, i, EINA_FALSE);
             break;
          }
     }
}

static void
_button_remove(Evas_Object *obj,
               int pos,
               Eina_Bool delete)
{
   int i = 0;
   char buf[128];

   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (!sd->button_count) return;

   if (!sd->buttons[pos]) return;

   if (delete) evas_object_del(sd->buttons[pos]->btn);

   evas_object_event_callback_del
     (sd->buttons[pos]->btn, EVAS_CALLBACK_DEL, _on_button_del);
   ELM_SAFE_FREE(sd->buttons[pos], free);
   sd->button_count -= 1;

   if (!sd->no_shift)
     {
        /* shift left the remaining buttons */
        for (i = pos; i < ELM_POPUP_ACTION_BUTTON_MAX - 1; i++)
          {
             sd->buttons[i] = sd->buttons[i + 1];

             snprintf(buf, sizeof(buf), "actionbtn%d", pos + 1);
             elm_object_part_content_unset(sd->action_area, buf);
             snprintf(buf, sizeof(buf), "actionbtn%d", pos);
             elm_object_part_content_set
               (sd->action_area, buf, sd->buttons[i]->btn);
          }
     }

   if (!sd->button_count)
     {
        _visuals_set(obj);
        edje_object_part_unswallow
          (obj, edje_object_part_swallow_get(obj, "elm.swallow.action_area"));
        evas_object_del(sd->action_area);
        sd->action_area = NULL;
        edje_object_message_signal_process(wd->resize_obj);
     }
   else
     {
        snprintf(buf, sizeof(buf), "buttons%u", sd->button_count);
        if (!elm_layout_theme_set
            (sd->action_area, "popup", buf, elm_widget_style_get(obj)))
          CRITICAL("Failed to set layout!");
     }
}

static void
_layout_change_cb(void *data,
                  Evas_Object *obj __UNUSED__,
                  const char *emission __UNUSED__,
                  const char *source __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_restack_cb(void *data __UNUSED__,
            Evas *e __UNUSED__,
            Evas_Object *obj,
            void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(obj, sd);

   evas_object_layer_set(sd->notify, evas_object_layer_get(obj));
}

static void
_list_add(Evas_Object *obj)
{
   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   //Table
   sd->tbl = elm_table_add(obj);
   evas_object_event_callback_add(sd->tbl, EVAS_CALLBACK_DEL,
                                  _on_table_del, obj);
   edje_object_part_swallow(wd->resize_obj, CONTENT_PART, sd->tbl);
   evas_object_show(sd->tbl);

   //Spacer
   sd->spacer = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(sd->spacer, 0, 0, 0, 0);
   elm_table_pack(sd->tbl, sd->spacer, 0, 0, 1, 1);

   //Scroller
   sd->scr = elm_scroller_add(obj);
   elm_scroller_content_min_limit(sd->scr, EINA_TRUE, EINA_FALSE);
   elm_scroller_bounce_set(sd->scr, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set(sd->scr,
                                    EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->scr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add(sd->scr, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _size_hints_changed_cb, obj);
   elm_table_pack(sd->tbl, sd->scr, 0, 0, 1, 1);
   evas_object_show(sd->scr);

   //Box
   sd->box = elm_box_add(obj);
   evas_object_size_hint_weight_set(sd->box, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(sd->box, EVAS_HINT_FILL, 0.0);
   elm_object_content_set(sd->scr, sd->box);
   evas_object_show(sd->box);
}

static void
_item_select_cb(void *data,
                Evas_Object *obj __UNUSED__,
                const char *emission __UNUSED__,
                const char *source __UNUSED__)
{
   Elm_Popup_Item *item = data;

   if (!item || item->disabled) return;
   if (item->func)
     item->func((void *)item->base.data, WIDGET(item), data);
}

static void
_item_text_set(Elm_Popup_Item *item,
               const char *label)
{
   if (!eina_stringshare_replace(&item->label, label)) return;

   edje_object_part_text_escaped_set(VIEW(item), "elm.text", label);

   if (item->label)
     edje_object_signal_emit
       (VIEW(item), "elm,state,item,text,visible", "elm");
   else
     edje_object_signal_emit
       (VIEW(item), "elm,state,item,text,hidden", "elm");

   edje_object_message_signal_process(VIEW(item));
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Popup_Item *item = (Elm_Popup_Item *)it;

   ELM_POPUP_ITEM_CHECK_OR_RETURN(it);

   if ((!part) || (!strcmp(part, "default")))
     {
        _item_text_set(item, label);
        return;
     }

   WRN("The part name is invalid! : popup=%p", WIDGET(item));
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   Elm_Popup_Item *item = (Elm_Popup_Item *)it;

   ELM_POPUP_ITEM_CHECK_OR_RETURN(it, NULL);

   if ((!part) || (!strcmp(part, "default")))
     return item->label;

   WRN("The part name is invalid! : popup=%p", WIDGET(item));

   return NULL;
}

static void
_item_icon_set(Elm_Popup_Item *item,
               Evas_Object *icon)
{
   if (item->icon == icon) return;

   if (item->icon)
     evas_object_del(item->icon);

   item->icon = icon;
   if (item->icon)
     {
        elm_widget_sub_object_add(WIDGET(item), item->icon);
        evas_object_data_set(item->icon, "_popup_icon_parent_item", item);
        edje_object_part_swallow
          (VIEW(item), CONTENT_PART, item->icon);
        edje_object_signal_emit
          (VIEW(item), "elm,state,item,icon,visible", "elm");
     }
   else
     edje_object_signal_emit(VIEW(item), "elm,state,item,icon,hidden", "elm");

   edje_object_message_signal_process(item->base.view);
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Popup_Item *item = (Elm_Popup_Item *)it;

   ELM_POPUP_ITEM_CHECK_OR_RETURN(it);

   if ((!(part)) || (!strcmp(part, "default")))
     _item_icon_set(item, content);
   else
     WRN("The part name is invalid! : popup=%p", WIDGET(item));
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   Elm_Popup_Item *item = (Elm_Popup_Item *)it;

   ELM_POPUP_ITEM_CHECK_OR_RETURN(it, NULL);

   if ((!(part)) || (!strcmp(part, "default")))
     return item->icon;

   WRN("The part name is invalid! : popup=%p", WIDGET(item));

   return NULL;
}

static Evas_Object *
_item_icon_unset(Elm_Popup_Item *item)
{
   Evas_Object *icon = item->icon;

   if (!item->icon) return NULL;
   elm_widget_sub_object_del(WIDGET(item), icon);
   evas_object_data_del(icon, "_popup_icon_parent_item");
   edje_object_part_unswallow(item->base.view, icon);
   edje_object_signal_emit(VIEW(item), "elm,state,item,icon,hidden", "elm");
   item->icon = NULL;

   return icon;
}

static Evas_Object *
_item_content_unset_hook(const Elm_Object_Item *it,
                         const char *part)
{
   Evas_Object *content = NULL;
   Elm_Popup_Item *item = (Elm_Popup_Item *)it;

   ELM_POPUP_ITEM_CHECK_OR_RETURN(it, NULL);

   if ((!(part)) || (!strcmp(part, "default")))
     content = _item_icon_unset(item);
   else
     WRN("The part name is invalid! : popup=%p", WIDGET(item));

   return content;
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   Elm_Popup_Item *item = (Elm_Popup_Item *)it;

   ELM_POPUP_ITEM_CHECK_OR_RETURN(it);

   if (elm_widget_item_disabled_get(it))
     edje_object_signal_emit(VIEW(item), "elm,state,item,disabled", "elm");
   else
     edje_object_signal_emit(VIEW(item), "elm,state,item,enabled", "elm");
}

static void
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Popup_Item *item = (Elm_Popup_Item *)it;

   ELM_POPUP_ITEM_CHECK_OR_RETURN(it);
   ELM_POPUP_DATA_GET(WIDGET(item), sd);

   if (item->icon)
     evas_object_del(item->icon);

   eina_stringshare_del(item->label);
   sd->items = eina_list_remove(sd->items, item);
   if (!eina_list_count(sd->items))
     {
        sd->items = NULL;
        _list_del(sd);
     }
}

static void
_item_signal_emit_hook(Elm_Object_Item *it,
                       const char *emission,
                       const char *source)
{
   edje_object_signal_emit(VIEW(it), emission, source);
}

static void
_item_new(Elm_Popup_Item *item)
{
   elm_widget_item_text_set_hook_set(item, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(item, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(item, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(item, _item_content_unset_hook);
   elm_widget_item_disable_hook_set(item, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_signal_emit_hook_set(item, _item_signal_emit_hook);
   VIEW(item) = edje_object_add
       (evas_object_evas_get(WIDGET(item)));
   elm_widget_theme_object_set(WIDGET(item), VIEW(item), "popup", "item",
                               elm_widget_style_get(WIDGET(item)));
   edje_object_mirrored_set(VIEW(item), elm_widget_mirrored_get(WIDGET(item)));
   edje_object_signal_callback_add
     (VIEW(item), "elm,action,click", "", _item_select_cb, item);
   evas_object_size_hint_align_set
     (VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(VIEW(item));
}

static Eina_Bool
_title_text_set(Evas_Object *obj,
                const char *text)
{
   Evas_Object *ao;
   Eina_Bool title_visibility_old, title_visibility_current;

   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (sd->title_text == text) return EINA_TRUE;

   title_visibility_old = (sd->title_text) || (sd->title_icon);
   eina_stringshare_replace(&sd->title_text, text);

   //bare edje here because we're inside the hook, already
   edje_object_part_text_escaped_set
     (wd->resize_obj, "elm.text.title", text);

   /* access */
   if (_elm_config->access_mode)
     {
        ao = _access_object_get(obj, ACCESS_TITLE_PART);
        if (!ao)
          {
             ao = _elm_access_edje_object_part_object_register
                    (obj, wd->resize_obj, ACCESS_TITLE_PART);
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_TYPE, E_("Popup Title"));
          }
        _elm_access_text_set(_elm_access_info_get(ao), ELM_ACCESS_INFO, text);
     }

   if (sd->title_text)
     elm_layout_signal_emit(obj, "elm,state,title,text,visible", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,title,text,hidden", "elm");

   title_visibility_current = (sd->title_text) || (sd->title_icon);

   if (title_visibility_old != title_visibility_current)
     _visuals_set(obj);

   edje_object_message_signal_process(wd->resize_obj);
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_content_text_set(Evas_Object *obj,
                  const char *text)
{
   Evas_Object *ao;
   char buf[128];

   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (sd->items)
     {
        _items_remove(sd);
        _list_del(sd);
     }
   else edje_object_part_swallow(wd->resize_obj, CONTENT_PART,
                                 sd->content_area);
   if (!text) goto end;

   sd->text_content_obj = elm_label_add(obj);

   evas_object_event_callback_add
     (sd->text_content_obj, EVAS_CALLBACK_DEL, _on_text_content_del, obj);

   snprintf(buf, sizeof(buf), "popup/%s", elm_widget_style_get(obj));
   elm_object_style_set(sd->text_content_obj, buf);
   elm_label_line_wrap_set(sd->text_content_obj, sd->content_text_wrap_type);
   elm_object_text_set(sd->text_content_obj, text);
   evas_object_size_hint_weight_set
     (sd->text_content_obj, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set
     (sd->text_content_obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_layout_content_set
     (sd->content_area, CONTENT_PART, sd->text_content_obj);

   /* access */
   if (_elm_config->access_mode)
     {
        /* unregister label, ACCESS_BODY_PART will register */
        elm_access_object_unregister(sd->text_content_obj);

        ao = _access_object_get(obj, ACCESS_BODY_PART);
        if (!ao)
          {
             ao = _elm_access_edje_object_part_object_register
                    (obj, wd->resize_obj, ACCESS_BODY_PART);
             _elm_access_text_set(_elm_access_info_get(ao),
                                  ELM_ACCESS_TYPE, E_("Popup Body Text"));
          }
        _elm_access_text_set(_elm_access_info_get(ao), ELM_ACCESS_INFO, text);
     }

end:
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_elm_popup_smart_text_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret;

   if (!part || !strcmp(part, "default"))
     int_ret = _content_text_set(obj, label);
   else if (!strcmp(part, "title,text"))
     int_ret = _title_text_set(obj, label);
   else
     eo_do_super(obj, MY_CLASS, elm_obj_layout_text_set(part, label, &int_ret));

   if (ret) *ret = int_ret;
}

static const char *
_title_text_get(const Evas_Object *obj)
{
   ELM_POPUP_DATA_GET(obj, sd);

   return sd->title_text;
}

static const char *
_content_text_get(const Evas_Object *obj)
{
   const char *str = NULL;

   ELM_POPUP_DATA_GET(obj, sd);

   if (sd->text_content_obj)
     str = elm_object_text_get(sd->text_content_obj);

   return str;
}

static void
_elm_popup_smart_text_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   const char **text = va_arg(*list, const char **);

   if (!part || !strcmp(part, "default"))
     *text = _content_text_get(obj);
   else if (!strcmp(part, "title,text"))
     *text = _title_text_get(obj);
   else
     eo_do_super(obj, MY_CLASS, elm_obj_layout_text_get(part, text));
}

static Eina_Bool
_title_icon_set(Evas_Object *obj,
                Evas_Object *icon)
{
   Eina_Bool title_visibility_old, title_visibility_current;

   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (sd->title_icon == icon) return EINA_TRUE;
   title_visibility_old = (sd->title_text) || (sd->title_icon);
   if (sd->title_icon) evas_object_del(sd->title_icon);

   sd->title_icon = icon;
   title_visibility_current = (sd->title_text) || (sd->title_icon);

   //bare edje here because we're already in content_set virtual
   edje_object_part_swallow
     (wd->resize_obj, "elm.swallow.title.icon",
     sd->title_icon);

   if (sd->title_icon)
     elm_layout_signal_emit(obj, "elm,state,title,icon,visible", "elm");
   if (title_visibility_old != title_visibility_current) _visuals_set(obj);

   edje_object_message_signal_process(wd->resize_obj);
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_content_set(Evas_Object *obj,
             Evas_Object *content)
{
   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (sd->content && sd->content == content) return EINA_TRUE;
   if (sd->items)
     {
        _items_remove(sd);
        _list_del(sd);
     }

   sd->content = content;
   if (content)
     {
        //bare edje as to avoid loop
        edje_object_part_swallow
          (wd->resize_obj, CONTENT_PART,
          sd->content_area);

        elm_layout_content_set
          (sd->content_area, CONTENT_PART, content);
        evas_object_show(content);

        evas_object_event_callback_add
          (content, EVAS_CALLBACK_DEL, _on_content_del, obj);
     }
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static void
_action_button_set(Evas_Object *obj,
                   Evas_Object *btn,
                   unsigned int idx)
{
   Action_Area_Data *adata;
   char buf[128];

   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (idx >= ELM_POPUP_ACTION_BUTTON_MAX) return;

   if (!btn)
     {
        _button_remove(obj, idx, EINA_TRUE);
        return;
     }

   if (!sd->buttons[idx]) sd->button_count++;
   else
     {
        sd->no_shift = EINA_TRUE;
        evas_object_del(sd->buttons[idx]->btn);
        sd->no_shift = EINA_FALSE;
     }

   snprintf(buf, sizeof(buf), "buttons%u", sd->button_count);
   if (!sd->action_area)
     {
        sd->action_area = elm_layout_add(obj);
        evas_object_event_callback_add
          (sd->action_area, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
           _size_hints_changed_cb, obj);

        edje_object_part_swallow
          (wd->resize_obj, "elm.swallow.action_area", sd->action_area);
     }

   if (!elm_layout_theme_set(sd->action_area, "popup", buf,
                             elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");

   adata = ELM_NEW(Action_Area_Data);
   adata->obj = obj;
   adata->btn = btn;

   evas_object_event_callback_add
     (btn, EVAS_CALLBACK_DEL, _on_button_del, obj);

   sd->buttons[idx] = adata;

   snprintf(buf, sizeof(buf), "actionbtn%u", idx + 1);
   elm_object_part_content_set
     (sd->action_area, buf, sd->buttons[idx]->btn);

   if (sd->button_count == 1) _visuals_set(obj);

   edje_object_message_signal_process(wd->resize_obj);
   if (sd->items) _scroller_size_calc(obj);

   elm_layout_sizing_eval(obj);
}

static void
_elm_popup_smart_content_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Eina_Bool int_ret;

   unsigned int i;

   if (!part || !strcmp(part, "default"))
     int_ret = _content_set(obj, content);
   else if (!strcmp(part, "title,icon"))
     int_ret = _title_icon_set(obj, content);
   else if (!strncmp(part, "button", 6))
     {
        i = atoi(part + 6) - 1;

        if (i >= ELM_POPUP_ACTION_BUTTON_MAX)
          {
             ERR("The part name is invalid! : popup=%p", obj);
             int_ret = EINA_FALSE;
             return;
          }

        _action_button_set(obj, content, i);

        int_ret = EINA_TRUE;
     }
   else
     {
        eo_do_super(obj, MY_CLASS,
                    elm_obj_container_content_set(part, content, &int_ret));
     }

   if (ret) *ret = int_ret;
   return;
}

static Evas_Object *
_title_icon_get(const Evas_Object *obj)
{
   ELM_POPUP_DATA_GET(obj, sd);

   return sd->title_icon;
}

static Evas_Object *
_content_get(const Evas_Object *obj)
{
   ELM_POPUP_DATA_GET(obj, sd);

   return sd->content;
}

static Evas_Object *
_action_button_get(const Evas_Object *obj,
                   unsigned int idx)
{
   Evas_Object *button = NULL;

   ELM_POPUP_DATA_GET(obj, sd);
   if (!sd->action_area) return NULL;

   if (sd->buttons[idx])
     button = sd->buttons[idx]->btn;

   return button;
}

static void
_elm_popup_smart_content_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *content = NULL;
   unsigned int i;

   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);

   if (!part || !strcmp(part, "default"))
     content = _content_get(obj);
   else if (!strcmp(part, "title,text"))
     content = _title_icon_get(obj);
   else if (!strncmp(part, "button", 6))
     {
        i = atoi(part + 6) - 1;

        if (i >= ELM_POPUP_ACTION_BUTTON_MAX)
          goto err;

        content = _action_button_get(obj, i);
     }
   else
     goto err;

   *ret = content;
   return;

err:
   WRN("The part name is invalid! : popup=%p", obj);
   *ret = content;
}

static Evas_Object *
_content_unset(Evas_Object *obj)
{
   Evas_Object *content;

   ELM_POPUP_DATA_GET(obj, sd);

   if (!sd->content) return NULL;

   evas_object_event_callback_del
     (sd->content, EVAS_CALLBACK_DEL, _on_content_del);

   content = elm_layout_content_unset(sd->content_area, CONTENT_PART);
   sd->content = NULL;

   elm_layout_sizing_eval(obj);

   return content;
}

static Evas_Object *
_title_icon_unset(Evas_Object *obj)
{
   Evas_Object *icon;

   ELM_POPUP_DATA_GET(obj, sd);
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (!sd->title_icon) return NULL;

   icon = sd->title_icon;
   edje_object_part_unswallow(wd->resize_obj, sd->title_icon);
   sd->title_icon = NULL;

   return icon;
}

static void
_elm_popup_smart_content_unset(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *content = NULL;
   unsigned int i;

   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);

   if (!part || !strcmp(part, "default"))
     content = _content_unset(obj);
   else if (!strcmp(part, "title,icon"))
     content = _title_icon_unset(obj);
   else if (!strncmp(part, "button", 6))
     {
        i = atoi(part + 6) - 1;

        if (i >= ELM_POPUP_ACTION_BUTTON_MAX)
          goto err;

        _button_remove(obj, i, EINA_FALSE);
     }
   else
     goto err;

   *ret = content;
   return;

err:
   ERR("The part name is invalid! : popup=%p", obj);

   *ret = content;
}

static void
_elm_popup_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_popup_smart_focus_next(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *ao;
   Eina_List *items = NULL;

   Elm_Popup_Smart_Data *sd = _pd;

   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_TRUE;

   /* access */
   if (_elm_config->access_mode)
     {
        if (sd->title_text)
          {
             ao = _access_object_get(obj, ACCESS_TITLE_PART);
             items = eina_list_append(items, ao);
          }

        ao = _access_object_get(obj, ACCESS_BODY_PART);
        if (ao) items = eina_list_append(items, ao);
     }

   /* content area */
   if (sd->content) items = eina_list_append(items, sd->content_area);

   /* action area */
   if (sd->action_area) items = eina_list_append(items, sd->action_area);

   if (!elm_widget_focus_list_next_get(obj, items, eina_list_data_get, dir, next))
     *next = obj;

   return;
}

static void
_elm_popup_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_popup_smart_focus_direction(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *ao;
   Eina_List *items = NULL;

   Elm_Popup_Smart_Data *sd = _pd;

   Evas_Object *base = va_arg(*list, Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_TRUE;

   /* access */
   if (_elm_config->access_mode)
     {
        if (sd->title_text)
          {
             ao = _access_object_get(obj, ACCESS_TITLE_PART);
             items = eina_list_append(items, ao);
          }

        ao = _access_object_get(obj, ACCESS_BODY_PART);
        if (ao) items = eina_list_append(items, ao);
     }

   /* content area */
   if (sd->content) items = eina_list_append(items, sd->content_area);

   /* action area */
   if (sd->action_area) items = eina_list_append(items, sd->action_area);

   elm_widget_focus_list_direction_get
     (obj, base, items, eina_list_data_get, degree, direction, weight);

   return;
}

static void
_elm_popup_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   (void)src;
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   void *event_info = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Event_Key_Down *ev = event_info;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   if (!strcmp(ev->key, "Tab"))
     {
        if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
          elm_widget_focus_cycle(obj, ELM_FOCUS_PREVIOUS);
        else
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);

        goto success;
     }
   else if ((!strcmp(ev->key, "Left")) ||
            ((!strcmp(ev->key, "KP_Left")) && (!ev->string)))
     {
        elm_widget_focus_direction_go(obj, 270.0);
        goto success;
     }
   else if ((!strcmp(ev->key, "Right")) ||
            ((!strcmp(ev->key, "KP_Right")) && (!ev->string)))
     {
        elm_widget_focus_direction_go(obj, 90.0);
        goto success;
     }
   else if ((!strcmp(ev->key, "Up")) ||
            ((!strcmp(ev->key, "KP_Up")) && (!ev->string)))
     {
        elm_widget_focus_direction_go(obj, 0.0);
        goto success;
     }
   else if ((!strcmp(ev->key, "Down")) ||
            ((!strcmp(ev->key, "KP_Down")) && (!ev->string)))
     {
        elm_widget_focus_direction_go(obj, 180.0);
        goto success;
     }

   return;

success:
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_popup_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Popup_Smart_Data *priv = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);
   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   evas_object_size_hint_weight_set
     (wd->resize_obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (wd->resize_obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_widget_sub_object_add(eo_parent_get(obj), obj);

   if (!elm_layout_theme_set(obj, "popup", "base", elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");

   priv->notify = elm_notify_add(obj);
   elm_notify_align_set(priv->notify, 0.5, 0.5);
   elm_notify_allow_events_set(priv->notify, EINA_FALSE);
   evas_object_size_hint_weight_set
     (priv->notify, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (priv->notify, EVAS_HINT_FILL, EVAS_HINT_FILL);

   elm_object_style_set(priv->notify, "popup");

   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _on_show, NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _on_hide, NULL);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESTACK, _restack_cb, NULL);

   elm_layout_signal_callback_add
     (obj, "elm,state,title_area,visible", "elm", _layout_change_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,state,title_area,hidden", "elm", _layout_change_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,state,action_area,visible", "elm", _layout_change_cb, obj);
   elm_layout_signal_callback_add
     (obj, "elm,state,action_area,hidden", "elm", _layout_change_cb, obj);

   priv->content_area = elm_layout_add(obj);
   if (!elm_layout_theme_set
       (priv->content_area, "popup", "content", elm_widget_style_get(obj)))
     CRITICAL("Failed to set layout!");
   evas_object_event_callback_add
     (priv->content_area, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _size_hints_changed_cb, obj);

   priv->content_text_wrap_type = ELM_WRAP_MIXED;
   evas_object_smart_callback_add
     (priv->notify, "block,clicked", _block_clicked_cb, obj);

   evas_object_smart_callback_add(priv->notify, "timeout", _timeout_cb, obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   _visuals_set(obj);
   edje_object_message_signal_process(wd->resize_obj);
}

static void
_elm_popup_smart_parent_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Popup_Smart_Data *sd = _pd;
   Evas_Object *parent = va_arg(*list, Evas_Object *);

   elm_notify_parent_set(sd->notify, parent);
}

static void
_elm_popup_smart_access(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool is_access = va_arg(*list, int);
   _access_obj_process(obj, is_access);
}

EAPI Evas_Object *
elm_popup_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);

   /* access: parent could be any object such as elm_list which does
      not know elc_popup as its child object in the focus_next();    */
   Elm_Widget_Smart_Data *wd = eo_data_scope_get(obj, ELM_OBJ_WIDGET_CLASS);
   wd->highlight_root = EINA_TRUE;

   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI void
elm_popup_content_text_wrap_type_set(Evas_Object *obj,
                                     Elm_Wrap_Type wrap)
{
   ELM_POPUP_CHECK(obj);
   eo_do(obj, elm_obj_popup_content_text_wrap_type_set(wrap));
}

static void
_content_text_wrap_type_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Wrap_Type wrap = va_arg(*list, Elm_Wrap_Type);
   Elm_Popup_Smart_Data *sd = _pd;

   //Need to wrap the content text, so not allowing ELM_WRAP_NONE
   if (sd->content_text_wrap_type == ELM_WRAP_NONE) return;

   sd->content_text_wrap_type = wrap;
   if (sd->text_content_obj)
     elm_label_line_wrap_set(sd->text_content_obj, wrap);
}

EAPI Elm_Wrap_Type
elm_popup_content_text_wrap_type_get(const Evas_Object *obj)
{
   ELM_POPUP_CHECK(obj) ELM_WRAP_LAST;
   Elm_Wrap_Type ret = ELM_WRAP_LAST;
   eo_do((Eo *) obj, elm_obj_popup_content_text_wrap_type_get(&ret));
   return ret;
}

static void
_content_text_wrap_type_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Wrap_Type *ret = va_arg(*list, Elm_Wrap_Type *);
   Elm_Popup_Smart_Data *sd = _pd;

   *ret = sd->content_text_wrap_type;
}

/* keeping old externals orient api for notify, but taking away the
 * introduced deprecation warning by copying the deprecated code
 * here */
static Elm_Notify_Orient
_elm_notify_orient_get(const Evas_Object *obj)
{
   Elm_Notify_Orient orient;
   double horizontal, vertical;

   elm_notify_align_get(obj, &horizontal, &vertical);

   if ((horizontal == 0.5) && (vertical == 0.0))
     orient = ELM_NOTIFY_ORIENT_TOP;
   else if ((horizontal == 0.5) && (vertical == 0.5))
     orient = ELM_NOTIFY_ORIENT_CENTER;
   else if ((horizontal == 0.5) && (vertical == 1.0))
     orient = ELM_NOTIFY_ORIENT_BOTTOM;
   else if ((horizontal == 0.0) && (vertical == 0.5))
     orient = ELM_NOTIFY_ORIENT_LEFT;
   else if ((horizontal == 1.0) && (vertical == 0.5))
     orient = ELM_NOTIFY_ORIENT_RIGHT;
   else if ((horizontal == 0.0) && (vertical == 0.0))
     orient = ELM_NOTIFY_ORIENT_TOP_LEFT;
   else if ((horizontal == 1.0) && (vertical == 0.0))
     orient = ELM_NOTIFY_ORIENT_TOP_RIGHT;
   else if ((horizontal == 0.0) && (vertical == 1.0))
     orient = ELM_NOTIFY_ORIENT_BOTTOM_LEFT;
   else if ((horizontal == 1.0) && (vertical == 1.0))
     orient = ELM_NOTIFY_ORIENT_BOTTOM_RIGHT;
   else
     orient = ELM_NOTIFY_ORIENT_TOP;
   return orient;
}

static void
_elm_notify_orient_set(Evas_Object *obj,
                       Elm_Notify_Orient orient)
{
   double horizontal = 0, vertical = 0;

   switch (orient)
     {
      case ELM_NOTIFY_ORIENT_TOP:
         horizontal = 0.5; vertical = 0.0;
        break;

      case ELM_NOTIFY_ORIENT_CENTER:
         horizontal = 0.5; vertical = 0.5;
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM:
         horizontal = 0.5; vertical = 1.0;
        break;

      case ELM_NOTIFY_ORIENT_LEFT:
         horizontal = 0.0; vertical = 0.5;
        break;

      case ELM_NOTIFY_ORIENT_RIGHT:
         horizontal = 1.0; vertical = 0.5;
        break;

      case ELM_NOTIFY_ORIENT_TOP_LEFT:
         horizontal = 0.0; vertical = 0.0;
        break;

      case ELM_NOTIFY_ORIENT_TOP_RIGHT:
         horizontal = 1.0; vertical = 0.0;
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM_LEFT:
         horizontal = 0.0; vertical = 1.0;
        break;

      case ELM_NOTIFY_ORIENT_BOTTOM_RIGHT:
         horizontal = 1.0; vertical = 1.0;
        break;

      case ELM_NOTIFY_ORIENT_LAST:
        break;
     }

   elm_notify_align_set(obj, horizontal, vertical);
}

EAPI void
elm_popup_orient_set(Evas_Object *obj,
                     Elm_Popup_Orient orient)
{
   ELM_POPUP_CHECK(obj);
   eo_do(obj, elm_obj_popup_orient_set(orient));
}

static void
_orient_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Popup_Orient orient = va_arg(*list, Elm_Popup_Orient);
   Elm_Popup_Smart_Data *sd = _pd;

   if (orient >= ELM_POPUP_ORIENT_LAST) return;
   _elm_notify_orient_set(sd->notify, (Elm_Notify_Orient)orient);
}

EAPI Elm_Popup_Orient
elm_popup_orient_get(const Evas_Object *obj)
{
   ELM_POPUP_CHECK(obj) - 1;
   Elm_Popup_Orient ret = -1;
   eo_do((Eo *) obj, elm_obj_popup_orient_get(&ret));
   return ret;
}

static void
_orient_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Popup_Orient *ret = va_arg(*list, Elm_Popup_Orient *);
   Elm_Popup_Smart_Data *sd = _pd;

   *ret = (Elm_Popup_Orient)_elm_notify_orient_get(sd->notify);
}

EAPI void
elm_popup_timeout_set(Evas_Object *obj,
                      double timeout)
{
   ELM_POPUP_CHECK(obj);
   eo_do(obj, elm_obj_popup_timeout_set(timeout));
}

static void
_timeout_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double timeout = va_arg(*list, double);
   Elm_Popup_Smart_Data *sd = _pd;

   elm_notify_timeout_set(sd->notify, timeout);
}

EAPI double
elm_popup_timeout_get(const Evas_Object *obj)
{
   ELM_POPUP_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_popup_timeout_get(&ret));
   return ret;
}

static void
_timeout_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Popup_Smart_Data *sd = _pd;

   *ret = elm_notify_timeout_get(sd->notify);
}

EAPI void
elm_popup_allow_events_set(Evas_Object *obj,
                           Eina_Bool allow)
{
   ELM_POPUP_CHECK(obj);
   eo_do(obj, elm_obj_popup_allow_events_set(allow));
}

static void
_allow_events_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool allow = va_arg(*list, int);

   Eina_Bool allow_events = !!allow;
   Elm_Popup_Smart_Data *sd = _pd;

   elm_notify_allow_events_set(sd->notify, allow_events);
}

EAPI Eina_Bool
elm_popup_allow_events_get(const Evas_Object *obj)
{
   ELM_POPUP_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_popup_allow_events_get(&ret));
   return ret;
}

static void
_allow_events_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Popup_Smart_Data *sd = _pd;

   *ret =  elm_notify_allow_events_get(sd->notify);
}

EAPI Elm_Object_Item *
elm_popup_item_append(Evas_Object *obj,
                      const char *label,
                      Evas_Object *icon,
                      Evas_Smart_Cb func,
                      const void *data)
{
   ELM_POPUP_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_popup_item_append(label, icon, func, data, &ret));
   return ret;
}

static void
_item_append(Eo *obj, void *_pd, va_list *list)
{
   const char *label = va_arg(*list, const char *);
   Evas_Object *icon = va_arg(*list, Evas_Object *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   if (ret) *ret = NULL;

   Evas_Object *prev_content;
   Elm_Popup_Item *item;

   Elm_Popup_Smart_Data *sd = _pd;

   item = elm_widget_item_new(obj, Elm_Popup_Item);
   if (!item) return;
   if (sd->content || sd->text_content_obj)
     {
        prev_content = elm_layout_content_get
            (sd->content_area, CONTENT_PART);
        if (prev_content)
          evas_object_del(prev_content);
     }

   //The first item is appended.
   if (!sd->items)
     _list_add(obj);

   item->func = func;
   item->base.data = data;

   _item_new(item);
   _item_icon_set(item, icon);
   _item_text_set(item, label);

   elm_box_pack_end(sd->box, VIEW(item));
   sd->items = eina_list_append(sd->items, item);

   _scroller_size_calc(obj);
   elm_layout_sizing_eval(obj);

   if (ret) *ret = (Elm_Object_Item *)item;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_popup_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_popup_smart_del),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_PARENT_SET), _elm_popup_smart_parent_set),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_popup_smart_event),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME), _elm_popup_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_TRANSLATE), _elm_popup_smart_translate),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_popup_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT),  _elm_popup_smart_focus_next),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACCESS), _elm_popup_smart_access),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_popup_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_popup_smart_focus_direction),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_popup_smart_sub_object_del),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_popup_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_popup_smart_content_get),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_popup_smart_content_unset),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_SET), _elm_popup_smart_text_set),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_TEXT_GET), _elm_popup_smart_text_get),
        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_popup_smart_sizing_eval),

        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_CONTENT_TEXT_WRAP_TYPE_SET), _content_text_wrap_type_set),
        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_CONTENT_TEXT_WRAP_TYPE_GET), _content_text_wrap_type_get),
        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ORIENT_SET), _orient_set),
        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ORIENT_GET), _orient_get),
        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_TIMEOUT_SET), _timeout_set),
        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_TIMEOUT_GET), _timeout_get),
        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ALLOW_EVENTS_SET), _allow_events_set),
        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ALLOW_EVENTS_GET), _allow_events_get),
        EO_OP_FUNC(ELM_OBJ_POPUP_ID(ELM_OBJ_POPUP_SUB_ID_ITEM_APPEND), _item_append),
        EO_OP_FUNC_SENTINEL
  };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}
static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_CONTENT_TEXT_WRAP_TYPE_SET, "Sets the wrapping type of content text packed in content."),
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_CONTENT_TEXT_WRAP_TYPE_GET, "Returns the wrapping type of content text packed in content area of."),
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_ORIENT_SET, "Sets the orientation of the popup in the parent region."),
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_ORIENT_GET, "Returns the orientation of Popup."),
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_TIMEOUT_SET, "Sets a timeout to hide popup automatically."),
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_TIMEOUT_GET, "Returns the timeout value set to the popup (in seconds)."),
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_ALLOW_EVENTS_SET, "Sets whether events should be passed to by a click outside."),
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_ALLOW_EVENTS_GET, "Returns value indicating whether allow event is enabled or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_POPUP_SUB_ID_ITEM_APPEND, "Add a new item to a Popup object."),
     EO_OP_DESCRIPTION_SENTINEL
};
static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_POPUP_BASE_ID, op_desc, ELM_OBJ_POPUP_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Popup_Smart_Data),
     _class_constructor,
     NULL
};
EO_DEFINE_CLASS(elm_obj_popup_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, NULL);
