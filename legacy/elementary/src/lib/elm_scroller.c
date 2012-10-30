#include <Elementary.h>
#include "elm_priv.h"
#include "elm_widget_scroller.h"
EAPI const char ELM_SCROLLER_SMART_NAME[] = "elm_scroller";

static const char SIG_SCROLL[] = "scroll";
static const char SIG_SCROLL_ANIM_START[] = "scroll,anim,start";
static const char SIG_SCROLL_ANIM_STOP[] = "scroll,anim,stop";
static const char SIG_SCROLL_DRAG_START[] = "scroll,drag,start";
static const char SIG_SCROLL_DRAG_STOP[] = "scroll,drag,stop";
static const char SIG_EDGE_LEFT[] = "edge,left";
static const char SIG_EDGE_RIGHT[] = "edge,right";
static const char SIG_EDGE_TOP[] = "edge,top";
static const char SIG_EDGE_BOTTOM[] = "edge,bottom";
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
   {NULL, NULL}
};

static const Evas_Smart_Interface *_smart_interfaces[] =
{
   (Evas_Smart_Interface *)&ELM_SCROLLABLE_IFACE, NULL
};

EVAS_SMART_SUBCLASS_IFACE_NEW
  (ELM_SCROLLER_SMART_NAME, _elm_scroller, Elm_Scroller_Smart_Class,
  Elm_Layout_Smart_Class, elm_layout_smart_class_get, _smart_callbacks,
  _smart_interfaces);

static Eina_Bool
_elm_scroller_smart_event(Evas_Object *obj,
                          Evas_Object *src __UNUSED__,
                          Evas_Callback_Type type,
                          void *event_info)
{
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

   ELM_SCROLLER_DATA_GET(obj, sd);

   if (elm_widget_disabled_get(obj)) return EINA_FALSE;
   if (type != EVAS_CALLBACK_KEY_DOWN) return EINA_FALSE;
   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD) return EINA_FALSE;

   sd->s_iface->content_pos_get(obj, &x, &y);
   sd->s_iface->step_size_get(obj, &step_x, &step_y);
   sd->s_iface->page_size_get(obj, &page_x, &page_y);
   sd->s_iface->content_viewport_size_get(obj, &v_w, &v_h);
   evas_object_geometry_get(sd->content, &c_x, &c_y, &max_x, &max_y);

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
                            return EINA_TRUE;
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
                  return EINA_TRUE;
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
                       return EINA_TRUE;
                    }
               }
          }
     }
   if ((!strcmp(ev->keyname, "Left")) ||
       ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        if (x <= 0) return EINA_FALSE;
        x -= step_x;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        if (x >= (max_x - v_w)) return EINA_FALSE;
        x += step_x;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        if (y == 0) return EINA_FALSE;
        y -= step_y;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        if (y >= (max_y - v_h)) return EINA_FALSE;
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
   else return EINA_FALSE;

   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   sd->s_iface->content_pos_set(obj, x, y);

   return EINA_TRUE;
}

static Eina_Bool
_elm_scroller_smart_activate(Evas_Object *obj, Elm_Activate act)
{
   Evas_Coord x = 0;
   Evas_Coord y = 0;
   Evas_Coord v_w = 0;
   Evas_Coord v_h = 0;
   Evas_Coord page_x = 0;
   Evas_Coord page_y = 0;

   ELM_SCROLLER_DATA_GET(obj, sd);

   if ((elm_widget_disabled_get(obj)) ||
       (act == ELM_ACTIVATE_DEFAULT)) return EINA_FALSE;

   sd->s_iface->content_pos_get(obj, &x, &y);
   sd->s_iface->page_size_get(obj, &page_x, &page_y);
   sd->s_iface->content_viewport_size_get(obj, &v_w, &v_h);

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

   sd->s_iface->content_pos_set(obj, x, y);
   return EINA_TRUE;
}

