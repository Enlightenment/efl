#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_widget_toolbar.h"
#include "els_box.h"

EAPI Eo_Op ELM_OBJ_TOOLBAR_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_TOOLBAR_CLASS

#define MY_CLASS_NAME "Elm_Toolbar"
#define MY_CLASS_NAME_LEGACY "elm_toolbar"

#define ELM_TOOLBAR_ITEM_FROM_INLIST(item) \
  ((item) ? EINA_INLIST_CONTAINER_GET(item, Elm_Toolbar_Item) : NULL)

static const char SIG_SCROLL[] = "scroll";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_CLICKED[] = "clicked";
static const char SIG_LONGPRESSED[] = "longpressed";
static const char SIG_CLICKED_DOUBLE[] = "clicked,double";
static const char SIG_ITEM_FOCUSED[] = "item,focused";
static const char SIG_ITEM_UNFOCUSED[] = "item,unfocused";
static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_SCROLL, ""},
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {SIG_CLICKED, ""},
   {SIG_LONGPRESSED, ""},
   {SIG_CLICKED_DOUBLE, ""},
   {SIG_ITEM_FOCUSED, ""},
   {SIG_ITEM_UNFOCUSED, ""},
   {SIG_WIDGET_LANG_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_ACCESS_CHANGED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_FOCUSED, ""}, /**< handled by elm_widget */
   {SIG_WIDGET_UNFOCUSED, ""}, /**< handled by elm_widget */
   {NULL, NULL}
};

static void _item_select(Elm_Toolbar_Item *it);

static int
_toolbar_item_prio_compare_cb(const void *i1,
                              const void *i2)
{
   const Elm_Toolbar_Item *eti1 = i1;
   const Elm_Toolbar_Item *eti2 = i2;

   if (!eti2) return 1;
   if (!eti1) return -1;

   if (eti2->prio.priority == eti1->prio.priority)
     return -1;

   return eti2->prio.priority - eti1->prio.priority;
}

static void
_items_visibility_fix(Elm_Toolbar_Smart_Data *sd,
                      Evas_Coord *iw,
                      Evas_Coord vw,
                      Eina_Bool *more)
{
   Elm_Toolbar_Item *it, *prev;
   Evas_Coord ciw = 0, cih = 0;
   Eina_List *sorted = NULL;
   int count = 0, i = 0;

   *more = EINA_FALSE;

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->separator)
          {
             prev = ELM_TOOLBAR_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
             if (prev) it->prio.priority = prev->prio.priority;
          }
     }

   EINA_INLIST_FOREACH(sd->items, it)
     {
        sorted = eina_list_sorted_insert
            (sorted, _toolbar_item_prio_compare_cb, it);
     }

   if (sd->more_item)
     {
        evas_object_geometry_get(sd->VIEW(more_item), NULL, NULL, &ciw, &cih);
        if (sd->vertical) *iw += cih;
        else *iw += ciw;
     }

   EINA_LIST_FREE(sorted, it)
     {
        if (it->prio.priority > sd->standard_priority)
          {
             evas_object_geometry_get(VIEW(it), NULL, NULL, &ciw, &cih);
             if (sd->vertical) *iw += cih;
             else *iw += ciw;
             it->prio.visible = (*iw <= vw);
             it->in_box = sd->bx;
             if (!it->separator) count++;
          }
        else
          {
             it->prio.visible = EINA_FALSE;
             if (!it->separator) i++;
             if (i <= (count + 1))
               it->in_box = sd->bx_more;
             else
               it->in_box = sd->bx_more2;
             *more = EINA_TRUE;
          }
     }
}

static void
_item_menu_destroy(Elm_Toolbar_Item *item)
{
   ELM_SAFE_FREE(item->o_menu, evas_object_del);
   item->menu = EINA_FALSE;
}

static void
_item_unselect(Elm_Toolbar_Item *item)
{
   if ((!item) || (!item->selected)) return;

   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   item->selected = EINA_FALSE;
   sd->selected_item = NULL;
   edje_object_signal_emit(VIEW(item), "elm,state,unselected", "elm");
   if (item->icon)
     elm_widget_signal_emit(item->icon, "elm,state,unselected", "elm");
}

static void
_menu_hide(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Elm_Toolbar_Item *selected;
   Elm_Toolbar_Item *it = data;

   selected = (Elm_Toolbar_Item *)elm_toolbar_selected_item_get(WIDGET(it));
   _item_unselect(selected);
}

static void
_menu_del(void *data,
          Evas *e EINA_UNUSED,
          Evas_Object *obj,
          void *event_info EINA_UNUSED)
{
   // avoid hide being emitted during object deletion
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_HIDE, _menu_hide, data);
}

static void
_item_menu_create(Elm_Toolbar_Smart_Data *sd,
                  Elm_Toolbar_Item *item)
{
   item->o_menu = elm_menu_add(elm_widget_parent_get(WIDGET(item)));
   item->menu = EINA_TRUE;

   if (sd->menu_parent)
     elm_menu_parent_set(item->o_menu, sd->menu_parent);

   evas_object_event_callback_add
     (item->o_menu, EVAS_CALLBACK_HIDE, _menu_hide, item);
   evas_object_event_callback_add
     (item->o_menu, EVAS_CALLBACK_DEL, _menu_del, item);
}

static void
_elm_toolbar_item_menu_cb(void *data,
                          Evas_Object *obj EINA_UNUSED,
                          void *event_info EINA_UNUSED)
{
   Elm_Toolbar_Item *it = data;

   if (it->func) it->func((void *)(it->base.data), WIDGET(it), it);
}

static void
_item_show(Elm_Toolbar_Item *it)
{
   Evas_Coord x, y, w, h, bx, by;

   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   evas_object_geometry_get(sd->bx, &bx, &by, NULL, NULL);
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
   eo_do(WIDGET(it), elm_interface_scrollable_content_region_show
         (x - bx, y - by, w, h));
}

static void
_item_mirrored_set(Evas_Object *obj EINA_UNUSED,
                   Elm_Toolbar_Item *it,
                   Eina_Bool mirrored)
{
   edje_object_mirrored_set(VIEW(it), mirrored);
   if (it->o_menu) elm_widget_mirrored_set(it->o_menu, mirrored);
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool mirrored)
{
   Elm_Toolbar_Item *it;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->items, it)
     _item_mirrored_set(obj, it, mirrored);
   if (sd->more_item)
     _item_mirrored_set(obj, sd->more_item, mirrored);
}

static void
_items_size_fit(Evas_Object *obj, Evas_Coord *bl, Evas_Coord view)
{
   Elm_Toolbar_Item *it, *prev;
   Eina_Bool full = EINA_FALSE, more = EINA_FALSE;
   Evas_Coord min, mw, mh;
   int sumf = 0, sumb = 0, prev_min = 0;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->items, it)
     {
        mw = mh = -1;
        if (it->in_box && it->in_box == sd->bx)
          {
             if (!it->separator && !it->object)
               elm_coords_finger_size_adjust(1, &mw, 1, &mh);
             edje_object_size_min_restricted_calc(VIEW(it), &mw, &mh, mw, mh);
             if (!it->separator && !it->object)
               elm_coords_finger_size_adjust(1, &mw, 1, &mh);
          }
        else if (!more)
          {
             more = EINA_TRUE;
             elm_coords_finger_size_adjust(1, &mw, 1, &mh);
             edje_object_size_min_restricted_calc(sd->VIEW(more_item), &mw, &mh, mw, mh);
             elm_coords_finger_size_adjust(1, &mw, 1, &mh);
          }

        if (mw != -1 || mh != -1)
          {
             if (sd->vertical) min = mh;
             else min = mw;

             if ((!full) && ((sumf + min) > view))
               {
                  prev = ELM_TOOLBAR_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->prev);
                  if (prev && prev->separator)
                    {
                       sumf -= prev_min;
                       sumb += prev_min;
                    }
                  full = EINA_TRUE;
               }

             if (!full) sumf += min;
             else sumb += min;
             prev_min = min;
          }
     }
   if (sumf != 0) *bl = (Evas_Coord)(((sumf + sumb) * view) / sumf);
}

static Eina_Bool
_elm_toolbar_item_coordinates_calc(Elm_Toolbar_Item *item,
                                   Elm_Toolbar_Item_Scrollto_Type type,
                                   Evas_Coord *x,
                                   Evas_Coord *y,
                                   Evas_Coord *w,
                                   Evas_Coord *h)
{
   Evas_Coord ix, iy, iw, ih, bx, by, vw, vh;

   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   eo_do(WIDGET(item),
         elm_interface_scrollable_content_viewport_size_get(&vw, &vh));
   evas_object_geometry_get(sd->bx, &bx, &by, NULL, NULL);
   evas_object_geometry_get(VIEW(item), &ix, &iy, &iw, &ih);

   switch (type)
     {
      case ELM_TOOLBAR_ITEM_SCROLLTO_IN:
         *x = ix - bx;
         *y = iy - by;
         *w = iw;
         *h = ih;
         break;

      case ELM_TOOLBAR_ITEM_SCROLLTO_FIRST:
         *x = ix - bx;
         *y = iy - by;
         *w = vw;
         *h = vh;
         break;

      case ELM_TOOLBAR_ITEM_SCROLLTO_MIDDLE:
         *x = ix - bx + (iw / 2) - (vw / 2);
         *y = iy - by + (ih / 2) - (vh / 2);
         *w = vw;
         *h = vh;
         break;

      case ELM_TOOLBAR_ITEM_SCROLLTO_LAST:
         *x = ix - bx + iw - vw;
         *y = iy - by + ih - vh;
         *w = vw;
         *h = vh;
         break;

      default:
         return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_resize_job(void *data)
{
   Evas_Object *obj = (Evas_Object *)data;
   Evas_Coord mw, mh, vw = 0, vh = 0, w = 0, h = 0;
   Elm_Toolbar_Item *it;
   Eina_List *list;
   Eina_Bool more;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   sd->resize_job = NULL;
   eo_do(obj, elm_interface_scrollable_content_viewport_size_get(&vw, &vh));
   evas_object_size_hint_min_get(sd->bx, &mw, &mh);
   evas_object_geometry_get(sd->bx, NULL, NULL, &w, &h);

   if (sd->shrink_mode == ELM_TOOLBAR_SHRINK_MENU)
     {
        Evas_Coord iw = 0, ih = 0, more_w = 0, more_h = 0;

        if (sd->vertical)
          {
             h = vh;
             _items_visibility_fix(sd, &ih, vh, &more);
          }
        else
          {
             w = vw;
             _items_visibility_fix(sd, &iw, vw, &more);
          }
        evas_object_geometry_get
          (sd->VIEW(more_item), NULL, NULL, &more_w, &more_h);

        if (sd->vertical)
          {
             if ((ih - more_h) <= vh) ih -= more_h;
          }
        else
          {
             if ((iw - more_w) <= vw) iw -= more_w;
          }

        /* All items are removed from the box object, since removing
         * individual items won't trigger a resize. Items are be
         * readded below. */
        evas_object_box_remove_all(sd->bx, EINA_FALSE);
        if (((sd->vertical) && (ih > vh)) ||
            ((!sd->vertical) && (iw > vw)) || more)
          {
             Evas_Object *menu;

             _item_menu_destroy(sd->more_item);
             _item_menu_create(sd, sd->more_item);
             menu =
               elm_toolbar_item_menu_get((Elm_Object_Item *)sd->more_item);
             EINA_INLIST_FOREACH(sd->items, it)
               {
                  if (!it->prio.visible)
                    {
                       if (it->separator)
                         elm_menu_item_separator_add(menu, NULL);
                       else
                         {
                            Elm_Object_Item *menu_it;

                            menu_it = elm_menu_item_add
                                (menu, NULL, it->icon_str, it->label,
                                _elm_toolbar_item_menu_cb, it);
                            elm_object_item_disabled_set
                              (menu_it, elm_widget_item_disabled_get(it));
                            if (it->o_menu)
                              elm_menu_clone(it->o_menu, menu, menu_it);
                         }
                       evas_object_hide(VIEW(it));
                    }
                  else
                    {
                       evas_object_box_append(sd->bx, VIEW(it));
                       evas_object_show(VIEW(it));
                    }
               }
             evas_object_box_append(sd->bx, sd->VIEW(more_item));
             evas_object_show(sd->VIEW(more_item));
          }
        else
          {
             /* All items are visible, show them all (except for the
              * "More" button, of course). */
             EINA_INLIST_FOREACH(sd->items, it)
               {
                  evas_object_show(VIEW(it));
                  evas_object_box_append(sd->bx, VIEW(it));
               }
             evas_object_hide(sd->VIEW(more_item));
          }
     }
   else if (sd->shrink_mode == ELM_TOOLBAR_SHRINK_HIDE)
     {
        Evas_Coord iw = 0, ih = 0;

        if (sd->vertical)
          {
             h = vh;
             _items_visibility_fix(sd, &ih, vh, &more);
          }
        else
          {
             w = vw;
             _items_visibility_fix(sd, &iw, vw, &more);
          }
        evas_object_box_remove_all(sd->bx, EINA_FALSE);
        if (((sd->vertical) && (ih > vh)) ||
            ((!sd->vertical) && (iw > vw)) || more)
          {
             EINA_INLIST_FOREACH(sd->items, it)
               {
                  if (!it->prio.visible)
                    evas_object_hide(VIEW(it));
                  else
                    {
                       evas_object_box_append(sd->bx, VIEW(it));
                       evas_object_show(VIEW(it));
                    }
               }
          }
        else
          {
             /* All items are visible, show them all */
             EINA_INLIST_FOREACH(sd->items, it)
               {
                  evas_object_show(VIEW(it));
                  evas_object_box_append(sd->bx, VIEW(it));
               }
          }
     }
   else if (sd->shrink_mode == ELM_TOOLBAR_SHRINK_EXPAND)
     {
        Evas_Coord iw = 0, ih = 0;

        if (sd->vertical)
          h = (vh >= mh) ? vh : mh;
        else
          w = (vw >= mw) ? vw : mw;

        if (sd->vertical)
          _items_visibility_fix(sd, &ih, vh, &more);
        else
          _items_visibility_fix(sd, &iw, vw, &more);

        evas_object_box_remove_all(sd->bx, EINA_FALSE);
        evas_object_box_remove_all(sd->bx_more, EINA_FALSE);
        evas_object_box_remove_all(sd->bx_more2, EINA_FALSE);

        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (it->in_box)
               {
                  evas_object_box_append(it->in_box, VIEW(it));
                  evas_object_show(VIEW(it));
               }
          }
        if (more)
          {
             evas_object_box_append(sd->bx, sd->VIEW(more_item));
             evas_object_show(sd->VIEW(more_item));
          }
        else
          evas_object_hide(sd->VIEW(more_item));

        if (sd->vertical)
          {
             if (h > vh) _items_size_fit(obj, &h, vh);
             if (sd->item_count - sd->separator_count > 0)
               eo_do(obj, elm_interface_scrollable_paging_set
                     (0.0, 0.0, 0, (h / (sd->item_count - sd->separator_count))));
          }
        else
          {
             if (w > vw) _items_size_fit(obj, &w, vw);
             if (sd->item_count - sd->separator_count > 0)
               eo_do(obj, elm_interface_scrollable_paging_set
                     (0.0, 0.0, (w / (sd->item_count - sd->separator_count)), 0));
          }
     }
   else
     {
        if (sd->vertical)
          {
             if ((vh >= mh) && (h != vh)) h = vh;
          }
        else
          {
             if ((vw >= mw) && (w != vw)) w = vw;
          }
        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (it->selected)
               {
                  _item_show(it);
                  break;
               }
          }
     }

   if (sd->transverse_expanded)
     {
        if (sd->vertical)
          w = vw;
        else
          h = vh;
     }

   evas_object_resize(sd->bx, w, h);

// Remove the first or last separator since it is not necessary
   list = evas_object_box_children_get(sd->bx_more);
   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->separator &&
            ((VIEW(it) == eina_list_data_get(list)) ||
             (VIEW(it) == eina_list_nth(list, eina_list_count(list) - 1))))
          {
             evas_object_box_remove(sd->bx_more, VIEW(it));
             evas_object_move(VIEW(it), -9999, -9999);
             evas_object_hide(VIEW(it));
          }
     }
   list = evas_object_box_children_get(sd->bx_more2);
   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->separator &&
            ((VIEW(it) == eina_list_data_get(list)) ||
             (VIEW(it) == eina_list_nth(list, eina_list_count(list) - 1))))
          {
             evas_object_box_remove(sd->bx_more2, VIEW(it));
             evas_object_move(VIEW(it), -9999, -9999);
             evas_object_hide(VIEW(it));
          }
     }

   _mirrored_set(obj, elm_widget_mirrored_get(obj));
}

