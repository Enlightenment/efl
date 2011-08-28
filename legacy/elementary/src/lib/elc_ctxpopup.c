#include <Elementary.h>
#include "elm_priv.h"

typedef struct _Widget_Data Widget_Data;
typedef struct _Elm_Ctxpopup_Item Elm_Ctxpopup_Item;

struct _Elm_Ctxpopup_Item
{
   Elm_Widget_Item base;
   const char *label;
   Evas_Object *icon;
   Evas_Smart_Cb func;
   Eina_Bool disabled:1;
};

struct _Widget_Data
{
   Evas_Object *parent;
   Evas_Object *base;
   Evas_Object *content;
   Evas_Object *box;
   Evas_Object *arrow;
   Evas_Object *scr;
   Evas_Object *bg;
   Evas_Object *hover_parent;
   Eina_List *items;
   Elm_Ctxpopup_Direction dir;
   Elm_Ctxpopup_Direction dir_priority[4];
   Evas_Coord max_sc_w, max_sc_h;
   Eina_Bool horizontal:1;
   Eina_Bool visible:1;
   Eina_Bool finished:1;
};

static const char *widtype = NULL;

static void _freeze_on(void *data, Evas_Object *obj, void *event_info);
static void _freeze_off(void *data, Evas_Object *obj, void *event_info);
static void _hold_on(void *data, Evas_Object *obj, void *event_info);
static void _hold_off(void *data, Evas_Object *obj, void *event_info);
static void _scroller_size_reset(Widget_Data *wd);
static void _hover_parent_callbacks_del(Evas_Object *obj);
static void _hover_parent_resize(void *data, Evas *e,
                                 Evas_Object *obj,
                                 void *event_info);
static void _hover_parent_move(void *data, Evas *e,
                               Evas_Object *obj,
                               void *event_info);
static void _hover_parent_del(void *data, Evas *e,
                              Evas_Object *obj,
                              void *event_info);
static void _item_sizing_eval(Elm_Ctxpopup_Item *item);
static void _adjust_pos_x(Evas_Coord_Point *pos,
                          Evas_Coord_Point *base_size,
                          Evas_Coord_Rectangle *hover_area);
static void _adjust_pos_y(Evas_Coord_Point *pos,
                          Evas_Coord_Point *base_size,
                          Evas_Coord_Rectangle *hover_area);
static void _ctxpopup_changed_size_hints(void *data,
                                         Evas *e,
                                         Evas_Object *obj,
                                         void *event_info);
static Elm_Ctxpopup_Direction _calc_base_geometry(Evas_Object *obj,
                                                  Evas_Coord_Rectangle *rect);
static void _update_arrow(Evas_Object *obj, Elm_Ctxpopup_Direction dir);
static void _sizing_eval(Evas_Object *obj);
static void _shift_base_by_arrow(Evas_Object *arrow,
                                 Elm_Ctxpopup_Direction dir,
                                 Evas_Coord_Rectangle *rect);
static void _del_pre_hook(Evas_Object *obj);
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);
static void _content_set_hook(Evas_Object *obj,
                              const char *part,
                              Evas_Object *content);
static Evas_Object * _content_unset_hook(Evas_Object *obj,
                                         const char *part__);
static Evas_Object * _content_get_hook(const Evas_Object *obj,
                                       const char *part);
static void _bg_clicked_cb(void *data, Evas_Object *obj,
                           const char *emission,
                           const char *source);
static void _parent_resize(void *data, Evas *e, Evas_Object *obj,
                           void *event_info);
static void _ctxpopup_show(void *data,
                           Evas *e,
                           Evas_Object *obj,
                           void *event_info);
static void _hide(Evas_Object *obj);
static void _ctxpopup_hide(void *data,
                           Evas *e,
                           Evas_Object *obj,
                           void *event_info);
static void _scroller_resize(void *data,
                             Evas *e,
                             Evas_Object *obj,
                             void *event_info);
static void _ctxpopup_move(void *data,
                           Evas *e,
                           Evas_Object *obj,
                           void *event_info);
static void _item_select_cb(void *data, Evas_Object *obj,
                            const char *emission,
                            const char *source);
static void _item_icon_set(Elm_Ctxpopup_Item *item, Evas_Object *icon);
static void _item_label_set(Elm_Ctxpopup_Item *item, const char *label);
static void _item_new(Elm_Ctxpopup_Item *item, char *group_name);
static void _content_del(void *data,
                         Evas *e,
                         Evas_Object *obj,
                         void *event_info);
static void _list_del(Widget_Data *wd);
static void _list_new(Evas_Object *obj);
static void _remove_items(Widget_Data * wd);

static const char SIG_DISMISSED[] = "dismissed";

static const Evas_Smart_Cb_Description _signals[] = {
   {SIG_DISMISSED, ""},
   {NULL, NULL}
};

static void
_freeze_on(void *data __UNUSED__, Evas_Object *obj,
           void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->scr)) return;
   elm_object_scroll_freeze_push(wd->scr);
}

static void
_freeze_off(void *data __UNUSED__, Evas_Object *obj,
            void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->scr)) return;
   elm_object_scroll_freeze_pop(wd->scr);
}

