#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_diskselector.h"

#ifndef MAX
# define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef CEIL
#define CEIL(a)    (((a) % 2 != 0) ? ((a) / 2 + 1) : ((a) / 2))
#endif

#define DISPLAY_ITEM_NUM_MIN 3

EAPI const char ELM_DISKSELECTOR_SMART_NAME[] = "elm_diskselector";

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
   {NULL, NULL}
};

static const Evas_Smart_Interface *_smart_interfaces[] =
{
   (Evas_Smart_Interface *)&ELM_SCROLLABLE_IFACE, NULL
};

EVAS_SMART_SUBCLASS_IFACE_NEW
  (ELM_DISKSELECTOR_SMART_NAME, _elm_diskselector,
  Elm_Diskselector_Smart_Class, Elm_Widget_Smart_Class,
  elm_widget_smart_class_get, _smart_callbacks, _smart_interfaces);

static void
_selected_item_indicate(Elm_Diskselector_Item *it)
{
   Eina_List *l;
   Elm_Diskselector_Item *item;

   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   if (!it->label) return;

   EINA_LIST_FOREACH (sd->r_items, l, item)
     {
        if (item->label && !strcmp(item->label, it->label))
          edje_object_signal_emit(VIEW(item), "elm,state,selected", "elm");
        else
          edje_object_signal_emit(VIEW(item), "elm,state,default", "elm");
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
_item_signal_emit(Elm_Diskselector_Item *item)
{
   if ((item->icon) && (!item->label))
     edje_object_signal_emit(VIEW(item), "elm,state,icon,only", "elm");
   else if ((!item->icon) && (item->label))
     edje_object_signal_emit(VIEW(item), "elm,state,text,only", "elm");
   else
     edje_object_signal_emit(VIEW(item), "elm,state,text,icon", "elm");
}

static Eina_Bool
_string_check(void *data)
{
   Elm_Diskselector_Smart_Data *sd = data;
   int mid, steps, length, diff;
   Elm_Diskselector_Item *it;
   Eina_List *list, *l;
   Evas_Coord ox, ow;
   char buf[1024];

   evas_object_geometry_get(ELM_WIDGET_DATA(sd)->obj, &ox, NULL, &ow, NULL);

   if (ow <= 0)
     return EINA_FALSE;

   if (!sd->init)
     return EINA_FALSE;

   if (!sd->round)
     list = sd->items;
   else
     list = sd->r_items;

   EINA_LIST_FOREACH (list, l, it)
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

   if (sd->check_idler)
     ecore_idle_enterer_del(sd->check_idler);

   sd->check_idler = NULL;

   return EINA_FALSE;
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

   EINA_LIST_FOREACH (list, l, dit)
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
   sd->s_iface->content_region_show(obj, w / sd->display_item_num * i, y, w, h);
   _item_select(dit);

   sd->init = EINA_TRUE;
   _string_check(sd);

end:
   sd->idler = NULL;
   return ECORE_CALLBACK_CANCEL;
}

static void
_resize_cb(void *data __UNUSED__,
           Evas *e __UNUSED__,
           Evas_Object *obj,
           void *event_info __UNUSED__)
{
   Evas_Coord w, h;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if ((sd->minw == -1) && (sd->minh == -1))
     elm_coords_finger_size_adjust(6, &sd->minw, 1, &sd->minh);

   edje_object_size_min_restricted_calc
     (ELM_WIDGET_DATA(sd)->resize_obj, &sd->minw, &sd->minh, sd->minw,
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
                        * (sd->item_count + CEIL(sd->display_item_num)), h);

   sd->s_iface->paging_set(obj, 0, 0, (int)(w / sd->display_item_num), 0);

   if (!sd->idler)
     sd->idler = ecore_idle_enterer_before_add(_scroller_move, obj);
}

static void
_item_del(Elm_Diskselector_Item *item)
{
   if (!item) return;

   eina_stringshare_del(item->label);

   if (item->icon)
     evas_object_del(item->icon);
}

static void
_sizing_eval(Evas_Object *obj)
{
   _resize_cb(NULL, NULL, obj, NULL);
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Diskselector_Item *item, *item2, *dit;
   Eina_List *l;
   int i = 0;

   item = (Elm_Diskselector_Item *)it;

   ELM_DISKSELECTOR_DATA_GET(WIDGET(item), sd);

   elm_box_unpack(sd->main_box, VIEW(item));

   if (sd->round)
     sd->r_items = eina_list_remove(sd->r_items, item);

   sd->items = eina_list_remove(sd->items, item);

   if (sd->selected_item == item)
     {
        dit = (Elm_Diskselector_Item *)eina_list_nth(sd->items, 0);

        if (dit != item)
          sd->selected_item = dit;
        else
          sd->selected_item = eina_list_nth(sd->items, 1);

        _selected_item_indicate(sd->selected_item);
     }

   _item_del(item);
   sd->item_count -= 1;

   if (sd->round)
     {
        if (!sd->item_count)
          {
             evas_object_hide(sd->VIEW(first));
             evas_object_hide(sd->VIEW(second));
             evas_object_hide(sd->VIEW(last));
             evas_object_hide(sd->VIEW(s_last));

             EINA_LIST_FOREACH (sd->under_items, l, item2)
               evas_object_hide(VIEW(item2));

             EINA_LIST_FOREACH (sd->over_items, l, item2)
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

   sd->check_idler = ecore_idle_enterer_before_add(_string_check, sd);

   _sizing_eval(ELM_WIDGET_DATA(sd)->obj);

   return EINA_TRUE;
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Diskselector_Item *item;

   if (part && strcmp(part, "default")) return;

   item = (Elm_Diskselector_Item *)it;
   eina_stringshare_replace(&item->label, label);
   edje_object_part_text_escaped_set(VIEW(item), "elm.text", item->label);

   _item_signal_emit(item);
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

   if (it->icon) evas_object_del(it->icon);
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
        EINA_LIST_FOREACH (sd->r_items, l, dit)
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
        _sizing_eval(ELM_WIDGET_DATA(sd)->obj);
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
               Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__,
               const char *source __UNUSED__)
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
_access_info_cb(void *data,
                Evas_Object *obj __UNUSED__,
                Elm_Widget_Item *item __UNUSED__)
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

   sd->s_iface->region_bring_in(WIDGET(it), x, y, w, h);
}

static Elm_Diskselector_Item *
_item_new(Evas_Object *obj,
          Evas_Object *icon,
          const char *label,
          Evas_Smart_Cb func,
          const void *data)
{
   Elm_Diskselector_Item *it;
   const char *style = elm_widget_style_get(obj);

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
   elm_widget_theme_object_set(obj, VIEW(it), "diskselector", "item", style);

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
          (VIEW(it), "elm,action,click", "", _item_click_cb, it);
     }

   //XXX: ACCESS
   _elm_access_widget_item_register((Elm_Widget_Item *)it);

   _elm_access_text_set(_elm_access_object_get(it->base.access_obj),
                        ELM_ACCESS_TYPE, E_("diskselector item"));
   _elm_access_callback_set(_elm_access_object_get(it->base.access_obj),
                            ELM_ACCESS_INFO,
                            _access_info_cb, it);
   _elm_access_on_highlight_hook_set(
     _elm_access_object_get(it->base.access_obj), _access_on_highlight_cb,
     it);

   return it;
}

static void
_theme_data_get(Elm_Diskselector_Smart_Data *sd)
{
   const char *str;

   str = edje_object_data_get(sd->right_blank, "len_threshold");
   if (str) sd->len_threshold = MAX(0, atoi(str));
   else sd->len_threshold = 0;

   if (!sd->display_item_num_by_api)
     {
        str = edje_object_data_get(sd->right_blank, "display_item_num");
        if (str) sd->display_item_num = MAX(DISPLAY_ITEM_NUM_MIN, atoi(str));
        else sd->display_item_num = DISPLAY_ITEM_NUM_MIN;
     }

   str = edje_object_data_get(sd->right_blank, "min_width");
   if (str) sd->minw = MAX(-1, atoi(str));
   else sd->minw = -1;

   str = edje_object_data_get(sd->right_blank, "min_height");
   if (str) sd->minh = MAX(-1, atoi(str));
   else sd->minh = -1;
}

static Eina_Bool
_elm_diskselector_smart_theme(Evas_Object *obj)
{
   Eina_List *l;
   Elm_Diskselector_Item *it;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_diskselector_parent_sc)->theme(obj))
     return EINA_FALSE;

   if (sd->round)
     {
        EINA_LIST_FOREACH (sd->r_items, l, it)
          {
             elm_widget_theme_object_set(obj, VIEW(it), "diskselector", "item",
                                         elm_widget_style_get(obj));
             edje_object_part_text_escaped_set
               (VIEW(it), "elm.text", it->label);
          }
     }
   else
     {
        EINA_LIST_FOREACH (sd->items, l, it)
          {
             elm_widget_theme_object_set(obj, VIEW(it), "diskselector", "item",
                                         elm_widget_style_get(obj));
             edje_object_part_text_escaped_set
               (VIEW(it), "elm.text", it->label);
          }
     }
   elm_widget_theme_object_set(obj, sd->right_blank, "diskselector", "item",
                               elm_widget_style_get(obj));

   _theme_data_get(sd);
   _sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_diskselector_smart_sub_object_del(Evas_Object *obj,
                                       Evas_Object *sobj)
{
   Elm_Diskselector_Item *it;
   const Eina_List *l;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_diskselector_parent_sc)->sub_object_del
         (obj, sobj))
     return EINA_FALSE;

   EINA_LIST_FOREACH (sd->items, l, it)
     {
        if (sobj == it->icon)
          {
             it->icon = NULL;

             _sizing_eval(obj);
             break;
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_elm_diskselector_smart_on_focus(Evas_Object *obj)
{
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (elm_widget_focus_get(obj))
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,action,focus", "elm");
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_TRUE);
     }
   else
     {
        edje_object_signal_emit
          (ELM_WIDGET_DATA(sd)->resize_obj, "elm,action,unfocus", "elm");
        evas_object_focus_set(ELM_WIDGET_DATA(sd)->resize_obj, EINA_FALSE);
     }

   return EINA_TRUE;
}