static void
_elm_toolbar_item_focused(Elm_Toolbar_Item *it)
{
   Evas_Object *obj = WIDGET(it);
   ELM_TOOLBAR_DATA_GET(obj, sd);
   const char *focus_raise;

   if ((!sd) || (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) ||
       (it == sd->focused_item))
     return;

   sd->focused_item = it;
   elm_toolbar_item_show((Elm_Object_Item *)it, ELM_TOOLBAR_ITEM_SCROLLTO_IN);
   if (elm_widget_focus_highlight_enabled_get(obj))
     {
        edje_object_signal_emit
           (VIEW(it), "elm,state,focused", "elm");
     }
   edje_object_signal_emit
      (VIEW(it), "elm,highlight,on", "elm");
   focus_raise = edje_object_data_get(VIEW(it), "focusraise");
   if ((focus_raise) && (!strcmp(focus_raise, "on")))
     evas_object_raise(VIEW(it));
   evas_object_smart_callback_call
      (obj, SIG_ITEM_FOCUSED, it);
}

static void
_elm_toolbar_item_unfocused(Elm_Toolbar_Item *it)
{
   Evas_Object *obj = WIDGET(it);
   ELM_TOOLBAR_DATA_GET(obj, sd);

   if ((!sd) || !sd->focused_item ||
       (it != sd->focused_item))
     return;
   if (sd->select_mode == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
     return;
   sd->prev_focused_item = it;
   if (elm_widget_focus_highlight_enabled_get(obj))
     {
        edje_object_signal_emit
           (VIEW(sd->focused_item), "elm,state,unfocused", "elm");
     }
   edje_object_signal_emit
      (VIEW(it), "elm,highlight,off", "elm");
   sd->focused_item = NULL;
   evas_object_smart_callback_call
      (obj, SIG_ITEM_UNFOCUSED, it);
}

/*
 * This function searches the nearest visible item based on the given item.
 * If the given item is in the toolbar viewport, this returns the given item.
 * Or this searches other items and checks the nearest fully visible item
 * according to the given item's position.
 */
static Elm_Object_Item *
_elm_toolbar_nearest_visible_item_get(Evas_Object *obj, Elm_Object_Item *it)
{
   Evas_Coord vx = 0, vy = 0, vw = 0, vh = 0; // toolbar viewport geometry
   Evas_Coord ix = 0, iy = 0, iw = 0, ih = 0; // given item geometry
   Evas_Coord cx = 0, cy = 0, cw = 0, ch = 0; // candidate item geometry
   Eina_List *item_list = NULL;
   Elm_Object_Item *item = NULL;
   ELM_TOOLBAR_DATA_GET(obj, sd);
   Eina_Bool search_next = EINA_FALSE;
   Evas_Object *it_obj;

   if (!it) return NULL;

   evas_object_geometry_get(obj, &vx, &vy, &vw, &vh);
   evas_object_geometry_get(VIEW(it), &ix, &iy, &iw, &ih);

   item_list = evas_object_box_children_get(sd->bx);

   if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, ix, iy, iw, ih))
     {
        if (!elm_object_item_disabled_get(it))
          return it;
        else
          search_next = EINA_TRUE;
     }

   if ((sd->vertical && (iy < vy)) ||
       (!sd->vertical && (iw < vw)) ||
       search_next)
     {
        while ((item_list = eina_list_next(item_list)))
          {
             it_obj = eina_list_data_get(item_list);
             if (it_obj)
               item = evas_object_data_get(it_obj, "item");
             if (!item)
               break;
             evas_object_geometry_get(VIEW(item), &cx, &cy, &cw, &ch);
             if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, cx, cy, cw, ch) &&
                 !elm_object_item_disabled_get(item))
               return item;
          }
     }
   else
     {
        while ((item_list = eina_list_prev(item_list)))
          {
             it_obj = eina_list_data_get(item_list);
             if (it_obj)
               item = evas_object_data_get(it_obj, "item");
             if (!item)
               break;
             evas_object_geometry_get(VIEW(item), &cx, &cy, &cw, &ch);
             if (ELM_RECTS_INCLUDE(vx, vy, vw, vh, cx, cy, cw, ch) &&
                 !elm_object_item_disabled_get(item))
               return item;
          }
     }
   return NULL;
}

static void
_elm_toolbar_smart_on_focus(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   ELM_TOOLBAR_DATA_GET(obj, sd);
   Eina_Bool int_ret = EINA_FALSE;
   Elm_Object_Item *it = NULL;

   eo_do_super(obj, MY_CLASS, elm_obj_widget_on_focus(&int_ret));
   if (!int_ret) return;
   if (!sd->items) return;

   if (elm_widget_focus_get(obj) && !sd->mouse_down)
     {
        if (sd->last_focused_item)
          it = (Elm_Object_Item *)sd->last_focused_item;
        else
          it = (Elm_Object_Item *)ELM_TOOLBAR_ITEM_FROM_INLIST(sd->items);
        if (it)
          {
             it = _elm_toolbar_nearest_visible_item_get(obj, it);
             _elm_toolbar_item_focused((Elm_Toolbar_Item *)it);
          }
        evas_object_focus_set(wd->resize_obj, EINA_TRUE);
     }
   else
     {
        sd->prev_focused_item = sd->focused_item;
        sd->last_focused_item = sd->focused_item;
        if (sd->focused_item)
          _elm_toolbar_item_unfocused(sd->focused_item);
        evas_object_focus_set(wd->resize_obj, EINA_FALSE);
     }
   if (ret) *ret = EINA_TRUE;
}

static Elm_Toolbar_Item *
_focus_next_item_get(Evas_Object *obj, Eina_Bool reverse)
{
   ELM_TOOLBAR_DATA_GET(obj, sd);
   Eina_List *list = NULL;
   Elm_Toolbar_Item *it = NULL;
   Evas_Object *it_obj = NULL;

   list = evas_object_box_children_get(sd->bx);
   if (reverse)
     list = eina_list_reverse(list);

   if (sd->focused_item)
     {
        list = eina_list_data_find_list(list, VIEW(sd->focused_item));
        if (list) list = eina_list_next(list);
     }
   it_obj = eina_list_data_get(list);
   if (it_obj) it = evas_object_data_get(it_obj, "item");
   else it = NULL;

   while (it &&
          (it->separator ||
           elm_object_item_disabled_get((Elm_Object_Item *)it)))
     {
        if (list) list = eina_list_next(list);
        if (!list)
          {
             it = NULL;
             break;
          }
        it_obj = eina_list_data_get(list);
        if (it_obj) it = evas_object_data_get(it_obj, "item");
        else it = NULL;
     }

   return it;
}

static void
_item_focus_set_hook(Elm_Object_Item *it, Eina_Bool focused)
{
   ELM_TOOLBAR_ITEM_CHECK(it);
   Evas_Object *obj = WIDGET(it);
   ELM_TOOLBAR_DATA_GET(obj, sd);

   if (focused)
     {
        if (!elm_object_focus_get(obj))
          elm_object_focus_set(obj, EINA_TRUE);
        if (it != (Elm_Object_Item *)sd->focused_item)
          {
             if (sd->focused_item)
               _elm_toolbar_item_unfocused(sd->focused_item);
             _elm_toolbar_item_focused((Elm_Toolbar_Item *)it);
          }
     }
   else
     {
        if (it)
          _elm_toolbar_item_unfocused((Elm_Toolbar_Item *)it);
     }
   _elm_widget_focus_highlight_start(obj);
}

static Eina_Bool
_item_focus_get_hook(Elm_Object_Item *it)
{
   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Evas_Object *obj = WIDGET(it);
   ELM_TOOLBAR_CHECK(obj) EINA_FALSE;
   ELM_TOOLBAR_DATA_GET(obj, sd);

   if (it == (Elm_Object_Item *)sd->focused_item)
     return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_Bool
_item_focused_next( Evas_Object *obj,
                   Eina_Bool reverse,
                   Elm_Focus_Direction dir)
{
   ELM_TOOLBAR_DATA_GET(obj, sd);
   Elm_Toolbar_Item *next_focused_item;

   next_focused_item = _focus_next_item_get(obj, reverse);
   if (!next_focused_item)
     return EINA_FALSE;

   if ((!sd->vertical && (dir == ELM_FOCUS_LEFT || dir == ELM_FOCUS_RIGHT))
         || (sd->vertical && (dir == ELM_FOCUS_UP || dir == ELM_FOCUS_DOWN)))
   {
      elm_object_item_focus_set((Elm_Object_Item *)next_focused_item, EINA_TRUE);
      return EINA_TRUE;
   }
   _elm_widget_focus_highlight_start(obj);
   return EINA_FALSE;
}

static void
_elm_toolbar_smart_event(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   Evas_Event_Key_Down *ev = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   if (ret) *ret = EINA_FALSE;
   (void) src;
   (void) type;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;
   if (!sd->items) return;

   if ((!strcmp(ev->key, "Return")) ||
       (!strcmp(ev->key, "KP_Enter")) ||
       (!strcmp(ev->key, "space")))
     {
         if (sd->focused_item)
           _item_select(sd->focused_item);

        goto success;
     }
   else if ((!strcmp(ev->key, "Left")) ||
            ((!strcmp(ev->key, "KP_Left")) && !ev->string))
     {
        if (_item_focused_next(obj, EINA_TRUE, ELM_FOCUS_LEFT))
          {
             goto success;
          }
        else
          {
             if (ret) *ret = EINA_FALSE;
             return;
          }
     }
   else if ((!strcmp(ev->key, "Right")) ||
            ((!strcmp(ev->key, "KP_Right")) && !ev->string))
     {
        if (_item_focused_next(obj, EINA_FALSE, ELM_FOCUS_RIGHT))
          {
             goto success;
          }
        else
          {
             if (ret) *ret = EINA_FALSE;
             return;
          }
     }
   else if ((!strcmp(ev->key, "Up")) ||
            ((!strcmp(ev->key, "KP_Up")) && !ev->string))
     {
        if (_item_focused_next(obj, EINA_TRUE, ELM_FOCUS_UP))
          {
             goto success;
          }
        else
          {
             if (ret) *ret = EINA_FALSE;
             return;
          }
     }
   else if ((!strcmp(ev->key, "Down")) ||
            ((!strcmp(ev->key, "KP_Down")) && !ev->string))
     {
        if (_item_focused_next(obj, EINA_FALSE, ELM_FOCUS_DOWN))
          {
             goto success;
          }
        else
          {
             if (ret) *ret = EINA_FALSE;
             return;
          }
     }

success:
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_resize_cb(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   Evas_Coord x, y, h;

   ELM_TOOLBAR_DATA_GET(data, sd);

   evas_object_geometry_get(data, &x, &y, NULL, &h);
   evas_object_move(sd->more, x, y + h);

   ecore_job_del(sd->resize_job);
   sd->resize_job = ecore_job_add(_resize_job, data);
}

static void
_item_disable_hook(Elm_Object_Item *it)
{
   Elm_Toolbar_Item *toolbar_it = (Elm_Toolbar_Item *)it;

   const char* emission;

   if (elm_widget_item_disabled_get(toolbar_it))
     emission = "elm,state,disabled";
   else
     emission = "elm,state,enabled";

   edje_object_signal_emit(VIEW(toolbar_it), emission, "elm");
   if (toolbar_it->icon)
     edje_object_signal_emit(toolbar_it->icon, emission, "elm");

   _resize_cb(WIDGET(toolbar_it), NULL, NULL, NULL);
}

static Eina_Bool
_item_icon_set(Evas_Object *icon_obj,
               const char *type,
               const char *icon)
{
   char icon_str[512];

   if ((!type) || (!*type)) goto end;
   if ((!icon) || (!*icon)) return EINA_FALSE;
   if ((snprintf(icon_str, sizeof(icon_str), "%s%s", type, icon) > 0)
       && (elm_icon_standard_set(icon_obj, icon_str)))
     return EINA_TRUE;
end:
   if (elm_icon_standard_set(icon_obj, icon))
     return EINA_TRUE;

   WRN("couldn't find icon definition for '%s'", icon);
   return EINA_FALSE;
}

static int
_elm_toolbar_icon_size_get(Evas_Object *obj)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, 0);
   const char *icon_size = edje_object_data_get
       (wd->resize_obj, "icon_size");

   if (icon_size) return atoi(icon_size);

   return _elm_config->icon_size;
}

static void
_menu_move_resize_cb(void *data,
                     Evas *e EINA_UNUSED,
                     Evas_Object *obj EINA_UNUSED,
                     void *event_info EINA_UNUSED)
{
   Elm_Toolbar_Item *it = data;
   Evas_Coord x, y, h;

   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   if (!sd->menu_parent) return;
   evas_object_geometry_get(VIEW(it), &x, &y, NULL, &h);
   elm_menu_move(it->o_menu, x, y + h);
}

static void
_item_select(Elm_Toolbar_Item *it)
{
   Elm_Toolbar_Item *it2;
   Evas_Object *obj2;
   Eina_Bool sel;

   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   if (elm_widget_item_disabled_get(it) || (it->separator) || (it->object))
     return;
   sel = it->selected;

   if (sd->select_mode != ELM_OBJECT_SELECT_MODE_NONE)
     {
        if (sel)
          {
             if (sd->shrink_mode == ELM_TOOLBAR_SHRINK_EXPAND)
               {
                  if (sd->more_item == it)
                    {
                       elm_layout_signal_emit
                         (sd->more, "elm,state,close", "elm");
                       _item_unselect(it);
                    }
               }
             if (sd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS)
               _item_unselect(it);
          }
        else
          {
             it2 = (Elm_Toolbar_Item *)
               elm_toolbar_selected_item_get(WIDGET(it));
             _item_unselect(it2);

             it->selected = EINA_TRUE;
             sd->selected_item = it;
             if (sd->shrink_mode == ELM_TOOLBAR_SHRINK_EXPAND)
               {
                  if (sd->more_item == it)
                    {
                       if (!evas_object_box_children_get(sd->bx_more2))
                         elm_layout_signal_emit
                           (sd->more, "elm,state,open", "elm");
                       else
                         elm_layout_signal_emit
                           (sd->more, "elm,state,open2", "elm");
                    }
                  else
                    {
                       if (it->in_box != sd->bx)
                         {
                            edje_object_signal_emit
                              (sd->VIEW(more_item), "elm,state,selected",
                              "elm");
                            elm_widget_signal_emit
                              (sd->more_item->icon, "elm,state,selected",
                              "elm");
                         }
                       else
                         {
                            edje_object_signal_emit
                              (sd->VIEW(more_item), "elm,state,unselected",
                              "elm");
                            elm_widget_signal_emit
                              (sd->more_item->icon, "elm,state,unselected",
                              "elm");
                         }
                       elm_layout_signal_emit
                         (sd->more, "elm,state,close", "elm");
                    }
               }
             edje_object_signal_emit(VIEW(it), "elm,state,selected", "elm");
             if (it->icon)
               elm_widget_signal_emit(it->icon, "elm,state,selected", "elm");
             _item_show(it);
          }
     }

   obj2 = WIDGET(it);
   if (it->menu && (!sel))
     {
        evas_object_show(it->o_menu);
        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_RESIZE, _menu_move_resize_cb, it);
        evas_object_event_callback_add
          (VIEW(it), EVAS_CALLBACK_MOVE, _menu_move_resize_cb, it);

        _menu_move_resize_cb(it, NULL, NULL, NULL);
     }

   if ((!sel) || (sd->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS))
     {
        if (it->func) it->func((void *)(it->base.data), WIDGET(it), it);
     }
   evas_object_smart_callback_call(obj2, SIG_CLICKED, it);
}

