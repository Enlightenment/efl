#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#define ELM_INTERFACE_ATSPI_WIDGET_ACTION_PROTECTED

#include <Elementary.h>

#include "elm_priv.h"
#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"
#include "elm_widget_scroller.h"

#include "elm_scroller_internal_part.eo.h"
#include "elm_part_helper.h"

#define MY_CLASS ELM_SCROLLER_CLASS

#define MY_CLASS_NAME "Elm_Scroller"
#define MY_CLASS_NAME_LEGACY "elm_scroller"

#define ELM_PRIV_SCROLLER_SIGNALS(cmd) \
   cmd(SIG_SCROLL, "scroll", "") \
   cmd(SIG_SCROLL_LEFT, "scroll,left", "") \
   cmd(SIG_SCROLL_RIGHT, "scroll,right", "") \
   cmd(SIG_SCROLL_UP, "scroll,up", "") \
   cmd(SIG_SCROLL_DOWN, "scroll,down", "") \
   cmd(SIG_SCROLL_ANIM_START, "scroll,anim,start", "") \
   cmd(SIG_SCROLL_ANIM_STOP, "scroll,anim,stop", "") \
   cmd(SIG_SCROLL_DRAG_START, "scroll,drag,start", "") \
   cmd(SIG_SCROLL_DRAG_STOP, "scroll,drag,stop", "") \
   cmd(SIG_EDGE_LEFT, "edge,left", "") \
   cmd(SIG_EDGE_RIGHT, "edge,right", "") \
   cmd(SIG_EDGE_TOP, "edge,top", "") \
   cmd(SIG_EDGE_BOTTOM, "edge,bottom", "") \
   cmd(SIG_VBAR_DRAG, "vbar,drag", "") \
   cmd(SIG_VBAR_PRESS, "vbar,press", "") \
   cmd(SIG_VBAR_UNPRESS, "vbar,unpress", "") \
   cmd(SIG_HBAR_DRAG, "hbar,drag", "") \
   cmd(SIG_HBAR_PRESS, "hbar,press", "") \
   cmd(SIG_HBAR_UNPRESS, "hbar,unpress", "") \
   cmd(SIG_SCROLL_PAGE_CHANGE, "scroll,page,changed", "") \

ELM_PRIV_SCROLLER_SIGNALS(ELM_PRIV_STATIC_VARIABLE_DECLARE);

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   ELM_PRIV_SCROLLER_SIGNALS(ELM_PRIV_SMART_CALLBACKS_DESC)
   {SIG_LAYOUT_FOCUSED, ""}, /**< handled by elm_layout */
   {SIG_LAYOUT_UNFOCUSED, ""}, /**< handled by elm_layout */
   {NULL, NULL}
};
#undef ELM_PRIV_SCROLLER_SIGNALS

static Eina_Bool _key_action_move(Evas_Object *obj, const char *params);

static const Elm_Action key_actions[] = {
   {"move", _key_action_move},
   {NULL, NULL}
};

static void
_elm_scroller_proxy_set(Evas_Object *obj, Elm_Scroller_Data *sd, Evas_Object *proxy)
{
   Evas_Coord h_pagesize, v_pagesize;
   Evas_Coord cw = 0, ch = 0;
   Evas_Object *content = sd->content;

   if (!content) return;

   elm_interface_scrollable_paging_get(obj, NULL, NULL, &h_pagesize, &v_pagesize);
   elm_interface_scrollable_content_size_get(obj, &cw, &ch);
   /* Since Proxy has the texture size limitation problem, we set a key value
      for evas works in some hackish way to avoid this problem. This hackish
      code should be removed once evas supports a mechanism like a virtual
      texture. */
   evas_object_image_fill_set(proxy, 0, 0, cw, ch);
   evas_object_size_hint_min_set(proxy, h_pagesize, v_pagesize);
   evas_object_image_source_clip_set(proxy, EINA_FALSE);
   evas_object_image_source_set(proxy, content);
   evas_object_show(proxy);
}
//describe position of rect2 relative to rect1
// 1 = top outside
// 2 = left outside
// 4 = bottom outside
// 8 = right outside
static char
_intersect_direction(Eina_Rectangle *rect1, Eina_Rectangle *rect2)
{
   char ret = 0;

   if (rect1->y > rect2->y)
     ret |= 1;
   if (rect1->x > rect2->x)
     ret |= 2;
   if (rect1->y + rect1->h < rect2->y + rect2->h)
     ret |= 4;
   if (rect1->x + rect1->w < rect2->x + rect2->w)
     ret |= 8;

   return ret;
}

