#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_diskselector.h"
#include "elm_interface_scrollable.h"

#define DISPLAY_ITEM_NUM_MIN 3

EAPI Eo_Op ELM_OBJ_DISKSELECTOR_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_DISKSELECTOR_CLASS

#define MY_CLASS_NAME "Elm_Diskselector"
#define MY_CLASS_NAME_LEGACY "elm_diskselector"

static const char SIG_SELECTED[] = "selected";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_SELECTED, ""},
   {SIG_CLICKED, ""},
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_FOCUSED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_UNFOCUSED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static void
_elm_diskselector_smart_translate(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Diskselector_Smart_Data *sd = _pd;
   Elm_Diskselector_Item *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_widget_item_translate(it);

   if (ret) *ret = EINA_TRUE;
}

static void
_selected_item_indicate(Elm_Diskselector_Item *item)
{
   Eina_List *l;
   Elm_Diskselector_Item *it;

   ELM_DISKSELECTOR_DATA_GET(WIDGET(item), sd);

   if (!item->label) return;

   EINA_LIST_FOREACH(sd->r_items, l, it)
     {
        if (it->label && !strcmp(it->label, item->label))
          edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,default", "elm");
     }
}

static void
_item_select(Elm_Diskselector_Item *it)
{
   if (!it) return;

   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   sd->selected_item = it;
   _selected_item_indicate(sd->selected_item);

   if (it->func) it->func((void *)it->base.data, WIDGET(it), it);
   evas_object_smart_callback_call(WIDGET(it), SIG_SELECTED, it);
}

static int
_letters_count(const char *str)
{
   int pos = 0;
   int code = 0, chnum;

   for (chnum = 0;; chnum++)
     {
        pos = evas_string_char_next_get(str, pos, &code);
        if (code == 0) break;
     }

   return chnum;
}

static int
_letters_check(const char *str,
               int length)
{
   int pos = 0;
   int code = 0, chnum;

   for (chnum = 0;; chnum++)
     {
        if (chnum == length) break;
        pos = evas_string_char_next_get(str, pos, &code);
        if (code == 0) break;
     }

   return pos;
}
static void
_item_signal_emit(Elm_Diskselector_Item *it)
{
   if ((it->icon) && (!it->label))
     edje_object_signal_emit(VIEW(it), "elm,state,icon,only", "elm");
   else if ((!it->icon) && (it->label))
     edje_object_signal_emit(VIEW(it), "elm,state,text,only", "elm");
   else
     edje_object_signal_emit(VIEW(it), "elm,state,text,icon", "elm");
}