static void
_item_del(Elm_Toolbar_Item *it)
{
   Elm_Toolbar_Item_State *it_state;
   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   _item_unselect(it);

   EINA_LIST_FREE(it->states, it_state)
     {
        if (it->icon == it_state->icon)
          it->icon = NULL;
        eina_stringshare_del(it_state->label);
        eina_stringshare_del(it_state->icon_str);
        if (it_state->icon) evas_object_del(it_state->icon);
        free(it_state);
     }

   eina_stringshare_del(it->label);
   if (it->label)
     edje_object_signal_emit(VIEW(it), "elm,state,text,hidden", "elm");
   eina_stringshare_del(it->icon_str);

   if (it->icon)
     {
        edje_object_signal_emit(VIEW(it), "elm,state,icon,hidden", "elm");
        evas_object_del(it->icon);
     }

   if (sd->focused_item == it)
     sd->focused_item = NULL;
   if (sd->last_focused_item == it)
     sd->last_focused_item = NULL;
   if (sd->prev_focused_item == it)
     sd->prev_focused_item = NULL;

   evas_object_del(it->object);
   //TODO: See if checking for sd->menu_parent is necessary before
   //deleting menu
   evas_object_del(it->o_menu);
}

static void
_item_theme_hook(Evas_Object *obj,
                 Elm_Toolbar_Item *it,
                 double scale,
                 int icon_size)
{
   Evas_Coord mw, mh, minw, minh;
   Evas_Object *view = VIEW(it);
   const char *style;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   style = elm_widget_style_get(obj);

   _item_mirrored_set(obj, it, elm_widget_mirrored_get(obj));
   edje_object_scale_set(view, scale);

   if (!it->separator && !it->object)
     {
        elm_widget_theme_object_set(obj, view, "toolbar", "item", style);
        if (it->selected)
          {
             edje_object_signal_emit(view, "elm,state,selected", "elm");
             if (it->icon)
               elm_widget_signal_emit(it->icon, "elm,state,selected", "elm");
          }
        if (elm_widget_item_disabled_get(it))
          {
             edje_object_signal_emit(view, "elm,state,disabled", "elm");
             if (it->icon)
               elm_widget_signal_emit(it->icon, "elm,state,disabled", "elm");
          }
        if (it->icon)
          {
             int ms = 0;

             ms = ((double)icon_size * scale);
             evas_object_size_hint_min_set(it->icon, ms, ms);
             evas_object_size_hint_max_set(it->icon, ms, ms);
             edje_object_part_swallow(view, "elm.swallow.icon", it->icon);
             edje_object_signal_emit
               (view, "elm,state,icon,visible", "elm");
          }
        if (it->label)
          {
             edje_object_part_text_escaped_set(view, "elm.text", it->label);
             edje_object_signal_emit(view, "elm,state,text,visible", "elm");
          }
        if (sd->vertical)
          edje_object_signal_emit(view, "elm,orient,vertical", "elm");
        else
          edje_object_signal_emit(view, "elm,orient,horizontal", "elm");
     }
   else
     {
        if (!it->object)
          {
             elm_widget_theme_object_set
               (obj, view, "toolbar", "separator", style);
             if (sd->vertical)
               {
                  edje_object_signal_emit(view, "elm,orient,vertical", "elm");
                  evas_object_size_hint_weight_set
                    (view, EVAS_HINT_EXPAND, 0.0);
                  evas_object_size_hint_align_set
                    (view, EVAS_HINT_FILL, EVAS_HINT_FILL);
               }
             else
               {
                  edje_object_signal_emit(view, "elm,orient,horizontal", "elm");
                  evas_object_size_hint_weight_set
                    (view, 0.0, EVAS_HINT_EXPAND);
                  evas_object_size_hint_align_set
                    (view, EVAS_HINT_FILL, EVAS_HINT_FILL);
               }
          }
        else
          {
             elm_widget_theme_object_set
               (obj, view, "toolbar", "object", style);
             edje_object_part_swallow(view, "elm.swallow.object", it->object);
             if (sd->vertical)
               edje_object_signal_emit(view, "elm,orient,vertical", "elm");
             else
               edje_object_signal_emit(view, "elm,orient,horizontal", "elm");
          }
     }

   mw = mh = minw = minh = -1;
   if (!it->separator && !it->object)
     elm_coords_finger_size_adjust(1, &mw, 1, &mh);

   // If the min size is changed by edje signal in edc,
   //the below function should be called before the calculation.
   edje_object_message_signal_process(view);
   edje_object_size_min_restricted_calc(view, &mw, &mh, mw, mh);
   if (!it->separator && !it->object)
     elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   evas_object_size_hint_min_set(view, mw, mh);
}

static void
_inform_item_number(Evas_Object *obj)
{
   ELM_TOOLBAR_DATA_GET(obj, sd);
   Elm_Toolbar_Item *it;
   char buf[sizeof("elm,number,item,") + 4];
   static int scount = 0;
   int count = 0;

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (!it->separator) count++;
     }
   if (scount != count)
     {
        scount = count;
        if (snprintf(buf, sizeof(buf), "elm,number,item,%d", count) >= (int)sizeof(buf))
          ERR("Too many items to fit signal buffer (%d)", count);

        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (!it->separator && !it->object)
               edje_object_signal_emit(VIEW(it), buf, "elm");
          }
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw = -1, minh = -1, minw_bx = -1, minh_bx = -1;
   Evas_Coord vw = 0, vh = 0;
   Evas_Coord w, h;

   ELM_TOOLBAR_DATA_GET(obj, sd);
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   evas_object_smart_need_recalculate_set(sd->bx, EINA_TRUE);
   evas_object_smart_calculate(sd->bx);
   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   if (w < minw) w = minw;
   if (h < minh) h = minh;

   evas_object_resize(wd->resize_obj, w, h);

   evas_object_size_hint_min_get(sd->bx, &minw_bx, &minh_bx);
   eo_do(obj, elm_interface_scrollable_content_viewport_size_get(&vw, &vh));

   if (sd->shrink_mode == ELM_TOOLBAR_SHRINK_NONE)
     {
        minw = minw_bx + (w - vw);
        minh = minh_bx + (h - vh);
     }
   else if (sd->shrink_mode == ELM_TOOLBAR_SHRINK_EXPAND)
     {
        if (sd->vertical)
          {
             minw = minw_bx + (w - vw);
             if (minh_bx <= vh) minh_bx = vh;
             else _items_size_fit(obj, &minh_bx, vh);
          }
        else
          {
             minh = minh_bx + (h - vh);
             if (minw_bx <= vw) minw_bx = vw;
             else _items_size_fit(obj, &minw_bx, vw);
          }
     }
   else
     {
        if (sd->vertical)
          {
             minw = minw_bx + (w - vw);
             minh = h - vh;
          }
        else
          {
             minw = w - vw;
             minh = minh_bx + (h - vh);
          }
     }

   if (sd->transverse_expanded)
     {
        if (sd->vertical)
          minw_bx = vw;
        else
          minh_bx = vh;
     }

   evas_object_resize(sd->bx, minw_bx, minh_bx);
   evas_object_resize(sd->more, minw_bx, minh_bx);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, -1, -1);

   _inform_item_number(obj);
}

static void
_elm_toolbar_highlight_in_theme(Evas_Object *obj)
{
   const char *fh;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   fh = edje_object_data_get
       (wd->resize_obj, "focus_highlight");
   if ((fh) && (!strcmp(fh, "on")))
     elm_widget_highlight_in_theme_set(obj, EINA_TRUE);
   else
     elm_widget_highlight_in_theme_set(obj, EINA_FALSE);
}