static Eina_Bool
_key_action_move(Evas_Object *obj, const char *params)
{
   ELM_SCROLLER_DATA_GET(obj, sd);
   const char *dir = params;
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_x = 0;
   Evas_Coord v_y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord max_x = 0;
   Evas_Coord max_y = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Coord pagesize_h = 0, pagesize_v = 0;
   Evas_Coord pagenumber_h = 0, pagenumber_v = 0;

   elm_interface_scrollable_content_pos_get(obj, &x, &y);
   elm_interface_scrollable_step_size_get(obj, &step_x, &step_y);
   elm_interface_scrollable_page_size_get(obj, &page_x, &page_y);
   elm_interface_scrollable_content_viewport_geometry_get
         (obj, &v_x, &v_y, &v_w, &v_h);
   evas_object_geometry_get(sd->content, NULL, NULL, &max_x, &max_y);

   {
      Efl_Ui_Focus_Object *focused;
      Eina_Rectangle focused_geom, viewport;

      focused = efl_ui_focus_manager_focused(obj);

      if (focused &&
          (!strcmp(dir, "next") ||
           !strcmp(dir, "prior")))
        return EINA_FALSE;

      if (focused &&
          (!strcmp(dir, "left") ||
           !strcmp(dir, "right") ||
           !strcmp(dir, "up") ||
           !strcmp(dir, "down")))
        {
           char relative;

           evas_object_geometry_get(focused,
                 &focused_geom.x, &focused_geom.y, &focused_geom.w, &focused_geom.h);
           elm_interface_scrollable_content_viewport_geometry_get(obj,
                 &viewport.x, &viewport.y, &viewport.w, &viewport.h);

           relative = _intersect_direction(&viewport, &focused_geom);

           //now precisly check if the direction is also lapping out
           if ((!strcmp(dir, "up") && !(relative & 1)) ||
               (!strcmp(dir, "left") && !(relative & 2)) ||
               (!strcmp(dir, "down") && !(relative & 4)) ||
               (!strcmp(dir, "right") && !(relative & 8)))
             {
                //focus will handle that
                return EINA_FALSE;
             }
        }
   }

   elm_interface_scrollable_paging_get(obj, NULL, NULL, &pagesize_h, &pagesize_v);
   elm_interface_scrollable_current_page_get(obj, &pagenumber_h, &pagenumber_v);

   if (!strcmp(dir, "left"))
     {
        if ((x <= 0) && (!sd->loop_h)) return EINA_FALSE;
        if (pagesize_h)
          {
             elm_interface_scrollable_page_bring_in(obj, pagenumber_h - 1, pagenumber_v);
             return EINA_TRUE;
          }
        else
          x -= step_x;
     }
   else if (!strcmp(dir, "right"))
     {
        if ((x >= (max_x - v_w)) && (!sd->loop_h)) return EINA_FALSE;
        if (pagesize_h)
          {
             elm_interface_scrollable_page_bring_in(obj, pagenumber_h + 1, pagenumber_v);
             return EINA_TRUE;
          }
        else
          x += step_x;
     }
   else if (!strcmp(dir, "up"))
     {
        if ((y <= 0) && (!sd->loop_v)) return EINA_FALSE;
        if (pagesize_v)
          {
             elm_interface_scrollable_page_bring_in(obj, pagenumber_h, pagenumber_v - 1);
             return EINA_TRUE;
          }
        else
          y -= step_y;
     }
   else if (!strcmp(dir, "down"))
     {
        if ((y >= (max_y - v_h)) && (!sd->loop_v)) return EINA_FALSE;
        if (pagesize_v)
          {
             elm_interface_scrollable_page_bring_in(obj, pagenumber_h, pagenumber_v + 1);
             return EINA_TRUE;
          }
        else
          y += step_y;
     }
   else if (!strcmp(dir, "first"))
     {
        y = 0;
     }
   else if (!strcmp(dir, "last"))
     {
        y = max_y - v_h;
     }
   else if (!strcmp(dir, "prior"))
     {
        if (pagesize_v)
          {
             elm_interface_scrollable_page_bring_in(obj, pagenumber_h, pagenumber_v - 1);
             return EINA_TRUE;
          }
        else
          {
             if (page_y < 0)
               y -= -(page_y * v_h) / 100;
             else
               y -= page_y;
          }
     }
   else if (!strcmp(dir, "next"))
     {
        if (pagesize_v)
          {
             elm_interface_scrollable_page_bring_in(obj, pagenumber_h, pagenumber_v + 1);
             return EINA_TRUE;
          }
        else
          {
             if (page_y < 0)
               y += -(page_y * v_h) / 100;
             else
               y += page_y;
          }
     }
   else return EINA_FALSE;

   elm_interface_scrollable_content_pos_set(obj, x, y, EINA_TRUE);


   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_scroller_elm_widget_widget_event(Eo *obj, Elm_Scroller_Data *sd EINA_UNUSED, Evas_Object *src, Evas_Callback_Type type, void *event_info)
{
   (void) src;
   Evas_Event_Key_Down *ev = event_info;

   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   if (!_elm_config_key_binding_call(obj, MY_CLASS_NAME, ev, key_actions))
     return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_scroller_elm_widget_activate(Eo *obj, Elm_Scroller_Data *_pd EINA_UNUSED, Elm_Activate act)
{
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (act == ELM_ACTIVATE_DEFAULT) return EINA_FALSE;

   elm_interface_scrollable_content_pos_get(obj, &x, &y);
   elm_interface_scrollable_page_size_get(obj, &page_x, &page_y);
   elm_interface_scrollable_content_viewport_geometry_get
         (obj, NULL, NULL, &v_w, &v_h);

   if (act == ELM_ACTIVATE_UP)
     {
        if (page_y < 0)
          y -= -(page_y * v_h) / 100;
        else
          y -= page_y;
     }
   else if (act == ELM_ACTIVATE_DOWN)
     {
        if (page_y < 0)
          y += -(page_y * v_h) / 100;
        else
          y += page_y;
     }
   else if (act == ELM_ACTIVATE_LEFT)
     {
        if (page_x < 0)
          x -= -(page_x * v_w) / 100;
        else
          x -= page_x;
     }
   else if (act == ELM_ACTIVATE_RIGHT)
     {
        if (page_x < 0)
          x += -(page_x * v_w) / 100;
        else
          x += page_x;
     }

   elm_interface_scrollable_content_pos_set(obj, x, y, EINA_TRUE);

   return EINA_TRUE;
}

EOLIAN static void
_elm_scroller_elm_layout_sizing_eval(Eo *obj, Elm_Scroller_Data *sd)
{
   Evas_Coord vw = 0, vh = 0, minw = 0, minh = 0, maxw = 0, maxh = 0, w, h,
              vmw, vmh;
   Evas_Coord h_pagesize, v_pagesize;
   double xw = 0.0, yw = 0.0;
   int i;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);

   if (sd->content)
     {
        efl_gfx_size_hint_combined_min_get(sd->content, &minw, &minh);
        evas_object_size_hint_max_get(sd->content, &maxw, &maxh);
        evas_object_size_hint_weight_get(sd->content, &xw, &yw);
     }

   elm_interface_scrollable_content_viewport_geometry_get
         (obj, NULL, NULL, &vw, &vh);
   if (xw > 0.0)
     {
        if ((minw > 0) && (vw < minw))
          vw = minw;
        else if ((maxw > 0) && (vw > maxw))
          vw = maxw;
     }
   else if (minw > 0)
     vw = minw;

   if (yw > 0.0)
     {
        if ((minh > 0) && (vh < minh))
          vh = minh;
        else if ((maxh > 0) && (vh > maxh))
          vh = maxh;
     }
   else if (minh > 0)
     vh = minh;

   if (sd->content) evas_object_resize(sd->content, vw, vh);
   if (sd->contents) evas_object_resize(sd->contents, vw, vh);

   for (i = 0 ; i < 3 ; i++)
     {
        if (!sd->proxy_content[i]) continue;
        elm_interface_scrollable_paging_get((Eo *)obj, NULL, NULL, &h_pagesize, &v_pagesize);
        evas_object_image_fill_set(sd->proxy_content[i], 0, 0, vw, vh);
        evas_object_size_hint_min_set(sd->proxy_content[i],
                                      h_pagesize, v_pagesize);
     }

   w = -1;
   h = -1;
   vmw = vmh = 0;
   edje_object_size_min_calc(wd->resize_obj, &vmw, &vmh);

   if (sd->min_w) w = vmw + minw;
   if (sd->min_h) h = vmh + minh;

   evas_object_size_hint_max_get(obj, &maxw, &maxh);
   if ((maxw > 0) && (w > maxw)) w = maxw;
   if ((maxh > 0) && (h > maxh)) h = maxh;

   evas_object_size_hint_min_set(obj, w, h);
}

static void
_mirrored_set(Evas_Object *obj,
              Eina_Bool mirrored)
{
   elm_interface_scrollable_mirrored_set(obj, mirrored);
}

EOLIAN static Elm_Theme_Apply
_elm_scroller_elm_widget_theme_apply(Eo *obj, Elm_Scroller_Data *sd EINA_UNUSED)
{
   Elm_Theme_Apply int_ret = ELM_THEME_APPLY_FAILED;
   int_ret = elm_obj_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (!int_ret) return ELM_THEME_APPLY_FAILED;

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   elm_layout_sizing_eval(obj);

   return int_ret;
}

EOLIAN static Eina_Bool
_elm_scroller_elm_widget_focus_next_manager_is(Eo *obj EINA_UNUSED, Elm_Scroller_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_scroller_elm_widget_focus_next(Eo *obj EINA_UNUSED, Elm_Scroller_Data *sd, Elm_Focus_Direction dir, Evas_Object **next, Elm_Object_Item **next_item)
{
   Evas_Object *cur;

   if (!sd->content) return EINA_FALSE;

   cur = sd->content;

   /* access */
   if (_elm_config->access_mode)
     {
        if ((elm_widget_can_focus_get(cur)) ||
            (elm_widget_child_can_focus_get(cur)))
          {
             return elm_widget_focus_next_get(cur, dir, next, next_item);
          }

        return EINA_FALSE;
     }

   /* Try focus cycle in subitem */
   if ((elm_widget_can_focus_get(cur)) ||
       (elm_widget_child_can_focus_get(cur)))
     {
        Eina_Bool ret = EINA_FALSE;

        ret =  elm_widget_focus_next_get(cur, dir, next, next_item);
        if (*next)
          {
             Evas_Coord x = 0, y = 0;
             Evas_Coord v_w = 0, v_h = 0;
             Evas_Coord c_x = 0, c_y = 0;
             Evas_Coord f_x = 0, f_y = 0, f_w = 0, f_h = 0;
             Evas_Coord l_x = 0, l_y = 0, l_w = 0, l_h = 0;
             Evas_Coord step_x = 0, step_y = 0;

             elm_interface_scrollable_content_pos_get(obj, &x, &y);
             elm_interface_scrollable_step_size_get(obj, &step_x, &step_y);
             elm_interface_scrollable_content_viewport_geometry_get
                (obj, NULL, NULL, &v_w, &v_h);
             evas_object_geometry_get(sd->content, &c_x, &c_y, NULL, NULL);
             evas_object_geometry_get(*next, &f_x, &f_y, &f_w, &f_h);
             l_x = f_x - c_x - step_x;
             l_y = f_y - c_y - step_y;
             l_w = f_w + (step_x * 2);
             l_h = f_h + (step_y * 2);

             if (!ret || ELM_RECTS_INTERSECT(x, y, v_w, v_h, l_x, l_y, l_w, l_h))
               return ret;
          }
     }

   if (!(elm_widget_can_focus_get(obj)) &&
       !(elm_widget_can_focus_get(cur)))
      return EINA_FALSE;

   /* Return */
   *next = (Evas_Object *)obj;

   return !elm_widget_focus_get(obj);
}

EOLIAN static Eina_Bool
_elm_scroller_elm_widget_focus_direction_manager_is(Eo *obj EINA_UNUSED, Elm_Scroller_Data *_pd EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_elm_scroller_elm_widget_focus_direction(Eo *obj, Elm_Scroller_Data *sd, const Evas_Object *base, double degree, Evas_Object **direction, Elm_Object_Item **direction_item, double *weight)
{
   Evas_Object *cur;

   if (!sd->content) return EINA_FALSE;

   cur = sd->content;

   /* access */
   if (_elm_config->access_mode)
     {
        if ((elm_widget_can_focus_get(cur)) ||
            (elm_widget_child_can_focus_get(cur)))
          {
             return elm_widget_focus_direction_get(cur, base, degree, direction, direction_item, weight);
          }

        return EINA_FALSE;
     }

   /* Try focus cycle in subitem */
   if ((elm_widget_can_focus_get(cur)) ||
       (elm_widget_child_can_focus_get(cur)))
     {
        Eina_Bool ret = EINA_FALSE;
        Evas_Coord x = 0, y = 0;
        Evas_Coord v_w = 0, v_h = 0;
        Evas_Coord c_x = 0, c_y = 0;
        Evas_Coord f_x = 0, f_y = 0, f_w = 0, f_h = 0;
        Evas_Coord l_x = 0, l_y = 0, l_w = 0, l_h = 0;
        Evas_Coord step_x = 0, step_y = 0;

        ret = elm_widget_focus_direction_get(cur, base, degree, direction, direction_item, weight);

        elm_interface_scrollable_content_pos_get(obj, &x, &y);
        elm_interface_scrollable_step_size_get(obj, &step_x, &step_y);
        elm_interface_scrollable_content_viewport_geometry_get
              (obj, NULL, NULL, &v_w, &v_h);
        evas_object_geometry_get(sd->content, &c_x, &c_y, NULL, NULL);
        evas_object_geometry_get(*direction, &f_x, &f_y, &f_w, &f_h);
        l_x = f_x - c_x - step_x;
        l_y = f_y - c_y - step_y;
        l_w = f_w + (step_x * 2);
        l_h = f_h + (step_y * 2);

        if (!ret || ELM_RECTS_INTERSECT(x, y, v_w, v_h, l_x, l_y, l_w, l_h))
          return ret;
     }

   /* Return */
   *direction = (Evas_Object *)obj;

   return !elm_widget_focus_get(obj);
}

static void
_show_region_hook(void *data,
                  Evas_Object *content_obj)
{
   Evas_Coord x, y, w, h;

   elm_widget_show_region_get(content_obj, &x, &y, &w, &h);
   elm_interface_scrollable_content_region_show(data, x, y, w, h);
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e EINA_UNUSED,
                       Evas_Object *obj EINA_UNUSED,
                       void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

EOLIAN static Eina_Bool
_elm_scroller_elm_widget_sub_object_del(Eo *obj, Elm_Scroller_Data *sd, Evas_Object *sobj)
{
   Eina_Bool int_ret = EINA_FALSE;
   int_ret = elm_obj_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   if (!int_ret) return EINA_FALSE;

   if (sobj == sd->content)
     {
        if (elm_widget_is(sobj))
          elm_widget_on_show_region_hook_set(sd->content, NULL, NULL);

        sd->content = NULL;
     }

   return EINA_TRUE;
}

static void
_resize_cb(void *data,
           Evas *e EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   elm_layout_sizing_eval(data);
}

static void
_edge_left_cb(Evas_Object *obj,
              void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_EDGE_LEFT, NULL);
}

static void
_edge_right_cb(Evas_Object *obj,
               void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_EDGE_RIGHT, NULL);
}

static void
_edge_top_cb(Evas_Object *obj,
             void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_EDGE_TOP, NULL);
}

static void
_edge_bottom_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_EDGE_BOTTOM, NULL);
}