static Eina_Bool
_elm_diskselector_smart_focus_next(const Evas_Object *obj,
                           Elm_Focus_Direction dir,
                           Evas_Object **next)
{
   Eina_List *items = NULL;
   Eina_List *elist = NULL;
   Elm_Diskselector_Item *it;

   ELM_DISKSELECTOR_CHECK(obj) EINA_FALSE;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   EINA_LIST_FOREACH (sd->items, elist, it)
     items = eina_list_append(items, it->base.access_obj);

   if (!sd->round)
     return elm_widget_focus_list_next_get
              (obj, items, eina_list_data_get, dir, next);

   if (!elm_widget_focus_list_next_get
          (obj, items, eina_list_data_get, dir, next))
     {
        elm_widget_focused_object_clear((Evas_Object *)obj);
        elm_widget_focus_list_next_get
          (obj, items, eina_list_data_get, dir, next);
     }
   return EINA_TRUE;
}

static Eina_Bool
_elm_diskselector_smart_event(Evas_Object *obj,
                              Evas_Object *src __UNUSED__,
                              Evas_Callback_Type type,
                              void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;
   Elm_Diskselector_Item *it = NULL;
   Eina_List *l;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;
   if (elm_widget_disabled_get(obj)) return EINA_FALSE;

   if (!sd->selected_item)
     {
        ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

        return EINA_TRUE;
     }

   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)) ||
       (!strcmp(ev->keyname, "Up")) ||
       ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        l = sd->selected_item->node->prev;
        if ((!l) && (sd->round))
          l = eina_list_last(sd->items);
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)) ||
            (!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        l = sd->selected_item->node->next;
        if ((!l) && (sd->round))
          l = sd->items;
     }
   else if ((!strcmp(ev->keyname, "Home")) ||
            ((!strcmp(ev->keyname, "KP_Home")) && (!ev->string)))
     l = sd->items;
   else if ((!strcmp(ev->keyname, "End")) ||
            ((!strcmp(ev->keyname, "KP_End")) && (!ev->string)))
     l = eina_list_last(sd->items);
   else return EINA_FALSE;

   if (l)
     it = eina_list_data_get(l);

   if (it)
     {
        sd->selected_item = it;
        if (!sd->idler)
          sd->idler = ecore_idle_enterer_before_add(_scroller_move, obj);
     }

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;

   return EINA_TRUE;
}