static void
_hold_on(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->scr)) return;
   elm_object_scroll_hold_push(wd->scr);
}

static void
_hold_off(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->scr)) return;
   elm_object_scroll_hold_pop(wd->scr);
}

static void
_scroller_size_reset(Widget_Data *wd)
{
   wd->finished = EINA_FALSE;
   wd->max_sc_h = -1;
   wd->max_sc_w = -1;
}

static void
_hover_parent_callbacks_del(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if ((!wd) || (!wd->hover_parent))
     return;

   evas_object_event_callback_del_full(wd->hover_parent, EVAS_CALLBACK_DEL,
                                       _hover_parent_del, obj);
   evas_object_event_callback_del_full(wd->hover_parent, EVAS_CALLBACK_MOVE,
                                       _hover_parent_move, obj);
   evas_object_event_callback_del_full(wd->hover_parent, EVAS_CALLBACK_RESIZE,
                                       _hover_parent_resize, obj);
}

static void
_hover_parent_resize(void *data, Evas *e __UNUSED__,
                     Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;

   if (wd->visible)
     {
        _scroller_size_reset(wd);
        _sizing_eval(data);
     }
}

static void
_hover_parent_move(void *data, Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;

   if (wd->visible)
     {
        _scroller_size_reset(wd);
        _sizing_eval(obj);
     }
}

static void
_hover_parent_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Widget_Data *wd = elm_widget_data_get(data);

   if (!wd) return;

   wd->hover_parent = NULL;
}

static void
_item_sizing_eval(Elm_Ctxpopup_Item *item)
{
   Evas_Coord min_w = -1, min_h = -1, max_w = -1, max_h = -1;

   if (!item) return;

   edje_object_size_min_restricted_calc(item->base.view, &min_w, &min_h, min_w,
                                        min_h);
   evas_object_size_hint_min_set(item->base.view, min_w, min_h);
   evas_object_size_hint_max_set(item->base.view, max_w, max_h);
}

static void
_adjust_pos_x(Evas_Coord_Point *pos, Evas_Coord_Point *base_size,
              Evas_Coord_Rectangle *hover_area)
{
   pos->x -= (base_size->x / 2);

   if (pos->x < hover_area->x)
     pos->x = hover_area->x;
   else if ((pos->x + base_size->x) > (hover_area->x + hover_area->w))
     pos->x = (hover_area->x + hover_area->w) - base_size->x;

   if (base_size->x > hover_area->w)
     base_size->x -= (base_size->x - hover_area->w);

   if (pos->x < hover_area->x)
     pos->x = hover_area->x;
}

static void
_adjust_pos_y(Evas_Coord_Point *pos, Evas_Coord_Point *base_size,
              Evas_Coord_Rectangle *hover_area)
{
   pos->y -= (base_size->y / 2);

   if (pos->y < hover_area->y)
     pos->y = hover_area->y;
   else if ((pos->y + base_size->y) > (hover_area->y + hover_area->h))
     pos->y = hover_area->y + hover_area->h - base_size->y;

   if (base_size->y > hover_area->h)
     base_size->y -= (base_size->y - hover_area->h);

   if (pos->y < hover_area->y)
     pos->y = hover_area->y;
}

static void
_ctxpopup_changed_size_hints(void *data __UNUSED__, Evas *e __UNUSED__,
                             Evas_Object *obj, void *event_info __UNUSED__)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (wd->visible)
     _sizing_eval(obj);
}