static void
_scroll_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL, NULL);
}

static void
_scroll_left_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_SCROLL_LEFT, NULL);
}

static void
_scroll_right_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_SCROLL_RIGHT, NULL);
}

static void
_scroll_up_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_SCROLL_UP, NULL);
}

static void
_scroll_down_cb(Evas_Object *obj,
           void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_SCROLL_DOWN, NULL);
}

static void
_scroll_anim_start_cb(Evas_Object *obj,
                      void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL_ANIM_START, NULL);
}

static void
_scroll_anim_stop_cb(Evas_Object *obj,
                     void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, EFL_UI_EVENT_SCROLL_DRAG_STOP, NULL);
}

static void
_vbar_drag_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_VBAR_DRAG, NULL);
}

static void
_vbar_press_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_VBAR_PRESS, NULL);
}

static void
_vbar_unpress_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_VBAR_UNPRESS, NULL);
}

static void
_hbar_drag_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_HBAR_DRAG, NULL);
}

static void
_hbar_press_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_HBAR_PRESS, NULL);
}

static void
_hbar_unpress_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call(obj, ELM_SCROLLER_EVENT_HBAR_UNPRESS, NULL);
}

static void
_page_change_cb(Evas_Object *obj,
                void *data EINA_UNUSED)
{
   efl_event_callback_legacy_call
     (obj, ELM_SCROLLER_EVENT_SCROLL_PAGE_CHANGED, NULL);
}