static Eina_Bool
_string_check_idle_enterer_cb(void *data)
{
   Evas_Object *obj = data;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   int mid, steps, length, diff;
   Elm_Diskselector_Item *it;
   Eina_List *list, *l;
   Evas_Coord ox, ow;
   char buf[1024];

   evas_object_geometry_get(obj, &ox, NULL, &ow, NULL);

   if (ow <= 0)
     {
        sd->string_check_idle_enterer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   if (!sd->init)
     {
        sd->string_check_idle_enterer = NULL;
        return ECORE_CALLBACK_CANCEL;
     }

   if (!sd->round)
     list = sd->items;
   else
     list = sd->r_items;

   EINA_LIST_FOREACH(list, l, it)
     {
        Evas_Coord x, w;
        int len;

        if (it->icon && !it->label)
          continue;

        evas_object_geometry_get(VIEW(it), &x, NULL, &w, NULL);
        /* item not visible */
        if ((x + w <= ox) || (x >= ox + ow))
          continue;

        len = _letters_count(it->label);
        // FIXME: len should be # of ut8f letters. ie count using utf8
        // string walk, not stringshare len
        //len = eina_stringshare_strlen(it->label);

        if (x <= ox + 5)
          edje_object_signal_emit(VIEW(it), "elm,state,left_side", "elm");
        else if (x + w >= ox + ow - 5)
          edje_object_signal_emit(VIEW(it), "elm,state,right_side", "elm");
        else
          {
             if ((sd->len_threshold) && (len > sd->len_threshold))
               edje_object_signal_emit
                 (VIEW(it), "elm,state,center_small", "elm");
             else
               edje_object_signal_emit(VIEW(it), "elm,state,center", "elm");
          }

        // if len is les that the limit len, skip anyway
        if (len <= sd->len_side)
          continue;

        steps = len - sd->len_side + 1;
        mid = x + w / 2;
        if (mid <= ox + ow / 2)
          diff = (ox + ow / 2) - mid;
        else
          diff = mid - (ox + ow / 2);

        length = len - (int)(diff * steps / (ow / 3));
        length = MAX(length, sd->len_side);
        // limit string len to "length" ut8f chars
        length = _letters_check(it->label, length);
        // cut it off at byte mark returned form _letters_check
        strncpy(buf, it->label, length);
        buf[length] = '\0';
        edje_object_part_text_escaped_set(VIEW(it), "elm.text", buf);
     }

   sd->string_check_idle_enterer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

static void
_string_check(void *data)
{
   Evas_Object *obj = data;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   ELM_SAFE_FREE(sd->string_check_idle_enterer, ecore_idle_enterer_del);

   // call string check idle enterer directly
   _string_check_idle_enterer_cb(data);
}

static Eina_Bool
_scroller_move(void *data)
{
   Evas_Object *obj = data;
   Elm_Diskselector_Item *dit;
   Eina_List *list, *l;
   Evas_Coord y, w, h;
   int i;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (!sd->round)
     {
        i = 0;
        list = sd->items;
     }
   else
     {
        i = 1;
        list = sd->r_items;
     }

   EINA_LIST_FOREACH(list, l, dit)
     {
        if (sd->selected_item == dit)
          break;
        i++;
     }

   if (sd->round) i -= CEIL(sd->display_item_num);

   if (!dit)
     {
        sd->selected_item =
          (Elm_Diskselector_Item *)eina_list_nth(sd->items, 0);
        goto end;
     }

   evas_object_geometry_get(obj, NULL, &y, &w, &h);
   eo_do(obj, elm_interface_scrollable_content_region_show(w / sd->display_item_num * i, y, w, h));
   _item_select(dit);

   sd->init = EINA_TRUE;
   _string_check(obj);

end:
   sd->scroller_move_idle_enterer = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_resize_cb(void *data EINA_UNUSED,
           Evas *e EINA_UNUSED,
           Evas_Object *obj,
           void *event_info EINA_UNUSED)
{
   Evas_Coord w, h, vw = 0, vh = 0, mw = 0, mh = 0;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if ((sd->minw == -1) && (sd->minh == -1))
     elm_coords_finger_size_adjust(6, &sd->minw, 1, &sd->minh);

   eo_do(obj, elm_interface_scrollable_content_viewport_size_get(&vw, &vh));
   if (sd->items)
     {
        Elm_Diskselector_Item *it = sd->items->data;
        evas_object_size_hint_min_get(VIEW(it), &mw, &mh);
     }
   if (sd->minh < mh) sd->minh = mh;
   edje_object_size_min_restricted_calc
     (wd->resize_obj, &sd->minw, &sd->minh, sd->minw,
     sd->minh);

   evas_object_size_hint_min_set(obj, sd->minw, sd->minh);
   evas_object_size_hint_max_set(obj, -1, -1);

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (sd->round)
     evas_object_resize(sd->main_box, (w / sd->display_item_num)
                        * (sd->item_count + (CEIL(sd->display_item_num) * 2)),
                        h);
   else
     evas_object_resize(sd->main_box, (w / sd->display_item_num)
                        * (sd->item_count + eina_list_count(sd->left_blanks)
                           + eina_list_count(sd->right_blanks)), h);

   eo_do(obj, elm_interface_scrollable_paging_set(0, 0, (int)(w / sd->display_item_num), 0));

   if (!sd->scroller_move_idle_enterer)
     sd->scroller_move_idle_enterer = ecore_idle_enterer_before_add(_scroller_move, obj);
}

static void
_item_del(Elm_Diskselector_Item *item)
{
   if (!item) return;

   ELM_DISKSELECTOR_DATA_GET(WIDGET(item), sd);
   sd->item_count -= 1;
   eina_stringshare_del(item->label);
   evas_object_del(item->icon);
}

static void
_sizing_eval(Evas_Object *obj)
{
   _resize_cb(NULL, NULL, obj, NULL);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *item)
{
   Elm_Diskselector_Item *it, *item2, *dit;
   Eina_List *l;
   int i = 0;

   it = (Elm_Diskselector_Item *)item;
   Evas_Object *obj = WIDGET(it);

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   elm_box_unpack(sd->main_box, VIEW(it));

   if (sd->round)
     sd->r_items = eina_list_remove(sd->r_items, it);

   sd->items = eina_list_remove(sd->items, it);

   if (sd->selected_item == it)
     {
        if (sd->items)
          {
             sd->selected_item = (Elm_Diskselector_Item *)eina_list_nth(sd->items, 0);
             _selected_item_indicate(sd->selected_item);
          }
        else
          sd->selected_item = NULL;
     }

   _item_del(it);

   if (sd->round)
     {
        if (!sd->item_count)
          {
             evas_object_hide(sd->VIEW(first));
             evas_object_hide(sd->VIEW(second));
             evas_object_hide(sd->VIEW(last));
             evas_object_hide(sd->VIEW(s_last));

             EINA_LIST_FOREACH(sd->under_items, l, item2)
               evas_object_hide(VIEW(item2));

             EINA_LIST_FOREACH(sd->over_items, l, item2)
               evas_object_hide(VIEW(item2));
          }
        else
          {
             dit = eina_list_nth(sd->items, 0);
             if (dit)
               {
                  eina_stringshare_replace(&sd->first->label, dit->label);
                  edje_object_part_text_escaped_set
                    (sd->VIEW(first), "elm.text", sd->first->label);
               }

             dit = eina_list_nth(sd->items, 1);
             if (dit)
               {
                  eina_stringshare_replace(&sd->second->label, dit->label);
                  edje_object_part_text_escaped_set
                    (sd->VIEW(second), "elm.text", sd->second->label);
               }
             // if more than 3 itmes should be displayed
             for (i = 2; i < CEIL(sd->display_item_num); i++)
               {
                  dit = eina_list_nth(sd->items, i);
                  item2 = eina_list_nth(sd->over_items, i - 2);
                  eina_stringshare_replace(&item2->label, dit->label);
                  edje_object_part_text_escaped_set
                    (VIEW(item2), "elm.text", item2->label);
               }

             dit = eina_list_nth(sd->items, eina_list_count(sd->items) - 1);
             if (dit)
               {
                  eina_stringshare_replace(&sd->last->label, dit->label);
                  edje_object_part_text_escaped_set
                    (sd->VIEW(last), "elm.text", sd->last->label);
               }

             dit = eina_list_nth(sd->items, eina_list_count(sd->items) - 2);
             if (dit)
               {
                  eina_stringshare_replace(&sd->s_last->label, dit->label);
                  edje_object_part_text_escaped_set
                    (sd->VIEW(s_last), "elm.text", sd->s_last->label);
               }
             // if more than 3 itmes should be displayed
             for (i = 3; i <= CEIL(sd->display_item_num); i++)
               {
                  dit = eina_list_nth(sd->items, sd->item_count - i);
                  item2 = eina_list_nth(sd->under_items, i - 3);
                  eina_stringshare_replace(&item2->label, dit->label);
                  edje_object_part_text_escaped_set
                    (VIEW(item2), "elm.text", item2->label);
               }
          }
     }

   sd->string_check_idle_enterer =
      ecore_idle_enterer_before_add(_string_check_idle_enterer_cb,
                                    obj);

   _sizing_eval(obj);

   return EINA_TRUE;
}

static void
_item_text_set_hook(Elm_Object_Item *item,
                    const char *part,
                    const char *label)
{
   Elm_Diskselector_Item *it;
   Evas_Coord mw, mh;

   if (part && strcmp(part, "default")) return;

   it = (Elm_Diskselector_Item *)item;
   eina_stringshare_replace(&it->label, label);
   edje_object_part_text_escaped_set(VIEW(it), "elm.text", it->label);
   _item_signal_emit(it);
   edje_object_size_min_calc(VIEW(it), &mw, &mh);
   evas_object_size_hint_min_set(VIEW(it), 0, mh);
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   if (part && strcmp(part, "default")) return NULL;

   return ((Elm_Diskselector_Item *)it)->label;
}

static void
_item_icon_set(Elm_Diskselector_Item *it,
               Evas_Object *icon)
{
   if (it->icon == icon) return;

   evas_object_del(it->icon);
   it->icon = icon;

   if (VIEW(it))
     {
        evas_object_size_hint_min_set(it->icon, 24, 24);
        evas_object_size_hint_max_set(it->icon, 40, 40);
        edje_object_part_swallow(VIEW(it), "elm.swallow.icon", it->icon);
        evas_object_show(it->icon);
        elm_widget_sub_object_add(WIDGET(it), it->icon);
     }

   _item_signal_emit(it);
}

static void
_identical_item_check(Elm_Diskselector_Item *it,
                      Evas_Object *icon)
{
   Elm_Diskselector_Item *dit;
   Eina_List *l;
   int idx = 0;
   int ic = 0;
   int ac = 0;

   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   if (sd->round)
     {
        // Get index from indentical item from round items
        EINA_LIST_FOREACH(sd->r_items, l, dit)
          {
             if (it == dit) break;
             idx++;
          }

        // No item to match
        ic = eina_list_count(sd->r_items);

        if (idx >= ic) return;
        dit = NULL;

        // Number of added items: CEIL(sd->display_item_num)
        ac = CEIL(sd->display_item_num);

        if (((idx >= 0) && (idx < ac)) ||
            ((idx >= ac) && (idx < (2 * ac))))
          {
             // Selected item: under, low region
             dit = eina_list_nth(sd->r_items,
                                 idx + ic - (2 * ac));
          }
        else if (((idx >= (ic - ac)) && (idx < ic)) ||
                 ((idx >= (ic - (2 * ac))) && (idx < ic - ac)))
          {
             // Selected item: over, high region
             dit = eina_list_nth(sd->r_items,
                                 idx - ic + (2 * ac));
          }

        if (dit) _item_icon_set(dit, icon);
        _sizing_eval(WIDGET(it));
     }
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   if (part && strcmp(part, "icon")) return;

   _item_icon_set((Elm_Diskselector_Item *)it, content);
   _identical_item_check((Elm_Diskselector_Item *)it, content);
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   if (part && strcmp(part, "icon")) return NULL;

   return ((Elm_Diskselector_Item *)it)->icon;
}

static void
_item_click_cb(void *data,
               Evas_Object *obj EINA_UNUSED,
               const char *emission EINA_UNUSED,
               const char *source EINA_UNUSED)
{
   Elm_Diskselector_Item *it = data;

   if (!it) return;
   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   if (sd->selected_item != it)
     {
        sd->selected_item = it;
        _selected_item_indicate(sd->selected_item);
     }

   if (it->func) it->func((void *)it->base.data, WIDGET(it), it);
   evas_object_smart_callback_call(WIDGET(it), SIG_CLICKED, it);
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_Diskselector_Item *it = (Elm_Diskselector_Item *)data;
   const char *txt = NULL;
   if (!it) return NULL;

   if (!txt) txt = it->label;
   if (txt) return strdup(txt);

   return NULL;
}

static void
_access_on_highlight_cb(void *data)
{
   Evas_Coord bx, by, bw, bh;
   Evas_Coord x, y, w, h;

   Elm_Diskselector_Item *it = (Elm_Diskselector_Item *)data;

   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it);
   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   evas_smart_objects_calculate(evas_object_evas_get(sd->main_box));
   evas_object_geometry_get(sd->main_box, &bx, &by, &bw, &bh);
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);

   x -= bx;
   y -= by;

   eo_do(WIDGET(it), elm_interface_scrollable_region_bring_in(x, y, w, h));
}

static Elm_Diskselector_Item *
_item_new(Evas_Object *obj,
          Evas_Object *icon,
          const char *label,
          Evas_Smart_Cb func,
          const void *data)
{
   Elm_Diskselector_Item *it;

   it = elm_widget_item_new(obj, Elm_Diskselector_Item);
   if (!it) return NULL;

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);

   it->label = eina_stringshare_add(label);
   it->func = func;
   it->base.data = data;
   VIEW(it) = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set(obj, VIEW(it), "diskselector", "item",
                               elm_widget_style_get(obj));

   evas_object_size_hint_weight_set
     (VIEW(it), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(VIEW(it), EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(VIEW(it));

   if (icon)
     _item_content_set_hook((Elm_Object_Item *)it, "icon", icon);

   if (it->label)
     {
        _item_text_set_hook((Elm_Object_Item *)it, "default", it->label);
        edje_object_signal_callback_add
          (VIEW(it), "elm,action,click", "*", _item_click_cb, it);
     }

   //XXX: ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        _elm_access_widget_item_register((Elm_Widget_Item *)it);

        _elm_access_text_set(_elm_access_info_get(it->base.access_obj),
                             ELM_ACCESS_TYPE, E_("diskselector item"));
        _elm_access_callback_set(_elm_access_info_get(it->base.access_obj),
                                 ELM_ACCESS_INFO,
                                 _access_info_cb, it);
        _elm_access_on_highlight_hook_set(
           _elm_access_info_get(it->base.access_obj), _access_on_highlight_cb,
           it);
     }

   return it;
}

static Evas_Object *
_blank_add(Evas_Object *obj)
{
   Evas_Object *blank;
   blank = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set(obj, blank, "diskselector", "item", "default");
   evas_object_size_hint_weight_set(blank, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(blank, EVAS_HINT_FILL, EVAS_HINT_FILL);

   return blank;
}

static void
_blank_item_check(Evas_Object *obj, int item_num)
{
   Evas_Object *blank;
   Eina_List *last;
   int diff, i;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   diff = (item_num / 2) - eina_list_count(sd->left_blanks);
   if (!diff) return;

   /* left blank */
   for (i = 0; i < ((diff > 0) ? (diff) : (-diff)); i++)
     {
        if (diff > 0)
          {
             blank = _blank_add(obj);
             elm_box_pack_start(sd->main_box, blank);
             evas_object_show(blank);
             sd->left_blanks = eina_list_append(sd->left_blanks, blank);
          }
        else
          {
             last = eina_list_last(sd->left_blanks);
             blank = eina_list_data_get(last);
             elm_box_unpack(sd->main_box, blank);
             evas_object_del(blank);
             sd->left_blanks = eina_list_remove_list(sd->left_blanks, last);
          }
     }

   /* right blank */
   for (i = 0; i < ((diff > 0) ? (diff) : (-diff)); i++)
     {
        if (diff > 0)
          {
             blank = _blank_add(obj);
             elm_box_pack_end(sd->main_box, blank);
             evas_object_show(blank);
             sd->right_blanks = eina_list_append(sd->right_blanks, blank);
          }
        else
          {
             last = eina_list_last(sd->right_blanks);
             blank = eina_list_data_get(last);
             elm_box_unpack(sd->main_box, blank);
             evas_object_del(blank);
             sd->right_blanks = eina_list_remove_list(sd->right_blanks, last);
          }
     }
}

static void
_theme_data_get(Evas_Object *obj)
{
   const char *str;
   Evas_Object *blank;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   blank = eina_list_data_get(sd->right_blanks);
   if (blank) return;

   str = edje_object_data_get(blank, "len_threshold");
   if (str) sd->len_threshold = MAX(0, atoi(str));
   else sd->len_threshold = 0;

   if (!sd->display_item_num_by_api)
     {
        str = edje_object_data_get(blank, "display_item_num");
        if (str)
          {
             sd->display_item_num = MAX(DISPLAY_ITEM_NUM_MIN, atoi(str));
             if (!sd->round) _blank_item_check(obj, sd->display_item_num);
          }

        else sd->display_item_num = DISPLAY_ITEM_NUM_MIN;
     }

   str = edje_object_data_get(blank, "min_width");
   if (str) sd->minw = MAX(-1, atoi(str));
   else sd->minw = -1;

   str = edje_object_data_get(blank, "min_height");
   if (str) sd->minh = MAX(-1, atoi(str));
   else sd->minh = -1;
}

static void
_elm_diskselector_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_List *l;
   Elm_Diskselector_Item *it;
   Evas_Object *blank;
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   Evas *evas;
   const char *style = elm_widget_style_get(obj);

   Elm_Diskselector_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   evas = evas_object_evas_get(obj);
   evas_event_freeze(evas);

   if (sd->round)
     {
        EINA_LIST_FOREACH(sd->r_items, l, it)
          {
             elm_widget_theme_object_set(obj, VIEW(it),
                                         "diskselector", "item", style);
             edje_object_part_text_escaped_set
               (VIEW(it), "elm.text", it->label);
          }
     }
   else
     {
        EINA_LIST_FOREACH(sd->items, l, it)
          {
             elm_widget_theme_object_set(obj, VIEW(it),
                                         "diskselector","item", style);
             edje_object_part_text_escaped_set
               (VIEW(it), "elm.text", it->label);
          }

        /* left blank */
        EINA_LIST_FOREACH (sd->left_blanks, l, blank)
          {
             elm_widget_theme_object_set(obj, blank,
                                         "diskselector", "item", style);
          }

        /* right blank */
        EINA_LIST_FOREACH (sd->right_blanks, l, blank)
          {
             elm_widget_theme_object_set(obj, blank,
                                         "diskselector", "item", style);
          }
     }

   _theme_data_get(obj);
   _sizing_eval(obj);

   evas_event_thaw(evas);
   evas_event_thaw_eval(evas);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_diskselector_smart_sub_object_del(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Diskselector_Item *it;
   const Eina_List *l;

   Elm_Diskselector_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

   EINA_LIST_FOREACH(sd->items, l, it)
     {
        if (sobj == it->icon)
          {
             it->icon = NULL;

             _sizing_eval(obj);
             break;
          }
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_diskselector_smart_on_focus(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_on_focus(&int_ret));
   if (!int_ret) return;

   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit
          (wd->resize_obj, "elm,action,focus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit
          (wd->resize_obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }

   if (ret) *ret = EINA_TRUE;
}

static Eina_Bool _elm_diskselector_smart_focus_next_enable = EINA_FALSE;

static void
_elm_diskselector_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = _elm_diskselector_smart_focus_next_enable;
}

static void
_elm_diskselector_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   Eina_List *items = NULL;
   Eina_List *elist = NULL;
   Elm_Diskselector_Item *it;

   ELM_DISKSELECTOR_CHECK(obj);
   Elm_Diskselector_Smart_Data *sd = _pd;

   EINA_LIST_FOREACH(sd->items, elist, it)
     items = eina_list_append(items, it->base.access_obj);

   if (!sd->round)
     {
        int_ret = elm_widget_focus_list_next_get
              (obj, items, eina_list_data_get, dir, next);
        if (ret) *ret = int_ret;
        return;
     }

   if (!elm_widget_focus_list_next_get
          (obj, items, eina_list_data_get, dir, next))
     {
        elm_widget_focused_object_clear((Evas_Object *)obj);
        elm_widget_focus_list_next_get
          (obj, items, eina_list_data_get, dir, next);
     }
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_diskselector_smart_event(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Diskselector_Smart_Data *sd = _pd;
   Elm_Diskselector_Item *it = NULL;
   Eina_List *l;

   if (ret) *ret = EINA_FALSE;
   (void) src;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   if (!sd->selected_item)
     {
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
        if (ret) *ret = EINA_TRUE;
        return;
     }

   if ((!strcmp(ev->key, "Left")) ||
       ((!strcmp(ev->key, "KP_Left")) && (!ev->string)) ||
       (!strcmp(ev->key, "Up")) ||
       ((!strcmp(ev->key, "KP_Up")) && (!ev->string)))
     {
        l = sd->selected_item->node->prev;
        if ((!l) && (sd->round))
          l = eina_list_last(sd->items);
     }
   else if ((!strcmp(ev->key, "Right")) ||
            ((!strcmp(ev->key, "KP_Right")) && (!ev->string)) ||
            (!strcmp(ev->key, "Down")) ||
            ((!strcmp(ev->key, "KP_Down")) && (!ev->string)))
     {
        l = sd->selected_item->node->next;
        if ((!l) && (sd->round))
          l = sd->items;
     }
   else if ((!strcmp(ev->key, "Home")) ||
            ((!strcmp(ev->key, "KP_Home")) && (!ev->string)))
     l = sd->items;
   else if ((!strcmp(ev->key, "End")) ||
            ((!strcmp(ev->key, "KP_End")) && (!ev->string)))
     l = eina_list_last(sd->items);
   else return;

   if (l)
     it = eina_list_data_get(l);

   if (it)
     {
        sd->selected_item = it;
        if (!sd->scroller_move_idle_enterer)
          sd->scroller_move_idle_enterer = ecore_idle_enterer_before_add(_scroller_move, obj);
     }

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_scroll_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   Evas_Coord x, y, w, h, bw, x_boundary;
   unsigned int adjust_pixels;
   Eina_Bool h_bounce;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   _string_check(obj);
   eo_do(obj,
         elm_interface_scrollable_content_pos_get(&x, &y),
         elm_interface_scrollable_content_viewport_size_get(&w, &h));
   if (sd->round)
     {
        evas_object_geometry_get(sd->main_box, NULL, NULL, &bw, NULL);
        x_boundary = bw - w;

        if (x >= x_boundary)
          {
              if (sd->left_boundary_reached) return;

              sd->right_boundary_reached = EINA_TRUE;
              eo_do(obj, elm_interface_scrollable_bounce_allow_get(&h_bounce, NULL));
              /* If scroller's bounce effect is disabled, add 1 pixel
               *  to provide circular effect */
              adjust_pixels = (_elm_config->thumbscroll_bounce_enable
                               && h_bounce) ? 0 : 1;
              eo_do(obj, elm_interface_scrollable_content_region_show
                 (x - x_boundary + adjust_pixels, y, w, h));
              sd->left_boundary_reached = EINA_FALSE;
          }
        else if (x <= 0)
          {
              if (sd->right_boundary_reached) return;

              sd->left_boundary_reached = EINA_TRUE;
              eo_do(obj, elm_interface_scrollable_bounce_allow_get(&h_bounce, NULL));
              /* If scroller's bounce effect is disabled, subtract 1 pixel
               *  to provide circular effect */
              adjust_pixels = (_elm_config->thumbscroll_bounce_enable
                               && h_bounce) ? 0 : 1;
              eo_do(obj, elm_interface_scrollable_content_region_show
                 (x + x_boundary - adjust_pixels, y, w, h));
              sd->right_boundary_reached = EINA_FALSE;
          }
        else
          {
              sd->left_boundary_reached = EINA_FALSE;
              sd->right_boundary_reached = EINA_FALSE;
          }
     }
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data EINA_UNUSED)
{
   Elm_Diskselector_Item *it;
   Evas_Coord x, w, ox, ow;
   Eina_List *l, *list;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (sd->scroller_move_idle_enterer) return;

   if (!sd->round)
     list = sd->items;
   else
     list = sd->r_items;

   evas_object_geometry_get(obj, &ox, NULL, &ow, NULL);
   EINA_LIST_FOREACH(list, l, it)
     {
        evas_object_geometry_get(VIEW(it), &x, NULL, &w, NULL);
        if (abs((int)(ox + (ow / 2)) - (int)(x + (w / 2))) < 10) break;
     }

   if (!it) return;
   _item_select(it);
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_STOP, it);
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data EINA_UNUSED)
{
   evas_object_smart_callback_call
     (obj, SIG_SCROLL_ANIM_START, elm_diskselector_selected_item_get(obj));
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data EINA_UNUSED)
{
   evas_object_smart_callback_call
     (obj, SIG_SCROLL_DRAG_START, elm_diskselector_selected_item_get(obj));
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data EINA_UNUSED)
{
   evas_object_smart_callback_call
     (obj, SIG_SCROLL_DRAG_STOP, elm_diskselector_selected_item_get(obj));
}

static void
_round_item_del(Elm_Diskselector_Smart_Data *sd,
                Elm_Diskselector_Item *it)
{
   if (!it) return;

   elm_box_unpack(sd->main_box, VIEW(it));
   sd->r_items = eina_list_remove(sd->r_items, it);
   eina_stringshare_del(it->label);
   elm_widget_item_free(it);
}

static void
_round_items_del(Elm_Diskselector_Smart_Data *sd)
{
   Eina_List *l;
   Elm_Diskselector_Item *it;

   _round_item_del(sd, sd->last);
   sd->last = NULL;
   _round_item_del(sd, sd->s_last);
   sd->s_last = NULL;
   _round_item_del(sd, sd->second);
   sd->second = NULL;
   _round_item_del(sd, sd->first);
   sd->first = NULL;

   EINA_LIST_FOREACH(sd->under_items, l, it)
     {
        _round_item_del(sd, it);
     }
   sd->under_items = eina_list_free(sd->under_items);

   EINA_LIST_FOREACH(sd->over_items, l, it)
     {
        _round_item_del(sd, it);
     }
   sd->over_items = eina_list_free(sd->over_items);
}

static Evas_Object *
_icon_duplicate(Evas_Object *icon)
{
   Evas_Object *ic;
   const char *file;
   const char *group;

   if (!icon) return NULL;

   elm_image_file_get(icon, &file, &group);
   ic = elm_icon_add(icon);
   elm_image_file_set(ic, file, group);
   elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);

   return ic;
}

static void
_round_items_add(Elm_Diskselector_Smart_Data *sd)
{
   Elm_Diskselector_Item *temp_it;
   Elm_Diskselector_Item *dit;
   Elm_Diskselector_Item *it;
   int i = 0;

   dit = it = eina_list_nth(sd->items, 0);
   if (!dit) return;

   if (!sd->first)
     {
        sd->first = _item_new
            (WIDGET(it), _icon_duplicate(it->icon), it->label, it->func,
            it->base.data);
        sd->first->node = it->node;
        sd->r_items = eina_list_append(sd->r_items, sd->first);
     }

   it = eina_list_nth(sd->items, 1);
   if (!it)
     it = dit;

   if (!sd->second)
     {
        sd->second = _item_new
            (WIDGET(it), _icon_duplicate(it->icon), it->label, it->func,
            it->base.data);
        sd->second->node = it->node;
        sd->r_items = eina_list_append(sd->r_items, sd->second);
     }

   // if more than 3 itmes should be displayed
   for (i = 2; i < CEIL(sd->display_item_num); i++)
     {
        it = eina_list_nth(sd->items, i);

        if (!it) it = dit;
        temp_it = _item_new
            (WIDGET(it), _icon_duplicate
              (it->icon), it->label, it->func, it->base.data);
        sd->over_items = eina_list_append(sd->over_items, temp_it);
        sd->r_items = eina_list_append(sd->r_items, temp_it);
     }

   it = eina_list_nth(sd->items, sd->item_count - 1);
   if (!it)
     it = dit;
   if (!sd->last)
     {
        sd->last = _item_new
            (WIDGET(it), _icon_duplicate(it->icon), it->label, it->func,
            it->base.data);
        sd->last->node = it->node;
        sd->r_items = eina_list_prepend(sd->r_items, sd->last);
     }

   it = eina_list_nth(sd->items, sd->item_count - 2);
   if (!it)
     it = dit;
   if (!sd->s_last)
     {
        sd->s_last = _item_new
            (WIDGET(it), _icon_duplicate(it->icon), it->label, it->func,
            it->base.data);
        sd->s_last->node = it->node;
        sd->r_items = eina_list_prepend(sd->r_items, sd->s_last);
     }

   // if more than 3 itmes should be displayed
   for (i = 3; i <= CEIL(sd->display_item_num); i++)
     {
        it = eina_list_nth(sd->items, sd->item_count - i);
        if (!it) it = dit;
        temp_it = _item_new
            (WIDGET(it), _icon_duplicate
              (it->icon), it->label, it->func, it->base.data);
        sd->under_items = eina_list_append(sd->under_items, temp_it);
        sd->r_items = eina_list_prepend(sd->r_items, temp_it);
     }
}

static void
_elm_diskselector_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Diskselector_Smart_Data *priv = _pd;
   Evas *evas;
   Evas_Object *blank, *edje;

   elm_widget_sub_object_parent_add(obj);

   evas = evas_object_evas_get(obj);
   evas_event_freeze(evas);

   edje = edje_object_add(evas);
   elm_widget_resize_object_set(obj, edje, EINA_TRUE);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_theme_object_set
     (obj, edje, "diskselector", "base", elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas);
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->len_side = 3;

   eo_do(obj,
         elm_interface_scrollable_objects_set(edje, priv->hit_rect),
         elm_obj_widget_theme_apply(NULL),
         elm_interface_scrollable_policy_set(ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF),
         elm_interface_scrollable_bounce_allow_set(EINA_TRUE, EINA_FALSE),
         elm_interface_scrollable_animate_start_cb_set(_scroll_animate_start_cb),
         elm_interface_scrollable_animate_stop_cb_set(_scroll_animate_stop_cb),
         elm_interface_scrollable_drag_start_cb_set(_scroll_drag_start_cb),
         elm_interface_scrollable_drag_stop_cb_set(_scroll_drag_stop_cb),
         elm_interface_scrollable_scroll_cb_set(_scroll_cb));

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize_cb, NULL);

   priv->main_box = elm_box_add(obj);
   elm_box_horizontal_set(priv->main_box, EINA_TRUE);
   elm_box_homogeneous_set(priv->main_box, EINA_TRUE);
   evas_object_size_hint_weight_set
     (priv->main_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (priv->main_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_widget_theme_set(priv->main_box, elm_widget_theme_get(obj));

   elm_widget_sub_object_add(obj, priv->main_box);

   eo_do(obj, elm_interface_scrollable_content_set(priv->main_box));

   /* left blank */
   blank = _blank_add(obj);
   elm_box_pack_start(priv->main_box, blank);
   evas_object_show(blank);
   priv->left_blanks = eina_list_append(priv->left_blanks, blank);

   /* right blank */
   blank = _blank_add(obj);
   elm_box_pack_end(priv->main_box, blank);
   evas_object_show(blank);
   priv->right_blanks = eina_list_append(priv->right_blanks, blank);

   _theme_data_get(obj);
   _sizing_eval(obj);

   evas_event_thaw(evas);
   evas_event_thaw_eval(evas);
}

static void
_elm_diskselector_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Diskselector_Item *it;
   Eina_List *l;
   Evas_Object *blank;

   Elm_Diskselector_Smart_Data *sd = _pd;

   /* left blank */
   EINA_LIST_FOREACH (sd->left_blanks, l, blank)
     evas_object_del(blank);

   /* right blank */
   EINA_LIST_FOREACH (sd->right_blanks, l, blank)
     evas_object_del(blank);

   if (sd->last)
     {
        eina_stringshare_del(sd->last->label);
        evas_object_del(sd->VIEW(last));
        free(sd->last);
     }

   if (sd->s_last)
     {
        eina_stringshare_del(sd->s_last->label);
        evas_object_del(sd->VIEW(s_last));
        free(sd->s_last);
     }

   if (sd->second)
     {
        eina_stringshare_del(sd->second->label);
        evas_object_del(sd->VIEW(second));
        free(sd->second);
     }

   if (sd->first)
     {
        eina_stringshare_del(sd->first->label);
        evas_object_del(sd->VIEW(first));
        free(sd->first);
     }

   EINA_LIST_FOREACH(sd->under_items, l, it)
     {
        if (it)
          {
             eina_stringshare_del(it->label);
             evas_object_del(VIEW(it));
             free(it);
          }
     }

   EINA_LIST_FOREACH(sd->over_items, l, it)
     {
        if (it)
          {
             eina_stringshare_del(it->label);
             evas_object_del(VIEW(it));
             free(it);
          }
     }

   EINA_LIST_FREE(sd->items, it)
     {
        _item_del(it);
        elm_widget_item_free(it);
     }
   sd->r_items = eina_list_free(sd->r_items);

   ELM_SAFE_FREE(sd->scroller_move_idle_enterer, ecore_idle_enterer_del);
   ELM_SAFE_FREE(sd->string_check_idle_enterer, ecore_idle_enterer_del);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static void
_elm_diskselector_smart_move(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Elm_Diskselector_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_diskselector_smart_resize(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Elm_Diskselector_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_diskselector_smart_member_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *member = va_arg(*list, Evas_Object *);
   Elm_Diskselector_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_add(member));

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_elm_diskselector_smart_access(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   _elm_diskselector_smart_focus_next_enable = va_arg(*list, int);
}

EAPI Evas_Object *
elm_diskselector_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, MY_CLASS, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME_LEGACY),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));
}