static Elm_Ctxpopup_Direction
_calc_base_geometry(Evas_Object *obj, Evas_Coord_Rectangle *rect)
{
   Widget_Data *wd;
   Evas_Coord_Point pos = {0, 0};
   Evas_Coord_Point base_size;
   Evas_Coord_Point max_size;
   Evas_Coord_Point min_size;
   Evas_Coord_Rectangle hover_area;
   Evas_Coord_Rectangle parent_size;
   Evas_Coord_Point arrow_size;
   Elm_Ctxpopup_Direction dir = ELM_CTXPOPUP_DIRECTION_DOWN;
   Evas_Coord_Point temp;
   int idx;

   wd = elm_widget_data_get(obj);

   if ((!wd) || (!rect))
     return ELM_CTXPOPUP_DIRECTION_DOWN;

   edje_object_part_geometry_get(wd->arrow, "ctxpopup_arrow", NULL, NULL,
                                 &arrow_size.x, &arrow_size.y);
   evas_object_resize(wd->arrow, arrow_size.x, arrow_size.y);

   //Initialize Area Rectangle.
   if (wd->hover_parent)
     evas_object_geometry_get(wd->hover_parent, &hover_area.x, &hover_area.y,
                              &hover_area.w, &hover_area.h);
   else
     {
        evas_object_geometry_get(wd->parent, &parent_size.x, &parent_size.y,
                                 &parent_size.w, &parent_size.h);
        hover_area.x = parent_size.x;
        hover_area.y = parent_size.y;
        hover_area.w = parent_size.w;
        hover_area.h = parent_size.h;
     }

   evas_object_geometry_get(obj, &pos.x, &pos.y, NULL, NULL);

   //recalc the edje
   edje_object_size_min_calc(wd->base, &base_size.x, &base_size.y);
   evas_object_smart_calculate(wd->base);

   //Limit to Max Size
   evas_object_size_hint_max_get(obj, &max_size.x, &max_size.y);

   if ((max_size.y > 0) && (base_size.y > max_size.y))
     base_size.y = max_size.y;

   if ((max_size.x > 0) && (base_size.x > max_size.x))
     base_size.x = max_size.x;

   //Limit to Min Size
   evas_object_size_hint_min_get(obj, &min_size.x, &min_size.y);

   if ((min_size.y > 0) && (base_size.y < min_size.y))
     base_size.y = min_size.y;

   if ((min_size.x > 0) && (base_size.x < min_size.x))
     base_size.x = min_size.x;

   //Check the Which direction is available.
   //If find a avaialble direction, it adjusts position and size.
   for (idx = 0; idx < 4; idx++)
     {
        switch (wd->dir_priority[idx])
          {
           case ELM_CTXPOPUP_DIRECTION_UP:
              temp.y = (pos.y - base_size.y);
              if ((temp.y - arrow_size.y) < hover_area.y)
                continue;
              _adjust_pos_x(&pos, &base_size, &hover_area);
              pos.y -= base_size.y;
              dir = ELM_CTXPOPUP_DIRECTION_UP;
              break;
           case ELM_CTXPOPUP_DIRECTION_LEFT:
              temp.x = (pos.x - base_size.x);
              if ((temp.x - arrow_size.x) < hover_area.x)
                continue;
              _adjust_pos_y(&pos, &base_size, &hover_area);
              pos.x -= base_size.x;
              dir = ELM_CTXPOPUP_DIRECTION_LEFT;
              break;
           case ELM_CTXPOPUP_DIRECTION_RIGHT:
              temp.x = (pos.x + base_size.x);
              if ((temp.x + arrow_size.x) >
                  (hover_area.x + hover_area.w))
                continue;
              _adjust_pos_y(&pos, &base_size, &hover_area);
              dir = ELM_CTXPOPUP_DIRECTION_RIGHT;
              break;
           case ELM_CTXPOPUP_DIRECTION_DOWN:
              temp.y = (pos.y + base_size.y);
              if ((temp.y + arrow_size.y) >
                  (hover_area.y + hover_area.h))
                continue;
              _adjust_pos_x(&pos, &base_size, &hover_area);
              dir = ELM_CTXPOPUP_DIRECTION_DOWN;
              break;
           default:
              break;
          }
        break;
     }

   //In this case, all directions are invalid because of lack of space.
   if (idx == 4)
     {
        Evas_Coord length[2];

        if(!wd->horizontal)
          {
             length[0] = pos.y - hover_area.y;
             length[1] = (hover_area.y + hover_area.h) - pos.y;

             // ELM_CTXPOPUP_DIRECTION_UP
             if (length[0] > length[1])
               {
                  _adjust_pos_x(&pos, &base_size, &hover_area);
                  pos.y -= base_size.y;
                  dir = ELM_CTXPOPUP_DIRECTION_UP;
                  if (pos.y < (hover_area.y + arrow_size.y))
                    {
                       base_size.y -= ((hover_area.y + arrow_size.y) - pos.y);
                       pos.y = hover_area.y + arrow_size.y;
                    }
               }
             //ELM_CTXPOPUP_DIRECTION_DOWN
             else
               {
                  _adjust_pos_x(&pos, &base_size, &hover_area);
                  dir = ELM_CTXPOPUP_DIRECTION_DOWN;
                  if ((pos.y + arrow_size.y + base_size.y) >
                      (hover_area.y + hover_area.h))
                     base_size.y -=
                        ((pos.y + arrow_size.y + base_size.y) -
                         (hover_area.y + hover_area.h));
               }
          }
        else
          {
             length[0] = pos.x - hover_area.x;
             length[1] = (hover_area.x + hover_area.w) - pos.x;

             //ELM_CTXPOPUP_DIRECTION_LEFT
             if (length[0] > length[1])
               {
                  _adjust_pos_y(&pos, &base_size, &hover_area);
                  pos.x -= base_size.x;
                  dir = ELM_CTXPOPUP_DIRECTION_LEFT;
                  if (pos.x < (hover_area.x + arrow_size.x))
                    {
                       base_size.x -= ((hover_area.x + arrow_size.x) - pos.x);
                       pos.x = hover_area.x + arrow_size.x;
                    }
               }
             //ELM_CTXPOPUP_DIRECTION_RIGHT
             else
               {
                  _adjust_pos_y(&pos, &base_size, &hover_area);
                  dir = ELM_CTXPOPUP_DIRECTION_RIGHT;
                  if (pos.x + (arrow_size.x + base_size.x) >
                      hover_area.x + hover_area.w)
                     base_size.x -=
                        ((pos.x + arrow_size.x + base_size.x) -
                         (hover_area.x + hover_area.w));
               }
          }
     }

   //Final position and size.
   rect->x = pos.x;
   rect->y = pos.y;
   rect->w = base_size.x;
   rect->h = base_size.y;

   return dir;
}

