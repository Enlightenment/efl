#include <Elementary.h>
#include "elm_priv.h"
#include "elm_interface_scrollable.h"
#include "elm_widget_layout.h"
#include "elm_widget_scroller.h"

EAPI Eo_Op ELM_OBJ_SCROLLER_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_SCROLLER_CLASS

#define MY_CLASS_NAME "elm_scroller"

static const char SIG_SCROLL[] = "scroll";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
static const char SIG_VBAR_DRAG[] = "vbar,drag";
static const char SIG_VBAR_PRESS[] = "vbar,press";
static const char SIG_VBAR_UNPRESS[] = "vbar,unpress";
static const char SIG_HBAR_DRAG[] = "hbar,drag";
static const char SIG_HBAR_PRESS[] = "hbar,press";
static const char SIG_HBAR_UNPRESS[] = "hbar,unpress";
static const Evas_Smart_Cb_Description _smart_callbacks[] =
{
   {SIG_SCROLL, ""},
   {SIG_SCROLL_ANIM_START, ""},
   {SIG_SCROLL_ANIM_STOP, ""},
   {SIG_SCROLL_DRAG_START, ""},
   {SIG_SCROLL_DRAG_STOP, ""},
   {SIG_EDGE_LEFT, ""},
   {SIG_EDGE_RIGHT, ""},
   {SIG_EDGE_TOP, ""},
   {SIG_EDGE_BOTTOM, ""},
   {SIG_VBAR_DRAG, ""},
   {SIG_VBAR_PRESS, ""},
   {SIG_VBAR_UNPRESS, ""},
   {SIG_HBAR_DRAG, ""},
   {SIG_HBAR_PRESS, ""},
   {SIG_HBAR_UNPRESS, ""},
   {NULL, NULL}
};