EAPI Eina_Bool
elm_diskselector_round_enabled_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_diskselector_round_enabled_get(&ret));
   return ret;
}

static void
_round_enabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Diskselector_Smart_Data *sd = _pd;

   *ret = sd->round;
}

EAPI void
elm_diskselector_round_enabled_set(Evas_Object *obj,
                                   Eina_Bool enabled)
{
   ELM_DISKSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_diskselector_round_enabled_set(enabled));
}

static void
_round_enabled_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_List *elist;
   Elm_Diskselector_Item *it;
   Evas_Object *blank;

   Eina_Bool enabled = va_arg(*list, int);
   Elm_Diskselector_Smart_Data *sd = _pd;

   if (sd->round == enabled)
     return;

   sd->round = !!enabled;
   if (enabled)
     {
        sd->r_items = eina_list_clone(sd->items);
        _blank_item_check(obj, DISPLAY_ITEM_NUM_MIN);

        /* left blank */
        EINA_LIST_FOREACH (sd->left_blanks, elist, blank)
          {
             elm_box_unpack(sd->main_box, blank);
             evas_object_hide(blank);
          }

        /* right blank */
        EINA_LIST_FOREACH (sd->right_blanks, elist, blank)
          {
             elm_box_unpack(sd->main_box, blank);
             evas_object_hide(blank);
          }

        if (!sd->items)
          return;

        _round_items_add(sd);

        if (sd->last)
          elm_box_pack_start(sd->main_box, sd->VIEW(last));
        if (sd->s_last)
          elm_box_pack_start(sd->main_box, sd->VIEW(s_last));

        // if more than 3 items should be displayed
        EINA_LIST_FOREACH(sd->under_items, elist, it)
          elm_box_pack_start(sd->main_box, VIEW(it));

        if (sd->first)
          elm_box_pack_end(sd->main_box, sd->VIEW(first));
        if (sd->second)
          elm_box_pack_end(sd->main_box, sd->VIEW(second));

        // if more than 3 items should be displayed
        EINA_LIST_FOREACH(sd->over_items, elist, it)
          elm_box_pack_end(sd->main_box, VIEW(it));
     }
   else
     {
        _round_items_del(sd);

        /* left blank */
        EINA_LIST_FOREACH (sd->left_blanks, elist, blank)
          {
             elm_box_pack_start(sd->main_box, blank);
             evas_object_show(blank);
          }

        /* right blank */
        EINA_LIST_FOREACH (sd->right_blanks, elist, blank)
          {
             elm_box_pack_end(sd->main_box, blank);
             evas_object_show(blank);
          }

        _blank_item_check(obj, sd->display_item_num);

        sd->r_items = eina_list_free(sd->r_items);
     }

   if (sd->selected_item) _selected_item_indicate(sd->selected_item);
   _sizing_eval(obj);
}