static void
_update_arrow(Evas_Object *obj, Elm_Ctxpopup_Direction dir)
{
   Evas_Coord x, y;
   Evas_Coord_Rectangle arrow_size;
   Evas_Coord_Rectangle base_size;
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_geometry_get(obj, &x, &y, NULL, NULL);
   evas_object_geometry_get(wd->arrow, NULL, NULL, &arrow_size.w,
                            &arrow_size.h);
   evas_object_geometry_get(wd->base, &base_size.x, &base_size.y,
                            &base_size.w, &base_size.h);

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
         edje_object_signal_emit(wd->arrow, "elm,state,left", "elm");
         edje_object_part_swallow(wd->base, "elm.swallow.arrow_left", wd->arrow);
         if (base_size.h > 0)
           {
              if (y < ((arrow_size.h * 0.5) + base_size.y))
                y = 0;
              else if (y > base_size.y + base_size.h - (arrow_size.h * 0.5))
                y = base_size.h - arrow_size.h;
              else
                y = y - base_size.y - (arrow_size.h * 0.5);
              edje_object_part_drag_value_set(wd->base, "elm.swallow.arrow_left", 1,
                                              (double) (y) / (double) (base_size.h - arrow_size.h));
           }
         break;
      case ELM_CTXPOPUP_DIRECTION_LEFT:
         edje_object_signal_emit(wd->arrow, "elm,state,right", "elm");
         edje_object_part_swallow(wd->base, "elm.swallow.arrow_right", wd->arrow);
         if (base_size.h > 0)
            {
              if (y < (arrow_size.h * 0.5) + base_size.y)
                y = 0;
              else if (y > (base_size.y + base_size.h - (arrow_size.h * 0.5)))
                y = base_size.h - arrow_size.h;
              else y = y - base_size.y - (arrow_size.h * 0.5);
              edje_object_part_drag_value_set(wd->base, "elm.swallow.arrow_right", 0,
                                              (double) (y) / (double) (base_size.h - arrow_size.h));
            }
         break;
      case ELM_CTXPOPUP_DIRECTION_DOWN:
         edje_object_signal_emit(wd->arrow, "elm,state,top", "elm");
         edje_object_part_swallow(wd->base, "elm.swallow.arrow_up", wd->arrow);
         if (base_size.w > 0)
           {
              if (x < (arrow_size.w * 0.5) + base_size.x)
                x = 0;
              else if (x > (base_size.x + base_size.w - (arrow_size.w * 0.5)))
                x = base_size.w - arrow_size.w;
              else
                x = x - base_size.x - (arrow_size.w * 0.5);
              edje_object_part_drag_value_set(wd->base, "elm.swallow.arrow_up",
                                              (double) (x) / (double) (base_size.w - arrow_size.w), 1);
           }
         break;
      case ELM_CTXPOPUP_DIRECTION_UP:
         edje_object_signal_emit(wd->arrow, "elm,state,bottom", "elm");
         edje_object_part_swallow(wd->base, "elm.swallow.arrow_down", wd->arrow);
         if (base_size.w > 0)
           {
              if (x < (arrow_size.w * 0.5) + base_size.x)
                x = 0;
              else if (x > (base_size.x + base_size.w - (arrow_size.w * 0.5)))
                x = base_size.w - arrow_size.w;
              else x = x - base_size.x - (arrow_size.w * 0.5);
              edje_object_part_drag_value_set(wd->base, "elm.swallow.arrow_down",
                                              (double) (x) / (double) (base_size.w - arrow_size.w), 0);
           }
         break;
      default:
         break;
     }
}

static void
_show_signal_emit(Evas_Object *obj, Elm_Ctxpopup_Direction dir)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd || wd->visible) return;

   switch (dir)
     {
        case ELM_CTXPOPUP_DIRECTION_UP:
           edje_object_signal_emit(wd->base, "elm,state,show,up", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_LEFT:
           edje_object_signal_emit(wd->base, "elm,state,show,left", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_RIGHT:
           edje_object_signal_emit(wd->base, "elm,state,show,right", "elm");
           break;
        case ELM_CTXPOPUP_DIRECTION_DOWN:
           edje_object_signal_emit(wd->base, "elm,state,show,down", "elm");
           break;
        default:
           break;
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Widget_Data *wd;
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;
   Evas_Coord_Rectangle rect = { 0, 0, 1, 1 };
   Evas_Coord_Point box_size = { 0, 0 };
   Evas_Coord_Point _box_size = { 0, 0 };

   wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->parent)) return;

   //Box, Scroller
   EINA_LIST_FOREACH(wd->items, elist, item)
     {
        _item_sizing_eval(item);
        evas_object_size_hint_min_get(item->base.view, &_box_size.x, &_box_size.y);
        if (!wd->horizontal)
          {
             if (_box_size.x > box_size.x)
               box_size.x = _box_size.x;
             if (_box_size.y != -1)
               box_size.y += _box_size.y;
          }
        else
          {
             if (_box_size.x != -1)
               box_size.x += _box_size.x;
             if (_box_size.y > box_size.y)
               box_size.y = _box_size.y;
          }
     }

   if (!wd->content)
     {
        evas_object_size_hint_min_set(wd->box, box_size.x, box_size.y);
        evas_object_size_hint_min_set(wd->scr, box_size.x, box_size.y);
     }

   //Base
   wd->dir = _calc_base_geometry(obj, &rect);
   _show_signal_emit(obj, wd->dir);
   _update_arrow(obj, wd->dir);
   _shift_base_by_arrow(wd->arrow, wd->dir, &rect);

   //resize scroller according to final size.
   if (!wd->content)
     evas_object_smart_calculate(wd->scr);

   evas_object_move(wd->base, rect.x, rect.y);
   evas_object_resize(wd->base, rect.w, rect.h);
}

