#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED
#define EFL_ACCESS_WIDGET_ACTION_PROTECTED
#define EFL_ACCESS_COMPONENT_PROTECTED
#define ELM_WIDGET_ITEM_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "els_box.h"
#include "elm_widget_layout.h"
#include "elm_widget_index.h"

#define MY_CLASS ELM_INDEX_CLASS

#define MY_CLASS_NAME "Elm_Index"
#define MY_CLASS_NAME_LEGACY "elm_index"

#define INDEX_DELAY_CHANGE_TIME 0.2

static const char SIG_CHANGED[] = "changed";
static const char SIG_DELAY_CHANGED[] = "delay,changed";
static const char SIG_SELECTED[] = "selected";
static const char SIG_LEVEL_UP[] = "level,up";
static const char SIG_LEVEL_DOWN[] = "level,down";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_CHANGED, ""},
   {SIG_DELAY_CHANGED, ""},
   {SIG_SELECTED, ""},
   {SIG_LEVEL_UP, ""},
   {SIG_LEVEL_DOWN, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};

static void
_item_free(Elm_Index_Item_Data *it)
{
   ELM_INDEX_DATA_GET(WIDGET(it), sd);

   sd->items = eina_list_remove(sd->items, EO_OBJ(it));

   if (it->omitted)
     it->omitted = eina_list_free(it->omitted);

   ELM_SAFE_FREE(it->letter, eina_stringshare_del);
}

static void
_box_custom_layout(Evas_Object *o, Evas_Object_Box_Data *priv, void *data)
{
   ELM_INDEX_DATA_GET(data, sd);
   Eina_Bool horizontal;

   horizontal = efl_ui_dir_is_horizontal(sd->dir, EINA_FALSE);
   _els_box_layout(o, priv, horizontal, EINA_TRUE, efl_ui_mirrored_get(data));
}

static void
_index_box_clear(Evas_Object *obj, int level)
{
   Eina_List *l;
   Elm_Object_Item *eo_item;

   ELM_INDEX_DATA_GET(obj, sd);

   if (!sd->level_active[level]) return;

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        if (it->level != level) continue;
        evas_object_box_remove(sd->bx[level], VIEW(it));
        evas_object_hide(VIEW(it));
     }

   sd->level_active[level] = EINA_FALSE;
}

static void
_access_index_register(Evas_Object *obj)
{
   Evas_Object *ao;
   elm_widget_can_focus_set(obj, EINA_TRUE);
   ao = _elm_access_edje_object_part_object_register
              (obj, elm_layout_edje_get(obj), "access");
   _elm_access_text_set
     (_elm_access_info_get(ao), ELM_ACCESS_TYPE, E_("Index"));
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   const char *txt = NULL;

   Elm_Index_Item_Data *it = (Elm_Index_Item_Data *)data;
   ELM_INDEX_ITEM_CHECK_OR_RETURN(it, NULL);

   txt = elm_widget_access_info_get(obj);
   if (!txt) txt = it->letter;
   if (txt) return strdup(txt);

   return NULL;
}

EOLIAN static Evas_Object*
_elm_index_item_elm_widget_item_access_register(Eo *eo_item, Elm_Index_Item_Data *it)
{
   Elm_Access_Info *ai;

   Evas_Object *ret = NULL;
   ret = elm_wdg_item_access_register(efl_super(eo_item, ELM_INDEX_ITEM_CLASS));

   ai = _elm_access_info_get(it->base->access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Index Item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);

   return ret;
}

static void
_omit_calc(void *data, int num_of_items, int max_num_of_items)
{
   Elm_Index_Data *sd = data;
   int max_group_num, num_of_extra_items, i, g, size, sum, start, *group_pos, *omit_info;
   Elm_Index_Omit *o;

   if ((max_num_of_items < 3) || (num_of_items <= max_num_of_items)) return;

   if (sd->group_num > 0)
     start = sd->show_group + sd->default_num;
   else start = 0;

   max_group_num = (max_num_of_items - 1) / 2;
   num_of_extra_items = num_of_items - max_num_of_items;

   group_pos = (int *)malloc(sizeof(int) * max_group_num);
   if (!group_pos)
     {
        ERR("failed to allocate memory!");
        return;
     }
   omit_info = (int *)malloc(sizeof(int) * max_num_of_items);
   if (!omit_info)
     {
        ERR("failed to allocate memory!");
        free(group_pos);
        return;
     }

   if (num_of_extra_items >= max_group_num)
     {
        g = 1;
        for (i = 0; i < max_group_num; i++)
          {
             group_pos[i] = g;
             g += 2;
          }
     }
   else
     {
        size = max_num_of_items / (num_of_extra_items + 1);
        g = size;
        for (i = 0; i < num_of_extra_items; i++)
          {
             group_pos[i] = g;
             g += size;
          }
     }
   for (i = 0; i < max_num_of_items; i++)
     omit_info[i] = 1;
   for (i = 0; i < num_of_extra_items; i++)
     omit_info[group_pos[i % max_group_num]]++;

   sum = 0;
   for (i = 0; i < max_num_of_items; i++)
     {
        if (omit_info[i] > 1)
          {
             o = (Elm_Index_Omit *)malloc(sizeof(Elm_Index_Omit));
             if (!o)
               {
                  ERR("failed to allocate memory!");
                  break;
               }
             o->offset = sum + start;
             o->count = omit_info[i];
             sd->omit = eina_list_append(sd->omit, o);
          }
        sum += omit_info[i];
     }

   free(group_pos);
   free(omit_info);
}