EAPI int
elm_diskselector_side_text_max_length_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_obj_diskselector_side_text_max_length_get(&ret));
   return ret;
}

static void
_side_text_max_length_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Diskselector_Smart_Data *sd = _pd;

   *ret = sd->len_side;
}

EAPI void
elm_diskselector_side_text_max_length_set(Evas_Object *obj,
                                          int len)
{
   ELM_DISKSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_diskselector_side_text_max_length_set(len));
}

static void
_side_text_max_length_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int len = va_arg(*list, int);
   Elm_Diskselector_Smart_Data *sd = _pd;

   sd->len_side = len;
}

EAPI void
elm_diskselector_bounce_set(Evas_Object *obj,
                            Eina_Bool h_bounce,
                            Eina_Bool v_bounce)
{
   ELM_DISKSELECTOR_CHECK(obj);

   eo_do(obj, elm_interface_scrollable_bounce_allow_set(h_bounce, v_bounce));
}

EAPI void
elm_diskselector_bounce_get(const Evas_Object *obj,
                            Eina_Bool *h_bounce,
                            Eina_Bool *v_bounce)
{
   ELM_DISKSELECTOR_CHECK(obj);

   eo_do((Eo *) obj, elm_interface_scrollable_bounce_allow_get(h_bounce, v_bounce));
}