static void
_elm_toolbar_smart_theme(Eo *obj, void *_pd, va_list *list)
{
   Elm_Toolbar_Item *it;
   double scale = 0;
   Elm_Toolbar_Smart_Data *sd = _pd;
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (sd->delete_me)
     {
        if (ret) *ret = EINA_TRUE;
        return;
     }

   Eina_Bool int_ret;
   eo_do_super(obj, MY_CLASS, elm_obj_widget_theme_apply(&int_ret));
   if (!int_ret) return;

   elm_widget_theme_object_set
     (obj, wd->resize_obj, "toolbar", "base",
     elm_widget_style_get(obj));
   if (sd->vertical)
     edje_object_signal_emit(wd->resize_obj, "elm,orient,vertical", "elm");
   else
     edje_object_signal_emit(wd->resize_obj, "elm,orient,horizontal", "elm");

   if (!elm_layout_theme_set
       (sd->more, "toolbar", "more", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");
   if (sd->vertical)
     edje_object_signal_emit(sd->more, "elm,orient,vertical", "elm");
   else
     edje_object_signal_emit(sd->more, "elm,orient,horizontal", "elm");

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   sd->theme_icon_size = _elm_toolbar_icon_size_get(obj);
   if (sd->priv_icon_size) sd->icon_size = sd->priv_icon_size;
   else sd->icon_size = sd->theme_icon_size;

   scale = (elm_widget_scale_get(obj) * elm_config_scale_get());
   EINA_INLIST_FOREACH(sd->items, it)
     _item_theme_hook(obj, it, scale, sd->icon_size);

   if (sd->more_item)
     _item_theme_hook(obj, sd->more_item, scale, sd->icon_size);

   _elm_toolbar_highlight_in_theme(obj);
   _sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_toolbar_item_label_update(Elm_Toolbar_Item *item)
{
   Evas_Coord mw = -1, mh = -1, minw = -1, minh = -1;

   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   edje_object_part_text_escaped_set(VIEW(item), "elm.text", item->label);
   if (item->label)
     edje_object_signal_emit(VIEW(item), "elm,state,text,visible", "elm");
   else
     edje_object_signal_emit(VIEW(item), "elm,state,text,hidden", "elm");

   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   // If the min size is changed by edje signal in edc,
   //the below function should be called before the calculation.
   edje_object_message_signal_process(VIEW(item));
   edje_object_size_min_restricted_calc(VIEW(item), &mw, &mh, mw, mh);
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   if (sd->shrink_mode != ELM_TOOLBAR_SHRINK_EXPAND)
     {
        if (sd->vertical)
          {
             evas_object_size_hint_weight_set
               (VIEW(item), EVAS_HINT_EXPAND, -1.0);
             evas_object_size_hint_align_set
               (VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
          }
        else
          {
             evas_object_size_hint_weight_set
               (VIEW(item), -1.0, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
               (VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
          }
     }
   else
     {
        evas_object_size_hint_weight_set
          (VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set
          (VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
     }

   evas_object_size_hint_min_get(VIEW(item), &minw, &minh);
   if ((minw < mw) && (minh < mh))
     evas_object_size_hint_min_set(VIEW(item), mw, mh);
   else if ((minw < mw) && (minh > mh))
     evas_object_size_hint_min_set(VIEW(item), mw, minh);
   else if ((minw > mw) && (minh < mh))
     evas_object_size_hint_min_set(VIEW(item), minw, mh);
}

static void
_elm_toolbar_item_label_set_cb(void *data,
                               Evas_Object *obj,
                               const char *emission,
                               const char *source)
{
   Elm_Toolbar_Item *item = data;

   _elm_toolbar_item_label_update(item);
   edje_object_signal_callback_del
     (obj, emission, source, _elm_toolbar_item_label_set_cb);
   edje_object_signal_emit(VIEW(item), "elm,state,label,reset", "elm");
}

static void
_item_label_set(Elm_Toolbar_Item *item,
                const char *label,
                const char *sig)
{
   const char *s;

   if ((label) && (item->label) && (!strcmp(label, item->label))) return;

   eina_stringshare_replace(&item->label, label);
   s = edje_object_data_get(VIEW(item), "transition_animation_on");
   if ((s) && (atoi(s)))
     {
        edje_object_part_text_escaped_set
          (VIEW(item), "elm.text_new", item->label);
        edje_object_signal_emit(VIEW(item), sig, "elm");
        edje_object_signal_callback_add
          (VIEW(item), "elm,state,label_set,done", "elm",
          _elm_toolbar_item_label_set_cb, item);
     }
   else
     _elm_toolbar_item_label_update(item);

   _resize_cb(WIDGET(item), NULL, NULL, NULL);
}

static void
_item_text_set_hook(Elm_Object_Item *it,
                    const char *part,
                    const char *label)
{
   Elm_Toolbar_Item *item;
   char buf[256];
   item = (Elm_Toolbar_Item *)it;

   if ((!part) || (!strcmp(part, "default")) ||
       (!strcmp(part, "elm.text")))
     {
        _item_label_set(((Elm_Toolbar_Item *)it), label, "elm,state,label_set");
     }
   else
     {
        if (label)
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,visible", part);
             edje_object_signal_emit(VIEW(item), buf, "elm");
          }
        else
          {
             snprintf(buf, sizeof(buf), "elm,state,%s,hidden", part);
             edje_object_signal_emit(VIEW(item), buf, "elm");
          }
        edje_object_part_text_escaped_set(VIEW(item), part, label);
     }
}

static const char *
_item_text_get_hook(const Elm_Object_Item *it,
                    const char *part)
{
   char buf[256];

   if (!part || !strcmp(part, "default"))
     snprintf(buf, sizeof(buf), "elm.text");
   else
     snprintf(buf, sizeof(buf), "%s", part);

   return edje_object_part_text_get(VIEW(it), buf);
}

static void
_item_content_set_hook(Elm_Object_Item *it,
                       const char *part,
                       Evas_Object *content)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;
   Evas_Object *obj = WIDGET(item);
   double scale;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   if (part && strcmp(part, "object")) return;
   if (item->object == content) return;

   evas_object_del(item->object);

   item->object = content;
   if (item->object)
     elm_widget_sub_object_add(obj, item->object);

   scale = (elm_widget_scale_get(obj) * elm_config_scale_get());
   _item_theme_hook(obj, item, scale, sd->icon_size);
}

static Evas_Object *
_item_content_get_hook(const Elm_Object_Item *it,
                       const char *part)
{
   if (part && strcmp(part, "object")) return NULL;
   return ((Elm_Toolbar_Item *)it)->object;
}

static Evas_Object *
_item_content_unset_hook(Elm_Object_Item *it,
                         const char *part)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;
   Evas_Object *obj = WIDGET(item);
   Evas_Object *o;
   double scale;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   if (part && strcmp(part, "object")) return NULL;

   edje_object_part_unswallow(VIEW(it), item->object);
   elm_widget_sub_object_del(obj, item->object);
   o = item->object;
   item->object = NULL;
   scale = (elm_widget_scale_get(obj) * elm_config_scale_get());
   _item_theme_hook(obj, item, scale, sd->icon_size);

   return o;
}

static void
_elm_toolbar_smart_translate(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Toolbar_Smart_Data *sd = _pd;
   Elm_Toolbar_Item *it;

   EINA_INLIST_FOREACH(sd->items, it)
     elm_widget_item_translate(it);

   eo_do_super(obj, MY_CLASS, elm_obj_widget_translate(NULL));

   if (ret) *ret = EINA_TRUE;
}

static void
_item_resize(void *data,
             Evas *e EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   _sizing_eval(data);
   _resize_cb(data, NULL, NULL, NULL);
}

static void
_move_cb(void *data,
         Evas *e EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED,
         void *event_info EINA_UNUSED)
{
   Evas_Coord x, y, h;

   ELM_TOOLBAR_DATA_GET(data, sd);
   evas_object_geometry_get(data, &x, &y, NULL, &h);
   evas_object_move(sd->more, x, y + h);
}

static void
_select_filter_cb(Elm_Toolbar_Item *it,
                  Evas_Object *obj EINA_UNUSED,
                  const char *emission,
                  const char *source EINA_UNUSED)
{
   int button;
   char buf[sizeof("elm,action,click,") + 1];

   button = atoi(emission + sizeof("mouse,clicked,") - 1);
   if (button == 1) return;  /* regular left click event */
   snprintf(buf, sizeof(buf), "elm,action,click,%d", button);
   edje_object_signal_emit(VIEW(it), buf, "elm");
}

static void
_select_cb(void *data,
           Evas_Object *obj EINA_UNUSED,
           const char *emission EINA_UNUSED,
           const char *source EINA_UNUSED)
{
   Elm_Toolbar_Item *it = data;

   if ((_elm_config->access_mode == ELM_ACCESS_MODE_OFF) ||
       (_elm_access_2nd_click_timeout(VIEW(it))))
     {
        if (_elm_config->access_mode != ELM_ACCESS_MODE_OFF)
          _elm_access_say(E_("Selected"));
        _item_select(it);
     }
}

static void
_item_move_cb(void *data,
         Evas *e EINA_UNUSED,
         Evas_Object *obj EINA_UNUSED,
         void *event_info EINA_UNUSED)
{
   Elm_Toolbar_Item *item = data;

   item->on_move = EINA_FALSE;

   evas_object_event_callback_del_full
     (VIEW(item), EVAS_CALLBACK_MOVE, _item_move_cb, data);
}

static void
_items_change(Elm_Toolbar_Item *reorder_from, Elm_Toolbar_Item *reorder_to)
{
   Elm_Toolbar_Item *prev = NULL, *next = NULL;
   int tmp;

   if (!reorder_from) return;
   ELM_TOOLBAR_DATA_GET(WIDGET(reorder_from), sd);
   if (reorder_from == reorder_to) return;

   if ((reorder_to) &&
       (!reorder_from->separator) && (!reorder_to->separator))
     {
        prev = ELM_TOOLBAR_ITEM_FROM_INLIST
            (EINA_INLIST_GET(reorder_from)->prev);
        if (prev == reorder_to)
          prev = reorder_from;
        if (!prev)
          next = ELM_TOOLBAR_ITEM_FROM_INLIST
              (EINA_INLIST_GET(reorder_from)->next);
        if (next == reorder_to)
          next = NULL;

        sd->items = eina_inlist_remove
            (sd->items, EINA_INLIST_GET(reorder_from));
        sd->items = eina_inlist_append_relative
            (sd->items, EINA_INLIST_GET(reorder_from),
            EINA_INLIST_GET(reorder_to));

        sd->items = eina_inlist_remove
            (sd->items, EINA_INLIST_GET(reorder_to));
        if (prev)
          sd->items = eina_inlist_append_relative
              (sd->items, EINA_INLIST_GET(reorder_to),
              EINA_INLIST_GET(prev));
        else if (next)
          sd->items = eina_inlist_prepend_relative
              (sd->items, EINA_INLIST_GET(reorder_to),
              EINA_INLIST_GET(next));
        else
          sd->items = eina_inlist_prepend
             (sd->items, EINA_INLIST_GET(reorder_to));

        evas_object_box_remove(sd->bx, VIEW(reorder_from));
        evas_object_box_insert_after(sd->bx, VIEW(reorder_from),
                                     VIEW(reorder_to));
        evas_object_box_remove(sd->bx, VIEW(reorder_to));
        if (prev)
          evas_object_box_insert_after(sd->bx, VIEW(reorder_to),
                                       VIEW(prev));
        else if (next)
          evas_object_box_insert_before(sd->bx, VIEW(reorder_to),
                                        VIEW(next));
        else
          evas_object_box_prepend(sd->bx, VIEW(reorder_to));

        tmp = reorder_from->prio.priority;
        reorder_from->prio.priority = reorder_to->prio.priority;
        reorder_to->prio.priority = tmp;

        reorder_from->on_move = EINA_TRUE;
        reorder_to->on_move = EINA_TRUE;

        evas_object_event_callback_add
           (VIEW(reorder_from), EVAS_CALLBACK_MOVE,
           _item_move_cb, reorder_from);
        evas_object_event_callback_add
           (VIEW(reorder_to), EVAS_CALLBACK_MOVE,
           _item_move_cb, reorder_to);
     }

   _resize_cb(WIDGET(reorder_from), NULL, NULL, NULL);
}

static void
_transit_del_cb(void *data, Elm_Transit *transit EINA_UNUSED)
{
   Elm_Toolbar_Item *it, *item = data;
   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   if (item->reorder_to)
     {
        if (item->reorder_to == sd->reorder_empty)
          sd->reorder_empty = item;
        else if (item == sd->reorder_empty)
          sd->reorder_empty = item->reorder_to;

        _items_change(item->reorder_to, item);

        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (it != item)
               {
                  if (it->reorder_to == item)
                    it->reorder_to = item->reorder_to;
                  else if (it->reorder_to == item->reorder_to)
                    it->reorder_to = item;
               }
          }
     }
   if (item->proxy)
     {
        evas_object_image_source_visible_set(elm_image_object_get(item->proxy), EINA_TRUE);
        ELM_SAFE_FREE(item->proxy, evas_object_del);
     }
   item->trans = NULL;

   if (item->reorder_to)
     {
        EINA_INLIST_FOREACH(sd->items, it)
           if (it->trans) break;

        if (!it) sd->reorder_empty = sd->reorder_item;
     }
   item->reorder_to = NULL;
}

static void
_item_transition_start
(Elm_Toolbar_Item *it, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord tx, ty;
   Evas_Object *obj = WIDGET(it), *img = NULL;
   ELM_TOOLBAR_DATA_GET(obj, sd);

   it->proxy = elm_image_add(obj);
   img = elm_image_object_get(it->proxy);
   elm_image_aspect_fixed_set(it->proxy, EINA_FALSE);
   evas_object_image_source_set(img, VIEW(it));
   evas_object_image_source_visible_set(img, EINA_FALSE);
   evas_object_image_source_clip_set(img, EINA_FALSE);

   it->trans = elm_transit_add();
   elm_transit_object_add(it->trans, it->proxy);
   evas_object_geometry_get(VIEW(sd->reorder_empty), &tx, &ty, NULL, NULL);
   evas_object_move(it->proxy, x, y);
   evas_object_resize(it->proxy, w, h);
   evas_object_show(it->proxy);

   elm_transit_effect_translation_add(it->trans, 0, 0, tx - x, 0);
   elm_transit_duration_set(it->trans, 0.2);
   elm_transit_del_cb_set(it->trans, _transit_del_cb, it);
   elm_transit_go(it->trans);

   it->reorder_to = sd->reorder_empty;
}

static void
_animate_missed_items(Elm_Toolbar_Item *prev, Elm_Toolbar_Item *next)
{
   ELM_TOOLBAR_DATA_GET(WIDGET(prev), sd);
   Elm_Toolbar_Item *it, *it2;
   Eina_List *list, *l;
   Evas_Object *o;
   Eina_Bool reverse = EINA_FALSE;
   Evas_Coord fx, fy, fw, fh;

   list = evas_object_box_children_get(sd->bx);

   EINA_LIST_FOREACH(list, l, o)
     {
        if (o == VIEW(prev))
          break;
        else if (o == VIEW(next))
          reverse = EINA_TRUE;
     }

   if (!reverse)
     l = eina_list_next(l);
   else
     l = eina_list_prev(l);

   while (VIEW(next) != eina_list_data_get(l))
     {
        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (VIEW(it) == eina_list_data_get(l))
               {
                  if (!it->trans && it != sd->reorder_item)
                    {
                       evas_object_geometry_get(VIEW(sd->reorder_empty), &fx, &fy, &fw, &fh);
                       _item_transition_start(it, fx, fy, fw, fh);
                       sd->reorder_empty = it;
                    }
                  EINA_INLIST_FOREACH(sd->items, it2)
                    {
                       if (it == it2->reorder_to) break;
                    }
                  if (it2)
                    {
                       it2->reorder_to = NULL;
                       evas_object_geometry_get(it2->proxy, &fx, &fy, &fw, &fh);
                       if (it2->trans) elm_transit_del(it2->trans);
                       _item_transition_start(it2, fx, fy, fw, fh);
                       sd->reorder_empty = it;
                    }
               }
          }
        if (!reverse)
          l = eina_list_next(l);
        else
          l = eina_list_prev(l);
     }
}

static void
_mouse_move_reorder(Elm_Toolbar_Item *item,
                    Evas *evas EINA_UNUSED,
                    Evas_Object *obj EINA_UNUSED,
                    Evas_Event_Mouse_Move *ev)
{
   Evas_Coord x, y, w, h;
   Evas_Coord fx, fy, fw, fh;
   Elm_Toolbar_Item *it, *it2;

   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   evas_object_geometry_get(VIEW(item), &x, &y, &w, &h);
   if (sd->vertical)
     evas_object_move(item->proxy, x, ev->cur.canvas.y - (h / 2));
   else
     evas_object_move(item->proxy, ev->cur.canvas.x - (w / 2), y);
   evas_object_show(item->proxy);

   if (sd->reorder_empty->on_move) return;

   evas_object_geometry_get(sd->VIEW(reorder_empty), &x, &y, &w, &h);
   if (ev->cur.canvas.x < x || ev->cur.canvas.x > x + w)
     {
        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (it->on_move) continue;
             evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);
             if (ev->cur.canvas.x > x && ev->cur.canvas.x < x + w) break;
          }
        if (it && (it != sd->reorder_empty))
          {
             _animate_missed_items(sd->reorder_empty, it);
             if (!it->trans && it != item)
               {
                  evas_object_geometry_get(VIEW(it), &fx, &fy, &fw, &fh);
                  _item_transition_start(it, fx, fy, fw, fh);
                  sd->reorder_empty = it;
               }
             EINA_INLIST_FOREACH(sd->items, it2)
               {
                  if (it == it2->reorder_to) break;
               }
             if (it2)
               {
                  it2->reorder_to = NULL;
                  evas_object_geometry_get(it2->proxy, &fx, &fy, &fw, &fh);
                  if (it2->trans) elm_transit_del(it2->trans);
                  _item_transition_start(it2, fx, fy, fw, fh);
                  sd->reorder_empty = it;
               }
          }
     }
}

static void
_mouse_up_reorder(Elm_Toolbar_Item *it,
                  Evas *evas EINA_UNUSED,
                  Evas_Object *obj,
                  Evas_Event_Mouse_Up *ev EINA_UNUSED)
{
   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOUSE_MOVE,
     (Evas_Object_Event_Cb)_mouse_move_reorder, it);
   evas_object_event_callback_del_full
     (sd->more, EVAS_CALLBACK_MOUSE_MOVE,
     (Evas_Object_Event_Cb)_mouse_move_reorder, it);
   evas_object_event_callback_del_full
     (VIEW(it), EVAS_CALLBACK_MOUSE_MOVE,
     (Evas_Object_Event_Cb)_mouse_move_reorder, it);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOUSE_UP,
     (Evas_Object_Event_Cb)_mouse_up_reorder, it);
   evas_object_event_callback_del_full
     (sd->more, EVAS_CALLBACK_MOUSE_UP,
     (Evas_Object_Event_Cb)_mouse_up_reorder, it);

   if (it->proxy)
     {
        evas_object_image_source_visible_set(elm_image_object_get(it->proxy), EINA_TRUE);
        ELM_SAFE_FREE(it->proxy, evas_object_del);
     }

   eo_do(obj, elm_interface_scrollable_hold_set(EINA_FALSE));
}

static void
_item_reorder_start(Elm_Toolbar_Item *item)
{
   Evas_Object *obj = WIDGET(item), *img = NULL;
   Evas_Coord x, y, w, h;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   sd->reorder_empty = sd->reorder_item = item;

   item->proxy = elm_image_add(obj);
   img = elm_image_object_get(item->proxy);
   elm_image_aspect_fixed_set(item->proxy, EINA_FALSE);
   evas_object_image_source_set(img, VIEW(item));
   evas_object_image_source_visible_set(img, EINA_FALSE);
   evas_object_image_source_clip_set(img, EINA_FALSE);

   evas_object_layer_set(item->proxy, 100);
   edje_object_signal_emit(VIEW(item), "elm,state,moving", "elm");

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_MOVE,
     (Evas_Object_Event_Cb)_mouse_move_reorder, item);

   evas_object_event_callback_add
     (sd->more, EVAS_CALLBACK_MOUSE_MOVE,
     (Evas_Object_Event_Cb)_mouse_move_reorder, item);

   evas_object_event_callback_add
     (item->proxy, EVAS_CALLBACK_MOUSE_MOVE,
     (Evas_Object_Event_Cb)_mouse_move_reorder, item);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOUSE_UP,
     (Evas_Object_Event_Cb)_mouse_up_reorder, item);

   evas_object_event_callback_add
     (sd->more, EVAS_CALLBACK_MOUSE_UP,
     (Evas_Object_Event_Cb)_mouse_up_reorder, item);

   evas_object_geometry_get(VIEW(item), &x, &y, &w, &h);
   evas_object_resize(item->proxy, w, h);
   evas_object_move(item->proxy, x, y);
   evas_object_show(item->proxy);

   eo_do(WIDGET(item), elm_interface_scrollable_hold_set(EINA_TRUE));
}

static Eina_Bool
_long_press_cb(void *data)
{
   Elm_Toolbar_Item *it = data;
   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   sd->long_timer = NULL;
   sd->long_press = EINA_TRUE;

   if (sd->reorder_mode)
     _item_reorder_start(it);

   evas_object_smart_callback_call(WIDGET(it), SIG_LONGPRESSED, it);

   return ECORE_CALLBACK_CANCEL;
}

static void
_mouse_move_cb(Elm_Toolbar_Item *it,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               Evas_Event_Mouse_Move *ev)
{
   Evas_Coord x, y, w, h;

   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);
   evas_object_geometry_get(VIEW(it), &x, &y, &w, &h);

   if ((x > ev->cur.canvas.x) || (ev->cur.canvas.x > x + w) ||
       (y > ev->cur.canvas.y) || (ev->cur.canvas.y > y + h))
     {
        ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);
     }
}

static void
_mouse_down_cb(Elm_Toolbar_Item *it,
               Evas *evas EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               Evas_Event_Mouse_Down *ev)
{
   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   if (ev->button != 1) return;
   if (ev->flags & EVAS_BUTTON_DOUBLE_CLICK)
     evas_object_smart_callback_call(WIDGET(it), SIG_CLICKED_DOUBLE, it);
   sd->mouse_down = EINA_TRUE;
   sd->long_press = EINA_FALSE;
   if (sd->long_timer)
     ecore_timer_interval_set
       (sd->long_timer, _elm_config->longpress_timeout);
   else
     sd->long_timer = ecore_timer_add
         (_elm_config->longpress_timeout, _long_press_cb, it);

   evas_object_event_callback_add(VIEW(it), EVAS_CALLBACK_MOUSE_MOVE,
                                  (Evas_Object_Event_Cb)_mouse_move_cb, it);
}