static void
_elm_scroller_smart_sizing_eval(Evas_Object *obj)
{
   Evas_Coord vw = 0, vh = 0, minw = 0, minh = 0, maxw = 0, maxh = 0, w, h,
              vmw, vmh;
   double xw = 0.0, yw = 0.0;

   ELM_SCROLLER_DATA_GET(obj, sd);

   /* parent class' early call */
   if (!sd->s_iface) return;

   if (sd->content)
     {
        evas_object_size_hint_min_get(sd->content, &minw, &minh);
        evas_object_size_hint_max_get(sd->content, &maxw, &maxh);
        evas_object_size_hint_weight_get(sd->content, &xw, &yw);
     }

   sd->s_iface->content_viewport_size_get(obj, &vw, &vh);
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
   edje_object_size_min_calc(ELM_WIDGET_DATA(sd)->resize_obj, &vmw, &vmh);

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
   ELM_SCROLLER_DATA_GET(obj, sd);

   sd->s_iface->mirrored_set(obj, mirrored);
}

static Eina_Bool
_elm_scroller_smart_theme(Evas_Object *obj)
{
   if (!ELM_WIDGET_CLASS(_elm_scroller_parent_sc)->theme(obj))
     return EINA_FALSE;

   _mirrored_set(obj, elm_widget_mirrored_get(obj));

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Eina_Bool
_elm_scroller_smart_focus_next(const Evas_Object *obj,
                               Elm_Focus_Direction dir,
                               Evas_Object **next)
{
   Evas_Object *cur;

   ELM_SCROLLER_DATA_GET(obj, sd);

   if (!sd->content) return EINA_FALSE;

   cur = sd->content;

   /* Try focus cycle in subitem */
   if (elm_widget_focus_get(obj))
     {
        if ((elm_widget_can_focus_get(cur)) ||
            (elm_widget_child_can_focus_get(cur)))
          return elm_widget_focus_next_get(cur, dir, next);
     }

   /* Return */
   *next = (Evas_Object *)obj;

   return !elm_widget_focus_get(obj);
}

static void
_show_region_hook(void *data,
                  Evas_Object *content_obj)
{
   Evas_Coord x, y, w, h;

   ELM_SCROLLER_DATA_GET(data, sd);

   elm_widget_show_region_get(content_obj, &x, &y, &w, &h);
   sd->s_iface->content_region_show(data, x, y, w, h);
}

static void
_changed_size_hints_cb(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj __UNUSED__,
                       void *event_info __UNUSED__)
{
   elm_layout_sizing_eval(data);
}

static Eina_Bool
_elm_scroller_smart_sub_object_del(Evas_Object *obj,
                                   Evas_Object *sobj)
{
   ELM_SCROLLER_DATA_GET(obj, sd);

   if (!ELM_WIDGET_CLASS(_elm_scroller_parent_sc)->sub_object_del(obj, sobj))
     return EINA_FALSE;

   if (sobj == sd->content)
     {
        elm_widget_on_show_region_hook_set(sd->content, NULL, NULL);

        sd->content = NULL;
     }

   return EINA_TRUE;
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

static Eina_Bool
_elm_scroller_smart_content_set(Evas_Object *obj,
                                const char *part,
                                Evas_Object *content)
{
   ELM_SCROLLER_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS
              (_elm_scroller_parent_sc)->content_set(obj, part, content);

   if (sd->content == content) return EINA_TRUE;

   if (sd->content) evas_object_del(sd->content);
   sd->content = content;

   if (content)
     {
        elm_widget_on_show_region_hook_set(content, _show_region_hook, obj);
        elm_widget_sub_object_add(obj, content);

        sd->s_iface->content_set(obj, content);
     }

   elm_layout_sizing_eval(obj);

   return EINA_TRUE;
}

static Evas_Object *
_elm_scroller_smart_content_get(const Evas_Object *obj,
                                const char *part)
{
   ELM_SCROLLER_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS
              (_elm_scroller_parent_sc)->content_get(obj, part);

   return sd->content;
}

static Evas_Object *
_elm_scroller_smart_content_unset(Evas_Object *obj,
                                  const char *part)
{
   Evas_Object *content;

   ELM_SCROLLER_DATA_GET(obj, sd);

   if (part && strcmp(part, "default"))
     return ELM_CONTAINER_CLASS
              (_elm_scroller_parent_sc)->content_unset(obj, part);

   if (!sd->content) return NULL;

   content = sd->content;
   elm_widget_sub_object_del(obj, sd->content);
   sd->s_iface->content_set(obj, NULL);
   sd->content = NULL;

   return content;
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
_elm_scroller_smart_add(Evas_Object *obj)
{
   Evas_Coord minw, minh;

   EVAS_SMART_DATA_ALLOC(obj, Elm_Scroller_Smart_Data);

   ELM_WIDGET_CLASS(_elm_scroller_parent_sc)->base.add(obj);

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_layout_theme_set(obj, "scroller", "base", elm_widget_style_get(obj));

   priv->hit_rect = evas_object_rectangle_add(evas_object_evas_get(obj));
   evas_object_smart_member_add(priv->hit_rect, obj);
   elm_widget_sub_object_add(obj, priv->hit_rect);

   evas_object_color_set(priv->hit_rect, 0, 0, 0, 0);
   evas_object_show(priv->hit_rect);
   evas_object_repeat_events_set(priv->hit_rect, EINA_TRUE);

   priv->s_iface = evas_object_smart_interface_get
       (obj, ELM_SCROLLABLE_IFACE_NAME);

   priv->s_iface->objects_set
     (obj, ELM_WIDGET_DATA(priv)->resize_obj, priv->hit_rect);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _changed_size_hints_cb, obj);

   edje_object_size_min_calc(ELM_WIDGET_DATA(priv)->resize_obj, &minw, &minh);
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_event_callback_add(obj, EVAS_CALLBACK_RESIZE, _resize_cb, obj);

   priv->s_iface->edge_left_cb_set(obj, _edge_left_cb);
   priv->s_iface->edge_right_cb_set(obj, _edge_right_cb);
   priv->s_iface->edge_top_cb_set(obj, _edge_top_cb);
   priv->s_iface->edge_bottom_cb_set(obj, _edge_bottom_cb);
   priv->s_iface->scroll_cb_set(obj, _scroll_cb);
   priv->s_iface->animate_start_cb_set(obj, _scroll_anim_start_cb);
   priv->s_iface->animate_stop_cb_set(obj, _scroll_anim_stop_cb);
   priv->s_iface->drag_start_cb_set(obj, _scroll_drag_start_cb);
   priv->s_iface->drag_stop_cb_set(obj, _scroll_drag_stop_cb);

   priv->s_iface->content_min_limit_cb_set
     (obj, _elm_scroller_content_min_limit_cb);
}

static void
_elm_scroller_smart_move(Evas_Object *obj,
                         Evas_Coord x,
                         Evas_Coord y)
{
   ELM_SCROLLER_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_scroller_parent_sc)->base.move(obj, x, y);

   evas_object_move(sd->hit_rect, x, y);
}

static void
_elm_scroller_smart_resize(Evas_Object *obj,
                           Evas_Coord w,
                           Evas_Coord h)
{
   ELM_SCROLLER_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_scroller_parent_sc)->base.resize(obj, w, h);

   evas_object_resize(sd->hit_rect, w, h);
}