EAPI void
elm_diskselector_scroller_policy_get(const Evas_Object *obj,
                                     Elm_Scroller_Policy *policy_h,
                                     Elm_Scroller_Policy *policy_v)
{
   ELM_DISKSELECTOR_CHECK(obj);
   eo_do((Eo *) obj, elm_interface_scrollable_policy_get(policy_h, policy_v));
}

static void
_scroller_policy_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Scroller_Policy s_policy_h, s_policy_v;

   Elm_Scroller_Policy *policy_h = va_arg(*list, Elm_Scroller_Policy *);
   Elm_Scroller_Policy *policy_v = va_arg(*list, Elm_Scroller_Policy *);

   eo_do_super((Eo *)obj, MY_CLASS, elm_interface_scrollable_policy_get(&s_policy_h, &s_policy_v));
   if (policy_h) *policy_h = (Elm_Scroller_Policy)s_policy_h;
   if (policy_v) *policy_v = (Elm_Scroller_Policy)s_policy_v;
}

EAPI void
elm_diskselector_scroller_policy_set(Evas_Object *obj,
                                     Elm_Scroller_Policy policy_h,
                                     Elm_Scroller_Policy policy_v)
{
   ELM_DISKSELECTOR_CHECK(obj);
   eo_do(obj, elm_interface_scrollable_policy_set(policy_h, policy_v));
}