static void
_loop_content_set(Evas_Object *obj, Elm_Scroller_Data *sd, Evas_Object *content)
{
   if (!sd->contents)
     {
        sd->contents = elm_layout_add(obj);
        evas_object_smart_member_add(sd->contents, obj);
        if (!elm_layout_theme_set(sd->contents, "scroller", "contents", elm_widget_style_get(obj)))
          CRI("Failed to set layout!");

        evas_object_size_hint_weight_set(sd->contents, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(sd->contents, EVAS_HINT_FILL, EVAS_HINT_FILL);

        elm_widget_sub_object_add(obj, sd->contents);
        elm_widget_on_show_region_hook_set(sd->contents, _show_region_hook, obj);

        efl_ui_mirrored_automatic_set(sd->contents, EINA_FALSE);
        efl_ui_mirrored_set(sd->contents, EINA_FALSE);
     }
   elm_object_part_content_set(sd->contents, "elm.swallow.content", content);
   sd->content = content;

   if (sd->loop_h)
     {
       if (!sd->proxy_content[0])
          sd->proxy_content[0] =
             evas_object_image_add(evas_object_evas_get(sd->contents));
        _elm_scroller_proxy_set(obj, sd, sd->proxy_content[0]);
        elm_object_part_content_set(sd->contents, "elm.swallow.content_r",
                                    sd->proxy_content[0]);
     }

   if (sd->loop_v)
     {
        if (!sd->proxy_content[1])
          sd->proxy_content[1] =
             evas_object_image_add(evas_object_evas_get(sd->contents));
        _elm_scroller_proxy_set(obj, sd, sd->proxy_content[1]);
        elm_object_part_content_set(sd->contents, "elm.swallow.content_b",
                                    sd->proxy_content[1]);
     }

   if (sd->loop_h && sd->loop_v)
     {
        if (!sd->proxy_content[2])
          sd->proxy_content[2] =
             evas_object_image_add(evas_object_evas_get(sd->contents));
        _elm_scroller_proxy_set(obj, sd, sd->proxy_content[2]);
        elm_object_part_content_set(sd->contents, "elm.swallow.content_rb",
                                    sd->proxy_content[2]);
     }
}

static Eina_Bool
_elm_scroller_content_set(Eo *obj, Elm_Scroller_Data *sd, const char *part, Evas_Object *content)
{
   if (part && strcmp(part, "default"))
     {
        return efl_content_set(efl_part(efl_super(obj, MY_CLASS), part), content);
     }

   if (sd->content == content) return EINA_TRUE;

   evas_object_del(sd->content);
   sd->content = content;

   if (content)
     {
        if (elm_widget_is(content))
          elm_widget_on_show_region_hook_set(content, _show_region_hook, obj);
        elm_widget_sub_object_add(obj, content);

        if (sd->loop_h || sd->loop_v)
          {
             _loop_content_set(obj, sd, content);
             if(sd->contents)
               content = sd->contents;
          }
        elm_interface_scrollable_content_set(obj, content);
     }
   else
     {
        int i;
        for (i = 0; i < 3; i ++)
          {
             if (!sd->proxy_content[i]) continue;
             evas_object_del(sd->proxy_content[i]);
             sd->proxy_content[i] = NULL;
          }
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object*
_elm_scroller_content_get(Eo *obj, Elm_Scroller_Data *sd, const char *part)
{
   if (part && strcmp(part, "default"))
     {
        return efl_content_get(efl_part(efl_super(obj, MY_CLASS), part));
     }

   return sd->content;
}

static Evas_Object*
_elm_scroller_content_unset(Eo *obj, Elm_Scroller_Data *sd, const char *part)
{
   Evas_Object *ret = NULL;
   if (part && strcmp(part, "default"))
     {
        return efl_content_unset(efl_part(efl_super(obj, MY_CLASS), part));
     }

   if (!sd->content) return NULL;

   ret = sd->content;
   if (sd->loop_h || sd->loop_v)
     _elm_widget_sub_object_redirect_to_top(obj, sd->contents);
   else
     _elm_widget_sub_object_redirect_to_top(obj, sd->content);
   elm_interface_scrollable_content_set(obj, NULL);
   sd->content = NULL;

   return ret;
}

EOLIAN static Eina_Bool
_elm_scroller_efl_container_content_set(Eo *obj, Elm_Scroller_Data *sd, Eo *content)
{
   return _elm_scroller_content_set(obj, sd, "default", content);
}

EOLIAN static Eo *
_elm_scroller_efl_container_content_get(Eo *obj, Elm_Scroller_Data *sd)
{
   return _elm_scroller_content_get(obj, sd, "default");
}

EOLIAN static Eo *
_elm_scroller_efl_container_content_unset(Eo *obj, Elm_Scroller_Data *sd)
{
   return _elm_scroller_content_unset(obj, sd, "default");
}

static void
_elm_scroller_content_min_limit_cb(Evas_Object *obj,
                                   Eina_Bool w,
                                   Eina_Bool h)
{
   ELM_SCROLLER_DATA_GET(obj, sd);

   sd->min_w = !!w;
   sd->min_h = !!h;

   elm_layout_sizing_eval(obj);
}

static void
_elm_scroller_content_viewport_resize_cb(Evas_Object *obj,
                                   Evas_Coord w EINA_UNUSED,
                                   Evas_Coord h EINA_UNUSED)
{
   elm_layout_sizing_eval(obj);
}

EOLIAN static void
_elm_scroller_efl_canvas_group_group_add(Eo *obj, Elm_Scroller_Data *priv)
{
   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd);
   Evas_Coord minw, minh;

   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);
   elm_widget_can_focus_set(obj, EINA_TRUE);

   if (!elm_layout_theme_set
       (obj, "scroller", "base", elm_widget_style_get(obj)))
     CRI("Failed to set layout!");

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   elm_interface_scrollable_objects_set(obj, wd->resize_obj, priv->hit_rect);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints_cb, obj);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize_cb, obj);

   _mirrored_set(obj, efl_ui_mirrored_get(obj));

   elm_interface_scrollable_edge_left_cb_set(obj, _edge_left_cb);
   elm_interface_scrollable_edge_right_cb_set(obj, _edge_right_cb);
   elm_interface_scrollable_edge_top_cb_set(obj, _edge_top_cb);
   elm_interface_scrollable_edge_bottom_cb_set(obj, _edge_bottom_cb);
   elm_interface_scrollable_scroll_cb_set(obj, _scroll_cb);
   elm_interface_scrollable_scroll_left_cb_set(obj, _scroll_left_cb);
   elm_interface_scrollable_scroll_right_cb_set(obj, _scroll_right_cb);
   elm_interface_scrollable_scroll_up_cb_set(obj, _scroll_up_cb);
   elm_interface_scrollable_scroll_down_cb_set(obj, _scroll_down_cb);
   elm_interface_scrollable_animate_start_cb_set(obj, _scroll_anim_start_cb);
   elm_interface_scrollable_animate_stop_cb_set(obj, _scroll_anim_stop_cb);
   elm_interface_scrollable_drag_start_cb_set(obj, _scroll_drag_start_cb);
   elm_interface_scrollable_drag_stop_cb_set(obj, _scroll_drag_stop_cb);
   elm_interface_scrollable_vbar_drag_cb_set(obj, _vbar_drag_cb);
   elm_interface_scrollable_vbar_press_cb_set(obj, _vbar_press_cb);
   elm_interface_scrollable_vbar_unpress_cb_set(obj, _vbar_unpress_cb);
   elm_interface_scrollable_hbar_drag_cb_set(obj, _hbar_drag_cb);
   elm_interface_scrollable_hbar_press_cb_set(obj, _hbar_press_cb);
   elm_interface_scrollable_hbar_unpress_cb_set(obj, _hbar_unpress_cb);
   elm_interface_scrollable_page_change_cb_set(obj, _page_change_cb);
   elm_interface_scrollable_content_min_limit_cb_set
         (obj, _elm_scroller_content_min_limit_cb);
   elm_interface_scrollable_content_viewport_resize_cb_set
         (obj, _elm_scroller_content_viewport_resize_cb);
}