static void
_shift_base_by_arrow(Evas_Object *arrow, Elm_Ctxpopup_Direction dir,
                     Evas_Coord_Rectangle *rect)
{
   Evas_Coord arrow_w, arrow_h;

   evas_object_geometry_get(arrow, NULL, NULL, &arrow_w, &arrow_h);

   switch (dir)
     {
      case ELM_CTXPOPUP_DIRECTION_RIGHT:
         rect->x += arrow_w;
         break;
      case ELM_CTXPOPUP_DIRECTION_LEFT:
         rect->x -= arrow_w;
         break;
      case ELM_CTXPOPUP_DIRECTION_DOWN:
         rect->y += arrow_h;
         break;
      case ELM_CTXPOPUP_DIRECTION_UP:
         rect->y -= arrow_h;
         break;
      default:
         break;
     }
}

static void
_del_pre_hook(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   evas_object_event_callback_del_full(wd->parent, EVAS_CALLBACK_RESIZE,
                                       _parent_resize, obj);

   _hover_parent_callbacks_del(obj);
}

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   elm_ctxpopup_clear(obj);
   evas_object_del(wd->arrow);
   evas_object_del(wd->base);
   free(wd);
}

static void
_theme_hook(Evas_Object *obj)
{
   Widget_Data *wd;
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   //Items
   EINA_LIST_FOREACH(wd->items, elist, item)
     {
        if (item->label && item->icon)
          _elm_theme_object_set(obj, item->base.view, "ctxpopup",
                                "icon_text_style_item",
                                elm_widget_style_get(obj));
        else if (item->label)
          _elm_theme_object_set(obj, item->base.view, "ctxpopup", "text_style_item",
                                elm_widget_style_get(obj));
        else if (item->icon)
          _elm_theme_object_set(obj, item->base.view, "ctxpopup", "icon_style_item",
                                elm_widget_style_get(obj));
        if (item->label)
          edje_object_part_text_set(item->base.view, "elm.text", item->label);

        if (item->disabled)
          edje_object_signal_emit(item->base.view, "elm,state,disabled", "elm");

        edje_object_message_signal_process(item->base.view);
     }

   _elm_theme_object_set(obj, wd->bg, "ctxpopup", "bg",
                         elm_widget_style_get(obj));
   _elm_theme_object_set(obj, wd->base, "ctxpopup", "base",
                         elm_widget_style_get(obj));
   _elm_theme_object_set(obj, wd->arrow, "ctxpopup", "arrow",
                         elm_widget_style_get(obj));

   if (wd->scr)
     {
        if (!strncmp(elm_object_style_get(obj), "default",
                     strlen("default")))
           elm_object_style_set(wd->scr, "ctxpopup");
        else
           elm_object_style_set(wd->scr, elm_object_style_get(obj));
     }

   if (wd->visible)
     {
        _scroller_size_reset(wd);
        _sizing_eval(obj);
     }
}

static void
_content_set_hook(Evas_Object *obj, const char *part __UNUSED__,
                  Evas_Object *content)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if ((!wd) || (!content)) return;

   if (wd->items) elm_ctxpopup_clear(obj);
   if (wd->content) evas_object_del(wd->content);

   evas_object_event_callback_add(content, EVAS_CALLBACK_DEL, _content_del,
                                  obj);

   elm_widget_sub_object_add(obj, content);
   edje_object_part_swallow(wd->base, "elm.swallow.content", content);

   wd->content = content;

   if (wd->visible)
     _sizing_eval(obj);
}

static Evas_Object *
_content_unset_hook(Evas_Object *obj, const char *part __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;
   Evas_Object *content;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   content = wd->content;
   if (!content) return NULL;

   edje_object_part_unswallow(wd->base, content);
   elm_widget_sub_object_del(obj, content);
   evas_object_event_callback_del(content, EVAS_CALLBACK_DEL, _content_del);
   edje_object_signal_emit(wd->base, "elm,state,content,disable", "elm");

   wd->content = NULL;

   return content;

}

static Evas_Object *
_content_get_hook(const Evas_Object *obj, const char *part __UNUSED__)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return NULL;
   return wd->content;
}

static void
_bg_clicked_cb(void *data, Evas_Object *obj __UNUSED__,
               const char *emission __UNUSED__, const char *source __UNUSED__)
{
   evas_object_hide(data);
}

static void
_parent_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Evas_Coord w, h;
   Widget_Data *wd;

   wd = elm_widget_data_get(data);
   if (!wd) return;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_resize(wd->bg, w, h);

   if (!wd->visible) return;

   _hide(data);
}