static void
_scroller_policy_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Scroller_Policy policy_h = va_arg(*list, Elm_Scroller_Policy);
   Elm_Scroller_Policy policy_v = va_arg(*list, Elm_Scroller_Policy);

   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   eo_do_super(obj, MY_CLASS, elm_interface_scrollable_policy_set(policy_h, policy_v));
}

EAPI void
elm_diskselector_clear(Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_diskselector_clear());
}

static void
_clear(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Diskselector_Item *it;

   Elm_Diskselector_Smart_Data *sd = _pd;

   if (!sd->items) return;

   sd->selected_item = NULL;
   EINA_LIST_FREE(sd->items, it)
     {
        _item_del(it);
        elm_widget_item_free(it);
     }

   _round_items_del(sd);
   sd->r_items = eina_list_free(sd->r_items);
   _sizing_eval(obj);
}

EAPI const Eina_List *
elm_diskselector_items_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) NULL;
   const Eina_List *ret = NULL;
   eo_do((Eo *) obj, elm_obj_diskselector_items_get(&ret));
   return ret;
}

static void
_items_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Eina_List **ret = va_arg(*list, const Eina_List **);
   Elm_Diskselector_Smart_Data *sd = _pd;

   *ret = sd->items;
}

EAPI Elm_Object_Item *
elm_diskselector_item_append(Evas_Object *obj,
                             const char *label,
                             Evas_Object *icon,
                             Evas_Smart_Cb func,
                             const void *data)
{
   ELM_DISKSELECTOR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_diskselector_item_append(label, icon, func, data, &ret));
   return ret;
}

