#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_popup.h"

#define ELM_POPUP_ACTION_BUTTON_MAX 3

EAPI const char ELM_POPUP_SMART_NAME[] = "elm_popup";

static void _button_remove(Evas_Object *, int, Eina_Bool);

static const char SIG_BLOCK_CLICKED[] = "block,clicked";
static const char SIG_TIMEOUT[] = "timeout";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_BLOCK_CLICKED, ""},
   {SIG_TIMEOUT, ""},
   {NULL, NULL}
};

EVAS_SMART_SUBCLASS_NEW
  (ELM_POPUP_SMART_NAME, _elm_popup, Elm_Popup_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks);

static void
_visuals_set(Evas_Object *obj)
{
   ELM_POPUP_DATA_GET(obj, sd);

   if (!sd->title_text && !sd->title_icon)
     elm_layout_signal_emit(obj, "elm,state,title_area,hidden", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,title_area,visible", "elm");

   if (sd->button_count)
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

static void
_on_show(void *data __UNUSED__,
         Evas *e __UNUSED__,
         Evas_Object *obj,
         void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(obj, sd);

   evas_object_show(sd->notify);

   /* yeah, ugly, but again, this widget needs a rewrite */
   elm_object_content_set(sd->notify, obj);

   // XXX: ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        evas_object_focus_set(sd->title_access_obj, EINA_TRUE);
        _elm_access_highlight_set(sd->title_access_obj);
     }
}

static void
_on_hide(void *data __UNUSED__,
            Evas *e __UNUSED__,
            Evas_Object *obj,
            void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(obj, sd);

   evas_object_hide(sd->notify);

   elm_object_content_unset(sd->notify);
}

static void
_scroller_size_calc(Evas_Object *obj)
{
   Evas_Coord h;
   Evas_Coord h_title = 0;
   Evas_Coord h_content = 0;
   Evas_Coord h_action_area = 0;
   const char *action_area_height;

   ELM_POPUP_DATA_GET(obj, sd);

   if (!sd->items) return;

   sd->scr_size_recalc = EINA_FALSE;
   sd->max_sc_h = -1;
   sd->max_sc_w = -1;
   evas_object_geometry_get(sd->notify, NULL, NULL, NULL, &h);
   if (sd->title_text || sd->title_icon)
     edje_object_part_geometry_get(ELM_WIDGET_DATA(sd)->resize_obj,
                                   "elm.bg.title", NULL, NULL, NULL, &h_title);
   if (sd->button_count)
     {
        action_area_height = edje_object_data_get(
            elm_layout_edje_get(sd->action_area), "action_area_height");
        if (action_area_height)
          h_action_area =
            (int)(atoi(action_area_height)
                  * elm_config_scale_get() * elm_object_scale_get(obj));
     }

   h_content = h - (h_title + h_action_area);
   sd->max_sc_h = h_content;
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

   EINA_LIST_FREE (sd->items, item)
     elm_widget_item_del(item);

   sd->items = NULL;
}

static void
_elm_popup_smart_del(Evas_Object *obj)
{
   unsigned int i;

   ELM_POPUP_DATA_GET(obj, sd);

   evas_object_smart_callback_del
     (sd->notify, "block,clicked", _block_clicked_cb);
   evas_object_smart_callback_del(sd->notify, "timeout", _timeout_cb);

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

   ELM_WIDGET_CLASS(_elm_popup_parent_sc)->base.del(obj);
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
     EINA_LIST_FOREACH (sd->items, elist, item)
       edje_object_mirrored_set(VIEW(item), rtl);
}

static Eina_Bool
_elm_popup_smart_theme(Evas_Object *obj)
{
   Elm_Popup_Item *item;
   unsigned int i = 0;
   Eina_List *elist;
   char buf[128];

   ELM_POPUP_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_popup_parent_sc)->theme(obj))
     return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   elm_object_style_set(sd->notify, elm_widget_style_get(obj));

   if (sd->button_count)
     {
        snprintf(buf, sizeof(buf), "buttons%u", sd->button_count);
        elm_layout_theme_set(sd->action_area, "popup", buf,
                             elm_widget_style_get(obj));
        for (i = 0; i < ELM_POPUP_ACTION_BUTTON_MAX; i++)
          {
             if (!sd->buttons[i]) continue;
             elm_object_style_set(sd->buttons[i]->btn, buf);
          }
     }
   elm_layout_theme_set(sd->content_area, "popup", "content",
                        elm_widget_style_get(obj));
   if (sd->text_content_obj)
     {
        snprintf(buf, sizeof(buf), "popup/%s", elm_widget_style_get(obj));
        elm_object_style_set(sd->text_content_obj, buf);
     }
   else if (sd->items)
     {
        EINA_LIST_FOREACH (sd->items, elist, item)
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
   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
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
_elm_popup_smart_sizing_eval(Evas_Object *obj)
{
   Eina_List *elist;
   Elm_Popup_Item *item;
   Evas_Coord h_box = 0, minh_box = 0;
   Evas_Coord minw = -1, minh = -1, maxw = -1, maxh = -1;

   ELM_POPUP_DATA_GET(obj, sd);

   if (sd->items)
     {
        EINA_LIST_FOREACH (sd->items, elist, item)
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

   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &minw, &minh);

   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
}

static Eina_Bool
_elm_popup_smart_sub_object_del(Evas_Object *obj,
                                Evas_Object *sobj)
{
   Elm_Popup_Item *item;

   ELM_POPUP_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_popup_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   if (sobj == sd->title_icon)
     {
        elm_layout_signal_emit(obj, "elm,state,title,icon,hidden", "elm");
        edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
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

   return EINA_TRUE;
}

static void
_on_content_del(void *data,
                Evas *e __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__)
{
   ELM_POPUP_DATA_GET(data, sd);

   sd->content = NULL;
   edje_object_part_unswallow
       (data, edje_object_part_swallow_get(data, "elm.swallow.content"));
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
   edje_object_part_unswallow
       (data, edje_object_part_swallow_get(data, "elm.swallow.content"));
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

   if (!sd->button_count) return;

   if (!sd->buttons[pos]) return;

   if (delete) evas_object_del(sd->buttons[pos]->btn);

   evas_object_event_callback_del
     (sd->buttons[pos]->btn, EVAS_CALLBACK_DEL, _on_button_del);
   free(sd->buttons[pos]);

   sd->buttons[pos] = NULL;
   sd->button_count -= 1;

   if (!sd->no_shift)
     {
        /* shift left the remaining buttons */
        for (i = pos; i < ELM_POPUP_ACTION_BUTTON_MAX - 1; i++)
          {
             sd->buttons[i] = sd->buttons[i + 1];

             snprintf(buf, sizeof(buf), "actionbtn%u", pos + 1);
             elm_object_part_content_unset(sd->action_area, buf);
             snprintf(buf, sizeof(buf), "actionbtn%u", pos);
             elm_object_part_content_set
               (sd->action_area, buf, sd->buttons[i]->btn);
          }
     }

   if (!sd->button_count)
     {
        _visuals_set(obj);
        edje_object_part_unswallow
          (obj, edje_object_part_swallow_get(obj, "elm.swallow.action_area"));
        evas_object_hide(sd->action_area);
        edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
     }
   else
     {
        snprintf(buf, sizeof(buf), "buttons%u", sd->button_count);
        elm_layout_theme_set
          (sd->action_area, "popup", buf, elm_widget_style_get(obj));
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

   sd->tbl = elm_table_add(obj);

   evas_object_event_callback_add
     (sd->tbl, EVAS_CALLBACK_DEL, _on_table_del, obj);

   edje_object_part_swallow
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.content", sd->tbl);
   evas_object_size_hint_weight_set
     (sd->tbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->tbl, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(sd->tbl);

   sd->spacer = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(sd->spacer, 0, 0, 0, 0);
   elm_table_pack(sd->tbl, sd->spacer, 0, 0, 1, 1);

   //Scroller
   sd->scr = elm_scroller_add(obj);
   elm_scroller_content_min_limit(sd->scr, EINA_TRUE, EINA_FALSE);
   elm_scroller_bounce_set(sd->scr, EINA_FALSE, EINA_TRUE);
   evas_object_size_hint_weight_set
     (sd->scr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(sd->scr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add
     (sd->scr, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _size_hints_changed_cb,
     obj);
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
          (VIEW(item), "elm.swallow.content", item->icon);
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
   Elm_Popup_Item *item = (Elm_Popup_Item *)it;

   ELM_POPUP_ITEM_CHECK_OR_RETURN(it);

   edje_object_signal_emit(VIEW(item), emission, source);
}

static void
_item_new(Elm_Popup_Item *item)
{
   ELM_POPUP_DATA_GET(WIDGET(item), sd);

   elm_widget_item_text_set_hook_set(item, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(item, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(item, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(item, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(item, _item_content_unset_hook);
   elm_widget_item_disable_hook_set(item, _item_disable_hook);
   elm_widget_item_del_pre_hook_set(item, _item_del_pre_hook);
   elm_widget_item_signal_emit_hook_set(item, _item_signal_emit_hook);
   VIEW(item) = edje_object_add
       (evas_object_evas_get(ELM_WIDGET_DATA(sd)->obj));
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
   Eina_Bool title_visibility_old, title_visibility_current;

   ELM_POPUP_DATA_GET(obj, sd);

   if (sd->title_text == text) return EINA_TRUE;

   title_visibility_old = (sd->title_text) || (sd->title_icon);
   eina_stringshare_replace(&sd->title_text, text);

   //bare edje here because we're inside the hook, already
   edje_object_part_text_escaped_set
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.text.title", text);

   // XXX: ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        sd->title_access_obj = _elm_access_edje_object_part_object_register
            (ELM_WIDGET_DATA(sd)->resize_obj, ELM_WIDGET_DATA(sd)->resize_obj,
            "elm.text.title");
        _elm_access_text_set(_elm_access_object_get(sd->title_access_obj),
                             ELM_ACCESS_TYPE, E_("popup title"));
        elm_widget_sub_object_add(obj, sd->title_access_obj);
     }

   if (sd->title_text)
     elm_layout_signal_emit(obj, "elm,state,title,text,visible", "elm");
   else
     elm_layout_signal_emit(obj, "elm,state,title,text,hidden", "elm");

   title_visibility_current = (sd->title_text) || (sd->title_icon);

   if (title_visibility_old != title_visibility_current)
     _visuals_set(obj);

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_content_text_set(Evas_Object *obj,
                  const char *text)
{
   Evas_Object *prev_content;
   char buf[128];

   ELM_POPUP_DATA_GET(obj, sd);

   if (sd->items)
     {
        _items_remove(sd);
        _list_del(sd);
     }

   prev_content = elm_layout_content_get
       (sd->content_area, "elm.swallow.content");

   if (prev_content)
     evas_object_del(prev_content);

   if (!text) goto end;

   edje_object_part_swallow
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.content",
     sd->content_area);
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
     (sd->content_area, "elm.swallow.content", sd->text_content_obj);

   // XXX: ACCESS
   _elm_access_text_set
     (_elm_access_object_get(sd->text_content_obj),
     ELM_ACCESS_TYPE, E_("popup label"));

end:
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_popup_smart_text_set(Evas_Object *obj,
                          const char *part,
                          const char *label)
{
   if (!part || !strcmp(part, "default"))
     return _content_text_set(obj, label);
   else if (!strcmp(part, "title,text"))
     return _title_text_set(obj, label);
   else
     return _elm_popup_parent_sc->text_set(obj, part, label);
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

static const char *
_elm_popup_smart_text_get(const Evas_Object *obj,
                          const char *part)
{
   const char *str = NULL;

   if (!part || !strcmp(part, "default"))
     str = _content_text_get(obj);
   else if (!strcmp(part, "title,text"))
     str = _title_text_get(obj);
   else
     str = _elm_popup_parent_sc->text_get(obj, part);

   return str;
}

static Eina_Bool
_title_icon_set(Evas_Object *obj,
                Evas_Object *icon)
{
   Eina_Bool title_visibility_old, title_visibility_current;

   ELM_POPUP_DATA_GET(obj, sd);

   if (sd->title_icon == icon) return EINA_TRUE;
   title_visibility_old = (sd->title_text) || (sd->title_icon);
   if (sd->title_icon) evas_object_del(sd->title_icon);

   sd->title_icon = icon;
   title_visibility_current = (sd->title_text) || (sd->title_icon);

   //bare edje here because we're already in content_set virtual
   edje_object_part_swallow
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.title.icon",
     sd->title_icon);

   if (sd->title_icon)
     elm_layout_signal_emit(obj, "elm,state,title,icon,visible", "elm");
   if (title_visibility_old != title_visibility_current) _visuals_set(obj);

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_content_set(Evas_Object *obj,
             Evas_Object *content)
{
   Evas_Object *prev_content;

   ELM_POPUP_DATA_GET(obj, sd);

   if (sd->content && sd->content == content) return EINA_TRUE;
   if (sd->items)
     {
        _items_remove(sd);
        _list_del(sd);
     }
   prev_content =
     elm_layout_content_get(sd->content_area, "elm.swallow.content");
   if (prev_content)
     evas_object_del(prev_content);

   sd->content = content;
   if (content)
     {
        //bare edje as to avoid loop
        edje_object_part_swallow
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.content",
          sd->content_area);

        elm_layout_content_set
          (sd->content_area, "elm.swallow.content", content);
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
   int i = 0;
   Action_Area_Data *adata;
   char buf[128];

   ELM_POPUP_DATA_GET(obj, sd);

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
   elm_layout_theme_set
     (sd->action_area, "popup", buf, elm_widget_style_get(obj));

   adata = ELM_NEW(Action_Area_Data);
   adata->obj = obj;
   adata->btn = btn;

   evas_object_event_callback_add
     (btn, EVAS_CALLBACK_DEL, _on_button_del, obj);

   sd->buttons[idx] = adata;

   snprintf(buf, sizeof(buf), "actionbtn%u", idx + 1);
   elm_object_part_content_set
     (sd->action_area, buf, sd->buttons[idx]->btn);
   evas_object_show(sd->buttons[i]->btn);

   edje_object_part_swallow
     (ELM_WIDGET_DATA(sd)->resize_obj, "elm.swallow.action_area",
     sd->action_area);
   if (sd->button_count == 1) _visuals_set(obj);

   edje_object_message_signal_process(ELM_WIDGET_DATA(sd)->resize_obj);
   if (sd->items) _scroller_size_calc(obj);

   elm_layout_sizing_eval(obj);
}

static Eina_Bool
_elm_popup_smart_content_set(Evas_Object *obj,
                             const char *part,
                             Evas_Object *content)
{
   unsigned int i;

   if (!part || !strcmp(part, "default"))
     return _content_set(obj, content);
   else if (!strcmp(part, "title,icon"))
     return _title_icon_set(obj, content);
   else if (!strncmp(part, "button", 6))
     {
        i = atoi(part + 6) - 1;

        if (i >= ELM_POPUP_ACTION_BUTTON_MAX)
          goto err;

        _action_button_set(obj, content, i);

        return EINA_TRUE;
     }

err:
   ERR("The part name is invalid! : popup=%p", obj);

   return EINA_FALSE;
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
   unsigned int num = idx - 1;
   Evas_Object *button = NULL;

   ELM_POPUP_DATA_GET(obj, sd);
   if (!sd->button_count) return NULL;

   if (sd->buttons[num])
     button = sd->buttons[num]->btn;

   return button;
}

static Evas_Object *
_elm_popup_smart_content_get(const Evas_Object *obj,
                             const char *part)
{
   Evas_Object *content = NULL;
   unsigned int i;

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

   return content;

err:
   WRN("The part name is invalid! : popup=%p", obj);
   return content;
}

static Evas_Object *
_content_unset(Evas_Object *obj)
{
   Evas_Object *content;

   ELM_POPUP_DATA_GET(obj, sd);

   if (!sd->content) return NULL;

   evas_object_event_callback_del
     (sd->content, EVAS_CALLBACK_DEL, _on_content_del);

   content = elm_layout_content_unset(sd->content_area, "elm.swallow.content");
   sd->content = NULL;

   elm_layout_sizing_eval(obj);

   return content;
}

static Evas_Object *
_title_icon_unset(Evas_Object *obj)
{
   Evas_Object *icon;

   ELM_POPUP_DATA_GET(obj, sd);

   if (!sd->title_icon) return NULL;

   icon = sd->title_icon;
   edje_object_part_unswallow(ELM_WIDGET_DATA(sd)->resize_obj, sd->title_icon);
   sd->title_icon = NULL;

   return icon;
}

static Evas_Object *
_elm_popup_smart_content_unset(Evas_Object *obj,
                               const char *part)
{
   Evas_Object *content = NULL;
   unsigned int i;

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

   return content;

err:
   ERR("The part name is invalid! : popup=%p", obj);

   return content;
}

static Eina_Bool
_elm_popup_smart_focus_next(const Evas_Object *obj,
                            Elm_Focus_Direction dir,
                            Evas_Object **next)
{
   ELM_POPUP_DATA_GET(obj, sd);

   if (!elm_widget_focus_next_get(sd->notify, dir, next))
     {
        // XXX: ACCESS
        if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
          {
             *next = sd->title_access_obj;
             return EINA_TRUE;
          }
        elm_widget_focused_object_clear(sd->notify);
        elm_widget_focus_next_get(sd->notify, dir, next);
     }

   return EINA_TRUE;
}

static Eina_Bool
_elm_popup_smart_focus_direction(const Evas_Object *obj,
                                 const Evas_Object *base,
                                 double degree,
                                 Evas_Object **direction,
                                 double *weight)
{
   ELM_POPUP_DATA_GET(obj, sd);

   return elm_widget_focus_direction_get
            (sd->notify, base, degree, direction, weight);
}

static void
_elm_popup_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Popup_Smart_Data);

   ELM_WIDGET_CLASS(_elm_popup_parent_sc)->base.add(obj);

   elm_layout_theme_set(obj, "popup", "base", elm_widget_style_get(obj));

   evas_object_size_hint_weight_set
     (ELM_WIDGET_DATA(priv)->resize_obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (ELM_WIDGET_DATA(priv)->resize_obj, EVAS_HINT_FILL, EVAS_HINT_FILL);

   priv->notify = elm_notify_add(obj);
   elm_notify_orient_set(priv->notify, ELM_NOTIFY_ORIENT_CENTER);
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
   elm_layout_theme_set
     (priv->content_area, "popup", "content", elm_widget_style_get(obj));
   priv->action_area = elm_layout_add(obj);
   evas_object_size_hint_weight_set(priv->action_area, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(priv->action_area, EVAS_HINT_FILL,
                                   EVAS_HINT_FILL);
   evas_object_event_callback_add
     (priv->action_area, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _size_hints_changed_cb, obj);
   evas_object_event_callback_add
     (priv->content_area, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _size_hints_changed_cb, obj);

   priv->content_text_wrap_type = ELM_WRAP_MIXED;
   evas_object_smart_callback_add
     (priv->notify, "block,clicked", _block_clicked_cb, obj);

   evas_object_smart_callback_add(priv->notify, "timeout", _timeout_cb, obj);

   elm_widget_can_focus_set(obj, EINA_FALSE);

   _visuals_set(obj);
}

static void
_elm_popup_smart_parent_set(Evas_Object *obj,
                            Evas_Object *parent)
{
   ELM_POPUP_DATA_GET(obj, sd);

   elm_notify_parent_set(sd->notify, parent);
}

static void
_elm_popup_smart_set_user(Elm_Popup_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_popup_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_popup_smart_del;

   ELM_WIDGET_CLASS(sc)->parent_set = _elm_popup_smart_parent_set;
   ELM_WIDGET_CLASS(sc)->theme = _elm_popup_smart_theme;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_popup_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->focus_direction = _elm_popup_smart_focus_direction;
   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_popup_smart_sub_object_del;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_popup_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_popup_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_popup_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->text_set = _elm_popup_smart_text_set;
   ELM_LAYOUT_CLASS(sc)->text_get = _elm_popup_smart_text_get;
   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_popup_smart_sizing_eval;
}

EAPI const Elm_Popup_Smart_Class *
elm_popup_smart_class_get(void)
{
   static Elm_Popup_Smart_Class _sc =
     ELM_POPUP_SMART_CLASS_INIT_NAME_VERSION(ELM_POPUP_SMART_NAME);
   static const Elm_Popup_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_popup_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_popup_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_popup_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI void
elm_popup_content_text_wrap_type_set(Evas_Object *obj,
                                     Elm_Wrap_Type wrap)
{
   ELM_POPUP_CHECK(obj);
   ELM_POPUP_DATA_GET(obj, sd);

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
   ELM_POPUP_DATA_GET(obj, sd);

   return sd->content_text_wrap_type;
}

EAPI void
elm_popup_orient_set(Evas_Object *obj,
                     Elm_Popup_Orient orient)
{
   ELM_POPUP_CHECK(obj);
   ELM_POPUP_DATA_GET(obj, sd);

   if (orient >= ELM_POPUP_ORIENT_LAST) return;
   elm_notify_orient_set(sd->notify, (Elm_Notify_Orient)orient);
}

EAPI Elm_Popup_Orient
elm_popup_orient_get(const Evas_Object *obj)
{
   ELM_POPUP_CHECK(obj) - 1;
   ELM_POPUP_DATA_GET(obj, sd);

   return (Elm_Popup_Orient)elm_notify_orient_get(sd->notify);
}

EAPI void
elm_popup_timeout_set(Evas_Object *obj,
                      double timeout)
{
   ELM_POPUP_CHECK(obj);
   ELM_POPUP_DATA_GET(obj, sd);

   elm_notify_timeout_set(sd->notify, timeout);
}

EAPI double
elm_popup_timeout_get(const Evas_Object *obj)
{
   ELM_POPUP_CHECK(obj) 0.0;
   ELM_POPUP_DATA_GET(obj, sd);

   return elm_notify_timeout_get(sd->notify);
}

EAPI void
elm_popup_allow_events_set(Evas_Object *obj,
                           Eina_Bool allow)
{
   Eina_Bool allow_events = !!allow;

   ELM_POPUP_CHECK(obj);
   ELM_POPUP_DATA_GET(obj, sd);

   elm_notify_allow_events_set(sd->notify, allow_events);
}

EAPI Eina_Bool
elm_popup_allow_events_get(const Evas_Object *obj)
{
   ELM_POPUP_CHECK(obj) EINA_FALSE;
   ELM_POPUP_DATA_GET(obj, sd);

   return elm_notify_allow_events_get(sd->notify);
}

EAPI Elm_Object_Item *
elm_popup_item_append(Evas_Object *obj,
                      const char *label,
                      Evas_Object *icon,
                      Evas_Smart_Cb func,
                      const void *data)
{
   Evas_Object *prev_content;
   Elm_Popup_Item *item;

   ELM_POPUP_CHECK(obj) NULL;
   ELM_POPUP_DATA_GET(obj, sd);

   item = elm_widget_item_new(obj, Elm_Popup_Item);
   if (!item) return NULL;
   if (sd->content || sd->text_content_obj)
     {
        prev_content = elm_layout_content_get
            (sd->content_area, "elm.swallow.content");
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

   return (Elm_Object_Item *)item;
}