static void
_ctxpopup_show(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if ((!wd->items) && (!wd->content)) return;

   wd->visible = EINA_TRUE;

   evas_object_show(wd->bg);
   evas_object_show(wd->base);
   evas_object_show(wd->arrow);

   edje_object_signal_emit(wd->bg, "elm,state,show", "elm");
   edje_object_signal_emit(wd->base, "elm,state,show", "elm");

   _sizing_eval(obj);
}

static void
_hide(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);

   if (!wd) return;

   evas_object_hide(wd->bg);
   evas_object_hide(wd->arrow);
   evas_object_hide(wd->base);

   _scroller_size_reset(wd);

   evas_object_smart_callback_call(obj, SIG_DISMISSED, NULL);
   wd->visible = EINA_FALSE;
}

static void
_ctxpopup_hide(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->visible))
     return;

   _hide(obj);
}

static void
_scroller_resize(void *data, Evas *e __UNUSED__, Evas_Object *obj,
                 void *event_info __UNUSED__)
{
   Widget_Data *wd;
   Evas_Coord w, h;

   wd = elm_widget_data_get(data);
   if (!wd) return;
   if (!wd->visible) return;
   if (wd->finished) return;

   evas_object_geometry_get(obj, 0, 0, &w, &h);

   if (w != 0 && h != 0)
     {
        if ((w <= wd->max_sc_w) && (h <= wd->max_sc_h))
          {
             _sizing_eval(data);
             wd->finished = EINA_TRUE;
             return;
          }
     }

   if (wd->max_sc_w < w)
     wd->max_sc_w = w;
   if (wd->max_sc_h < h)
     wd->max_sc_h = h;

   _sizing_eval(data);
}

static void
_ctxpopup_move(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj,
               void *event_info __UNUSED__)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);

   if (!wd) return;

   if (wd->visible)
     evas_object_show(wd->arrow);

   _scroller_size_reset(wd);
   _sizing_eval(obj);
}

static void
_item_select_cb(void *data, Evas_Object *obj __UNUSED__,
                const char *emission __UNUSED__, const char *source __UNUSED__)
{
   Elm_Ctxpopup_Item *item = data;

   if (!item) return;
   if (item->disabled) return;

   if (item->func)
     item->func((void*) item->base.data, item->base.widget, data);
}

static void
_item_icon_set(Elm_Ctxpopup_Item *item, Evas_Object *icon)
{
   if (item->icon)
     evas_object_del(item->icon);

   item->icon = icon;
   if (!icon) return;

   edje_object_part_swallow(item->base.view, "elm.swallow.icon", item->icon);
   edje_object_message_signal_process(item->base.view);
}

static void
_item_label_set(Elm_Ctxpopup_Item *item, const char *label)
{
   if (!eina_stringshare_replace(&item->label, label))
     return;

   edje_object_part_text_set(item->base.view, "elm.text", label);
   edje_object_message_signal_process(item->base.view);
}