static void
_item_append(Eo *obj, void *_pd, va_list *list)
{
   Elm_Diskselector_Item *it, *dit;
   Eina_List *elist;

   const char *label = va_arg(*list, const char *);
   Evas_Object *icon = va_arg(*list, Evas_Object *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Diskselector_Smart_Data *sd = _pd;
   Evas *evas;

   evas = evas_object_evas_get(obj);
   evas_event_freeze(evas);

   it = _item_new(obj, icon, label, func, data);
   sd->items = eina_list_append(sd->items, it);
   it->node = eina_list_last(sd->items);
   sd->item_count++;
   if (sd->round)
     {
        _round_items_del(sd);
        sd->r_items = eina_list_append(sd->r_items, it);
        _round_items_add(sd);

        if (sd->last)
          elm_box_pack_start(sd->main_box, sd->VIEW(last));
        if (sd->s_last)
          elm_box_pack_start(sd->main_box, sd->VIEW(s_last));

        // if more than 3 items should be displayed
         EINA_LIST_FOREACH (sd->under_items, elist, dit)
           elm_box_pack_start(sd->main_box, VIEW(dit));

        elm_box_pack_end(sd->main_box, VIEW(it));

        if (sd->first)
          elm_box_pack_end(sd->main_box, sd->VIEW(first));
        if (sd->second)
          elm_box_pack_end(sd->main_box, sd->VIEW(second));

        // if more than 3 items should be displayed
         EINA_LIST_FOREACH (sd->over_items, elist, dit)
           elm_box_pack_end(sd->main_box, VIEW(dit));
     }
   else
     {
        Evas_Object *blank;
        blank = eina_list_data_get(sd->right_blanks);
        elm_box_pack_before(sd->main_box, VIEW(it), blank);
     }

   if (!sd->selected_item)
     sd->selected_item = it;

   if (!sd->scroller_move_idle_enterer)
     sd->scroller_move_idle_enterer = ecore_idle_enterer_before_add(_scroller_move, obj);

   _sizing_eval(obj);

   evas_event_thaw(evas);
   evas_event_thaw_eval(evas);

   *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_diskselector_selected_item_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_diskselector_selected_item_get(&ret));
   return ret;
}