static void
_elm_scroller_smart_member_add(Evas_Object *obj,
                               Evas_Object *member)
{
   ELM_SCROLLER_DATA_GET(obj, sd);

   ELM_WIDGET_CLASS(_elm_scroller_parent_sc)->base.member_add(obj, member);

   if (sd->hit_rect)
     evas_object_raise(sd->hit_rect);
}

static void
_elm_scroller_smart_set_user(Elm_Scroller_Smart_Class *sc)
{
   ELM_WIDGET_CLASS(sc)->base.add = _elm_scroller_smart_add;
   ELM_WIDGET_CLASS(sc)->base.move = _elm_scroller_smart_move;
   ELM_WIDGET_CLASS(sc)->base.resize = _elm_scroller_smart_resize;
   ELM_WIDGET_CLASS(sc)->base.member_add = _elm_scroller_smart_member_add;

   ELM_WIDGET_CLASS(sc)->sub_object_del = _elm_scroller_smart_sub_object_del;
   ELM_WIDGET_CLASS(sc)->theme = _elm_scroller_smart_theme;
   ELM_WIDGET_CLASS(sc)->focus_next = _elm_scroller_smart_focus_next;
   ELM_WIDGET_CLASS(sc)->event = _elm_scroller_smart_event;
   ELM_WIDGET_CLASS(sc)->focus_direction = NULL;
   ELM_WIDGET_CLASS(sc)->activate = _elm_scroller_smart_activate;

   ELM_CONTAINER_CLASS(sc)->content_set = _elm_scroller_smart_content_set;
   ELM_CONTAINER_CLASS(sc)->content_get = _elm_scroller_smart_content_get;
   ELM_CONTAINER_CLASS(sc)->content_unset = _elm_scroller_smart_content_unset;

   ELM_LAYOUT_CLASS(sc)->sizing_eval = _elm_scroller_smart_sizing_eval;
}