static void
_item_new(Elm_Ctxpopup_Item *item, char *group_name)
{
   Widget_Data *wd;

   wd = elm_widget_data_get(item->base.widget);
   if (!wd) return;

   item->base.view = edje_object_add(evas_object_evas_get(wd->base));
   _elm_theme_object_set(item->base.widget, item->base.view, "ctxpopup", group_name,
                         elm_widget_style_get(item->base.widget));
   edje_object_signal_callback_add(item->base.view, "elm,action,click", "",
                                   _item_select_cb, item);
   evas_object_size_hint_align_set(item->base.view, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(item->base.view);
}

static void
_content_del(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   elm_object_content_unset(data);
}

static void
_list_del(Widget_Data *wd)
{
   if (!wd->scr) return;

   edje_object_part_unswallow(wd->base, wd->scr);
   evas_object_del(wd->scr);
   wd->scr = NULL;
   wd->box = NULL;
}

static void
_list_new(Evas_Object *obj)
{
   Widget_Data *wd;
   wd = elm_widget_data_get(obj);
   if (!wd) return;

   //scroller
   wd->scr = elm_scroller_add(obj);
   elm_object_style_set(wd->scr, "ctxpopup");
   evas_object_size_hint_align_set(wd->scr, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_event_callback_add(wd->scr, EVAS_CALLBACK_RESIZE,
                                  _scroller_resize, obj);
   edje_object_part_swallow(wd->base, "elm.swallow.content", wd->scr);

   //box
   wd->box = elm_box_add(obj);
   evas_object_size_hint_weight_set(wd->box, EVAS_HINT_EXPAND,
                                    EVAS_HINT_EXPAND);

   elm_scroller_content_set(wd->scr, wd->box);
   elm_ctxpopup_horizontal_set(obj, wd->horizontal);
}

static void
_remove_items(Widget_Data *wd)
{
   Eina_List *elist;
   Elm_Ctxpopup_Item *item;

   if (!wd->items) return;

   EINA_LIST_FOREACH(wd->items, elist, item)
     {
        if (item->label)
          eina_stringshare_del(item->label);
        if (item->icon)
          evas_object_del(item->icon);
        wd->items = eina_list_remove(wd->items, item);
        free(item);
     }

   wd->items = NULL;
}

EAPI Evas_Object *
elm_ctxpopup_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;
   Evas_Coord x, y, w, h;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "ctxpopup");
   elm_widget_type_set(obj, "ctxpopup");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_pre_hook_set(obj, _del_pre_hook);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_theme_hook_set(obj, _theme_hook);
   elm_widget_content_set_hook_set(obj, _content_set_hook);
   elm_widget_content_unset_hook_set(obj, _content_unset_hook);
   elm_widget_content_get_hook_set(obj, _content_get_hook);

   wd->parent = parent;

   //Background
   wd->bg = edje_object_add(e);
   elm_widget_sub_object_add(obj, wd->bg);
   _elm_theme_object_set(obj, wd->bg, "ctxpopup", "bg", "default");
   evas_object_geometry_get(parent, &x, &y, &w, &h);
   evas_object_move(wd->bg, x, y);
   evas_object_resize(wd->bg, w, h);
   edje_object_signal_callback_add(wd->bg, "elm,action,click", "",
                                   _bg_clicked_cb, obj);

   //Base
   wd->base = edje_object_add(e);
   elm_widget_sub_object_add(obj, wd->base);
   _elm_theme_object_set(obj, wd->base, "ctxpopup", "base", "default");

   //Arrow
   wd->arrow = edje_object_add(e);
   elm_widget_sub_object_add(obj, wd->arrow);
   _elm_theme_object_set(obj, wd->arrow, "ctxpopup", "arrow", "default");

   wd->dir_priority[0] = ELM_CTXPOPUP_DIRECTION_UP;
   wd->dir_priority[1] = ELM_CTXPOPUP_DIRECTION_LEFT;
   wd->dir_priority[2] = ELM_CTXPOPUP_DIRECTION_RIGHT;
   wd->dir_priority[3] = ELM_CTXPOPUP_DIRECTION_DOWN;

   evas_object_event_callback_add(parent, EVAS_CALLBACK_RESIZE, _parent_resize,
                                  obj);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_SHOW, _ctxpopup_show,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_HIDE, _ctxpopup_hide,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_MOVE, _ctxpopup_move,
                                  NULL);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                  _ctxpopup_changed_size_hints, NULL);
   evas_object_smart_callback_add(obj, "scroll-freeze-on", _freeze_on, obj);
   evas_object_smart_callback_add(obj, "scroll-freeze-off", _freeze_off, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-on", _hold_on, obj);
   evas_object_smart_callback_add(obj, "scroll-hold-off", _hold_off, obj);

   evas_object_smart_callbacks_descriptions_set(obj, _signals);

   return obj;
}

EAPI Evas_Object *
elm_ctxpopup_item_icon_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Ctxpopup_Item *ctxpopup_it = ELM_CAST(it);
   return ctxpopup_it->icon;
}

EAPI void
elm_ctxpopup_item_icon_set(Elm_Object_Item *it, Evas_Object *icon)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);

   Widget_Data *wd;
   Elm_Ctxpopup_Item *ctxpopup_it  = ELM_CAST(it);

   wd = elm_widget_data_get(ctxpopup_it->base.widget);
   if (!wd) return;

   _item_icon_set(ctxpopup_it, icon);

   if (wd->visible)
     {
        _scroller_size_reset(wd);
        _sizing_eval(ctxpopup_it->base.widget);
     }
}

EAPI const char *
elm_ctxpopup_item_label_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, NULL);
   Elm_Ctxpopup_Item *ctxpopup_it = ELM_CAST(it);
   return ctxpopup_it->label;
}

EAPI void
elm_ctxpopup_item_label_set(Elm_Object_Item *it, const char *label)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);

   Widget_Data *wd;
   Elm_Ctxpopup_Item *ctxpopup_it = ELM_CAST(it);

   wd = elm_widget_data_get(ctxpopup_it->base.widget);
   if (!wd) return;

   _item_label_set(ctxpopup_it, label);

   if (wd->visible)
     {
        _scroller_size_reset(wd);
        _sizing_eval(ctxpopup_it->base.widget);
     }
}

EAPI void
elm_ctxpopup_hover_parent_set(Evas_Object *obj, Evas_Object *hover_parent)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   _hover_parent_callbacks_del(obj);

   if (hover_parent)
     {
        evas_object_event_callback_add(hover_parent, EVAS_CALLBACK_DEL,
                                       _hover_parent_del, obj);
        evas_object_event_callback_add(hover_parent, EVAS_CALLBACK_MOVE,
                                       _hover_parent_move, obj);
        evas_object_event_callback_add(hover_parent, EVAS_CALLBACK_RESIZE,
                                       _hover_parent_resize, obj);
     }

   wd->hover_parent = hover_parent;
}

EAPI Evas_Object *
elm_ctxpopup_hover_parent_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   return wd->hover_parent;
}

EAPI void
elm_ctxpopup_clear(Evas_Object * obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;

   _remove_items(wd);
   _list_del(wd);
}

