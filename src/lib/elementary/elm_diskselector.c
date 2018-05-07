#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define ELM_WIDGET_ITEM_PROTECTED
#define EFL_UI_TRANSLATABLE_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"
#include "elm_diskselector.eo.h"
#include "elm_diskselector_item.eo.h"
#include "elm_widget_diskselector.h"
#include "elm_interface_scrollable.h"

#define DISPLAY_ITEM_NUM_MIN 3

#define MY_CLASS ELM_DISKSELECTOR_CLASS

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

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {NULL, NULL}
};

EOLIAN static void
_elm_diskselector_efl_ui_translatable_translation_update(Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd)
{
   Elm_Diskselector_Item_Data *it;
   Eina_List *l;

   EINA_LIST_FOREACH(sd->items, l, it)
     elm_wdg_item_translate(EO_OBJ(it));
}

static void
_selected_item_indicate(Elm_Diskselector_Item_Data *item)
{
   Eina_List *l;
   Elm_Diskselector_Item_Data *it;

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
_item_select(Elm_Diskselector_Item_Data *it)
{
   if (!it) return;

   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);
   Elm_Object_Item *eo_it = EO_OBJ(it);

   sd->selected_item = it;
   _selected_item_indicate(sd->selected_item);

   if (it->func) it->func((void *)WIDGET_ITEM_DATA_GET(eo_it), WIDGET(it), eo_it);
   efl_event_callback_legacy_call
     (WIDGET(it), EFL_UI_EVENT_SELECTED, eo_it);
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
_item_signal_emit(Elm_Diskselector_Item_Data *it)
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
   Elm_Diskselector_Item_Data *it;
   Eina_List *list, *l;
   Evas_Coord ox, ow;
   char buf[1024];

   if (!sd)
     return ECORE_CALLBACK_CANCEL;

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
   Elm_Diskselector_Item_Data *dit;
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
        sd->selected_item = eina_list_nth(sd->items, 0);
        goto end;
     }

   evas_object_geometry_get(obj, NULL, &y, &w, &h);
   elm_interface_scrollable_content_region_show(obj, w / sd->display_item_num * i, y, w, h);
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

   elm_interface_scrollable_content_viewport_geometry_get
         (obj, NULL, NULL, &vw, &vh);
   if (sd->items)
     {
        Elm_Diskselector_Item_Data *it = sd->items->data;
        evas_object_size_hint_combined_min_get(VIEW(it), &mw, &mh);
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

   elm_interface_scrollable_paging_set(obj, 0, 0, (int)(w / sd->display_item_num), 0);

   if (!sd->scroller_move_idle_enterer)
     sd->scroller_move_idle_enterer = ecore_idle_enterer_before_add(_scroller_move, obj);
}

static void
_item_del(Elm_Diskselector_Item_Data *item)
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