EAPI const Elm_Scroller_Smart_Class *
elm_scroller_smart_class_get(void)
{
   static Elm_Scroller_Smart_Class _sc =
     ELM_SCROLLER_SMART_CLASS_INIT_NAME_VERSION(ELM_SCROLLER_SMART_NAME);
   static const Elm_Scroller_Smart_Class *class = NULL;
   Evas_Smart_Class *esc = (Evas_Smart_Class *)&_sc;

   if (class)
     return class;

   _elm_scroller_smart_set(&_sc);
   esc->callbacks = _smart_callbacks;
   class = &_sc;

   return class;
}

EAPI Evas_Object *
elm_scroller_add(Evas_Object *parent)
{
   Evas *e;
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   e = evas_object_evas_get(parent);
   if (!e) return NULL;

   obj = evas_object_smart_add(e, _elm_scroller_smart_class_new());

   if (!elm_widget_sub_object_add(parent, obj))
     ERR("could not add %p as sub object of %p", obj, parent);

   return obj;
}

/* deprecated */
EAPI void
elm_scroller_custom_widget_base_theme_set(Evas_Object *obj,
                                          const char *klass,
                                          const char *group)
{
   ELM_SCROLLER_CHECK(obj);
   ELM_SCROLLER_DATA_GET(obj, sd);

   EINA_SAFETY_ON_NULL_RETURN(klass);
   EINA_SAFETY_ON_NULL_RETURN(group);

   if (eina_stringshare_replace(&(ELM_LAYOUT_DATA(sd)->klass), klass) ||
       eina_stringshare_replace(&(ELM_LAYOUT_DATA(sd)->group), group))
     _elm_scroller_smart_theme(obj);
}

EAPI void
elm_scroller_content_min_limit(Evas_Object *obj,
                               Eina_Bool w,
                               Eina_Bool h)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->content_min_limit(obj, w, h);
}

EAPI void
elm_scroller_region_show(Evas_Object *obj,
                         Evas_Coord x,
                         Evas_Coord y,
                         Evas_Coord w,
                         Evas_Coord h)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->content_region_show(obj, x, y, w, h);
}

EAPI void
elm_scroller_policy_set(Evas_Object *obj,
                        Elm_Scroller_Policy policy_h,
                        Elm_Scroller_Policy policy_v)
{
   ELM_SCROLLABLE_CHECK(obj);

   if ((policy_h >= ELM_SCROLLER_POLICY_LAST) ||
       (policy_v >= ELM_SCROLLER_POLICY_LAST))
     return;

   s_iface->policy_set(obj, policy_h, policy_v);
}

EAPI void
elm_scroller_policy_get(const Evas_Object *obj,
                        Elm_Scroller_Policy *policy_h,
                        Elm_Scroller_Policy *policy_v)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->policy_get(obj, policy_h, policy_v);
}

EAPI void
elm_scroller_region_get(const Evas_Object *obj,
                        Evas_Coord *x,
                        Evas_Coord *y,
                        Evas_Coord *w,
                        Evas_Coord *h)
{
   ELM_SCROLLABLE_CHECK(obj);

   if ((x) || (y)) s_iface->content_pos_get(obj, x, y);
   if ((w) || (h)) s_iface->content_viewport_size_get(obj, w, h);
}