EOLIAN static void
_elm_scroller_efl_gfx_position_set(Eo *obj, Elm_Scroller_Data *sd, Evas_Coord x, Evas_Coord y)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 0, x, y))
     return;

   efl_gfx_position_set(efl_super(obj, MY_CLASS), x, y);

   evas_object_move(sd->hit_rect, x, y);
}

EOLIAN static void
_elm_scroller_efl_gfx_size_set(Eo *obj, Elm_Scroller_Data *sd, Evas_Coord w, Evas_Coord h)
{
   if (_evas_object_intercept_call(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, w, h))
     return;

   efl_gfx_size_set(efl_super(obj, MY_CLASS), w, h);

   evas_object_resize(sd->hit_rect, w, h);
}

EOLIAN static void
_elm_scroller_efl_canvas_group_group_member_add(Eo *obj, Elm_Scroller_Data *sd, Evas_Object *member)
{
   efl_canvas_group_member_add(efl_super(obj, MY_CLASS), member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EAPI Evas_Object *
elm_scroller_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return efl_add(MY_CLASS, parent, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_elm_scroller_efl_object_constructor(Eo *obj, Elm_Scroller_Data *_pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   evas_object_smart_callbacks_descriptions_set(obj, _smart_callbacks);
   elm_interface_atspi_accessible_role_set(obj, ELM_ATSPI_ROLE_SCROLL_PANE);

   return obj;
}

/* deprecated */
EOLIAN static void
_elm_scroller_custom_widget_base_theme_set(Eo *obj, Elm_Scroller_Data *_pd EINA_UNUSED, const char *klass, const char *group)
{
   ELM_LAYOUT_DATA_GET(obj, ld);

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(group);

   if (eina_stringshare_replace(&(ld->klass), klass) ||
       eina_stringshare_replace(&(ld->group), group))
      elm_obj_widget_theme_apply(obj);
}

EAPI void
elm_scroller_content_min_limit(Evas_Object *obj,
                               Eina_Bool w,
                               Eina_Bool h)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_content_min_limit(obj, w, h);
}

EAPI void
elm_scroller_region_show(Evas_Object *obj,
                         Evas_Coord x,
                         Evas_Coord y,
                         Evas_Coord w,
                         Evas_Coord h)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_content_region_show(obj, x, y, w, h);
}

EAPI void
elm_scroller_policy_set(Evas_Object *obj,
                        Elm_Scroller_Policy policy_h,
                        Elm_Scroller_Policy policy_v)
{
   ELM_SCROLLABLE_CHECK(obj);
   elm_interface_scrollable_policy_set(obj, policy_h, policy_v);
}

EOLIAN static void
_elm_scroller_elm_interface_scrollable_policy_set(Eo *obj, Elm_Scroller_Data *sd EINA_UNUSED, Elm_Scroller_Policy policy_h, Elm_Scroller_Policy policy_v)
{
   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   elm_interface_scrollable_policy_set(efl_super(obj, MY_CLASS), policy_h, policy_v);
}

EAPI void
elm_scroller_policy_get(const Evas_Object *obj,
                        Elm_Scroller_Policy *policy_h,
                        Elm_Scroller_Policy *policy_v)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_policy_get((Eo *) obj, policy_h, policy_v);
}