static void
_mouse_up_cb(Elm_Toolbar_Item *it,
             Evas *evas EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             Evas_Event_Mouse_Up *ev)
{
   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   if (ev->button != 1) return;
   sd->mouse_down = EINA_FALSE;
   ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);
   if ((!elm_object_item_disabled_get((Elm_Object_Item *)it)) &&
       (sd->focused_item != it))
     elm_object_item_focus_set((Elm_Object_Item *)it, EINA_TRUE);
   evas_object_event_callback_del_full
     (VIEW(it), EVAS_CALLBACK_MOUSE_MOVE,
     (Evas_Object_Event_Cb)_mouse_move_cb, it);
}

static void
_mouse_in_cb(void *data,
             Evas_Object *obj EINA_UNUSED,
             const char *emission EINA_UNUSED,
             const char *source EINA_UNUSED)
{
   Elm_Toolbar_Item *it = data;

   edje_object_signal_emit(VIEW(it), "elm,state,highlighted", "elm");
   if (it->icon)
     elm_widget_signal_emit(it->icon, "elm,state,highlighted", "elm");
}

static void
_mouse_out_cb(void *data,
              Evas_Object *obj EINA_UNUSED,
              const char *emission EINA_UNUSED,
              const char *source EINA_UNUSED)
{
   Elm_Toolbar_Item *it = data;

   edje_object_signal_emit(VIEW(it), "elm,state,unhighlighted", "elm");
   if (it->icon)
     elm_widget_signal_emit(it->icon, "elm,state,unhighlighted", "elm");
}

static void
_scroll_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL, NULL);
}

static void
_scroll_anim_start_cb(Evas_Object *obj,
                      void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_anim_stop_cb(Evas_Object *obj,
                     void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data EINA_UNUSED)
{
   ELM_TOOLBAR_DATA_GET(obj, sd);
   ELM_SAFE_FREE(sd->long_timer, ecore_timer_del);

   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data EINA_UNUSED)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_layout(Evas_Object *o,
        Evas_Object_Box_Data *priv,
        void *data)
{
   Evas_Object *obj = (Evas_Object *)data;

   ELM_TOOLBAR_DATA_GET(obj, sd);
   _els_box_layout
     (o, priv, !sd->vertical, sd->homogeneous, elm_widget_mirrored_get(obj));
}

static char *
_access_info_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_Toolbar_Item *it = (Elm_Toolbar_Item *)data;
   const char *txt = ((Elm_Widget_Item *)it)->access_info;

   if (!txt) txt = it->label;
   if (txt) return strdup(txt);

   return NULL;
}

static char *
_access_state_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
   Elm_Toolbar_Item *it = (Elm_Toolbar_Item *)data;

   if (it->separator)
     return strdup(E_("Separator"));
   else if (elm_widget_item_disabled_get(it))
     return strdup(E_("State: Disabled"));
   else if (it->selected)
     return strdup(E_("State: Selected"));
   else if (it->menu)
     return strdup(E_("Has menu"));

   return NULL;
}

static Eina_Bool
_item_del_pre_hook(Elm_Object_Item *it)
{
   Elm_Toolbar_Item *item, *next = NULL;
   Evas_Object *obj;

   item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   obj = WIDGET(item);

   if (item != sd->more_item) /* more item does not get in the list */
     {
        if (!sd->delete_me)
          next = ELM_TOOLBAR_ITEM_FROM_INLIST(EINA_INLIST_GET(item)->next);
        sd->items = eina_inlist_remove(sd->items, EINA_INLIST_GET(item));
        sd->item_count--;
        if (!sd->delete_me)
          {
             if (!next) next = ELM_TOOLBAR_ITEM_FROM_INLIST(sd->items);
             if ((sd->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS) &&
                 item->selected && next) _item_select(next);
          }
     }

   _item_del(item);

   if (item != sd->more_item)
      eo_do(obj, elm_obj_widget_theme_apply(NULL));

   return EINA_TRUE;
}

static void
_access_activate_cb(void *data EINA_UNUSED,
                    Evas_Object *part_obj EINA_UNUSED,
                    Elm_Object_Item *item)
{
   Elm_Toolbar_Item *it;
   it = (Elm_Toolbar_Item *)item;
   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   if (elm_widget_item_disabled_get(it)) return;

   if (it->selected && (sd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS))
     {
        _elm_access_say(E_("Unselected"));
        _item_unselect(it);
     }
   else
     {
        _elm_access_say(E_("Selected"));
        _item_select(it);
     }
}

static void
_access_widget_item_register(Elm_Toolbar_Item *it)
{
   Elm_Access_Info *ai;
   _elm_access_widget_item_register((Elm_Widget_Item *)it);
   ai = _elm_access_info_get(it->base.access_obj);

   _elm_access_text_set(ai, ELM_ACCESS_TYPE, E_("Toolbar Item"));
   _elm_access_callback_set(ai, ELM_ACCESS_INFO, _access_info_cb, it);
   _elm_access_callback_set(ai, ELM_ACCESS_STATE, _access_state_cb, it);
   _elm_access_activate_callback_set(ai, _access_activate_cb, NULL);
}

static Elm_Toolbar_Item *
_item_new(Evas_Object *obj,
          const char *icon,
          const char *label,
          Evas_Smart_Cb func,
          const void *data)
{
   Evas_Object *icon_obj;
   Elm_Toolbar_Item *it;
   Evas_Coord mw, mh;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   icon_obj = elm_icon_add(obj);
   elm_icon_order_lookup_set(icon_obj, sd->lookup_order);
   if (!icon_obj) return NULL;

   it = elm_widget_item_new(obj, Elm_Toolbar_Item);
   if (!it)
     {
        evas_object_del(icon_obj);
        return NULL;
     }

   elm_widget_item_del_pre_hook_set(it, _item_del_pre_hook);
   elm_widget_item_disable_hook_set(it, _item_disable_hook);
   elm_widget_item_text_set_hook_set(it, _item_text_set_hook);
   elm_widget_item_text_get_hook_set(it, _item_text_get_hook);
   elm_widget_item_content_set_hook_set(it, _item_content_set_hook);
   elm_widget_item_content_get_hook_set(it, _item_content_get_hook);
   elm_widget_item_content_unset_hook_set(it, _item_content_unset_hook);
   elm_widget_item_focus_set_hook_set(it, _item_focus_set_hook);
   elm_widget_item_focus_get_hook_set(it, _item_focus_get_hook);

   it->label = eina_stringshare_add(label);
   it->prio.visible = 1;
   it->prio.priority = 0;
   it->func = func;
   it->separator = EINA_FALSE;
   it->object = NULL;
   it->base.data = data;

   VIEW(it) = edje_object_add(evas_object_evas_get(obj));
   evas_object_data_set(VIEW(it), "item", it);

   if (_elm_config->access_mode == ELM_ACCESS_MODE_ON)
     _access_widget_item_register(it);

   if (_item_icon_set(icon_obj, "toolbar/", icon))
     {
        it->icon = icon_obj;
        it->icon_str = eina_stringshare_add(icon);
     }
   else
     {
        it->icon = NULL;
        it->icon_str = NULL;
        evas_object_del(icon_obj);
     }

   elm_widget_theme_object_set
     (obj, VIEW(it), "toolbar", "item", elm_widget_style_get(obj));
   edje_object_signal_callback_add
     (VIEW(it), "elm,action,click", "elm", _select_cb, it);
   edje_object_signal_callback_add
     (VIEW(it), "mouse,clicked,*", "*", (Edje_Signal_Cb)_select_filter_cb, it);
   edje_object_signal_callback_add
     (VIEW(it), "elm,mouse,in", "elm", _mouse_in_cb, it);
   edje_object_signal_callback_add
     (VIEW(it), "elm,mouse,out", "elm", _mouse_out_cb, it);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_DOWN, (Evas_Object_Event_Cb)_mouse_down_cb,
     it);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_MOUSE_UP, (Evas_Object_Event_Cb)_mouse_up_cb, it);
   elm_widget_sub_object_add(obj, VIEW(it));

   if (it->icon)
     {
        int ms = 0;

        ms = ((double)sd->icon_size * elm_config_scale_get());
        evas_object_size_hint_min_set(it->icon, ms, ms);
        evas_object_size_hint_max_set(it->icon, ms, ms);
        edje_object_part_swallow(VIEW(it), "elm.swallow.icon", it->icon);
        edje_object_signal_emit(VIEW(it), "elm,state,icon,visible", "elm");
        evas_object_show(it->icon);
        elm_widget_sub_object_add(obj, it->icon);
     }
   if (it->label)
     {
        edje_object_part_text_escaped_set(VIEW(it), "elm.text", it->label);
        edje_object_signal_emit(VIEW(it), "elm,state,text,visible", "elm");
     }

   mw = mh = -1;
   if (!it->separator && !it->object)
     elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   // If the min size is changed by edje signal in edc,
   //the below function should be called before the calculation.
   edje_object_message_signal_process(VIEW(it));
   edje_object_size_min_restricted_calc(VIEW(it), &mw, &mh, mw, mh);
   if (!it->separator && !it->object)
     elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   if (sd->shrink_mode != ELM_TOOLBAR_SHRINK_EXPAND)
     {
        if (sd->vertical)
          {
             evas_object_size_hint_weight_set(VIEW(it), EVAS_HINT_EXPAND, -1.0);
             evas_object_size_hint_align_set
               (VIEW(it), EVAS_HINT_FILL, EVAS_HINT_FILL);
          }
        else
          {
             evas_object_size_hint_weight_set(VIEW(it), -1.0, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
               (VIEW(it), EVAS_HINT_FILL, EVAS_HINT_FILL);
          }
     }
   else
     {
        evas_object_size_hint_weight_set
          (VIEW(it), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set
          (VIEW(it), EVAS_HINT_FILL, EVAS_HINT_FILL);
     }

   evas_object_size_hint_min_set(VIEW(it), mw, mh);
   evas_object_size_hint_max_set(VIEW(it), -1, -1);
   evas_object_event_callback_add
     (VIEW(it), EVAS_CALLBACK_RESIZE, _item_resize, obj);

   if ((!sd->items) && (sd->select_mode == ELM_OBJECT_SELECT_MODE_ALWAYS))
     _item_select(it);
   return it;
}

static void
_elm_toolbar_item_icon_update(Elm_Toolbar_Item *item)
{
   Evas_Coord mw = -1, mh = -1, minw = -1, minh = -1;
   Elm_Toolbar_Item_State *it_state;
   Evas_Object *old_icon =
     edje_object_part_swallow_get(VIEW(item), "elm.swallow.icon");
   Eina_List *l;

   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   elm_widget_sub_object_del(WIDGET(item), old_icon);
   edje_object_part_swallow(VIEW(item), "elm.swallow.icon", item->icon);
   if (item->icon)
       edje_object_signal_emit(VIEW(item), "elm,state,icon,visible", "elm");
   else
       edje_object_signal_emit(VIEW(item), "elm,state,icon,hidden", "elm");
   evas_object_hide(old_icon);
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   // If the min size is changed by edje signal in edc,
   //the below function should be called before the calculation.
   edje_object_message_signal_process(VIEW(item));
   edje_object_size_min_restricted_calc(VIEW(item), &mw, &mh, mw, mh);
   elm_coords_finger_size_adjust(1, &mw, 1, &mh);
   if (sd->shrink_mode != ELM_TOOLBAR_SHRINK_EXPAND)
     {
        if (sd->vertical)
          {
             evas_object_size_hint_weight_set
               (VIEW(item), EVAS_HINT_EXPAND, -1.0);
             evas_object_size_hint_align_set
               (VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
          }
        else
          {
             evas_object_size_hint_weight_set
               (VIEW(item), -1.0, EVAS_HINT_EXPAND);
             evas_object_size_hint_align_set
               (VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
          }
     }
   else
     {
        evas_object_size_hint_weight_set
          (VIEW(item), EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set
          (VIEW(item), EVAS_HINT_FILL, EVAS_HINT_FILL);
     }

   evas_object_size_hint_min_get(VIEW(item), &minw, &minh);
   if ((minw < mw) && (minh < mh))
     evas_object_size_hint_min_set(VIEW(item), mw, mh);
   else if ((minw < mw) && (minh > mh))
     evas_object_size_hint_min_set(VIEW(item), mw, minh);
   else if ((minw > mw) && (minh < mh))
     evas_object_size_hint_min_set(VIEW(item), minw, mh);

   EINA_LIST_FOREACH(item->states, l, it_state)
     {
        if (it_state->icon == old_icon) return;
     }
   evas_object_del(old_icon);
}

static void
_elm_toolbar_item_icon_set_cb(void *data,
                              Evas_Object *obj,
                              const char *emission,
                              const char *source)
{
   Elm_Toolbar_Item *item = data;

   edje_object_part_unswallow(VIEW(item), item->icon);
   _elm_toolbar_item_icon_update(item);
   edje_object_signal_callback_del
     (obj, emission, source, _elm_toolbar_item_icon_set_cb);
   edje_object_signal_emit(VIEW(item), "elm,state,icon,reset", "elm");
}

static void
_elm_toolbar_item_icon_obj_set(Evas_Object *obj,
                               Elm_Toolbar_Item *item,
                               Evas_Object *icon_obj,
                               const char *icon_str,
                               double icon_size,
                               const char *sig)
{
   Evas_Object *old_icon;
   const char *s;
   int ms = 0;

   if (icon_str)
     eina_stringshare_replace(&item->icon_str, icon_str);
   else
     {
        eina_stringshare_del(item->icon_str);
        item->icon_str = NULL;
     }
   item->icon = icon_obj;
   if (icon_obj)
     {
        ms = (icon_size * elm_config_scale_get());
        evas_object_size_hint_min_set(item->icon, ms, ms);
        evas_object_size_hint_max_set(item->icon, ms, ms);
        evas_object_show(item->icon);
        elm_widget_sub_object_add(obj, item->icon);
     }
   s = edje_object_data_get(VIEW(item), "transition_animation_on");
   if ((s) && (atoi(s)))
     {
        old_icon = edje_object_part_swallow_get
            (VIEW(item), "elm.swallow.icon_new");
        if (old_icon)
          {
             elm_widget_sub_object_del(WIDGET(item), old_icon);
             evas_object_hide(old_icon);
          }
        edje_object_part_swallow
          (VIEW(item), "elm.swallow.icon_new", item->icon);
        edje_object_signal_emit(VIEW(item), sig, "elm");
        edje_object_signal_callback_add
          (VIEW(item), "elm,state,icon_set,done", "elm",
          _elm_toolbar_item_icon_set_cb, item);
     }
   else
     _elm_toolbar_item_icon_update(item);

   _resize_cb(obj, NULL, NULL, NULL);
}

static void
_elm_toolbar_item_state_cb(void *data EINA_UNUSED,
                           Evas_Object *obj EINA_UNUSED,
                           void *event_info)
{
   Elm_Toolbar_Item *it = event_info;
   Elm_Toolbar_Item_State *it_state;

   it_state = eina_list_data_get(it->current_state);
   if (it_state->func)
     it_state->func((void *)it_state->data, obj, event_info);
}

static Elm_Toolbar_Item_State *
_item_state_new(const char *label,
                const char *icon_str,
                Evas_Object *icon,
                Evas_Smart_Cb func,
                const void *data)
{
   Elm_Toolbar_Item_State *it_state;

   it_state = ELM_NEW(Elm_Toolbar_Item_State);
   it_state->label = eina_stringshare_add(label);
   it_state->icon_str = eina_stringshare_add(icon_str);
   it_state->icon = icon;
   it_state->func = func;
   it_state->data = data;

   return it_state;
}

static void
_elm_toolbar_action_left_cb(void *data,
                            Evas_Object *o EINA_UNUSED,
                            const char *sig EINA_UNUSED,
                            const char *src EINA_UNUSED)
{
   Evas_Object *obj = data;
   Elm_Toolbar_Item *it, *it2;
   Eina_Bool done = EINA_FALSE;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->selected)
          {
             Eina_Bool found = EINA_FALSE;

             EINA_INLIST_REVERSE_FOREACH(sd->items, it2)
               {
                  if (elm_object_item_disabled_get((Elm_Object_Item *)it2))
                    continue;
                  if (it2 == it)
                    {
                       found = EINA_TRUE;
                       continue;
                    }
                  if (!found) continue;
                  if (it2->separator) continue;
                  _item_unselect(it);
                  _item_select(it2);
                  break;
               }
             done = EINA_TRUE;
             break;
          }
     }
   if (!done)
     {
        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (elm_object_item_disabled_get((Elm_Object_Item *)it)) continue;
             if (it->separator) continue;
             _item_select(it);
             break;
          }
     }
}

static void
_elm_toolbar_action_right_cb(void *data,
                             Evas_Object *o EINA_UNUSED,
                             const char *sig EINA_UNUSED,
                             const char *src EINA_UNUSED)
{
   Evas_Object *obj = data;
   Elm_Toolbar_Item *it, *it2;
   Eina_Bool done = EINA_FALSE;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->selected)
          {
             Eina_Bool found = EINA_FALSE;

             EINA_INLIST_FOREACH(sd->items, it2)
               {
                  if (elm_object_item_disabled_get((Elm_Object_Item *)it2))
                    continue;
                  if (it2 == it)
                    {
                       found = EINA_TRUE;
                       continue;
                    }
                  if (!found) continue;
                  if (it2->separator) continue;
                  _item_unselect(it);
                  _item_select(it2);
                  break;
               }
             done = EINA_TRUE;
             break;
          }
     }
   if (!done)
     {
        EINA_INLIST_REVERSE_FOREACH(sd->items, it)
          {
             if (elm_object_item_disabled_get((Elm_Object_Item *)it)) continue;
             if (it->separator) continue;
             _item_select(it);
             break;
          }
     }
}