// FIXME: always have index filled
static void
_index_box_auto_fill(Evas_Object *obj,
                     int level)
{
   int i = 0, max_num_of_items = 0, num_of_items = 0, g = 0, skip = 0;
   Eina_List *l;
   Eina_Bool rtl;
   Elm_Object_Item *eo_item;
   Elm_Index_Item_Data *head = NULL, *last_it = NULL;
   Evas_Coord mw, mh, iw, ih;
   Evas_Object *o;
   Elm_Index_Omit *om;
   const char *style = elm_widget_style_get(obj);

   ELM_INDEX_DATA_GET(obj, sd);

   if (sd->level_active[level]) return;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   evas_object_geometry_get(wd->resize_obj, NULL, NULL, &iw, &ih);

   if ((sd->omit_enabled) && (ih <= 0)) return;

   rtl = efl_ui_mirrored_get(obj);

   EINA_LIST_FREE(sd->omit, om)
     free(om);

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        if (it->omitted)
          it->omitted = eina_list_free(it->omitted);
        if (it->head) it->head = NULL;
     }

   if (sd->omit_enabled)
     {
        o = edje_object_add(evas_object_evas_get(obj));
        if (efl_ui_dir_is_horizontal(sd->dir, EINA_FALSE))
          {
             elm_widget_theme_object_set
                (obj, o, "index", "item/horizontal", style);

             edje_object_size_min_restricted_calc(o, &mw, NULL, 0, 0);
             if (mw != 0)
                max_num_of_items = iw / mw;
          }
        else
          {
             elm_widget_theme_object_set
                (obj, o, "index", "item/vertical", style);

             edje_object_size_min_restricted_calc(o, NULL, &mh, 0, 0);
             if (mh != 0)
                max_num_of_items = ih / mh;
          }

        evas_object_del(o);

        EINA_LIST_FOREACH(sd->items, l, eo_item)
          {
             ELM_INDEX_ITEM_DATA_GET(eo_item, it);
             if (it->level == level && it->priority == sd->show_group) num_of_items++;
          }

        if (sd->group_num)
          max_num_of_items -= (sd->group_num + sd->default_num - 1);

        _omit_calc(sd, num_of_items, max_num_of_items);
     }

   om = eina_list_nth(sd->omit, g);
   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        const char *stacking;
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);

        if (it->level != level) continue;

        /** when index has more than one group,
         * one group is shown completely and other groups are represented by one item
         */
        if (it->priority != -1)
          {
             // for groups of higher priority, the first item represents the group
             if (it->priority < sd->show_group)
               {
                  if (last_it && (last_it->priority == it->priority)) continue;
               }
             // for groups of lower priority, the last item represents the group
             else if (it->priority > sd->show_group)
               {
                  l = eina_list_next(l);
                  if (l)
                    {
                       ELM_INDEX_ITEM_DATA_GET(eina_list_data_get(l), next_it);
                       l = eina_list_prev(l);
                       if (next_it->priority == it->priority) continue;
                    }
               }
          }

        if ((om) && (i == om->offset))
          {
             skip = om->count;
             skip--;
             head = it;
             it->head = head;
             head->omitted = eina_list_append(head->omitted, it);
             om = eina_list_nth(sd->omit, ++g);
          }
        else if (skip > 0)
          {
             skip--;
             i++;
             if (head)
               {
                  it->head = head;
                  head->omitted = eina_list_append(head->omitted, it);
                  // if it is selected but omitted, send signal to it's head
                  if (it->selected)
                    edje_object_signal_emit(VIEW(it->head), "elm,state,active", "elm");
               }
             continue;
          }

        edje_object_mirrored_set(VIEW(it), rtl);
        o = VIEW(it);

        if (efl_ui_dir_is_horizontal(sd->dir, EINA_FALSE))
          {
             if (i & 0x1)
               elm_widget_theme_object_set
                 (obj, o, "index", "item_odd/horizontal", style);
             else
               elm_widget_theme_object_set
                 (obj, o, "index", "item/horizontal", style);
          }
        else
          {
             if (i & 0x1)
               elm_widget_theme_object_set
                 (obj, o, "index", "item_odd/vertical", style);
             else
               elm_widget_theme_object_set
                 (obj, o, "index", "item/vertical", style);
          }

        if (skip > 0)
          edje_object_part_text_escaped_set(o, "elm.text", "*");
        else
          edje_object_part_text_escaped_set(o, "elm.text", it->letter);
        edje_object_size_min_restricted_calc(o, &mw, &mh, 0, 0);
        evas_object_size_hint_min_set(o, mw, mh);
        evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_widget_sub_object_add(obj, o);
        evas_object_box_append(sd->bx[level], o);
        stacking = edje_object_data_get(o, "stacking");

        if (it->selected)
          edje_object_signal_emit(o, "elm,state,active", "elm");

        if (stacking)
          {
             if (!strcmp(stacking, "below")) evas_object_lower(o);
             else if (!strcmp(stacking, "above"))
               evas_object_raise(o);
          }

        evas_object_show(o);

        i++;

        // ACCESS
        if ((it->level == 0) &&
            (_elm_config->access_mode == ELM_ACCESS_MODE_ON))
          elm_wdg_item_access_register(eo_item);

        last_it = it;
     }

   evas_object_smart_calculate(sd->bx[level]);
   sd->level_active[level] = EINA_TRUE;
}

static void
_priority_change_job(void *data)
{
   ELM_INDEX_DATA_GET(data, sd);
   Elm_Object_Item *selected_it;

   sd->show_group = sd->next_group;
   _index_box_clear(data, 0);
   _index_box_auto_fill(data, 0);

   selected_it = elm_index_selected_item_get(data, sd->level);
   if (selected_it)
     elm_index_item_selected_set(selected_it, EINA_FALSE);
}

static void
_priority_up_cb(void *data)
{
   _priority_change_job(data);
}

static void
_priority_down_cb(void *data)
{
   _priority_change_job(data);
}

static void
_index_priority_change(void *data, Elm_Index_Item_Data *it)
{
   ELM_INDEX_DATA_GET(data, sd);

   if ((it->priority != -1) && (it->priority != sd->show_group))
     {
        sd->next_group = it->priority;
        if (it->priority < sd->show_group)
          _priority_up_cb(data);
        else
          _priority_down_cb(data);
     }
}