static void
_elm_scroller_smart_event(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *src = va_arg(*list, Evas_Object *);
   (void) src;
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   void *event_info = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord c_x = 0;
   Evas_Coord c_y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord max_x = 0;
   Evas_Coord max_y = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;
   Evas_Coord step_x = 0;
   Evas_Coord step_y = 0;
   Evas_Event_Key_Down *ev = event_info;

   Elm_Scroller_Smart_Data *sd = _pd;

   if (elm_widget_disabled_get(obj)) return;
   if (type != EVAS_CALLBACK_KEY_DOWN) return;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return;

   eo_do(obj,
         elm_scrollable_interface_content_pos_get(&x, &y),
         elm_scrollable_interface_step_size_get(&step_x, &step_y),
         elm_scrollable_interface_page_size_get(&page_x, &page_y),
         elm_scrollable_interface_content_viewport_size_get(&v_w, &v_h));
   eo_do(sd->content,
         evas_obj_position_get(&c_x, &c_y),
         evas_obj_size_get(&max_x, &max_y));

   if (((!strcmp(ev->keyname, "Left")) ||
        (!strcmp(ev->keyname, "KP_Left")) ||
        (!strcmp(ev->keyname, "Right")) ||
        (!strcmp(ev->keyname, "KP_Right")) ||
        (!strcmp(ev->keyname, "Up")) ||
        (!strcmp(ev->keyname, "KP_Up")) ||
        (!strcmp(ev->keyname, "Down")) ||
        (!strcmp(ev->keyname, "KP_Down"))) && (!ev->string))
     {
        Evas_Object *current_focus = NULL;
        Eina_List *can_focus_list = NULL;
        Evas_Object *new_focus = NULL;
        Evas_Coord f_x = 0;
        Evas_Coord f_y = 0;
        Evas_Coord f_w = 0;
        Evas_Coord f_h = 0;

        current_focus = elm_widget_focused_object_get(obj);
        evas_object_geometry_get(current_focus, &f_x, &f_y, &f_w, &f_h);
        can_focus_list = elm_widget_can_focus_child_list_get(obj);
        if ((current_focus == obj) ||
            (!ELM_RECTS_INTERSECT
               (x, y, v_w, v_h, (f_x - c_x), (f_y - c_y), f_w, f_h)))
          {
             Eina_List *l;
             Evas_Object *cur;
             double weight = 0.0;

             EINA_LIST_FOREACH(can_focus_list, l, cur)
               {
                  double cur_weight = 0.0;

                  evas_object_geometry_get(cur, &f_x, &f_y, &f_w, &f_h);
                  if (ELM_RECTS_INTERSECT
                        (x, y, v_w, v_h, (f_x - c_x), (f_y - c_y), f_w, f_h))
                    {
                       if ((f_x - c_x) > x)
                         cur_weight += ((f_x - c_x) - x) * ((f_x - c_x) - x);
                       if ((f_y - c_y) > y)
                         cur_weight += ((f_y - c_y) - y) * ((f_y - c_y) - y);
                       if (cur_weight == 0.0)
                         {
                            elm_widget_focus_steal(cur);
                            ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                            if (ret) *ret = EINA_TRUE;
                            return;
                         }
                       cur_weight = 1.0 / cur_weight;
                       if (cur_weight > weight)
                         {
                            new_focus = cur;
                            weight = cur_weight;
                         }
                    }
               }
             if (new_focus)
               {
                  elm_widget_focus_steal(new_focus);
                  ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                  if (ret) *ret = EINA_TRUE;
                  return;
               }
          }
        else
          {
             Evas_Object *tmp = NULL;
             double degree = 0.0, weight = 0.0;
             void *(*list_data_get)(const Eina_List *list);

             list_data_get = eina_list_data_get;

             if ((!strcmp(ev->keyname, "Left")) ||
                 (!strcmp(ev->keyname, "KP_Left")))
               degree = 270.0;
             else if ((!strcmp(ev->keyname, "Right")) ||
                      (!strcmp(ev->keyname, "KP_Right")))
               degree = 90.0;
             else if ((!strcmp(ev->keyname, "Up")) ||
                      (!strcmp(ev->keyname, "KP_Up")))
               degree = 0.0;
             else if ((!strcmp(ev->keyname, "Down")) ||
                      (!strcmp(ev->keyname, "KP_Down")))
               degree = 180.0;

             if (elm_widget_focus_list_direction_get
                   (obj, current_focus, can_focus_list, list_data_get, degree,
                   &tmp, &weight))
               new_focus = tmp;

             if (new_focus)
               {
                  Evas_Coord l_x = 0;
                  Evas_Coord l_y = 0;
                  Evas_Coord l_w = 0;
                  Evas_Coord l_h = 0;

                  evas_object_geometry_get(new_focus, &f_x, &f_y, &f_w, &f_h);
                  l_x = f_x - c_x - step_x;
                  l_y = f_y - c_y - step_y;
                  l_w = f_w + (step_x * 2);
                  l_h = f_h + (step_y * 2);

                  if (ELM_RECTS_INTERSECT(x, y, v_w, v_h, l_x, l_y, l_w, l_h))
                    {
                       elm_widget_focus_steal(new_focus);
                       ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
                       if (ret) *ret = EINA_TRUE;
                       return;
                    }
               }
          }
     }
   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        if (x <= 0) return;
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        if (x >= (max_x - v_w)) return;
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        if (y == 0) return;
        y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        if (y >= (max_y - v_h)) return;
        y += step_y;
     }
   else if ((!strcmp(ev->keyname, "Home")) ||
            ((!strcmp(ev->keyname, "KP_Home")) && (!ev->string)))
     {
        y = 0;
     }
   else if ((!strcmp(ev->keyname, "End")) ||
            ((!strcmp(ev->keyname, "KP_End")) && (!ev->string)))
     {
        y = max_y - v_h;
     }
   else if ((!strcmp(ev->keyname, "Prior")) ||
            ((!strcmp(ev->keyname, "KP_Prior")) && (!ev->string)))
     {
        if (page_y < 0)
          y -= -(page_y * v_h) / 100;
        else
          y -= page_y;
     }
   else if ((!strcmp(ev->keyname, "Next")) ||
            ((!strcmp(ev->keyname, "KP_Next")) && (!ev->string)))
     {
        if (page_y < 0)
          y += -(page_y * v_h) / 100;
        else
          y += page_y;
     }
   else return;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   eo_do(obj, elm_scrollable_interface_content_pos_set(x, y, EINA_TRUE));

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_scroller_smart_activate(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Activate act = va_arg(*list, Elm_Activate);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   if ((elm_widget_disabled_get(obj)) ||
       (act == ELM_ACTIVATE_DEFAULT)) return;

   eo_do(obj,
         elm_scrollable_interface_content_pos_get(&x, &y),
         elm_scrollable_interface_page_size_get(&page_x, &page_y),
         elm_scrollable_interface_content_viewport_size_get(&v_w, &v_h));

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

   eo_do(obj, elm_scrollable_interface_content_pos_set(x, y, EINA_TRUE));
   if (ret) *ret = EINA_TRUE;
}