EAPI void
elm_scroller_single_direction_set(Evas_Object *obj,
                                  Elm_Scroller_Single_Direction single_dir)
{
   ELM_SCROLLABLE_CHECK(obj);
   elm_interface_scrollable_single_direction_set(obj, single_dir);
}

EOLIAN static void
_elm_scroller_elm_interface_scrollable_single_direction_set(Eo *obj, Elm_Scroller_Data *sd EINA_UNUSED, Elm_Scroller_Single_Direction single_dir)
{
   if (single_dir >= ELM_SCROLLER_SINGLE_DIRECTION_LAST)
     return;

   elm_interface_scrollable_single_direction_set(efl_super(obj, MY_CLASS), single_dir);
}

EAPI Elm_Scroller_Single_Direction
elm_scroller_single_direction_get(const Evas_Object *obj)
{
   ELM_SCROLLABLE_CHECK(obj, ELM_SCROLLER_SINGLE_DIRECTION_NONE);
   Elm_Scroller_Single_Direction single_dir = ELM_SCROLLER_SINGLE_DIRECTION_NONE;

   single_dir = elm_interface_scrollable_single_direction_get((Eo *) obj);
   return single_dir;
}

EOLIAN static Elm_Scroller_Single_Direction
_elm_scroller_elm_interface_scrollable_single_direction_get(Eo *obj, Elm_Scroller_Data *sd EINA_UNUSED)
{
   return elm_interface_scrollable_single_direction_get(efl_super(obj, MY_CLASS));
}