static void
_scroll_cb(Evas_Object *obj,
           void *data __UNUSED__)
{
   Evas_Coord x, y, w, h, bw;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   _string_check(sd);
   sd->s_iface->content_pos_get(obj, &x, &y);
   sd->s_iface->content_viewport_size_get(obj, &w, &h);
   if (sd->round)
     {
        evas_object_geometry_get(sd->main_box, NULL, NULL, &bw, NULL);
        if (x > ((w / sd->display_item_num) * (sd->item_count
                                               + (sd->display_item_num % 2))))
          sd->s_iface->content_region_show
            (obj, x - ((w / sd->display_item_num) * sd->item_count),
            y, w, h);
        else if (x < 0)
          sd->s_iface->content_region_show
            (obj, x + ((w / sd->display_item_num) * sd->item_count),
            y, w, h);
     }
}

static void
_scroll_animate_stop_cb(Evas_Object *obj,
                        void *data __UNUSED__)
{
   Elm_Diskselector_Item *it;
   Evas_Coord x, w, ow;
   Eina_List *l, *list;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (sd->idler) return;

   if (!sd->round)
     list = sd->items;
   else
     list = sd->r_items;

   evas_object_geometry_get(obj, NULL, NULL, &ow, NULL);
   EINA_LIST_FOREACH (list, l, it)
     {
        evas_object_geometry_get(VIEW(it), &x, NULL, &w, NULL);
        if (abs((int)(ow / 2 - (int)(x + w / 2))) < 10) break;
     }

   if (!it) return;
   _item_select(it);
   evas_object_smart_callback_call(data, SIG_SCROLL_ANIM_STOP, it);
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data __UNUSED__)
{
   evas_object_smart_callback_call
     (obj, SIG_SCROLL_ANIM_START, elm_diskselector_selected_item_get(obj));
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data __UNUSED__)
{
   evas_object_smart_callback_call
     (obj, SIG_SCROLL_DRAG_START, elm_diskselector_selected_item_get(obj));
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data __UNUSED__)
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

   EINA_LIST_FOREACH (sd->under_items, l, it)
     {
        _round_item_del(sd, it);
     }
   sd->under_items = eina_list_free(sd->under_items);

   EINA_LIST_FOREACH (sd->over_items, l, it)
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
   elm_image_resizable_set(ic, 1, 1);

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
_elm_diskselector_smart_add(Evas_Object *obj)
{
   EVAS_SMART_DATA_ALLOC(obj, Elm_Diskselector_Smart_Data);

   ELM_WIDGET_DATA(priv)->resize_obj =
     edje_object_add(evas_object_evas_get(obj));

   ELM_WIDGET_CLASS(_elm_diskselector_parent_sc)->base.add(obj);

   elm_widget_theme_object_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, "diskselector", "base",
     elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   priv->s_iface = evas_object_smart_interface_get
       (obj, ELM_SCROLLABLE_IFACE_NAME);

   priv->s_iface->objects_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, priv->hit_rect);

   priv->item_count = 0;
   priv->round = EINA_FALSE;
   priv->init = EINA_FALSE;
   priv->len_side = 3;
   priv->display_item_num_by_api = EINA_FALSE;

   _elm_diskselector_smart_theme(obj);
   priv->s_iface->policy_set
     (obj, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   priv->s_iface->bounce_allow_set(obj, EINA_TRUE, EINA_FALSE);

   priv->s_iface->animate_start_cb_set(obj, _scroll_animate_start_cb);
   priv->s_iface->animate_stop_cb_set(obj, _scroll_animate_stop_cb);
   priv->s_iface->drag_start_cb_set(obj, _scroll_drag_start_cb);
   priv->s_iface->drag_stop_cb_set(obj, _scroll_drag_stop_cb);
   priv->s_iface->scroll_cb_set(obj, _scroll_cb);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize_cb, NULL);

   priv->main_box = elm_box_add(obj);
   elm_box_horizontal_set(priv->main_box, EINA_TRUE);
   elm_box_homogeneous_set(priv->main_box, EINA_TRUE);
   evas_object_size_hint_weight_set
     (priv->main_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (priv->main_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_widget_theme_object_set
     (obj, priv->main_box, "diskselector", "base", "default");

   elm_widget_sub_object_add(obj, priv->main_box);

   priv->s_iface->content_set(obj, priv->main_box);

   priv->left_blank = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set
     (obj, priv->left_blank, "diskselector", "item", "default");
   evas_object_size_hint_weight_set
     (priv->left_blank, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (priv->left_blank, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(priv->main_box, priv->left_blank);
   evas_object_show(priv->left_blank);

   priv->right_blank = edje_object_add(evas_object_evas_get(obj));
   elm_widget_theme_object_set
     (obj, priv->right_blank, "diskselector", "item", "default");
   evas_object_size_hint_weight_set
     (priv->right_blank, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set
     (priv->right_blank, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(priv->main_box, priv->right_blank);
   evas_object_show(priv->right_blank);

   _theme_data_get(priv);

   _sizing_eval(obj);
}

static void
_elm_diskselector_smart_del(Evas_Object *obj)
{
   Elm_Diskselector_Item *it;
   Eina_List *l;

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (sd->left_blank)
     evas_object_del(sd->left_blank);
   if (sd->right_blank)
     evas_object_del(sd->right_blank);

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

   EINA_LIST_FOREACH (sd->under_items, l, it)
     {
        if (it)
          {
             eina_stringshare_del(it->label);
             evas_object_del(VIEW(it));
             free(it);
          }
     }

   EINA_LIST_FOREACH (sd->over_items, l, it)
     {
        if (it)
          {
             eina_stringshare_del(it->label);
             evas_object_del(VIEW(it));
             free(it);
          }
     }

   EINA_LIST_FREE (sd->items, it)
     {
        _item_del(it);
        elm_widget_item_free(it);
     }
   eina_list_free(sd->r_items);

   if (sd->idler)
     {
        ecore_idle_enterer_del(sd->idler);
        sd->idler = NULL;
     }

   if (sd->check_idler)
     {
        ecore_idle_enterer_del(sd->check_idler);
        sd->check_idler = NULL;
     }

   ELM_WIDGET_CLASS(_elm_diskselector_parent_sc)->base.del(obj);
}

static void
_elm_diskselector_smart_move(Evas_Object *obj,
                             Evas_Coord x,
                             Evas_Coord y)
{
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_diskselector_parent_sc)->base.move(obj, x, y);

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_diskselector_smart_resize(Evas_Object *obj,
                               Evas_Coord w,
                               Evas_Coord h)
{
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_diskselector_parent_sc)->base.resize(obj, w, h);

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_diskselector_smart_member_add(Evas_Object *obj,
                                   Evas_Object *member)
{
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_diskselector_parent_sc)->base.member_add(obj, member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_access_hook(Evas_Object *obj, Eina_Bool is_access)
{
   ELM_DISKSELECTOR_DATA_GET(obj, sd);
   
   if (is_access)
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next =  _elm_diskselector_smart_focus_next;
   else
     ELM_WIDGET_CLASS(ELM_WIDGET_DATA(sd)->api)->focus_next = NULL;
}


static void
_elm_diskselector_smart_set_user(Elm_Diskselector_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_diskselector_smart_add;
   ELM_WIDGET_CLASS(sc)->base.del = _elm_diskselector_smart_del;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_diskselector_smart_move;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_diskselector_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.member_add = _elm_diskselector_smart_member_add;

   ELM_WIDGET_CLASS(sc)->sub_object_del =
     _elm_diskselector_smart_sub_object_del;
   ELM_WIDGET_CLASS(sc)->on_focus = _elm_diskselector_smart_on_focus;
   ELM_WIDGET_CLASS(sc)->theme = _elm_diskselector_smart_theme;
   ELM_WIDGET_CLASS(sc)->event = _elm_diskselector_smart_event;

   //XXX: ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     ELM_WIDGET_CLASS(sc)->focus_next = _elm_diskselector_smart_focus_next;

   ELM_WIDGET_CLASS(sc)->access = _access_hook;
}

EAPI const Elm_Diskselector_Smart_Class *
elm_diskselector_smart_class_get(void)
{
   static Elm_Diskselector_Smart_Class _sc =
     ELM_DISKSELECTOR_SMART_CLASS_INIT_NAME_VERSION
       (ELM_DISKSELECTOR_SMART_NAME);
   static const Elm_Diskselector_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class) return class;

   _elm_diskselector_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_diskselector_add(Evas_Object *parent)
{
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   obj = elm_widget_add(_elm_diskselector_smart_class_new(), parent);
   if (!obj) return NULL;

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

EAPI Eina_Bool
elm_diskselector_round_enabled_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) EINA_FALSE;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   return sd->round;
}

EAPI void
elm_diskselector_round_enabled_set(Evas_Object *obj,
                                   Eina_Bool enabled)
{
   Eina_List *elist;
   Elm_Diskselector_Item *it;

   ELM_DISKSELECTOR_CHECK(obj);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (sd->round == enabled)
     return;

   sd->round = !!enabled;
   if (enabled)
     {
        sd->r_items = eina_list_clone(sd->items);
        elm_box_unpack(sd->main_box, sd->left_blank);
        evas_object_hide(sd->left_blank);
        elm_box_unpack(sd->main_box, sd->right_blank);
        evas_object_hide(sd->right_blank);
        if (!sd->items)
          return;

        _round_items_add(sd);

        if (sd->last)
          elm_box_pack_start(sd->main_box, sd->VIEW(last));
        if (sd->s_last)
          elm_box_pack_start(sd->main_box, sd->VIEW(s_last));

        // if more than 3 items should be displayed
        EINA_LIST_FOREACH (sd->under_items, elist, it)
          elm_box_pack_start(sd->main_box, VIEW(it));

        if (sd->first)
          elm_box_pack_end(sd->main_box, sd->VIEW(first));
        if (sd->second)
          elm_box_pack_end(sd->main_box, sd->VIEW(second));

        // if more than 3 items should be displayed
        EINA_LIST_FOREACH (sd->over_items, elist, it)
          elm_box_pack_end(sd->main_box, VIEW(it));
     }
   else
     {
        _round_items_del(sd);
        elm_box_pack_start(sd->main_box, sd->left_blank);
        elm_box_pack_end(sd->main_box, sd->right_blank);
        eina_list_free(sd->r_items);
        sd->r_items = NULL;
     }

   _selected_item_indicate(sd->selected_item);
   _sizing_eval(obj);
}

EAPI int
elm_diskselector_side_text_max_length_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) 0;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   return sd->len_side;
}

EAPI void
elm_diskselector_side_text_max_length_set(Evas_Object *obj,
                                          int len)
{
   ELM_DISKSELECTOR_CHECK(obj);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   sd->len_side = len;
}

EAPI void
elm_diskselector_bounce_set(Evas_Object *obj,
                            Eina_Bool h_bounce,
                            Eina_Bool v_bounce)
{
   ELM_DISKSELECTOR_CHECK(obj);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   sd->s_iface->bounce_allow_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_diskselector_bounce_get(const Evas_Object *obj,
                            Eina_Bool *h_bounce,
                            Eina_Bool *v_bounce)
{
   ELM_DISKSELECTOR_CHECK(obj);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   sd->s_iface->bounce_allow_get(obj, h_bounce, v_bounce);
}

EAPI void
elm_diskselector_scroller_policy_get(const Evas_Object *obj,
                                     Elm_Scroller_Policy *policy_h,
                                     Elm_Scroller_Policy *policy_v)
{
   Elm_Scroller_Policy s_policy_h, s_policy_v;

   ELM_DISKSELECTOR_CHECK(obj);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   sd->s_iface->policy_get(obj, &s_policy_h, &s_policy_v);
   if (policy_h) *policy_h = (Elm_Scroller_Policy)s_policy_h;
   if (policy_v) *policy_v = (Elm_Scroller_Policy)s_policy_v;
}

EAPI void
elm_diskselector_scroller_policy_set(Evas_Object *obj,
                                     Elm_Scroller_Policy policy_h,
                                     Elm_Scroller_Policy policy_v)
{
   ELM_DISKSELECTOR_CHECK(obj);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   sd->s_iface->policy_set(obj, policy_h, policy_v);
}

EAPI void
elm_diskselector_clear(Evas_Object *obj)
{
   Elm_Diskselector_Item *it;

   ELM_DISKSELECTOR_CHECK(obj);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (!sd->items) return;

   sd->selected_item = NULL;
   EINA_LIST_FREE (sd->items, it)
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
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   return sd->items;
}

EAPI Elm_Object_Item *
elm_diskselector_item_append(Evas_Object *obj,
                             const char *label,
                             Evas_Object *icon,
                             Evas_Smart_Cb func,
                             const void *data)
{
   Elm_Diskselector_Item *it;

   ELM_DISKSELECTOR_CHECK(obj) NULL;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

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
        elm_box_pack_end(sd->main_box, VIEW(it));
        if (sd->first)
          elm_box_pack_end(sd->main_box, sd->VIEW(first));
        if (sd->second)
          elm_box_pack_end(sd->main_box, sd->VIEW(second));
     }
   else
     {
        elm_box_unpack(sd->main_box, sd->right_blank);
        elm_box_pack_end(sd->main_box, VIEW(it));
        elm_box_pack_end(sd->main_box, sd->right_blank);
     }

   if (!sd->selected_item)
     sd->selected_item = it;

   if (!sd->idler)
     sd->idler = ecore_idle_enterer_before_add(_scroller_move, obj);

   _sizing_eval(obj);
   return (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_diskselector_selected_item_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) NULL;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   return (Elm_Object_Item *)sd->selected_item;
}

EAPI void
elm_diskselector_item_selected_set(Elm_Object_Item *it,
                                   Eina_Bool selected)
{
   Elm_Diskselector_Item *item = (Elm_Diskselector_Item *)it;

   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it);
   ELM_DISKSELECTOR_DATA_GET(WIDGET(item), sd);

   if ((sd->selected_item == item) && (selected))
     return;

   if ((sd->selected_item == item) && (!selected))
     sd->selected_item = eina_list_data_get(sd->items);
   else
     {
        sd->selected_item = item;
        _selected_item_indicate(sd->selected_item);
     }

   if (!sd->idler)
     sd->idler = ecore_idle_enterer_before_add(_scroller_move, WIDGET(item));
}

EAPI Eina_Bool
elm_diskselector_item_selected_get(const Elm_Object_Item *it)
{
   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   return sd->selected_item == ((Elm_Diskselector_Item *)it);
}

EAPI Elm_Object_Item *
elm_diskselector_item_prev_get(const Elm_Object_Item *it)
{
   Elm_Diskselector_Item *item = (Elm_Diskselector_Item *)it;

   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);

   if (item->node->prev) return item->node->prev->data;
   else return NULL;
}

EAPI Elm_Object_Item *
elm_diskselector_item_next_get(const Elm_Object_Item *it)
{
   Elm_Diskselector_Item *item = (Elm_Diskselector_Item *)it;

   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);

   if (item->node->next) return item->node->next->data;
   else return NULL;
}

EAPI Elm_Object_Item *
elm_diskselector_first_item_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) NULL;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);
   if (!sd || !sd->items) return NULL;
   return eina_list_data_get(sd->items);
}

EAPI Elm_Object_Item *
elm_diskselector_last_item_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) NULL;
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (!sd->items) return NULL;
   return eina_list_data_get(eina_list_last(sd->items));
}

EAPI void
elm_diskselector_display_item_num_set(Evas_Object *obj,
                                      int num)
{
   ELM_DISKSELECTOR_CHECK(obj);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   if (num < DISPLAY_ITEM_NUM_MIN) num = DISPLAY_ITEM_NUM_MIN;
   sd->display_item_num = num;
   sd->display_item_num_by_api = EINA_TRUE;
}

EAPI int
elm_diskselector_display_item_num_get(const Evas_Object *obj)
{
   ELM_DISKSELECTOR_CHECK(obj) (-1);
   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   return sd->display_item_num;
}