EOLIAN static Eina_Error
_elm_index_efl_ui_widget_theme_apply(Eo *obj, Elm_Index_Data *sd)
{
   Evas_Coord minw = 0, minh = 0;
   Elm_Object_Item *eo_item;

   Eina_Error int_ret = EFL_UI_THEME_APPLY_ERROR_GENERIC;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EINA_FALSE);

   _index_box_clear(obj, 0);
   _index_box_clear(obj, 1);

   if (efl_ui_dir_is_horizontal(sd->dir, EINA_FALSE))
     elm_widget_theme_element_set(obj, "base/horizontal");
   else
     elm_widget_theme_element_set(obj, "base/vertical");

   int_ret = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (int_ret == EFL_UI_THEME_APPLY_ERROR_GENERIC) return int_ret;

   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(sd->event_rect[0], minw, minh);

   if (edje_object_part_exists
         (wd->resize_obj, "elm.swallow.index.1"))
     {
        if (!sd->bx[1])
          {
             sd->bx[1] = evas_object_box_add(evas_object_evas_get(obj));
             evas_object_box_layout_set
               (sd->bx[1], _box_custom_layout, obj, NULL);
             elm_widget_sub_object_add(obj, sd->bx[1]);
          }
        elm_layout_content_set(obj, "elm.swallow.index.1", sd->bx[1]);
     }
   else
     ELM_SAFE_FREE(sd->bx[1], evas_object_del);

   if (edje_object_part_exists
         (wd->resize_obj, "elm.swallow.event.1"))
     {
        if (!sd->event_rect[1])
          {
             sd->event_rect[1] =
               evas_object_rectangle_add(evas_object_evas_get(obj));
             evas_object_color_set(sd->event_rect[1], 0, 0, 0, 0);
             elm_widget_sub_object_add(obj, sd->event_rect[1]);
          }
        elm_layout_content_set(obj, "elm.swallow.event.1", sd->event_rect[1]);
        evas_object_size_hint_min_set(sd->event_rect[1], minw, minh);
     }
   else
     ELM_SAFE_FREE(sd->event_rect[1], evas_object_del);

   edje_object_message_signal_process(wd->resize_obj);

   elm_layout_sizing_eval(obj);
   _index_box_auto_fill(obj, 0);

   if (sd->autohide_disabled)
     {
        if (sd->level == 1) _index_box_auto_fill(obj, 1);
        elm_layout_signal_emit(obj, "elm,state,active", "elm");
     }
   else elm_layout_signal_emit(obj, "elm,state,inactive", "elm");

   eo_item = elm_index_selected_item_get(obj, sd->level);
   if (eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        if (it->head)
          edje_object_signal_emit(VIEW(it->head), "elm,state,active", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,active", "elm");
     }

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_index_register(obj);

   return int_ret;
}

EOLIAN static void
_elm_index_elm_layout_sizing_eval(Eo *obj, Elm_Index_Data *_pd EINA_UNUSED)
{
   Evas_Coord minw = -1, minh = -1;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);
}

EOLIAN static void
_elm_index_item_efl_object_destructor(Eo *eo_item EINA_UNUSED, Elm_Index_Item_Data *it)
{
   ELM_INDEX_DATA_GET(WIDGET(it), sd);

   _item_free(it);
   _index_box_clear(WIDGET(it), sd->level);

   efl_destructor(efl_super(eo_item, ELM_INDEX_ITEM_CLASS));
}

EOLIAN static Eo *
_elm_index_item_efl_object_constructor(Eo *obj, Elm_Index_Item_Data *it)
{
   obj = efl_constructor(efl_super(obj, ELM_INDEX_ITEM_CLASS));
   it->base = efl_data_scope_get(obj, ELM_WIDGET_ITEM_CLASS);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_RADIO_MENU_ITEM);

   return obj;
}

static Elm_Object_Item *
_item_new(Evas_Object *obj,
          const char *letter,
          Evas_Smart_Cb func,
          const void *data)
{
   Eo *eo_item;

   ELM_INDEX_DATA_GET(obj, sd);

   eo_item = efl_add(ELM_INDEX_ITEM_CLASS, obj);
   if (!eo_item) return NULL;

   ELM_INDEX_ITEM_DATA_GET(eo_item, it);

   if (letter) it->letter = eina_stringshare_add(letter);
   it->func = func;
   WIDGET_ITEM_DATA_SET(EO_OBJ(it), data);
   it->level = sd->level;
   it->priority = -1;

   return eo_item;
}

static Elm_Index_Item_Data *
_item_find(Evas_Object *obj,
           const void *data)
{
   Eina_List *l;
   Elm_Object_Item *eo_item;

   ELM_INDEX_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        if (WIDGET_ITEM_DATA_GET(eo_item) == data) return it;
     }
   return NULL;
}

static Eina_Bool
_delay_change_cb(void *data)
{
   Elm_Object_Item *item;

   ELM_INDEX_DATA_GET(data, sd);

   sd->delay = NULL;
   item = elm_index_selected_item_get(data, sd->level);

   if (item)
     {
        efl_event_callback_legacy_call
              (data, ELM_INDEX_EVENT_DELAY_CHANGED, item);
        ELM_INDEX_ITEM_DATA_GET(item, it);
        _index_priority_change(data, it);
     }
   return ECORE_CALLBACK_CANCEL;
}