static void
_elm_toolbar_action_up_cb(void *data,
                          Evas_Object *o,
                          const char *sig,
                          const char *src)
{
   _elm_toolbar_action_left_cb(data, o, sig, src);
}

static void
_elm_toolbar_action_down_cb(void *data,
                            Evas_Object *o,
                            const char *sig,
                            const char *src)
{
   _elm_toolbar_action_right_cb(data, o, sig, src);
}

static void
_elm_toolbar_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Toolbar_Smart_Data *priv = _pd;
   Evas_Object *edje;

   elm_widget_sub_object_parent_add(obj);

   edje = edje_object_add(evas_object_evas_get(obj));
   elm_widget_resize_object_set(obj, edje, EINA_TRUE);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_add());

   elm_widget_theme_object_set
     (obj, edje, "toolbar", "base", elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   /* common scroller hit rectangle setup */
   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   eo_do(obj, elm_interface_scrollable_objects_set(edje, priv->hit_rect));

   priv->standard_priority = -99999;

   eo_do(obj,
         elm_interface_scrollable_bounce_allow_set
         (_elm_config->thumbscroll_bounce_enable, EINA_FALSE),
         elm_interface_scrollable_policy_set
         (ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF),
         elm_interface_scrollable_scroll_cb_set(_scroll_cb),
         elm_interface_scrollable_animate_start_cb_set(_scroll_anim_start_cb),
         elm_interface_scrollable_animate_stop_cb_set(_scroll_anim_stop_cb),
         elm_interface_scrollable_drag_start_cb_set(_scroll_drag_start_cb),
         elm_interface_scrollable_drag_stop_cb_set(_scroll_drag_stop_cb));

   edje_object_signal_callback_add
     (edje, "elm,action,left", "elm", _elm_toolbar_action_left_cb, obj);
   edje_object_signal_callback_add
     (edje, "elm,action,right", "elm", _elm_toolbar_action_right_cb, obj);
   edje_object_signal_callback_add
     (edje, "elm,action,up", "elm", _elm_toolbar_action_up_cb, obj);
   edje_object_signal_callback_add
     (edje, "elm,action,down", "elm", _elm_toolbar_action_down_cb, obj);

   priv->shrink_mode = ELM_TOOLBAR_SHRINK_NONE;
   priv->theme_icon_size = _elm_toolbar_icon_size_get(obj);
   priv->icon_size = priv->theme_icon_size;

   priv->homogeneous = EINA_TRUE;
   priv->align = 0.5;

   priv->bx = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_size_hint_align_set(priv->bx, priv->align, 0.5);
   evas_object_box_layout_set(priv->bx, _layout, obj, NULL);
   elm_widget_sub_object_add(obj, priv->bx);
   eo_do(obj, elm_interface_scrollable_content_set(priv->bx));
   evas_object_show(priv->bx);

   priv->more = elm_layout_add(obj);
   if (!elm_layout_theme_set(priv->more, "toolbar", "more", "default"))
     CRI("Failed to set layout!");
   if (priv->vertical)
     edje_object_signal_emit(priv->more, "elm,orient,vertical", "elm");
   else
     edje_object_signal_emit(priv->more, "elm,orient,horizontal", "elm");

   elm_widget_sub_object_add(obj, priv->more);
   evas_object_show(priv->more);

   priv->bx_more = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_size_hint_align_set(priv->bx_more, priv->align, 0.5);
   evas_object_box_layout_set(priv->bx_more, _layout, obj, NULL);
   elm_widget_sub_object_add(obj, priv->bx_more);
   elm_layout_content_set
     (priv->more, "elm.swallow.content", priv->bx_more);
   evas_object_show(priv->bx_more);

   priv->bx_more2 = evas_object_box_add(evas_object_evas_get(obj));
   evas_object_size_hint_align_set(priv->bx_more2, priv->align, 0.5);
   evas_object_box_layout_set(priv->bx_more2, _layout, obj, NULL);
   elm_widget_sub_object_add(obj, priv->bx_more2);
   elm_layout_content_set
     (priv->more, "elm.swallow.content2", priv->bx_more2);
   evas_object_show(priv->bx_more2);

   elm_toolbar_shrink_mode_set(obj, _elm_config->toolbar_shrink_mode);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize_cb, obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _move_cb, obj);
   evas_object_event_callback_add
     (priv->bx, EVAS_CALLBACK_RESIZE, _resize_cb, obj);
   elm_toolbar_icon_order_lookup_set(obj, ELM_ICON_LOOKUP_THEME_FDO);

   _elm_toolbar_highlight_in_theme(obj);
   _sizing_eval(obj);
}

static void
_elm_toolbar_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Toolbar_Item *it, *next;

   Elm_Toolbar_Smart_Data *sd = _pd;

   sd->delete_me = EINA_TRUE;

   ecore_job_del(sd->resize_job);
   sd->resize_job = NULL;

   it = ELM_TOOLBAR_ITEM_FROM_INLIST(sd->items);
   while (it)
     {
        next = ELM_TOOLBAR_ITEM_FROM_INLIST(EINA_INLIST_GET(it)->next);
        elm_widget_item_del(it);
        it = next;
     }
   if (sd->more_item) elm_widget_item_del(sd->more_item);
   ecore_timer_del(sd->long_timer);

   eo_do_super(obj, MY_CLASS, evas_obj_smart_del());
}

static void
_elm_toolbar_smart_move(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Elm_Toolbar_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_move(x, y));

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_toolbar_smart_resize(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Elm_Toolbar_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_resize(w, h));

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_toolbar_smart_member_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *member = va_arg(*list, Evas_Object *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   eo_do_super(obj, MY_CLASS, evas_obj_smart_member_add(member));

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static Eina_List *
_access_item_find_append(const Evas_Object *obj,
                         Evas_Object *bx,
                         Eina_List *items)
{
   Elm_Toolbar_Item *it;
   Eina_List *list;

   ELM_TOOLBAR_DATA_GET(obj, sd);

   list = evas_object_box_children_get(bx);
   if (!list) return items;

   EINA_INLIST_FOREACH (sd->items, it)
     {
        if (it->separator) continue;
        if (eina_list_data_find(list, it->base.view))
          items = eina_list_append(items, it->base.access_obj);
     }

   return items;
}

static Eina_Bool _elm_toolbar_smart_focus_next_enable = EINA_FALSE;

static void
_elm_toolbar_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = _elm_toolbar_smart_focus_next_enable;
}

static void
_elm_toolbar_smart_focus_next(Eo *obj, void *_pd, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Eina_List *items = NULL;

   Elm_Toolbar_Smart_Data *sd = _pd;

   if (sd->more_item && sd->more_item->selected)
     {
        items = _access_item_find_append(obj, sd->bx_more, items);
        items = _access_item_find_append(obj, sd->bx_more2, items);
        items = eina_list_append(items, sd->more_item->base.access_obj);
     }
   else
     {
        items = _access_item_find_append(obj, sd->bx, items);
        if (sd->more_item &&
            eina_list_data_find(evas_object_box_children_get(sd->bx),
                                            sd->more_item->base.view))
          items = eina_list_append(items, sd->more_item->base.access_obj);
     }

   if (ret) *ret = elm_widget_focus_list_next_get
            (obj, items, eina_list_data_get, dir, next);
}

static void
_access_obj_process(Elm_Toolbar_Smart_Data * sd, Eina_Bool is_access)
{
   Elm_Toolbar_Item *it;

   EINA_INLIST_FOREACH (sd->items, it)
     {
        if (is_access) _access_widget_item_register(it);
        else _elm_access_widget_item_unregister((Elm_Widget_Item *)it);
     }
}

static void
_elm_toolbar_smart_access(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Toolbar_Smart_Data *sd = _pd;
   _elm_toolbar_smart_focus_next_enable = va_arg(*list, int);
   _access_obj_process(sd, _elm_toolbar_smart_focus_next_enable);
}

static void
_elm_toolbar_coordinates_adjust(Elm_Toolbar_Item *it,
                                Evas_Coord *x,
                                Evas_Coord *y,
                                Evas_Coord *w,
                                Evas_Coord *h)
{
   ELM_TOOLBAR_DATA_GET(WIDGET(it), sd);

   Evas_Coord ix, iy, iw, ih, vx, vy, vw, vh;

   evas_object_geometry_get(sd->hit_rect, &vx, &vy, &vw, &vh);
   evas_object_geometry_get(VIEW(it), &ix, &iy, &iw, &ih);
   *x = ix;
   *y = iy;
   *w = iw;
   *h = ih;
   if (sd->vertical)
     {
        //TODO: Enhance it later.
        if ((ix < vx) || (ix + iw) > (vx + vw) || (iy + ih) > (vy + vh))
          *y = iy - ih;
        else if (iy < vy)
          *y = iy + ih;
     }
   else
     {
        //TODO: Enhance it later.
        if ((iy < vy) || (ix + iw) > (vx + vw) || (iy + ih) > (vy + vh))
          *x = ix - iw;
        else if (ix < vx)
          *x = ix + iw;
     }
}

static void
_elm_toolbar_focus_highlight_geometry_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);
   Eina_Bool is_next = va_arg(*list, int);

   Elm_Toolbar_Smart_Data *sd = _pd;

   if (is_next)
     {
        if (sd->focused_item)
          {
             _elm_toolbar_coordinates_adjust
                (sd->focused_item, x, y, w, h);
             elm_widget_focus_highlight_focus_part_geometry_get
                (VIEW(sd->focused_item), x, y, w, h);
          }
     }
   else
     {
        if (sd->prev_focused_item)
          {
             _elm_toolbar_coordinates_adjust
                (sd->prev_focused_item, x, y, w, h);
             elm_widget_focus_highlight_focus_part_geometry_get
                (VIEW(sd->prev_focused_item), x, y, w, h);
          }
     }
}

EAPI Evas_Object *
elm_toolbar_add(Evas_Object *parent)
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

EAPI void
elm_toolbar_icon_size_set(Evas_Object *obj,
                          int icon_size)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_icon_size_set(icon_size));
}

static void
_icon_size_set(Eo *obj, void *_pd, va_list *list)
{
   int icon_size = va_arg(*list, int);
   Elm_Toolbar_Smart_Data *sd = _pd;

   if (sd->priv_icon_size == icon_size) return;
   sd->priv_icon_size = icon_size;

   if (sd->priv_icon_size) sd->icon_size = sd->priv_icon_size;
   else sd->icon_size = sd->theme_icon_size;

   eo_do(obj, elm_obj_widget_theme_apply(NULL));
}

EAPI int
elm_toolbar_icon_size_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_obj_toolbar_icon_size_get(&ret));
   return ret;
}

static void
_icon_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Toolbar_Smart_Data *sd = _pd;
   *ret = sd->priv_icon_size;
}

EAPI Elm_Object_Item *
elm_toolbar_item_append(Evas_Object *obj,
                        const char *icon,
                        const char *label,
                        Evas_Smart_Cb func,
                        const void *data)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret;
   eo_do(obj, elm_obj_toolbar_item_append(icon, label, func, data, &ret));
   return ret;
}

static void
_item_append(Eo *obj, void *_pd, va_list *list)
{
   const char *icon = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   if (ret) *ret = NULL;

   Elm_Toolbar_Item *it;
   double scale;

   Elm_Toolbar_Smart_Data *sd = _pd;

   it = _item_new(obj, icon, label, func, data);
   if (!it) return;
   scale = (elm_widget_scale_get(obj) * elm_config_scale_get());

   sd->items = eina_inlist_append(sd->items, EINA_INLIST_GET(it));
   evas_object_box_append(sd->bx, VIEW(it));
   evas_object_show(VIEW(it));

   _item_theme_hook(obj, it, scale, sd->icon_size);
   _sizing_eval(obj);
   sd->item_count++;

   if (ret) *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_toolbar_item_prepend(Evas_Object *obj,
                         const char *icon,
                         const char *label,
                         Evas_Smart_Cb func,
                         const void *data)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret;
   eo_do(obj, elm_obj_toolbar_item_prepend(icon, label, func, data, &ret));
   return ret;
}

static void
_item_prepend(Eo *obj, void *_pd, va_list *list)
{
   const char *icon = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   if (ret) *ret = NULL;

   Elm_Toolbar_Item *it;
   double scale;

   Elm_Toolbar_Smart_Data *sd = _pd;

   it = _item_new(obj, icon, label, func, data);
   if (!it) return;
   scale = (elm_widget_scale_get(obj) * elm_config_scale_get());

   sd->items = eina_inlist_prepend(sd->items, EINA_INLIST_GET(it));
   evas_object_box_prepend(sd->bx, VIEW(it));
   evas_object_show(VIEW(it));
   _item_theme_hook(obj, it, scale, sd->icon_size);
   _sizing_eval(obj);
   sd->item_count++;

   if (ret) *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_toolbar_item_insert_before(Evas_Object *obj,
                               Elm_Object_Item *before,
                               const char *icon,
                               const char *label,
                               Evas_Smart_Cb func,
                               const void *data)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_toolbar_item_insert_before(before, icon, label, func, data, &ret));
   return ret;
}