EAPI void
elm_scroller_child_size_get(const Evas_Object *obj,
                            Evas_Coord *w,
                            Evas_Coord *h)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->content_size_get(obj, w, h);
}

EAPI void
elm_scroller_bounce_set(Evas_Object *obj,
                        Eina_Bool h_bounce,
                        Eina_Bool v_bounce)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->bounce_allow_set(obj, h_bounce, v_bounce);
}

EAPI void
elm_scroller_bounce_get(const Evas_Object *obj,
                        Eina_Bool *h_bounce,
                        Eina_Bool *v_bounce)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->bounce_allow_get(obj, h_bounce, v_bounce);
}

EAPI void
elm_scroller_page_relative_set(Evas_Object *obj,
                               double h_pagerel,
                               double v_pagerel)
{
   Evas_Coord pagesize_h, pagesize_v;

   ELM_SCROLLABLE_CHECK(obj);

   s_iface->paging_get(obj, NULL, NULL, &pagesize_h, &pagesize_v);

   s_iface->paging_set
     (obj, h_pagerel, v_pagerel, pagesize_h, pagesize_v);
}

EAPI void
elm_scroller_page_relative_get(const Evas_Object *obj,
                               double *h_pagerel,
                               double *v_pagerel)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->paging_get(obj, h_pagerel, v_pagerel, NULL, NULL);
}

EAPI void
elm_scroller_page_size_set(Evas_Object *obj,
                           Evas_Coord h_pagesize,
                           Evas_Coord v_pagesize)
{
   double pagerel_h, pagerel_v;

   ELM_SCROLLABLE_CHECK(obj);

   s_iface->paging_get(obj, &pagerel_h, &pagerel_v, NULL, NULL);

   s_iface->paging_set
     (obj, pagerel_h, pagerel_v, h_pagesize, v_pagesize);
}

EAPI void
elm_scroller_page_size_get(const Evas_Object *obj,
                           Evas_Coord *h_pagesize,
                           Evas_Coord *v_pagesize)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->paging_get(obj, NULL, NULL, h_pagesize, v_pagesize);
}

EAPI void
elm_scroller_current_page_get(const Evas_Object *obj,
                              int *h_pagenumber,
                              int *v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->current_page_get(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_last_page_get(const Evas_Object *obj,
                           int *h_pagenumber,
                           int *v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->last_page_get(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_page_show(Evas_Object *obj,
                       int h_pagenumber,
                       int v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->page_show(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_page_bring_in(Evas_Object *obj,
                           int h_pagenumber,
                           int v_pagenumber)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->page_bring_in(obj, h_pagenumber, v_pagenumber);
}

EAPI void
elm_scroller_region_bring_in(Evas_Object *obj,
                             Evas_Coord x,
                             Evas_Coord y,
                             Evas_Coord w,
                             Evas_Coord h)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->region_bring_in(obj, x, y, w, h);
}

EAPI void
elm_scroller_gravity_set(Evas_Object *obj,
                         double x,
                         double y)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->gravity_set(obj, x, y);
}

EAPI void
elm_scroller_gravity_get(const Evas_Object *obj,
                         double *x,
                         double *y)
{
   ELM_SCROLLABLE_CHECK(obj);

   s_iface->gravity_get(obj, x, y);
}

EAPI void
elm_scroller_propagate_events_set(Evas_Object *obj,
                                  Eina_Bool propagation)
{
   Elm_Widget_Smart_Data *sd;

   ELM_SCROLLABLE_CHECK(obj);

   sd = evas_object_smart_data_get(obj);
   if (!sd) return;  /* just being paranoid */

   evas_object_propagate_events_set(sd->resize_obj, propagation);
}

EAPI Eina_Bool
elm_scroller_propagate_events_get(const Evas_Object *obj)
{
   Elm_Widget_Smart_Data *sd;

   ELM_SCROLLABLE_CHECK(obj, EINA_FALSE);

   sd = evas_object_smart_data_get(obj);
   if (!sd) return EINA_FALSE;  /* just being paranoid */

   return evas_object_propagate_events_get(sd->resize_obj);
}