static void
_sel_eval(Evas_Object *obj,
          Evas_Coord evx,
          Evas_Coord evy)
{
   Evas_Coord x, y, w, h, bx, by, bw, bh, xx, yy;
   Elm_Index_Item_Data *it, *it_closest, *it_last, *om_closest;
   Elm_Object_Item *eo_item;
   char *label = NULL, *last = NULL;
   double cdv = 0.5;
   Evas_Coord dist;
   Eina_List *l;
   int i, j, size, dh, dx, dy;

   ELM_INDEX_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   for (i = 0; i <= sd->level; i++)
     {
        it_last = NULL;
        it_closest = NULL;
        om_closest = NULL;
        dist = 0x7fffffff;
        evas_object_geometry_get(sd->bx[i], &bx, &by, &bw, &bh);

        EINA_LIST_FOREACH(sd->items, l, eo_item)
          {
             it = efl_data_scope_get(eo_item, ELM_INDEX_ITEM_CLASS);
             if (it->level != i) continue;
             if (it->level != sd->level)
               {
                  if (it->selected)
                    {
                       it_closest = it;
                       break;
                    }
                  continue;
               }
             if (it->selected)
               {
                  it_last = it;
                  it->selected = EINA_FALSE;
               }
             if (evas_object_visible_get(VIEW(it)))
               {
                  evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
                  xx = x + (w / 2);
                  yy = y + (h / 2);
                  x = evx - xx;
                  y = evy - yy;
                  x = (x * x) + (y * y);
                  if ((x < dist) || (!it_closest))
                    {
                       if (efl_ui_dir_is_horizontal(sd->dir, EINA_FALSE))
                         cdv = (double)(xx - bx) / (double)bw;
                       else
                         cdv = (double)(yy - by) / (double)bh;
                       it_closest = it;
                       dist = x;
                    }
               }
          }
        if ((i == 0) && (sd->level == 0))
          edje_object_part_drag_value_set
            (wd->resize_obj, "elm.dragable.index.1", cdv, cdv);

        if (it_closest && it_closest->omitted)
          {
             it = it_closest;
             size = eina_list_count(it->omitted);
             evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
             dist = 0x7fffffff;
             dh = h / size;
             if (dh == 0)
               WRN("too many index items to omit."); //FIXME
             else
               {
                  for (j = 0; j < size; j++)
                    {
                       xx = x + (w / 2);
                       yy = y + (dh * j) + (dh / 2);
                       dx = evx - xx;
                       dy = evy - yy;
                       dx = (dx * dx) + (dy * dy);
                       if ((dx < dist) || (!om_closest))
                         {
                            om_closest = eina_list_nth(it->omitted, j);
                            dist = dx;
                         }
                    }
               }
          }

        if (om_closest) om_closest->selected = EINA_TRUE;
        else if (it_closest) it_closest->selected = EINA_TRUE;

        if (it_closest != it_last)
          {
             if (it_last)
               {
                  const char *stacking, *selectraise;

                  it = it_last;
                  if (it->head)
                    {
                       if (it->head != it_closest) it = it->head;
                       else it = NULL;
                    }
                  if (it)
                    {
                       edje_object_signal_emit
                          (VIEW(it), "elm,state,inactive", "elm");
                       stacking = edje_object_data_get(VIEW(it), "stacking");
                       selectraise = edje_object_data_get(VIEW(it), "selectraise");
                       if ((selectraise) && (!strcmp(selectraise, "on")))
                         {
                            if ((stacking) && (!strcmp(stacking, "below")))
                              evas_object_lower(VIEW(it));
                         }
                    }
               }
             if (it_closest)
               {
                  const char *selectraise;

                  it = it_closest;

                  if (!((it_last) && (it_last->head) && (it_last->head == it_closest)))
                    {
                       edje_object_signal_emit(VIEW(it), "elm,state,active", "elm");
                       selectraise = edje_object_data_get(VIEW(it), "selectraise");
                       if ((selectraise) && (!strcmp(selectraise, "on")))
                         evas_object_raise(VIEW(it));
                    }

                  // ACCESS
                  if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
                    {
                       char *ret;
                       Eina_Strbuf *buf;
                       buf = eina_strbuf_new();

                       if (om_closest)
                         eina_strbuf_append_printf(buf, "index item %s clicked", om_closest->letter);
                       else
                         eina_strbuf_append_printf(buf, "index item %s clicked", it->letter);
                       ret = eina_strbuf_string_steal(buf);
                       eina_strbuf_free(buf);

                       _elm_access_highlight_set(it->base->access_obj);
                       _elm_access_say(ret);
                    }

                  if (om_closest) 
                    efl_event_callback_legacy_call
                      (obj, ELM_INDEX_EVENT_CHANGED, EO_OBJ(om_closest));
                  else
                    efl_event_callback_legacy_call
                      (obj, ELM_INDEX_EVENT_CHANGED, EO_OBJ(it));
                  ecore_timer_del(sd->delay);
                  sd->delay = ecore_timer_add(sd->delay_change_time,
                                              _delay_change_cb, obj);
               }
          }
        if (it_closest)
          {
             if (om_closest) it = om_closest;
             else it = it_closest;
             if (!last && it->letter) last = strdup(it->letter);
             else
               {
                  if (!label && last) label = strdup(last);
                  else
                    {
                       if (label && last)
                         {
                            char *temp;

                            temp = realloc(label, strlen(label) +
                                            strlen(last) + 1);
                            if (!temp)
                              {
                                 free(label);
                                 free(last);
                                 return;
                              }
                            label = strcat(temp, last);
                         }
                    }
                  free(last);
                  last = NULL;
                  if (it->letter) last = strdup(it->letter);
               }
          }
     }
   if (!label) label = strdup("");
   if (!last) last = strdup("");

   elm_layout_text_set(obj, "elm.text.body", label);
   elm_layout_text_set(obj, "elm.text", last);

   free(label);
   free(last);
}

static void
_on_mouse_down(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *o EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Down *ev = event_info;
   Evas_Coord x, y, w;

   ELM_INDEX_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   if (ev->button != 1) return;
   sd->mouse_down = EINA_TRUE;
   evas_object_geometry_get(wd->resize_obj, &x, &y, &w, NULL);
   sd->dx = ev->canvas.x - x;
   sd->dy = ev->canvas.y - y;
   if (!sd->autohide_disabled)
     {
        _index_box_clear(data, 1);
        elm_layout_signal_emit(data, "elm,state,active", "elm");
     }
   _sel_eval(data, ev->canvas.x, ev->canvas.y);
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.pointer",
     (!elm_object_mirrored_get(data)) ? sd->dx : (sd->dx - w), sd->dy);
   if (sd->items && !sd->indicator_disabled)
     elm_layout_signal_emit(data, "elm,indicator,state,active", "elm");
}

static void
_on_mouse_up(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *o EINA_UNUSED,
             void *event_info)
{
   Evas_Event_Mouse_Up *ev = event_info;
   Elm_Object_Item *eo_item;
   Elm_Object_Item *eo_id_item;

   ELM_INDEX_DATA_GET(data, sd);

   if (ev->button != 1) return;
   sd->mouse_down = EINA_FALSE;
   eo_item = elm_index_selected_item_get(data, sd->level);
   if (eo_item)
     {
        efl_event_callback_legacy_call
          (data, EFL_UI_EVENT_CLICKED, eo_item);
        evas_object_smart_callback_call(data, "selected", eo_item);
        eo_id_item = eo_item;
        ELM_INDEX_ITEM_DATA_GET(eo_id_item, id_item);
        if (id_item->func)
          id_item->func((void *)WIDGET_ITEM_DATA_GET(eo_id_item), WIDGET(id_item), eo_id_item);
     }
   if (!sd->autohide_disabled)
     elm_layout_signal_emit(data, "elm,state,inactive", "elm");

   elm_layout_signal_emit(data, "elm,state,level,0", "elm");
   if (sd->items && !sd->indicator_disabled)
     elm_layout_signal_emit(data, "elm,indicator,state,inactive", "elm");
}