EOLIAN static void
_elm_diskselector_item_efl_object_destructor(Eo *eo_it, Elm_Diskselector_Item_Data *it)
{
   Elm_Diskselector_Item_Data *item2, *dit;
   Eina_List *l;
   int i = 0;

   Evas_Object *obj = WIDGET(it);

   ELM_DISKSELECTOR_DATA_GET(obj, sd);

   elm_box_unpack(sd->main_box, VIEW(it));

   if (sd->round)
     sd->r_items = eina_list_remove(sd->r_items, it);

   sd->items = eina_list_remove(sd->items, it);
   sd->under_items = eina_list_remove(sd->under_items, it);
   sd->over_items = eina_list_remove(sd->over_items, it);

   if (sd->selected_item == it)
     {
        if (sd->items)
          {
             sd->selected_item = eina_list_nth(sd->items, 0);
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
             if (sd->first) evas_object_hide(sd->VIEW(first));
             if (sd->second) evas_object_hide(sd->VIEW(second));
             if (sd->last) evas_object_hide(sd->VIEW(last));
             if (sd->s_last) evas_object_hide(sd->VIEW(s_last));

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
                  if (sd->first)
                    {
                       eina_stringshare_replace(&sd->first->label, dit->label);
                       edje_object_part_text_escaped_set
                          (sd->VIEW(first), "elm.text", sd->first->label);
                    }
               }

             dit = eina_list_nth(sd->items, 1);
             if (dit)
               {
                  if (sd->second)
                    {
                       eina_stringshare_replace(&sd->second->label, dit->label);
                       edje_object_part_text_escaped_set
                          (sd->VIEW(second), "elm.text", sd->second->label);
                    }
               }
             // if more than 3 itmes should be displayed
             for (i = 2; i < CEIL(sd->display_item_num); i++)
               {
                  dit = eina_list_nth(sd->items, i);
                  item2 = eina_list_nth(sd->over_items, i - 2);
                  if (!dit || !item2) continue;
                  eina_stringshare_replace(&item2->label, dit->label);
                  edje_object_part_text_escaped_set
                    (VIEW(item2), "elm.text", item2->label);
               }

             dit = eina_list_nth(sd->items, eina_list_count(sd->items) - 1);
             if (dit)
               {
                  if (sd->last)
                    {
                       eina_stringshare_replace(&sd->last->label, dit->label);
                       edje_object_part_text_escaped_set
                          (sd->VIEW(last), "elm.text", sd->last->label);
                    }
               }

             dit = eina_list_nth(sd->items, eina_list_count(sd->items) - 2);
             if (dit)
               {
                  if (sd->s_last)
                    {
                       eina_stringshare_replace(&sd->s_last->label, dit->label);
                       edje_object_part_text_escaped_set
                          (sd->VIEW(s_last), "elm.text", sd->s_last->label);
                    }
               }
             // if more than 3 itmes should be displayed
             for (i = 3; i <= CEIL(sd->display_item_num); i++)
               {
                  dit = eina_list_nth(sd->items, sd->item_count - i);
                  item2 = eina_list_nth(sd->under_items, i - 3);
                  if (!dit || !item2) continue;
                  eina_stringshare_replace(&item2->label, dit->label);
                  edje_object_part_text_escaped_set
                    (VIEW(item2), "elm.text", item2->label);
               }
          }
     }

   if (!sd->string_check_idle_enterer)
     {
        sd->string_check_idle_enterer =
           ecore_idle_enterer_before_add(_string_check_idle_enterer_cb,
                                         obj);
     }

   _sizing_eval(obj);

   efl_destructor(efl_super(eo_it, ELM_DISKSELECTOR_ITEM_CLASS));
}

EOLIAN static void
_elm_diskselector_item_elm_widget_item_part_text_set(Eo *eo_it EINA_UNUSED, Elm_Diskselector_Item_Data *it,
                    const char *part,
                    const char *label)
{
   Evas_Coord mw, mh;

   if (part && strcmp(part, "default")) return;

   eina_stringshare_replace(&it->label, label);
   edje_object_part_text_escaped_set(VIEW(it), "elm.text", it->label);
   _item_signal_emit(it);
   edje_object_size_min_calc(VIEW(it), &mw, &mh);
   evas_object_size_hint_min_set(VIEW(it), 0, mh);
}

EOLIAN static const char *
_elm_diskselector_item_elm_widget_item_part_text_get(const Eo *eo_it EINA_UNUSED, Elm_Diskselector_Item_Data *it,
                    const char *part)
{
   if (part && strcmp(part, "default")) return NULL;

   return it->label;
}

static void
_item_icon_set(Elm_Diskselector_Item_Data *it,
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
_identical_item_check(Elm_Diskselector_Item_Data *it,
                      Evas_Object *icon)
{
   Elm_Diskselector_Item_Data *dit;
   Eina_List *l;
   int idx = 0;
   int ic = 0;
   int ac = 0;

   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   if (sd->round)
     {
        // Get index from identical item from round items
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

EOLIAN static void
_elm_diskselector_item_elm_widget_item_part_content_set(Eo *eo_it EINA_UNUSED, Elm_Diskselector_Item_Data *it,
                       const char *part,
                       Evas_Object *content)
{
   if (part && strcmp(part, "icon")) return;

   _item_icon_set(it, content);
   _identical_item_check(it, content);
}

EOLIAN static Evas_Object *
_elm_diskselector_item_elm_widget_item_part_content_get(const Eo *eo_it EINA_UNUSED, Elm_Diskselector_Item_Data *it,
                       const char *part)
{
   if (part && strcmp(part, "icon")) return NULL;

   return it->icon;
}

static void
_item_click_cb(void *data,
               Evas_Object *obj EINA_UNUSED,
               const char *emission EINA_UNUSED,
               const char *source EINA_UNUSED)
{
   Elm_Diskselector_Item_Data *it = data;

   if (!it) return;
   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);
   Elm_Object_Item *eo_it = EO_OBJ(it);

   if (sd->selected_item != it)
     {
        sd->selected_item = it;
        _selected_item_indicate(sd->selected_item);
     }

   if (it->func) it->func((void *)WIDGET_ITEM_DATA_GET(eo_it), WIDGET(it), eo_it);
   efl_event_callback_legacy_call
     (WIDGET(it), EFL_UI_EVENT_CLICKED, eo_it);
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_Diskselector_Item_Data *it = (Elm_Diskselector_Item_Data *)data;
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

   Elm_Diskselector_Item_Data *it = (Elm_Diskselector_Item_Data *)data;

   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it);
   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   evas_smart_objects_calculate(evas_object_evas_get(sd->main_box));
   evas_object_geometry_get(sd->main_box, &bx, &by, &bw, &bh);
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);

   x -= bx;
   y -= by;

   elm_interface_scrollable_region_bring_in(WIDGET(it), x, y, w, h);
}