static void
_elm_scroller_smart_sizing_eval(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Coord vw = 0, vh = 0, minw = 0, minh = 0, maxw = 0, maxh = 0, w, h,
              vmw, vmh;
   double xw = 0.0, yw = 0.0;

   Elm_Scroller_Smart_Data *sd = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   if (sd->content)
     {
        evas_object_size_hint_min_get(sd->content, &minw, &minh);
        evas_object_size_hint_max_get(sd->content, &maxw, &maxh);
        evas_object_size_hint_weight_get(sd->content, &xw, &yw);
     }

   eo_do(obj, elm_scrollable_interface_content_viewport_size_get(&vw, &vh));
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

   w = -1;
   h = -1;
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
   eo_do(obj, elm_scrollable_interface_mirrored_set(mirrored));
}

static void
_elm_scroller_smart_theme(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;

   eo_do_super(obj, elm_wdg_theme(&int_ret));
   if (!int_ret) return;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   elm_layout_sizing_eval(obj);

   if (ret) *ret = EINA_TRUE;
}

static void
_elm_scroller_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_scroller_smart_focus_next(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;
   Evas_Object *cur;

   Elm_Scroller_Smart_Data *sd = _pd;

   if (!sd->content) return;

   cur = sd->content;

   /* access */
   if (_elm_config->access_mode)
     {
        if ((elm_widget_can_focus_get(cur)) ||
            (elm_widget_child_can_focus_get(cur)))
          {
             int_ret = elm_widget_focus_next_get(cur, dir, next);
             if (ret) *ret = int_ret;
             return;
          }

        return;
     }

   /* Try focus cycle in subitem */
   if (elm_widget_focus_get(obj))
     {
        if ((elm_widget_can_focus_get(cur)) ||
            (elm_widget_child_can_focus_get(cur)))
          {
             int_ret = elm_widget_focus_next_get(cur, dir, next);
             if (ret) *ret = int_ret;
             return;
          }
     }

   /* Return */
   *next = (Evas_Object *)obj;

   int_ret = !elm_widget_focus_get(obj);
   if (ret) *ret = int_ret;
}

static void
_elm_scroller_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_FALSE;
}

static void
_show_region_hook(void *data,
                  Evas_Object *content_obj)
{
   Evas_Coord x, y, w, h;

   elm_widget_show_region_get(content_obj, &x, &y, &w, &h);
   eo_do(data, elm_scrollable_interface_content_region_show(x, y, w, h));
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_elm_scroller_smart_sub_object_del(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *sobj = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret;
   Elm_Scroller_Smart_Data *sd = _pd;

   eo_do_super(obj, elm_wdg_sub_object_del(sobj, &int_ret));
   if (!int_ret) return;

   if (sobj == sd->content)
     {
        elm_widget_on_show_region_hook_set(sd->content, NULL, NULL);

        sd->content = NULL;
     }

   if (ret) *ret = EINA_TRUE;
}

static void
_resize_cb(void *data,
           Evas *e __UNUSED__,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static void
_edge_left_cb(Evas_Object *obj,
              void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_LEFT, NULL);
}

static void
_edge_right_cb(Evas_Object *obj,
               void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_RIGHT, NULL);
}

static void
_edge_top_cb(Evas_Object *obj,
             void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_TOP, NULL);
}

static void
_edge_bottom_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_EDGE_BOTTOM, NULL);
}

static void
_scroll_cb(Evas_Object *obj,
           void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL, NULL);
}

static void
_scroll_anim_start_cb(Evas_Object *obj,
                      void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_START, NULL);
}