static void
_on_mouse_move(void *data,
               Evas *e EINA_UNUSED,
               Evas_Object *o EINA_UNUSED,
               void *event_info)
{
   Evas_Event_Mouse_Move *ev = event_info;
   Evas_Coord minw = 0, minh = 0, x, y, dx, adx, w;
   char buf[1024];

   ELM_INDEX_DATA_GET(data, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(data, wd);

   if (!sd->mouse_down) return;
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_geometry_get(wd->resize_obj, &x, &y, &w, NULL);
   x = ev->cur.canvas.x - x;
   y = ev->cur.canvas.y - y;
   dx = x - sd->dx;
   adx = dx;
   if (adx < 0) adx = -dx;
   edje_object_part_drag_value_set
     (wd->resize_obj, "elm.dragable.pointer",
     (!edje_object_mirrored_get(wd->resize_obj)) ?
     x : (x - w), y);
   if (!efl_ui_dir_is_horizontal(sd->dir, EINA_FALSE) && (sd->event_rect[1]))
     {
        if (adx > minw)
          {
             if (!sd->level)
               {
                  sd->level = 1;
                  snprintf(buf, sizeof(buf), "elm,state,level,%i", sd->level);
                  elm_layout_signal_emit(data, buf, "elm");
                  efl_event_callback_legacy_call
                    (data, ELM_INDEX_EVENT_LEVEL_UP, NULL);
               }
          }
        else
          {
             if (sd->level == 1)
               {
                  sd->level = 0;
                  snprintf(buf, sizeof(buf), "elm,state,level,%i", sd->level);
                  elm_layout_signal_emit(data, buf, "elm");
                  efl_event_callback_legacy_call
                    (data, ELM_INDEX_EVENT_LEVEL_DOWN, NULL);
               }
          }
     }
   _sel_eval(data, ev->cur.canvas.x, ev->cur.canvas.y);
}

static void
_on_mouse_in_access(void *data,
                    Evas *e EINA_UNUSED,
                    Evas_Object *o EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   ELM_INDEX_DATA_GET(data, sd);

   if (sd->mouse_down) return;

   if (!sd->autohide_disabled)
     {
        _index_box_clear(data, 1);
        elm_layout_signal_emit(data, "elm,state,active", "elm");
     }
}

static void
_on_mouse_move_access(void *data,
                      Evas *e EINA_UNUSED,
                      Evas_Object *o EINA_UNUSED,
                      void *event_info)
{

   Evas_Event_Mouse_Down *ev = event_info;
   Elm_Object_Item *eo_item;
   Elm_Index_Item_Data *it_closest;
   Eina_List *l;
   Evas_Coord dist = 0;
   Evas_Coord x, y, w, h, xx, yy;

   ELM_INDEX_DATA_GET(data, sd);

   it_closest = NULL;
   dist = 0x7fffffff;

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
        xx = x + (w / 2);
        yy = y + (h / 2);
        x = ev->canvas.x - xx;
        y = ev->canvas.y - yy;
        x = (x * x) + (y * y);
        if ((x < dist) || (!it_closest))
          {
             it_closest = it;
             dist = x;
          }
     }

   if (it_closest)
     _elm_access_highlight_set(it_closest->base->access_obj);
}

static void
_on_mouse_out_access(void *data,
                      Evas *e EINA_UNUSED,
                      Evas_Object *o EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   ELM_INDEX_DATA_GET(data, sd);

   if (!sd->autohide_disabled)
     elm_layout_signal_emit(data, "elm,state,inactive", "elm");
}

static void
_index_resize_cb(void *data EINA_UNUSED,
                 Evas *e EINA_UNUSED,
                 Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   ELM_INDEX_DATA_GET_OR_RETURN(obj, sd);

   if (!sd->omit_enabled) return;

   Elm_Object_Item *eo_item;

   _index_box_clear(obj, 0);
   _index_box_auto_fill(obj, 0);

   eo_item = elm_index_selected_item_get(obj, sd->level);
   if (eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        if (it->head)
          edje_object_signal_emit(VIEW(it->head), "elm,state,active", "elm");
        else
          edje_object_signal_emit(VIEW(it), "elm,state,active", "elm");
     }
}

static int
_sort_cb(const void *d1, const void *d2)
{
   Elm_Object_Item *eo_it1 = (Elm_Object_Item *)d1, *eo_it2 = (Elm_Object_Item *)d2;

   ELM_INDEX_ITEM_DATA_GET(eo_it1, it1);
   ELM_INDEX_ITEM_DATA_GET(eo_it2, it2);
   if (it1->priority <= it2->priority) return -1;
   else return 1;
}


EOLIAN static void
_elm_index_efl_canvas_group_group_add(Eo *obj, Elm_Index_Data *priv)
{
   Evas_Object *o;
   Evas_Coord minw = 0, minh = 0;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   if (!elm_layout_theme_set
       (obj, "index", "base/vertical", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _index_resize_cb, NULL);

   priv->event_rect[0] = o = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_color_set(o, 0, 0, 0, 0);
   elm_coords_finger_size_adjust(1, &minw, 1, &minh);
   evas_object_size_hint_min_set(o, minw, minh);
   elm_layout_content_set(obj, "elm.swallow.event.0", o);

   evas_object_event_callback_add
     (o, EVAS_CALLBACK_MOUSE_DOWN, _on_mouse_down, obj);
   evas_object_event_callback_add
     (o, EVAS_CALLBACK_MOUSE_UP, _on_mouse_up, obj);
   evas_object_event_callback_add
     (o, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move, obj);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     {
        evas_object_event_callback_add
          (o, EVAS_CALLBACK_MOUSE_IN, _on_mouse_in_access, obj);
        evas_object_event_callback_add
          (o, EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move_access, obj);
        evas_object_event_callback_add
          (o, EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out_access, obj);
     }

   if (edje_object_part_exists
         (wd->resize_obj, "elm.swallow.event.1"))
     {
        o = evas_object_rectangle_add(evas_object_evas_get(obj));
        priv->event_rect[1] = o;
        evas_object_color_set(o, 0, 0, 0, 0);
        evas_object_size_hint_min_set(o, minw, minh);
        elm_layout_content_set(obj, "elm.swallow.event.1", o);
     }

   priv->bx[0] = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_box_layout_set(priv->bx[0], _box_custom_layout, obj, NULL);
   elm_layout_content_set(obj, "elm.swallow.index.0", priv->bx[0]);
   evas_object_show(priv->bx[0]);

   priv->delay_change_time = INDEX_DELAY_CHANGE_TIME;
   priv->dir = EFL_UI_DIR_VERTICAL;

   if (edje_object_part_exists
         (wd->resize_obj, "elm.swallow.index.1"))
     {
        priv->bx[1] = evas_object_box_add(evas_object_evas_get(obj));
        evas_object_box_layout_set
          (priv->bx[1], _box_custom_layout, obj, NULL);
        elm_widget_sub_object_add(obj, priv->bx[1]);
        elm_layout_content_set(obj, "elm.swallow.index.1", priv->bx[1]);
        evas_object_show(priv->bx[1]);
     }

   elm_layout_sizing_eval(obj);
   elm_widget_can_focus_set(obj, EINA_FALSE);

   // ACCESS
   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_index_register(obj);
}

EOLIAN static void
_elm_index_efl_canvas_group_group_del(Eo *obj, Elm_Index_Data *sd)
{
   Elm_Index_Omit *o;

   while (sd->items)
     efl_del(sd->items->data);

   EINA_LIST_FREE(sd->omit, o)
     free(o);

   ecore_timer_del(sd->delay);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

static Eina_Bool _elm_index_smart_focus_next_enable = EINA_FALSE;

static void
_access_obj_process(Evas_Object *obj, Eina_Bool is_access)
{
   Eina_List *l;
   Elm_Object_Item *eo_item;

   ELM_INDEX_DATA_GET(obj, sd);

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        if (it->level != 0) continue;
        if (is_access) elm_wdg_item_access_register(eo_item);
        else elm_wdg_item_access_unregister(eo_item);
     }

   if (is_access)
     {
        _access_index_register(obj);

        evas_object_event_callback_add
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_IN, _on_mouse_in_access, obj);
        evas_object_event_callback_add
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move_access, obj);
        evas_object_event_callback_add
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out_access, obj);
     }
   else
     {
        // opposition of  _access_index_register();
        elm_widget_can_focus_set(obj, EINA_FALSE);
        _elm_access_edje_object_part_object_unregister
             (obj, elm_layout_edje_get(obj), "access");

        evas_object_event_callback_del_full
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_IN, _on_mouse_in_access, obj);
        evas_object_event_callback_del_full
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_MOVE, _on_mouse_move_access, obj);
        evas_object_event_callback_del_full
          (sd->event_rect[0], EVAS_CALLBACK_MOUSE_OUT, _on_mouse_out_access, obj);
     }
}