EAPI void
elm_ctxpopup_horizontal_set(Evas_Object *obj, Eina_Bool horizontal)
{
   ELM_CHECK_WIDTYPE(obj, widtype);

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->horizontal = !!horizontal;

   if ((!wd->scr) && (!wd->box))
      return;

   if (!horizontal)
     {
        elm_box_horizontal_set(wd->box, EINA_FALSE);
        elm_scroller_bounce_set(wd->scr, EINA_FALSE, EINA_TRUE);
     }
   else
     {
        elm_box_horizontal_set(wd->box, EINA_TRUE);
        elm_scroller_bounce_set(wd->scr, EINA_TRUE, EINA_FALSE);
     }

   if (wd->visible)
      _sizing_eval(obj);
}

EAPI Eina_Bool
elm_ctxpopup_horizontal_get(const Evas_Object *obj)
{
   ELM_CHECK_WIDTYPE(obj, widtype) EINA_FALSE;

   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return EINA_FALSE;

   return wd->horizontal;
}

EAPI Elm_Object_Item *
elm_ctxpopup_item_append(Evas_Object *obj, const char *label,
                         Evas_Object *icon, Evas_Smart_Cb func,
                         const void *data)
{
   ELM_CHECK_WIDTYPE(obj, widtype) NULL;

   Widget_Data *wd;
   Elm_Ctxpopup_Item *item;

   wd = elm_widget_data_get(obj);
   if (!wd) return NULL;

   item = elm_widget_item_new(obj, Elm_Ctxpopup_Item);
   if (!item) return NULL;

   //The first item is appended.
   if (wd->content)
     evas_object_del(elm_object_content_unset(obj));

   if (!wd->items)
     _list_new(obj);

   item->func = func;
   item->base.data = data;

   if (icon && label)
     _item_new(item, "icon_text_style_item");
   else if (label)
     _item_new(item, "text_style_item");
   else
     _item_new(item, "icon_style_item");

   _item_icon_set(item, icon);
   _item_label_set(item, label);
   elm_box_pack_end(wd->box, item->base.view);
   wd->items = eina_list_append(wd->items, item);

   if (wd->visible)
     {
        _scroller_size_reset(wd);
        _sizing_eval(obj);
     }

   return ELM_CAST(item);
}

EAPI void
elm_ctxpopup_item_del(Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);

   Widget_Data *wd;
   Elm_Ctxpopup_Item *ctxpopup_it = ELM_CAST(it);

   wd = elm_widget_data_get(ctxpopup_it->base.widget);
   if (!wd) return;

   if (ctxpopup_it->icon)
      evas_object_del(ctxpopup_it->icon);
   if (ctxpopup_it->base.view)
     evas_object_del(ctxpopup_it->base.view);

   eina_stringshare_del(ctxpopup_it->label);

   wd->items = eina_list_remove(wd->items, ctxpopup_it);

   if (eina_list_count(wd->items) < 1)
     wd->items = NULL;

   if (wd->visible)
     _sizing_eval(ctxpopup_it->base.widget);

   free(ctxpopup_it);
}

EAPI void
elm_ctxpopup_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it);

   Widget_Data *wd;
   Elm_Ctxpopup_Item *ctxpopup_it = ELM_CAST(it);

   wd = elm_widget_data_get(ctxpopup_it->base.widget);
   if (!wd) return;

   ctxpopup_it = ELM_CAST(it);

   if (disabled == ctxpopup_it->disabled)
     return;

   if (disabled)
     edje_object_signal_emit(ctxpopup_it->base.view, "elm,state,disabled", "elm");
   else
     edje_object_signal_emit(ctxpopup_it->base.view, "elm,state,enabled", "elm");

   ctxpopup_it->disabled = !!disabled;
}

EAPI Eina_Bool
elm_ctxpopup_item_disabled_get(const Elm_Object_Item *it)
{
   ELM_OBJ_ITEM_CHECK_OR_RETURN(it, EINA_FALSE);
   Elm_Ctxpopup_Item *ctxpopup_it = ELM_CAST(it);
   return ctxpopup_it->disabled;
}

EAPI void
elm_ctxpopup_content_set(Evas_Object *obj, Evas_Object *content)
{
   elm_object_content_set(obj, content);
}

EAPI Evas_Object *
elm_ctxpopup_content_unset(Evas_Object *obj)
{
   return elm_object_content_unset(obj);
}

EAPI void
elm_ctxpopup_direction_priority_set(Evas_Object *obj,
                                    Elm_Ctxpopup_Direction first,
                                    Elm_Ctxpopup_Direction second,
                                    Elm_Ctxpopup_Direction third,
                                    Elm_Ctxpopup_Direction fourth)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   wd->dir_priority[0] = first;
   wd->dir_priority[1] = second;
   wd->dir_priority[2] = third;
   wd->dir_priority[3] = fourth;

   if (wd->visible)
     _sizing_eval(obj);
}

EAPI void
elm_ctxpopup_direction_priority_get(Evas_Object *obj,
                                    Elm_Ctxpopup_Direction *first,
                                    Elm_Ctxpopup_Direction *second,
                                    Elm_Ctxpopup_Direction *third,
                                    Elm_Ctxpopup_Direction *fourth)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd;

   wd = elm_widget_data_get(obj);
   if (!wd) return;

   if (first) *first = wd->dir_priority[0];
   if (second) *second = wd->dir_priority[1];
   if (third) *third = wd->dir_priority[2];
   if (fourth) *fourth = wd->dir_priority[3];
}