static void
_item_insert_before(Eo *obj, void *_pd, va_list *list)
{
   Elm_Object_Item *before = va_arg(*list, Elm_Object_Item *);
   const char *icon = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);

   if (ret) *ret = NULL;

   Elm_Toolbar_Item *it, *_before;
   double scale;

   ELM_TOOLBAR_ITEM_CHECK(before);
   Elm_Toolbar_Smart_Data *sd = _pd;

   _before = (Elm_Toolbar_Item *)before;
   it = _item_new(obj, icon, label, func, data);
   if (!it) return;
   scale = (elm_widget_scale_get(obj) * elm_config_scale_get());

   sd->items = eina_inlist_prepend_relative
       (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(_before));
   evas_object_box_insert_before(sd->bx, VIEW(it), VIEW(_before));
   evas_object_show(VIEW(it));
   _item_theme_hook(obj, it, scale, sd->icon_size);
   _sizing_eval(obj);
   sd->item_count++;

   if (ret) *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_toolbar_item_insert_after(Evas_Object *obj,
                              Elm_Object_Item *after,
                              const char *icon,
                              const char *label,
                              Evas_Smart_Cb func,
                              const void *data)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do(obj, elm_obj_toolbar_item_insert_after(after, icon, label, func, data, &ret));
   return ret;
}

static void
_item_insert_after(Eo *obj, void *_pd, va_list *list)
{
   Elm_Object_Item *after = va_arg(*list, Elm_Object_Item *);
   const char *icon = va_arg(*list, const char *);
   const char *label = va_arg(*list, const char *);
   Evas_Smart_Cb func = va_arg(*list, Evas_Smart_Cb);
   const void *data = va_arg(*list, const void *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   if (ret) *ret = NULL;

   Elm_Toolbar_Item *it, *_after;
   double scale;

   ELM_TOOLBAR_ITEM_CHECK(after);

   Elm_Toolbar_Smart_Data *sd = _pd;
   _after = (Elm_Toolbar_Item *)after;
   it = _item_new(obj, icon, label, func, data);
   if (!it) return;
   scale = (elm_widget_scale_get(obj) * elm_config_scale_get());

   sd->items = eina_inlist_append_relative
       (sd->items, EINA_INLIST_GET(it), EINA_INLIST_GET(_after));
   evas_object_box_insert_after(sd->bx, VIEW(it), VIEW(_after));
   evas_object_show(VIEW(it));
   _item_theme_hook(obj, it, scale, sd->icon_size);
   _sizing_eval(obj);
   sd->item_count++;

   if (ret) *ret = (Elm_Object_Item *)it;
}

EAPI Elm_Object_Item *
elm_toolbar_first_item_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_toolbar_first_item_get(&ret));
   return ret;
}

static void
_first_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;
   Elm_Toolbar_Smart_Data *sd = _pd;

   if (!sd->items) return;
   *ret = (Elm_Object_Item *)ELM_TOOLBAR_ITEM_FROM_INLIST(sd->items);
}

EAPI Elm_Object_Item *
elm_toolbar_last_item_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_toolbar_last_item_get(&ret));
   return ret;
}

static void
_last_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;
   Elm_Toolbar_Smart_Data *sd = _pd;

   if (!sd->items) return;

   *ret = (Elm_Object_Item *)ELM_TOOLBAR_ITEM_FROM_INLIST(sd->items->last);
}

EAPI Elm_Object_Item *
elm_toolbar_item_next_get(const Elm_Object_Item *it)
{
   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   return (Elm_Object_Item *)ELM_TOOLBAR_ITEM_FROM_INLIST(
            EINA_INLIST_GET(((Elm_Toolbar_Item *)it))->next);
}

EAPI Elm_Object_Item *
elm_toolbar_item_prev_get(const Elm_Object_Item *it)
{
   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   return (Elm_Object_Item *)ELM_TOOLBAR_ITEM_FROM_INLIST(
            EINA_INLIST_GET(((Elm_Toolbar_Item *)it))->prev);
}

EAPI void
elm_toolbar_item_priority_set(Elm_Object_Item *it,
                              int priority)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it);

   if (item->prio.priority == priority) return;
   item->prio.priority = priority;
   _resize_cb(WIDGET(item), NULL, NULL, NULL);
}

EAPI int
elm_toolbar_item_priority_get(const Elm_Object_Item *it)
{
   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, 0);

   return ((Elm_Toolbar_Item *)it)->prio.priority;
}

EAPI Elm_Object_Item *
elm_toolbar_item_find_by_label(const Evas_Object *obj,
                               const char *label)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_toolbar_item_find_by_label(label, &ret));
   return ret;
}

static void
_item_find_by_label(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *label = va_arg(*list, const char *);
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   *ret = NULL;
   Elm_Toolbar_Item *it;

   Elm_Toolbar_Smart_Data *sd = _pd;

   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (!strcmp(it->label, label))
          {
             *ret = (Elm_Object_Item *)it;
             return;
          }
     }
}

EAPI void
elm_toolbar_item_selected_set(Elm_Object_Item *it,
                              Eina_Bool selected)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it);

   if (item->selected == selected) return;
   if (selected) _item_select(item);
   else _item_unselect(item);
}

EAPI Eina_Bool
elm_toolbar_item_selected_get(const Elm_Object_Item *it)
{
   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_Toolbar_Item *)it)->selected;
}

EAPI Elm_Object_Item *
elm_toolbar_selected_item_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_toolbar_selected_item_get(&ret));
   return ret;
}

static void
_selected_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = (Elm_Object_Item *)sd->selected_item;
}

EAPI Elm_Object_Item *
elm_toolbar_more_item_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Elm_Object_Item *ret = NULL;
   eo_do((Eo *) obj, elm_obj_toolbar_more_item_get(&ret));
   return ret;
}

static void
_more_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = (Elm_Object_Item *)sd->more_item;
}

EAPI void
elm_toolbar_item_icon_set(Elm_Object_Item *it,
                          const char *icon)
{
   Evas_Object *obj;
   Evas_Object *icon_obj;
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it);

   obj = WIDGET(item);
   ELM_TOOLBAR_DATA_GET(obj, sd);
   if ((icon) && (item->icon_str) && (!strcmp(icon, item->icon_str))) return;

   icon_obj = elm_icon_add(obj);
   if (!icon_obj) return;
   if (_item_icon_set(icon_obj, "toolbar/", icon))
     _elm_toolbar_item_icon_obj_set
       (obj, item, icon_obj, icon, sd->icon_size, "elm,state,icon_set");
   else
     {
        _elm_toolbar_item_icon_obj_set
          (obj, item, NULL, NULL, 0, "elm,state,icon_set");
        evas_object_del(icon_obj);
     }
}

EAPI const char *
elm_toolbar_item_icon_get(const Elm_Object_Item *it)
{
   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   return ((Elm_Toolbar_Item *)it)->icon_str;
}

EAPI Evas_Object *
elm_toolbar_item_object_get(const Elm_Object_Item *it)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   return VIEW(item);
}

EAPI Evas_Object *
elm_toolbar_item_icon_object_get(Elm_Object_Item *it)
{
   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   return ((Elm_Toolbar_Item *)it)->icon;
}

EAPI Eina_Bool
elm_toolbar_item_icon_memfile_set(Elm_Object_Item *it,
                                  const void *img,
                                  size_t size,
                                  const char *format,
                                  const char *key)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;
   Evas_Object *icon_obj;
   Evas_Object *obj;
   Eina_Bool ret;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   obj = WIDGET(item);
   ELM_TOOLBAR_DATA_GET(obj, sd);

   if (img && size)
     {
        icon_obj = elm_icon_add(obj);
        evas_object_repeat_events_set(icon_obj, EINA_TRUE);
        ret = elm_image_memfile_set(icon_obj, img, size, format, key);
        if (!ret)
          {
             evas_object_del(icon_obj);
             return EINA_FALSE;
          }
        _elm_toolbar_item_icon_obj_set
          (obj, item, icon_obj, NULL, sd->icon_size, "elm,state,icon_set");
     }
   else
     _elm_toolbar_item_icon_obj_set
       (obj, item, NULL, NULL, 0, "elm,state,icon_set");
   return EINA_TRUE;
}

EAPI Eina_Bool
elm_toolbar_item_icon_file_set(Elm_Object_Item *it,
                               const char *file,
                               const char *key)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;
   Evas_Object *icon_obj;
   Evas_Object *obj;
   Eina_Bool ret;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   obj = WIDGET(item);
   ELM_TOOLBAR_DATA_GET(obj, sd);

   if (file)
     {
        icon_obj = elm_icon_add(obj);
        evas_object_repeat_events_set(icon_obj, EINA_TRUE);
        ret = elm_image_file_set(icon_obj, file, key);
        if (!ret)
          {
             evas_object_del(icon_obj);
             return EINA_FALSE;
          }
        _elm_toolbar_item_icon_obj_set
          (obj, item, icon_obj, NULL, sd->icon_size, "elm,state,icon_set");
     }
   else
     _elm_toolbar_item_icon_obj_set
       (obj, item, NULL, NULL, 0, "elm,state,icon_set");
   return EINA_TRUE;
}

EAPI void
elm_toolbar_item_separator_set(Elm_Object_Item *it,
                               Eina_Bool separator)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;
   Evas_Object *obj = WIDGET(item);
   double scale;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it);
   ELM_TOOLBAR_DATA_GET(obj, sd);

   if (item->separator == separator) return;
   item->separator = separator;
   scale = (elm_widget_scale_get(obj) * elm_config_scale_get());
   _item_theme_hook(obj, item, scale, sd->icon_size);
   evas_object_size_hint_min_set(VIEW(item), -1, -1);
   if (separator) sd->separator_count++;
   else sd->separator_count--;
}

EAPI Eina_Bool
elm_toolbar_item_separator_get(const Elm_Object_Item *it)
{
   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   return ((Elm_Toolbar_Item *)it)->separator;
}

EAPI void
elm_toolbar_shrink_mode_set(Evas_Object *obj,
                            Elm_Toolbar_Shrink_Mode shrink_mode)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_shrink_mode_set(shrink_mode));
}

static void
_shrink_mode_set(Eo *obj, void *_pd, va_list *list)
{
   Elm_Toolbar_Shrink_Mode shrink_mode = va_arg(*list, Elm_Toolbar_Shrink_Mode);
   Eina_Bool bounce;

   Elm_Toolbar_Smart_Data *sd = _pd;

   if (sd->shrink_mode == shrink_mode) return;
   sd->shrink_mode = shrink_mode;
   bounce = (_elm_config->thumbscroll_bounce_enable) &&
     (shrink_mode == ELM_TOOLBAR_SHRINK_SCROLL);
   eo_do(obj, elm_interface_scrollable_bounce_allow_set(bounce, EINA_FALSE));

   if (sd->more_item)
     {
        elm_widget_item_del(sd->more_item);
        sd->more_item = NULL;
     }

   if (shrink_mode == ELM_TOOLBAR_SHRINK_MENU)
     {
        elm_toolbar_homogeneous_set(obj, EINA_FALSE);
        eo_do(obj, elm_interface_scrollable_policy_set
              (ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF));
        sd->more_item = _item_new(obj, "more_menu", "More", NULL, NULL);
     }
   else if (shrink_mode == ELM_TOOLBAR_SHRINK_HIDE)
     {
        elm_toolbar_homogeneous_set(obj, EINA_FALSE);
        eo_do(obj, elm_interface_scrollable_policy_set
              (ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF));
     }
   else if (shrink_mode == ELM_TOOLBAR_SHRINK_EXPAND)
     {
        elm_toolbar_homogeneous_set(obj, EINA_FALSE);
        eo_do(obj, elm_interface_scrollable_policy_set
              (ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF));
        sd->more_item = _item_new(obj, "more_menu", "More", NULL, NULL);
     }
   else
      eo_do(obj, elm_interface_scrollable_policy_set
            (ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_OFF));

   _sizing_eval(obj);
}

EAPI Elm_Toolbar_Shrink_Mode
elm_toolbar_shrink_mode_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) ELM_TOOLBAR_SHRINK_NONE;
   Elm_Toolbar_Shrink_Mode ret = ELM_TOOLBAR_SHRINK_NONE;
   eo_do((Eo *) obj, elm_obj_toolbar_shrink_mode_get(&ret));
   return ret;
}

static void
_shrink_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Toolbar_Shrink_Mode *ret = va_arg(*list, Elm_Toolbar_Shrink_Mode *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = sd->shrink_mode;
}

EAPI void
elm_toolbar_transverse_expanded_set(Evas_Object *obj, Eina_Bool transverse_expanded)
{
   ELM_TOOLBAR_CHECK(obj);
   ELM_TOOLBAR_DATA_GET(obj, sd);

   if (sd->transverse_expanded == transverse_expanded) return;
   sd->transverse_expanded = transverse_expanded;

   _sizing_eval(obj);
}

EAPI Eina_Bool
elm_toolbar_transverse_expanded_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) EINA_FALSE;
   ELM_TOOLBAR_DATA_GET(obj, sd);

   return sd->transverse_expanded;
}

EAPI void
elm_toolbar_homogeneous_set(Evas_Object *obj,
                            Eina_Bool homogeneous)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_homogeneous_set(homogeneous));
}

static void
_homogeneous_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool homogeneous = va_arg(*list, int);
   Elm_Toolbar_Smart_Data *sd = _pd;

   homogeneous = !!homogeneous;
   if (homogeneous == sd->homogeneous) return;
   sd->homogeneous = homogeneous;
   if (homogeneous) elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_NONE);
   evas_object_smart_calculate(sd->bx);
}

EAPI Eina_Bool
elm_toolbar_homogeneous_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_toolbar_homogeneous_get(&ret));
   return ret;
}

static void
_homogeneous_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = sd->homogeneous;
}

EAPI void
elm_toolbar_menu_parent_set(Evas_Object *obj,
                            Evas_Object *parent)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_menu_parent_set(parent));
}

static void
_menu_parent_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *parent = va_arg(*list, Evas_Object *);
   Elm_Toolbar_Item *it;

   Elm_Toolbar_Smart_Data *sd = _pd;
   EINA_SAFETY_ON_NULL_RETURN(parent);

   sd->menu_parent = parent;
   EINA_INLIST_FOREACH(sd->items, it)
     {
        if (it->o_menu)
          elm_menu_parent_set(it->o_menu, sd->menu_parent);
     }
   if ((sd->more_item) && (sd->more_item->o_menu))
     elm_menu_parent_set(sd->more_item->o_menu, sd->menu_parent);
}

EAPI Evas_Object *
elm_toolbar_menu_parent_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_toolbar_menu_parent_get(&ret));
   return ret;
}

static void
_menu_parent_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = sd->menu_parent;
}

EAPI void
elm_toolbar_align_set(Evas_Object *obj,
                      double align)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_align_set(align));
}

static void
_align_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double align = va_arg(*list, double);
   Elm_Toolbar_Smart_Data *sd = _pd;

   if (sd->vertical)
     {
        if (sd->align != align)
          evas_object_size_hint_align_set(sd->bx, 0.5, align);
     }
   else
     {
        if (sd->align != align)
          evas_object_size_hint_align_set(sd->bx, align, 0.5);
     }
   sd->align = align;
}

EAPI double
elm_toolbar_align_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_toolbar_align_get(&ret));
   return ret;
}

static void
_align_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = sd->align;
}

EAPI void
elm_toolbar_item_menu_set(Elm_Object_Item *it,
                          Eina_Bool menu)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it);
   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   if (item->menu == menu) return;
   if (menu) _item_menu_create(sd, item);
   else _item_menu_destroy(item);
}

EAPI Evas_Object *
elm_toolbar_item_menu_get(const Elm_Object_Item *it)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   if (!item->menu) return NULL;
   return item->o_menu;
}