EOLIAN static void
_elm_index_efl_ui_widget_on_access_update(Eo *obj, Elm_Index_Data *_pd EINA_UNUSED, Eina_Bool acs)
{
   _elm_index_smart_focus_next_enable = acs;
   _access_obj_process(obj, _elm_index_smart_focus_next_enable);
}

EAPI Evas_Object *
elm_index_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(MY_CLASS, parent);
}

EAPI void elm_index_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   Efl_Ui_Dir dir = horizontal ? EFL_UI_DIR_HORIZONTAL : EFL_UI_DIR_VERTICAL;

   efl_ui_direction_set(obj, dir);
}

EAPI Eina_Bool elm_index_horizontal_get(const Evas_Object *obj)
{
   Efl_Ui_Dir dir = efl_ui_direction_get(obj);

   return efl_ui_dir_is_horizontal(dir, EINA_FALSE);
}

EOLIAN static Eo *
_elm_index_efl_object_constructor(Eo *obj, Elm_Index_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_SCROLL_BAR);

   return obj;
}

EOLIAN static void
_elm_index_autohide_disabled_set(Eo *obj, Elm_Index_Data *sd, Eina_Bool disabled)
{
   disabled = !!disabled;
   if (sd->autohide_disabled == disabled) return;
   sd->autohide_disabled = disabled;
   sd->level = 0;
   if (sd->autohide_disabled)
     {
        _index_box_clear(obj, 1);
        elm_layout_signal_emit(obj, "elm,state,active", "elm");
     }
   else
     elm_layout_signal_emit(obj, "elm,state,inactive", "elm");

   //FIXME: Should be update indicator based on the indicator visibility
}

EOLIAN static Eina_Bool
_elm_index_autohide_disabled_get(const Eo *obj EINA_UNUSED, Elm_Index_Data *sd)
{
   return sd->autohide_disabled;
}

EOLIAN static void
_elm_index_item_level_set(Eo *obj EINA_UNUSED, Elm_Index_Data *sd, int level)
{
   if (sd->level == level) return;
   sd->level = level;
}

EOLIAN static int
_elm_index_item_level_get(const Eo *obj EINA_UNUSED, Elm_Index_Data *sd)
{
   return sd->level;
}

//FIXME: Should update indicator based on the autohidden status & indicator visibility
EOLIAN static void
_elm_index_item_selected_set(Eo *eo_it,
                             Elm_Index_Item_Data *it,
                             Eina_Bool selected)
{
   Elm_Index_Item_Data *it_sel, *it_inactive, *it_active;
   Evas_Object *obj = WIDGET(it);

   ELM_INDEX_ITEM_CHECK_OR_RETURN(it);
   ELM_INDEX_DATA_GET(obj, sd);

   selected = !!selected;
   it_sel = it;
   if (it_sel->selected == selected) return;

   if (selected)
     {
        Elm_Object_Item *eo_it_last = elm_index_selected_item_get(obj, sd->level);

        if (eo_it_last)
          {
             ELM_INDEX_ITEM_DATA_GET(eo_it_last, it_last);
             it_last->selected = EINA_FALSE;
             if (it_last->head)
               it_inactive = it_last->head;
             else
               it_inactive = it_last;

             edje_object_signal_emit(VIEW(it_inactive),
                                     "elm,state,inactive", "elm");
             edje_object_message_signal_process(VIEW(it_inactive));
          }

        it_sel->selected = EINA_TRUE;
        if (it_sel->head)
          it_active = it_sel->head;
        else
          it_active = it_sel;

        edje_object_signal_emit(VIEW(it_active), "elm,state,active", "elm");
        edje_object_message_signal_process(VIEW(it_active));

        efl_event_callback_legacy_call
          (obj, ELM_INDEX_EVENT_CHANGED, eo_it);
        evas_object_smart_callback_call(obj, "selected", eo_it);
        ecore_timer_del(sd->delay);
        sd->delay = ecore_timer_add(sd->delay_change_time,
                                    _delay_change_cb, obj);
        if (_elm_config->atspi_mode)
          efl_access_state_changed_signal_emit(eo_it, EFL_ACCESS_STATE_TYPE_SELECTED, EINA_TRUE);
     }
   else
     {
        it_sel->selected = EINA_FALSE;
        if (it_sel->head)
          it_inactive = it_sel->head;
        else
          it_inactive = it_sel;

        edje_object_signal_emit(VIEW(it_inactive), "elm,state,inactive", "elm");
        edje_object_message_signal_process(VIEW(it_inactive));

        // for the case in which the selected item is unselected before mouse up
        elm_layout_signal_emit(obj, "elm,indicator,state,inactive", "elm");
     }
}