EAPI void
elm_scroller_region_get(const Evas_Object *obj,
                        Evas_Coord *x,
                        Evas_Coord *y,
                        Evas_Coord *w,
                        Evas_Coord *h)
{
   ELM_SCROLLABLE_CHECK(obj);
   elm_interface_scrollable_content_region_get((Eo *) obj, x, y, w, h);
}

EAPI void
elm_scroller_child_size_get(const Evas_Object *obj,
                            Evas_Coord *w,
                            Evas_Coord *h)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_content_size_get((Eo *) obj, w, h);
}

EAPI void
elm_scroller_page_snap_set(Evas_Object *obj,
                             Eina_Bool page_snap_h,
                             Eina_Bool page_snap_v)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_page_snap_allow_set
         (obj, page_snap_h, page_snap_v);
}

EAPI void
elm_scroller_page_snap_get(const Evas_Object *obj,
                             Eina_Bool *page_snap_h,
                             Eina_Bool *page_snap_v)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_page_snap_allow_get
         ((Eo *) obj, page_snap_h, page_snap_v);
}

EAPI void
elm_scroller_bounce_set(Evas_Object *obj,
                        Eina_Bool h_bounce,
                        Eina_Bool v_bounce)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_bounce_allow_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_scroller_bounce_get(const Evas_Object *obj,
                        Eina_Bool *h_bounce,
                        Eina_Bool *v_bounce)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_bounce_allow_get((Eo *) obj, h_bounce, v_bounce);
}

EAPI void
elm_scroller_page_relative_set(Evas_Object *obj,
                               double h_pagerel,
                               double v_pagerel)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_page_relative_set((Eo *)obj, h_pagerel, v_pagerel);
}

EAPI void
elm_scroller_page_relative_get(const Evas_Object *obj,
                               double *h_pagerel,
                               double *v_pagerel)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_paging_get((Eo *)obj, h_pagerel, v_pagerel, NULL, NULL);
}

EAPI void
elm_scroller_page_size_set(Evas_Object *obj,
                           Evas_Coord h_pagesize,
                           Evas_Coord v_pagesize)
{
   ELM_SCROLLABLE_CHECK(obj);
   elm_interface_scrollable_page_size_set(obj, h_pagesize, v_pagesize);
}

EOLIAN static void
_elm_scroller_elm_interface_scrollable_page_size_set(Eo *obj, Elm_Scroller_Data *sd EINA_UNUSED, Evas_Coord h_pagesize, Evas_Coord v_pagesize)
{
   double pagerel_h = 0.0, pagerel_v = 0.0;

   elm_interface_scrollable_paging_get(obj, &pagerel_h, &pagerel_v, NULL, NULL);

   elm_interface_scrollable_paging_set(obj, pagerel_h, pagerel_v, h_pagesize, v_pagesize);
}

EAPI void
elm_scroller_page_size_get(const Evas_Object *obj,
                           Evas_Coord *h_pagesize,
                           Evas_Coord *v_pagesize)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_paging_get((Eo *)obj, NULL, NULL, h_pagesize, v_pagesize);
}

EOLIAN static void
_elm_scroller_page_scroll_limit_set(const Eo *obj, Elm_Scroller_Data *_pd EINA_UNUSED, int page_limit_h, int page_limit_v)
{
   if (page_limit_h < 1)
     page_limit_h = 9999;
   if (page_limit_v < 1)
     page_limit_v = 9999;

   elm_interface_scrollable_page_scroll_limit_set
         ((Eo *) obj, page_limit_h, page_limit_v);
}

EOLIAN static void
_elm_scroller_page_scroll_limit_get(const Eo *obj, Elm_Scroller_Data *_pd EINA_UNUSED, int *page_limit_h, int *page_limit_v)
{
   elm_interface_scrollable_page_scroll_limit_get
         (obj, page_limit_h, page_limit_v);
}