static void
_scroll_anim_stop_cb(Evas_Object *obj,
                     void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_ANIM_STOP, NULL);
}

static void
_scroll_drag_start_cb(Evas_Object *obj,
                      void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_START, NULL);
}

static void
_scroll_drag_stop_cb(Evas_Object *obj,
                     void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_SCROLL_DRAG_STOP, NULL);
}

static void
_vbar_drag_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_VBAR_DRAG, NULL);
}

static void
_vbar_press_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_VBAR_PRESS, NULL);
}

static void
_vbar_unpress_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_VBAR_UNPRESS, NULL);
}

static void
_hbar_drag_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_HBAR_DRAG, NULL);
}

static void
_hbar_press_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_HBAR_PRESS, NULL);
}

static void
_hbar_unpress_cb(Evas_Object *obj,
                void *data __UNUSED__)
{
   evas_object_smart_callback_call(obj, SIG_HBAR_UNPRESS, NULL);
}

static void
_elm_scroller_smart_content_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object *content = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_TRUE;
   Eina_Bool int_ret;

   Elm_Scroller_Smart_Data *sd = _pd;

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, elm_obj_container_content_set(part, content, &int_ret));
        if (ret) *ret = int_ret;
        return;
     }

   if (sd->content == content) return;

   if (sd->content) evas_object_del(sd->content);
   sd->content = content;

   if (content)
     {
        elm_widget_on_show_region_hook_set(content, _show_region_hook, obj);
        elm_widget_sub_object_add(obj, content);

        eo_do(obj, elm_scrollable_interface_content_set(content));
     }

   elm_layout_sizing_eval(obj);

   return;
}

static void
_elm_scroller_smart_content_get(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);

   Elm_Scroller_Smart_Data *sd = _pd;

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, elm_obj_container_content_get(part, ret));
        return;
     }

   *ret = sd->content;
}

static void
_elm_scroller_smart_content_unset(Eo *obj, void *_pd, va_list *list)
{
   const char *part = va_arg(*list, const char *);
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   if (ret) *ret = NULL;
   Evas_Object *int_ret;

   Elm_Scroller_Smart_Data *sd = _pd;

   if (part && strcmp(part, "default"))
     {
        eo_do_super(obj, elm_obj_container_content_unset(part, &int_ret));
        return;
     }

   if (!sd->content) return;

   int_ret = sd->content;
   if (ret) *ret = int_ret;
   elm_widget_sub_object_del(obj, sd->content);
   eo_do(obj, elm_scrollable_interface_content_set(NULL));
   sd->content = NULL;
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
_elm_scroller_smart_add(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Coord minw, minh;

   Elm_Scroller_Smart_Data *priv = _pd;
   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   eo_do_super(obj, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_theme_set(obj, "scroller", "base", elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   eo_do(obj, elm_scrollable_interface_objects_set(wd->resize_obj, priv->hit_rect));

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints_cb, obj);

   edje_object_size_min_calc(wd->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize_cb, obj);

   eo_do(obj,
         elm_scrollable_interface_edge_left_cb_set(_edge_left_cb),
         elm_scrollable_interface_edge_right_cb_set(_edge_right_cb),
         elm_scrollable_interface_edge_top_cb_set(_edge_top_cb),
         elm_scrollable_interface_edge_bottom_cb_set(_edge_bottom_cb),
         elm_scrollable_interface_scroll_cb_set(_scroll_cb),
         elm_scrollable_interface_animate_start_cb_set(_scroll_anim_start_cb),
         elm_scrollable_interface_animate_stop_cb_set(_scroll_anim_stop_cb),
         elm_scrollable_interface_drag_start_cb_set(_scroll_drag_start_cb),
         elm_scrollable_interface_drag_stop_cb_set(_scroll_drag_stop_cb),
         elm_scrollable_interface_vbar_drag_cb_set(_vbar_drag_cb),
         elm_scrollable_interface_vbar_press_cb_set(_vbar_press_cb),
         elm_scrollable_interface_vbar_unpress_cb_set(_vbar_unpress_cb),
         elm_scrollable_interface_hbar_drag_cb_set(_hbar_drag_cb),
         elm_scrollable_interface_hbar_press_cb_set(_hbar_press_cb),
         elm_scrollable_interface_hbar_unpress_cb_set(_hbar_unpress_cb),
         elm_scrollable_interface_content_min_limit_cb_set(_elm_scroller_content_min_limit_cb));
}

static void
_elm_scroller_smart_move(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Elm_Scroller_Smart_Data *sd = _pd;

   eo_do_super(obj, evas_obj_smart_move(x, y));

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_scroller_smart_resize(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);
   Elm_Scroller_Smart_Data *sd = _pd;

   eo_do_super(obj, evas_obj_smart_resize(w, h));

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_scroller_smart_member_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *member = va_arg(*list, Evas_Object *);
   Elm_Scroller_Smart_Data *sd = _pd;

   eo_do_super(obj, evas_obj_smart_member_add(member));

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

EAPI Evas_Object *
elm_scroller_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = eo_add(MY_CLASS, parent);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));

   Evas_Object *parent = eo_parent_get(obj);
   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);
}