EAPI Elm_Toolbar_Item_State *
elm_toolbar_item_state_add(Elm_Object_Item *it,
                           const char *icon,
                           const char *label,
                           Evas_Smart_Cb func,
                           const void *data)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;
   Elm_Toolbar_Item_State *it_state;
   Evas_Object *icon_obj;
   Evas_Object *obj;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   obj = WIDGET(item);
   ELM_TOOLBAR_DATA_GET(WIDGET(item), sd);

   if (!item->states)
     {
        it_state = _item_state_new
            (item->label, item->icon_str, item->icon, item->func,
            item->base.data);
        item->states = eina_list_append(item->states, it_state);
        item->current_state = item->states;
     }

   icon_obj = elm_icon_add(obj);
   elm_icon_order_lookup_set(icon_obj, sd->lookup_order);
   if (!icon_obj) goto error_state_add;

   if (!_item_icon_set(icon_obj, "toolbar/", icon))
     {
        ELM_SAFE_FREE(icon_obj, evas_object_del);
        icon = NULL;
     }

   it_state = _item_state_new(label, icon, icon_obj, func, data);
   item->states = eina_list_append(item->states, it_state);
   item->func = _elm_toolbar_item_state_cb;
   item->base.data = NULL;

   return it_state;

error_state_add:
   if (item->states && !eina_list_next(item->states))
     {
        eina_stringshare_del(item->label);
        eina_stringshare_del(item->icon_str);
        free(eina_list_data_get(item->states));
        item->states = eina_list_free(item->states);
     }
   return NULL;
}

EAPI Eina_Bool
elm_toolbar_item_state_del(Elm_Object_Item *it,
                           Elm_Toolbar_Item_State *state)
{
   Elm_Toolbar_Item_State *it_state;
   Elm_Toolbar_Item *item;
   Eina_List *del_state;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   if (!state) return EINA_FALSE;

   item = (Elm_Toolbar_Item *)it;
   if (!item->states) return EINA_FALSE;

   del_state = eina_list_data_find_list(item->states, state);
   if (del_state == item->states) return EINA_FALSE;
   if (del_state == item->current_state)
     elm_toolbar_item_state_unset(it);

   eina_stringshare_del(state->label);
   eina_stringshare_del(state->icon_str);
   evas_object_del(state->icon);
   free(state);

   item->states = eina_list_remove_list(item->states, del_state);
   if (item->states && !eina_list_next(item->states))
     {
        it_state = eina_list_data_get(item->states);
        item->base.data = it_state->data;
        item->func = it_state->func;
        eina_stringshare_del(it_state->label);
        eina_stringshare_del(it_state->icon_str);
        free(eina_list_data_get(item->states));
        item->states = eina_list_free(item->states);
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_toolbar_item_state_set(Elm_Object_Item *it,
                           Elm_Toolbar_Item_State *state)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;
   Elm_Toolbar_Item_State *it_state;
   Eina_List *next_state;
   Evas_Object *obj;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);

   obj = WIDGET(item);
   ELM_TOOLBAR_DATA_GET(obj, sd);
   if (!item->states) return EINA_FALSE;

   if (state)
     {
        next_state = eina_list_data_find_list(item->states, state);
        if (!next_state) return EINA_FALSE;
     }
   else
     next_state = item->states;

   if (next_state == item->current_state) return EINA_TRUE;

   it_state = eina_list_data_get(next_state);
   if (eina_list_data_find(item->current_state, state))
     {
        _item_label_set(item, it_state->label, "elm,state,label_set,forward");
        _elm_toolbar_item_icon_obj_set
          (obj, item, it_state->icon, it_state->icon_str,
          sd->icon_size, "elm,state,icon_set,forward");
     }
   else
     {
        _item_label_set(item, it_state->label, "elm,state,label_set,backward");
        _elm_toolbar_item_icon_obj_set
          (obj, item, it_state->icon, it_state->icon_str,
          sd->icon_size, "elm,state,icon_set,backward");
     }
   if (item->icon)
     {
        if (elm_widget_item_disabled_get(item))
          elm_widget_signal_emit(item->icon, "elm,state,disabled", "elm");
        else
          elm_widget_signal_emit(item->icon, "elm,state,enabled", "elm");
     }

   item->current_state = next_state;

   return EINA_TRUE;
}

EAPI void
elm_toolbar_item_state_unset(Elm_Object_Item *it)
{
   elm_toolbar_item_state_set(it, NULL);
}

EAPI Elm_Toolbar_Item_State *
elm_toolbar_item_state_get(const Elm_Object_Item *it)
{
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   if ((!item->states) || (!item->current_state)) return NULL;
   if (item->current_state == item->states) return NULL;

   return eina_list_data_get(item->current_state);
}

EAPI Elm_Toolbar_Item_State *
elm_toolbar_item_state_next(Elm_Object_Item *it)
{
   Eina_List *next_state;
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   if (!item->states) return NULL;

   next_state = eina_list_next(item->current_state);
   if (!next_state)
     next_state = eina_list_next(item->states);
   return eina_list_data_get(next_state);
}

EAPI Elm_Toolbar_Item_State *
elm_toolbar_item_state_prev(Elm_Object_Item *it)
{
   Eina_List *prev_state;
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it, NULL);

   if (!item->states) return NULL;

   prev_state = eina_list_prev(item->current_state);
   if ((!prev_state) || (prev_state == item->states))
     prev_state = eina_list_last(item->states);
   return eina_list_data_get(prev_state);
}

EAPI void
elm_toolbar_icon_order_lookup_set(Evas_Object *obj,
                                  Elm_Icon_Lookup_Order order)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_icon_order_lookup_set(order));
}

static void
_icon_order_lookup_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Icon_Lookup_Order order = va_arg(*list, Elm_Icon_Lookup_Order);
   Elm_Toolbar_Item *it;

   Elm_Toolbar_Smart_Data *sd = _pd;

   if (sd->lookup_order == order) return;
   sd->lookup_order = order;
   EINA_INLIST_FOREACH(sd->items, it)
     elm_icon_order_lookup_set(it->icon, order);
   if (sd->more_item)
     elm_icon_order_lookup_set(sd->more_item->icon, order);
}

EAPI Elm_Icon_Lookup_Order
elm_toolbar_icon_order_lookup_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) ELM_ICON_LOOKUP_THEME_FDO;
   Elm_Icon_Lookup_Order ret = ELM_ICON_LOOKUP_THEME_FDO;
   eo_do((Eo *) obj, elm_obj_toolbar_icon_order_lookup_get(&ret));
   return ret;
}

static void
_icon_order_lookup_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Icon_Lookup_Order *ret = va_arg(*list, Elm_Icon_Lookup_Order *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = sd->lookup_order;
}

EAPI void
elm_toolbar_horizontal_set(Evas_Object *obj,
                           Eina_Bool horizontal)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_horizontal_set(horizontal));
}

static void
_horizontal_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool horizontal = va_arg(*list, int);
   Elm_Toolbar_Smart_Data *sd = _pd;

   horizontal = !!horizontal;
   if (!horizontal == sd->vertical) return;
   sd->vertical = !horizontal;
   if (sd->vertical)
     evas_object_size_hint_align_set(sd->bx, 0.5, sd->align);
   else
     evas_object_size_hint_align_set(sd->bx, sd->align, 0.5);

   _sizing_eval(obj);
}

EAPI Eina_Bool
elm_toolbar_horizontal_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_toolbar_horizontal_get(&ret));
   return ret;
}

static void
_horizontal_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = !sd->vertical;
}

EAPI unsigned int
elm_toolbar_items_count(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) 0;
   unsigned int ret = 0;
   eo_do((Eo *) obj, elm_obj_toolbar_items_count(&ret));
   return ret;
}

static void
_items_count(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   unsigned int *ret = va_arg(*list, unsigned int *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = sd->item_count;
}

EAPI void
elm_toolbar_standard_priority_set(Evas_Object *obj,
                                  int priority)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_standard_priority_set(priority));
}

static void
_standard_priority_set(Eo *obj, void *_pd, va_list *list)
{
   int priority = va_arg(*list, int);
   Elm_Toolbar_Smart_Data *sd = _pd;

   if (sd->standard_priority == priority) return;
   sd->standard_priority = priority;
   _resize_cb(obj, NULL, NULL, NULL);
}

EAPI int
elm_toolbar_standard_priority_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_obj_toolbar_standard_priority_get(&ret));
   return ret;
}

static void
_standard_priority_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = sd->standard_priority;
}

EAPI void
elm_toolbar_select_mode_set(Evas_Object *obj,
                            Elm_Object_Select_Mode mode)
{
   ELM_TOOLBAR_CHECK(obj);
   eo_do(obj, elm_obj_toolbar_select_mode_set(mode));
}

static void
_select_mode_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Select_Mode mode = va_arg(*list, Elm_Object_Select_Mode);
   Elm_Toolbar_Smart_Data *sd = _pd;

   if (mode >= ELM_OBJECT_SELECT_MODE_MAX)
     return;

   if (sd->select_mode == mode) return;

   if ((mode == ELM_OBJECT_SELECT_MODE_ALWAYS) &&
       (sd->select_mode != ELM_OBJECT_SELECT_MODE_ALWAYS) &&
       sd->items)
     _item_select(ELM_TOOLBAR_ITEM_FROM_INLIST(sd->items));

   if (sd->select_mode != mode)
     sd->select_mode = mode;
}

EAPI Elm_Object_Select_Mode
elm_toolbar_select_mode_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) ELM_OBJECT_SELECT_MODE_MAX;
   Elm_Object_Select_Mode ret = ELM_OBJECT_SELECT_MODE_MAX;
   eo_do((Eo *) obj, elm_obj_toolbar_select_mode_get(&ret));
   return ret;
}

static void
_select_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Select_Mode *ret = va_arg(*list, Elm_Object_Select_Mode *);
   Elm_Toolbar_Smart_Data *sd = _pd;

   *ret = sd->select_mode;
}

EAPI void
elm_toolbar_reorder_mode_set(Evas_Object *obj,
                             Eina_Bool    reorder_mode)
{
   ELM_TOOLBAR_CHECK(obj);
   ELM_TOOLBAR_DATA_GET(obj, sd);

   sd->reorder_mode = !!reorder_mode;
}

EAPI Eina_Bool
elm_toolbar_reorder_mode_get(const Evas_Object *obj)
{
   ELM_TOOLBAR_CHECK(obj) EINA_FALSE;
   ELM_TOOLBAR_DATA_GET(obj, sd);

   return sd->reorder_mode;
}

EAPI void
elm_toolbar_item_show(Elm_Object_Item *it, Elm_Toolbar_Item_Scrollto_Type type)
{
   Evas_Coord x, y, w, h;
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it);

   if (_elm_toolbar_item_coordinates_calc(item, type, &x, &y, &w, &h))
     eo_do(WIDGET(item), elm_interface_scrollable_content_region_show
     (x, y, w, h));
}

EAPI void
elm_toolbar_item_bring_in(Elm_Object_Item *it, Elm_Toolbar_Item_Scrollto_Type type)
{
   Evas_Coord x, y, w, h;
   Elm_Toolbar_Item *item = (Elm_Toolbar_Item *)it;

   ELM_TOOLBAR_ITEM_CHECK_OR_RETURN(it);

   if (_elm_toolbar_item_coordinates_calc(item, type, &x, &y, &w, &h))
     eo_do(WIDGET(item), elm_interface_scrollable_region_bring_in
     (x, y, w, h));
}

static void
_elm_toolbar_focused_item_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Object_Item **ret = va_arg(*list, Elm_Object_Item **);
   Elm_Toolbar_Smart_Data *sd = _pd;

   if (ret) *ret = (Elm_Object_Item *)sd->focused_item;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), _elm_toolbar_smart_member_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_toolbar_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_toolbar_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_toolbar_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_toolbar_smart_move),

        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ON_FOCUS), _elm_toolbar_smart_on_focus),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_THEME_APPLY), _elm_toolbar_smart_theme),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_TRANSLATE), _elm_toolbar_smart_translate),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_EVENT), _elm_toolbar_smart_event),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_toolbar_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_NEXT), _elm_toolbar_smart_focus_next),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_ACCESS), _elm_toolbar_smart_access),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUS_HIGHLIGHT_GEOMETRY_GET), _elm_toolbar_focus_highlight_geometry_get),
        EO_OP_FUNC(ELM_OBJ_WIDGET_ID(ELM_OBJ_WIDGET_SUB_ID_FOCUSED_ITEM_GET), _elm_toolbar_focused_item_get),

        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_SET), _icon_size_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_GET), _icon_size_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_APPEND), _item_append),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_PREPEND), _item_prepend),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_BEFORE), _item_insert_before),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_AFTER), _item_insert_after),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_FIRST_ITEM_GET), _first_item_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_LAST_ITEM_GET), _last_item_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_FIND_BY_LABEL), _item_find_by_label),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECTED_ITEM_GET), _selected_item_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MORE_ITEM_GET), _more_item_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_SET), _shrink_mode_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_GET), _shrink_mode_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_SET), _homogeneous_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_GET), _homogeneous_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_SET), _menu_parent_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_GET), _menu_parent_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_SET), _align_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_GET), _align_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_SET), _icon_order_lookup_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_GET), _icon_order_lookup_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_SET), _horizontal_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_GET), _horizontal_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_ITEMS_COUNT), _items_count),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_SET), _standard_priority_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_GET), _standard_priority_get),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_SET), _select_mode_set),
        EO_OP_FUNC(ELM_OBJ_TOOLBAR_ID(ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_GET), _select_mode_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_SET, "Set the icon size, in pixels, to be used by toolbar items."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ICON_SIZE_GET, "Get the icon size, in pixels, to be used by toolbar items."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_APPEND, "Append item to the toolbar."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_PREPEND, "Prepend item to the toolbar."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_BEFORE, "Insert a new item into the toolbar object before item before."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_INSERT_AFTER, "Insert a new item into the toolbar object after item after."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_FIRST_ITEM_GET, "Get the first item in the given toolbar widget's list of items."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_LAST_ITEM_GET, "Get the last item in the given toolbar widget's list of items."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ITEM_FIND_BY_LABEL, "Returns a pointer to a toolbar item by its label."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_SELECTED_ITEM_GET, "Get the selected item."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_MORE_ITEM_GET, "Get the more item."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_SET, "Set the item displaying mode of a given toolbar widget obj."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_SHRINK_MODE_GET, "Get the shrink mode of toolbar obj."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_SET, "Enable/disable homogeneous mode."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_HOMOGENEOUS_GET, "Get whether the homogeneous mode is enabled."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_SET, "Set the parent object of the toolbar items' menus."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_MENU_PARENT_GET, "Get the parent object of the toolbar items' menus."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_SET, "Set the alignment of the items."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ALIGN_GET, "Get the alignment of the items."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_SET, "Sets icon lookup order, for toolbar items' icons."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ICON_ORDER_LOOKUP_GET, "Get the icon lookup order."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_SET, "Change a toolbar's orientation."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_HORIZONTAL_GET, "Get a toolbar's orientation."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_ITEMS_COUNT, "Get the number of items in a toolbar."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_SET, "Set the standard priority of visible items in a toolbar."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_STANDARD_PRIORITY_GET, "Get the standard_priority of visible items in a toolbar."),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_SET, "Set the toolbar select mode"),
     EO_OP_DESCRIPTION(ELM_OBJ_TOOLBAR_SUB_ID_SELECT_MODE_GET, "Get the toolbar select mode"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_TOOLBAR_BASE_ID, op_desc, ELM_OBJ_TOOLBAR_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Toolbar_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_toolbar_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, ELM_INTERFACE_SCROLLABLE_CLASS, NULL);