EAPI void
elm_scroller_current_page_get(const Evas_Object *obj,
                              int *h_pagenumber,
                              int *v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_current_page_get((Eo *) obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_last_page_get(const Evas_Object *obj,
                           int *h_pagenumber,
                           int *v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_last_page_get((Eo *) obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_page_show(Evas_Object *obj,
                       int h_pagenumber,
                       int v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_page_show(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_page_bring_in(Evas_Object *obj,
                           int h_pagenumber,
                           int v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_page_bring_in(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_region_bring_in(Evas_Object *obj,
                             Evas_Coord x,
                             Evas_Coord y,
                             Evas_Coord w,
                             Evas_Coord h)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_region_bring_in(obj, x, y, w, h);
}

EAPI void
elm_scroller_gravity_set(Evas_Object *obj,
                         double x,
                         double y)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_gravity_set(obj, x, y);
}

EAPI void
elm_scroller_gravity_get(const Evas_Object *obj,
                         double *x,
                         double *y)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_gravity_get((Eo *) obj, x, y);
}

EAPI void
elm_scroller_movement_block_set(Evas_Object *obj,
                                Elm_Scroller_Movement_Block block)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_movement_block_set(obj, block);
}

EAPI Elm_Scroller_Movement_Block
elm_scroller_movement_block_get(const Evas_Object *obj)
{
   Elm_Scroller_Movement_Block block = ELM_SCROLLER_MOVEMENT_NO_BLOCK;

   ELM_SCROLLABLE_CHECK(obj, ELM_SCROLLER_MOVEMENT_NO_BLOCK);

   block = elm_interface_scrollable_movement_block_get((Eo *) obj);

   return block;
}

EAPI void
elm_scroller_step_size_set(Evas_Object *obj,
                           Evas_Coord x,
                           Evas_Coord y)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_step_size_set(obj, x, y);
}

EAPI void
elm_scroller_step_size_get(const Evas_Object *obj,
                           Evas_Coord *x,
                           Evas_Coord *y)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_step_size_get((Eo *) obj, x, y);
}

EAPI void
elm_scroller_loop_set(Evas_Object *obj,
                      Eina_Bool loop_h,
                      Eina_Bool loop_v)
{
   ELM_SCROLLABLE_CHECK(obj);
   ELM_SCROLLER_DATA_GET(obj, sd);

   int i;

   if (sd->loop_h == loop_h && sd->loop_v == loop_v) return;

   sd->loop_h = loop_h;
   sd->loop_v = loop_v;

   elm_interface_scrollable_loop_set(obj, loop_h, loop_v);

   if (sd->content)
     {
        if (sd->loop_h || sd->loop_v)
          {
             elm_interface_scrollable_content_set(obj, NULL);
             _loop_content_set(obj, sd, sd->content);

             if (sd->contents)
               {
                  elm_interface_scrollable_content_set(obj, sd->contents);
                  elm_widget_sub_object_add(obj, sd->contents);
                  elm_widget_on_show_region_hook_set(sd->contents, _show_region_hook, obj);
               }
          }
        else
          {
             for (i = 0 ; i < 3 ; i++)
               {
                  if (sd->proxy_content[i])
                    {
                       evas_object_del(sd->proxy_content[i]);
                       sd->proxy_content[i]= NULL;
                    }
               }
          }
     }
   elm_layout_sizing_eval(obj);
}

EAPI void
elm_scroller_loop_get(const Evas_Object *obj,
                      Eina_Bool *loop_h,
                      Eina_Bool *loop_v)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_loop_get(obj, loop_h, loop_v);
}

EAPI void
elm_scroller_wheel_disabled_set(Evas_Object *obj, Eina_Bool disabled)
{
   ELM_SCROLLABLE_CHECK(obj);

   elm_interface_scrollable_wheel_disabled_set((Eo *)obj, disabled);
}

EAPI Eina_Bool
elm_scroller_wheel_disabled_get(const Evas_Object *obj)
{
   ELM_SCROLLABLE_CHECK(obj, EINA_FALSE);

   return elm_interface_scrollable_wheel_disabled_get((Eo *) obj);
}

EAPI void
elm_scroller_propagate_events_set(Evas_Object *obj, Eina_Bool propagation)
{
   evas_object_propagate_events_set(elm_layout_edje_get(obj), propagation);
}

EAPI Eina_Bool
elm_scroller_propagate_events_get(const Evas_Object *obj)
{
   return evas_object_propagate_events_get(elm_layout_edje_get(obj));
}

static void
_elm_scroller_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eina_Bool
_elm_scroller_elm_widget_focus_register(Eo *obj, Elm_Scroller_Data *pd EINA_UNUSED, Efl_Ui_Focus_Manager *manager, Efl_Ui_Focus_Object *logical, Eina_Bool *logical_flag)
{
   //undepended from logical or not we always reigster as full with ourself as redirect
   *logical_flag = EINA_TRUE;
   return efl_ui_focus_manager_register_logical(manager, obj, logical, obj);
}


EOLIAN const Elm_Atspi_Action *
_elm_scroller_elm_interface_atspi_widget_action_elm_actions_get(Eo *obj EINA_UNUSED, Elm_Scroller_Data *pd EINA_UNUSED)
{
   static Elm_Atspi_Action atspi_actions[] = {
          { "move,prior", "move", "prior", _key_action_move},
          { "move,next", "move", "next", _key_action_move},
          { "move,left", "move", "left", _key_action_move},
          { "move,right", "move", "right", _key_action_move},
          { "move,up", "move", "up", _key_action_move},
          { "move,down", "move", "down", _key_action_move},
          { "move,first", "move", "first", _key_action_move},
          { "move,last", "move", "last", _key_action_move},
          { NULL, NULL, NULL, NULL }
   };
   return &atspi_actions[0];
}

/* Efl.Part begin */

ELM_PART_OVERRIDE(elm_scroller, ELM_SCROLLER, ELM_LAYOUT, Elm_Scroller_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_SET(elm_scroller, ELM_SCROLLER, ELM_LAYOUT, Elm_Scroller_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_GET(elm_scroller, ELM_SCROLLER, ELM_LAYOUT, Elm_Scroller_Data, Elm_Part_Data)
ELM_PART_OVERRIDE_CONTENT_UNSET(elm_scroller, ELM_SCROLLER, ELM_LAYOUT, Elm_Scroller_Data, Elm_Part_Data)
// FIXME: should be "content" but "default" was legacy API
ELM_PART_CONTENT_DEFAULT_SET(elm_scroller, "default")
#include "elm_scroller_internal_part.eo.c"

/* Efl.Part end */

/* Internal EO APIs and hidden overrides */

#define ELM_SCROLLER_EXTRA_OPS \
   ELM_PART_CONTENT_DEFAULT_OPS(elm_scroller), \
   EFL_CANVAS_GROUP_ADD_OPS(elm_scroller)

#include "elm_scroller.eo.c"