/* deprecated */
EAPI void
elm_scroller_custom_widget_base_theme_set(Evas_Object *obj,
                                          const char *klass,
                                          const char *group)
{
   ELM_SCROLLER_CHECK(obj);
   eo_do(obj, elm_obj_scroller_custom_widget_base_theme_set(klass, group));
}

static void
_custom_widget_base_theme_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *klass = va_arg(*list, const char *);
   const char *group = va_arg(*list, const char *);
   Elm_Layout_Smart_Data *ld = eo_data_get(obj, ELM_OBJ_LAYOUT_CLASS);

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(group);

   if (eina_stringshare_replace(&(ld->klass), klass) ||
       eina_stringshare_replace(&(ld->group), group))
      eo_do(obj, elm_wdg_theme(NULL));
}

EAPI void
elm_scroller_content_min_limit(Evas_Object *obj,
                               Eina_Bool w,
                               Eina_Bool h)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do(obj, elm_scrollable_interface_content_min_limit(w, h));
}

EAPI void
elm_scroller_region_show(Evas_Object *obj,
                         Evas_Coord x,
                         Evas_Coord y,
                         Evas_Coord w,
                         Evas_Coord h)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do(obj, elm_scrollable_interface_content_region_show(x, y, w, h));
}

EAPI void
elm_scroller_policy_set(Evas_Object *obj,
                        Elm_Scroller_Policy policy_h,
                        Elm_Scroller_Policy policy_v)
{
   ELM_SCROLLABLE_CHECK(obj);
   eo_do(obj, elm_scrollable_interface_policy_set(policy_h, policy_v));
}

static void
_policy_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Scroller_Policy policy_h = va_arg(*list, Elm_Scroller_Policy);
   Elm_Scroller_Policy policy_v = va_arg(*list, Elm_Scroller_Policy);

   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   eo_do_super(obj, elm_scrollable_interface_policy_set(policy_h, policy_v));
}

EAPI void
elm_scroller_policy_get(const Evas_Object *obj,
                        Elm_Scroller_Policy *policy_h,
                        Elm_Scroller_Policy *policy_v)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do((Eo *) obj, elm_scrollable_interface_policy_get(policy_h, policy_v));
}

EAPI void
elm_scroller_region_get(const Evas_Object *obj,
                        Evas_Coord *x,
                        Evas_Coord *y,
                        Evas_Coord *w,
                        Evas_Coord *h)
{
   ELM_SCROLLABLE_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_scroller_region_get(x, y, w, h));
}

static void
_region_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);

   if ((x) || (y)) eo_do((Eo *) obj, elm_scrollable_interface_content_pos_get(x, y));
   if ((w) || (h)) eo_do((Eo *) obj, elm_scrollable_interface_content_viewport_size_get(w, h));
}

EAPI void
elm_scroller_child_size_get(const Evas_Object *obj,
                            Evas_Coord *w,
                            Evas_Coord *h)
{
   ELM_SCROLLABLE_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_scroller_child_size_get(w, h));
}

static void
_child_size_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);

   eo_do((Eo *) obj, elm_scrollable_interface_content_size_get(w, h));
}