EOLIAN static Eo *
_elm_diskselector_item_efl_object_constructor(Eo *eo_it, Elm_Diskselector_Item_Data *it)
{
   eo_it = efl_constructor(efl_super(eo_it, ELM_DISKSELECTOR_ITEM_CLASS));
   it->base = efl_data_scope_get(eo_it, ELM_WIDGET_ITEM_CLASS);

   return eo_it;
}

static Elm_Diskselector_Item_Data *
_item_new(Evas_Object *obj,
          Evas_Object *icon,
          const char *label,
          Evas_Smart_Cb func,
          const void *data)
{
   Eo *eo_it = efl_add(ELM_DISKSELECTOR_ITEM_CLASS, obj);
   if (!eo_it) return NULL;
   ELM_DISKSELECTOR_ITEM_DATA_GET(eo_it, it);

   it->label = eina_stringshare_add(label);
   it->func = func;
   WIDGET_ITEM_DATA_SET(eo_it, data);
   VIEW_SET(it, edje_object_add(evas_object_evas_get(obj)));
   elm_widget_theme_object_set(obj, VIEW(it), "diskselector", "item",
                               elm_widget_style_get(obj));

   evas_object_size_hint_weight_set
     (VIEW(it), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(VIEW(it), EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(VIEW(it));

   if (icon)
     elm_wdg_item_part_content_set(eo_it, "icon", icon);

   if (it->label)
     {
        elm_wdg_item_part_text_set(eo_it, "default", it->label);
        edje_object_signal_callback_add
          (VIEW(it), "elm,action,click", "*", _item_click_cb, it);
     }

   //XXX: ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        _elm_access_widget_item_register(it->base);

        _elm_access_text_set(_elm_access_info_get(it->base->access_obj),
                             ELM_ACCESS_TYPE, E_("diskselector item"));
        _elm_access_callback_set(_elm_access_info_get(it->base->access_obj),
                                 ELM_ACCESS_INFO,
                                 _access_info_cb, it);
        _elm_access_on_highlight_hook_set(
           _elm_access_info_get(it->base->access_obj), _access_on_highlight_cb,
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

EOLIAN static Efl_Ui_Theme_Apply
_elm_diskselector_efl_ui_widget_theme_apply(Eo *obj, Elm_Diskselector_Data *sd)
{
   Eina_List *l;
   Elm_Diskselector_Item_Data *it;
   Evas_Object *blank;
   Efl_Ui_Theme_Apply int_ret = EFL_UI_THEME_APPLY_FAILED;
   Evas *evas;
   const char *style = elm_widget_style_get(obj);

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return EFL_UI_THEME_APPLY_FAILED;

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

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_diskselector_efl_ui_widget_widget_sub_object_del(Eo *obj, Elm_Diskselector_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;

   Elm_Diskselector_Item_Data *it;
   const Eina_List *l;

   int_ret = elm_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   EINA_LIST_FOREACH(sd->items, l, it)
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

EOLIAN static Eina_Bool
_elm_diskselector_efl_ui_focus_object_on_focus_update(Eo *obj, Elm_Diskselector_Data *_pd EINA_UNUSED)
{
   Eina_Bool int_ret = EINA_FALSE;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   int_ret = efl_ui_focus_object_on_focus_update(efl_super(obj, MY_CLASS));
   if (!int_ret) return EINA_FALSE;

   if (efl_ui_focus_object_focus_get(obj))
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

   return EINA_TRUE;
}

static Eina_Bool _elm_diskselector_smart_focus_next_enable = EINA_FALSE;

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   ELM_DISKSELECTOR_DATA_GET(obj, sd);
   Elm_Diskselector_Item_Data *it = NULL;
   Eina_List *l = NULL;
   const char *dir = params;

   if (!sd->selected_item) return EINA_TRUE;
   _elm_widget_focus_auto_show(obj);
   if (!strcmp(dir, "prev"))
     {
        l = sd->selected_item->node->prev;
        if ((!l) && (sd->round))
          l = eina_list_last(sd->items);
     }
   else if (!strcmp(dir, "next"))
     {
        l = sd->selected_item->node->next;
        if ((!l) && (sd->round))
          l = sd->items;
     }
   else if (!strcmp(dir, "first"))
     l = sd->items;
   else if (!strcmp(dir, "last"))
     l = eina_list_last(sd->items);
   else return EINA_FALSE;

   if (l)
     it = eina_list_data_get(l);

   if (it)
     {
        sd->selected_item = it;
        if (!sd->scroller_move_idle_enterer)
          sd->scroller_move_idle_enterer = ecore_idle_enterer_before_add(_scroller_move, obj);
     }

   return EINA_TRUE;
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
   elm_interface_scrollable_content_pos_get(obj, &x, &y);
   elm_interface_scrollable_content_viewport_geometry_get
         (obj, NULL, NULL, &w, &h);
   if (sd->round)
     {
        evas_object_geometry_get(sd->main_box, NULL, NULL, &bw, NULL);
        x_boundary = bw - w;

        if (x >= x_boundary)
          {
              if (sd->left_boundary_reached) return;

              sd->right_boundary_reached = EINA_TRUE;
              elm_interface_scrollable_bounce_allow_get(obj, &h_bounce, NULL);
              /* If scroller's bounce effect is disabled, add 1 pixel
               *  to provide circular effect */
              adjust_pixels = (_elm_config->thumbscroll_bounce_enable
                               && h_bounce) ? 0 : 1;
              elm_interface_scrollable_content_region_show
                 (obj, x - x_boundary + adjust_pixels, y, w, h);
              sd->left_boundary_reached = EINA_FALSE;
          }
        else if (x <= 0)
          {
              if (sd->right_boundary_reached) return;

              sd->left_boundary_reached = EINA_TRUE;
              elm_interface_scrollable_bounce_allow_get(obj, &h_bounce, NULL);
              /* If scroller's bounce effect is disabled, subtract 1 pixel
               *  to provide circular effect */
              adjust_pixels = (_elm_config->thumbscroll_bounce_enable
                               && h_bounce) ? 0 : 1;
              elm_interface_scrollable_content_region_show
                 (obj, x + x_boundary - adjust_pixels, y, w, h);
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
   Elm_Diskselector_Item_Data *it;
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
   efl_event_callback_legacy_call
     (data, EFL_UI_EVENT_SCROLL_ANIM_STOP, EO_OBJ(it));
}

static void
_scroll_animate_start_cb(Evas_Object *obj,
                         void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL_ANIM_START, elm_diskselector_selected_item_get(obj));
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL_DRAG_START, elm_diskselector_selected_item_get(obj));
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL_DRAG_STOP, elm_diskselector_selected_item_get(obj));
}

static void
_round_item_del(Elm_Diskselector_Data *sd EINA_UNUSED,
                Elm_Diskselector_Item_Data *it)
{
   if (!it) return;
   efl_del(EO_OBJ(it));
}

static void
_round_items_del(Elm_Diskselector_Data *sd)
{
   Eina_List *l;
   Elm_Diskselector_Item_Data *it;

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
_round_items_add(Elm_Diskselector_Data *sd)
{
   Elm_Diskselector_Item_Data *temp_it;
   Elm_Diskselector_Item_Data *dit;
   Elm_Diskselector_Item_Data *it;
   int i = 0;

   dit = it = eina_list_nth(sd->items, 0);
   if (!dit) return;

   if (!sd->first)
     {
        sd->first = _item_new
            (WIDGET(it), _icon_duplicate(it->icon), it->label, it->func,
            WIDGET_ITEM_DATA_GET(EO_OBJ(it)));
        if (!sd->first) return;
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
            WIDGET_ITEM_DATA_GET(EO_OBJ(it)));
        if (!sd->second) return;
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
              (it->icon), it->label, it->func, WIDGET_ITEM_DATA_GET(EO_OBJ(it)));
        if (!temp_it) return;
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
            WIDGET_ITEM_DATA_GET(EO_OBJ(it)));
        if (!sd->last) return;
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
            WIDGET_ITEM_DATA_GET(EO_OBJ(it)));
        if (!sd->s_last) return;
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
              (it->icon), it->label, it->func, WIDGET_ITEM_DATA_GET(EO_OBJ(it)));
        if (!temp_it) return;
        sd->under_items = eina_list_append(sd->under_items, temp_it);
        sd->r_items = eina_list_prepend(sd->r_items, temp_it);
     }
}