static void
_selected_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Diskselector_Smart_Data *sd = _pd;

   *ret = (Elm_Object_Item *)sd->selected_item;
}

EAPI void
elm_diskselector_item_selected_set(Elm_Object_Item *item,
                                   Eina_Bool selected)
{
   Elm_Diskselector_Item *it = (Elm_Diskselector_Item *)item;

   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it);
   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   if ((sd->selected_item == it) && (selected))
     return;

   if ((sd->selected_item == it) && (!selected))
     sd->selected_item = eina_list_data_get(sd->items);
   else
     {
        sd->selected_item = it;
        _selected_item_indicate(sd->selected_item);
     }

   if (!sd->scroller_move_idle_enterer)
     sd->scroller_move_idle_enterer = ecore_idle_enterer_before_add(_scroller_move, WIDGET(item));
}

EAPI Eina_Bool
elm_diskselector_item_selected_get(const Elm_Object_Item *it)
{
   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   return sd->selected_item == ((Elm_Diskselector_Item *)it);
}

EAPI Elm_Object_Item *
elm_diskselector_item_prev_get(const Elm_Object_Item *item)
{
   Elm_Diskselector_Item *it = (Elm_Diskselector_Item *)item;

   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);

   if (it->node->prev) return it->node->prev->data;
   else return NULL;
}

EAPI Elm_Object_Item *
elm_diskselector_item_next_get(const Elm_Object_Item *item)
{
   Elm_Diskselector_Item *it = (Elm_Diskselector_Item *)item;

   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);

   if (it->node->next) return it->node->next->data;
   else return NULL;
}

EAPI Elm_Object_Item *
elm_diskselector_first_item_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_diskselector_first_item_get(&ret));
   return ret;
}

static void
_first_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Diskselector_Smart_Data *sd = _pd;
   if (!sd || !sd->items) *ret = NULL;
   else *ret = eina_list_data_get(sd->items);
}

EAPI Elm_Object_Item *
elm_diskselector_last_item_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_diskselector_last_item_get(&ret));
   return ret;
}

static void
_last_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Diskselector_Smart_Data *sd = _pd;

   if (!sd->items) *ret = NULL;
   else *ret = eina_list_data_get(eina_list_last(sd->items));
}

EAPI void
elm_diskselector_display_item_num_set(Evas_Object *obj,
                                      int num)
{
   ELM_DISKSELECTOR_CHECK(obj);
   eo_do(obj, elm_obj_diskselector_display_item_num_set(num));
}

static void
_display_item_num_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int num = va_arg(*list, int);
   Elm_Diskselector_Smart_Data *sd = _pd;

   if (num < DISPLAY_ITEM_NUM_MIN) num = DISPLAY_ITEM_NUM_MIN;
   sd->display_item_num = num;
   sd->display_item_num_by_api = EINA_TRUE;

   if (!sd->round) _blank_item_check(obj, num);
}

EAPI int
elm_diskselector_display_item_num_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) (-1);
   int ret = -1;
   eo_do((Eo *) obj, elm_obj_diskselector_display_item_num_get(&ret));
   return ret;
}

static void
_display_item_num_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Diskselector_Smart_Data *sd = _pd;

   *ret = sd->display_item_num;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), _elm_diskselector_smart_member_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_diskselector_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_diskselector_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_diskselector_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_diskselector_smart_move),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_diskselector_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_TRANSLATE), _elm_diskselector_smart_translate),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_diskselector_smart_event),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ON_FOCUS), _elm_diskselector_smart_on_focus),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_diskselector_smart_sub_object_del),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_diskselector_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_diskselector_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACCESS), _elm_diskselector_smart_access),

        EO_OP_FUNC(ELM_INTERFACE_SCROLLABLE_ID(ELM_INTERFACE_SCROLLABLE_SUB_ID_POLICY_SET), _scroller_policy_set),
        EO_OP_FUNC(ELM_INTERFACE_SCROLLABLE_ID(ELM_INTERFACE_SCROLLABLE_SUB_ID_POLICY_GET), _scroller_policy_get),

        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_ROUND_ENABLED_GET), _round_enabled_get),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_ROUND_ENABLED_SET), _round_enabled_set),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_SIDE_TEXT_MAX_LENGTH_GET), _side_text_max_length_get),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_SIDE_TEXT_MAX_LENGTH_SET), _side_text_max_length_set),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_CLEAR), _clear),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_ITEMS_GET), _items_get),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_ITEM_APPEND), _item_append),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_SELECTED_ITEM_GET), _selected_item_get),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_FIRST_ITEM_GET), _first_item_get),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_LAST_ITEM_GET), _last_item_get),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_DISPLAY_ITEM_NUM_SET), _display_item_num_set),
        EO_OP_FUNC(ELM_OBJ_DISKSELECTOR_ID(ELM_OBJ_DISKSELECTOR_SUB_ID_DISPLAY_ITEM_NUM_GET), _display_item_num_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
      _elm_diskselector_smart_focus_next_enable = EINA_TRUE;
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_ROUND_ENABLED_GET, "Get a value whether round mode is enabled or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_ROUND_ENABLED_SET, "Enable or disable round mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_SIDE_TEXT_MAX_LENGTH_GET, "Get the side labels max length."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_SIDE_TEXT_MAX_LENGTH_SET, "Set the side labels max length."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_CLEAR, "Remove all diskselector's items."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_ITEMS_GET, "Get a list of all the diskselector items."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_ITEM_APPEND, "Appends a new item to the diskselector object."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_SELECTED_ITEM_GET, "Get the selected item."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_FIRST_ITEM_GET, "Get the first item of the diskselector."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_LAST_ITEM_GET, "Get the last item of the diskselector."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_DISPLAY_ITEM_NUM_SET, "Set the number of items to be displayed."),
     EO_OP_DESCRIPTION(ELM_OBJ_DISKSELECTOR_SUB_ID_DISPLAY_ITEM_NUM_GET, "Get the number of items in the diskselector object."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_DISKSELECTOR_BASE_ID, op_desc, ELM_OBJ_DISKSELECTOR_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Diskselector_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_diskselector_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, ELM_INTERFACE_SCROLLABLE_CLASS, NULL);