EAPI void
elm_scroller_bounce_set(Evas_Object *obj,
                        Eina_Bool h_bounce,
                        Eina_Bool v_bounce)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do(obj, elm_scrollable_interface_bounce_allow_set(h_bounce, v_bounce));
}

EAPI void
elm_scroller_bounce_get(const Evas_Object *obj,
                        Eina_Bool *h_bounce,
                        Eina_Bool *v_bounce)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do((Eo *) obj, elm_scrollable_interface_bounce_allow_get(h_bounce, v_bounce));
}

EAPI void
elm_scroller_page_relative_set(Evas_Object *obj,
                               double h_pagerel,
                               double v_pagerel)
{
   ELM_SCROLLABLE_CHECK(obj);
   eo_do(obj, elm_obj_scroller_page_relative_set(h_pagerel, v_pagerel));
}

static void
_page_relative_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord pagesize_h, pagesize_v;

   double h_pagerel = va_arg(*list, double);
   double v_pagerel = va_arg(*list, double);

   eo_do(obj, elm_scrollable_interface_paging_get(NULL, NULL, &pagesize_h, &pagesize_v));

   eo_do(obj, elm_scrollable_interface_paging_set(h_pagerel, v_pagerel, pagesize_h, pagesize_v));
}

EAPI void
elm_scroller_page_relative_get(const Evas_Object *obj,
                               double *h_pagerel,
                               double *v_pagerel)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do((Eo *)obj, elm_scrollable_interface_paging_get(h_pagerel, v_pagerel, NULL, NULL));
}

EAPI void
elm_scroller_page_size_set(Evas_Object *obj,
                           Evas_Coord h_pagesize,
                           Evas_Coord v_pagesize)
{
   ELM_SCROLLABLE_CHECK(obj);
   eo_do(obj, elm_scrollable_interface_page_size_set(h_pagesize, v_pagesize));
}

static void
_page_size_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   double pagerel_h, pagerel_v;

   Evas_Coord h_pagesize = va_arg(*list, Evas_Coord);
   Evas_Coord v_pagesize = va_arg(*list, Evas_Coord);

   eo_do(obj, elm_scrollable_interface_paging_get(&pagerel_h, &pagerel_v, NULL, NULL));

   eo_do(obj, elm_scrollable_interface_paging_set(pagerel_h, pagerel_v, h_pagesize, v_pagesize));
}

EAPI void
elm_scroller_page_size_get(const Evas_Object *obj,
                           Evas_Coord *h_pagesize,
                           Evas_Coord *v_pagesize)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do((Eo *)obj, elm_scrollable_interface_paging_get(NULL, NULL, h_pagesize, v_pagesize));
}

EAPI void
elm_scroller_current_page_get(const Evas_Object *obj,
                              int *h_pagenumber,
                              int *v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do((Eo *) obj, elm_scrollable_interface_current_page_get(h_pagenumber, v_pagenumber));
}

EAPI void
elm_scroller_last_page_get(const Evas_Object *obj,
                           int *h_pagenumber,
                           int *v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do((Eo *) obj, elm_scrollable_interface_last_page_get(h_pagenumber, v_pagenumber));
}

EAPI void
elm_scroller_page_show(Evas_Object *obj,
                       int h_pagenumber,
                       int v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do(obj, elm_scrollable_interface_page_show(h_pagenumber, v_pagenumber));
}

EAPI void
elm_scroller_page_bring_in(Evas_Object *obj,
                           int h_pagenumber,
                           int v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do(obj, elm_scrollable_interface_page_bring_in(h_pagenumber, v_pagenumber));
}

EAPI void
elm_scroller_region_bring_in(Evas_Object *obj,
                             Evas_Coord x,
                             Evas_Coord y,
                             Evas_Coord w,
                             Evas_Coord h)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do(obj, elm_scrollable_interface_region_bring_in(x, y, w, h));
}

EAPI void
elm_scroller_gravity_set(Evas_Object *obj,
                         double x,
                         double y)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do(obj, elm_scrollable_interface_gravity_set(x, y));
}

EAPI void
elm_scroller_gravity_get(const Evas_Object *obj,
                         double *x,
                         double *y)
{
   ELM_SCROLLABLE_CHECK(obj);

   eo_do((Eo *) obj, elm_scrollable_interface_gravity_get(x, y));
}