EOLIAN static Elm_Object_Item*
_elm_index_selected_item_get(const Eo *obj EINA_UNUSED, Elm_Index_Data *sd, int level)
{
   Eina_List *l;
   Elm_Object_Item *eo_item;

   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        if ((it->selected) && (it->level == level))
          {
             return eo_item;
          }
     }

   return NULL;
}

EOLIAN static Elm_Object_Item*
_elm_index_item_append(Eo *obj, Elm_Index_Data *sd, const char *letter, Evas_Smart_Cb func, const void *data)
{
   Elm_Object_Item *eo_item;

   eo_item = _item_new(obj, letter, func, data);
   if (!eo_item) return NULL;

   sd->items = eina_list_append(sd->items, eo_item);

   ELM_INDEX_ITEM_DATA_GET(eo_item, it);
   VIEW_SET(it, edje_object_add(evas_object_evas_get(obj)));

   if (_elm_config->atspi_mode)
     {
        efl_access_added(eo_item);
        efl_access_children_changed_added_signal_emit(obj, eo_item);
     }

   return eo_item;
}

EOLIAN static Elm_Object_Item*
_elm_index_item_prepend(Eo *obj, Elm_Index_Data *sd, const char *letter, Evas_Smart_Cb func, const void *data)
{
   Elm_Object_Item *eo_item;

   eo_item = _item_new(obj, letter, func, data);
   if (!eo_item) return NULL;

   sd->items = eina_list_prepend(sd->items, eo_item);

   ELM_INDEX_ITEM_DATA_GET(eo_item, it);
   VIEW_SET(it, edje_object_add(evas_object_evas_get(obj)));

   if (_elm_config->atspi_mode)
     {
        efl_access_added(eo_item);
        efl_access_children_changed_added_signal_emit(obj, eo_item);
     }

   return eo_item;
}

EINA_DEPRECATED EAPI Elm_Object_Item *
elm_index_item_prepend_relative(Evas_Object *obj,
                                const char *letter,
                                const void *item,
                                const Elm_Object_Item *relative)
{
   return elm_index_item_insert_before
            (obj, (Elm_Object_Item *)relative, letter, NULL, item);
}

EOLIAN static Elm_Object_Item*
_elm_index_item_insert_after(Eo *obj, Elm_Index_Data *sd, Elm_Object_Item *after, const char *letter, Evas_Smart_Cb func, const void *data)
{
   Elm_Object_Item *eo_item;


   if (!after) return elm_index_item_append(obj, letter, func, data);

   eo_item = _item_new(obj, letter, func, data);
   if (!eo_item) return NULL;

   sd->items = eina_list_append_relative(sd->items, eo_item, after);

   ELM_INDEX_ITEM_DATA_GET(eo_item, it);
   VIEW_SET(it, edje_object_add(evas_object_evas_get(obj)));

   if (_elm_config->atspi_mode)
     {
        efl_access_added(eo_item);
        efl_access_children_changed_added_signal_emit(obj, eo_item);
     }

   return eo_item;
}

EOLIAN static Elm_Object_Item*
_elm_index_item_insert_before(Eo *obj, Elm_Index_Data *sd, Elm_Object_Item *before, const char *letter, Evas_Smart_Cb func, const void *data)
{
   Elm_Object_Item *eo_item;

   if (!before) return elm_index_item_prepend(obj, letter, func, data);

   eo_item = _item_new(obj, letter, func, data);
   if (!eo_item) return NULL;

   sd->items = eina_list_prepend_relative(sd->items, eo_item, before);

   ELM_INDEX_ITEM_DATA_GET(eo_item, it);
   VIEW_SET(it, edje_object_add(evas_object_evas_get(obj)));

   if (_elm_config->atspi_mode)
     {
        efl_access_added(eo_item);
        efl_access_children_changed_added_signal_emit(obj, eo_item);
     }

   return eo_item;
}

EOLIAN static Elm_Object_Item*
_elm_index_item_sorted_insert(Eo *obj, Elm_Index_Data *sd, const char *letter, Evas_Smart_Cb func, const void *data, Eina_Compare_Cb cmp_func, Eina_Compare_Cb cmp_data_func)
{
   Elm_Object_Item *eo_item;
   Eina_List *lnear;
   int cmp;

   if (!(sd->items)) return elm_index_item_append(obj, letter, func, data);

   eo_item = _item_new(obj, letter, func, data);
   if (!eo_item) return NULL;

   lnear = eina_list_search_sorted_near_list(sd->items, cmp_func, eo_item, &cmp);
   if (cmp < 0)
     sd->items = eina_list_append_relative_list(sd->items, eo_item, lnear);
   else if (cmp > 0)
     sd->items = eina_list_prepend_relative_list(sd->items, eo_item, lnear);
   else
     {
        /* If cmp_data_func is not provided, append a duplicated item */
        if (!cmp_data_func)
          sd->items = eina_list_append_relative_list(sd->items, eo_item, lnear);
        else
          {
             Elm_Object_Item *eo_p_it = eina_list_data_get(lnear);
             const void *item_data = WIDGET_ITEM_DATA_GET(eo_item);
             if (cmp_data_func(WIDGET_ITEM_DATA_GET(eo_p_it), item_data) >= 0)
               WIDGET_ITEM_DATA_SET(eo_p_it, item_data);
             efl_del(eo_item);
             return NULL;
          }
     }
   ELM_INDEX_ITEM_DATA_GET(eo_item, it);
   VIEW_SET(it, edje_object_add(evas_object_evas_get(obj)));

   if (_elm_config->atspi_mode)
     {
        efl_access_added(eo_item);
        efl_access_children_changed_added_signal_emit(obj, eo_item);
     }

   return eo_item;
}

EOLIAN static Elm_Object_Item*
_elm_index_item_find(Eo *obj, Elm_Index_Data *_pd EINA_UNUSED, const void *data)
{
   Elm_Index_Item_Data *it = _item_find(obj, data);
   return EO_OBJ(it);
}

EOLIAN static void
_elm_index_item_clear(Eo *obj, Elm_Index_Data *sd)
{
   Elm_Object_Item *eo_item;
   Eina_List *l, *clear = NULL;

   _index_box_clear(obj, sd->level);
   EINA_LIST_FOREACH(sd->items, l, eo_item)
     {
        ELM_INDEX_ITEM_DATA_GET(eo_item, it);
        if (it->level != sd->level) continue;
        clear = eina_list_append(clear, eo_item);
     }
   EINA_LIST_FREE(clear, eo_item)
     efl_del(eo_item);
}