EOLIAN static void
_elm_diskselector_efl_canvas_group_group_add(Eo *obj, Elm_Diskselector_Data *priv)
{
   Evas *evas;
   Evas_Object *blank, *edje;

   elm_widget_sub_object_parent_add(obj);

   evas = evas_object_evas_get(obj);
   evas_event_freeze(evas);

   edje = edje_object_add(evas);
   elm_widget_resize_object_set(obj, edje);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

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

   elm_interface_scrollable_objects_set(obj, edje, priv->hit_rect);
   efl_ui_widget_theme_apply(obj);
   elm_interface_scrollable_policy_set(obj, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
   elm_interface_scrollable_bounce_allow_set(obj, EINA_TRUE, EINA_FALSE);
   elm_interface_scrollable_animate_start_cb_set(obj, _scroll_animate_start_cb);
   elm_interface_scrollable_animate_stop_cb_set(obj, _scroll_animate_stop_cb);
   elm_interface_scrollable_drag_start_cb_set(obj, _scroll_drag_start_cb);
   elm_interface_scrollable_drag_stop_cb_set(obj, _scroll_drag_stop_cb);
   elm_interface_scrollable_scroll_cb_set(obj, _scroll_cb);

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

   elm_interface_scrollable_content_set(obj, priv->main_box);

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

EOLIAN static void
_elm_diskselector_efl_canvas_group_group_del(Eo *obj, Elm_Diskselector_Data *sd)
{
   Elm_Diskselector_Item_Data *it;
   Eina_List *l, *l2;
   Evas_Object *blank;

   /* left blank */
   EINA_LIST_FREE (sd->left_blanks, blank)
     evas_object_del(blank);

   /* right blank */
   EINA_LIST_FREE (sd->right_blanks, blank)
     evas_object_del(blank);

   if (sd->last)
     {
        evas_object_del(sd->VIEW(last));
        efl_del(EO_OBJ(sd->last));
        sd->last = NULL;
     }

   if (sd->s_last)
     {
        evas_object_del(sd->VIEW(s_last));
        efl_del(EO_OBJ(sd->s_last));
        sd->s_last = NULL;
     }

   if (sd->second)
     {
        evas_object_del(sd->VIEW(second));
        efl_del(EO_OBJ(sd->second));
        sd->second = NULL;
     }

   if (sd->first)
     {
        evas_object_del(sd->VIEW(first));
        efl_del(EO_OBJ(sd->first));
        sd->first = NULL;
     }

   EINA_LIST_FOREACH_SAFE(sd->under_items, l, l2, it)
     {
        if (it)
          {
             evas_object_del(VIEW(it));
             efl_del(EO_OBJ(it));
          }
     }
   sd->under_items = eina_list_free(sd->under_items);

   EINA_LIST_FOREACH_SAFE(sd->over_items, l, l2, it)
     {
        if (it)
          {
             evas_object_del(VIEW(it));
             efl_del(EO_OBJ(it));
          }
     }
   sd->over_items = eina_list_free(sd->over_items);

   EINA_LIST_FOREACH_SAFE(sd->items, l, l2, it)
     {
        efl_del(EO_OBJ(it));
     }
   sd->items = eina_list_free(sd->items);
   sd->r_items = eina_list_free(sd->r_items);

   ELM_SAFE_FREE(sd->scroller_move_idle_enterer, ecore_idle_enterer_del);
   ELM_SAFE_FREE(sd->string_check_idle_enterer, ecore_idle_enterer_del);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_elm_diskselector_efl_gfx_entity_position_set(Eo *obj, Elm_Diskselector_Data *sd, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_gfx_entity_position_set(sd->hit_rect, pos);
}

EOLIAN static void
_elm_diskselector_efl_gfx_entity_size_set(Eo *obj, Elm_Diskselector_Data *sd, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(sd->hit_rect, sz);

   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
}

EOLIAN static void
_elm_diskselector_efl_canvas_group_group_member_add(Eo *obj, Elm_Diskselector_Data *sd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EOLIAN static void
_elm_diskselector_efl_ui_widget_on_access_update(Eo *obj EINA_UNUSED, Elm_Diskselector_Data *_pd EINA_UNUSED, Eina_Bool acs)
{
   _elm_diskselector_smart_focus_next_enable = acs;
}

EAPI Evas_Object *
elm_diskselector_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EOLIAN static Eo *
_elm_diskselector_efl_object_constructor(Eo *obj, Elm_Diskselector_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_LIST);

   return obj;
}

EOLIAN static Eina_Bool
_elm_diskselector_round_enabled_get(const Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd)
{
   return sd->round;
}

EOLIAN static void
_elm_diskselector_round_enabled_set(Eo *obj, Elm_Diskselector_Data *sd, Eina_Bool enabled)
{
   Eina_List *elist;
   Elm_Diskselector_Item_Data *it;
   Evas_Object *blank;

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

EOLIAN static int
_elm_diskselector_side_text_max_length_get(const Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd)
{
   return sd->len_side;
}

EOLIAN static void
_elm_diskselector_side_text_max_length_set(Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd, int len)
{
   sd->len_side = len;
}

EAPI void
elm_diskselector_bounce_set(Evas_Object *obj,
                            Eina_Bool h_bounce,
                            Eina_Bool v_bounce)
{
   ELM_DISKSELECTOR_CHECK(obj);

   elm_interface_scrollable_bounce_allow_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_diskselector_bounce_get(const Evas_Object *obj,
                            Eina_Bool *h_bounce,
                            Eina_Bool *v_bounce)
{
   ELM_DISKSELECTOR_CHECK(obj);

   elm_interface_scrollable_bounce_allow_get((Eo *) obj, h_bounce, v_bounce);
}

EAPI void
elm_diskselector_scroller_policy_get(const Evas_Object *obj,
                                     Elm_Scroller_Policy *policy_h,
                                     Elm_Scroller_Policy *policy_v)
{
   ELM_DISKSELECTOR_CHECK(obj);
   elm_interface_scrollable_policy_get((Eo *) obj, policy_h, policy_v);
}

EOLIAN static void
_elm_diskselector_elm_interface_scrollable_policy_get(const Eo *obj, Elm_Diskselector_Data *sd EINA_UNUSED, Elm_Scroller_Policy *policy_h, Elm_Scroller_Policy *policy_v)
{
   Elm_Scroller_Policy s_policy_h, s_policy_v;

   elm_interface_scrollable_policy_get(efl_super((Eo *)obj, MY_CLASS), &s_policy_h, &s_policy_v);
   if (policy_h) *policy_h = (Elm_Scroller_Policy)s_policy_h;
   if (policy_v) *policy_v = (Elm_Scroller_Policy)s_policy_v;
}

EAPI void
elm_diskselector_scroller_policy_set(Evas_Object *obj,
                                     Elm_Scroller_Policy policy_h,
                                     Elm_Scroller_Policy policy_v)
{
   ELM_DISKSELECTOR_CHECK(obj);
   elm_interface_scrollable_policy_set(obj, policy_h, policy_v);
}

EOLIAN static void
_elm_diskselector_elm_interface_scrollable_policy_set(Eo *obj, Elm_Diskselector_Data *sd EINA_UNUSED, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   elm_interface_scrollable_policy_set(efl_super(obj, MY_CLASS), policy_h, policy_v);
}

EOLIAN static void
_elm_diskselector_clear(Eo *obj, Elm_Diskselector_Data *sd)
{
   Elm_Diskselector_Item_Data *it;

   if (!sd->items) return;

   sd->selected_item = NULL;
   EINA_LIST_FREE(sd->items, it)
     {
        efl_del(EO_OBJ(it));
     }

   _round_items_del(sd);
   sd->r_items = eina_list_free(sd->r_items);
   _sizing_eval(obj);
}

EOLIAN static const Eina_List*
_elm_diskselector_items_get(const Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd)
{
   return sd->items;
}

EOLIAN static Elm_Object_Item*
_elm_diskselector_item_append(Eo *obj, Elm_Diskselector_Data *sd, const char *label, Evas_Object *icon, Evas_Smart_Cb func, const void *data)
{
   Elm_Diskselector_Item_Data *it, *dit;
   Eina_List *elist;

   Evas *evas;

   evas = evas_object_evas_get(obj);
   evas_event_freeze(evas);

   it = _item_new(obj, icon, label, func, data);
   if (!it) return NULL;
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

   return EO_OBJ(it);
}

EOLIAN static Elm_Object_Item*
_elm_diskselector_selected_item_get(const Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd)
{
   return EO_OBJ(sd->selected_item);
}

EOLIAN static void
_elm_diskselector_item_selected_set(Eo *eo_it EINA_UNUSED, Elm_Diskselector_Item_Data *it,
                               Eina_Bool selected)
{
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
     sd->scroller_move_idle_enterer = ecore_idle_enterer_before_add(_scroller_move, WIDGET(it));
}

EOLIAN static Eina_Bool
_elm_diskselector_item_selected_get(const Eo *eo_it EINA_UNUSED, Elm_Diskselector_Item_Data *it)
{
   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   ELM_DISKSELECTOR_DATA_GET(WIDGET(it), sd);

   return sd->selected_item == it;
}

EOLIAN static Elm_Object_Item *
_elm_diskselector_item_prev_get(const Eo *eo_it EINA_UNUSED, Elm_Diskselector_Item_Data *it)
{
   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Diskselector_Item_Data *prev_it = NULL;

   if (it->node->prev) prev_it = it->node->prev->data;
   if (prev_it) return EO_OBJ(prev_it);
   else return NULL;
}

EOLIAN static Elm_Object_Item *
_elm_diskselector_item_next_get(const Eo *eo_it EINA_UNUSED, Elm_Diskselector_Item_Data *it)
{
   ELM_DISKSELECTOR_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Diskselector_Item_Data *next_it = NULL;

   if (it->node->next) next_it = it->node->next->data;
   if (next_it) return EO_OBJ(next_it);
   else return NULL;
}

EOLIAN static Elm_Object_Item*
_elm_diskselector_first_item_get(const Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd)
{
   if (!sd || !sd->items) return NULL;
   Elm_Diskselector_Item_Data *it = eina_list_data_get(sd->items);
   if (it) return EO_OBJ(it);
   else return NULL;
}

EOLIAN static Elm_Object_Item*
_elm_diskselector_last_item_get(const Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd)
{
   if (!sd->items) return NULL;
   Elm_Diskselector_Item_Data *it = eina_list_data_get(eina_list_last(sd->items));
   if (it) return EO_OBJ(it);
   else return NULL;
}

EOLIAN static void
_elm_diskselector_display_item_num_set(Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd, int num)
{
   if (num < DISPLAY_ITEM_NUM_MIN) num = DISPLAY_ITEM_NUM_MIN;
   sd->display_item_num = num;
   sd->display_item_num_by_api = EINA_TRUE;

   if (!sd->round) _blank_item_check(obj, num);
}

EOLIAN static int
_elm_diskselector_display_item_num_get(const Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd)
{
   return sd->display_item_num;
}

static void
_elm_diskselector_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
      _elm_diskselector_smart_focus_next_enable = EINA_TRUE;
}

EOLIAN static const Efl_Access_Action_Data*
_elm_diskselector_efl_access_widget_action_elm_actions_get(const Eo *obj EINA_UNUSED, Elm_Diskselector_Data *sd EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "move,previous", "move", "prev", _key_action_move},
          { "move,next", "move", "next", _key_action_move},
          { "move,first", "move", "first", _key_action_move},
          { "move,last", "move", "last", _key_action_move},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

/* Standard widget overrides */

ELM_WIDGET_KEY_DOWN_DEFAULT_IMPLEMENT(elm_diskselector, Elm_Diskselector_Data)

/* Internal EO APIs and hidden overrides */

#define ELM_DISKSELECTOR_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_diskselector)

#include "elm_diskselector.eo.c"
#include "elm_diskselector_item.eo.c"