EAPI void
elm_scroller_propagate_events_set(Evas_Object *obj,
                                  Eina_Bool propagation)
{
   ELM_SCROLLABLE_CHECK(obj);
   eo_do(obj, elm_obj_scroller_propagate_events_set(propagation));
}

static void
_propagate_events_set(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool propagation = va_arg(*list, int);
   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   evas_object_propagate_events_set(wd->resize_obj, propagation);
}

EAPI Eina_Bool
elm_scroller_propagate_events_get(const Evas_Object *obj)
{
   ELM_SCROLLABLE_CHECK(obj, EINA_FALSE);

   Eina_Bool ret;
   eo_do((Eo *) obj, elm_obj_scroller_propagate_events_get(&ret));
   return ret;
}

static void
_propagate_events_get(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);
   *ret = evas_object_propagate_events_get(wd->resize_obj);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),

        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), _elm_scroller_smart_member_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_scroller_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_scroller_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_scroller_smart_move),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_THEME), _elm_scroller_smart_theme),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_scroller_smart_event),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_scroller_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT), _elm_scroller_smart_focus_next),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_scroller_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_SUB_OBJECT_DEL), _elm_scroller_smart_sub_object_del),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ACTIVATE), _elm_scroller_smart_activate),

        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_SET), _elm_scroller_smart_content_set),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_GET), _elm_scroller_smart_content_get),
        EO_OP_FUNC(ELM_OBJ_CONTAINER_ID(ELM_OBJ_CONTAINER_SUB_ID_CONTENT_UNSET), _elm_scroller_smart_content_unset),

        EO_OP_FUNC(ELM_OBJ_LAYOUT_ID(ELM_OBJ_LAYOUT_SUB_ID_SIZING_EVAL), _elm_scroller_smart_sizing_eval),

        EO_OP_FUNC(ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_POLICY_SET), _policy_set),
        EO_OP_FUNC(ELM_SCROLLABLE_INTERFACE_ID(ELM_SCROLLABLE_INTERFACE_SUB_ID_PAGE_SIZE_SET), _page_size_set),

        EO_OP_FUNC(ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_CUSTOM_WIDGET_BASE_THEME_SET), _custom_widget_base_theme_set),
        EO_OP_FUNC(ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_REGION_GET), _region_get),
        EO_OP_FUNC(ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_CHILD_SIZE_GET), _child_size_get),
        EO_OP_FUNC(ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_PAGE_RELATIVE_SET), _page_relative_set),
        EO_OP_FUNC(ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_PROPAGATE_EVENTS_SET), _propagate_events_set),
        EO_OP_FUNC(ELM_OBJ_SCROLLER_ID(ELM_OBJ_SCROLLER_SUB_ID_PROPAGATE_EVENTS_GET), _propagate_events_get),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);

   evas_smart_legacy_type_register(MY_CLASS_NAME, klass);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_SCROLLER_SUB_ID_CUSTOM_WIDGET_BASE_THEME_SET, "DEPRECATED: Set custom theme elements for the scroller"),
     EO_OP_DESCRIPTION(ELM_OBJ_SCROLLER_SUB_ID_REGION_GET, "Get the currently visible content region."),
     EO_OP_DESCRIPTION(ELM_OBJ_SCROLLER_SUB_ID_CHILD_SIZE_GET, "Get the size of the content object."),
     EO_OP_DESCRIPTION(ELM_OBJ_SCROLLER_SUB_ID_PAGE_RELATIVE_SET, "Set scroll page size relative to viewport size."),
     EO_OP_DESCRIPTION(ELM_OBJ_SCROLLER_SUB_ID_PROPAGATE_EVENTS_SET, "Set event propagation on a scroller."),
     EO_OP_DESCRIPTION(ELM_OBJ_SCROLLER_SUB_ID_PROPAGATE_EVENTS_GET, "Get event propagation for a scroller."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_SCROLLER_BASE_ID, op_desc, ELM_OBJ_SCROLLER_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Scroller_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_scroller_class_get, &class_desc, ELM_OBJ_LAYOUT_CLASS, ELM_SCROLLABLE_INTERFACE, NULL);