EOLIAN static void
_elm_index_level_go(Eo *obj, Elm_Index_Data *sd, int level)
{
   Elm_Object_Item *eo_it;
   Eina_List *l;
   int prev;

   sd->items = eina_list_sort(sd->items, 0, EINA_COMPARE_CB(_sort_cb));

   if (level == 0)
     {
        sd->default_num = 0;
        sd->group_num = 0;
        sd->show_group = -1;
        prev = -1;
        EINA_LIST_FOREACH(sd->items, l, eo_it)
          {
             ELM_INDEX_ITEM_DATA_GET(eo_it, it);
             if (it->priority == -1) sd->default_num++;
             if (it->priority != prev)
               {
                  if (prev == -1) sd->show_group = it->priority;
                  sd->group_num++;
                  prev = it->priority;
               }
          }
     }

   _index_box_clear(obj, 0);
   _index_box_auto_fill(obj, 0);
   if (sd->level == 1)
     {
        _index_box_clear(obj, 1);
        _index_box_auto_fill(obj, 1);
     }
}

EOLIAN static void
_elm_index_indicator_disabled_set(Eo *obj, Elm_Index_Data *sd, Eina_Bool disabled)
{
   disabled = !!disabled;
   if (sd->indicator_disabled == disabled) return;
   sd->indicator_disabled = disabled;
   if (!sd->items) return;
   if (disabled)
     elm_layout_signal_emit(obj, "elm,indicator,state,inactive", "elm");
   else
     elm_layout_signal_emit(obj, "elm,indicator,state,active", "elm");
}

EOLIAN static Eina_Bool
_elm_index_indicator_disabled_get(const Eo *obj EINA_UNUSED, Elm_Index_Data *sd)
{
   return sd->indicator_disabled;
}

EOLIAN static const char *
_elm_index_item_letter_get(const Eo *item EINA_UNUSED, Elm_Index_Item_Data *it)
{
   return it->letter;
}

EOLIAN static void
_elm_index_efl_ui_direction_direction_set(Eo *obj, Elm_Index_Data *sd, Efl_Ui_Dir dir)
{
   // Adjust direction to be either horizontal or vertical.
   if (efl_ui_dir_is_horizontal(dir, EINA_FALSE))
     dir = EFL_UI_DIR_HORIZONTAL;
   else
     dir = EFL_UI_DIR_VERTICAL;

   sd->dir = dir;

   efl_ui_widget_theme_apply(obj);
}

EOLIAN static Efl_Ui_Dir
_elm_index_efl_ui_direction_direction_get(const Eo *obj EINA_UNUSED, Elm_Index_Data *sd)
{
   return sd->dir;
}

EOLIAN static void
_elm_index_delay_change_time_set(Eo *obj EINA_UNUSED, Elm_Index_Data *sd, double dtime)
{
   sd->delay_change_time = dtime;
}

EOLIAN static double
_elm_index_delay_change_time_get(const Eo *obj EINA_UNUSED, Elm_Index_Data *sd)
{
   return sd->delay_change_time;
}

EOLIAN static void
_elm_index_omit_enabled_set(Eo *obj, Elm_Index_Data *sd, Eina_Bool enabled)
{
   enabled = !!enabled;
   if (sd->omit_enabled == enabled) return;
   sd->omit_enabled = enabled;

   _index_box_clear(obj, 0);
   _index_box_auto_fill(obj, 0);
   if (sd->level == 1)
     {
        _index_box_clear(obj, 1);
        _index_box_auto_fill(obj, 1);
     }
}

EOLIAN static Eina_Bool
_elm_index_omit_enabled_get(const Eo *obj EINA_UNUSED, Elm_Index_Data *sd)
{
   return sd->omit_enabled;
}

EOLIAN static void
_elm_index_item_priority_set(Eo *eo_it EINA_UNUSED, Elm_Index_Item_Data *it, int priority)
{
   if (priority < -1)
     {
        WRN("priority value should be greater than or equal to -1.");
        return;
     }

   it->priority = priority;
}

EOLIAN static void
_elm_index_standard_priority_set(Eo *obj, Elm_Index_Data *sd, int priority)
{
   if (priority < -1)
     {
        WRN("priority value should be greater than or equal to -1.");
        return;
     }
   if (priority != sd->show_group)
     {
        sd->next_group = priority;
        if (priority > sd->show_group)
          _priority_up_cb((void *)obj);
        else
          _priority_down_cb((void *)obj);
     }
}

EOLIAN static int
_elm_index_standard_priority_get(const Eo *obj EINA_UNUSED, Elm_Index_Data *sd)
{
   return sd->show_group;
}

static void
_elm_index_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static Eina_Bool
_item_action_activate(Eo *obj, const char *params EINA_UNUSED EINA_UNUSED)
{
   elm_index_item_selected_set(obj, EINA_TRUE);
   return EINA_TRUE;
}

EOLIAN static Eina_List*
_elm_index_efl_access_object_access_children_get(const Eo *obj, Elm_Index_Data *data)
{
   Eina_List *ret;
   ret = efl_access_object_access_children_get(efl_super(obj, ELM_INDEX_CLASS));
   return eina_list_merge(eina_list_clone(data->items), ret);
}

EOLIAN static const char*
_elm_index_item_efl_access_object_i18n_name_get(const Eo *eo_it, Elm_Index_Item_Data *data)
{
   const char *name;
   name = efl_access_object_i18n_name_get(efl_super(eo_it, ELM_INDEX_ITEM_CLASS));
   if (name) return name;

   return _elm_widget_item_accessible_plain_name_get(eo_it, data->letter);
}

EOLIAN static const Efl_Access_Action_Data*
_elm_index_item_efl_access_widget_action_elm_actions_get(const Eo *eo_it EINA_UNUSED, Elm_Index_Item_Data *data EINA_UNUSED)
{
   static Efl_Access_Action_Data atspi_actions[] = {
          { "activate", "activate", NULL, _item_action_activate},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

/* Internal EO APIs and hidden overrides */

#define ELM_INDEX_EXTRA_OPS \
   ELM_LAYOUT_SIZING_EVAL_OPS(elm_index), \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(elm_index)

#include "elm_index_item_eo.c"
#include "elm_index_eo.c"
